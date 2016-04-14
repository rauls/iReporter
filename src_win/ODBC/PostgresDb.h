/* PostgresDb file is a subclass for reading from or writing data to Postgres database by using ODBC.
	file title display formate: Postgres DATABASE=database name->data source name:table Name	
*/
#ifndef POSTGRESDB_H
#define POSTGRESDB_H

#ifdef __cplusplus

class PostgresDb:public ConvertDb
{
public:
		PostgresDb();
		virtual ~PostgresDb();

		//virtual bool checkFileName();

protected:
		virtual void setFileInfo(char *type);
		virtual void getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num);

};
#endif
#endif