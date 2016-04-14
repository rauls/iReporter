/* MysqlDb file is a subclass for reading from or writing data to Mysql database by using ODBC.
	file title display formate: Mysql DATABASE=database name->data source name:table Name	
*/
#include "ConvertDb.h"
#include "MysqlDb.h"

MysqlDb::MysqlDb():ConvertDb()
{}

MysqlDb::~MysqlDb()
{}

void MysqlDb::getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num)
{
		
	strcpy(tList[num], (const char *)tName);
	num++;
		

}

// get file name and type: file name=database name. type=mysql
void MysqlDb::setFileInfo(char *type)
{
	char *point, *fName;

	if(*type)
	{
		strcpy(fileType, "MySQL");
		point=strstr( type,"DB=" );
		if (point)
		{
			point += 3;
			fName=point;
			point =strstr(point, ";" );
			*point='\0';
			point++;
			strcpy(fileName, "MySQL DATABASE= ");
			strcat(fileName,fName);
		}
	}
}


