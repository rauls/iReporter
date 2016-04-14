
#ifndef	__SERIALREG_H
#define	__SERIALREG_H

#ifdef __cplusplus
extern "C"{
#endif

 typedef struct{
/* Struct used for the user registration information.
	This is the format of the 'RVal' resource that is saved in
	installation files.
	Pascal strings.. PUKE PUKE
*/
 	char			serialNumber[32];
 	char			userName[256];
 	char			organization[256];
 	char			localUNAME[128];
 	char			localUSERNAME[32];
 	long			startTime;
 	long			localIP;
 } SerialInfo, *SerialInfoPtr, **SerialInfoH;
 
#define	SERIALINFO_SIZE	sizeof( SerialInfo )
 

short	DoRegistrationProcedure( void );
char	*GetRegisteredUsername( void );
char	*GetRegisteredOrganization( void );

void	DecryptInfo( SerialInfoPtr regDataPtr );
void	EncryptInfo( SerialInfoPtr regDataPtr );
int		VerifySerialNumFmt( char * serialNumStr );

#ifdef __cplusplus
}
#endif

#endif
 











