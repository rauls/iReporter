/* FileMakerDb file is a subclass for reading from or writing data to FileMaker database by using ODBC.
	file title display formate: file full path->data source name:table Name	
*/
#include "ConvertDb.h"
#include "FileMakerDb.h"
#include "ResDefs.h"	// for ReturnString() etc

FileMakerDb::FileMakerDb():ConvertDb()
{}

FileMakerDb::~FileMakerDb()
{}


bool FileMakerDb::findFilePath(char* dsName)
{
	HANDLE h;
	WIN32_FIND_DATA fileInfo;

	h = FindFirstFile(dsName, &fileInfo);

	if(h!=INVALID_HANDLE_VALUE)
	{
		/*DWORD low=fileInfo.nFileSizeLow;
		DWORD high=fileInfo.nFileSizeHigh;*/
		FindClose(h);
	//	return (( high* (MAXDWORD+1)) + low);
		return 1;
	}
	else 
		return 0;
}

// fileMaker table name = data source name withput extention
bool FileMakerDb::getTableNameFromODBC(char* dsName, char cList[][MAXBUFLEN],int &num, bool driverConnected)
{
	char* point;

	strcpy(cList[0],(char*)dsName);  //get table name
	if (point = strstr(cList[0], "."))
		*point='\0';
	setTableName(cList[0]);
	num=1;  // for fileMak, only one table
 	return true; 
}

// get file name and type: file name=file path\filename.mdb. type=.mdb
void FileMakerDb::setFileInfo(char *type)
{
;

	if(*type)
	{		
		strcpy(fileType, ".fp");
		strcpy(fileName,(type+4));
		return;   
	}
}

void FileMakerDb::moreInfo(char* fileName)
{
	char txt[MAXBUFLEN];

	sprintf(txt, "Can not save log file to fileMaker file  '%s'. \r\r Because the file is not opened.", fileName);
	MessageBox(NULL, txt,ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONINFORMATION);
}

