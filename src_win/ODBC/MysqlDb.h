/* MysqlDb file is a subclass for reading from or writing data to Mysql database by using ODBC.
	file title display formate: Mysql DATABASE=database name->data source name:table Name	
*/
#ifndef MYSQLDB_H
#define MYSQLDB_H

#ifdef __cplusplus

class MysqlDb:public ConvertDb
{
public:
		MysqlDb();
		virtual ~MysqlDb();

		//virtual bool checkFileName();

protected:
		virtual void setFileInfo(char *type);
		virtual void getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num);

};
#endif
#endif