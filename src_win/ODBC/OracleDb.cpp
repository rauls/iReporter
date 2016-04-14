
/* OracleDb file is a subclass for reading from or writing data to oracle database
   by using ODBC. file title display formate: Oracle DATABAAE=database name->data source name:table Name	
*/
#include "ConvertDb.h"
#include "OracleDb.h"

OracleDb::OracleDb():ConvertDb()
{}

OracleDb::~OracleDb()
{}

int OracleDb::createTable()
{	
	char SQLStatement[4*MAXBUFLEN];
	int i = 0, j;

	strcpy( SQLStatement, "CREATE TABLE " );
	strcat(SQLStatement, tableName);
	strcat(SQLStatement, " (");

	if (strcmp(tableFieldName[i],""))	
	{
			strcat(SQLStatement,tableFieldName[i]);  //date/time
			strcat(SQLStatement," DATE, ");
	}

	
	if (strcmp(tableFieldName[++i],""))		//status
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(25), ");
	}

	if (strcmp(tableFieldName[++i],""))		//clientaddress
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//source
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(255), ");
	}
	
	if (strcmp(tableFieldName[++i],""))		//Rev_domain
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(512), ");
	}
	if (strcmp(tableFieldName[++i],""))		//File
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(512), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Agent
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(512), ");
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
		strcat(SQLStatement," VARCHAR2(512), ");
	}
	if (strcmp(tableFieldName[++i],""))		//cooki
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(512), ");
	}
	if (strcmp(tableFieldName[++i],""))		//User Name
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(255), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Virtual Host
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(512), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Method
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(50), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Type
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(150), ");
	}
	if (strcmp(tableFieldName[++i],""))		//Protocol
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," VARCHAR2(150), ");
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

// fetching all the tables from the table	
bool OracleDb::getTableNameFromODBC(char* dsName, char cList[][MAXBUFLEN],int &num, bool driverConnected)
{
	char buffer[MAXBUFLEN];      // table Name
   
	prepareRecordSet(1, 0); //oracel get table list
	num=0;
	while (readRecord(buffer,0))
	{
		 if(!strstr((const char *)buffer, "$_") && !strstr((const char *)buffer, "HELP")
			&& !strstr((const char *)buffer, "SQLPLUS_PRODUCT_PROFILE"))
		 {
			strcpy(cList[num], (const char *)buffer);
			num++;
		 }
	}
	return true;
	
}

// get file name and file type: file name= Oracle DATABASE=database name, file type=Oracle
void OracleDb::setFileInfo(char *type)
{
	char *point, *fName;

	if(*type)
	{
		strcpy(fileType, "Oracle");
		point=strstr( type,"DBQ=" );
		if (point)
		{
			point += 4;
			fName=point;
			point =strstr(point, ";" );
			*point='\0';
			point++;
			strcpy(fileName, "Oracle DATABASE= ");
			strcat(fileName,fName);
		}
	}
}
