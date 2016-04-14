


#ifndef	UTILITES_H
#define	UTILITES_H

#ifdef __cplusplus
extern "C" {
#endif


//long wildcardmatch( char *text, char *pattern, short nocase ); // No need to declare this function for external use as it isn't used externally.
long match( const char *text, const char *pattern, short nocase );

void CleanStringOfBadChars( char *string );
char *CleanURLString( const char *string );

#ifdef __cplusplus
}
#endif
  
#endif // UTILITES_H
