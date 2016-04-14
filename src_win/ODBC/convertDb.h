//ConvertDb.h file
/* ConvertDb file is a base class for reading from or writing data to any database by using ODBC
*/
#ifndef CONVERTDB_H
#define CONVERTDB_H

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <fstream.h>
#include <string>
#include "resource.h"
#include "sqltypes.h"
#define RC_INVOKED  //exclude odbc function in the lib
#include "sql.h"
#include "sqlext.h"


/* odbc dll functions */
typedef SQLRETURN (SQL_API *MySQLAllocHandleFunc)(SQLSMALLINT,SQLHANDLE,SQLHANDLE*);
typedef SQLRETURN (SQL_API *MySQLSetEnvAttrFunc)(SQLHENV,SQLINTEGER,SQLPOINTER,SQLINTEGER);
typedef SQLRETURN (SQL_API *MySQLFreeHandleFunc)(SQLSMALLINT, SQLHANDLE);
typedef SQLRETURN (SQL_API *MySQLCloseCursorFunc)(SQLHSTMT);
typedef SQLRETURN (SQL_API *MySQLFreeStmtFunc)(SQLHSTMT,SQLUSMALLINT);
typedef SQLRETURN (SQL_API *MySQLDataSourcesFunc)(SQLHENV,SQLUSMALLINT,SQLCHAR*,
		SQLSMALLINT, SQLSMALLINT*,SQLCHAR*, SQLSMALLINT,SQLSMALLINT*);
typedef SQLRETURN (SQL_API *MySQLConnectFunc)(SQLHDBC,SQLCHAR*,SQLSMALLINT, 
					SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT );
typedef SQLRETURN (SQL_API *MySQLDriverConnectFunc)(SQLHDBC,SQLHWND,SQLCHAR*,SQLSMALLINT,
					SQLCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLUSMALLINT);
typedef SQLRETURN (SQL_API *MySQLGetInfoFunc)(SQLHDBC,SQLUSMALLINT, SQLPOINTER,
					SQLSMALLINT, SQLSMALLINT*);
typedef SQLRETURN (SQL_API *MySQLDisconnectFunc)(SQLHDBC);
typedef SQLRETURN (SQL_API *MySQLTablesFunc)(SQLHSTMT,SQLCHAR*, SQLSMALLINT,
					SQLCHAR*, SQLSMALLINT,SQLCHAR*,SQLSMALLINT, SQLCHAR*, SQLSMALLINT );
typedef SQLRETURN (SQL_API *MySQLNumResultColsFunc)(SQLHSTMT,SQLSMALLINT*);

typedef SQLRETURN (SQL_API *MySQLBindColFunc)(SQLHSTMT,SQLUSMALLINT,SQLSMALLINT,
					SQLPOINTER, SQLLEN, SQLLEN*);
typedef SQLRETURN (SQL_API *MySQLFetchFunc)(SQLHSTMT);
typedef SQLRETURN (SQL_API *MySQLExecDirectFunc)(SQLHSTMT,SQLCHAR*, SQLINTEGER);
typedef SQLRETURN (SQL_API *MySQLGetDiagRecFunc)(SQLSMALLINT, SQLHANDLE, SQLSMALLINT, SQLCHAR*,
					SQLINTEGER*, SQLCHAR*, SQLSMALLINT , SQLSMALLINT*);
typedef SQLRETURN (SQL_API *MySQLDescribeColFunc)(SQLHSTMT, SQLSMALLINT, SQLCHAR*, SQLSMALLINT,
					SQLSMALLINT*, SQLSMALLINT*, SQLUINTEGER* , SQLSMALLINT*,SQLSMALLINT*);


extern MySQLAllocHandleFunc SQLAllocHandle;
extern MySQLSetEnvAttrFunc SQLSetEnvAttr;
extern MySQLFreeHandleFunc SQLFreeHandle;

extern MySQLCloseCursorFunc SQLCloseCursor;
extern MySQLFreeStmtFunc SQLFreeStmt;
extern MySQLDataSourcesFunc SQLDataSources;

extern MySQLConnectFunc SQLConnect;
extern MySQLDriverConnectFunc SQLDriverConnect;
extern MySQLGetInfoFunc SQLGetInfo;

extern MySQLDisconnectFunc SQLDisconnect;
extern MySQLTablesFunc SQLTables;
extern MySQLNumResultColsFunc SQLNumResultCols;

extern MySQLBindColFunc SQLBindCol;
extern MySQLFetchFunc SQLFetch;
extern MySQLExecDirectFunc SQLExecDirect;
extern MySQLGetDiagRecFunc SQLGetDiagRec;
extern MySQLDescribeColFunc SQLDescribeCol;

bool loadODBCLib();
void freeODBCLib();


//Constants 
#define MAXBUFLEN           256         //display buffer size
#define MAX_COL             100          //maximum columns in result set

#define MAXDATALEN          300          //maximum data length per column
#define MAXDISPLAYSIZE      MAX_COL*(MAXDATALEN+1)


#ifdef __cplusplus

class ConvertDb
{
public:

	ConvertDb();
	virtual ~ConvertDb();
	bool	initSQLEnvironment();
	bool	allocateSQLHSTMT();
	bool	freeSQLEnvironment();
	void	freeSQLHDBC();
	void	freeSQLHSTMT();
	void	setTableName(char* tName);
	void	getTableName(char* tName);
	bool	fetchTableLength(long &len, bool show);
	void	prepareRecordSet(bool qTable, bool selectDate);
	long	getNumOfRecord() const;
	bool	readRecord(char dispBuffer[], bool readArecord);

	int		getNumberOfColumn() const;
	long    getTableLength() const;
	SQLHSTMT getHSTMT() const;
	void	displayError(SQLRETURN nResult, SWORD fHandleType, SQLHANDLE handle);
	bool	execSQL(const char* sql, bool show);
	void	setFileName(char* name);
	const char*	getFileName(); 
	char*	getFileType() ;
	long	getFileLength();
	bool	driverConnectToDSN(HWND hWnd,char* dsn);
	bool	getDataSourceFromODBC(char clist[][MAXBUFLEN], int &num);
	virtual bool	connectToDSN(char* dBName, char* fName, char* userName,char* pwd, bool display);
	virtual bool	getTableNameFromODBC(char* dsName, char clist[][MAXBUFLEN],int &num, bool driverConnected);
	virtual int		createTable();
	virtual bool	checkFileName(){return true;};
	void			setTableFieldName(const char fieldName[][MAXBUFLEN]);
	void			getTableFieldName(char fieldName[][MAXBUFLEN]);
	void			setDateRange(char* date1, char* date2);
	
protected:
	bool	allocateSQLHDBC();
	bool	IsSuccess(bool display);
	void	bindingColumns();
	void	setTableLength(long &length);
	bool	getTotalColumes();

	virtual void getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num){};
	virtual void setFileInfo(char* type){};
	virtual void moreInfo(char* fileName){};


	static char	tableName[MAXBUFLEN];
	static char	fileName[MAXBUFLEN];		
	static char	fileType[MAXBUFLEN];
	static unsigned long  fileLength;
 	static long	numOfRecord;
	static long	tableLength;

	static UCHAR   recordData[MAX_COL][MAXDATALEN];   // Results Data Array
	static SDWORD  recordDataLength[MAX_COL];     // Results Data Length Array
	static SWORD	numOfColums;
	static SQLHENV     henv;                   // Environment Handle
	static SQLHDBC		hdbc;                     // hdbc
	static SQLHSTMT    hstmt;                  // hstmt
	static SQLRETURN  returnCode;				//

	static  char tableFieldName[MAXBUFLEN][MAXBUFLEN];
	static  int numOfColumn;
	static	char dateStart[MAXBUFLEN], dateEnd[MAXBUFLEN];


private:
	void selectFields(char* statement);
	
	
	
	
};

#endif

#endif