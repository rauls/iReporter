
#ifndef REPORTFUNCS_H
#define REPORTFUNCS_H

long WriteToStorage( const char *source, void *fp );
long Fwrite( FILE *fp, const char *source );
int WriteLine( FILE *fp, const char *txt );
void Fclose( FILE *fp );

#endif // REPORTFUNCS_H
