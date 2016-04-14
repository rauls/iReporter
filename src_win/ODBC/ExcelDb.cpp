/* ExcelDb file is a subclass for reading from or writing data to excel database by using ODBC.
	file title display formate: file full path->data source name:table Name	
*/
#include "ConvertDb.h"
#include "ExcelDb.h"
#include "ResDefs.h"	// for ReturnString() etc

ExcelDb::ExcelDb():ConvertDb()
{}

ExcelDb::~ExcelDb()
{}

// provide DSN, driver name and file name to connecte to excel database 
bool ExcelDb::connectToDSN(char* dBName, char* fName, char* userName,char* pwd, bool display)
{
	SQLCHAR  connectString[MAXBUFLEN];
	SQLCHAR  buffer[MAXBUFLEN];  //connection info 
	SWORD    strLength;               //String length
  
	strcpy((char *)connectString,"DSN=" );
	strcat((char *)connectString, dBName);
	strcat((char *)connectString, "; ");
	strcat((char *)connectString, " DRIVER={Microsoft Excel}; ");
	strcat((char *)connectString, "DBQ=");
	strcat((char *)connectString, fName);

	if (!allocateSQLHDBC())	 // Allocate a Connection Handle.
	   return false;
	//make a connection request. 
	returnCode = SQLDriverConnect(hdbc, NULL, connectString, SQL_NTS, buffer,  MAXBUFLEN, &strLength, SQL_DRIVER_NOPROMPT);
	if ( !IsSuccess(1) )
		return false ;  // error return
	SQLGetInfo(hdbc, SQL_DATA_SOURCE_NAME, (SQLPOINTER)dBName, MAXBUFLEN, &strLength);
	setFileInfo((char*)buffer);
	return true;
}

void ExcelDb::getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num)
{
	if (!strstr((const char *)tName, "$"))
	{
		strcpy(tList[num], (const char *)tName);
		num++;
	}
}

// get file name and type: file name=file path\filename.xls. type=.xls
void ExcelDb::setFileInfo(char *type)
{
	char *point, *fName;

	if(*type)
	{
		strcpy(fileType, ".xls");
		point=strstr( type,"DBQ=" );//excel
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

bool ExcelDb::checkFileName()
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
