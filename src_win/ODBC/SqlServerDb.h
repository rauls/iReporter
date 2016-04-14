/* SqlServerDb file is a subclass for reading from or writeing data to SQL Server database
   by using ODBC. file title display formate: SQL Server DATABAAE=database name->data source name:table Name	
*/

#ifndef SQLSERVER_H
#define SQLSERVER_H

#ifdef __cplusplus

class SqlServerDb:public ConvertDb
{
public:
		SqlServerDb();
		virtual ~SqlServerDb();
	
protected:
		virtual void	setFileInfo(char *type);
		virtual void	getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num);

};
#endif
#endif
