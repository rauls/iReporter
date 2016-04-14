/* ExcelDb file is a subclass for reading from or writing data to excel database by using ODBC.
	file title display formate: file full path->data source name:table Name	
*/
#ifndef EXCELDB_H
#define EXCELDB_H

#ifdef __cplusplus

class ExcelDb:public ConvertDb
{
public:
		ExcelDb();
		virtual ~ExcelDb();

		virtual bool connectToDSN(char* dBName, char* fName, char* userName,char* pwd, bool display);
		virtual bool checkFileName();

protected:
		virtual void getTableList(char tList[][MAXBUFLEN], char* tName, char* typeN, int &num);
		virtual void setFileInfo(char *type);

};
#endif
#endif