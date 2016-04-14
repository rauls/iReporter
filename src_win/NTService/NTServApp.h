// NTService.h

#include <windows.h>
#include <stdio.h>

/*#ifdef __cplusplus
extern "C" {
#endif*/

#include "ntservice.h"

int InitNTService(int argc, char* argv[], int allowprintf, char *errmsg );
int IsServiceRunning( void );
int IsServiceInstalled( long flag );
void GetEXEPath( char *newPath );

/*#ifdef __cplusplus
}
#endif*/
  
 
 