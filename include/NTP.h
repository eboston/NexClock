#if !defined(NTP_H)
#define NTP_H


extern WiFiUDP Udp;
extern struct tmClock  tmTime;
extern bool bTimeIsValid;

struct tmClock
{
  // tm structure styled variables.
  
  int           tm_sec;                                   // 0 through 59
  int           tm_min;                                   // 0 through 59
  int           tm_hour;                                  // 0 through 23
  int           tm_mday;                                  // 1 through 28 - 31
  int           tm_mon;                                   // 0 through 11
  int           tm_year;                                  // 1970 through 2036ish
  int           tm_wday;                                  // 0 (Sunday) through 6 (Saturday)
  int           tm_yday;                                  // not currently implemented
  int           tm_isdst;                                 // 1 if daylight savings time is on, 0 if not
  
  // Modifications.
  
  unsigned long ulTime;                                   // time in seconds since 1970
  unsigned long ulDstStart;                               // daylight savings time start in seconds since 1970
  unsigned long ulDstEnd;                                 // daylight savings time end in seconds since 1970
  bool          bDstValid;                                // daylight savings time start and end is valid (true) or not (false)
};


void UpdateTime(tmClock * tmTime);
uint8_t StartNTP();




#endif
