/* OracleDb file is a subclass for reading from or writeing data to oracle database
   by using ODBC. file title display formate: Oracle DATABAAE=database name->data source name:table Name	
*/
#ifndef ORACLEDB_H
#define ORACLEDB_H

#ifdef __cplusplus

class OracleDb:public ConvertDb
{
public:
	OracleDb();
	virtual ~OracleDb();

	virtual int		createTable();
	virtual bool	getTableNameFromODBC(char* dsName, char clist[][MAXBUFLEN],int &num, bool driverConnected);

protected:
	virtual void	setFileInfo(char *type);

};
#endif
#endif