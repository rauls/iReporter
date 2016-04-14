
/* DB2 file is a subclass for reading from or writeing data to DB2 database
   by using ODBC. file title display formate: DB2 DATABAAE=database name->data source name:table Name	
*/
#include "ConvertDb.h"
#include "DB2.h"

DB2::DB2():ConvertDb()
{}

DB2::~DB2()
{}


void DB2::getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num)
{
	if (!strcmp(typeN, "TABLE"))
	{
		strcpy(tList[num], (const char *)tName);
		num++;
	}
}

int DB2::createTable()
{	
	char SQLStatement[4*MAXBUFLEN];
	int i = 0, j;

	strcpy( SQLStatement, "CREATE TABLE " );
	strcat(SQLStatement, tableName);
	strcat(SQLStatement, " (");

	if (strcmp(tableFieldName[i],""))		// 
	{
			strcat(SQLStatement,tableFieldName[i]);  
			strcat(SQLStatement," TIMESTAMP, ");
	}

	
	if (strcmp(tableFieldName[++i],""))		//status
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(25), ");
	}

	if (strcmp(tableFieldName[++i],""))		//clientaddress
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//source
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	
	if (strcmp(tableFieldName[++i],""))		//Rev_domain
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//File
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Agent
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Bytes
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," INTEGER, ");
	}
	if (strcmp(tableFieldName[++i],""))		//BytesIn
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," INTEGER, ");
	}
	if (strcmp(tableFieldName[++i],""))		//Duration
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," INTEGER, ");  
	}
	if (strcmp(tableFieldName[++i],""))		//Referal
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//cooki
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//User Name
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Virtual Host
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Method
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(50), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Type
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(150), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Protocol
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR(150), ");
	} 
	if (strcmp(tableFieldName[++i],""))		//Port
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," INTEGER)");
	}

	j=strlen(SQLStatement)-2;
	
	if (SQLStatement[j] == ',')
	{
		SQLStatement[j] = ')';
		SQLStatement[j+1] = '\0';
	}
	

	if (!allocateSQLHSTMT()) //Allocate a statement Handle 
		return 0;		// error
		
	if (execSQL(SQLStatement, 1))
	{
		freeSQLHSTMT();
		return 1;
	}
	else 
	{
		freeSQLHSTMT();
		return 0;//error 
	}
	
}



// get file name and file type: file name= Oracle DATABASE=database name, file type=Oracle
void DB2::setFileInfo(char *type)
{
	char *point, *fName;

	if(*type)
	{
		strcpy(fileType, "DB2");
		point=strstr( type,"DBALIAS=" );
		if (point)
		{
			point += 7;
			*point='\0';
			point++;
			fName=point;
			point =strstr(point, ";" );
			*point='\0';
			strcpy(fileName, "DB2 DATABASE= ");
			strcat(fileName,fName);
		}
	}
}
