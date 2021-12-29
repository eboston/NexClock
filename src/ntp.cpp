#include <Arduino.h>
#include <WiFiUdp.h>
#include <WiFi.h>

#include "main.h"
#include "NTP.h"


String ntpServer = "time.nist.gov";
//String ntpServer = "128.138.140.44";
bool bTimeIsValid = false;


#define UDP_PORT 4000        // ntp time server udp port

WiFiUDP Udp;                                    // udp structure

enum    NtpPacketMode {REQUEST, SENT, RECEIVED};        // possible values for an NtpPacketMode variable.
#define NTP_PACKET_LENGTH                   48          // size, in bytes, of an ntp packet
#define PACKET_DELAY_RESET                  5000           // delay (in seconds at 1hz loop()) before resending an ntp packet request

byte chNtpPacket[NTP_PACKET_LENGTH];         // ntp packet


struct tmClock  tmTime = {0, 0, 0, 1, 2, 2018, 0, 0, 0, 0, 0, false}; // where time occurs
bool            bTimeStatsValid  = false;

int nTimeZone = -6;             // offset from utc to your timezone (code was designed in Oklahoma, US central time zone)

// Daylight savings time.
int nDstStartMonth  = 3;              // start month of year (1 == January, 0 for dst off)
int nDstStartDay    = 0;              // 0 for start on second Sunday, 1 through 28-31 for specific date
int nDstStartHour   = 2;              // start hour (0 through 23)

int nDstEndMonth    = 11;             // end month of year (1 == January, 0 for dst off)
int nDstEndDay      = 0;              // 0 for end on first Sunday, 1 through 28-31 for specific date
int nDstEndHour     = 2;              // end hour (0 through 23)


#define SECONDS_PER_MINUTE  60UL
#define SECONDS_PER_HOUR    (SECONDS_PER_MINUTE * 60UL)
#define SECONDS_PER_DAY     (SECONDS_PER_HOUR * 24UL)

//
// SecondsToDateAndTime
// Convert seconds since 1970 to year, month, day, hours, minutes and seconds.
// Entry   : pointer to tmClock structure
// Returns : nothing
// Notes   :
//
// 1) Derived from this excellent document:  http://howardhinnant.github.io/date_algorithms.html
//

void  SecondsToDateAndTime(tmClock * tmTime)
{
  unsigned long ulZ = (tmTime->ulTime / SECONDS_PER_DAY) + 719468UL;
  unsigned long ulEra = ((ulZ >= 0UL) ? ulZ : ulZ - 146096UL) / 146097UL;
  unsigned long ulDayOfEra = (ulZ - ulEra * 146097UL);
  unsigned long ulYearOfEra = (ulDayOfEra - (ulDayOfEra / 1460UL) + (ulDayOfEra / 36524UL) - (ulDayOfEra / 146096UL)) / 365UL;
  unsigned long ulYear = ulYearOfEra + (ulEra * 400UL);
  unsigned long ulDayOfYear = ulDayOfEra - ((365UL * ulYearOfEra) + (ulYearOfEra / 4UL) - (ulYearOfEra / 100UL));
  unsigned long ulMonthNumber = (5UL * ulDayOfYear + 2UL) / 153UL;
  unsigned long ulDayOfMonth = ulDayOfYear - (153UL * ulMonthNumber + 2UL) / 5UL + 1UL;
  unsigned long ulMonthOfYear = ulMonthNumber + (ulMonthNumber < 10UL ? 3UL : -9UL);
  long          lDays = (tmTime->ulTime / SECONDS_PER_DAY);

  tmTime->tm_year = ulYear + ((ulMonthOfYear <= 2) ? 1 : 0);
  tmTime->tm_mon = ulMonthOfYear - 1;
  tmTime->tm_mday = ulDayOfMonth;
  tmTime->tm_hour = (tmTime->ulTime % SECONDS_PER_DAY) / SECONDS_PER_HOUR;
  tmTime->tm_min = (tmTime->ulTime % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
  tmTime->tm_sec = (tmTime->ulTime % SECONDS_PER_MINUTE);
  tmTime->tm_wday = (lDays >= - 4) ? (lDays + 4) % 7 : (lDays + 5) % 7 + 6;
  
  return; 
}


//
// UpdateNtp
// Update ntp time.
// Entry   : pointer to tmClock structure
// Returns : true if time updated, false if not
// Notes   :
//
//  1) UpdateNtp operates in one of three modes, REQUEST, SENT and RECEIVED:
//
//    a) If nNtpPacketMode == REQUEST, UpdateNtp will create and send an ntp
//       packet to request the time, then set nNtpPacketMode = SENT to indicate
//       the packet has been sent.
//
//    b) If nNtpPacketMode == SENT, UpdateNtp will await a response from the
//       ntp time server.  If a response is received, UpdateNtp will convert
//       the received time from ntp time to unix time and update the tmClock
//       structure, then set nNtpPacketMode = RECEIVED to indicate that the
//       time has been received from the ntp time server.  If a response is
//       not received after five seconds, UpdateNtp will set nNtpPacketMode =
//       REQUEST to request another packet.
//
//    c) If nNtpPacketMode == RECEIVED, UpdateNtp will wait for the top of the
//       next hour, then set nNtpPacketMode = REQUEST.
//
//  2) If everything works as planned:
//
//    a) On cold start, nNtpPacketMode = REQUEST, and bNtpTimeRequestEnabled = false.
//
//    b) UpdateNtp case REQUEST sends a time request to the ntp server, then sets
//       nNtpPacketMode = SENT.
//
//    c) UpdateNtp case SENT waits for the ntp time server response.
//
//    d) If UpdateNtp case SENT receives ntp time from the server, it converts the
//       received time from ntp time to unix time, updates the tmClock structure,
//       then sets nNtpPacketMode = RECEIVED.
//
//    e) If UpdateNtp case SENT does not receive ntp time from the server, it downcounts
//       nNtpPacketDelay until zero, at which time it sets nNtpPacketMode = REQUEST to
//       return to 2.b and start the process again.
//
//    f) UpdateNtp case RECEIVED waits for the top of the next hour, then sets
//       nNtpPacketMode = REQUEST to start the process again.
//
//    g) If all goes well (e.g. the ntp time server responds), steps 2.b, 2.c, 2.d and
//       2.f are continuously repeated at a rate of once per hour.
//
//    h) If all does not go well (e.g. the ntp time server does not respond), steps 2.b,
//       2.c, and 2.e are continuously repeated at a rate of 5hz until the ntp time
//       server responds.
//
//
bool  UpdateNtp(tmClock * tmTime)
{
//  static int              nNtpPacketDelay        = PACKET_DELAY_RESET;
  static NtpPacketMode    nNtpPacketMode         = REQUEST;             // initialized to REQUEST to immediately request ntp time on cold start
  static unsigned long    ulLastNTPRequest       = 0;

  // Determine the current mode.
  switch(nNtpPacketMode)
  {
    case REQUEST:
    {
      // REQUEST mode, create and initialize ntp packet.
      byte chNtpPacket[NTP_PACKET_LENGTH];
      memset(chNtpPacket, 0, NTP_PACKET_LENGTH);
      chNtpPacket[0]  = 0b00011011;

      IPAddress ipNtpServer;
      int err = WiFi.hostByName(ntpServer.c_str(), ipNtpServer);
      if (err != 1)
      {
        log_w("Error code: %d", err);
        ipNtpServer = IPAddress(132,163,96,1);
      }
   
  
      // Send the ntp packet (see https://tf.nist.gov/tf-cgi/servers.cgi for other ip addresses).
      Udp.beginPacket(ipNtpServer, 123);
      Udp.write(chNtpPacket, NTP_PACKET_LENGTH);
      Udp.endPacket();
  
      // Packet has been sent, on the next pass...
      // UpdateNtp is in SENT mode,
      nNtpPacketMode = SENT;

      // and initialize nNtpPacketDelay for possible UpdateNtp SENT mode
      // ntp timeouts.
      ulLastNTPRequest = millis();

      break;
    }
    
    case SENT:
    {
      // SENT mode, an ntp time request has been sent, wait for a response
      // from the ntp time server.
      if(Udp.parsePacket())
      {
        // Server responded, read the packet.
        Udp.read(chNtpPacket, NTP_PACKET_LENGTH);

        // Obtain the time from the packet, convert to Unix time, and adjust for the time zone.
        //
        // Unix time is indicated as the number of seconds since 1/1/1970 at 00:00.
        // NTP time is indicated as the number of seconds since 1/1/1900 at 00:00.
        // Hence there is a 70 year difference between the two, and 17 leap years
        // during those 70 years.
        //
        // The following equation obtains the ntp seconds bytes then converts ntp
        // seconds to unix seconds by subtracting from the ntp time in seconds the
        // value (((70 * 356) + 17) * SECONDS_PER_DAY) where:
        //
        //  1) 70 is the number of years between unix and ntp time.
        //  2) 365 is the number of days in a year.
        //  3) 17 is the number of leap years during the 70 year difference.
  
        tmTime->ulTime = ((unsigned long)chNtpPacket[40] << 24) +       // bits 24 through 31 of ntp time
                         ((unsigned long)chNtpPacket[41] << 16) +       // bits 16 through 23 of ntp time
                         ((unsigned long)chNtpPacket[42] <<  8) +       // bits  8 through 15 of ntp time
                         ((unsigned long)chNtpPacket[43]) -             // bits  0 through  7 of ntp time
                         (((70UL * 365UL) + 17UL) * SECONDS_PER_DAY) +  // ntp to unix conversion
                         (nTimeZone * SECONDS_PER_HOUR) +               // time zone adjustment
                         2UL;                                           // compenstation for ntp request delay
                         
        // Received seconds, update date and time.
        SecondsToDateAndTime(tmTime);
  
        // On the next pass, UpdateNtp is in RECEIVED mode.
        nNtpPacketMode = RECEIVED;

        // Time updated.
        bTimeIsValid = true;
        return true;
      }
      else
      {
        if (millis() - ulLastNTPRequest > 5000)
        {
          // Timeout, on the next pass, place UpdateNtp in REQUEST mode
          // to try again.
          nNtpPacketMode = REQUEST;
        }
      }
      break;
    }
    
    case RECEIVED:
    {
      // RECEIVED mode, ntp time request has been sent and received.  Wait
      // until the top of the next hour to return to REQUEST mode.

      static bool bNtpTimeRequestEnabled = false;   // latch initialized to false for cold start
      
      // Check minutes.
      if (tmTime->tm_min == 0)
      {
        // tmTime->tm_min == 0 (top of hour) will occur for 60 seconds.  To
        // avoid sending 60 ntp time request packets during this time, check
        // if ntp time has already been requested for this hour.
        //
        // I know, I know, you're asking why not check for tmTime->tm_sec ==
        // 0?  In the unusual but plausible event where the clock falls 1 or
        // more seconds behind per hour, the check for tmTime->tm_sec == 0
        // would fail so this code checks for tmTime->tm_min == 0 along with
        // the latching variable bNtpTimeRequestEnabled used as a one shot
        // per hour latch.
        
        if(bNtpTimeRequestEnabled)
        {
          // Entering a new hour, place UpdateNtp in REQUEST mode for the next
          // pass.
          nNtpPacketMode = REQUEST;

          // Then dissable further nNtpPacketMode = REQUEST for the remainder
          // of tmTime->tm_min == 0 for the new hour.
          bNtpTimeRequestEnabled = false;
        }
      }
      else
      {
        // No longer in tmTime->tm_min == 0 for the new hour, enable ntp time request
        // for the next hour.
        bNtpTimeRequestEnabled = true;
      }
    }
    break;
  }
  
  // Time not updated.
  return false;
}


//
// DateAndTimeToSeconds
// Convert year, month, day, hours, minutes and seconds to seconds since 1970.
// Entry   : pointer to tmClock structure
// Returns : time in seconds since 1970
// Notes   :
//
// 1) Derived from this excellent document:  http://howardhinnant.github.io/date_algorithms.html
//

unsigned long DateAndTimeToSeconds(tmClock * tmTime)
{
  long          lYear = (long)tmTime->tm_year - ((((long)tmTime->tm_mon + 1UL) <= 2UL) ? 1UL : 0UL);
  unsigned long ulEra = (((unsigned long)lYear >= 0UL) ? lYear : lYear - 399UL) / 400UL;
  unsigned long ulMonthOfEra = (lYear - (ulEra * 400UL));
  unsigned long ulDayOfYear = (153UL * (((long)tmTime->tm_mon + 1UL) + (((long)tmTime->tm_mon + 1UL) > 2UL ? -3UL : 9UL)) + 2UL) / 5UL + tmTime->tm_mday - 1UL;
  unsigned long ulDayOfEra = (ulMonthOfEra * 365UL) + (ulMonthOfEra / 4UL) - (ulMonthOfEra / 100UL) + ulDayOfYear;
  
  return (((ulEra * 146097UL + ulDayOfEra - 719468UL) * (SECONDS_PER_DAY)) +
          ((long)tmTime->tm_hour * SECONDS_PER_HOUR) +
          ((long)tmTime->tm_min * SECONDS_PER_MINUTE) +
           (long)tmTime->tm_sec);
}


//
// FirstSunday
// Given the week day of the first day of the month, returns the day of the month of the first Sunday.
// Entry   : weekday (0 through 6 for Sunday through Saturday) of the first day of the month
// Returns : monthday (1 through 7) of the first Sunday of the current month
// Notes   :
//
//  1) Example:
//
//    a) If the first weekday of the month == 0 (Sun) then return 1 (first Sun on the 1st day of the month)
//    b) If the first weekday of the month == 1 (Mon) then return 7 (first Sun on the 7th day of the month)
//    c) If the first weekday of the month == 2 (Tue) then return 6 (first Sun on the 6th day of the month)
//    d) If the first weekday of the month == 3 (Wed) then return 5 (first Sun on the 5th day of the month)
//    e) If the first weekday of the month == 4 (Thu) then return 4 (first Sun on the 4th day of the month)
//    f) If the first weekday of the month == 5 (Fri) then return 3 (first Sun on the 3rd day of the month)
//    g) If the first weekday of the month == 6 (Sat) then return 2 (first Sun on the 2nd day of the month)
//

int FirstSunday(int nDayOfWeek)
{
  static const int  nFirstSundayDayOfMonthByDayOfWeek[] = {1, 7, 6, 5, 4, 3, 2};
  return(nFirstSundayDayOfMonthByDayOfWeek[nDayOfWeek % 6]);
}


//
// SecondSunday
// Given the week day of the first day of the month, returns the day of the month of the second Sunday.
// Entry   : weekday (0 through 6 for Sunday through Saturday) of the first day of the current month
// Returns : monthday (8  through 14) of the first Sunday of the current month
// Notes   :
//
//  1) Example:
//
//    a) If the first weekday of the month == 0 (Sun) then return  8 (second Sun on the 8th day of the month)
//    b) If the first weekday of the month == 1 (Mon) then return 14 (second Sun on the 14th day of the month)
//    c) If the first weekday of the month == 2 (Tue) then return 13 (second Sun on the 13th day of the month)
//    d) If the first weekday of the month == 3 (Wed) then return 12 (second Sun on the 12th day of the month)
//    e) If the first weekday of the month == 4 (Thu) then return 11 (second Sun on the 11th day of the month)
//    f) If the first weekday of the month == 5 (Fri) then return 10 (second Sun on the 10th day of the month)
//    g) If the first weekday of the month == 6 (Sat) then return  9 (second Sun on the 9th day of the month)
//
int   SecondSunday(int nDayOfWeek)
{
  return (FirstSunday(nDayOfWeek) + 7);
}


//
// UpdateDstStartAndEnd
// Update the daylight savings time variables ulDstStart and ulDstEnd.
// Entry   : the year in which to generate dst variables
//         : pointer to tmClock structure
// Returns : nothing
//
void UpdateDstStartAndEnd(int nYear, tmClock * tmTime)
{
  // Local variables.
  tmClock  tmTimeLocal;

  // Both user settings months must be non-zero for dst to be processed.
  if(nDstStartMonth && nDstEndMonth)
  {
    // User desires dst, calculate the start date of dst in seconds for this year.
    // Obtain the seconds since 1970 for the start month of the year
    // (see tmClock structure for element details).
    tmTimeLocal = {0, 0, 0, 1, (nDstStartMonth - 1) % 12, nYear, 0, 0, 0, 0, 0, false};                        
    tmTimeLocal.ulTime = DateAndTimeToSeconds(& tmTimeLocal);
  
    // Fill in the remaining tmTime elements.
    SecondsToDateAndTime(& tmTimeLocal);
  
    // Set tm_mday and tm_hour based on user input.
    //
    // Note if the user enters 0 for the day, then use SecondSunday(),
    // else use the user day.
    tmTimeLocal.tm_mday = (nDstStartDay) ? nDstStartDay : SecondSunday(tmTimeLocal.tm_wday);
    tmTimeLocal.tm_hour = nDstStartHour % 24;

    // Set tmTime->ulDstStart.
    tmTime->ulDstStart = DateAndTimeToSeconds(& tmTimeLocal);
                     
    // Obtain the seconds since 1970 for the end month of the year
    // (see tmClock structure for element details).
    tmTimeLocal = {0, 0, 0, 1, (nDstEndMonth - 1) % 12, nYear, 0, 0, 0, 0, 0, false};                        
    tmTimeLocal.ulTime = DateAndTimeToSeconds(& tmTimeLocal);
  
    // Fill in the remaining tmTime elements.
    SecondsToDateAndTime(& tmTimeLocal);

    // Set tm_mday and tm_hour based on user input.
    //
    // Note if the user enters 0 for the day, then use FirstSunday(),
    // else use the user day.  Note also the clock logic runs on
    // ntp time which does not account for dst, so the end hour is
    // set to the user end hour - 1.
    tmTimeLocal.tm_mday = (nDstEndDay) ? nDstEndDay : FirstSunday(tmTimeLocal.tm_wday);
    tmTimeLocal.tm_hour = (nDstEndHour - 1) % 24;
    
    // Set tmTime->ulDstEnd.
    tmTime->ulDstEnd = DateAndTimeToSeconds(& tmTimeLocal);

    // Dst start and end are now valid.
    tmTime->bDstValid = true;
  }
}


//
// IsDst
// Returns true if daylight savings time is on, false is not.
// Entry   : pointer to tmClock structure
// Returns : 1 if dst is on, 0 if not
// Notes   :
//
// 1) The return value is true if (current time in seconds >= daylight savings time
//    start in seconds) and (current time in seconds < daylight savings time end
//    in seconds), false if not.
//
int IsDst(tmClock * tmTime)
{
  if((tmTime->ulTime >= tmTime->ulDstStart) && (tmTime->ulTime < tmTime->ulDstEnd) && nDstEndMonth && nDstStartMonth)
  {
    // dst is on.
    return 1;
  }
  else
  {
    // dst is off.
    return 0;
  }
}


//
// UpdateTime
// Update time.
// Entry   : pointer to tmClock structure
// Returns : nothing
//

void UpdateTime(tmClock * tmTime)
{
  // Check for an ntp update.
  
  if(UpdateNtp(tmTime))
  {
    // UpdateNtp() did update time, update daylight savings start and end times.
    //
    // Note UpdateNtp() updates ntp at the top of each hour, so after each update
    // is a good time for updating the dst start and end times.
    
    UpdateDstStartAndEnd(tmTime->tm_year, tmTime);
  }
  else
  {
    // UpdateNtp() did not update time, manually update time
    // in seconds.
    
    tmTime->ulTime += 1;
    
    // Update year, month, day, hours, minutes, seconds from ulTime.
    
    SecondsToDateAndTime(tmTime);
  }
    
  // Update daylight savings time status.

  if(tmTime->bDstValid)
    tmTime->tm_isdst = IsDst(tmTime);
}

uint8_t StartNTP()
{
    return Udp.begin(UDP_PORT);
}