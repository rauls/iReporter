
#ifndef FILE_TYPES_H
#define FILE_TYPES_H

void MakeSafeURL( char *url );
void MakeURL( char *out, const char *site, const char *path );
char *IsURL( const char *filename );
const char *IsShortCut( const char *filename );
const char *IsURLShortCut( const char *filename );
const char *IsGZIP( const char *filename );
const char *IsBZIP( const char *filename );
const char *IsPKZIP( const char *filename );
long IsCompressed( char *filename );
long IsFileaFolder( const char *file );
void ExtractUserFromURL( char *url, char *server, char *username, char *passwd, char *file );
void MakeURLfromDetails( char *url, char *site, char *name, char *pass, char *path );

#endif // FILE_TYPES_H
