
/* ODBCInterface file is an interface used in winmain 
   for saving log file to or reading from any databse  
*/

#ifndef ODBCInterface_H
#define ODBCInterface_H


#ifdef __cplusplus
extern "C" {
#endif

#include "myansi.h"

	
// Note: to make things work in every OS, dont use 'true', use 'TRUE'
void	SQL_FreeODBCLib();
int		SQL_loadLib();
int		SQL_initSQLEnvironment();
int		SQL_allocateSQLHSTMT();
void	SQL_freeSQLEnvironment();
void	SQL_freeSQLHDBC();
void	SQL_freeSQLHSTMT();
int		SQL_SaveDataSource( char dsList[][256], int* number );
int  	SQL_dataSourceFile(char sList[][256], int* num, char* dsName);
int		SQL_getFileLocation(HWND hWnd, char* fileFullName, char* fileTitle, char* dlgTitle );
int		SQL_ValidateLocation(const char* path);
int 	SQL_UseSelectedDir(char* path, char* dsName);
int		SQL_UseCurrentDir(char* dsName);
int		displayDatabase(HWND hWnd, char tableList[][256],int* number, char *dsName, int isLogFileProcess); //used in winmain.c engine.cpp
void	SQL_setTableName( char* Name);
void	SQL_getTableName( char* Name );
long	SQL_totalTableLength( bool show); //used in engine.cpp
long	SQL_tableLength( void );
void	SQL_prepareRecordSet(bool selectDate);
long	SQL_getNumOfRecord( void );
int		SQL_readOneRecord( char *buffer, bool firstRecord);
int		SQL_Database(const char* str1);
int		SQL_createTable();

int		SQL_getNumberOfColumn();
const char*	SQL_getFileName();
void	SQL_setFileName(char* name);
char*	SQL_getFileType();
int		SQL_validateDatabase(char* fileFullName);
int		SQL_connectToDatabase(char* dBName, char* userName, char* pwd,bool showmessage);
void	SQL_setTableFieldName(const char fieldName[][256]);
void	SQL_getTableFieldName(char fieldName[][256]);
void	SQL_setDateRange(char* dateStart, char* dateEnd);

#ifdef __cplusplus
}
#endif

#endif
