/*
 * Registry I/O group funcs()
 *
 *
 *
 */
#include <windows.h>
#include <winreg.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "MacEmu.h"
#include "ice.h"
#include "reg.h"

#define	MYREG_APP_NAME		"iReporter"

#define	APPLICATION_REG		"Software\\iReporter Software\\iReporter\\Serial"



int MakeRegKey( char *keyName,  HKEY *hKey )
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
			char tmpstr[128];
			wsprintf( tmpstr, "failed to create regkey '%s', retcode=%d", keyName,retCode );
			MessageBox( NULL, tmpstr, "Error", MB_OK);
			return TRUE;
	}
	return FALSE;
}


int OpenRegKey( char *keyName,  HKEY *hKey )
{
	DWORD OpenType;
	int		retCode;

	// Open our registry key (or create it if it
	// doesn't already exist)
	if (retCode=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
       keyName,
	   0,
	   KEY_QUERY_VALUE,
	   hKey) != ERROR_SUCCESS) {
			char tmpstr[128];
			wsprintf( tmpstr, "failed to open regkey '%s', retcode=%d", keyName,retCode );
			//MessageBox( NULL, tmpstr, "Error", MB_OK);
			return TRUE;
	}
	return FALSE;
}


int GetRegKey( char *keyStr, char *valueStr, void *data, unsigned long *dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
	if ( OpenRegKey( keyStr, &hParamKey ) ) return TRUE;

	if ( retCode=RegQueryValueEx( hParamKey,
		valueStr,
		NULL,
		NULL,
		data,
		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[128];
		   	wsprintf( tmpstr, "Could not open Registry Value '%s' - '%s', retcode=%d", keyStr,valueStr,retCode );
			//MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}



int SetRegKeyData( char *keyStr, char *valueStr, void *data, long dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
	if ( MakeRegKey( keyStr, &hParamKey ) ) return TRUE;

	if ( retCode=RegSetValueEx(hParamKey,
        valueStr,
		0,
 		REG_BINARY,
  		(LPBYTE)data,
  		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[128];
		   	wsprintf( tmpstr, "Could not create Registry Value '%s' - '%s', retcode=%d", keyStr,valueStr,retCode );
			MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}

int SetRegKeyString( char *keyStr, char *valueStr, void *data, long dataSize )
{
	HKEY hParamKey;
	int		retCode;
	
	if ( MakeRegKey( keyStr, &hParamKey ) ) return TRUE;

	if ( retCode=RegSetValueEx(hParamKey,
        valueStr,
		0,
 		REG_SZ,
  		(LPBYTE)data,
  		dataSize ) != ERROR_SUCCESS) {
		   	char tmpstr[128];
		   	wsprintf( tmpstr, "Could not create Registry Value '%s' - '%s', retcode=%d", keyStr,valueStr,retCode );
			MessageBox( NULL, tmpstr, MYREG_APP_NAME, MB_OK);
			return TRUE;
	}

	RegCloseKey(hParamKey);

	return FALSE;
}



/*********************** GetUserInfo **********************************/

SerialInfoH	GetSerialInfo( void )
{
static	SerialInfo		readData;
static	SerialInfoPtr	readDataP= &readData;
	Boolean			ok;
	OSErr			err = 0;
 	HANDLE			hFile;
	unsigned long	len=SERIALINFO_SIZE;

	if ( GetRegKey ( APPLICATION_REG, "Licence_Data", &readData, &len ) ){
		ok = FALSE;
	} else
		ok = TRUE;
	
	if ( ok ){
		char txtStr[100];
		
		//wsprintf( txtStr, "datasize=%d\n", len );
		//ErrorMsg( txtStr );

		// DECRYPT DATA JUST AFTER LOADING!!!
		DecryptInfo( readDataP );

		//ErrorMsg( "Decrypt Done." );
		//ErrorMsg( GetRegisteredUsername() );
		return &readDataP;
	} else
		return NULL;
}


OSErr	SaveToFile(SerialInfoH userInfo, char *name)
{
	int			ok;
	OSErr		err = 0;
 	HANDLE          	hFile;
	unsigned long		written;

	SetRegKeyString( APPLICATION_REG, "Name", GetRegisteredUsername(), strlen(GetRegisteredUsername()) );
	SetRegKeyString( APPLICATION_REG, "Company", GetRegisteredOrganization(), strlen(GetRegisteredOrganization()) );

	// ENCRYPT DATA BEFORE SAVING!!!!!!
	#if DATA_ENCRYPTED
	if (userInfo)
		EncryptInfo(*userInfo);
	#endif

	if ( SetRegKeyData( APPLICATION_REG, "Licence_Data", *userInfo, SERIALINFO_SIZE ) ){
		if ((hFile = CreateFile( "ireporter.ser", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL , NULL)) == (HANDLE)-1) {
			ErrorMsg( "cant write serial"); return -1;
		}
		ok = WriteFile(  hFile, *userInfo, SERIALINFO_SIZE,  &written, NULL );
		CloseHandle(hFile);
	} else ok = TRUE;

	return 0;
}
















