#if !defined(__STATIONS_H_)
#define __STATIONS_H_


/* 
    Example URL: [port optional, 80 assumed]
        stream.antenne1.de[:80]/a1stg/livestream1.aac Antenne1 (Stuttgart)
*/

struct radioStationLayout
{
	char host[64];
	char path[128];
	int port;
	char friendlyName[64];
	uint8_t useMetaData;
};


struct radioStationLayout radioStation[] = 
{
	//0
	"stream.antenne1.de",
	"/a1stg/livestream1.aac",
	80,
	"Antenne1.de",
	1,

	//1
	"bbcmedia.ic.llnwd.net",
	"/stream/bbcmedia_radio4fm_mf_q", // also mf_p works
	80,
	"BBC Radio 4",
	0,

	//2
	"stream.antenne1.de",
	"/a1stg/livestream2.mp3",
	80,
	"Antenne1 128k",
	1,

	//3
	"listen.181fm.com",
	"/181-beatles_128k.mp3",
	80,
	"Beatles 128k",
	1,

	//4
	"stream-mz.planetradio.co.uk",
	"/magicmellow.mp3",
	80,
	"Mellow Magic",
	1,

	//5
	"edge-bauermz-03-gos2.sharp-stream.com",
	"/net2national.mp3",
	80,
	"Greatest Hits 112k (National)",
	1,

	//6
	"airspectrum.cdnstream1.com",
	"/1302_192",
	8024,
	"Mowtown Magic Oldies",
	1,

	//7
	"live-bauer-mz.sharp-stream.com",
	"/magicmellow.aac",
	80,
	"Mellow Magic (48k AAC)",
	1,

	// 8
	"us4.internet-radio.com",
	"/",
	8258,
	"Classic Rock Florida",
	1,

	// 9
	"us2.internet-radio.com",
	"/",
	8075,
	"Classic Hits Global",
	1,
};

#endif