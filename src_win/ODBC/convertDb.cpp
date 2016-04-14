/* ConvertDb file is a base class for reading from or writing data to any database by using ODBC.
*/
#include "ConvertDb.h"
#include "config_struct.h"		// for ODBCTABLE_FIELDS
#include "ResDefs.h"			// for ReturnString() etc

// bullshit
extern ConvertDb* theConvertDb;
extern ConvertDb* oraclePtr;
extern ConvertDb* sqlServerPtr;
extern ConvertDb* accessPtr;
extern ConvertDb* excelPtr;
extern ConvertDb* fileMakerPtr;
extern ConvertDb* mysqlPtr;
extern ConvertDb* postgresPtr;
extern ConvertDb* db2Ptr;

// class statics
char			ConvertDb::tableName[MAXBUFLEN];
char			ConvertDb::fileName[MAXBUFLEN];		
char			ConvertDb::fileType[MAXBUFLEN];
unsigned long	ConvertDb::fileLength;
long			ConvertDb::numOfRecord;
long			ConvertDb::tableLength;
UCHAR			ConvertDb::recordData[MAX_COL][MAXDATALEN];   // Results Data Array
SDWORD			ConvertDb::recordDataLength[MAX_COL];     // Results Data Length Array
SWORD			ConvertDb::numOfColums;
SQLHENV			ConvertDb::henv;                   // Environment Handle
SQLHDBC			ConvertDb::hdbc;                     // hdbc
SQLHSTMT		ConvertDb::hstmt;                  // hstmt
SQLRETURN		ConvertDb::returnCode;
int				ConvertDb::numOfColumn= ODBCTABLE_FIELDS ;
char			ConvertDb::tableFieldName[MAXBUFLEN][MAXBUFLEN];
char			ConvertDb::dateStart[MAXBUFLEN];
char			ConvertDb::dateEnd[MAXBUFLEN];

/* odbc dll functions */
HINSTANCE odbc32LibHnd;

MySQLAllocHandleFunc SQLAllocHandle;
MySQLSetEnvAttrFunc SQLSetEnvAttr;
MySQLFreeHandleFunc SQLFreeHandle;

MySQLCloseCursorFunc SQLCloseCursor;
MySQLFreeStmtFunc SQLFreeStmt;
MySQLDataSourcesFunc SQLDataSources;

MySQLConnectFunc SQLConnect;
MySQLDriverConnectFunc SQLDriverConnect;
MySQLGetInfoFunc SQLGetInfo;

MySQLDisconnectFunc SQLDisconnect;
MySQLTablesFunc SQLTables;
MySQLNumResultColsFunc SQLNumResultCols;

MySQLBindColFunc SQLBindCol;
MySQLFetchFunc SQLFetch;
MySQLExecDirectFunc SQLExecDirect;
MySQLGetDiagRecFunc SQLGetDiagRec;
MySQLDescribeColFunc SQLDescribeCol;

bool loadODBCLib()
{
	
	odbc32LibHnd = LoadLibrary("odbc32");
	if(!odbc32LibHnd)
		return false;
	
	SQLAllocHandle = (MySQLAllocHandleFunc) GetProcAddress(odbc32LibHnd, "SQLAllocHandle"); 
	SQLSetEnvAttr = (MySQLSetEnvAttrFunc) GetProcAddress(odbc32LibHnd, "SQLSetEnvAttr"); 
	SQLFreeHandle = (MySQLFreeHandleFunc) GetProcAddress(odbc32LibHnd, "SQLFreeHandle");

	SQLCloseCursor = (MySQLCloseCursorFunc) GetProcAddress(odbc32LibHnd, "SQLCloseCursor"); 
	SQLFreeStmt = (MySQLFreeStmtFunc) GetProcAddress(odbc32LibHnd, "SQLFreeStmt"); 
	SQLDataSources = (MySQLDataSourcesFunc) GetProcAddress(odbc32LibHnd, "SQLDataSources");

	SQLConnect = (MySQLConnectFunc) GetProcAddress(odbc32LibHnd, "SQLConnect"); 
	SQLDriverConnect = (MySQLDriverConnectFunc) GetProcAddress(odbc32LibHnd, "SQLDriverConnect"); 
	SQLGetInfo = (MySQLGetInfoFunc) GetProcAddress(odbc32LibHnd, "SQLGetInfo");

	SQLDisconnect = (MySQLDisconnectFunc) GetProcAddress(odbc32LibHnd, "SQLDisconnect"); 
	SQLTables = (MySQLTablesFunc) GetProcAddress(odbc32LibHnd, "SQLTables"); 
	SQLNumResultCols = (MySQLNumResultColsFunc) GetProcAddress(odbc32LibHnd, "SQLNumResultCols");

	SQLBindCol = (MySQLBindColFunc) GetProcAddress(odbc32LibHnd, "SQLBindCol");
	SQLFetch = (MySQLFetchFunc) GetProcAddress(odbc32LibHnd, "SQLFetch"); 
	SQLExecDirect = (MySQLExecDirectFunc) GetProcAddress(odbc32LibHnd, "SQLExecDirect"); 
	SQLGetDiagRec = (MySQLGetDiagRecFunc) GetProcAddress(odbc32LibHnd, "SQLGetDiagRec");
	SQLDescribeCol = (MySQLDescribeColFunc) GetProcAddress(odbc32LibHnd, "SQLDescribeCol");
	return true;
}

void freeODBCLib()
{
	FreeLibrary(odbc32LibHnd);
}


ConvertDb::ConvertDb()
{
	henv=NULL;
	hdbc=NULL;
	hstmt=NULL;
	
}

ConvertDb::~ConvertDb()
{}

//Allocate an environment handle for ODBC function calls.
bool ConvertDb::initSQLEnvironment()
{   
	if (SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv) != SQL_SUCCESS)
		return false;
	return (SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
                         (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER) == SQL_SUCCESS);
}

//Free the ODBC environment.
bool ConvertDb::freeSQLEnvironment()
{
	return (SQLFreeHandle(SQL_HANDLE_ENV,henv) == SQL_SUCCESS);
}

// Allocate a SQLHDBC.
bool ConvertDb::allocateSQLHDBC()
{
   if ((SQLAllocHandle(SQL_HANDLE_DBC,henv, (SQLHDBC FAR *)&hdbc)) != SQL_SUCCESS) 
   {
      displayError(returnCode, SQL_HANDLE_ENV, henv);
      return false;
   }
   else 
  	   return true;
}

// Allocate a statement Handle 
bool ConvertDb::allocateSQLHSTMT()
{
	if ((SQLAllocHandle(SQL_HANDLE_STMT,hdbc, &hstmt)) != SQL_SUCCESS)
	{
		displayError(returnCode, SQL_HANDLE_DBC, hdbc);
		return false;
	}
	else 
		return true;
}

// free a statement Handle 
void ConvertDb::freeSQLHSTMT()
{
	if (!hstmt)
		return;

	SQLCloseCursor(hstmt); // Close the open result set.
	SQLFreeStmt(hstmt, SQL_UNBIND);  // ferr any bindings
	hstmt=NULL;
}

int ConvertDb::createTable()
{
	char SQLStatement[4*MAXBUFLEN];
	int i = 0, j;

	strcpy( SQLStatement, "CREATE TABLE " );
	strcat(SQLStatement, tableName);
	strcat(SQLStatement, " (");

	if (strcmp(tableFieldName[i],""))		
	{
			strcat(SQLStatement,tableFieldName[i]);  //date/time
			strcat(SQLStatement," DATETIME, ");
	}

	
	if (strcmp(tableFieldName[++i],""))		//status
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}

	if (strcmp(tableFieldName[++i],""))		//clientaddress
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//source
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	
	if (strcmp(tableFieldName[++i],""))		//Rev_domain
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//File
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//Agent
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
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
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//cooki
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//User Name
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//Virtual Host
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//Method
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//Type
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
	}
	if (strcmp(tableFieldName[++i],""))		//Protocol
	{
		strcat(SQLStatement,tableFieldName[i]);
		strcat(SQLStatement," TEXT, ");
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

// fetch all database source. if successful, return true
bool ConvertDb::getDataSourceFromODBC(char cList[][MAXBUFLEN], int &num)
{
   SWORD     direction;                             //fetch direction
   SWORD     dataSourceNameLength;                 
   SWORD     sourceDescriptionLength;              //Driver Description length
   SQLCHAR   dataSourceName[MAXBUFLEN];          //DSN 
   SQLCHAR   sourceDescription[MAXBUFLEN];       //Driver Description 

   // fetch all data source ,until SQL_NO_DATA. 
   num=0;
	for (direction = SQL_FETCH_FIRST;
        (returnCode = SQLDataSources(henv, direction, dataSourceName, MAXBUFLEN,
                   &dataSourceNameLength, sourceDescription, MAXBUFLEN, &sourceDescriptionLength)) 
				   != SQL_NO_DATA && returnCode != SQL_ERROR; direction = SQL_FETCH_NEXT)
	{
		strcpy(cList[num],(const char *)dataSourceName);
		num++;
	}
	if(returnCode == SQL_ERROR)
	{
		displayError(returnCode,SQL_HANDLE_DBC, hdbc);
		return false;
	}
	else 
		return true;
}

// result from a request
bool ConvertDb::IsSuccess(bool display)
{
	if (returnCode != SQL_SUCCESS && returnCode != SQL_SUCCESS_WITH_INFO)
	{
		if (display)
			displayError(returnCode,SQL_HANDLE_DBC, hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
		return 0;
	}
	if (returnCode == SQL_SUCCESS_WITH_INFO)	// display any connection information 
	{
		if (display)
		displayError(returnCode, SQL_HANDLE_DBC, hdbc);
		return 1;
	}

	else
	{
		return 1;
	}
}

//Connect to a DSN. if successful, return true
bool ConvertDb::connectToDSN(char* dBName, char* fName, char* userName,char* pwd, bool display)
{
	if (!allocateSQLHDBC())// Allocate a SQLHDBC. 
	   return false;
	 	
	returnCode = SQLConnect(hdbc,(SQLCHAR *)dBName,SQL_NTS,(SQLCHAR *)userName,SQL_NTS,(SQLCHAR *)pwd,SQL_NTS);
	// if failed to connect, free the allocated hdbc before return
	if ( !IsSuccess(display) )
		return false;  // error return
	else
		return true;

	
}
//display a driver dialog for user creating or selecting data source and table
// if successful, return true.
bool ConvertDb::driverConnectToDSN(HWND hWnd, char* dsn)
{
	SQLCHAR  connectString[]="";
 	SQLCHAR  buffer[MAXBUFLEN], temp[MAXBUFLEN];  //connection info 
	SWORD    strLength;               //String length
	char *point;
	  
	if (!allocateSQLHDBC())	 // Allocate a Connection Handle.
	   return false;
	
	//make a connection request. 
	returnCode = SQLDriverConnect(hdbc, (SQLHWND )hWnd, connectString, 0, buffer,  MAXBUFLEN, &strLength, SQL_DRIVER_COMPLETE_REQUIRED);
	if ( !IsSuccess(1) )
		return false ;  // error return
	strcpy((char *) temp,(const char *)buffer);
	//get DSN  name for the connection.
	SQLGetInfo(hdbc, SQL_DATA_SOURCE_NAME, (SQLPOINTER)dsn, MAXBUFLEN, &strLength);

	if (strstr( (char*)buffer, "DBA="))
		theConvertDb = oraclePtr;
	else if (strstr((char*)buffer, "PROTOCOL"))
		theConvertDb = postgresPtr;
	else if (strstr((char*)buffer,"DATABASE"))
		theConvertDb = sqlServerPtr;	
	else if (strstr((char*)buffer,".xls"))  //skip this if it is an excel  file
		theConvertDb = excelPtr;
	else if (strstr((char*)buffer,".mdb"))
		theConvertDb = accessPtr;
	else if (strstr((char*)buffer, ".fp"))
		theConvertDb = fileMakerPtr;
	else if (strstr((char*)buffer, "DB="))
		theConvertDb = mysqlPtr;
	else if (strstr((char*)buffer, "MODE="))
	{
		theConvertDb = db2Ptr;
		
		if (point=strstr((char*)buffer, "DSN="))
		{
			point +=4;
			strcpy((char *)buffer,point);
			if(point=strstr((char *)buffer, ";")) 
				*point ='\0';
			strcpy( dsn,(char *)buffer);
			//only db2 need get dsn from buffer, rather than from SQLGetInfo
		}
	}

	theConvertDb->setFileInfo((char*)temp);
	return true;

}

void ConvertDb::freeSQLHDBC()
{
	if(!hdbc)
		return;
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
}

// fetch all tables in the database. if successful, return true.
bool ConvertDb::getTableNameFromODBC(char* dsName, char cList[][MAXBUFLEN], int &num, bool driverConnected )
{
	SQLCHAR	catalog[MAXBUFLEN], schema[MAXBUFLEN],tableName[MAXBUFLEN];// result set data 
	SQLCHAR typeName[MAXBUFLEN], remarks[MAXBUFLEN];
	// bytes available to return
	SQLINTEGER catalogBytes, schemaBytes, tableNameBytes, typeNameBytes, remarksBytes;
	SWORD  numOfColums;
	
	if (!allocateSQLHSTMT()) //Allocate a statement Handle 
		return false;		// error	

	returnCode = SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0); // request name of tables

	if (returnCode==SQL_SUCCESS)
	{	
		returnCode = SQLNumResultCols(hstmt, &numOfColums); //binding colums
		SQLBindCol(hstmt, 1, SQL_C_CHAR, catalog, MAXBUFLEN,&catalogBytes);//cat
		SQLBindCol(hstmt, 2, SQL_C_CHAR, schema, MAXBUFLEN, &schemaBytes);// scheme
		SQLBindCol(hstmt, 3, SQL_C_CHAR, tableName, MAXBUFLEN,&tableNameBytes);// name
		SQLBindCol(hstmt, 4, SQL_C_CHAR, typeName, MAXBUFLEN, &typeNameBytes);//type
  		SQLBindCol(hstmt, 5, SQL_C_CHAR, remarks, MAXBUFLEN, &remarksBytes);//remarks
  		num=0;

		while(TRUE) 
		{
			returnCode = SQLFetch(hstmt);
			if (returnCode == SQL_ERROR) 
			{
				 displayError(returnCode, SQL_HANDLE_STMT, hstmt);
				 break;
			}
			if (returnCode == SQL_SUCCESS || returnCode == SQL_SUCCESS_WITH_INFO)
				getTableList(cList, (char *)tableName, (char *)typeName, num);
			else
				break;
		}// while
		freeSQLHSTMT();
		return true;
	}
	else
	{
		moreInfo(dsName);
		freeSQLHSTMT();
		return false;
	}
}

void ConvertDb::setTableName(char* tName)
{
	strcpy(tableName, tName);
}

void ConvertDb::getTableName(char* tName)
{
	strcpy( tName,tableName);
}

// prepare record set for reading the records from the table
void ConvertDb::prepareRecordSet(bool qTable, bool selectDate)
{
	char qStatement[MAXBUFLEN+MAXBUFLEN];		// query

	
	if(qTable)
		strcpy( qStatement, "select table_name from user_tables" );	
	else if (selectDate)
	{
		//if (strcmp(tableFieldName[0],""))	//date/time
	//	{
			strcpy( qStatement, "select * from " );
		//	strcat(qStatement,tableFieldName[0]);
			//strcat( qStatement, " from " );
			strcat(qStatement,tableName);	// select Date/time
		//}
	}
	else
		selectFields(qStatement); //select fields


	
	if (!allocateSQLHSTMT()) // Allocate a statement Handle 
		return;
	for (int i = 0; i < MAX_COL; i++) // initialize column data and column data length arrays
	{
		recordDataLength[i] = 0;
		recordData[i][0] = '\0';
	}
	
	if ( !execSQL(qStatement,1)) 	// execute SQL and process errors 
		return;

	if (!getTotalColumes()) // get number of columns
		return;

	bindingColumns();
	numOfRecord=0; //next for read one record
}

// read a row from a table
bool ConvertDb::readRecord(char dispBuffer[], bool readArecord)
{
	DWORD   tabStop;                     // tab stop 
	
	// fetch a row of the result set.    
	dispBuffer[0]='\0';
	returnCode = SQLFetch(hstmt);
	if (returnCode==SQL_SUCCESS || returnCode==SQL_SUCCESS_WITH_INFO)
	{
		if (returnCode != SQL_SUCCESS)
		{
			displayError(returnCode, SQL_HANDLE_STMT, hstmt);
		}
	
		for(int i=0; i<numOfColums; i++) 
		{
			char *msg;
			 // check if the column is a null value?
			if ( recordDataLength[i] == SQL_NULL_DATA )
				msg = ReturnString( IDS_SQLNULLDATASTRING );
			else
				msg = (char*)recordData[i];
			strcat(dispBuffer, msg );
			tabStop = strlen(dispBuffer);
			dispBuffer[tabStop++] = ',';//each field end with ,
			dispBuffer[tabStop] = '\0';
		
		}
		if (*dispBuffer)
			dispBuffer[strlen(dispBuffer)-1]='\0'; // read  a row in dispBuffer
		numOfRecord++;
		if (readArecord)
			freeSQLHSTMT();  // only read one record, so free handle here
	
		return true;  // can continue;
	}
	else if (returnCode == SQL_ERROR)  // if there is any error returned by fetch display it 
	{
      displayError(returnCode, SQL_HANDLE_STMT, hstmt);
	  freeSQLHSTMT();
		return false; // end of record set	
	}
	else 
	{
		freeSQLHSTMT();
		return false; // end of record set	
	}
   
}

long ConvertDb::getNumOfRecord() const
{
	return numOfRecord;
}

int ConvertDb::getNumberOfColumn() const
{
	return numOfColums;
}

// executin a sql. 
bool ConvertDb::execSQL(const char* sql, bool show)
{
	returnCode = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
	if (returnCode != SQL_SUCCESS)
	{
		if (show)
			displayError(returnCode, SQL_HANDLE_STMT, hstmt);
		freeSQLHSTMT();
	//	MessageBox(NULL, sql, ReturnString( IDS_SQLINFO ), MB_OK | MB_ICONINFORMATION);
		return false;
	}
	else
		return true;
}

bool ConvertDb::getTotalColumes()
{
	char dispBuffer[MAXBUFLEN]; // Display Buffer

	returnCode = SQLNumResultCols(hstmt, &numOfColums); // get number of columns 
	if (numOfColums >= MAX_COL) 
	{
      numOfColums = MAX_COL;
      wsprintf(dispBuffer, ReturnString( IDS_SQLCOLTRUNC_WARNG ), MAX_COL);
      MessageBox(NULL, dispBuffer, ReturnString( IDS_SQLTRUNCERR ), MB_OK | MB_ICONINFORMATION);
	  return true;
	}
   else if (numOfColums == 0) 
   {
	  SQLCloseCursor(hstmt); // Close the open result set.
	  return false;
   }
   else
	   return true;
}

void ConvertDb::bindingColumns()
{
   //  bind column data array and column data length
   for( int i=0/*,buffer[0]='\0'*/; i<numOfColums; i++)
   {
	  SQLBindCol(hstmt, (UWORD)(i+1), SQL_C_CHAR, recordData[i], MAXDATALEN, &recordDataLength[i]);

   }
}


bool ConvertDb::fetchTableLength(long &len, bool show)
{
	char qStatement[MAXBUFLEN];		// query

	strcpy( qStatement, "select * from " );	
   	strcat(qStatement,tableName);	// prepare SQL, table name from user typed
	if (!allocateSQLHSTMT()) // Allocate a statement Handle 
		return false;
	if ( !execSQL(qStatement, show)) 	// execute SQL and process errors 
		return false;

	if (!getTotalColumes()) // get number of columns
		return false;

	bindingColumns();
	setTableLength(len); // get length
	freeSQLHSTMT();
	return true;

 
}

void ConvertDb::setTableLength(long &length)
{
	     
	// fetch each row of the result set.    
   for(int j = 0; (returnCode = SQLFetch(hstmt))==SQL_SUCCESS || returnCode==SQL_SUCCESS_WITH_INFO;) 
   {
      if (returnCode != SQL_SUCCESS)
	  {
         displayError(returnCode, SQL_HANDLE_STMT, hstmt);
	  }
	
      for(int i=0; i<numOfColums; i++) 
		;
	  length++;
	        
   }
   tableLength=length;
   // display error returned by SQLFetch
   if (returnCode == SQL_ERROR)
   {
      displayError(returnCode, SQL_HANDLE_STMT, hstmt);
   }
}

long ConvertDb::getTableLength() const
{
	return tableLength;
}


void ConvertDb::displayError(SQLRETURN result, SWORD handleType, SQLHANDLE handle)
{
   SQLCHAR  errState[SQL_SQLSTATE_SIZE+1];    // SQL Error State string
   SQLCHAR  errMsg[SQL_MAX_MESSAGE_LENGTH+1];    // SQL Error message
   SWORD	msgNumber = 1;
   SWORD	firstRun = true;
   SWORD    errMsgLength;             // Error message length
   SQLINTEGER 		errCode;

   int		dispBufferSize;                // Display Error buffer  size
   char		buffer[SQL_SQLSTATE_SIZE+SQL_MAX_MESSAGE_LENGTH+MAXBUFLEN]; //error info buffer
   char     dispBuffer[MAXDISPLAYSIZE+1]; // Display Buffer
 
   buffer[0] = '\0';
   do	// display all errors 
      {
         // initialize display buffer with the string in error info buffer
         strcpy(dispBuffer, buffer);
         // call SQLGetDiagRec until SQL_NO_DATA. Concatenate all error strings
         while ((returnCode = SQLGetDiagRec(handleType, handle, msgNumber++,
                            errState, &errCode, errMsg,
                            SQL_MAX_MESSAGE_LENGTH-1, &errMsgLength))!= SQL_NO_DATA) 
		 {
			if(returnCode == SQL_ERROR || returnCode == SQL_INVALID_HANDLE)
				break;
            sprintf(buffer, ReturnString(IDS_SQLERR_FORMAT ), (LPSTR)errState, errCode, (LPSTR)errMsg);
            dispBufferSize = strlen(dispBuffer);
            if (dispBufferSize && (dispBufferSize+strlen(buffer)+1) >= MAXDISPLAYSIZE)
               break;
            if (dispBufferSize)
               strcat(dispBuffer, "\n");
            strcat(dispBuffer, buffer);
         }
    
         if (result == SQL_SUCCESS_WITH_INFO)// display message 	
		 {
			 
			 /*if (!strstr(dispBuffer,"[SQL Server]Changed database context to"))// ignor SQL server disply and mysql info 
			 {
				 char* msg;
				 if(firstRun)
					 msg = ReturnString(IDS_SQLWRNMSGTITLE);
				 else
					 msg = ReturnString(IDS_SQLWRNCNTDTITLE);
				 

				MessageBox(NULL,dispBuffer, msg, MB_OK | MB_ICONINFORMATION);
			 }
			 else
				 if(!strstr(dispBuffer,"SQLSetEnvAttr")) ignore info*/
			 
			 
		 }
         else
		 {
			 
			if (*dispBuffer)
			{
				char* msg;
				if (firstRun)
					msg=ReturnString( IDS_SQLERRMSGTITLE );
				else ReturnString( IDS_SQLERRCNTDTITLE );
			
				MessageBox(NULL,dispBuffer, msg, MB_OK | MB_ICONEXCLAMATION);
			}
			 
		 }

         if (firstRun)
            firstRun = FALSE;
      }while (!(returnCode == SQL_NO_DATA || returnCode == SQL_ERROR || returnCode == SQL_INVALID_HANDLE));
}

SQLHSTMT ConvertDb::getHSTMT() const
{
	return hstmt;
}

//if file exists return 1
long ConvertDb::getFileLength()
{
	HANDLE h;
	WIN32_FIND_DATA fileInfo;

	h = FindFirstFile(getFileName(), &fileInfo);

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

void ConvertDb::setFileName(char* name)
{
	
	strcpy(fileName,name);
}


const char* ConvertDb::getFileName() 
{
	return fileName;
}

char* ConvertDb::getFileType() 
{
	return fileType;
}

void ConvertDb::setTableFieldName(const char fieldName[][MAXBUFLEN])  //to get size
{
	for (int i=0; i<numOfColumn; i++)
	{
		strcpy(tableFieldName[i],fieldName[i]);
	}
}

void ConvertDb::getTableFieldName(char fieldName[][MAXBUFLEN])
{
	for (int i=0; i<numOfColumn; i++)
	{
		strcpy(fieldName[i],tableFieldName[i]);
	}
}

void ConvertDb::selectFields(char* statement)
{
	char* mark =", ";
	int i=0;

	strcpy(statement, "SELECT ");

	if (strcmp(tableFieldName[i],""))	//date/time
		strcat(statement,tableFieldName[i]);  
	
	
	if (strcmp(tableFieldName[++i],""))		//status
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}

	if (strcmp(tableFieldName[++i],""))		//clientaddress
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}
	if (strcmp(tableFieldName[++i],""))		//source
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
		
	}
	
	if (strcmp(tableFieldName[++i],""))		//Rev_domain
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}
	if (strcmp(tableFieldName[++i],""))		//File
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}
	if (strcmp(tableFieldName[++i],""))		//Agent
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
		
	}
	if (strcmp(tableFieldName[++i],""))		//Bytes
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}
	if (strcmp(tableFieldName[++i],""))		//BytesIn
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
		
	}
	if (strcmp(tableFieldName[++i],""))		//Duration
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	 
	}
	if (strcmp(tableFieldName[++i],""))		//Referal
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}
	if (strcmp(tableFieldName[++i],""))		//cooki
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}
	if (strcmp(tableFieldName[++i],""))		//User Name
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}
	if (strcmp(tableFieldName[++i],""))		//Virtual Host
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
		
	}
	if (strcmp(tableFieldName[++i],""))		//Method
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
	
	}
	if (strcmp(tableFieldName[++i],""))		//Type
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
		
	}
	if (strcmp(tableFieldName[++i],""))		//Protocol
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
		
	} 
	if (strcmp(tableFieldName[++i],""))		//Port
	{
		strcat(statement,mark); 
		strcat(statement,tableFieldName[i]);
		
	}

	strcat(statement," FROM ");
	strcat(statement,tableName);

/*	if (strlen(tableFieldName[0]))  //speed up process if has date field
	{
		strcat(statement," WHERE ");
		strcat(statement,tableFieldName[0]);
		strcat(statement," >=#");
		strcat(statement, dateStart);
		strcat(statement, "# AND ");
		strcat(statement, tableFieldName[0]);
		strcat(statement," <=#");
		strcat(statement, dateEnd);
		strcat(statement, " 23:59:59 PM#");
	} mysql does not support it*/

}
void ConvertDb::setDateRange(char* date1, char* date2)
{
	strcpy(dateStart,date1);
	strcpy(dateEnd,date2);
}




