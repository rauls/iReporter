/* DB2 file is a subclass for reading from or writeing data to DB2 database
   by using ODBC. file title display formate: DB2 DATABAAE=database name->data source name:table Name	
*/
#ifndef DB2_H
#define DB2_H

#ifdef __cplusplus

class DB2:public ConvertDb
{
public:
	DB2();
	virtual ~DB2();

	virtual int		createTable();
	

protected:
	virtual void	getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num);
	virtual void	setFileInfo(char *type);

};
#endif
#endif