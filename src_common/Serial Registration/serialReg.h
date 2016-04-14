#ifndef	__SERIALREG_H
#define	__SERIALREG_H

#include "FWA.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "time.h"

// ------------------ DEFINES
#define	kUserInfoDataType		'RVal'
#define	kUserInfoDataID			5000
#define	kUserInfoDataName		"serial.dat"

#ifdef DEF_FULLVERSION
#define	IS_ENT	1
#define	APPLICATION_IRDEMOREG	"Software\\Analysis Software\\iReporter\\Demo45E"
#define	APPLICATION_DEMOREG		"Software\\Analysis Software\\iReporter\\Demo4E"
#else
#define	IS_ENT	0
#define	APPLICATION_IRDEMOREG	"Software\\Analysis Software\\iReporter\\Demo45"
#define	APPLICATION_DEMOREG		"Software\\Analysis Software\\iReporter\\Demo45"
#endif


#define	APPLICATION_CLIENTREG		"Software\\Analysis Software\\iReporter\\Personal"

#ifdef	DEF_FREEVERSION
#define	APPLICATION_REG			APPLICATION_CLIENTREG
#define	APPLICATION_PROREG		APPLICATION_CLIENTREG
#define	APPLICATION_IRPROREG	APPLICATION_CLIENTREG
#define	APPLICATION_IRREG		APPLICATION_CLIENTREG
#else
#define	APPLICATION_IRPROREG	"Software\\Analysis Software\\iReporter\\Serial4E"
#define	APPLICATION_IRREG		"Software\\Analysis Software\\iReporter\\Serial4"
#define	APPLICATION_PROREG		"Software\\Analysis Software\\iReporter\\Serial4E"
#define	APPLICATION_REG			"Software\\Analysis Software\\iReporter\\Serial4"
#endif

#define	APPLICATION_IRPREFSREG "Software\\Analysis Software\\iReporter\\"
#define	APPLICATION_PREFSREG		"Software\\Analysis Software\\iReporter\\"




// ------------------ Structures

 typedef struct{
/* Struct used for the user registration information.
	This is the format of the 'RVal' resource that is saved in
	installation files.
	Pascal strings.. PUKE PUKE
*/
 	char			serialNumber[32];
 	char			userName[256];
 	char			organization[256];
 	char			localHostName[128];
 	long			clusterTotal;
	long			startTime;
	long			customer_id;
 } SerialInfo, *SerialInfoPtr, **SerialInfoH;
 
#define	SERIALINFO_SIZE	sizeof( SerialInfo )




 // ------------------ Prototypes

void ReSaveRegistry( void );

void	EncryptInfo( SerialInfoPtr regDataPtr );
void	DecryptInfo( SerialInfoPtr regDataPtr );

time_t GetExpireDate( char *dateOut );
double GetDaysLeft( void );
short	DoRegistrationProcedure( void );
char	*GetRegisteredUsername( void );
char	*GetRegisteredOrganization( void );
long	GetRegisteredCustomerID( void );
long GetClusterValue( void );
short IsDemoReg( void );
short IsFullReg( void );
short IsDemoTimedout( void );

long GetRegVariable( char *name );
void RestartRegistration( void );
short DoRegistrationAgain( void );
long GetRegisteredCustomerID( void );
SerialInfoH	GetDemoInfo( SerialInfoH Data );

int		VerifySerial_Standard( char * serialNumStr );
int		VerifySerial_Ent( char * serialNumStr );
int		VerifySerial_Firewall( char * serialNumStr );
int		CurrentVersion( void );

int		VerifySerialRelease( char * serialNumStr );
int		VerifySerialDemo( char * serialNumStr );






#ifdef __cplusplus
}
#endif

#endif
