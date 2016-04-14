
#include "Compiler.h"
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include "datetime.h"

#include "myansi.h"
#include "log_io.h"
#include "config_struct.h"
#include "EngineParse.h"
#include "engine_proc.h"

#include "config.h"

#include "EngineVirtualDomain.h"

extern __int64		gFilesize;
extern  short		logStyle;

extern long allTotalInDataSize;
extern int	endall,stopall;
extern long logIdx;
extern long logfiles_opened;


#if DEF_WINDOWS

// ######################################
// ##
// ##
// ##   SQL Area for Windows only
// ##

#include "ODBCInterface.h"

VDinfoP WinGetODBCRecord( VDinfoP VDptr, char **fsFile, int logNum, HitDataRec *Line, long *logError )
{
	static char sql_buff[3000];

	int			LogFileStat = FALSE;

	if ( VDptr )
		LogFileStat = SQL_readOneRecord(sql_buff,0);

	if ( !LogFileStat ){
		// reached end of log, now try 'next' log or end
		if ( VDptr ){
			allTotalInDataSize += SQL_getNumOfRecord();			// make total file size = total read size so far

			if ( MyPrefStruct.multidomains > 0 ) {
				if ( VDptr ) VDptr->totalInDataSize = SQL_getNumOfRecord();			// make total file size = total read size so far
			} else {
				if ( VDptr ) VDptr->totalInDataSize += SQL_getNumOfRecord();			// make total file size = total read size so far
			}
		
		
			if ( VDptr ){
				VDptr->time2 = timems() - VDptr->time1;
				if ( VDptr->time2 == 0 ) VDptr->time2=1;
			}
		
			logIdx++;
		}
		if ( logIdx < logNum ) {
			char buf2[256], dateStart[256], dateEnd[256];

			gFilesize = SQL_totalTableLength(1);
			//DaysDateToString(gDate1, dateStart, GetDateStyle(), '/', 1,1);
			//DaysDateToString(gDate2, dateEnd, GetDateStyle(), '/', 1,1);
			SQL_setDateRange(dateStart, dateEnd);

			SQL_prepareRecordSet(0); // select fields
			logType	= LOGFORMAT_READFROMLINE; 
					
			logfiles_opened++;

			if ( MyPrefStruct.multidomains > 0 ) {
				if ( MyPrefStruct.live_sleeptype > 0){
					VDnum++;
					VDptr = VD[VDnum];
					VDptr->time1 = timems();
				} else {
					if ( VDnum < MyPrefStruct.multidomains ){
						VDinfoP newvd;
						if ( VDptr )
							VDnum++;
						CorrectVirtualName( fsFile[0], buf2, 1 );
						newvd = InitVirtualDomain( VDnum, buf2, logStyle );		/* initialize virt domains info/data */
						if ( VDptr ){
							VDptr->next = newvd;
							VDptr = newvd;
						} else
							VDptr = newvd;

						if ( !VDptr ){
							stopall = 1;
							*logError = -1; return VDptr;
						}
					}
				}
			} else
			if ( !VDptr ) {

				CorrectVirtualName( fsFile[0], buf2, 1 );
				strcpy(buf2,fsFile[0]);
		
				VDptr = InitVirtualDomain( VDnum, buf2, logStyle );
				if ( !VDptr ){
					stopall = 1;
					*logError = -1; return VDptr;
				}
			}

			MemClr( Line, sizeof( HitDataRec ) );

		} else {
			// no more log files....go away and write out the report then exit
			endall = TRUE;
			*logError = -1;
		}
	}
	*logError = ProcessLogLine( sql_buff, Line );
	return VDptr;
}


void WinWriteLinetoODBC( VDinfoP VDptr, HitDataRec *Line, char buff1)
{
	char fieldName[256][256];
	std::string day, month,temp, year;
	int monthPos, dayPos;
	int i=0, j;
	
	if ( Line ) {
		if (buff1 !='\0' && buff1 !='#' && buff1 !='!' && Line->date ) {
			static const std::string  quote="'", comma=", ", space=" ",quotespace="', " ;
			static const std::string  dateformat="'YYYY-MM-DD HH24:MI:SS'), ";
			static const std::string  timeformat="'HH24:MI:SS'), ";
			std::string table,field,values;
			std::string ftype;
			char longBytes[10];
			char tName[256];

			SQL_getTableName(tName);
			table=tName;
			SQL_getTableFieldName(fieldName);
			
			ftype=SQL_getFileType();
			//test
		//	ftype="Oracle";

			field="INSERT INTO "+table + "(";
			if (ftype != "Oracle")
				values="VALUES(";
			else
				values="VALUES(TO_DATE(";


			if (strcmp(fieldName[i],""))
			{
				field +=fieldName[i]+comma;  //date/time

				if (Line->date)	  //FWB internal date:mm/dd/yyyy. database need dd/mm/yyyyy
				{
					temp=Line->date;
					
					
					if (monthPos=temp.find_first_of('/'))
						month=temp.substr(0,monthPos);
					
					if (dayPos=temp.find_first_of('/',monthPos+1))
					{
						day=temp.substr(monthPos+1,dayPos-(monthPos+1));
						
						int i=temp.size();
						year=temp.substr((dayPos+1),temp.size()-(dayPos+1));
					}
				
				//	if (ftype == "MySQL" || ftype == "SQL Server")
						values +=quote + year+"-"+month+"-"+day + space;  // yyyy-mm-dd
				//	else 
					//	values +=quote + day+"/"+month+"/"+year + space;  // dd/mm/yyyy
					if (Line->time)
					{
						if (ftype != "Oracle")
							values += Line->time + quotespace;
						else
							values += Line->time + quotespace+dateformat;


					}
				}
				else if (Line->time)
				{
					if (ftype != "Oracle")
						values += Line->time + quotespace;
					else
						values += Line->time + quotespace+timeformat;

				}
				else 
					values += "NULL, ";
			}

			
			if (strcmp(fieldName[++i],""))		//state
			{
				field += fieldName[i]+comma;
				if (!Line->stat)
					values += "NULL, ";
				else
					values += quote+Line->stat + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//client
			{
				field += fieldName[i]+comma;
				if (!Line->clientaddr)
					values +=  "NULL, ";
				else
					values += quote+Line->clientaddr + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//source
			{
				field += fieldName[i]+comma;
				if (!Line->sourceaddr)
					values += "NULL, ";
				else
					values += quote+Line->sourceaddr + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//REV_domain
			{
				field += fieldName[i]+comma;
				if (!Line->rev_domain)
					values += "NULL, ";
				else
					values +=quote+Line->rev_domain + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//file
			{
				field += fieldName[i]+comma;
				if (!Line->file)
					values += "NULL, ";
				else
					values += quote+Line->file + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//agent
			{
				field += fieldName[i]+comma;
				if (!Line->agent)
					values += "NULL, ";
				else
					values += quote+Line->agent + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//Bytes
			{
				field += fieldName[i]+comma;
				if (!Line->bytes)
					values += "NULL, ";
				else
				{

					sprintf(longBytes,"%10d",Line->bytes);
					if (ftype != "DB2")
						values +=quote+longBytes + quotespace;
					else
						values +=longBytes + comma;

				}
					
			}
			if (strcmp(fieldName[++i],""))		//BytesIn
			{
				field += fieldName[i]+comma;
				if (!Line->bytesIn)
					values += "NULL, ";
				else
				{
					sprintf(longBytes,"%10d",Line->bytesIn);
					if (ftype != "DB2")
						values += quote+longBytes + quotespace; 
					else
						values +=longBytes + comma;

				}
					
			}
			if (strcmp(fieldName[++i],""))		//Duration
			{
				field += fieldName[i]+comma;
				if (!Line->ms)
					values += "NULL, ";
				else
				{
					sprintf(longBytes,"%10d",Line->ms);
					if (ftype != "DB2")
						values += quote+longBytes + quotespace; 
					else
						values += longBytes + comma; 
				}
					
			}
			if (strcmp(fieldName[++i],""))		//referal
			{
				field +=fieldName[i]+comma;
				if (!Line->refer)
					values += "NULL, ";
				else
					values += quote+Line->refer + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//cookie
			{
				field += fieldName[i]+comma;
				if (!Line->cookie)
					values += "NULL, ";
				else
					values +=quote+Line->cookie + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//username
			{
				field += fieldName[i]+comma;
				if (!Line->user)
					values += "NULL, ";
				else
					values += quote+Line->user + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//virtual host
			{
				field +=fieldName[i]+comma;
				if (!Line->vhost)
					values += "NULL, ";
				else
					values += quote+Line->vhost + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//method
			{
				field += fieldName[i]+comma;
				if (!Line->method)
					values += "NULL, ";
				else
					values += quote+Line->method + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//type
			{
				field += fieldName[i]+comma;
				if (!Line->type)
					values += "NULL, ";
				else
					values += quote+Line->type + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//protocol
			{
				field += fieldName[i]+comma;
				if (!Line->protocol)
					values += "NULL, ";
				else
					values += quote+Line->protocol + quotespace; 
			}
			if (strcmp(fieldName[++i],""))		//Port
			{
				field += fieldName[i]+std::string( ") ");
			
				if (!Line->port)
					values += "NULL)"  ;  //last item in table
				else
				{
					sprintf(longBytes,"%10d",Line->port);
					if (ftype != "DB2")
					{
						values += quote+longBytes + "')";
					}
					else
					{
						values += longBytes ;
						values +=")";
					}
				}
			}

			j=field.size();

			if ( (char)field[j-2]== ',')
			{
				field[j-2] =')';
				field[j-1] =' ';
			}
				
			j=values.size();

			if ( (char)values[j-2]== ',') //", "
			{
				values[j-2] =')';
				values[j-1] ='\0';
			}
				
			field = field + values;
			SQL_Database(field.c_str()); 	// execute SQL 
			
		}
	}

}

#endif



// SQL odbc wrappers so that code compiles in unix/mac
void Init_ODBCSystem( void )
{
#if DEF_WINDOWS
	SQL_allocateSQLHSTMT();
#endif
}

VDinfoP GetODBCRecord( VDinfoP VDptr, char **fsFile, int logNum, HitDataRec *Line, long *logError )
{
#if DEF_WINDOWS
	return WinGetODBCRecord( VDptr, fsFile, logNum, Line, logError );
#else
	return 0;
#endif
}

void WriteLinetoODBC( VDinfoP VDptr, HitDataRec *Line, char buff1 )
{
#if DEF_WINDOWS
	WinWriteLinetoODBC( VDptr, Line, buff1 );
#endif
}


long SQL_GetIndex( void )
{
#if DEF_WINDOWS
	return SQL_getNumOfRecord();
#else
	return 0;
#endif
}