/* PostgresDb file is a subclass for reading from or writing data to Postgres database by using ODBC.
	file title display formate: Postgres DATABASE=database name->data source name:table Name	
*/
#include "ConvertDb.h"
#include "PostgresDb.h"

PostgresDb::PostgresDb():ConvertDb()
{}

PostgresDb::~PostgresDb()
{}

void PostgresDb::getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num)
{
		
	strcpy(tList[num], (const char *)tName);
	num++;
		

}

// get file name and type: file name=database name. type=Postgres
void PostgresDb::setFileInfo(char *type)
{
	char *point, *fName;

	if(*type)
	{
		strcpy(fileType, "PostgreSQL");
		point=strstr( type,"DATABASE=" );
		if (point)
		{
			point += 9;
			fName=point;
			point =strstr(point, ";" );
			*point='\0';
			point++;
			strcpy(fileName, "PostgreSQL DATABASE= ");
			strcat(fileName,fName);
		}
	}
}


