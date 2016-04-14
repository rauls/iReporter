#ifndef	_UNIXREG_
#define	_UNIXREG_

#ifdef __cplusplus
extern "C" {
#endif

#include "serialReg.h"

SerialInfoH	GetSerialInfoData( SerialInfoH Data, char *regStr );

int	SaveSerialData(SerialInfoH userInfo, char *name, char *regStr);


time_t GetRegDate( void );
time_t  GetFileDate( char *file );





#ifdef __cplusplus
}
#endif
  

#endif









