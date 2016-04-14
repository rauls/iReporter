/* FileMakerDb file is a subclass for reading from or writing data to access database by using ODBC.
	file title display formate: file full path->data source name:table Name	
*/
#ifndef FILEMAKERDB_H
#define FILEMAKERDB_H

#ifdef __cplusplus

class FileMakerDb:public ConvertDb
{
public:
		FileMakerDb();
		virtual ~FileMakerDb();
	
		virtual bool getTableNameFromODBC(char* dsName, char cList[][MAXBUFLEN],int &num, bool driverConnected);
	
protected:
		virtual void setFileInfo(char *type);
		virtual void moreInfo(char* fileName);
private:
		bool findFilePath(char* dsName);
};
#endif
#endif