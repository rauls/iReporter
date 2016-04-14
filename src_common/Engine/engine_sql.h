#ifndef	_ENGINESQL_
#define	_ENGINESQL_


void Init_ODBCSystem( void );
VDinfoP GetODBCRecord( VDinfoP VDptr, char **fsFile, int logNum, HitDataRec *Line, long *logError );
void WriteLinetoODBC( VDinfoP VDptr, HitDataRec *Line, char buff1);
long SQL_GetIndex( void );

#endif
