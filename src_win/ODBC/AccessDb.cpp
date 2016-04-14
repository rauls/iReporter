/* AccessDb file is a subclass for reading from or writing data to access database by using ODBC.
	file title display formate: file full path->data source name:table Name	
*/
#include "ConvertDb.h"
#include "AccessDb.h"
#include "ResDefs.h"	// for ReturnString() etc

AccessDb::AccessDb():ConvertDb()
{}

AccessDb::~AccessDb()
{}

void AccessDb::getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num)
{
	if (!strstr((const char *)typeN, "SYSTEM TABLE"))
	{
		if(!strstr(tName,  "~TMPCLP"))// only has this table after deleting all tables
		{
			strcpy(tList[num], (const char *)tName);
			num++;
		}
	}
}

// get file name and type: file name=file path\filename.mdb. type=.mdb
void AccessDb::setFileInfo(char *type)
{
	char *point, *fName;
	if(*type)
	{
		strcpy(fileType, ".mdb");
		point=strstr( type,"DBQ=" );// access 
		if (point)
		{
			point += 4;
			fName=point;
			point =strstr(point, ";" );
			*point='\0';
			point++;
			strcpy(fileName,fName);
		}
	}
}

// check whether the file  exists. if exists, return true
bool AccessDb::checkFileName()
{
	char dispBuffer[MAXBUFLEN];

	if (!getFileLength())
	{
		sprintf(dispBuffer,"The database '%s' does not exist", fileName);	
		MessageBox(NULL, dispBuffer, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONINFORMATION);
		return false;   //file not exists
	}
	return true;
}
