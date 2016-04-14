/*
 * Registry I/O group funcs()
 *
 *
 *
 */
#include <windows.h>
#include <winreg.h>
#include "resource.h"
#include "resdefs.h"	// for ReturnString().

#define	MYREG_APP_NAME		"iReporter"


int MakeLocalRegKey( char *keyName,  HKEY *hKey )
{
	DWORD OpenType;
	int		retCode;

	// Open our registry key (or create it if it
	// doesn't already exist)
	if (retCode=RegCreateKeyEx(HKEY_LOCAL_MACHINE,
       keyName,
	   0,
	   MYREG_APP_NAME,
	   REG_OPTION_NON_VOLATILE,
	   KEY_READ | KEY_WRITE,
	   NULL,
	   hKey,
	   &OpenType) != ERROR_SUCCESS) {
			//char tmpstr[128];
			//wsprintf( tmpstr, "failed to create regkey '%s', retcode=%d", keyName,retCode );
			//MessageBox( NULL, tmpstr, "Error", MB_OK);
			return TRUE;
	}
	return FALSE;
}


int OpenLocalRegKey( char *keyName,  HKEY *hKey )
{
	int		retCode;

	// Open our registry key (or create it if it
	// doesn't already exist)
	if (retCode=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
       keyName,
	   0,
	   KEY_QUERY_VALUE,
	   hKey) != ERROR_SUCCESS) {
			//char tmpstr[128];
			//wsprintf( tmpstr, "failed to open regkey '%s', retcode=%d", keyName,retCode );
			//MessageBox( NULL, tmpstr, "Error", MB_OK);
			return TRUE;
	}
	return FALSE;
}

int MakeUserRegKey( char *keyName,  HKEY *hKey )
{
	DWORD OpenType;
	int		retCode;

	// Open our registry key (or create it if it
	// doesn't already exist)
	if (retCode=RegCreateKeyEx(HKEY_CURRENT_USER,
       keyName,
	   0,
	   MYREG_APP_NAME,
	   REG_OPTION_NON_VOLATILE,
	   KEY_READ | KEY_WRITE,
	   NULL,
	   hKey,
	   &OpenType) != ERROR_SUCCESS) {
			return TRUE;
	}
	return FALSE;
}


int OpenUserRegKey( char *keyName,  HKEY *hKey )
{
	int		retCode;

	// Open our registry key (or create it if it
	// doesn't already exist)
	if (retCode=RegOpenKeyEx(HKEY_CURRENT_USER,
       keyName,
	   0,
	   KEY_QUERY_VALUE,
	   hKey) != ERROR_SUCCESS) {
			return TRUE;
	}
	return FALSE;
}







// SetRegKey ( "Software\\iReporter\\Info", "Licence_Data", &p, pSize );
int SetRegKeyLocalData( char *keyStr, char *valueStr, void *data, long dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
	if ( MakeLocalRegKey( keyStr, &hParamKey ) )
		return TRUE;

	if ( retCode=RegSetValueEx(hParamKey,
        valueStr,
		0,
 		REG_BINARY,
  		(LPBYTE)data,
  		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[128];
		   	wsprintf( (LPTSTR)tmpstr, (LPCTSTR)ReturnString(IDS_REGNOTCREATE), keyStr,valueStr,retCode );
			MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}

// SetRegKeyString ( "Software\\iReporter\\Info", "Company", &company_str, pSize );
int SetRegKeyLocalString( char *keyStr, char *valueStr, void *data, long dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
	if ( MakeLocalRegKey( keyStr, &hParamKey ) )
		return TRUE;

	if ( retCode=RegSetValueEx(hParamKey,
        valueStr,
		0,
 		REG_SZ,
  		(LPBYTE)data,
  		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[256];
		   	wsprintf( (LPTSTR)tmpstr, (LPCTSTR)ReturnString(IDS_REGNOTCREATE), keyStr,valueStr,retCode );
			MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}








// SetRegKey ( "Software\\iReporter\\Info", "Licence_Data", &p, pSize );
int SetRegKeyData( char *keyStr, char *valueStr, void *data, long dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
//	if ( MakeLocalRegKey( keyStr, &hParamKey ) )
		if ( MakeUserRegKey( keyStr, &hParamKey ) ) return TRUE;

	if ( retCode=RegSetValueEx(hParamKey,
        valueStr,
		0,
 		REG_BINARY,
  		(LPBYTE)data,
  		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[128];
		   	wsprintf( (LPTSTR)tmpstr, (LPCTSTR)ReturnString(IDS_REGNOTCREATE), keyStr,valueStr,retCode );
			MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}

// SetRegKeyString ( "Software\\iReporter\\Info", "Company", &company_str, pSize );
int SetRegKeyString( char *keyStr, char *valueStr, void *data, long dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
//	if ( MakeLocalRegKey( keyStr, &hParamKey ) )
		if ( MakeUserRegKey( keyStr, &hParamKey ) ) return TRUE;

	if ( retCode=RegSetValueEx(hParamKey,
        valueStr,
		0,
 		REG_SZ,
  		(LPBYTE)data,
  		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[256];
		   	wsprintf( (LPTSTR)tmpstr, (LPCTSTR)ReturnString(IDS_REGNOTCREATE), keyStr,valueStr,retCode );
			MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}

int SetRegKeyNumber( char *keyStr, char *valueStr, long data, long dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
//	if ( MakeLocalRegKey( keyStr, &hParamKey ) )
		if ( MakeUserRegKey( keyStr, &hParamKey ) ) return TRUE;

	if ( retCode=RegSetValueEx(hParamKey,
        valueStr,
		0,
 		REG_DWORD,
  		(unsigned char*)(&data),
  		dataSize ) != ERROR_SUCCESS) {
		   	/*char tmpstr[256];
		   	wsprintf( (LPTSTR)tmpstr, (LPCTSTR)ReturnString(IDS_REGNOTCREATE), keyStr,valueStr,retCode );
			MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);*/
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}

// GetRegKey ( "Software\\iReporter\\Info", "Licence_Data", &data, &dataSize );
int GetRegKey( char *keyStr, char *valueStr, void *data, unsigned long *dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
	if ( !OpenUserRegKey( keyStr, &hParamKey ) ){
		retCode = RegQueryValueEx( hParamKey,valueStr,NULL,NULL, data,dataSize );
		RegCloseKey( hParamKey );
		if ( retCode == ERROR_SUCCESS ) return FALSE;
	}
	if ( !OpenLocalRegKey( keyStr, &hParamKey ) ){
		retCode = RegQueryValueEx( hParamKey,valueStr,NULL,NULL, data,dataSize );
		RegCloseKey( hParamKey );
		if ( retCode == ERROR_SUCCESS ) return FALSE;
	}
	return TRUE;
}







// SetRegKeyString ( "Software\\iReporter\\Info", "Company", &company_str, pSize );
int SetUserRegKeyString( char *keyStr, char *valueStr, void *data, long dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
	if ( MakeUserRegKey( keyStr, &hParamKey ) ) return TRUE;

	if ( retCode=RegSetValueEx(hParamKey,
        valueStr,
		0,
 		REG_SZ,
  		(LPBYTE)data,
  		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[256];
		   	wsprintf( (LPTSTR)tmpstr, (LPCTSTR)ReturnString(IDS_REGNOTCREATE), keyStr,valueStr,retCode );
			MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}


// GetRegKey ( "Software\\iReporter\\Info", "Licence_Data", &data, &dataSize );
int GetUserRegKey( char *keyStr, char *valueStr, void *data, unsigned long *dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
	if ( OpenUserRegKey( keyStr, &hParamKey ) ) return TRUE;

	if ( retCode=RegQueryValueEx( hParamKey,
		valueStr,
		NULL,
		NULL,
		data,
		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[256];
		   	wsprintf( tmpstr, "Could not open Registry Value '%s' - '%s', retcode=%d", keyStr,valueStr,retCode );
			//MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}


