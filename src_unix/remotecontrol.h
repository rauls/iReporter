#ifndef	_REMOTECON_
#define	_REMOTECON_

#ifdef __cplusplus
extern "C" {
#endif

long GetHostIP( void );
long GetPeerIP( void );
long InitFWServer( long port );
void CloseFWServer( void );

void RemoteStatusSetText( char *message );
void DoDaemonMode( void );


#ifdef __cplusplus
}
#endif
  

#endif






