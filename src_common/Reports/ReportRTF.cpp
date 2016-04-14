/*
##
##      ##
##      ##
##      ##  ##
##      #########
##          ##
##          ##
##          ##
##
##		V4 Analyzer -	Write module, this outputs all html/rtf/, and spits out calls to make
##						Doc/Excel/PDF to other modules, but basically handles all the reports.
##
##
##
####################################################*/ 
#include "FWA.h"

#include <string>
#define NO_INLINE
#include <list>
#include <map>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "datetime.h"

#include "myansi.h"
#include "config_struct.h"

#include "gd.h"
#include "Report.h"
#include "ReportFuncs.h"
#include "editpath.h"
#include "translate.h"
#include "ReportRTF.h"
#include "Registration.h"
#include "StandardPlot.h"	// for GraphWidth() & GraphHeight()
#include "webpalette.h"
#include "FileTypes.h"


#if DEF_WINDOWS
	#include "postproc.h"
	#include "Winmain.h"
#endif				
#if DEF_UNIX		// UNIX include
	#include "main.h"
	#include "postproc.h"
#endif
#ifdef DEF_MAC
	#include "post_comp.h"
	#include "MacStatus.h"
	#include "SerialRegMac.h"
#endif


extern "C" struct App_config MyPrefStruct;

typedef struct {
	long	right_margin;
	long	left_margin;
} rtf_settings, *rtf_settingsPtr;

#define	PAPER_WIDTH		12240
#define	LMARGIN			1200
#define	RMARGIN			11640
#define	ACTIVEWIDTH		(RMARGIN - LMARGIN)

static char rtf_head[10240] = { 0 };

static char rtf_defaulct[] = { 	"\\red0\\green0\\blue0;" };

static char *rtf_foot = "{ \\par \\pagebb{\\b \\par }}       }}\n\n";

static char *rtf_lastfoot = "{ \\par \\pagebb {\\b \\par }} \\par {\\comment ! THE END !}\n}}\n";

/*
static unsigned char rtfpalette[]={	
	0,0,0,
	255,0,0,
	0,128,0,
	0,0,255,

	255,255,0,
	255,0,255,
	128,0,128,
	128,0,0,

	0,255,0
	255,0,255,
	128,0,128,
	128,0,0 };
*/

static unsigned char rtfcolors[256*3];


rtfFile::rtfFile() : baseFile()
{
	m_style = FORMAT_RTF;
	fileExtension = RTF_EXT;
	multiLinedCell = 0;
	sessionWriter = new CQRTFSessionWriter( fileExtension, *this );
}

rtfFile::~rtfFile()
{
}

long InitRTFHead( void )
{
	long r,g,b, lp, index=0;
	char *p;

	p = rtf_head;
	p += sprintf( p, "{\\rtf1\\ansi\\deff0\\deftab720" );
	p += sprintf( p, "{\\fonttbl{\\f0\\fnil\\charset0 Arial;}}" );
	p += sprintf( p, "\\paperw%d\\paperh15840\\margl%d\\margr600\\margt840\\margb840\n", PAPER_WIDTH, LMARGIN );


	p += sprintf( p, "{\\colortbl" );

	p += sprintf( p, rtf_defaulct );

	rtfcolors[index++] = 0;
	rtfcolors[index++] = 0;
	rtfcolors[index++] = 0;

	for( lp=0;lp<216;lp++){
		r = webpalette[ (lp*3) + 0];
		g = webpalette[ (lp*3) + 1];
		b = webpalette[ (lp*3) + 2];

		rtfcolors[index++] = r;
		rtfcolors[index++] = g;
		rtfcolors[index++] = b;
		p += sprintf( p, "\\red%d\\green%d\\blue%d;", r, g, b );
		if ( lp && lp%6 == 0 )
			p += sprintf( p, "\n" );
	}
	r=g=b=0;
	for( lp=0;lp<16;lp++){
		rtfcolors[index++] = r;
		rtfcolors[index++] = g;
		rtfcolors[index++] = b;

		p += sprintf( p, "\\red%d\\green%d\\blue%d;", r, g, b );
		if ( lp && lp%6 == 0 )
			p += sprintf( p, "\n" );
		r+=16; g+=16; b+=16;
	}
	p += sprintf( p, "}\n{\\comment !END_OF_COLOR_TABLE!}\n" );
	return p-rtf_head;
}



#define ABS(a,b) ( (a>b) ? a-b : b-a )
#define	SQR(x) sqrt(x)

/*


    DistanceSquared := SQR(R1-R2) + SQR(G1-G2) + SQR(B1-B2);
    IF   DistanceSquared < SmallestDistanceSquared
    THEN BEGIN
      RESULT := ColorList[i];
      SmallestDistanceSquared := DistanceSquared
    

  */


long ColourDistance( unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2 )
{
  long r,g,b;
  long rmean;

  rmean = ( (int)r1 + (int)r2 ) / 2;
  r = (int)r1 - (int)r2;
  g = (int)g1 - (int)g2;
  b = (int)b1 - (int)b2;
  return (((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8);
}


int findColorNearest( long rgb )
{
	int closest=0, sd=256*256*256, d;
	int i, i3;
	unsigned char r,g,b, r2,g2,b2, dr,db,dg;

	r = (rgb&0xff0000)>>16;
	g = (rgb&0xff00)>>8;
	b = (rgb&0xff);

	i3 = 0;
	for (i=0; (i<256); i++) {
		r2 = (unsigned char)rtfcolors[i3+0];
		g2 = (unsigned char)rtfcolors[i3+1];
		b2 = (unsigned char)rtfcolors[i3+2];

		dr = r-r2;
		dg = g-g2;
		db = b-b2;

		//d = SQR(dr) + SQR(dg) + SQR(db);
		d = ColourDistance( r,g,b, r2,g2,b2 );
		if ( d < sd ){
			sd = d;
			closest = i;
		}

		i3+=3;
	}
	return closest;
}


void rtfFile::Stat_WriteComment( FILE *fp, const char *name  )
{
	Fprintf(fp,"\n{\\comment !!!! %s }\n", name );
}

/*int rtfFile::Fprintf( FILE *fp, const char *fmt, ... )
{
	va_list		args;
static	char lineout[10240];

	va_start( args, fmt );
	vsprintf( lineout, fmt, args );
	va_end( args );

	SwapHTMLCharacterTokens( lineout,NULL );

	return WriteLine( fp, lineout );
}*/



void rtfFile::Stat_WriteSpace( FILE *fp , long size )
{
	while( size-- ){
		Fprintf( fp, " \\par \n" );
	}
}

void rtfFile::Stat_WriteItal( FILE *fp, const char *txt )
{
	Fprintf( fp, "\\i %s \\i0 \\par \n", txt );
}

void rtfFile::Stat_WriteBold( FILE *fp, const char *txt )
{
	Fprintf( fp, "\\b %s \\b0 \\par \n", txt );
}

void rtfFile::Stat_WriteBoldDual( FILE *fp, const char *txt, const char *txt2 )
{
	Fprintf( fp, "\\b %s %s \\b0 \\par \n", txt, txt2 );
}


void rtfFile::Stat_WriteLine( FILE *fp, const char *txt )
{
	Fprintf( fp, "%s \\par \n", txt );
}

void rtfFile::Stat_WriteCenterSmall( FILE *fp, const char *txt )
{
	Fprintf( fp, "{\\qc\\fs12 %s \\par }\n", txt );
}


void CQRTFSessionWriter::WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title )
{
	m_baseFile.WritePageTitle( VDptr, fh, title );
}


void CQRTFSessionWriter::WritePageFooter( VDinfoP VDptr, FILE *fh )
{
	m_baseFile.WritePageFooter( VDptr, fh );
}


void CQRTFSessionWriter::Write_SessBold( FILE *fp, const char *txt )
{
	Fprintf( fp, "{\\b %s } \\par \n\n", txt );
}


void CQRTFSessionWriter::Write_SessColumnBold( FILE *fp, const char *txt )
{
	Fprintf( fp, "{\\fs16\\b %s } \\par \\par \n\n", txt );
}


void CQRTFSessionWriter::Write_SessHeader( FILE *fp, const char *client, const char *timeStr, const char *browser, const char *opersys )
{
	Fprintf( fp, "{\\qc Session listing for {\\b %s}, Average length %s \\par ", client, timeStr );
	Fprintf( fp, "{\\fi1\\b\\qc\\fs16\\b\\i Sessions list \\par } \\par \\par \\fs16}\n\n" );
}


void CQRTFSessionWriter::Write_SessTotalTime( FILE *fp , long time, long days )
{
	char	numStr[32];
	CTimetoString( time, numStr );
	Fprintf( fp, "\n{\\b Total online time %s} \\par \n", numStr );
	CTimetoString( time/days, numStr );
	Fprintf( fp, "\n{\\b Average online time per day %s } \\par \n", numStr );
}



void CQRTFSessionWriter::Write_SessLength( FILE *fp, const char *timeStr )
{
	Fprintf( fp, "\n{\\fs16\\b Estimated Session length %s } \\par \n", timeStr );
}

void CQRTFSessionWriter::Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr )
{
	Fprintf( fp, " \\par {\\fs16\\b Session #%d %s } \\par \n", sitem, dateStr );
}


extern long strcmpExtensions( char *string, char exts[256][8] );
extern char *protocolStrings[];

void CQRTFSessionWriter::Write_SessPage( FILE *fp, long item, const char *dateStr, const char *thishost, const char *url, long proto )
{
	char *p, c = ' ';
	char  tbuf[2000];

	if ( proto <= -1 )
		c = '*';

	// WARNING: this could overflow
	if ( strstr( url, "://" ) )		// if the link already has a fully defined root URL
		mystrcpy( tbuf, url );
	else
		MakeURL( tbuf, thishost, url );
	//

	if ( proto>0 && proto<16 )
		p = protocolStrings[proto];
	else 
		p = " ";

	Fprintf( fp, "{\\fs14 %d.%s %s %s %c \\par }\n", item, dateStr, p, tbuf, c );
}















#define	RTFCELL_STR			"\\clbrdrt\\brdrs\\clbrdrb\\brdrs\\clbrdrl\\brdrs\\clbrdrr\\brdrs\\brdrw0\\clcbpat%d\\cellx%04ld\n"

#define	RTFCELLCLEAR_STR	"\\clbrdrt\\clbrdrb\\clbrdrl\\clbrdrr\\brdrw0\\clcbpat%d\\cellx%04ld\n"


// --------------------------------------------------
void rtfFile::RTF_WriteRowDef( FILE *fp , long rgb, long num )
{
	long i, brdw = ACTIVEWIDTH, brdsize, largecolumn = 2000, coltab;

	coltab = findColorNearest( rgb );

	if ( num ){
		long xpos = 0;
		if ( num >1 )
			brdsize = (brdw-largecolumn-500)/(num-1);
		else
			brdsize = (brdw-largecolumn-500)/(num);

		for( i=1; i<num; i++){
			if ( i==2 && MyPrefStruct.headingOnLeft )		// name column
				xpos += largecolumn;
			if ( i==1 )
				xpos += 500;								// index column
			else
				xpos += brdsize;							// all other columns
			Fprintf( fp, RTFCELL_STR, coltab, xpos );
		}
		Fprintf( fp, RTFCELL_STR, coltab, brdw );
	}
}

void rtfFile::WriteCellStartLeft( FILE *fp )
{
	Fprintf( fp, "\\intbl \\sb0\\sl\\sa0 \\plain\\f0\\fs16\\cf0 " );
}
void rtfFile::WriteCellEnd( FILE *fp )
{
	if ( multiLinedCell )
		Fprintf( fp, " } " ); // This is for Multi-line cells such as for the Mean Path table
	multiLinedCell = 0; // Reset the flag to indicate that we are finished writing to cell, if it is multi-lined

	Fprintf( fp, " \\cell\n" );
	fflush( fp );
}

void rtfFile::RTF_WriteCell( FILE *fp, const char *txt )
{
	WriteCellStartLeft( fp );
	Fprintf( fp," %s", txt );
	WriteCellEnd( fp );
}


void rtfFile::Stat_WriteCentreHeading( FILE *fp, const char *txt )
{
	Fprintf( fp, " \\par {\\qc\\b %s \\par }\n", txt );
}


void rtfFile::Stat_WriteRowStart( FILE *fp, long rgb, long col )
{
	Fprintf( fp,"{ " );
	if( col ){
		RTF_WriteRowDef( fp , rgb, col );
	}
}
void rtfFile::Stat_WriteRowEnd( FILE *fp )
{
	Fprintf( fp,"\\row}\n\n" );
}

void rtfFile::Stat_WriteTableStart( FILE *fp, long tabwidth)
{
	Fprintf( fp, " \\par { \\pard \\trowd\\trgaph0\\trleft0\n" );
}

void rtfFile::Stat_WriteTableStart( FILE *fp, long tabwidth, int iCellSpacing)
{
	Fprintf( fp, " \\par { \\pard \\trowd\\trgaph0\\trleft0\n" );
}

void rtfFile::Stat_WriteTableEnd( FILE *fp )
{
	Fprintf(fp,"}\n\n");
}

void rtfFile::Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title )
{
	WriteCellStartLeft( fp );
	Fprintf( fp,"{\\fs18\\qc\\b %s \\par }", title );
	WriteCellEnd( fp );
}



void rtfFile::Stat_WriteHeader( FILE *fp, const char *txt, long space )
{
	char temp[512];
	sprintf( temp, "\\fs18\\b %s", txt );
	RTF_WriteCell( fp, temp );
}


void rtfFile::Stat_WriteNumberData( FILE *fp, long num )
{
	char	numStr[32];

	FormatLongNum( num, numStr );
	RTF_WriteCell( fp, numStr );
}



void rtfFile::Stat_WriteFractionData( FILE *fp, long num, long num2 )
{
	char	numStr[32];
	char	numStr2[32];

	FormatLongNum( num, numStr );
	FormatLongNum( num2, numStr2 );

	RTF_WriteCell( fp, numStr );
}


void rtfFile::Stat_WriteFloatData( FILE *fp, double num )
{
	char	numStr[32];

	Double2CStr( num, numStr, 1 );
	FormatNum( numStr, 0 );

	RTF_WriteCell( fp, numStr );
}



void rtfFile::Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name )
{
	Stat_WriteText( fp, cols, rgb, name );
	//sprintf( temp, "{\\uldb %s {\\v !ExecFile(%s)}}", name, url );

}

void rtfFile::Stat_WriteAnotherLinkInMultiLineCell( FILE *fp, const char *url, const char *name, long num )
{
	if ( multiLinedCell ) // We are still in the same cell of a Multi-lined cell, so add an RTF new line
		Fprintf( fp," \\par " );
	else
		Fprintf( fp, "{" ); // This is for Multi-line cells such as for the Mean Path table

	multiLinedCell++; // Indicate that we are still in the same cell by adding to the line counter

//	Fprintf( fp, " %d. { \\uldb %s }", num, name ); // The last '}' is written by Stat_EndCell
	Fprintf( fp,
		"{ %d. \\field\\fldedit"
		"{\\*\\fldinst {\\f1\\cf1  HYPERLINK \"%s\" }}"
		"{\\fldrslt {\\cs15\\b\\f1\\ul\\cf17 %s}}"
		"}",
		num,
		url,
		name
		);
}

void rtfFile::Stat_WriteLink( FILE *fp, const char *url, const char *name, long num )
{
	//char temp[MAXURLSIZE];

	Fprintf( fp, "{%d. \\uldb %s }", num, name );
	/**
	Fprintf( fp,
		"{%d. \\field\\fldedit"
		"{\\*\\fldinst {\\f1\\cf1  HYPERLINK \"%s\" }}"
		"{\\fldrslt {\\cs15\\b\\f1\\ul\\cf17 %s}}"
		"}",
		num,
		url,
		name
		);
	/**/
	//Fprintf( fp," %s\n", temp );
}

void rtfFile::Stat_WriteLocalDiffFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name ) 
{
	Stat_WriteText( fp, cols, rgb, name);
//	Stat_WriteLink( fp, url, name, 1 );
}

void rtfFile::Stat_WriteLocalSameFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name )
{
	Stat_WriteText( fp, cols, rgb, name);
//	Stat_WriteLink( fp, url, name, 1 );
}

void rtfFile::Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name )
{
	char temp[MAXURLSIZE], *p = temp;

//	sprintf( temp, "{\\uldb %s }", name );  //{\\v !ExecFile(%s)}

	if (*url == '#')
	{
		sprintf( temp,
			"{\\field\\fldedit"
			"{\\*\\fldinst {\\f1\\cf1  HYPERLINK \"%s\" }}"
			"{\\fldrslt {\\cs15\\b\\f1\\ul\\cf17 %s}}"
			"}",
			name,
			name
			);
	}
	else
	{
		sprintf( temp,
			"{\\field\\fldedit"
			"{\\*\\fldinst {\\f1\\cf1  HYPERLINK \"%s\" }}"
			"{\\fldrslt {\\cs15\\b\\f1\\ul\\cf17 %s}}"
			"}",
			url,
			name
			);
	}
	RTF_WriteCell( fp, temp );
}

void rtfFile::Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name )
{
	char temp[MAXURLSIZE];
//	sprintf( temp, "{\\uldb %s }", name );  //{\\v !ExecFile(%s)}
	sprintf( temp,
		"{\\field\\fldedit"
		"{\\*\\fldinst {\\f1\\cf1  HYPERLINK \"%s\" }}"
		"{\\fldrslt {\\cs15\\b\\f1\\ul\\cf17 %s}}"
		"}",
		url,
		name
		);
	RTF_WriteCell( fp, temp );
}

void rtfFile::Stat_WriteText( FILE *fp, short cols, long rgb, const char *name )
{
	RTF_WriteCell( fp, name );
}

void rtfFile::Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name )
{
	RTF_WriteCell( fp, name );
}

void rtfFile::WriteFilterText(FILE* fp, const char* szFilterString)
{
	int	iStrLen;
	for (const char*	sz = szFilterString; iStrLen=mystrlen(sz); sz += (iStrLen+1))
	{
		Stat_WriteLine(fp, sz);
	}
}

void rtfFile::WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title )
{
	if ( rtf_head[0] == 0 )
		InitRTFHead();
	WriteLine( fh, rtf_head );
	Fprintf( fh,"{\\plain\\f0\\fs20\\cf0\\n");
	Fprintf( fh, "\n \\par \\qc\\widctlpar\\adjustright \\cbpat0 \\f4\\cgrid{\\b\\cf%d\\fs32 %s \\par } }\\pard \n", findColorNearest( 0xAAAAAA ), title);
}

//static char *imagesuffixStrings[] = { ".gif", ".jpg", ".png", ".bmp", 0,0,0,0,0 };

//---- Write out a default output table header
void rtfFile::WritePageImageLink( VDinfoP VDptr, FILE *fh, const char *imagename, const char *title, int graph, long sort )
{
	if ( graph && imagename && title ) {
		char	*suff;

		suff = imagesuffixStrings[ MyPrefStruct.image_format ];

		Fprintf( fh, " \\par {\\qc\\b } \\par \\par \n" );
		{
			char fname[256];
			sprintf( fname, "IMGSRC=%s.png", imagename );
			Stat_WriteComment( fh, fname );
		}
		Fprintf( fh, " \\par {\\qc\\b } \\par \\par \n" );
	}
}

void rtfFile::WritePageFooter( VDinfoP VDptr, FILE *fh )
{
	Fprintf( fh, rtf_foot );
}


void rtfFile::WriteDemoBanner( VDinfoP VDptr, FILE *fp )
{
	if ( IsDemoReg() )
		WriteLine( fp,"{ \\par \n\\qc0\\fs32\\b iReporter Demonstration \\par \n\n http://www.ac3dec.com/ \\par \nOrder iReporter Here \\par \n} \\par \n");
}


/***********************************************************

		SERVER SUMMARY FUNCTIONS

  *********************************************************/
void rtfFile::SummaryTableEntryURL( FILE *fp, const char *text, const char *url, long normal )
{
	Fprintf(fp,"{\\fs17\\b %s \\par }\n", text );
}

void rtfFile::SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal )
{
	long col, offset = LMARGIN*3;

	col = findColorNearest( rgb );
	Stat_WriteRowStart( fp, 0,0 );
	Fprintf( fp, RTFCELLCLEAR_STR, col, LMARGIN+(ACTIVEWIDTH/2) );
	Fprintf( fp, RTFCELLCLEAR_STR, col, ACTIVEWIDTH );

	RTF_WriteCell( fp, text );
	RTF_WriteCell( fp, number );

	Stat_WriteRowEnd( fp );
	//Fprintf(fp,"{\\fs17\\b %s\t\t\t %s \\par }\n", text, number );
}


void rtfFile::SummaryTableSubTitle( FILE *fp, const char *text, int x, int y )
{
	Stat_WriteSpace( fp, 2 );
	Stat_WriteBold( fp, text );
}



void ByteToHex( void *ptr, char *str )
{
	long	num, addr;
	short	digit;
	char	hex[]="0123456789ABCDEF";

	if ( ptr && str ){
		addr = (long)ptr;

		for ( num=1; num >=0; num--){
			digit = addr & 0xf;
			str[num] = hex[digit];
			addr >>= 4;
		}
		str[8] = 0;
	}
}

void rtfFile::RTF_IncludePicture( FILE *fp, const char *path, const char *file )
{
	FILE *in;
	char filename[256], fullpath[256];

	strcpybrk( filename, file, ' ', 1 );
	PathFromFullPath( (char*)path, fullpath );	// safe cast as fullpath is non-null.
	strcat( fullpath, filename );

	if ( in = fopen( fullpath, "rb" ) ){
		unsigned char c;
		long len = 0, width,height;
		char ttxt[256], *p;

		OutDebugs( "RTF: including image %s", fullpath );
		width  = GraphWidth()*10;
        height = GraphHeight()*10;
		
		//fflush( fp );
		//Fprintf( fp, "{\\pict\\jpegblip\\bliptag-1523234968{\\*\\blipuid\na535476830bc25577a8c99c20ca85033}" );
		sprintf( ttxt, "%08x%08x%08x%08x", rand() * 10000000,rand() * 10000000,rand() * 10000000,rand() * 10000000 );
#ifdef DEF_MAC
		Fprintf( fp, "{\\pict\\picscalex75\\picscaley75\\picwgoal%d\\pichgoal%d\\pngblip\\bliptag%d{\\*\\blipuid\n%s}", width, height, rand()*10000000, ttxt );
#else
		Fprintf( fp, "{\\pict\\picscalex95\\picscaley95\\picwgoal%d\\pichgoal%d\\pngblip\\bliptag%d{\\*\\blipuid\n%s}", width, height, rand()*10000000, ttxt );
#endif
        //Fprintf( fp, "{\\pict\\picscalex75\\picscaley75\\picwgoal%d\\pichgoal%d\\pngblip\\bliptag%d{\\*\\blipuid\n%s}\n", width, height, rand()*10000000, ttxt );

		p = ttxt;
		while( !feof( in ) ){
			c = fgetc( in );

			//sprintf( ttxt, "%02x", c );
			//fwrite( ttxt, 1, 2, fp );
			Fprintf( fp, "%02x", c );
			len++;
			if ( len >= 40 ) {
				//fwrite( ttxt, 1, len*2, fp );
				Fprintf( fp, "\n" );
				len = 0;
			}
		}
		Fprintf( fp, "}\n" );

		Fclose( in );

		remove( fullpath );		// win32 call, make sure to use others for unix/mac
		OutDebugs( "RTF: deleting image %s", fullpath );
	}
}




// merge all out put files into one file
void rtfFile::MakeOneRTF( FILE *fp, const char *path )
{
	long len, index = 1, tot;
	long writesize;
	char *ram, *file, *imfile, *p, *start, *end, *strip;

	tot = FileHistoryGetTotal();
	fseek( fp, -6, SEEK_CUR );

	for( index = tot-1; index>0; index-- ){
		file = (char*)FileHistoryGet( index );
		if ( file ){
			if ( strstr( file, ".rtf" ) ){
				len = GetFileLength( file );
				if ( len ){
					ram = (char*)malloc( len+8 );
					FileToMem( file, ram, len );
					ram[len] = 0;

					OutDebugs( "RTF: including file %d %s", index, file );

					// stuff to make macs happy
					strip=ram;
                    while (*strip) {
                        if (*strip=='\r')	*strip='\n';
                        strip++;
                    }

					end = ram + (len-6);
					start = strstr( ram, "{\\comment" );

					if ( start )
					{

						while( p = strstr( start, "IMGSRC" ) ){
							imfile = p + 7;
							p = mystrchr( p, '\n' );
							if ( !p ) p = mystrchr( p, '\r' );
							if ( !p ) break;
							writesize = p - start;

							fwrite( start, 1, writesize , fp );
							RTF_IncludePicture( fp, path, imfile );
							start = p;
						}
					}
					writesize = end - start;
					fwrite( start, 1, writesize , fp );
					remove( file );
					OutDebugs( "RTF: deleting file %s", file );
					free( ram );
				}
			}
		}
	}
	fwrite( " \r\n", 1, 3 , fp );
	Stat_WriteComment( fp, "END OF REPORT" );
	fwrite( rtf_lastfoot, 1, strlen(rtf_lastfoot), fp );

	OutDebug( "RTF: writing footer" );

	StopFopenHistory();
	StartFopenHistory();
}


void rtfFile::FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs )
{
	StatusSet( "Building RTF report..." );
	MakeOneRTF( fp , filename );
	fflush( fp );
	Fclose( fp );		// close the first output file.
}

void rtfFile::Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip )
{
	Stat_WriteText( fp, 1, -1, name );
}
