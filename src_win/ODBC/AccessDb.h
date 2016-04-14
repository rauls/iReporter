/* AccessDb file is a subclass for reading from or writing data to access database by using ODBC.
	file title display formate: file full path->data source name:table Name	
*/
#ifndef ACCESSDB_H
#define ACCESSDB_H

#ifdef __cplusplus

class AccessDb:public ConvertDb
{
public:
		AccessDb();
		virtual ~AccessDb();

		virtual bool checkFileName();

protected:
		virtual void setFileInfo(char *type);
		virtual void getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num);

};
#endif
#endif