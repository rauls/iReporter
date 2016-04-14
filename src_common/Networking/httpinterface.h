
#ifndef	HTTP_INTERFACE_H
#define	HTTP_INTERFACE_H

#if DEF_UNIX
typedef	int SOCKET;
#define	INVALID_SOCKET	-1
#define	SOCKET_ERROR	-1
#endif

long SendRemappedHtml( long os, char *data, long datalen );
void InitHttpServer( short port );
long SendData( long os, const char *data, long len );
long HandleHttpURL( long os, char *line );
long SendStrHTTP( long os, char *txt );
void ConvertURLtoPlain( char *string );

#endif // HTTP_INTERFACE_H