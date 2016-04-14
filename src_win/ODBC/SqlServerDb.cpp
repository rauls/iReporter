/* SqlServerDb file is a subclass for reading from or writing data to SQL Server database
   by using ODBC. file title display formate: SQL Server DATABAAE=database name->data source name:table Name	
*/

#include "ConvertDb.h"
#include "SqlServerDb.h"

SqlServerDb::SqlServerDb():ConvertDb()
{}

SqlServerDb::~SqlServerDb()
{}

void SqlServerDb::getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num)
{
	if (!strstr((const char *)typeN, "SYSTEM TABLE") && !strstr((const char *)typeN, "VIEW")
						&& !strstr((const char *)tName, "dtproperties"))
	{
		strcpy(tList[num], (const char *)tName);
		num++;
	}
}

// get file name and file type: file name= SQL Server DATABASE=database name, file type=SQL Server
void SqlServerDb::setFileInfo(char *type)
{
	char *point, *fName;

	if(*type)
	{
		if(point = strstr( type, "DATABASE"))  // SQL server
		{
			fName=point;
			point =strstr(point, ";" );
			*point='\0';
			point++;
			strcpy(fileName, "SQL Server ");
			strcat(fileName,fName);
			strcpy(fileType, "SQL Server");
		}
	}
}
