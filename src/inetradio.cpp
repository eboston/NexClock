#include <Arduino.h>
#include <VS1053.h>
#include <WiFi.h>
#include <LITTLEFS.h>

#include "inetradio.h"
#include "my_cbuf.h"
#include "NexClock.h"
#include "settings.h"

// Wiring of VS1053 board (SPI connected in a standard way) on ESP32 only
#define VS1053_CS 32
#define VS1053_DCS 33
#define VS1053_DREQ 35
#define VOLUME 100 // treble/bass works better if NOT 100 here

#define PLAY		"4"
#define PAUSE		"<"

// MP3 decoder
VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

// Dedicated 32-byte buffer for VS1053 aligned on 4-byte boundary for efficiency
uint8_t mp3buff[32] __attribute__((aligned(4)));

// This determines whether the buffer contains sufficient data to allow playing
// It is shared between the main loop() and the task below, hence volatile.
volatile bool canPlayMusicFromBuffer = false;
volatile bool stationConnected = false;

// Create the task handle (a reference to the task being created later)
TaskHandle_t playMusicTaskHandle;
TaskHandle_t handleIncomingRadioDataTaskHandle;

// Circular "Read Buffer" to stop stuttering on some stations
#ifdef BOARD_HAS_PSRAM
//#define CIRCULARBUFFERSIZE 2097152	// 131.072 seconds
#define CIRCULARBUFFERSIZE 150000		// 9.375 seconds
//#define CIRCULARBUFFERSIZE 50000		// 3.123 seconds
#else
#define CIRCULARBUFFERSIZE 10000		// 0.625 seconds
#endif

uint32_t nextMetadata = 0;

WiFiClient client;

my_cbuf* pCircBuffer;

std::string streamArtistTitle = "";

#define streamingCharsMax 32

// Internet stream buffer that we copy in chunks to the ring buffer
char readBuffer[100] __attribute__((aligned(4)));

bool METADATA = true;
uint16_t metaDataInterval = 0; //bytes
uint16_t bytesUntilmetaData = 0;
int bitRate = 0;
bool redirected = false;

#include "stations.h"

const int8_t stationCnt = sizeof(radioStation) / sizeof(struct radioStationLayout);



// Our streaming station has been 'redirected' to another URL
// The header will look like: Location: http://<new host / path>[:<port>]
void getRedirectedStationInfo(String header, int currStationNo)
{
	log_w("--------------------------------------");
	log_w(" Extracting redirection information");
	log_w("--------------------------------------");

	// Placeholders for the new host/path
	String redirectedHost = "";
	String redirectedPath = "";

	// We'll assume the port is 80 unless we find one in the host name
	int redirectedPort = 80;

	// Skip the "redirected http://" bit at the front
	header = header.substring(17);
	log_w("Redirecting to: %s", header.c_str());

	// Split the header into host and path constituents
	int pathDelimiter = header.indexOf("/");
	if (pathDelimiter > 0)
	{
		redirectedPath = header.substring(pathDelimiter);
		redirectedHost = header.substring(0, pathDelimiter);
	}
	// Look to split host into host and port number
	// Example: stream/myradio.de:8080
	int portDelimter = header.indexOf(":");
	if (portDelimter > 0)
	{
		redirectedPort = header.substring(portDelimter + 1).toInt();

		// Adjust the host name to exclude the port information
		redirectedHost = redirectedHost.substring(0, portDelimter);
	}

	// Just overwrite the current entry for this station (reverts on reboot)
	// TODO: consider writing all this to EEPROM / SPIFFS
	log_w("New address: %s%s:%d", redirectedHost.c_str(), redirectedPath.c_str(), redirectedPort);
	strncpy(radioStation[currStationNo].host, redirectedHost.c_str(), 64);
	strncpy(radioStation[currStationNo].path, redirectedPath.c_str(), 128);

	return;
}


// Connect to the station list number
bool stationConnect(int stationNo)
{
	if (stationNo < 0 || stationNo > stationCnt)
		stationNo = 0;

	pRadio_t0.setText("Connecting to station");
	pRadio_t1.setText("");
	pRadio_t2.setText(radioStation[stationNo].friendlyName);

	log_w("--------------------------------------");
	log_w("        Connecting to station %d", stationNo);
	log_w("--------------------------------------");
	// Flag to indicate we need to buffer data before allowing player to stream audio
	canPlayMusicFromBuffer = false;

	// Disconnect from current station.  Pause radio data processing
	stationConnected = false;

	// Clear down the streaming buffer and optionally reset the player (to flush it)
	pCircBuffer->flush();
	nextMetadata = 0;

	// ****** This will be a settings option
	// Determine whether we want ICY metadata
//	METADATA = digitalRead(ICYDATAPIN) == HIGH;

	// For THIS radio station have we FORCED meta data to be ignored?
	METADATA = radioStation[stationNo].useMetaData;
	if (!radioStation[stationNo].useMetaData)
	{
		log_w("METADATA ignored for this radio station.");
	}

	// Set the metadataInterval value to zero so we can detect that we found a valid one
	metaDataInterval = 0;

	// We try a few times to connect to the station
	int connectAttempt = 0;

	while (!stationConnected && connectAttempt < 5)
	{
		if (redirected)
		{
			log_w("REDIRECTED URL DETECTED FOR STATION %d", stationNo);
		}

		connectAttempt++;
		log_w("Host: %s Port:%d", radioStation[stationNo].host, radioStation[stationNo].port);

		// Connect to the redirected URL
		if (client.connect(radioStation[stationNo].host, radioStation[stationNo].port))
		{
			ClockSettings.lastRadioStation = stationNo;
			writeSettings();			

			log_w("Connected to station");
			pRadio_t0.setText("Connected");
			stationConnected = true;
		}
	}

	// If we could not connect (eg bad URL) just exit
	if (!stationConnected)
	{
		pRadio_t0.setText("Unable to connect to station");
		log_w("Could not connect to %s", radioStation[stationNo].host);
		return false;
	}
	else
	{
		log_w("Connected to %s (%s%s)",
					  radioStation[stationNo].host, radioStation[stationNo].friendlyName,
					  redirected ? " - redirected" : "");
	}

	// Get the data stream plus any metadata (eg station name, track info between songs / ads)
	// TODO: Allow retries here (BBC Radio 4 very finicky before streaming).
	// We might also get a redirection URL given back.
	client.print(
		String("GET ") + radioStation[stationNo].path + " HTTP/1.1\r\n" +
		"Host: " + radioStation[stationNo].host + "\r\n" +
		(METADATA ? "Icy-MetaData:1\r\n" : "") +
		"Connection: close\r\n\r\n");

	// Give the client a chance to connect
	int retryCnt = 30;
	while (client.available() == 0 && --retryCnt > 0)
	{
		delay(100);
	}

	// If we aren't getting any data, I guess we are not connected
	if (client.available() < 1)
	{
		stationConnected = false;
		return false;
	}

	// Keep reading until we read two LF bytes in a row.
	//
	// The ICY (I Can Yell, a precursor to Shoutcast) format:
	// In the http response we can look for icy-metaint: XXXX to tell us how far apart
	// the header information (in bytes) is sent.

	// Process all responses until we run out of header info (blank header)
	while (client.available())
	{
		// Delimiter char is not included in data returned
		String responseLine = client.readStringUntil('\n');

		if (responseLine.indexOf("Status: 200 OK") > 0)
		{
			// TODO: we really should check we have a response of 200 before anything else
			log_w("200 - OK response");
			continue;
		}

		// If we have an EMPTY header (or just a CR) that means we had two linefeeds
		if (responseLine[0] == (uint8_t)13 || responseLine == "")
		{
			break;
		}

		// If the header is not empty process it
		//log_w("HEADER: ");
		//log_w("%s", responseLine.c_str());

		// Critical value for this whole sketch to work: bytes between "frames"
		// Sometimes you can't get this first time round so we just reconnect
		// (Actually it's only BCC Radio 4 that doesn't always give this out)
		if (responseLine.startsWith("icy-metaint"))
		{
			metaDataInterval = responseLine.substring(12).toInt();
			log_w("NEW Metadata Interval:%d", metaDataInterval);
			continue;
		}

		// The bit rate of the transmission (FYI) eye candy
		if (responseLine.startsWith("icy-br:"))
		{
			bitRate = responseLine.substring(7).toInt();
			log_w("Bit rate:%d", bitRate);
			continue;
		}

/*
		// TODO: Remove this testing override for station 4 (always redirects!)
		// The URL we used has been redirected
		if (!redirected && stationNo == 4)
		{
			responseLine = "location: http://stream.antenne1.de:80/a1stg/livestream1.aac";
		}
*/

		if (responseLine.startsWith("location: http://"))
		{
			getRedirectedStationInfo(responseLine, stationNo);
			redirected = true;
			if (!stationConnect(ClockSettings.lastRadioStation))
				return false;
		}
		redirected = false;
	}

	// If we didn't find required metaDataInterval value in the headers, abort this connection
	if (metaDataInterval == 0 && METADATA)
	{
		log_w("NO METADATA INTERVAL DETECTED - RECONNECTING");
		return false;
	}

	// Update the count of bytes until the next metadata interval (used in loop) and exit
	bytesUntilmetaData = metaDataInterval;

	// All done here
	return true;
}


// Populate ring buffer with streaming data
void populateRingBuffer()
{
	// Signed because we might -1 returned
	signed int bytesReadFromStream = 0;

	// Room in the ring buffer for (up to) X bytes?
	if (pCircBuffer->room() >= streamingCharsMax)
	{
		// Read either the maximum available (max 100) or the number of bytes to the next meata data interval
		bytesReadFromStream = client.read((uint8_t *)readBuffer, min(streamingCharsMax, METADATA ? (int)bytesUntilmetaData : streamingCharsMax));

		// If we get -1 here it means nothing could be read from the stream
		// TODO: find out why this might be. Remote server not streaming?
		if (bytesReadFromStream > 0)
		{
			// Add them to the circular buffer
			pCircBuffer->write(readBuffer, bytesReadFromStream);

			// If we didn't read the amount we "expected" debug that here
			// if (bytesReadFromStream < streamingCharsMax && bytesReadFromStream != bytesUntilmetaData)
			// {
			// 	Serial.printf("%db to circ\n", bytesReadFromStream);
			// }

			// Subtract bytes actually read from incoming http data stream from the bytesUntilmetaData
			bytesUntilmetaData -= bytesReadFromStream;
		}
	}
}


void displayMetadata()
{
//streamArtistTitle = "Bill Medley,Jennifer Warnes - (I've Had) The Time of My Life - From \"Dirty Dancing\" Soundtrack";
	// Always output the Artist/Track information even if just to clear it from screen
	int nIdx = streamArtistTitle.find(" - ");

//log_w("md='%s'", streamArtistTitle.c_str());
//log_w("nIdx=%d", nIdx);
//log_w("title='%s'", streamArtistTitle.substr(nIdx + 3).c_str());
//log_w("artist='%s'", streamArtistTitle.substr(0, nIdx - 0).c_str());


	pRadio_t0.setText(streamArtistTitle.substr(nIdx + 3).c_str());
	pRadio_t1.setText(streamArtistTitle.substr(0, nIdx - 0).c_str());

	pClock_SongTitleArtist.setText(streamArtistTitle.c_str());
}


inline bool readMetaData()
{
	// The first byte is going to be the length of the metadata
	int metaDataLength = client.read();

	// Usually there is none as track/artist info is only updated when it changes
	// It may also return the station URL (not necessarily the same as we are using).
	// Example:
	//  'StreamTitle='Love Is The Drug - Roxy Music';StreamUrl='https://listenapi.planetradio.co.uk/api9/eventdata/62247302';'
	if (metaDataLength < 1) // includes -1
	{
		// Warning sends out about 4 times per second!
		//Serial.println("No metadata to read.");
		return true;
	}

	// The actual length is 16 times bigger to allow from 16 up to 4080 bytes (255 * 16) of metadata
	metaDataLength = (metaDataLength * 16);

	char* pMetaData = (char*)malloc(metaDataLength + 1);
	char* p = pMetaData;
	int idx = 0;

	while (idx < metaDataLength)
	{
		if (client.available())
		{
			*p = client.read();
			if (*p > 0 && !isprint(*p))
			{
				log_w("Corrupt metadata found");
				free(pMetaData);
				return false;
			}

			++p;
			++idx;
		}
	}

	// Extract track Title/Artist from this string
	char *startTrack = NULL;
	char *endTrack = NULL;

	startTrack = strstr(pMetaData, "StreamTitle");
	if (startTrack != NULL)
	{
		// We have found the streamtitle so just skip over "StreamTitle="
		startTrack += 12;

		// Now look for the end marker
		endTrack = strstr(startTrack, ";");
		if (endTrack == NULL)
		{
			// No end (very weird), so just set it as the string length
			endTrack = (char *)startTrack + strlen(startTrack);
		}

		// There might be an opening and closing quote so skip over those (reduce data width) too
		if (startTrack[0] == '\'')
		{
			startTrack += 1;
			endTrack -= 1;
		}

		// We MUST terminate the 'string' (character array) with a null to denote the end
		endTrack[0] = '\0';

		// Extract the data by adjusting pointers
		ptrdiff_t startIdx = startTrack - pMetaData;
		ptrdiff_t endIdx = endTrack - pMetaData;
		std::string streamInfo(pMetaData, startIdx, endIdx);
		streamArtistTitle = streamInfo;

		// Debug only if there is something to see
		if (streamArtistTitle != "")
		{
			log_w("%s", streamArtistTitle.c_str());
			nextMetadata = pCircBuffer->end();
		}
		else
		{
			pRadio_t0.setText("");
			pRadio_t0.setText("");
		}
	}

	// All done
	free(pMetaData);
	return true;
}


// Read the ringBuffer and give to VS1053 to play
bool playMusicFromRingBuffer()
{
	// Did we run out of data to send the VS1053 decoder?
	bool dataPanic = false;

	// Now read (up to) 32 bytes of audio data and play it
	if (pCircBuffer->available() >= 32)
	{
		// Does the VS1053 actually want any more data (yet)?
		if (player.data_request())
		{
			// Read (up to) 32 bytes of data from the circular (ring) buffer
			int bytesRead = pCircBuffer->read((char *)mp3buff, 32);

			if (nextMetadata != 0 && (pCircBuffer->begin() - bytesRead <= nextMetadata && nextMetadata <= pCircBuffer->begin()))
			{
				displayMetadata();
				nextMetadata = 0;
			}

			// If we didn't read the full 32 bytes, that's a worry!
			if (bytesRead != 32)
			{
				log_w("Only read %db from ring buff\n", bytesRead);
				dataPanic = true;
			}

/*
			const char* spinner = "|/-\\*";
			static int spinnerCnt = 0;
			spinnerCnt++;
			spinnerCnt %= sizeof(spinner);
			Serial.printf("  %c   %d  \r", spinner[spinnerCnt], bytesRead);
*/
			// Actually send the data to the VS1053
			player.playChunk(mp3buff, bytesRead);
		}
	}
	else
	{
		Serial.printf("Buffer is empty\r");
	}

	return !dataPanic;
}


// Copy streaming data to our ring buffer and check wheter there's enough to start playing yet
// This is only excuted after a station connect to give the buffer a chance to fill up.
void checkBufferForPlaying()
{
	uint32_t startLevel = min(150000, CIRCULARBUFFERSIZE / 3);

	// If we have now got enough in the ring buffer to allow playing to start without stuttering?
	canPlayMusicFromBuffer = (pCircBuffer->available() > startLevel);
}


void connectToWiFi();

void handleIncomingRadioData()
{
	if (stationConnected)
	{
		if (client.available())
		{
			populateRingBuffer();

			if (bytesUntilmetaData == 0 && METADATA)
			{
				if (!readMetaData())
				{
					log_w("Unable to read meta data.  Reconnect to station");
					stationConnect(ClockSettings.lastRadioStation);
				}

				bytesUntilmetaData = metaDataInterval;
			}

			if (!client.connected())
			{
				connectToWiFi();
				stationConnect(ClockSettings.lastRadioStation);
			}
		}
	}
}

void handleIncomingRadioDataTask(void *parameter)
{
	static unsigned long prevMillis = 0;

	while (true)
	{
		handleIncomingRadioData();

		if (millis() - prevMillis > 60000)
		{
			unsigned long remainingStack = uxTaskGetStackHighWaterMark(NULL);
			log_w("Free stack:%lu\n", remainingStack);
			prevMillis = millis();
		}
	}
}


// This is the task that we will start running (on Core 1, don't use Core 0)
void playMusicTask(void *parameter)
{
	static unsigned long prevMillisS = 0;

	// Do this forever
	while (true)
	{
		// If we (no longer) need to buffer the streaming data (after a station change)
		// allow the buffer to be played, but if we get an error stop playing for a while
		if (canPlayMusicFromBuffer)
		{
			// If we failed to play 32b from the buffer (insufficient data) set the flag again
			if (!playMusicFromRingBuffer())
			{
				canPlayMusicFromBuffer = false;
			};
		}
		else
		{
			// Otherwise, check whether we now have enough data to start playing
			checkBufferForPlaying();
		}

		if (millis() - prevMillisS > 250)
		{
			char msg[16] = "100%";
			float usedP = 100.0 -(((float)pCircBuffer->room() / (float)pCircBuffer->size()) * 100.0);
			snprintf(msg, sizeof(msg), "%3.0f%%", usedP);
			pRadio_t3.setText(msg);

			uint32_t color = 63488;
			if (usedP > 90.0)
				color = 2016;
			else if (usedP > 40)
				color = 64512;

			pRadio_t3.Set_font_color_pco(color);

//			const char* spinner = "|/-\\*";
//			static int spinnerCnt = 0;
//			spinnerCnt++;
//			spinnerCnt %= sizeof(spinner);
//			Serial.printf("  %c  used=%5.1f%%          \r", spinner[spinnerCnt], usedP);

			prevMillisS = millis();
		}

		// Tight loops can cause the task watchdog (WDT) to fire and reboot the chip
		// yield() will only give way to higher priority tasks, delay() allows all tasks to run
		// delay(1);
		// As it happens this task runs quite happily even if I put a while(1); here!!!
		vTaskDelay(1);
	}
}


// Called from the main setup() routine, it sets up the above task and runs it as soon as it
// is declared (so choose your moment wisely)
void taskSetup()
{
	// Independent Task to play music
	xTaskCreatePinnedToCore(
		playMusicTask,		  /* Function to implement the task */
		"WebRadioPlay",		  /* Name of the task */
		2000,				  /* Stack size in words */
		NULL,				  /* Task input parameter */
		1,					  /* Priority of the task - must be higher than 0 (idle)*/
		&playMusicTaskHandle, /* Task handle. */
		1);					  /* Core where the task should run */

/*
	xTaskCreatePinnedToCore(
		handleIncomingRadioDataTask,
		"GetRadioData",
		9000,
		NULL,
		1,
		&handleIncomingRadioDataTaskHandle,
		1);
*/
}


void mp3Start()
{
    // Set up the ring buffer
	pCircBuffer = new my_cbuf(CIRCULARBUFFERSIZE);

	// Start MP3 Decoder
	player.begin();

	// Wait for the player to be ready to accept data
	while (!player.data_request())
		delay(1);
 
	// You MIGHT have to set the VS1053 to MP3 mode. No harm done if not required!
	player.switchToMp3Mode();

	// Set the volume here to MAX (100)w
	player.setVolume(ClockSettings.volumeLevel);

	uint8_t rtone[4];
	rtone[0] = 3;
	rtone[1] = 3;
	rtone[2] = 10;
	rtone[3] = 3;
	player.setTone(rtone);

	File file = LITTLEFS.open("/Intro.mp3");
	if (file)
	{
		player.startSong();
		uint8_t audioBuffer[32] __attribute__((aligned(4)));
		while (file.available())
		{
			while (!player.data_request()) delay(1);
			int bytesRead = file.read(audioBuffer, 32);
			player.playChunk(audioBuffer, bytesRead);
		}
		file.close();
		player.stopSong();
	}
	else
	{
		log_w("Unable to open greetings file");
	}

    taskSetup();

	playRadio(ClockSettings.bRadioOn, false);
}


void playRadio(bool state, bool saveChange)
{
	log_w("playRadio = %d", state);
	if (state)
	{
		pRadio_b5.setText(PAUSE);
		pRadio_b5.Set_font_color_pco(63488);
		ClockSettings.bRadioOn = true;
		vTaskResume(playMusicTaskHandle);
		if (!stationConnected)
			stationConnect(ClockSettings.lastRadioStation);

		pClock_b3.Set_background_color_bco(2016);
		pClock_b3.Set_font_color_pco(0);
	}
	else
	{
		pRadio_b5.setText(PLAY);
		pRadio_b5.Set_font_color_pco(2016);
		ClockSettings.bRadioOn = false;
		vTaskSuspend(playMusicTaskHandle);
		stationConnected = false;
		pCircBuffer->flush();
		client.stop();
		pRadio_t0.setText("");
		pRadio_t1.setText("");
		pRadio_t2.setText("");
		pRadio_t3.setText("");

		pClock_b3.Set_background_color_bco(50712);
		pClock_b3.Set_font_color_pco(0);
		pClock_SongTitleArtist.setText("");
		

	}

	if (saveChange)
		writeSettings();
}


void pRadio_b0PushCallback(void *ptr)
{
	ClockSettings.volumeLevel--;
	pRadio_VolDisplay.setValue(ClockSettings.volumeLevel);
	player.setVolume(ClockSettings.volumeLevel);
}


void pRadio_b1PushCallback(void *ptr)
{
	ClockSettings.volumeLevel++;
	pRadio_VolDisplay.setValue(ClockSettings.volumeLevel);
	player.setVolume(ClockSettings.volumeLevel);
}


void pRadio_b2PushCallback(void *ptr)
{
	if (ClockSettings.lastRadioStation == 0)
		ClockSettings.lastRadioStation = stationCnt - 1;
	else
		ClockSettings.lastRadioStation--;

	writeSettings();

	stationConnect(ClockSettings.lastRadioStation);
}


void pRadio_b3PushCallback(void *ptr)
{
	ClockSettings.lastRadioStation++;
	if (ClockSettings.lastRadioStation > stationCnt - 1)
		ClockSettings.lastRadioStation = 0;

	writeSettings();

	stationConnect(ClockSettings.lastRadioStation);
}


void pRadio_b5PushCallback(void *ptr)
{
	char ret[8];

	uint16_t rsize = pRadio_b5.getText(ret, sizeof(ret));

	ret[rsize] = '\0';

	if (rsize)
	{
		if (strncmp(ret, PLAY, rsize) == 0)
		{
			playRadio(true);
		}
		else
		{
			playRadio(false);
		}
	}
}
