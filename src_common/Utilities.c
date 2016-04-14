
#include <ctype.h>
#include "myansi.h"
#include "Utilities.h"


#define	TWOVARS(a,b)	( (a<<16) | (b) )


/*
 * Routine to see if a text string is matched by a wildcard pattern.
 * Returns 1 if the text is matched, or 0 if it is not matched
 * or if the pattern is invalid.
 *  *		matches zero or more characters
 *  ?		matches a single character
 *  [abc]	matches 'a', 'b' or 'c'
 *  \c		quotes character c
 *  Adapted from code written by Ingo Wilken.
 */

long wildcardmatch( const char* text, const char* pattern, short nocase )     
{
	const char* const origtext=text;
	const char* firstchar=text;

	const char*	retrypat=0;
	const char*	retrytxt=0;

	short found=0, dots=0;
	int	ch, tc;

	while (*text || *pattern)
	{
		ch = *pattern++;
		if ( nocase ) ch=mytoupper(ch);

		switch (ch)
		{
			case '*':  
				retrypat = pattern;
				retrytxt = text;
				break;

			case '(':
			case '#':
				if ( dots == 3 )
				{			// "203.111.20.#128/28
					long range,start,ip;
					char *p;

					found = 0;
					if( p = mystrchr( pattern, '/' ) ){
						start = myatoi( pattern );
						range = myatoi( p+1 );
						range = 1<<(32-range);
						ip = myatoi( text );
						if ( (ip >= start) && (ip<= start+range) )
							found=1;
					} else
					if( p = mystrchr( pattern, '-' ) ){
						start = myatoi( pattern );
						range = myatoi( p+1 );
						ip = myatoi( text );
						if ( (ip >= start) && (ip<= range) )
							found=1;
					}
					return found;
				}
				break;

			case '[':
				found = 0;
				while ((ch = *pattern++) != ']') {
					if (ch == '\\')
						ch = *pattern++;
					if (ch == '\0')
						return 0;
					tc = *text;

					if ( nocase ) { ch=mytoupper(ch); tc=mytoupper(tc); }

					if (tc == ch)
						found = 1;
				}
				if (!found)
				{
					if( (pattern=retrypat)==0 )
					{
						return 0;
					}
					text = ++retrytxt;
				}
				/* fall into next case */

			case '?':  
				if (*text++ == '\0')
					return 0;
				break;

			case '\\':  
				ch = *pattern++;
				if (ch == '\0')
					return 0;
				/* fall into next case */

			default:
				if ( ch == '.' ) ++dots;
				tc = *text;
				if ( nocase ) tc=mytoupper(tc);

				// EQUAL
				if (tc == ch) {
					if (firstchar==0) firstchar=text;
					if (*text)
						++text;
					break;
				}
				// NOT FOUND CHAR
				if (*text)
				{
					if( (pattern=retrypat)==0 )
					{
						return 0;
					}
					firstchar=0;
					text = ++retrytxt;
					break;
				}
				return 0;
		}
	}

	tc = (firstchar - origtext);
	ch = (text - origtext);
	return TWOVARS(tc,ch);
}

long match( const char *text, const char *pattern, short nocase )
{
	long result, reverse = 0;

	if ( *pattern == '!' ){
		 reverse = 1;
		 pattern++;
	}
	result = wildcardmatch( text, pattern , nocase );

	if ( reverse ){
		if( result ) return 0;
		else return 1;
	} else
		return result;
}

void CleanStringOfBadChars( char *string )
{
	register char c, c2, *p = string;
	while( (c=*string++) ){
		if ( c == '%' ){
			c = *string++;
			c = mytoupper(c);
			if ( c>= 'A' ) c -= 'A'-10;
			else c -= '0';

			c2 = *string++;
			c2 = mytoupper(c2);
			if ( c2>= 'A' ) c2 -= 'A'-10;
			else c2 -= '0';
			c = c<<4 | c2;
		}
		if ( c == '*' ||
			 c == '?' ||
			 c == '<' ||
			 c == '>' ||
			 c == '|' ||
			 c == '\"' ||
			 c == '#' ||
			 c <0 ) c = '_';

		*p++ = c;
	}
}



// for a given url with lots of %20s convert them to appropriate characters so we can see them
// and also if the string is TOO LONG, insert spaces after slashes to force browsers to wrap.
char *CleanURLString( const char *string )
{
static char l_string[1024];
	register char c, c2, *p = l_string;
	int		out=0;

	while( (c=*string++) && (out<1024) )
	{
		if ( c == '%' )
		{
			c = *string++;
			c = mytoupper(c);
			if ( c>= 'A' ) c -= 'A'-10;
			else c -= '0';

			c2 = *string++;
			c2 = mytoupper(c2);
			if ( c2>= 'A' ) c2 -= 'A'-10;
			else c2 -= '0';
			c = c<<4 | c2;
		}
		if ( c == '/' && out>128 )
		{
			*p++ = c;
			c = ' ';
			out+=1;
		}
		*p++ = c;
		out++;
	}
	*p++ = 0;
	return l_string;
}

