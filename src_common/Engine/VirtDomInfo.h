#ifndef	__VIRTDOMINFO_H
#define __VIRTDOMINFO_H

// System Includes
#include "FWA.h"			// always the first include
#include <ctime>			// needed for time_t
#include <map>


// Project Includes
#include "Stats.h"			// needed for StatList
#include "myansi.h"			// need this for __int64 definition
#include "FailedRequestInfo.h"


typedef struct {
	// common for all
	long	Hour;
	long	Week;
	long	Weekdays[8];
	long	Month;
	long	Date;
	long 	Client;
	long 	User;
	long 	Govt;
	long 	SourceAddr;
	long	SecondDomain;
	long	Domain;
	long	Orgs;
	long	Regions;
	long	Circulation;
	long	Loyalty;
	long	Timeon;
	// web server data
	long	File;
	long	Browser;
	long	OperSys;
	long	Refer;
	long	ReferSite;
	long	Dir;
	long	Groups;
	long	TopDir;
	long	Type;
	long	Errors;
	long	ErrorURL;
	long	Pages;
	long	MeanPath;
	long	SearchStr;
	long	SearchSite;
	long	Download;
	long	AddHit;
	long	Robot;
	// firewall/router data
	long	Protocol;
	long	HTTP;
	long	HTTPS;
	long	Mail;
	long	FTP;
	long	Telnet;
	long	RealAudio;
	long	DNS;
	long	POP3;
	long	Others;
} doneRec;


typedef struct MeanPathRec {
	long	hashseq;				// sequence hash of multiple page hashes, unique for all orders
	long	clientHashId;			// client hash that it belongs to
	long	pageIndex;				// offset with in the session of this sequence
	short	count;
	long	hits;					// and how many hits do we get for this sequence.
} MeanPathData;



#define MAX_DOMAINNAMESIZE		199
#define MAX_DOMAINPATHSIZE		127

/* Virtual Domain struct for each HOST */
struct VDinfo
{
	// Construction, Destruction, Initialisation & Assignment
	friend VDinfo* InitVirtualDomain( int, char*, short );	// a psuedo ctor
	VDinfo();
	~VDinfo();	// Intentionally non-virtual - this aint a base class.  Also, a virtual dtor would stop existing FWA DB's from loading.
	

	// Public operations	
	const char* GetSiteURL() const;
	CQFailedRequestInfo& GetFailedRequestInfo( Statistic* failedRequestStat );
	size_t GetNumFailedRequests( Statistic* failedRequestStat );

	// Attributes, most of which are public
	long			domainNum;			// this is Domain X [0..15]
	long			domainTotal;		// total amount of domains
	char			domainName[MAX_DOMAINNAMESIZE+1];	// This virtual Domains NAME
	char			domainPath[MAX_DOMAINPATHSIZE+1];
	long			totalInDataSize;	// total in Log file size
	short			logType, logStyle;
	double			time1,time2;
	long 			totalFailedRequests,
		 			totalRequests,
					totalCachedHits,
					totalDays,
					hash,
					badones,
					totalUniqueClients,
					totalFlushedClients,
					totalRepeatClients,
					totalRedirectedHits,
					total404Hits,
					pad1[1];
private:
	typedef std::map< unsigned long, CQFailedRequestInfo > FailedRequestInfoMap; 	// map a referal page's hash id to instance of FailedRequestInfo
	FailedRequestInfoMap*	m_pFailedRequestInfoMap;
public:
	__int64			totalBytes,pad2;
	__int64			totalBytesIn,pad3;
	/*long*/ time_t			firstTime,
					lastTime,
					pad4[8];
					// general common data
	StatList		*byHour,
					*byWeekday,
					*byWeekdays[7],
					*byMonth,
					*byDate,
					*byUser,
					*byGovt,			// unused, deactivated.
					*byClient,			// or dest IP
					*byServers,			// or dest IP
					*bySecondDomain,	// ie 2LD, ie .net.au
					*byDomain,			// ie counties .au
					*byOrgs,			// defined names
					*byRegions,			// world regions

					*byCirculation,
					*byLoyalty,
					*byTimeon,
					// webserver data
					*byFile,
					*byBrowser,
					*byOperSys,
					*byRefer,
					*byReferSite,
					*byDir,
					*byGroups,
					*byTopDir,
					*byType,
					*byErrors,
					*byErrorURL,
					*byPages,			// or source IP
					*byUpload,
					*bySearchStr,
					*bySearchSite,
					*byDownload,
					*byAdvert,
					*byAdCamp,
					*byRobot,

					*byMediaPlayers,
					*byAudio,
					*byVideo,
					*byMediaTypes,
					
					// firewall data
					*byProtocol,
					*byHTTP,
					*byHTTPS,
					*byMail,
					*byFTP,
					*byTelnet,
					*byRealAudio,
					*byDNS,
					*byPOP3,
					*byReal,
					*byOthers,
					*byClusters,
					// If the site URL (i.e. default host) is specified then both byBrokenLinkReferal
					// and byIntBrokenLinkReferal are used to specify external & internal pages
					// containing broken links respectively.  Otherwise, only byBrokenLinkReferal is
					// used to specify all pages containing broken links.  So blow it out yer...nose.
					*byBrokenLinkReferal,		
					*byIntBrokenLinkReferal,
					*byUnrecognizedAgents,

					// Streaming
					*byClips,
					*byVideoCodecs,
					*byAudioCodecs,
					*byCPU,
					*byLang,
					*byBilling,
					*byEnd;

	long			pad5[21];			// 28 empty longs for future stats.

	struct MeanPathRec *MeanPaths;
	long			meanpath_totals;
	long			meanpath_max;

	doneRec			Done;
	void			*next;
};

typedef VDinfo* VDinfoP;

#endif // __VIRTDOMINFO_H

