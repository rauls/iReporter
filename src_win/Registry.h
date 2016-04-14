#ifndef	__REG_H
#define __REG_H

#ifdef __cplusplus
extern "C"{
#endif

int OpenLocalRegKey( char *keyName,  HKEY *hKey );
int OpenUserRegKey( char *keyName,  HKEY *hKey );

int SetRegKeyLocalData( char *keyStr, char *valueStr, void *data, long dataSize );
int SetRegKeyLocalString( char *keyStr, char *valueStr, void *data, long dataSize );

int SetRegKeyData( char *keyStr, char *valueStr, void *data, long dataSize );
int SetRegKeyString( char *keyStr, char *valueStr, void *data, long dataSize );
int SetRegKeyNumber( char *keyStr, char *valueStr, long data, long dataSize );

int GetRegKey( char *keyStr, char *valueStr, void *data, unsigned long *dataSize );

int SetUserRegKeyString( char *keyStr, char *valueStr, void *data, long dataSize );
int GetUserRegKey( char *keyStr, char *valueStr, void *data, unsigned long *dataSize );

#ifdef __cplusplus
}
#endif

#endif
