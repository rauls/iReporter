/* ODBCInterface file is an interface used in winmain 
   for saving log file to or reading from any databse.  
*/

#include <sys/stat.h>

#include "ConvertDb.h"
#include "ODBCInterface.h"
#include "OracleDb.h"
#include "SqlServerDb.h"
#include "AccessDb.h"
#include "ExcelDb.h"
#include "FileMakerDb.h"
#include "MysqlDb.h"
#include "PostgresDb.h"
#include "DB2.h"
#include "ResDefs.h"	// for ReturnString() etc


ConvertDb* theConvertDb;
ConvertDb* oraclePtr;
ConvertDb* sqlServerPtr;
ConvertDb* accessPtr;
ConvertDb* excelPtr;
ConvertDb* fileMakerPtr;
ConvertDb* basePtr;
ConvertDb* mysqlPtr;
ConvertDb* postgresPtr;
ConvertDb* db2Ptr;


int SQL_loadLib()
{
	return loadODBCLib();
}

void SQL_FreeODBCLib()
{
	freeODBCLib();
}
// initializing SQLEnvironment and create database objects
int SQL_initSQLEnvironment()
{
	basePtr = new ConvertDb();
	theConvertDb = basePtr;
	oraclePtr = new OracleDb();
	sqlServerPtr = new SqlServerDb();
	accessPtr = new AccessDb();
	excelPtr = new ExcelDb();
	fileMakerPtr = new FileMakerDb();
	mysqlPtr = new MysqlDb();
	postgresPtr = new PostgresDb();
	db2Ptr = new DB2();


	if (!theConvertDb->initSQLEnvironment())
	{
		MessageBox(NULL,ReturnString( IDS_SQLINITERROR ), ReturnString( IDS_SQLEXECERROR ), MB_OK|MB_ICONHAND);
		return false;
	}
	else return true;
}

void SQL_freeSQLEnvironment()
{
	delete basePtr;
	delete oraclePtr;
	delete sqlServerPtr;
	delete accessPtr;
	delete excelPtr;
	delete fileMakerPtr;

	theConvertDb->freeSQLEnvironment();
}

void SQL_freeSQLHDBC()
{
	theConvertDb->freeSQLHDBC();
}

int SQL_allocateSQLHSTMT()
{
	return theConvertDb->allocateSQLHSTMT();
}

void SQL_freeSQLHSTMT()
{
	theConvertDb->freeSQLHSTMT();
}

int SQL_SaveDataSource( char dsList[][MAXBUFLEN], int* number )
{
	char tempList[MAXBUFLEN][MAXBUFLEN];
	int num=0;
	
	if (!theConvertDb->getDataSourceFromODBC(tempList, num))
		return false;

	*number=0;
	for(int i=0; i<num; i++)
	{
		if (strstr(tempList[i], ".fp"))
		{
			strcpy(dsList[*number], tempList[i]);
			*number=*number+1;
		}
	}
	return true;
}

int SQL_dataSourceFile(char sList[][MAXBUFLEN], int* num, char* dsName)
{
	bool found = false;

	for(int i=0; i<*num; i++)
	{
		if (!strcmp(sList[i], dsName))
		{
			found=true;
			break;
		}
	}
	return found;
}

int SQL_getFileLocation(HWND hWnd, char* fileFullName, char* fileTitle, char* dlgTitle )
{
	OPENFILENAME    fStruct;
	char	 dir[MAXBUFLEN];


	GetCurrentDirectory( MAXBUFLEN, dir );
	char* filter="FileMaker Files (*.fp5)\0*.fp5\0\0";

	ZeroMemory( &fStruct, sizeof(OPENFILENAME));
    fStruct.lStructSize = sizeof(OPENFILENAME);
    fStruct.hwndOwner = hWnd;
    fStruct.lpstrFilter = filter;
    fStruct.lpstrCustomFilter = (LPSTR) NULL;
    fStruct.nMaxCustFilter = NULL;
    fStruct.nFilterIndex = 1;
    fStruct.lpstrFile = fileFullName;
    fStruct.nMaxFile = 511;
    fStruct.lpstrFileTitle = fileTitle; //receives the file name and extension 
    fStruct.nMaxFileTitle = sizeof(fileTitle);
    fStruct.lpstrInitialDir = dir;
    fStruct.lpstrTitle = dlgTitle;
    fStruct.Flags = NULL;
    fStruct.nFileOffset = 0;
    fStruct.nFileExtension = 0;
    fStruct.lpstrDefExt = "fp5";
	GetSaveFileName(&fStruct);
	return true;
}

int SQL_ValidateLocation(const char* path)
{
	struct	stat	pathStat;
	long	err;
	char cpyPath[MAXBUFLEN];

	//getPath(fullpath, path );
	strcpy(cpyPath, path);
	err = strlen( cpyPath );
	if ( err )
	{
		if ( cpyPath[err-1] == '\\' )
			cpyPath[err-1] = 0;
	}
	else
		return 0;
//	strcpy(fullpath, path);
	err = stat( cpyPath, &pathStat ); // 0 is valid
	return (long)err;
}

int SQL_UseCurrentDir(char* dsName)
{
	char location[MAXBUFLEN], newFile[MAXBUFLEN], defaultFile[MAXBUFLEN], dispBuff[MAXBUFLEN];
	bool ret;

	GetCurrentDirectory( MAXBUFLEN, location );
	strcpy(defaultFile, "C:\\");
	strcat(defaultFile, "\\default.fp5");// default.fp5 in the current dir.
	strcpy(newFile,location);
	strcat(newFile, "\\");
	strcat(newFile, dsName);
	if (CopyFile(defaultFile, newFile, FALSE))
	{
		sprintf(dispBuff, "FileMaker file '%s' has been created. \r\rPlease open the file if it is not opened.", newFile);
		MessageBox(NULL, dispBuff, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONINFORMATION);
		ret=true;
	}
	else 
	{
		sprintf(dispBuff, "Failed to create FileMaker file '%s'.", newFile);
		MessageBox(NULL, dispBuff, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONSTOP);
		ret=false;
	}
	
	return ret;
}

int SQL_UseSelectedDir(char* path, char* dsName)
{
	char newFile[MAXBUFLEN], defaultFile[MAXBUFLEN], dispBuff[MAXBUFLEN];
	bool ret;

	strcpy(defaultFile, "C:\\");
	strcat(defaultFile, "\\default.fp5");// default.fp5 in the dir.
	if (!strcmp(path, ""))
	{
		GetCurrentDirectory( MAXBUFLEN, path);
		strcat(path,"\\");
	}
	
	strcpy(newFile,path);
	strcat(newFile, dsName);
	if (CopyFile(defaultFile, newFile, FALSE))
	{
		sprintf(dispBuff, "FileMaker file '%s' has been created. \r\rPlease open the file if it is not opened.", newFile);
		MessageBox(NULL, dispBuff, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONINFORMATION);
		ret=true;
	}
	else 
	{
		sprintf(dispBuff, "Failed to create FileMaker file '%s'.", newFile);
		MessageBox(NULL, dispBuff, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONSTOP);
		ret=false;
	}

	return ret;
}

// display a dialog for user creating or select database source and table
// if connecting to database successful  return true, otherwise false
int displayDatabase(HWND hWnd, char tableList[][MAXBUFLEN], int* number, char *dsName, int isLogFileProcess)
{
	int connectedDSN=0;	// not any DSN connected

	connectedDSN=theConvertDb->driverConnectToDSN(hWnd,dsName);
	if (connectedDSN) //CONNECT TO DSN successfully
	{
		int num;
		if(theConvertDb->getTableNameFromODBC(dsName, tableList, num, 1))
		{
			*number=num;
			return true;  // to  add string to combo list
		}
	}
	return false;

}


// connect to a database, not prompt dialog. if successful, return true.
int SQL_connectToDatabase(char* dBName, char* userName, char* pwd,bool showmessage)  //process db from list
{
	char *temp1, *temp2, *temp3, *temp0;
	char  fileName[MAXBUFLEN];

	temp0=temp1=temp2=dBName;
	while(*temp1)  //data source name and table name
	{
		
		if (*temp1=='-' && *(temp1+1)=='>')
		{
			temp2=temp1+2;
			*temp1='\0'; 
			strcpy(fileName,temp0);
			break;
		}
		temp1++;
	}
	temp3=temp2;
	while(*temp2)  //data source name and table name
	{
		if (*temp2==':' && *(temp2+1)==':')
		{
			temp1=temp2+2;
			*temp2='\0';
			break;
		}
		temp2++;
	}
	strcpy(dBName, temp3);
	if (strstr(fileName,"Oracle"))
			theConvertDb = oraclePtr;
	else if (strstr(fileName,"SQL Server"))
		theConvertDb = sqlServerPtr;
	else if (strstr(fileName,".xls"))  //excel can not use connect function
		theConvertDb = excelPtr;
	else if (strstr(fileName,".mdb"))
		theConvertDb = accessPtr;
	else if (strstr(fileName,".fp"))
		theConvertDb = fileMakerPtr;
	else if (strstr(fileName,"MySQL"))
		theConvertDb = mysqlPtr;
	else if (strstr(fileName,"PostgreSQL"))
		theConvertDb = postgresPtr;
	else if (strstr(fileName,"DB2"))
		theConvertDb = db2Ptr;
		

	if (theConvertDb->connectToDSN(dBName, fileName, userName,pwd,showmessage))
	{		
		theConvertDb->setTableName(temp1);
		return true;
	}
	else 
		return false;
	
}


void SQL_setTableName( char* Name)
{
	theConvertDb->setTableName(Name);
}

void SQL_getTableName( char* Name )
{
	theConvertDb->getTableName(Name);
}

long SQL_totalTableLength( bool show )
{
	long ret = 0;
	theConvertDb->fetchTableLength(ret,show);
	return ret;
}

long SQL_tableLength( void ) 
{
	return theConvertDb->getTableLength();
}

void SQL_prepareRecordSet(bool selectDate)
{
	 theConvertDb->prepareRecordSet(0, selectDate);
}

long SQL_getNumOfRecord( void )
{
	return theConvertDb->getNumOfRecord();
}

// reading a record from table
int SQL_readOneRecord( char *buffer, bool firstRecord)
{
	return theConvertDb->readRecord(buffer,firstRecord);
}

// executing any sql. if successful, return true.
int SQL_Database(const char* str1)
{
	if (theConvertDb->execSQL(str1,1)) 	// execute SQL and process errors
		return true ; // to add string to combo list*/
	else 
	{
		//	MessageBox(NULL, str1, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONINFORMATION);
		return false;
	}
}

//if creating a table successful, return true.

int SQL_createTable()
{
	return theConvertDb->createTable();
}

int SQL_getNumberOfColumn()
{
	return theConvertDb->getNumberOfColumn();
}

const char* SQL_getFileName()
{
	return theConvertDb->getFileName();
}

char* SQL_getFileType()
{
	return theConvertDb->getFileType();
}

// validating database, data source and table whether exists. if successful, return true.		
int SQL_validateDatabase(char* fileFullName)
{
	char dsList[MAXBUFLEN][MAXBUFLEN];
	char tbList[MAXBUFLEN][MAXBUFLEN];
	char userName[MAXBUFLEN];
	char password[MAXBUFLEN];
	char dispBuffer[MAXBUFLEN];
	char *temp1,*temp2,*dsName,*fileName,*tbName;
	char temp3[MAXBUFLEN];
	int num;
	bool found=false;
	long len;

	userName[0]='\0';
	password[0]='\0';

	strcpy(temp3,fileFullName);
	temp1=temp2=fileName=temp3;
	while(*temp1)  //seperate file name, data source name and table name
	{
		if (*temp1=='-' && *(temp1+1)=='>')
		{
			temp2=temp1+2;
			*temp1='\0';
			break;
		}
		temp1++;
	}
	
	dsName=temp2;
	while(*temp2)
	{
		if (*temp2==':' && *(temp2+1)==':')
		{
			tbName=temp2+2;
			*temp2='\0';	
			break;
		}
		temp2++;
	}
	if (strstr(fileName,"Oracle"))
			theConvertDb = oraclePtr;
	else if (strstr(fileName,"SQL Server"))
		theConvertDb = sqlServerPtr;
	else if (strstr(fileName,".xls"))  //only xls and mdb need check
		theConvertDb = excelPtr;
	else if (strstr(fileName,".mdb"))
		theConvertDb = accessPtr;
	else if (strstr(fileName,".fp"))
		theConvertDb = fileMakerPtr;
	else if (strstr(fileName,"MySQL"))
		theConvertDb = mysqlPtr;
	else if (strstr(fileName,"PostgreSQL"))
		theConvertDb = postgresPtr;
	else if (strstr(fileName,"DB2"))
		theConvertDb = db2Ptr;

	theConvertDb->setFileName(fileName);
	if (!theConvertDb->checkFileName())
		return false;
	theConvertDb->getDataSourceFromODBC(dsList, num);
	for(int i=0; i<num; i++)
	{
		if(!strcmp(dsList[i],dsName))
		{
			found=true;
			break;
		}
	}
	if (found) //find data source
	{
		if(!theConvertDb->connectToDSN(dsName,fileName,userName,password, 0))
			return false;  // connect to datasource failed
			
		if (!theConvertDb->getTableNameFromODBC(dsName, tbList, num, 0)) //to find table
			return false;

		found=false;
		for(int i=0; i<num; i++)
		{
			if (!strcmp(tbList[i],tbName))
			{
				theConvertDb->setTableName(tbName);
				found=true;
				break;
			}
		}
		
		len=0;
		if (!theConvertDb->fetchTableLength(len,0)) 
		{
		//	theConvertDb->freeSQLHDBC(); // filemaker need open file to connect
			return false;  
		}
			//theConvertDb->freeSQLHDBC(); // close the connection after validating db
		if (found)
			return true;
		else
		{
			sprintf(dispBuffer,"The table  '%s'  in database '%s\n ' does not exist", tbName, fileName);
			MessageBox(NULL, dispBuffer, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONINFORMATION);
			return false;  // failed  to find table
		}
	}
	else 
	{
		sprintf(dispBuffer,"The data source '%s' does not exist", dsName);
		MessageBox(NULL,dispBuffer, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONINFORMATION);
		return false;  // failed  to find ds
	}

}

void SQL_setFileName(char* name)
{
	theConvertDb->setFileName(name);
}

void SQL_setTableFieldName(const char fieldName[][256])
{
	theConvertDb->setTableFieldName(fieldName);
}

void	SQL_getTableFieldName(char fieldName[][256])
{
	theConvertDb->getTableFieldName(fieldName);
}

void SQL_setDateRange(char* dateStart, char* dateEnd)
{
	theConvertDb->setDateRange(dateStart, dateEnd);
}


