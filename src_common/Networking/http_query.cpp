//
//  http_query.cpp
//

#include "FWA.h"

#include <string>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include "datetime.h"
#include "version.h"
#include "GlobalPaths.h"

#ifdef DEF_MAC
	#include <string.h>
	#ifndef __QUICKDRAW__
		#include <Quickdraw.h>
	#endif
	#include <MacTCP.h>
//	#include <AddressXlation.h>
	#include <OpenTransport.h>
	#include <OpenTptInternet.h>
	#include <Memory.h>
	#include <Threads.h>
	#include <profiler.h>
//	#include "util.h"
	#include "progress.h"
	#include "macdns.h"
	#include "asyncdnr.h"
	#include "server.h"
	#include "main.h"
//	#include "prefs.h"
#endif								

#include "myansi.h"
#include "config_struct.h"
#include "engine_drill.h"
#include "ReportInterface.h"

#ifdef DEF_APP_404
#include "engine.h"
#define DATE_LAST28 DATE_LAST30
#else
#include "engine_process.h"
#include "EngineMeanPath.h"
#endif
#include "report.h"

#ifdef DEF_MAC
int gSaved = FALSE ;
extern Boolean gDone;

#endif

#if DEF_WINDOWS
	#include <windows.h>
	#include <sys/stat.h>
	#include "Winmain.h"
	#include "resource.h"
extern "C" HWND hwndParent ;
extern "C" HWND ghMainDlg ;
#endif				
#if DEF_UNIX
	#include <sys/stat.h>
	#include "unistd.h"
	#include "main.h"
#endif

#include "http_query.h"
#include "httpinterface.h"
#include "LogFileHistory.h"

#define XML_BUF_SIZE        1024*64
#define MAX_TRAFFIC         10000
#define MAX_ENTRIES         1000
#define MAX_EXITS           1000

#define DATA_FORMAT_HTML	0
#define DATA_FORMAT_XML		1
#define DATA_FORMAT_RAW		2
#define DATA_FORMAT_HASH	3

#undef TRAFFIC_MATRIX

#define INTERFACE_VERSION   "106"    // Current interface version - INCREMENT EVERY SIGNIFICANT CHANGE


#define MAXFILENAMELEN 8191


#ifdef DEF_MAC
extern "C" long	GoProcessLogs( ProcessDataPtr logdata );
extern Boolean gShutdownRequestedViaHTTP ;
#else
extern     long	GoProcessLogs( ProcessDataPtr logdata );
#endif

extern long			VDnum;
extern VDinfoP		VD[MAX_DOMAINSPLUS];			/* 16/32 virtual domain pointers */

int	processing_http_request = 0;

unsigned long HashStr(char *string, long *len);

typedef struct {
	long fromPage;		// the hash of the first page
	long toPage;		// the hash of the second page
	long traffic;		// the number of times this link was traversed (including repeat visits within a session)
	long bytes;			// the number of bytes  
	long errors;		// the number of errors (future)
	long lastClient;	// the last client to traverse this link
	long clients;		// the number of distinct clients
    long timeBetween;   // time between nodes
} Node2NodeTrafficStruct, *PNode2NodeTrafficStruct;

typedef struct {
	long page;	// the hash of the page name
	long hits;	// the number of times this page is the entry page
} NodeEntryStruct, *PNodeEntryStruct;

typedef struct {
	int	count;
	Node2NodeTrafficStruct data[MAX_TRAFFIC];
	int enterCount;
	NodeEntryStruct entries[MAX_ENTRIES];
	int exitCount;
	NodeEntryStruct exits[MAX_ENTRIES];
    
#ifdef TRAFFIC_MATRIX
	int trafficMatrixSize;
	int* trafficMatrix;
#endif
} TrafficStruct, *PTrafficStruct;


typedef struct {
    long pageHash ;
    long timeOnPage ;
} PathElement, *PPathElement ;


#define MAX_PATH_LENGTH 64
typedef struct {
    long count ;
    PathElement paths[MAX_PATH_LENGTH] ;
    int  pathLength ; 
} PathTableEntry, *PPathTableEntry ;


PTrafficStruct traffic[MAX_DOMAINSPLUS];
PPathTableEntry pathTable = NULL;



// Utility function to convert a Posix path (/Users/matt/Desktop) to a HFS path (Moozik:Users:matt:Desktop)
#ifdef DEF_MAC
void strncpyHFSPath(char *HFSPathBuffer, const char *posixPath, int bufferLen)
{
   // MacOSX POSIX path must be converted into HFS format (ie: Volume:path:path2:filename)
   if (posixPath[0] == '/')
   {
     CFURLRef url= CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, 
                   (unsigned char*)posixPath, (long)strlen(posixPath), (unsigned char)0) ;
     CFStringRef sr = CFURLCopyFileSystemPath(url, kCFURLHFSPathStyle) ;
     CFStringGetCString(sr, HFSPathBuffer, bufferLen, 0) ;
   }
   else
   {
     if (posixPath != HFSPathBuffer) strncpy(HFSPathBuffer,posixPath,bufferLen) ;
   }
}
#endif			    


void ClearTrafficList()
{
	for(int i=0; i<MAX_DOMAINSPLUS; i++)
		if(traffic[i])
		{
#ifdef TRAFFIC_MATRIX
			if(traffic[i]->trafficMatrix)
				free(traffic[i]->trafficMatrix);
#endif
			free(traffic[i]);
		}

	memset(&traffic, 0 , sizeof(traffic));
}

// Generate time string from ctime
static void timeToString(long t, char *buffer)
{
  //CTimeToDateTimeStr(t,buffer) ;
  //CTimetoString(t,buffer) ;
  DateTimeToString(t,buffer) ;
}

// Hashing routine...
unsigned long fwHash(char *string, int dummy) 
{
   long len ; 
   return (HashStr(string,&len)) ;
}


// dequote an URL, replacing " with %2B so it can be put in an XML tag
// NB: uses one static buffer so encodeURL(encodeURL(x)) won't work..
#define URL_BUFF_SIZE 16384
static char *encodeURL(char *s) 
{
    static char *url=NULL ;
    int i = 0 ;
    
    if (url==NULL) url=(char *)malloc(URL_BUFF_SIZE);

    while ((*s!='\0') && (i<URL_BUFF_SIZE-3))
    {
        if (*s=='"') 
        {
            url[i++]='%' ; url[i++]='2'; url[i++] = 'B';
        }
        if (*s=='&')
        {
            url[i++]='%' ; url[i++]='2'; url[i++] = '6';
        }
        else
        {
            url[i++] = *s ;
        }
        s++ ;
    }
    url[i] = '\0' ;
    return(url) ;
}



void CreateDomain(int domain, int doTop)
{
	if(!traffic[domain]){
		traffic[domain] = (PTrafficStruct)malloc(sizeof(TrafficStruct));

		memset(traffic[domain], 0, sizeof(TrafficStruct));

		long matrixSize = sizeof(long) * doTop * doTop;

#ifdef TRAFFIC_MATRIX
		traffic[domain]->trafficMatrix = (int*)malloc(matrixSize);
		memset(traffic[domain]->trafficMatrix, 0, matrixSize);
		traffic[domain]->trafficMatrixSize = doTop;
#endif
	}
}




PNode2NodeTrafficStruct FindTrafficNode(int domain, long aFromPage, long aToPage)
{
	for(int i=0; i<traffic[domain]->count; i++)
		if(traffic[domain]->data[i].fromPage == aFromPage && traffic[domain]->data[i].toPage == aToPage)
			return &traffic[domain]->data[i];

	return NULL;
}

PNode2NodeTrafficStruct AddTraffic(int domain, long aClient, long aFromPage, long aToPage, long timeBetween)
{
	PNode2NodeTrafficStruct trafficNode;

	trafficNode = FindTrafficNode(domain, aFromPage, aToPage);

	if(!trafficNode)
    {
		if(traffic[domain]->count<MAX_TRAFFIC)                                                  
		{
			trafficNode = &(traffic[domain]->data[traffic[domain]->count]);

			trafficNode->fromPage    = aFromPage;
			trafficNode->toPage      = aToPage;
            trafficNode->timeBetween = timeBetween ;

			traffic[domain]->count++;
		}
    }

	if(trafficNode)
	{
		if(trafficNode->lastClient != aClient)
		{
			trafficNode->lastClient = aClient;
			trafficNode->clients++;
		}

        trafficNode->timeBetween += timeBetween ;
		trafficNode->traffic++;
	}

	return trafficNode;
}

PNode2NodeTrafficStruct GetTrafficNode(int domain, int n) 
{
	if(n>=0 || n<traffic[domain]->count)
		return &traffic[domain]->data[n];
	else
		return 0;
}

PNodeEntryStruct FindEntryPage(int domain, long page) {
	for(int i=0; i<traffic[domain]->enterCount; i++)
		if(traffic[domain]->entries[i].page == page)
			return &traffic[domain]->entries[i];

	return NULL;
}

PNodeEntryStruct AddEntryPage(int domain, long page) {
	PNodeEntryStruct apage = FindEntryPage(domain, page);

	if(!apage) 
    {
        // if more than maximum entry pages ignore this page
        if (traffic[domain]->enterCount >= MAX_ENTRIES - 1) return(NULL);
		apage = &traffic[domain]->entries[traffic[domain]->enterCount];
		apage->page = page;

		traffic[domain]->enterCount++;
	}

	apage->hits++;

	return apage;
}

PNodeEntryStruct GetEntryPage(int domain, int index) {
	if(index>=0 && index<traffic[domain]->enterCount)
		return &traffic[domain]->entries[index];
	else
		return NULL;
}

PNodeEntryStruct FindExitPage(int domain, long page) {
	for(int i=0; i<traffic[domain]->exitCount; i++)
		if((traffic[domain]!=NULL) && (traffic[domain]->exits[i].page == page))
			return &traffic[domain]->exits[i];

	return NULL;
}

PNodeEntryStruct AddExitPage(int domain, long page) {
	PNodeEntryStruct apage = FindExitPage(domain, page);

	if(!apage) {
        // if more than maximum exit pages ignore this page
        if (traffic[domain]->exitCount >= MAX_EXITS - 1) return(NULL);
		apage = &traffic[domain]->exits[traffic[domain]->exitCount];
		apage->page = page;

		traffic[domain]->exitCount++;
	}

	apage->hits++;

	return apage;
}

PNodeEntryStruct GetExitPage(int domain, int index) {
	if(index>=0 && index<traffic[domain]->exitCount)
		return &traffic[domain]->exits[index];
	else
		return NULL;
}                                       


//
// path utilities
//
static int copyPath(PPathElement destPath, PPathElement srcPath)
{
    int i = 0;
    while ((i<MAX_PATH_LENGTH) && (srcPath[i].pageHash!=0))
    {
        destPath[i].pageHash   = srcPath[i].pageHash ;
        destPath[i].timeOnPage = srcPath[i].timeOnPage;
        i++ ;
    }
    destPath[i].pageHash = 0 ;
         
    return(i) ;
}
static int comparePath(PPathElement a, PPathElement b)
{
    int i=0;

    while((i<MAX_PATH_LENGTH) && (a[i].pageHash!=0) && (a[i].pageHash == b[i].pageHash)) i++ ;

    return (!((i<MAX_PATH_LENGTH) && (a[i].pageHash != b[i].pageHash))) ;
}
static void incrementPathTimes(PPathElement destPath, PPathElement srcPath)
{
    int i = 0;
    while ((i<MAX_PATH_LENGTH) && (srcPath[i].pageHash!=0))
    {
        destPath[i].timeOnPage += srcPath[i].timeOnPage;
        i++ ;
    }
}

static int pathTableSize = 0 ;
static int pathTableEnd  = 0 ;

int pathCompareFunc(const void* a, const void* b)
{
  return ( ((PathTableEntry*)b)->count - ((PathTableEntry*)a)->count);
}
// sort pathTable   
static void sortPathTable()
{
    qsort(pathTable,pathTableEnd,sizeof(PathTableEntry), pathCompareFunc) ;
}
static void clearPathTable()
{
    pathTableEnd = 0 ;
    if (pathTable != NULL)
    {
        free(pathTable) ;
        pathTable = NULL ;
    }
}

// 
// add a path the list of paths, incrementing the path count if required.
//
#define PATH_ALLOC_CLUMP 64
void addPath(PPathElement path) 
{
   int  i ;
   int  found = FALSE ;

   // allocate basic table
   if (pathTable == NULL)
   {
      pathTable = (PPathTableEntry)malloc(sizeof(PathTableEntry) * PATH_ALLOC_CLUMP) ;
      if (pathTable == NULL) return;
      pathTableSize = PATH_ALLOC_CLUMP ;
      memset(pathTable,0,sizeof(PathTableEntry)*PATH_ALLOC_CLUMP) ;
   }

   // search for path

   for (found=FALSE,i=0;(!found) && (i<pathTableSize);i++) 
   {
      if (pathTable[i].pathLength == 0) break ; // not found 
      if (comparePath(path,pathTable[i].paths))
      {
          found = true ;
          pathTable[i].count ++ ;
          incrementPathTimes(pathTable[i].paths, path) ;
      }
   }

   if (!found)
   {
      // reallocate table 
      if (i>=pathTableSize)
      {
          pathTable = (PPathTableEntry)realloc(pathTable,sizeof(PathTableEntry) * (pathTableSize + PATH_ALLOC_CLUMP)) ;
          if (pathTable == NULL) return;
          memset(pathTable+pathTableSize,0,sizeof(PathTableEntry)*PATH_ALLOC_CLUMP) ;
          pathTableSize += PATH_ALLOC_CLUMP ;
      }
      pathTable[i].pathLength = copyPath(pathTable[i].paths,path);
      pathTable[i].count = 1 ;
      pathTableEnd = i ;
   }
}

void BuildTrafficList(int domain, StatList* list, ProcessData* logdata)
{
		register Statistic	*p;
		long				n,total;
		SessionStat*		sessStat;
		SessionRecPtr		srp;
		int					percent;
        int                 lastPercent = -1 ;
		int					count;

		char				buffer[1024];
		double				time1;

#ifdef TRAFFIC_MATRIX
		SessionRecPtr		srp2,sessionBase;
		StatList*			byHits = VD[domain]->byPages;
		int					p1,p2,temp;
		int					matrixSize = traffic[domain]->trafficMatrixSize;

		sessionBase = NULL;
#endif

		char*				name = VD[domain]->domainName;

		sprintf(buffer, "Processing domain %s", name);
		
		time1 = timems();
		total = list->num;
		count = 0;

		for(n=0; n<total; n++) {

			percent = ((100*n)/total);
            if (percent != lastPercent)
            {
			  logdata->ShowProgressDetail(percent*10, TRUE, buffer, time1);
            }

            lastPercent = percent ;

			p = list->GetStat(n);

			sessStat = p->sessionStat;

			if(sessStat)
			{
				srp = sessStat->Session;


				if(srp) {
					long lastPageID = 1;

					for(int i=0; i<sessStat->SessionNum; i++) {

						if( srp->pageID < 1 || srp->pageID > 10)
                        {
							if( lastPageID < 1 || lastPageID > 10)
							{
                                // ignore self references if preferences dictate
                                if (!(MyPrefStruct.ignore_selfreferral && (lastPageID == srp->pageID))) 
                                {
								   AddTraffic(domain, p->hash, lastPageID, srp->pageID, 0);
                                }

#ifdef TRAFFIC_MATRIX
								srp2 = sessionBase;
								for(int j=0; srp!=srp2; j++) {
								
									if(j==100) {
										sessionBase = srp;
										break;
									}

									p1 = byHits->FindHash(srp->pageID);
									p2 = byHits->FindHash(srp2->pageID);

									if(p1!=-1 && p2!=-1 && p1<matrixSize && p2<matrixSize) {
										if(p1>p2)
											temp = p2 + p1*matrixSize;
										else
											temp = p1 + p2*matrixSize;
										traffic[domain]->trafficMatrix[temp]++;
									}

									count++;
									srp2++;
								}

#else
								count++;
#endif
								if(count>100000) {
									count = 0;
									logdata->ShowProgressDetail(percent*10, TRUE, buffer, time1);
								}
							} else {
#ifdef TRAFFIC_MATRIX
								sessionBase = srp;
#endif
								AddEntryPage(domain, srp->pageID);  // Start of a click stream
							}
                        }
						else
                        {
							if( lastPageID < 1 || lastPageID > 10) {
#ifdef TRAFFIC_MATRIX
								sessionBase = NULL;
#endif

								AddExitPage(domain, lastPageID);	// End of a click stream
							}
                        }
						lastPageID = srp->pageID;

						srp++;
					}
				}
			}

		}
}




char* GetUrl(VDinfoP vd, long hash)
{
	long item;
	char* url = NULL;

	if(vd) {
		item = vd->byPages->FindHash( hash );
		if ( item >= 0 ) {
			url = vd->byPages->GetName(item);
		} else {
			item = vd->byDownload->FindHash( hash );
			if ( item >= 0 )
				url = vd->byDownload->GetName(item);
		}
	}

	return url;
}

char* NextWord(char** pos, char sep)
{
	char* result = *pos;
	char* temp = result;

	while(*temp && *temp!='&' && *temp!=sep)
	{
		temp++;
	} 

	if(*temp)
	{
		*temp = '\0';
		temp++;
	}

	*pos = temp;

	return result;
}

void DumpAllStats(int domain, char* buffer, long socket, char* page, char format, long minhits)
{
	StatList*	list;
	register	Statistic	*p;
	long		total;
	long		visits;
	long		hits;
	long		errors;
    long        sessions ;
    long        totalbytes ;
    long        avgtime ;
    

    VDinfoP	VDptr = VD[domain] ;

	switch(format)
	{
	case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Page Stats</h2>\n");
							SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Hash</td><td>Page</td><td>Visitors</td><td>Hits</td><td>Errors</td></tr>\n");
							break;

	case DATA_FORMAT_XML:	SendStrHTTP(socket, "<stats>\n");
							break;

	case DATA_FORMAT_RAW:
	case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#stats\n");
							break;
	}


	if(VDptr)
		if(*page!='*')
		{
			list = VDptr->byPages;

			p = list->FindStatbyName(page);
	
			if(p) {

				page       = p->GetName();
				sessions   = p->visits;
                visits     = p->counter ;
				hits       = p->files;
				errors     = p->errors;
                avgtime    = p->visitTot/p->visits ;
                totalbytes = p->bytes ;

				if(hits>=minhits)
				switch(format)
				{
				case DATA_FORMAT_HTML:
										sprintf(buffer, "<tr><td>%lX</td><td>%s</td><td>%ld</td><td>%ld</td><td>%ld</td></tr>\n", p->hash, page, visits, hits, errors);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:
										sprintf(buffer, "<file hash=\"%lx\" name=\"%s\" visits=\"%ld\" hits=\"%ld\" errors=\"%ld\"/>\n", p->hash, encodeURL(page), visits, hits, errors);
										SendStrHTTP(socket, buffer);
										break;
	
				case DATA_FORMAT_RAW:
				case DATA_FORMAT_HASH:
                                        // modified to give same stats as analyzer
										sprintf(buffer, "%lX %s %ld %ld %ld %ld %ld %ld %ld %ld\n", 
                                                        p->hash, 
                                                        page, 
                                                        hits,
                                                        totalbytes,
                                                        sessions, 
                                                        avgtime, 
                                                        visits,
                                                        errors,
                                                        p->GetVisitIn(),
                                                        p->counter4
                                                        );
										SendStrHTTP(socket, buffer);
										break;				
				}
			}
		} else 
        {

			list = VDptr->byPages;

			total = list->num;
			for(int j=0; j<total; j++) 
            {
				p = list->GetStat(j);

				page       = p->GetName();
				sessions   = p->visits;
                visits     = p->counter ;
				hits       = p->files;
				errors     = p->errors;
                avgtime    = p->visitTot/p->visits ;
                totalbytes = p->bytes ;

				if(visits>=minhits)
				switch(format)                 
				{
				case DATA_FORMAT_HTML:
										sprintf(buffer, "<tr><td>%lX</td><td>%s</td><td>%ld</td><td>%ld</td><td>%ld</td></tr>\n", p->hash, page, visits, hits, errors);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:
										sprintf(buffer, "<file hash=\"%lx\" name=\"%s\" visits=\"%ld\" hits=\"%ld\" errors=\"%ld\"/>\n", p->hash, encodeURL(page), visits, hits, errors);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_RAW:
				case DATA_FORMAT_HASH:
                                        // modified to give same stats as analyzer
										sprintf(buffer, "%lX %s %ld %ld %ld %ld %ld %ld %ld %ld\n", 
                                                        p->hash, 
                                                        page, 
                                                        hits,
                                                        totalbytes,
                                                        sessions, 
                                                        avgtime, 
                                                        visits,
                                                        errors,
                                                        p->GetVisitIn(),
                                                        p->counter4);
										SendStrHTTP(socket, buffer);
										break;				
				}
			}
		}

	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</stats>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}
}

//
void DumpPaths(int domain, StatList* list, char *buffer, long socket, char format)
{
    long  pathCount = MP_GetMeanPathTotal( VD[0] ) ;
    long  n ;
    long  hash, hits, pageIndex;
    short count ;

    if (format != DATA_FORMAT_RAW)
      SendStrHTTP(socket,"<paths>") ;

    for (n = 0 ; n < pathCount; n++)
    {
        if (MP_GetMeanPath( VD[0], n, &hash, &hits, &pageIndex, &count ))
        {

           if (format == DATA_FORMAT_RAW)
             sprintf(buffer,"%lx %d %d %d\n", hash, hits, pageIndex,count) ;
           else
             sprintf(buffer,"<path hash=\"%lx\" hits=\"%d\" pageindex=\"%d\" count=\"%d\" />", hash, hits, pageIndex,count) ;

           SendStrHTTP(socket,buffer) ;
        }
    }

    if (format != DATA_FORMAT_RAW)
      SendStrHTTP(socket,"</paths>") ;
}


void clearAllHits(int domain)
{
	StatList* list = VD[domain]->byPages;
	Statistic *p;

	for(int j=0; j<list->num; j++) 
    {
		p = list->GetStat(j);
        if (p!=NULL) p->files = 0 ;
    }
}

void DumpUserSessionsSummary(int domain, StatList* list, char *buffer  , long socket, char format, int topn, int maxPathLength, int minPathLength, long fromTime, long toTime)
{
     int i,j ;
     char *url ;
     long bytes; 
     char d1[128], d2[128];
     VDinfoP	VDptr = VD[domain] ;

     if (fromTime != 0)
       timeToString(fromTime,d1); 
     else
       timeToString(VDptr->firstTime,d1); 
     if (toTime != 0)
       timeToString(toTime,d2); 
     else
       timeToString(VDptr->lastTime,d2); 

     if (format == DATA_FORMAT_RAW)
     {
        SendStrHTTP(socket,"#sessions\n") ;
     }
     else
     {
        sprintf(buffer,"<sessions start=\"%s\" end=\"%s\">",d1,d2) ;
        SendStrHTTP(socket,buffer) ;
     }

     if (topn==0) topn=pathTableEnd ;

     for (i=0;(i<pathTableEnd)&&(i<topn)&&(pathTable[i].pathLength !=0); i++ )
     { 
         if (format != DATA_FORMAT_RAW)
         {
           sprintf(buffer,"<session id=\"%d\" length=\"%d\"  hits=\"%d\" >",i,pathTable[i].pathLength,pathTable[i].count) ;
           SendStrHTTP(socket,buffer) ;
         }

         for (j=0;(j<MAX_PATH_LENGTH)&&(pathTable[i].paths[j].pageHash!=0) ; j++ )
         {
         	url = VDptr->byPages->GetStatDetails( pathTable[i].paths[j].pageHash, 0, &bytes );
		    if ( !url )	url = VDptr->byDownload->GetStatDetails( pathTable[i].paths[j].pageHash, 0, &bytes );
            if (format == DATA_FORMAT_RAW)
               sprintf(buffer,"%d %d %lx\n",i,pathTable[i].paths[j].timeOnPage/pathTable[i].count,pathTable[i].paths[j].pageHash) ;
            else
               sprintf(buffer,"<file name=\"%s\" interarrivaltime=\"%d\" />",encodeURL(url),pathTable[i].paths[j].timeOnPage/pathTable[i].count) ;
            SendStrHTTP(socket,buffer) ;
         }
         if (format != DATA_FORMAT_RAW)
           SendStrHTTP(socket,"</session>") ;
     }

     if (format != DATA_FORMAT_RAW)
     {
        SendStrHTTP(socket,"</sessions>") ;
     }

}


// go through the path table and add up the total time on a page for a given page hash
long getTotalTimeOnPage(int domain, int topn, long page)
{
     int i,j ;
     long totalTime=0;

     if (topn==0) topn=pathTableEnd ;

     for (i=0;(i<pathTableEnd)&&(i<topn)&&(pathTable[i].pathLength !=0); i++ )
     { 
         for (j=0;(j<MAX_PATH_LENGTH)&&(pathTable[i].paths[j].pageHash!=0) ; j++ )
         {
             if (pathTable[i].paths[j].pageHash == page)
             {
                 totalTime += pathTable[i].paths[j].timeOnPage;
             }
         }
     }

     return(totalTime) ;
}


// Dump user sessions (all of them)
void DumpUserSessions(int domain, StatList* list, char *buffer  , long socket, char format, 
                      int allsessions, int topn, int maxPathLength, int minPathLength, 
                      long fromTime, long toTime)
{
  long		depth, sitem, n;
  VDinfoP	VDptr = VD[domain] ;
  register Statistic	*p,*p2;
  long maxpages;
  char *browser, *opersys, timeStr[256];
  long lastArrivalTime = 0 ;
  int  sessionNum = 0 ;
  int  lastInSession = TRUE ;

  PathElement tmpPath[MAX_PATH_LENGTH];
  int tmpPathLength = 0 ;

  clearPathTable() ;
  clearAllHits(domain) ;

  if (allsessions && (format == DATA_FORMAT_RAW))
  {
      SendStrHTTP(socket,"#allsessions\n") ;
  }

  for (n=0;n < list->num;n++)
  {
	p = list->GetStat(n);
	
	if ( !p->sessionStat->Session )
		return ;

	maxpages = p->sessionStat->GetNum();
	if ( VDptr->byBrowser )	browser = VDptr->byBrowser->GetName( p->counter2 ); else browser = NULL;
	if ( VDptr->byOperSys )	opersys = VDptr->byOperSys->GetName( p->counter3 ); else opersys = NULL;
//	WritePageTitle( VDptr, hout, title );
	CTimetoString( p->visitTot / p->visits, timeStr );
//	Write_SessHeader( hout , client, timeStr, browser, opersys );

	depth = 1;
	sitem = 1;
	
	if ( maxpages )
    {
		char	dateStr[256], *url;
		long	firstDate=0, dur, time, ptime=0;
		long	hashid, phashid=1, page;
		long	rgbcolor[]={ -1, 0xe0e0e0, 0, 0 };
		long	days=0, totalPages = 0, totalDownloads = 0, sessionDownloads=0;
		long	totaldownloadbytes = 0, sessionBytes = 0;
        int     sessionStarted = FALSE ;
        int     hitItem = -1 ;

		// limit the session output so its not 50meg html file.
		if (maxpages > MyPrefStruct.sessionLimit && MyPrefStruct.sessionLimit )
			maxpages = MyPrefStruct.sessionLimit;

		dateStr[0] = 0;
		days = 0;
		for ( page=0; page < maxpages; page++ )
        {
			hashid = p->sessionStat->GetSessionPage( page );
			time = p->sessionStat->GetSessionTime( page );

            lastInSession = FALSE ;

            if ((fromTime != 0) && (time < fromTime)) continue ;
            if ((toTime != 0)   && (time > toTime))
            {
                if (sessionStarted) 
                    lastInSession = TRUE ;
                else
                    continue ;
            }

            // add the hit to this page
    		hitItem = VDptr->byPages->FindHash( hashid );
		    if ( hitItem >= 0 ) 
            {
		    	p2 = VDptr->byPages->GetStat(hitItem) ;
                if (p2!=NULL) p2->files++ ;
		    }

			if ( hashid == 1 ){				// START OF SESSION (1,time)
				if ( ((time/ONEDAY) - (firstDate/ONEDAY)) >= 1 )
                {
					//SendStrHTTP(socket, "<day />\n") ;
					days++;
				}

                sessionNum ++ ;  // increment number of started sessions 

				timeToString( time, dateStr );
                if (allsessions)
                {
                  if (format == DATA_FORMAT_RAW)
				      sprintf(buffer, "session %d %d %s\n", sessionNum, time, dateStr );
                  else
				      sprintf(buffer, "<session id=\"%d\" hits=\"%d\" time=\"%s\" rawtime=\"%d\">\n", sessionNum, 1, dateStr, time );
                  SendStrHTTP(socket,buffer) ;
                }
				firstDate = time;
                lastArrivalTime = time ;
                tmpPathLength = 0 ;
                sessionStarted = TRUE ;
			}

            if (!sessionStarted) continue ;

			if ( hashid == 2 ){				// END OF SESSION (2,bytes)
				if ( !ptime )
					dur = 30;
				else
					dur = (ptime-firstDate) + 30;
				if ( phashid == 1 && dur<0 )
                {
                  if (allsessions)
                  {
                    if (format != DATA_FORMAT_RAW)
    					SendStrHTTP(socket,"   <nopageview />") ;
                  }
                }
				else
				{
                    if (allsessions) 
                    {
                      if (format != DATA_FORMAT_RAW)
                      {
					    if ( dur > 0 )
                        {
						    sprintf(buffer,"<duration seconds=\"%d\" />", dur );
                            SendStrHTTP(socket,buffer) ;
					    } 
                        else 
                        {
						    SendStrHTTP(socket,"<duration seconds=\"0\" />");
                        }
                      }
					}
				}
			} 
			if ( hashid == 4 ){				// REFERAL AT SESSION (4,referal)
				if ( VDptr->byRefer ) {
					long	bytes = 0;
					url = VDptr->byRefer->GetStatDetails( time, 0, &bytes );
					if ( url && allsessions)
                    {
                      if (format != DATA_FORMAT_RAW)
                      {
 						sprintf(buffer,"<referral src=\"%s\"/>", encodeURL(url));
                        SendStrHTTP(socket,buffer) ;
                      }
                    }
				}
			}
			if ( hashid<1 || hashid>10 ){	// display page used within session (hash,time)
				long	bytes = 0;

				url = VDptr->byPages->GetStatDetails( hashid, 0, &bytes );
				if ( !url )
					url = VDptr->byDownload->GetStatDetails( hashid, 0, &bytes );

				if ( url ) {
					char *urlext;
					int dopage = 0;
					timeToString( time, dateStr );

                    
					if ( urlext = strrchr( url, '.' ) ){
						if ( !strcmpExtensions( urlext, MyPrefStruct.downloadStr ) ) {
							totalDownloads++;
							totaldownloadbytes += bytes;
							sessionDownloads++;
							sessionBytes += bytes;
							dopage = -1;
						}
					}

                    if ((hashid != phashid) && (tmpPathLength < MAX_PATH_LENGTH))
                    {
                      tmpPath[tmpPathLength].pageHash   = hashid ;
                      tmpPath[tmpPathLength].timeOnPage = time-lastArrivalTime ;
                      tmpPathLength++ ;
                    }
					
                    if (allsessions)
                    {
                      if (format == DATA_FORMAT_RAW)
                        sprintf(buffer,"%d %lx\n",  time - lastArrivalTime, hashid) ;
                      else
                        sprintf(buffer,"<file name=\"%s\" interarrivaltime=\"%d\"/>\n", encodeURL(url), time - lastArrivalTime) ;
                      SendStrHTTP(socket, buffer) ;
                    }

                    lastArrivalTime = time ;

					depth++;
					totalPages++;
				}
			}
			if (( hashid == 3 ) || (lastInSession)) {				// END OF SESSION (3,time)
                if (allsessions)
                {
                    if (format != DATA_FORMAT_RAW)
                    {
				        sprintf(buffer,"<bandwidth bytes=\"%d\" />",(unsigned long)ptime );
                        SendStrHTTP(socket,buffer) ;

				        if ( sessionDownloads )
                        {
					        sprintf( buffer, "<downloads bytes=\"%d\" />", sessionBytes );
                            SendStrHTTP(socket,buffer) ;
				        }
				        SendStrHTTP(socket,"</session>");
                    }
                }
		        sessionDownloads = sessionBytes = 0;
                if (!allsessions && (tmpPathLength > 0))
                {
                    // check to see if path should be added to path list
                    if (!((minPathLength>0)&&(tmpPathLength<minPathLength)))
                    {
                        if ((maxPathLength>0)&&(tmpPathLength>maxPathLength)) tmpPathLength = maxPathLength ;
                        if (tmpPathLength < MAX_PATH_LENGTH) tmpPath[tmpPathLength].pageHash = 0 ; // 'null' terminate the path
                        addPath(tmpPath) ;
                    }
                }

				depth = 1;
				sitem++;
                sessionStarted = FALSE ;
			}

			ptime = time;
			phashid = hashid;
		} //for loop

		// plot last sessions summary if its incompleted
		if (sessionStarted && (hashid!=3) ){
			dur = (time-firstDate) + 30;

            if (allsessions)
            {
                if (format != DATA_FORMAT_RAW)
                {
    		      sprintf(buffer,"<duration seconds=\"%d\" />",dur );
                  SendStrHTTP(socket,buffer) ;
                }
            }

			CTimetoString( dur, timeStr );

			if ( sessionDownloads && allsessions)
            {
                if (format != DATA_FORMAT_RAW)
                {
				    sprintf( buffer, "<downloads bytes=%d />", sessionBytes);
                    SendStrHTTP(socket,buffer) ;
                }
			}
			sessionDownloads = sessionBytes = 0;

            if (allsessions)
            {
                if (format != DATA_FORMAT_RAW)
                {
  			        SendStrHTTP(socket,"</session>");
                }
            }

		}

        /*
		// show all the sessions summary for the client
		if ( days ){
			Write_SessTotalBand( hout , (unsigned long)byStat->GetBytes(unit), (unsigned long)byStat->GetBytesIn(unit), days );
			Write_SessTotalTime( hout , p->visitTot, days );
		}
		if ( totalPages ) {
			sprintf( tempSpec, "%0.2f", totalPages / (float)p->visits );
			Stat_WriteBoldDual( hout, "Average pages per session", tempSpec );
		}
		if ( totalDownloads ) {
			ValuetoString( TYPE_BYTES, totaldownloadbytes, totaldownloadbytes, timeStr );
			sprintf( tempSpec, "%d (%s)", totalDownloads, timeStr );

			Stat_WriteBoldDual( hout, "Total Attempted Downloads", tempSpec );
		}
        */
	}
  }


  if (!allsessions)
  {
    sortPathTable() ;
    DumpUserSessionsSummary(domain, list, buffer  , socket, format, topn, maxPathLength, minPathLength,fromTime,toTime);
  }  

  if (format == DATA_FORMAT_RAW)
  {
     SendStrHTTP(socket,"#end\n") ;
  }


}




void DumpHits(int domain, char* buffer, long socket, char* page, char format, long minhits, long fromTime, long toTime)
{
	StatList*	list;
	register	Statistic	*p;
	long		total;
	long		visits;
    char        d1[128], d2[128];
    VDinfoP	VDptr = VD[domain] ;


    if (fromTime != 0)
      timeToString(fromTime,d1); 
    else
      timeToString(VDptr->firstTime,d1); 
    if (toTime != 0)
      timeToString(toTime,d2); 
    else
      timeToString(VDptr->lastTime,d2); 


	switch(format)
	{
	case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Hits</h2>\n");
							SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Page</td><td>Hits</td></tr>\n");
							break;

	case DATA_FORMAT_XML:	sprintf(buffer,"<hits start=\"%s\" end=\"%s\">",d1,d2) ;
                            SendStrHTTP(socket,buffer) ;
							break;

	case DATA_FORMAT_RAW:
	case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#hits\n");
							break;
	}

	if(VD[domain])
		if(*page!='*')
		{
			list = VD[domain]->byPages;

			p = list->FindStatbyName(page);

			if(p) {


				visits = p->files;
	
				if ((visits>=minhits) && (visits!=0))
				switch(format)
				{
				case DATA_FORMAT_HTML:
										sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", page, visits);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:
										sprintf(buffer, "<file name=\"%s\" interarrivaltime=\"%ld\" hits=\"%ld\" />\n", 
                                                        encodeURL(page), getTotalTimeOnPage(domain, 0, p->hash)/visits, visits);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_RAW: 	sprintf(buffer, "%s %ld\n", page, visits);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_HASH:
										sprintf(buffer, "%lX %ld\n", p->hash, visits);
										SendStrHTTP(socket, buffer);
										break;
				
				}
			}
		} else {
			list = VD[domain]->byPages;
	
			total = list->num;
			for(int j=0; j<total; j++) {
				p = list->GetStat(j);


                
//   			    time = p->sessionStat->GetSessionTime(p->sessionStat->pageID );
//
//                if ((fromTime != 0) && (time < fromTime)) continue ;
//                if ((toTime != 0)   && (time > toTime))   continue ;
                
				visits = p->files;

				if ((visits>=minhits) && (visits!=0))
				switch(format)
				{
				case DATA_FORMAT_HTML:
										sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", p->GetName(), visits);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:
										sprintf(buffer, "<file name=\"%s\" interarrivaltime=\"%ld\" hits=\"%ld\"/>\n", 
                                                encodeURL(p->GetName()),getTotalTimeOnPage(domain, 0, p->GetHash())/visits,visits);
										SendStrHTTP(socket, buffer);
										break;
										  
				case DATA_FORMAT_RAW: 	
										sprintf(buffer, "%s %ld\n", p->GetName(), visits);
										SendStrHTTP(socket, buffer);
										break;
											
				case DATA_FORMAT_HASH: 
										sprintf(buffer, "%lX %ld\n", p->hash, visits);
										SendStrHTTP(socket, buffer);
										break;
					
				}
			}
		}

	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</hits>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}
}


#define MOST_ACTIVE_ALL    1
#define MOST_ACTIVE_MIN    2
#define MOST_ACTIVE_HOUR   4
#define MOST_ACTIVE_DAY    8

// dump out the most active day and most active hour and most active minute
void DumpMostActive(int domain, StatList* ilist, char *buffer  , long socket, char format,
                    int allsessions, int topn, int maxPathLength, int minPathLength, int minHits, int dumpWhat, 
                    int dumpDetail, long fromTime, long toTime)
{
	StatList*	list;
    char        d1[128], d2[128];
    VDinfoP	VDptr = VD[domain] ;
    int         sessionNum = 0 ;
    int         dayNum ;

    long        startTime        = 0 ;

    long        mostActiveDay    = 0 ;
    long        mostActiveHour   = 0 ;
    long        mostActiveMinute = 0 ;

    long        mostActiveDayHits    = 0 ;
    long        mostActiveHourHits   = 0 ;
    long        mostActiveMinuteHits = 0 ;

    long        mostActiveDaySessions= 0 ;
    long        mostActiveHourSessions= 0 ;
    long        mostActiveMinuteSessions = 0 ;

    long        currentDay       = 0 ;
    long        currentHour      = 0 ;
    long        currentMinute    = 0 ;

    long        currentDayHits   = 0 ;
    long        currentHourHits  = 0 ;
    long        currentMinuteHits= 0 ;

    long        currentDaySessions  = 0 ;
    long        currentHourSessions  = 0 ;
    long        currentMinuteSessions= 0 ;

    long        thisTime         = 0 ;

    long        dayHits          = 0 ;
    long        hourHits         = 0 ;
    
    

    if (fromTime!=0)
      timeToString(fromTime,d1); 
    else
      timeToString(VDptr->firstTime,d1); 
    if (toTime!=0)
      timeToString(toTime,d2); 
    else
      timeToString(VDptr->lastTime,d2); 

    currentDay = currentHour = currentMinute = 0;
    startTime = VDptr->firstTime ;

	switch(format)
	{
	case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Most Active</h2>\n");
							SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Page</td><td>MostActive</td></tr>\n");
							break;
	case DATA_FORMAT_XML:	sprintf(buffer,"<mostactive start=\"%s\" end=\"%s\">",d1,d2) ;
                            SendStrHTTP(socket,buffer) ;
							break;
	case DATA_FORMAT_RAW:
	case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#mostactive\n");
							break;
	}


    list = VDptr->byHour ;
    int hourNum ;
    long currTime ;

    // flip through each day , then through each hour
    for ( dayNum=0; dayNum < VDptr->totalDays; dayNum++ )
    {
       dayHits = 0 ;
       for (hourNum=0;hourNum < list->num;hourNum++)
       {
        
           currTime = VDptr->firstTime+ (dayNum*ONEDAY) + (hourNum*ONEHOUR);

           if ((fromTime != 0) && (currTime < fromTime)) continue ;
           if ((toTime != 0)   && (currTime > toTime))   continue ;

           hourHits =  list->GetFilesHistory(hourNum, (long)(VDptr->firstTime/ONEDAY)+dayNum) ;
           dayHits += hourHits ;

           // add stats to the current Hour 
           if (hourHits > mostActiveHourHits)
           {
              mostActiveHourHits      = hourHits ;
              mostActiveHour          = currTime ;
           }
        }

         // check the previous day's total hits
        if (dayHits > mostActiveDayHits)
        {
           mostActiveDayHits      = dayHits ;
           mostActiveDay          = VDptr->firstTime+ (dayNum*ONEDAY);
        }

	}


	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
                            if ((dumpWhat == MOST_ACTIVE_ALL)||(dumpWhat == MOST_ACTIVE_HOUR ))
                            {
                                timeToString(mostActiveHour,d1); 
                                sprintf(buffer,"<mostactivehour time=\"%s\" rawtime=\"%d\" hits=\"%d\" sessions=\"%d\">\n",
                                        d1,mostActiveHour,mostActiveHourHits,mostActiveHourSessions) ;
                                SendStrHTTP(socket,buffer) ;
                                if (dumpDetail)
                                {
                                    DumpUserSessions(domain, ilist, buffer, socket, format, FALSE,
                                                     topn, maxPathLength, minPathLength, 
                                                     mostActiveHour,mostActiveHour+3600);
                                    DumpHits(domain, buffer, socket, "*", format, minHits, mostActiveHour, mostActiveHour+3600) ;
                                }
                                SendStrHTTP(socket,"</mostactivehour>") ;
                            }
                            if ((dumpWhat == MOST_ACTIVE_ALL)||(dumpWhat == MOST_ACTIVE_DAY ))
                            {
                                timeToString(mostActiveDay,d1); 
                                sprintf(buffer,"<mostactiveday time=\"%s\" rawtime=\"%d\" hits=\"%d\" sessions=\"%d\">\n",
                                        d1,mostActiveDay,mostActiveDayHits,mostActiveDaySessions) ;
                                SendStrHTTP(socket,buffer) ;
                                if (dumpDetail)
                                {
                                    DumpUserSessions(domain, ilist, buffer, socket, format, FALSE,
                                                     topn, maxPathLength, minPathLength, 
                                                     mostActiveDay,mostActiveDay+86400);
                                    DumpHits(domain, buffer, socket, "*", format, minHits, mostActiveDayHits, mostActiveDay+86400) ;

                                }
                                SendStrHTTP(socket,"</mostactiveday>") ;
                            }
							SendStrHTTP(socket, "</mostactive>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}
}


void DumpErrors(int domain, char* buffer, long socket, char* page, char format, long minhits)
{
	StatList*	list;
	register	Statistic	*p;
	long		total;
	long		visits;

	switch(format)
	{
	case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Errors</h2>\n");
							SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Page</td><td>Errors</td></tr>\n");
							break;
	case DATA_FORMAT_XML:	SendStrHTTP(socket, "<Errors>\n");
							break;
	case DATA_FORMAT_RAW:
	case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#Errors\n");
							break;
	}

	if(VD[domain])
		if(*page!='*')
		{
			list = VD[domain]->byPages;

			p = list->FindStatbyName(page);

			if(p) {
				visits = p->errors;
	
				if(visits>=minhits)
				switch(format)
				{
				case DATA_FORMAT_HTML:
										sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", page, visits);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:
										sprintf(buffer, "<file name=\"%s\" errors=\"%ld\"/>\n", encodeURL(page), visits);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_RAW: 	sprintf(buffer, "%s %ld\n", page, visits);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_HASH:
										sprintf(buffer, "%lX %ld\n", p->hash, visits);
										SendStrHTTP(socket, buffer);
										break;
				
				}
			}
		} else {
			list = VD[domain]->byPages;

			total = list->num;
			for(int j=0; j<total; j++) {
				p = list->GetStat(j);

				visits = p->errors;
	
				if(visits>=minhits)
				switch(format)
				{
				case DATA_FORMAT_HTML:
										sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", p->GetName(), visits);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:
										sprintf(buffer, "<file name=\"%s\" errors=\"%ld\"/>\n", encodeURL(p->GetName()), visits);
										SendStrHTTP(socket, buffer);
										break;
									  
				case DATA_FORMAT_RAW: 	
										sprintf(buffer, "%s %ld\n", p->GetName(), visits);
										SendStrHTTP(socket, buffer);
										break;
										
				case DATA_FORMAT_HASH: 
										sprintf(buffer, "%lX %ld\n", p->hash, visits);
										SendStrHTTP(socket, buffer);
										break;
				
				}
			}
		}

	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</errors>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}
}


void DumpEntries(int domain, char* buffer, long socket, char* page, char format, long minhits) {

	PNodeEntryStruct node;
	long visits;

	switch(format)
	{
	case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Entry Points</h2>\n");
							SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Page</td><td>Entries</td></tr>\n");
							break;

	case DATA_FORMAT_XML:	SendStrHTTP(socket, "<entries>\n");
							break;

	case DATA_FORMAT_RAW:
	case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#entries\n");
							break;
	}

	if(*page!='*')
	{
		long hash = fwHash(page, -1);

		node = FindEntryPage(domain, hash);

		if(node) {
			visits = node->hits;

			if(visits>=minhits)
			switch(format)
			{
			case DATA_FORMAT_HTML:
									sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", page, visits);
									SendStrHTTP(socket, buffer);
									break;

			case DATA_FORMAT_XML:
									sprintf(buffer, "<file name=\"%s\" entries=\"%ld\"/>\n", encodeURL(page), visits);
									SendStrHTTP(socket, buffer);
									break;

			case DATA_FORMAT_RAW: 	sprintf(buffer, "%s %ld\n", page, visits);
									SendStrHTTP(socket, buffer);
									break;

			case DATA_FORMAT_HASH:
									sprintf(buffer, "%lX %ld\n", node->page, visits);
									SendStrHTTP(socket, buffer);
									break;
				
			}
		}
	} else {
		for(int i=0; i<traffic[domain]->enterCount; i++) {
			
			node = GetEntryPage(domain, i);
	
			visits = node->hits;

			if(visits>=minhits)
			switch(format)
			{
			case DATA_FORMAT_HTML:
									sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", GetUrl(VD[domain], node->page), visits);
									SendStrHTTP(socket, buffer);
									break;

			case DATA_FORMAT_XML:
									sprintf(buffer, "<file name=\"%s\" entries=\"%ld\"/>\n", encodeURL(GetUrl(VD[domain], node->page)), visits);
									SendStrHTTP(socket, buffer);
									break;
									  
			case DATA_FORMAT_RAW: 	
									sprintf(buffer, "%s %ld\n", GetUrl(VD[domain], node->page), visits);
									SendStrHTTP(socket, buffer);
									break;
										
			case DATA_FORMAT_HASH: 
									sprintf(buffer, "%lX %ld\n", node->page, visits);
									SendStrHTTP(socket, buffer);
									break;
				
			}
		}
	}

	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</entries>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}
}

void DumpExits(int domain, char* buffer, long socket, char* page, char format, long minhits) {

	PNodeEntryStruct node;
	long visits;

	switch(format)
	{
	case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Exit Points</h2>\n");
							SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Page</td><td>Exits</td></tr>\n");
							break;

	case DATA_FORMAT_XML:	SendStrHTTP(socket, "<exits>\n");
							break;

	case DATA_FORMAT_RAW:
	case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#exits\n");
							break;
	}

	if(*page!='*')
	{
		long hash = fwHash(page, -1);

		node = FindExitPage(domain, hash);

		if(node) {
			visits = node->hits;

			if(visits>=minhits)
			switch(format)
			{
			case DATA_FORMAT_HTML:
									sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", page, visits);
									SendStrHTTP(socket, buffer);
									break;

			case DATA_FORMAT_XML:
									sprintf(buffer, "<file name=\"%s\" exits=\"%ld\"/>\n", encodeURL(page), visits);
									SendStrHTTP(socket, buffer);
									break;

			case DATA_FORMAT_RAW: 	sprintf(buffer, "%s %ld\n", page, visits);
									SendStrHTTP(socket, buffer);
									break;

			case DATA_FORMAT_HASH:
									sprintf(buffer, "%lX %ld\n", node->page, visits);
									SendStrHTTP(socket, buffer);
									break;
				
			}
		}
	} else {
		for(int i=0; i<traffic[domain]->exitCount; i++) {
			
			node = GetExitPage(domain, i);
	
			visits = node->hits;

			if(visits>=minhits)
			switch(format)
			{
			case DATA_FORMAT_HTML:
									sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", GetUrl(VD[domain], node->page), visits);
									SendStrHTTP(socket, buffer);
									break;

			case DATA_FORMAT_XML:
									sprintf(buffer, "<file name=\"%s\" exits=\"%ld\"/>\n", encodeURL(GetUrl(VD[domain], node->page)), visits);
									SendStrHTTP(socket, buffer);
									break;
									  
			case DATA_FORMAT_RAW: 	
									sprintf(buffer, "%s %ld\n", GetUrl(VD[domain], node->page), visits);
									SendStrHTTP(socket, buffer);
									break;
										
			case DATA_FORMAT_HASH: 
									sprintf(buffer, "%lX %ld\n", node->page, visits);
									SendStrHTTP(socket, buffer);
									break;
				
			}
		}
	}

	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</exits>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}
}

void DumpTraversals(int domain, char* buffer, long socket, char* srcPage, char* destPage, char format, long minhits)
{
	PNode2NodeTrafficStruct node;

	long	fromHash;
	long	toHash;

	if(*srcPage!='*')
		fromHash = fwHash(srcPage, -1);
	else
		fromHash = 0;

	if(*destPage!='*')
		toHash = fwHash(destPage, -1);
	else
		toHash = 0;
	
	switch(format)
	{
	case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Traversals</h2>\n");
							SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Source Page</td><td>Destination Page</td><td>Traversals</td></tr>\n");
							break;

	case DATA_FORMAT_XML:	SendStrHTTP(socket, "<traversals>\n");
							break;

	case DATA_FORMAT_RAW:
	case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#traversals\n");
							break;
	}

	if(traffic[domain])
		for(int i=0; i<traffic[domain]->count; i++)
		{
			node = GetTrafficNode(domain, i);
	
			if((node->traffic>=minhits) && (!fromHash || fromHash==node->fromPage) && (!toHash || toHash==node->toPage))

				switch(format)
				{
				case DATA_FORMAT_HTML:	sprintf(buffer, "<tr><td>%s</td><td>%s</td><td>%ld</td></tr>\n", GetUrl(VD[domain], node->fromPage), GetUrl(VD[domain], node->toPage), node->traffic);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:	sprintf(buffer, "<traversal src=\"%s\" dest=\"%s\" traffic=\"%ld\"/>\n", encodeURL(GetUrl(VD[domain], node->fromPage)), encodeURL(GetUrl(VD[domain], node->toPage)), node->traffic);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_RAW:	sprintf(buffer, "%s %s %ld\n", GetUrl(VD[domain], node->fromPage), GetUrl(VD[domain], node->toPage), node->traffic);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_HASH:	sprintf(buffer, "%lX %lX %ld\n", node->fromPage, node->toPage, node->traffic);
										SendStrHTTP(socket, buffer);
										break;
				}
		}

	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</traversals>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}
}

void DumpPages(int domain, char* buffer, long socket, long minhits, char format){
	StatList*	list;
	register	Statistic	*p;
	long		total;

	switch(format)
	{
		case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Pages</h2>\n");
								SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Page</td><td>Hash</td></tr>\n");
								break;

		case DATA_FORMAT_XML:	SendStrHTTP(socket, "<pages>\n");
								break;

		case DATA_FORMAT_RAW:
		case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#pages\n");
								break;
	}

	if(VD[domain]) 
	{
		list = VD[domain]->byPages;

		total = list->num;
		for(int j=0; j<total; j++) 
		{
			p = list->GetStat(j);

			if ( p->files>=minhits )
			{
				switch(format)
				{
					case DATA_FORMAT_HTML:
											sprintf(buffer, "<tr><td>%s</td><td>%lX</td></tr>\n", p->GetName(), p->hash);
											SendStrHTTP(socket, buffer);
											break;

					case DATA_FORMAT_XML:
											sprintf(buffer, "<file name=\"%s\" hash=\"%lX\">\n", encodeURL(p->GetName()), p->hash);
											SendStrHTTP(socket, buffer);
											break;
											  
					case DATA_FORMAT_RAW: 	
					case DATA_FORMAT_HASH: 
											sprintf(buffer, "%lX %s\n", p->hash, p->GetName());
											SendStrHTTP(socket, buffer);
											break;
				}
			}
		}
	}

	switch(format)
	{
		case DATA_FORMAT_HTML: 
								SendStrHTTP(socket, "</table>\n");
								break;

		case DATA_FORMAT_XML:	
								SendStrHTTP(socket, "</pages>\n");
								break;
		case DATA_FORMAT_RAW: 
		case DATA_FORMAT_HASH: 
								SendStrHTTP(socket, "\n");
								break;
	}
}

void DumpDomains(char* buffer, long socket, char format)
{
	switch(format)
	{
		case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Domains</h2>\n");
								SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Domain</td><td>Number</td></tr>\n");
								break;

		case DATA_FORMAT_XML:	SendStrHTTP(socket, "<domains>\n");
								break;

		case DATA_FORMAT_RAW:
		case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#domains\n");
								break;
	}

	for(int i=0; i<=VDnum; i++) {
		if(VD[i])
			switch (format) {
				case DATA_FORMAT_HTML:	sprintf(buffer, "<tr><td>%s</td><td>%ld</td></tr>\n", VD[i]->domainName, i);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:	sprintf(buffer, "<domain name=\"%s\" num=\"%ld\"/>\n", encodeURL(VD[i]->domainName), i);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_RAW:	sprintf(buffer, "%s %ld\n", VD[i]->domainName, i);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_HASH:	sprintf(buffer, "%s %lX\n", VD[i]->domainName, VD[i]->hash);
										SendStrHTTP(socket, buffer);
										break;
			}
	}

	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</domains>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}

}

void DumpSummary(char* buffer, long socket, char format)
{
    char d1[128],d2[128];
    VDinfoP VDPtr ;
    long    tv ;

	switch(format)
	{
   	case DATA_FORMAT_XML:	SendStrHTTP(socket, "<summary>\n");
							break;
   	default:                SendStrHTTP(socket, "#summary\n");
                            break ;
							
	}

	for(int i=0; i<=VDnum; i++) {
        VDPtr = VD[i] ;
        if(VDPtr) 
        {
            timeToString(VDPtr->firstTime,d1); 
            timeToString(VDPtr->lastTime,d2); 
  		    tv = VDPtr->byClient->GetStatListTotalVisits();
		    if ( !tv ) tv = 1;

			switch (format) {

				case DATA_FORMAT_XML:	

                                        sprintf(buffer, 
                                                "<domain name=\"%s\" logstart=\"%s\" logend=\"%s\" hits=\"%ld\" cachedhits=\"%ld\" errors=\"%ld\" days=\"%ld\" visitors=\"%ld\" repeatclients=\"%ld\" bytes=\"%ld\"/>\n", 
                                                encodeURL(VDPtr->domainName),
                                                d1,d2,
                                                VDPtr->totalRequests,
					                            VDPtr->totalCachedHits,
	                                            VDPtr->totalFailedRequests,
					                            VDPtr->totalDays,
					                            VDPtr->byClient->totalRequests,
					                            VDPtr->totalRepeatClients,
	                    		                VDPtr->totalBytes
                                                );
										SendStrHTTP(socket, buffer);
										break;
				default:	

                                        sprintf(buffer, 
                                                "logstart=%s\nlogend=%s\nhits=%ld\ncachedhits=%ld\nerrors=%ld\ndays=%ld\nsessions=%ld\nvisitors=%ld\nrepeatclients=%ld\navgsesstime=%ld\nbytes=%ld\n", 
                                                d1,d2,
                                                VDPtr->totalRequests,
					                            VDPtr->totalCachedHits,
	                                            VDPtr->totalFailedRequests,
					                            VDPtr->totalDays,
                                                VDPtr->byClient->totalVisits,
					                            VDPtr->byClient->num,
					                            VDPtr->totalRepeatClients,
		                                        VDPtr->byClient->GetStatListTotalTime()/tv,
	                    		                VDPtr->totalBytes
                                                );
										SendStrHTTP(socket, buffer);
										break;

			}
        }
	}

	switch(format)
	{
	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</summary>\n");
							break;
   	default:                SendStrHTTP(socket, "#end\n");
                            break ;
	}

}


#ifdef TRAFFIC_MATRIX
void DumpMatrix(int domain, char* buffer, long socket, char format) {
	switch(format)
	{
	case DATA_FORMAT_HTML:	SendStrHTTP(socket, "<h2>Traffic Matrix</h2>\n");
							SendStrHTTP(socket, "<table><tr bgcolor=silver><td>Page 1</td><td>Page 2</td><td><b>Count</b></td></tr>\n");
							break;

	case DATA_FORMAT_XML:	SendStrHTTP(socket, "<matrix>\n");
							break;

	case DATA_FORMAT_RAW:
	case DATA_FORMAT_HASH:	SendStrHTTP(socket, "#matrix\n");
							break;
	}

	int p;
	int count = traffic[domain]->trafficMatrixSize;
	Statistic* p1, *p2;

	for(int i=0; i<count; i++) {
		p1 = VD[domain]->byPages->GetStat(i);

		for(int j=0; j<count; j++)
		{
			int pos = i + j*traffic[domain]->trafficMatrixSize;
			p = traffic[domain]->trafficMatrix[pos];

			if(p)
			{
				p2 = VD[domain]->byPages->GetStat(j);


				switch(format)
				{
				case DATA_FORMAT_HTML:	sprintf(buffer, "<tr><td>%s</td><td>%s</td><td>%ld</td></tr>\n", p1->GetName() , p2->GetName() , p);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_XML:	sprintf(buffer, "<traversal src=\"%s\" dest=\"%s\" traffic=\"%ld\"/>\n", encodeURL(p1->GetName()) , encodeURL(p2->GetName()) , p);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_RAW:	sprintf(buffer, "%s %s %ld\n", p1->GetName() , p2->GetName() , p);
										SendStrHTTP(socket, buffer);
										break;

				case DATA_FORMAT_HASH:	sprintf(buffer, "%lX %lX %ld\n", p1->hash , p2->hash , p);
										SendStrHTTP(socket, buffer);
										break;
				}
			}
		}
	}

	switch(format)
	{
	case DATA_FORMAT_HTML: 
							SendStrHTTP(socket, "</table>\n");
							break;

	case DATA_FORMAT_XML:	
							SendStrHTTP(socket, "</matrix>\n");
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							SendStrHTTP(socket, "\n");
							break;
	}


	if(format==DATA_FORMAT_HTML) {
		SendStrHTTP(socket, "<table>\n");

		SendStrHTTP(socket, "<tr bgcolor = silver><td>&nbsp;</td>");

		for(int i=0; i<count; i++) {
			sprintf(buffer, "<td>%d</td>", i);
			SendStrHTTP(socket, buffer);
		}

		for(i=0; i<count; i++) {
			sprintf(buffer, "<tr><td bgcolor=silver>%d</td>", i);
			SendStrHTTP(socket,buffer);
			for(int j=0; j<count; j++) {
				int pos = i + j*traffic[domain]->trafficMatrixSize;
				p = traffic[domain]->trafficMatrix[pos];
	
				if(p)
				{
					sprintf(buffer, "<td>%d</td>", p);
					SendStrHTTP(socket,buffer);
				} else {
					SendStrHTTP(socket, "<td>&nbsp;</td>");
				}
			}
			SendStrHTTP(socket, "</tr>\n");
		}
		SendStrHTTP(socket, "</table>\n");
	}
}
#endif

long  progressSocket;
char  progressFormat;
char* progressBuffer;

void HttpShowProgressDetail( long level, long forceShow, char *msg, double timesofar ) {

	if(progressBuffer)
		switch(progressFormat) {
			case DATA_FORMAT_HTML:	sprintf(progressBuffer, "<script>document.title = \"%s - %d %% \"</script>\n", msg, (int) level/10);
									SendStrHTTP(progressSocket, progressBuffer);
									break;

			case DATA_FORMAT_XML:	sprintf(progressBuffer, "<progress msg=\"%s\" level=\"%d\"/>\n", msg, level);
									SendStrHTTP(progressSocket, progressBuffer);
									break;

			case DATA_FORMAT_RAW:
			case DATA_FORMAT_HASH:	sprintf(progressBuffer, "##%s %d\n", msg, level);
									SendStrHTTP(progressSocket, progressBuffer);
									break;
		}

	ShowProgressDetail(level, forceShow, msg, timesofar);
}

void HttpOutputError( const char* errorString )
{
	if(progressBuffer)
    {
		switch(progressFormat) {
			case DATA_FORMAT_HTML:	sprintf(progressBuffer, "<font color=red><h1>%s</h1></font>\n", errorString);
									SendStrHTTP(progressSocket, progressBuffer);
									break;

			case DATA_FORMAT_XML:	sprintf(progressBuffer, "<error text=\"%s\"/>\n", errorString);
									SendStrHTTP(progressSocket, progressBuffer);
									break;

			case DATA_FORMAT_RAW:
			case DATA_FORMAT_HASH:	sprintf(progressBuffer, "#!%s\n", errorString);
									SendStrHTTP(progressSocket, progressBuffer);
									break;
		}
    }
}                                     

#define MAX_FILENAMES 512
int DoProcessData(char* buffer, long socket, char* files, char format, char progress, char* database, int doTop) {		
	ProcessData logdata;
	char*	temp;
	char   *tmpFileName ;
	int		fileCount = 0;
    long    retCode = 0 ;
    char    fileName[MAXFILENAMELEN+1] ;
    
	temp = NextWord(&files, 0);
                                                         
    ClearLogList();

	while(*temp && fileCount<MAX_FILENAMES) 
    {
        tmpFileName = NextWord(&temp,',') ;
#ifdef DEF_MAC
        // MacOSX POSIX path must be converted into HFS ,format (ie: Volume:path:path2:filename)
        strncpyHFSPath(fileName,tmpFileName,MAXFILENAMELEN) ;
#else
        strncpy(fileName,tmpFileName,MAXFILENAMELEN) ;
#endif			    

		fileCount = AddLogFile(fileName,fileCount) ;
	}


	logdata.prefs = &MyPrefStruct;

	if(progress) {
		progressSocket = socket;
		progressFormat = format;
		progressBuffer = buffer;

		logdata.ShowProgressDetail = HttpShowProgressDetail;
	} else
		logdata.ShowProgressDetail = ShowProgressDetail;

	logdata.fsFile = glogFilenames;
	logdata.logNum = fileCount;
	logdata.outputSocket = socket;
	logdata.reportType = REPORT_TYPE_XML;

	if(database) {
		strcpy((char*)&logdata.prefs->database_file, database);

		logdata.prefs->database_active = TRUE;
	}

	if(format==DATA_FORMAT_HTML) {
		sprintf(buffer, "<h2>Processing logs: %s</h2>", files);
		SendStrHTTP(socket, buffer);
		SendStrHTTP(socket, "<br>");
	}

	retCode = GoProcessLogs( &logdata );

    if (retCode != 0)
    {
      sprintf(buffer,"#!Log Processing failed with error code %ld\r\n", retCode) ;
      SendStrHTTP(socket,buffer) ;
      return(FALSE) ;
    }

	int domainCount = 0;

	ClearTrafficList();
	for(int i=0; i <= VDnum; i++) {
		if(VD[i]) {
			domainCount++; 
			if(format==DATA_FORMAT_HTML) {
				SendStrHTTP( socket, "<h2>Processing Virtual Domain:" );
				SendStrHTTP( socket, VD[i]->domainName );
				SendStrHTTP( socket, "</h2><br>" );
			}

			int top;
			if(VD[i]->byPages->num<doTop)
				top = VD[i]->byPages->num;
			else
				top = doTop;

			CreateDomain(i, top);

			BuildTrafficList(i, VD[i]->byClient, &logdata);
		}
	}

	if(!domainCount) {
		switch(format) {
		case DATA_FORMAT_HTML:	SendStrHTTP( socket, "<font color=red><h1>Processing Failed</h1></font>\n"); break;
		case DATA_FORMAT_XML:	SendStrHTTP( socket, "<error text=\"Processing Failed\"/>\n"); break;
		case DATA_FORMAT_HASH:
		case DATA_FORMAT_RAW:	SendStrHTTP( socket, "#!Processing Failed\n"); break;
		}
	} else {
		switch(format) {
		case DATA_FORMAT_HTML:	SendStrHTTP( socket, "<h1>Processing Successful</h1>\n"); break;
		case DATA_FORMAT_XML:	SendStrHTTP( socket, "<message text=\"Processing Successful\"/>\n"); break;
		case DATA_FORMAT_HASH:
		case DATA_FORMAT_RAW:	SendStrHTTP( socket, "#:)Processing Successful\n"); break;
		}
	}

	if(format==DATA_FORMAT_HTML) 
		SendStrHTTP(socket, "<br>");


	progressBuffer = NULL;

    return(domainCount != 0) ;
}



// Command Constants 
//  
//

#define QUERY_DATABASE                                      1
#define QUERY_FILE                                          2
#define QUERY_PROGRESS                                      3
#define QUERY_DOMAIN                                        4
#define QUERY_MINHITS                                       5
#define QUERY_DOTOP                                         6
#define QUERY_PAGE                                          7
#define QUERY_SRCPAGE                                       8
#define QUERY_DESTPAGE                                      9
#define QUERY_UNIQUEURLS                                    10
#define QUERY_REPORTDIR                                     11
#define QUERY_DETAIL                                        12
#define QUERY_FORMAT                                        13
#define QUERY_STAT                                          14
#define QUERY_DATE                                          15
#define QUERY_TORAW                                         16
#define QUERY_FROMRAW                                       17
#define QUERY_FROM                                          18
#define QUERY_TO                                            19
#define QUERY_IGNORESELF                                    20
#define QUERY_IGNORECASE                                    21
#define QUERY_IGNOREUSERNAME                                22
#define QUERY_IGNOREZEROBYTES                               23
#define QUERY_DNSAMOUNT                                     24
#define QUERY_COUNT                                         25
#define QUERY_MAXLENGTH                                     26
#define QUERY_MINLENGTH                                     27
#define QUERY_INFO                                          28
#define QUERY_SETWINDOW                                     29
#define QUERY_HELP                                          30
                                           
#define QUERY_WIN_TOPCORNER                                 1
#define QUERY_WIN_MINIMIZE                                  2
#define QUERY_WIN_MAXIMIZE                                  3
#define QUERY_WIN_NORMAL                                    4
#define QUERY_WIN_HIDE                                      5
#define QUERY_WIN_CLOSE                                     6
#define QUERY_WIN_COMPACT                                   8
                                           
#define QUERY_DATE_USER                                     1
#define QUERY_DATE_ALL                                      2
#define QUERY_DATE_TODAY                                    3
#define QUERY_DATE_YESTERDAY                                4
#define QUERY_DATE_THISWEEK                                 5
#define QUERY_DATE_PREVWEEK                                 6
#define QUERY_DATE_LAST7                                    7
#define QUERY_DATE_LAST14                                   8
#define QUERY_DATE_LAST21                                   9
#define QUERY_DATE_LAST30                                   10
#define QUERY_DATE_THISMONTH                                12
#define QUERY_DATE_PREVMONTH                                13
#define QUERY_DATE_THISQUART                                14
#define QUERY_DATE_PREVQUART                                15
#define QUERY_DATE_THIS6M                                   16
#define QUERY_DATE_PREV6M                                   17
#define QUERY_DATE_THISYEAR                                 18
#define QUERY_DATE_PREVYEAR                                 19
                                           
#define QUERY_STAT_ALLSESSIONS                              1 
#define QUERY_STAT_SESSIONS                                 2 
#define QUERY_STAT_ALL                                      3 
#define QUERY_STAT_HITS                                     4 
#define QUERY_STAT_MOSTACTIVEMINUTE                         5 
#define QUERY_STAT_MOSTACTIVEDAY                            6 
#define QUERY_STAT_MOSTACTIVEHOUR                           7 
#define QUERY_STAT_MOSTACTIVE                               8 
#define QUERY_STAT_ERROR                                    9 
#define QUERY_STAT_ENTRIES                                  10
#define QUERY_STAT_EXITS                                    11
#define QUERY_STAT_TRAVERSALS                               12
#define QUERY_STAT_PATHS                                    13
#define QUERY_STAT_PAGES                                    14
#define QUERY_STAT_DOMAINS                                  15
#define QUERY_STAT_SUMMARY                                  16
#define QUERY_STAT_MATRIX                                   17
                                                       
struct commandList                                    
{
     char *name ;
     char *description ;
     char *argType ;
     void *argList ; 
     int   id ;
} ;

#define  ARG_TYPE_STRING           "string"
#define  ARG_TYPE_BOOLEAN          "1=true 0=false"
#define  ARG_TYPE_INTEGER          "integer"
#define  ARG_TYPE_PATH             "filename"
#define  ARG_TYPE_URL              "URL"
#define  ARG_TYPE_NONE             "n/a"
#define  ARG_TYPE_DATE             "dd mon yyyy"
#define  ARG_TYPE_DATERAW          "seconds since 1/1/70"
#define  ARG_TYPE_CONSTANT         "Set Value"
#define  ARG_TYPE_EMPTY            "" 

struct commandList windowCommands[] =
{
    "topcorner" ,"Relocate the window in the top corner of the screen"                ,ARG_TYPE_EMPTY      ,NULL,   QUERY_WIN_TOPCORNER,
    "minimize"  ,"Minimize the window"                                                ,ARG_TYPE_EMPTY      ,NULL,   QUERY_WIN_MINIMIZE,
    "maximize"  ,"Maximize the window"                                                ,ARG_TYPE_EMPTY      ,NULL,   QUERY_WIN_MAXIMIZE,
    "normal"    ,"Return the window to its normal state"                              ,ARG_TYPE_EMPTY      ,NULL,   QUERY_WIN_NORMAL,
    "hide"      ,"Hie the window"                                                     ,ARG_TYPE_EMPTY      ,NULL,   QUERY_WIN_HIDE,
    "close"     ,"Close the window and terminate Analyzer"                            ,ARG_TYPE_EMPTY      ,NULL,   QUERY_WIN_CLOSE,
    "compact"   ,"Put a small version of the window in the top left corner of the screen",ARG_TYPE_EMPTY   ,NULL,   QUERY_WIN_COMPACT,
    NULL,NULL,NULL,NULL,0 
} ;

struct commandList dateCommands[] =
{
   	"user"      ,"Log file processing date range is set by fro, fromraw, to and toraw" ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_USER,
   	"all"       ,"Process all entries in all log files"                                ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_ALL,
	"today"     ,"Only process entries for today "                                     ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_TODAY,
	"yesterday" ,"Only process entries for yesterday"                                  ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_YESTERDAY,
	"thisweek"  ,"Only process entries for this week"                                  ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_THISWEEK,
	"prevweek"  ,"Only process entries for the previous week"                          ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_PREVWEEK,
	"last7"     ,"Only process entries for the last 7 days"                            ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_LAST7,
	"last14"    ,"Only process entries for the last 14 days"                           ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_LAST14,
	"last21"    ,"Only process entries for the last 21 days"                           ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_LAST21,
	"last30"    ,"Only process entries for the last 30 days"                           ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_LAST30,
	"thismonth" ,"Only process entries for this month"                                 ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_THISMONTH,
	"prevmonth" ,"Only process entries for the previous month"                         ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_PREVMONTH,
	"thisquart" ,"Only process entries for this quarter"                               ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_THISQUART,
	"prevquart" ,"Only process entries for the previous quarter"                       ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_PREVQUART,
	"this6m"    ,"Only process entries for the current half year"                      ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_THIS6M,
	"prev6m"    ,"Only process entries for the previous half year"                     ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_PREV6M,
	"thisyear"  ,"Only process entries for this year"                                  ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_THISYEAR,
	"prevyear"  ,"Only process entries for last year"                                  ,ARG_TYPE_EMPTY    ,NULL,   QUERY_DATE_PREVYEAR,
    NULL,NULL,NULL,NULL,0 
} ;

struct commandList statisticCommands[] =
{
    "allsessions"      ,"Report the paths for all individual sessions"                 ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_ALLSESSIONS,
	"sessions"         ,"Report unique paths for the sessions, sorted by the most popular path"          ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_SESSIONS,
	"all"              ,"Report some common statistics about each page"                ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_ALL,
	"hits"             ,"Report page hits"                                             ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_HITS,
	"mostactiveday"    ,"Report the most active day. Use detail=1 to include all the sessions and hits in that day"          ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_MOSTACTIVEDAY,
	"mostactivehour"   ,"Report the most active hour. Use detail=1 to include all the sessions and hits in that hour"          ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_MOSTACTIVEHOUR,
	"mostactive"       ,"Report the most active minute, hour and day. Use detail=1 to include all the sessions and hits in each period"          ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_MOSTACTIVE,
	"errors"           ,"Report page errors"                                           ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_ERROR,
	"entries"          ,"Report page entries"                                          ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_ENTRIES,
	"exits"            ,"Report page exits"                                            ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_EXITS,
	"traversals"       ,"Report traversals from one page to another"                   ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_TRAVERSALS,
	"paths"            ,"Report the top paths"                                         ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_PATHS,
	"pages"            ,"Report each pages URL and its internal hash value"            ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_PAGES,
	"domains"          ,"Report each domain covered by the log files"                  ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_DOMAINS,
	"summary"          ,"Report a summary of the traffic information for the site"     ,ARG_TYPE_EMPTY    ,NULL,   QUERY_STAT_SUMMARY,
    NULL,NULL,NULL,NULL,0 
} ;


struct commandList queryCommands[] =
{
	"progress"   ,"Show or do not show progress about the log file processing"         ,ARG_TYPE_BOOLEAN  ,NULL,   QUERY_PROGRESS,
	"domain"     ,"Specify the domain when log files span multiple sites [UNAVAILABLE]"  ,ARG_TYPE_INTEGER   ,NULL,   QUERY_DOMAIN,
	"minhits"    ,"Minumum number of hits to report"                                  ,ARG_TYPE_INTEGER  ,NULL,   QUERY_MINHITS,
	"dotop"      ,"Specify number of results to return (ie: top 10, top 20)"          ,ARG_TYPE_INTEGER  ,NULL,   QUERY_DOTOP,
	"page"       ,"Limit statistics to the page given"                                ,ARG_TYPE_URL  ,NULL,   QUERY_PAGE,
	"srcpage"    ,"When showing traversal statistics nominate a source page (default is any page)"     ,ARG_TYPE_URL  ,NULL,   QUERY_SRCPAGE,
	"destpage"   ,"When showing traversal statistics nominate a destination page (default is any page)"          ,ARG_TYPE_URL    ,NULL,   QUERY_DESTPAGE,
	"uniqueurls" ,"Treat query parameters (characters after a ?) as part of the URL"           ,ARG_TYPE_BOOLEAN   ,NULL,   QUERY_UNIQUEURLS,
	"reportfile" ,"If set to a file name, will generate analyzer reports in the directory of that file name" ,ARG_TYPE_PATH          ,NULL,   QUERY_REPORTDIR,
	"reportdir"  ,"If set to a file name, will generate analyzer reports in the directory of that file name [DEPRECATED]" ,ARG_TYPE_PATH          ,NULL,   QUERY_REPORTDIR,
	"format"     ,"Specify the output format. Valid chioces are html, raw and xml"    ,ARG_TYPE_BOOLEAN  ,NULL,   QUERY_FORMAT,
	"date"       ,"Specifies a date restriction on log file processing" ,ARG_TYPE_CONSTANT,dateCommands,   QUERY_DATE,
	"toraw"      ,"All log file entries prior to this time are ignored" ,ARG_TYPE_DATERAW,NULL,   QUERY_TORAW,
	"fromraw"    ,"All log file entries after to this time are ignored" ,ARG_TYPE_DATERAW,NULL,   QUERY_FROMRAW,
	"to"         ,"All log file entries prior to this time are ignored" ,ARG_TYPE_DATE,NULL,   QUERY_TO,
	"from"       ,"All log file entries after to this time are ignored" ,ARG_TYPE_DATE,NULL,   QUERY_FROM,
	"ignoreself" ,"Ignore traffic traveling directly from a page to itself"        ,ARG_TYPE_BOOLEAN  ,NULL,   QUERY_IGNORESELF,
	"ignorecase" ,"Ignore the case of URLs"                                        ,ARG_TYPE_BOOLEAN  ,NULL,   QUERY_IGNORECASE,
	"ignoreusername","Do not rely on the username field to seperate clients"       ,ARG_TYPE_BOOLEAN  ,NULL,   QUERY_IGNOREUSERNAME,
	"ignorezerobytes","Ignore zero byte transfers recorded in the log file"        ,ARG_TYPE_BOOLEAN  ,NULL,   QUERY_IGNOREZEROBYTES,
    "database"   ,"Specify a Report Database to process (all commands above must preceed this command)"                       ,ARG_TYPE_STRING  ,NULL,   QUERY_DATABASE ,
	"file"       ,"Specify a log file or log file directory to process."            ,ARG_TYPE_PATH  ,NULL,   QUERY_FILE,
	"detail"     ,"Show all session detail in mostactive type queries"             ,ARG_TYPE_BOOLEAN  ,NULL,   QUERY_DETAIL,
	"count"      ,"Specify number of results to return (ie: top 10, top 20)"           ,ARG_TYPE_INTEGER   ,NULL,   QUERY_COUNT,
	"maxlength"  ,"Truncate all paths returned by stat=sessions to this length" ,ARG_TYPE_INTEGER  , NULL, QUERY_MAXLENGTH,
	"minlength"  ,"Ignore paths returned by stat=sessions that are shorter than this value",ARG_TYPE_INTEGER  ,NULL,   QUERY_MINLENGTH,
	"info"       ,"Return an XML document describing this version of Analyzer and the version of the http interface" ,ARG_TYPE_EMPTY  ,NULL,   QUERY_INFO,
	"setwindow"  ,"Modify the position and size of the window (Win32 only)"       ,ARG_TYPE_CONSTANT   ,windowCommands,QUERY_SETWINDOW,
	"stat"       ,"Specifies the statistic of interest"                            ,ARG_TYPE_CONSTANT  ,statisticCommands, QUERY_STAT,
//dnsamount"  ,"Specify a log file or log file directory to process"           ,ARG_TYPE_BOOLEAN   ,NULL,   QUERY_DNSAMOUNT,
	"help"       ,"Generate this page"                                            ,ARG_TYPE_EMPTY   ,NULL,QUERY_HELP,
    NULL,NULL,NULL,NULL, -1
} ;


#define ROW_COLOR_1   "#eeeeff"
#define ROW_COLOR_2   "#ddddee"

void dumpList(int socket, char *buffer, struct commandList *list) 
{
    static char *rowColor=ROW_COLOR_1 ;

    SendStrHTTP(socket,"<TABLE BORDER=0>") ;
    SendStrHTTP(socket,"<TR>") ;
    SendStrHTTP(socket,"<TD bgcolor=#444444 width=10%%><font color=white size=2><b>Command</b></font></TD>") ;
    SendStrHTTP(socket,"<TD bgcolor=#444444 width=10%%><font color=white size=2><b>Value Type</b></font></TD>") ;
    SendStrHTTP(socket,"<TD bgcolor=#444444 ><font color=white size=2><b>Description</b></font></TD>") ;
    SendStrHTTP(socket,"</TR>") ;

    for (int i=0; list[i].name != NULL; i++)
    {
        SendStrHTTP(socket,"<TR>") ;
        sprintf(buffer,"<TD bgcolor=%s width=10%% valign=top><font size=2>%s</font></TD>",rowColor, list[i].name) ;
        SendStrHTTP(socket,buffer) ;
        sprintf(buffer,"<TD bgcolor=%s width=10%% valign=top><font size=2>%s</font></TD>",rowColor, list[i].argType) ;
        SendStrHTTP(socket,buffer) ;
        sprintf(buffer,"<TD bgcolor=%s valign=top><font size=2>%s</font>",rowColor, list[i].description) ;
        SendStrHTTP(socket,buffer) ;
        if (list[i].argList != NULL)
        {
            SendStrHTTP(socket,"<BR>") ;
            dumpList(socket, buffer, (struct commandList *) list[i].argList) ;
        }
        SendStrHTTP(socket,"</TD>") ;
        SendStrHTTP(socket,"</TR>") ;
        if (rowColor == ROW_COLOR_1)
            rowColor= ROW_COLOR_2 ;
        else
            rowColor= ROW_COLOR_1 ;
    }
    SendStrHTTP(socket,"</TABLE>") ;
}
void dumpHelp(long socket, char *buffer)
{
    sprintf(buffer,"<HTML><BODY><font size=2><b>%s %s HTTP Interface Help</b></font>",PRODUCT_TITLE, VERSION_STRING) ;
    SendStrHTTP(socket,buffer) ;
    sprintf(buffer,"<font size=1>(interface version %s)</font>",INTERFACE_VERSION) ;
    SendStrHTTP(socket,buffer) ;
    dumpList(socket, buffer, queryCommands) ;

    SendStrHTTP(socket, "<HR height=1><FONT size=2>") ;
    sprintf(buffer, "<A HREF=http://localhost:800/query?setwindow=close>Shut down %s<A><BR>",PRODUCT_TITLE) ;
    SendStrHTTP(socket,buffer) ;
    SendStrHTTP(socket, "<HR height=1>Analysis Software 2006</FONT>") ;
    SendStrHTTP(socket,"</BODY></HTML>") ;

}

void SendHeader(long socket, char format, char* buffer, char *newLogFileNames)
{
    static char *logFileNames = NULL ;
    
    if (newLogFileNames != NULL)
    {
       if (logFileNames != NULL) free(logFileNames) ;
       logFileNames = (char *)malloc(strlen(newLogFileNames)+1) ;
       strcpy(logFileNames,newLogFileNames) ;
    }
    
	sprintf( buffer, "HTTP/1.0 200 OK\r\n" );
	strcat( buffer, "Server: iReporter ") ;

    strcat( buffer, VERSION_STRING) ;

	#if DEF_UNIX
		strcat(buffer, "(Unix)\r\n" );
	#elif DEF_WINDOWS
		strcat(buffer, "(Windows)\r\n" );
	#endif

	strcat( buffer, "Accept-Ranges: bytes\r\n" );
	strcat( buffer, "Connection: close\r\n" );

	switch(format) {
	case DATA_FORMAT_HTML: 
							strcat( buffer, "Content-Type: text/html\r\n\r\n\0" );
							break;

	case DATA_FORMAT_XML:	
							strcat( buffer, "Content-Type: text/xml\r\n\r\n\0" );
							break;
	case DATA_FORMAT_RAW: 
	case DATA_FORMAT_HASH: 
							strcat( buffer, "Content-Type: text/plain\r\n\r\n\0" );
							break;
	}

	SendStrHTTP(socket, buffer);

	if(format==DATA_FORMAT_XML)
    {
        time_t now = time(0) ;
        sprintf(buffer,"<logfilestats src=\"%s %s\" created=\"%s\" logs=\"%s\">\n", PRODUCT_TITLE, VERSION_STRING, ctime(&now), (logFileNames==NULL)?"":logFileNames) ;
		SendStrHTTP(socket, buffer);
    }
}



void resetWindowTitle()
{
#if DEF_WINDOWS
    StatusSet("Processing Web Query") ;
    UpdateProgressBar(0);
#endif
}


void ProcessHTTPQuery(long socket, char* query)
{

	processing_http_request = TRUE;
    char  reportDirectory[MAXFILENAMELEN+1] = "" ;
    int   doReport = 0 ;
	char *buffer;
	buffer = (char*)malloc( XML_BUF_SIZE );
	buffer[0] = 0;

    struct App_config oldMyPrefStruct ;

	char	headerSent	= 0;
	char*	pageName	= "*";
	char*	srcPage		= "*";
	char*	destPage	= "*";
	char*	temp		= NULL;
	int		minHits		= 0;
	char	format		= DATA_FORMAT_XML;
	int		domain		= 0;
	char	progress	= 0;
	char*	database	= 0;
	int		doTop		= 100;
    int     maxPathLength = 0 ;
    int     minPathLength = 0 ;
    int     detailFlag    = 0 ;

    int     commandIndex  = 0 ;
    int     status      = 0;

    time_t    fromTime     = 0 ;
    time_t    toTime       = 0 ;



    resetWindowTitle() ;

    gSaved = TRUE ;
    strcpy(gPrefsFilename,"") ;

    // Make a copy of the current preferences so we can change some settings for this run 
    //
    memcpy( &oldMyPrefStruct, &MyPrefStruct, CONFIG_SIZE) ;
    
    // set some options up
    MyPrefStruct.dnslookup = 0 ; // This query needs DNS lookups turned off 'cos it's way too slow

	ConvertURLtoPlain(query);

    // Parse and Execute the Query.
    //
    // !! Do not return from within this loop since there is some tidy up code at the bottom !!
    //
	while(query && *query)
	{
        for (commandIndex=0; queryCommands[commandIndex].name != NULL;commandIndex++) 
        {
            if (strcmpd(queryCommands[commandIndex].name, query)==0)
            {
                query = query+strlen(queryCommands[commandIndex].name) ; // flush command
                if (*query=='=') query++ ; // flush any equals sign
                break ;
            }
        }

		switch (queryCommands[commandIndex].id) 
        {

        case QUERY_DATABASE:
  	            database = NextWord(&query,0);
		        break ;

		case QUERY_FILE:
			    temp = NextWord(&query,0) ;

			    if(!headerSent) 
                {
				    SendHeader(socket, format, buffer, temp);
				    headerSent = 1;
			    }
			    status = DoProcessData(buffer, socket, temp, format, progress, database, doTop) ;
                break ;

		case QUERY_PROGRESS:
	    
			    temp = NextWord(&query,0);

			    progress = atoi(temp);
                break ;

		case QUERY_DOMAIN:
	
			    temp = NextWord(&query,0);
			    domain = atoi(temp);
		        break ;

		case QUERY_MINHITS:
	
			temp = NextWord(&query,0);

			minHits = atoi(temp);
		    break;
		case QUERY_DOTOP:
	
			temp = NextWord(&query,0);

			doTop = atoi(temp);
		    break;
		case QUERY_PAGE:
	
			pageName = NextWord(&query,0);

		    break;
		case QUERY_SRCPAGE:
	
			srcPage = NextWord(&query,0);
		    break;
		case QUERY_DESTPAGE:
	
			destPage = NextWord(&query,0);
		    break;
		case QUERY_UNIQUEURLS:
	
			MyPrefStruct.useCGI = atoi(NextWord(&query,0));
		    break; 
		case QUERY_REPORTDIR:
	
			strcpy(reportDirectory,NextWord(&query,0));
#ifdef DEF_MAC
            strncpyHFSPath(reportDirectory, reportDirectory, MAXFILENAMELEN) ;                
#endif			    

            doReport = 1; 
		    break;
		case QUERY_DETAIL:
	
			detailFlag = atoi(NextWord(&query,0));
		    break;
		case QUERY_FORMAT:
	
			temp = NextWord(&query,0);

			if( !strcmpd("raw", temp) )
				format = DATA_FORMAT_RAW;
			else
				if( !strcmpd("xml", temp) )
					format = DATA_FORMAT_XML;
				else
					if( !strcmpd("hash", temp) )
						format = DATA_FORMAT_HASH;
					else
						format = DATA_FORMAT_HTML;

			if(!headerSent) {
				SendHeader(socket, format, buffer, NULL);
				headerSent = 1;
			}
		    break;
		case QUERY_STAT:
			if(!headerSent) 
            {
				SendHeader(socket, format, buffer, NULL);
				headerSent = 1;
			}

			if(!VD[0] || !traffic[0])
			{
				SendStrHTTP(socket, "#!No analysis has yet been run\n");
				break; 
			}

	
			temp = NextWord(&query,0);

			if(!strcmpd("allsessions",temp)) {
                int oldformat = format ;
                if (format == DATA_FORMAT_HASH) format = DATA_FORMAT_RAW ;
				DumpUserSessions(domain, VD[0]->byClient, buffer, socket, format, TRUE, 0, maxPathLength, minPathLength,fromTime,toTime);
                format = oldformat ;
			} else
			if(!strcmpd("sessions",temp)) {
                int oldformat = format ;
                if (format == DATA_FORMAT_HASH) format = DATA_FORMAT_RAW ;
				DumpUserSessions(domain, VD[0]->byClient, buffer, socket, format, FALSE, doTop, maxPathLength, minPathLength,fromTime,toTime);
                format = oldformat ;
			} else
			if(!strcmpd("all",temp)) {
				DumpAllStats(domain, buffer, socket, pageName, format, minHits);
			} else
			if(!strcmpd("hits",temp)) {
				DumpHits(domain, buffer, socket, pageName, format, minHits,0,0);
			} else
			if(!strcmpd("mostactiveday",temp)) {
				DumpMostActive(domain, VD[0]->byClient, buffer, socket, format, TRUE, doTop, maxPathLength, minPathLength, minHits, MOST_ACTIVE_DAY,detailFlag,fromTime,toTime);
			} else
			if(!strcmpd("mostactivehour",temp)) {
				DumpMostActive(domain, VD[0]->byClient, buffer, socket, format, TRUE, doTop, maxPathLength, minPathLength, minHits, MOST_ACTIVE_HOUR,detailFlag,fromTime,toTime);
			} else
			if(!strcmpd("mostactive",temp)) {
				DumpMostActive(domain, VD[0]->byClient, buffer, socket, format, TRUE, doTop, maxPathLength, minPathLength, minHits, MOST_ACTIVE_ALL,detailFlag,fromTime,toTime);
			} else
			if(!strcmpd("errors",temp)) {
				DumpErrors(domain, buffer, socket, pageName, format, minHits);
			} else
			if(!strcmpd("entries",temp)) {
				DumpEntries(domain, buffer, socket, pageName, format, minHits);
			} else
			if(!strcmpd("exits",temp)) {
				DumpExits(domain, buffer, socket, pageName, format, minHits);
			} else
			if(!strcmpd("traversals",temp)) {
				DumpTraversals(domain, buffer, socket, srcPage, destPage, format, minHits);
			} else
			if(!strcmpd("paths",temp)) {
				DumpPaths(domain, VD[0]->byClient, buffer, socket, format);
			} else
			if(!strcmpd("pages",temp)) {
				DumpPages(domain, buffer, socket, minHits, format);
			} else
			if(!strcmpd("domains",temp)) {
				DumpDomains(buffer, socket, format);
			} else
			if(!strcmpd("summary",temp)) {
				DumpSummary(buffer, socket, format);
			} else
#ifdef TRAFFIC_MATRIX
			if(!strcmpd("matrix",temp)) {
				DumpMatrix(domain,buffer, socket, format);
			} else
#endif
			{
				sprintf(buffer, "#!Unknown statistic '%s'\n", temp);
				SendStrHTTP( socket, buffer);
				break;
			}
		    break; 
		case QUERY_DATE:
	
			temp = NextWord(&query,0);
   			if ( !mystrcmpi( "user", temp ) )		MyPrefStruct.alldates = 0;                else
   			if ( !mystrcmpi( "all", temp ) )		MyPrefStruct.alldates = DATE_ALL;         else
			if ( !mystrcmpi( "today", temp ) )		MyPrefStruct.alldates = DATE_TODAY;       else
			if ( !mystrcmpi( "yesterday", temp ) )	MyPrefStruct.alldates = DATE_YESTERDAY;   else
			if ( !mystrcmpi( "thisweek", temp ) )	MyPrefStruct.alldates = DATE_THISWEEK;    else
			if ( !mystrcmpi( "prevweek", temp ) )	MyPrefStruct.alldates = DATE_PREVWEEK;    else
			if ( !mystrcmpi( "last7", temp ) )		MyPrefStruct.alldates = DATE_LAST7;       else
			if ( !mystrcmpi( "last14", temp ) )	    MyPrefStruct.alldates = DATE_LAST14;      else
			if ( !mystrcmpi( "last21", temp ) )	    MyPrefStruct.alldates = DATE_LAST21;      else
			if ( !mystrcmpi( "last28", temp ) )	    MyPrefStruct.alldates = DATE_LAST28;      else
			if ( !mystrcmpi( "thismonth", temp ) )	MyPrefStruct.alldates = DATE_THISMONTH;   else
			if ( !mystrcmpi( "prevmonth", temp ) )	MyPrefStruct.alldates = DATE_PREVMONTH;   else
			if ( !mystrcmpi( "thisquart", temp ) )	MyPrefStruct.alldates = DATE_THISQUART;   else
			if ( !mystrcmpi( "prevquart", temp ) )	MyPrefStruct.alldates = DATE_PREVQUART;   else
			if ( !mystrcmpi( "this6m", temp ) )	    MyPrefStruct.alldates = DATE_THIS6M;      else
			if ( !mystrcmpi( "prev6m", temp ) )	    MyPrefStruct.alldates = DATE_PREV6M;      else
			if ( !mystrcmpi( "thisyear", temp ) )	MyPrefStruct.alldates = DATE_THISYEAR;    else
			if ( !mystrcmpi( "prevyear", temp ) )	MyPrefStruct.alldates = DATE_PREVYEAR;    else
			MyPrefStruct.alldates = atoi( temp );

            DatetypeToDate(MyPrefStruct.alldates, &fromTime, &toTime) ;
		    break; 

		case QUERY_TORAW:
	        MyPrefStruct.endTimeT = toTime = atol(NextWord(&query,0)) ;
		    break;
		case QUERY_FROMRAW:
	        MyPrefStruct.startTimeT = fromTime = atol(NextWord(&query,0)) ;
		    break;
		case QUERY_FROM:
			{
				char t[64];
				strncpy(t,NextWord(&query,0),19);
		        TimeStringToDays(t,&fromTime) ;
				MyPrefStruct.startTimeT = fromTime;
			}
		    break;
		case QUERY_TO:
			{
				char t[64];
				strncpy(t,NextWord(&query,0),19);
		        TimeStringToDays(t,&toTime) ;
				MyPrefStruct.endTimeT = toTime;
			}
		    break;

		case QUERY_IGNORESELF:
	
			MyPrefStruct.ignore_selfreferral = atoi(NextWord(&query,0));
		    break;
		case QUERY_IGNORECASE:
	
			MyPrefStruct.ignorecase = atoi(NextWord(&query,0));
		    break;
		case QUERY_IGNOREUSERNAME:
	
			MyPrefStruct.ignore_usernames = atoi(NextWord(&query,0));
		    break;
		case QUERY_IGNOREZEROBYTES:
	
			MyPrefStruct.filter_zerobyte = atoi(NextWord(&query,0));
		    break;
		case QUERY_DNSAMOUNT:
	
			MyPrefStruct.dnsAmount = atoi(NextWord(&query,0));
		    break;
		case QUERY_COUNT:
            // placeholder
            doTop = atoi(NextWord(&query,0));
		    break;
		case QUERY_MAXLENGTH:
            maxPathLength = atoi(NextWord(&query,0));
		    break;
		case QUERY_MINLENGTH:
            minPathLength = atoi(NextWord(&query,0));
		    break;
        case QUERY_INFO:
            char buildInfoBuff[1024] ;
			if(!headerSent) 
            {
                int oldformat = format ;
                format = DATA_FORMAT_XML ;
				SendHeader(socket, format, buffer, NULL);
				headerSent = 1;
                format=oldformat ;
			}
            buildInfoBuff[0] = '\0' ;
            GetAppBuildDetails(buildInfoBuff) ;
            sprintf(buffer,"<info interfaceversion=\"%s\" name=\"%s\" build=\"%s\" />",INTERFACE_VERSION,PRODUCT_TITLE, VERSION_STRING, buildInfoBuff) ;
            SendStrHTTP(socket,buffer) ;
            break;
		case QUERY_SETWINDOW:
                temp = NextWord(&query,0) ; 
#ifdef DEF_MAC
                if (strcmp(temp,"close") == 0)
                {
                  gShutdownRequestedViaHTTP = true ;
                }
                else
                if (strcmp(temp,"compact") == 0)
                {
                  // change the window size and position in here
                }
#endif              
#ifdef DEF_WINDOWS           
                if (strcmp(temp,"topcorner") == 0)
	              SetWindowPos(hwndParent,NULL,0,0,0,0,SWP_NOSIZE | SWP_NOZORDER) ;
                else
                if (strcmp(temp,"minimize") == 0)
	              ShowWindow(hwndParent,SW_MINIMIZE) ;
                else
                if (strcmp(temp,"maximize") == 0)
	              ShowWindow(hwndParent,SW_MAXIMIZE) ;
                else
                if (strcmp(temp,"normal") == 0)
	              ShowWindow(hwndParent,SW_NORMAL) ;
                else
                if (strcmp(temp,"hide") == 0)
	              ShowWindow(hwndParent,SW_HIDE) ;
                else
                if (strcmp(temp,"close") == 0)
                {
                  gSaved = TRUE ;
	              SendMessage(hwndParent, WM_CLOSE,0,0) ;
                }
                else
                if (strcmp(temp,"compact") == 0)
                {
                    RECT rc ;
                    long style ;
			        GetWindowRect( hwndParent, &rc );
                    SetMenu(hwndParent, NULL) ;
                    ShowWindow(ghMainDlg, SW_HIDE) ;
                    style = GetWindowLong(hwndParent, GWL_STYLE) ;
                    SetWindowLong(hwndParent, GWL_STYLE, style & ~WS_MAXIMIZEBOX) ;
		   	        MoveWindow( hwndParent,  0,0, rc.right-rc.left, 46, TRUE);
                }
#endif
               break ;

        case QUERY_HELP:
               
			   if(!headerSent) 
               {
				    SendHeader(socket, FORMAT_HTML, buffer, NULL);
				    headerSent = 1;
			   }
               dumpHelp(socket,buffer) ;
               break ;

        default:
			    if(!headerSent) {
				    SendHeader(socket, format, buffer, NULL);
				    headerSent = 1;
			    }

			    sprintf(buffer, "#!Invalid Query '%s'\n", query);
			    SendStrHTTP(socket, buffer);

                query = NULL ;
			    break;
        }
	}

	if(format==DATA_FORMAT_XML)
		SendStrHTTP(socket, "</logfilestats>\n");

	free(buffer);

    // restore original preferences
    memcpy( &MyPrefStruct, &oldMyPrefStruct, CONFIG_SIZE) ;

#if DEF_WINDOWS
    StatusSet("Web Query Processed") ;
#endif
    resetWindowTitle() ;

	processing_http_request = FALSE;
}

