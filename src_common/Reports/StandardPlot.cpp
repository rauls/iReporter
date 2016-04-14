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
*/

#include "FWA.h"

#include <string>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>


#ifdef DEF_MAC
	#include "MacStatus.h"
	#include "MacUtil.h"
	#include "progress.h"	
	#include "post_comp.h"
#else
	#include "postproc.h"
#endif

#include "myansi.h"
#include "StandardPlot.h"
#include "gd.h"
#include "gdfont_Geneva.h"
#include "gdfont_Arial.h"
#include "gdfonts.h"
#include "gdfontt.h"
#include "config_struct.h"
#include "Report.h"
#include "editpath.h"
#include "worldmap_gif.h"
#include "datetime.h"
#include "translate.h"
#include "net_io.h"

#include "config.h"
#include "EngineParse.h"
#include "engine_drill.h"
#include "StatDefs.h"
#include "FileTypes.h"

extern "C" struct App_config MyPrefStruct;

extern long		allTotalRequests;
extern long		allTotalFailedRequests;
extern long		allTotalCachedHits;
extern __int64	allTotalBytes;
extern long		allfirstTime, allendTime;
extern long		badones;
extern long		totalDomains;
extern int 	endall,stopall;


#define	MULTI3D_HEIGHT	(100)

// legend position
#define	PIETAB_X		(25)
#define	PIETAB_Y		(60)
#define	PIETAB_W		(60)
#define	PIETAB_H		(140)

#define	PIETAB_XX	(20)   //
#define	PIETAB_YY	(55)
#define	PIETAB_WW	(385)
#define	PIETAB_HH	(265)

#define	PIE_RAD		180
//#define	MULTI_COLS	10

#define FORMAT_GIF 0
#define FORMAT_JPG 1
#define FORMAT_PNG 2
#define FORMAT_BMP 3
#define FORMAT_GRAPH_PDF 4 

// RGB COLORS			        red       orange    yellow    lgreen    dgreen    dblue      lblue     purple
long		pie_rgb[32]		= { 0xcc0033, 0xff6600, 0xcc9900, 0x669900, 0x006633, 0x000099, 0x3333ff, 0x990099, 0x000000};
long		pie_hlit[32]	= { 0xff6666, 0xffcc99, 0xffcc00, 0xcccc00, 0x66cc33, 0x99ccff, 0x33ccff, 0xcc0099, 0x222222};
long		multi_rgb[32]	= { 0xff0033, 0xff8C00, 0xffcc33, 0x00cc33, 0x339999, 0x00ffff, 0x008Cff, 0x9999ff, 0xcc66cc,0xff99cc,
								0x880019, 0x884600, 0x886619, 0x006619, 0x194c4c, 0x008888, 0x004288, 0x4c4c88, 0x663366,0x884c66};

#define		BLACK	0
#define		DGREY	0x555555
#define		GREY	0x888888
#define		LGREY	0xC8C8C8
#define		LGREY2	0xDDDDDD
#define		WHITE	0xffffff
#define		GCOLOR	0xaaaaaa
#define		RED		0xff0000
#define		GREEN	0x00ff00
#define		BLUE	0x0000ff
#define		YELLOW	0xFFFF00

#define		GRADIENT_STEPS		(64)

// INDEX COLORS
long		black,dgrey,grey,lgrey,white,
			red,green,blue,yellow,
			label_col,grid_col,base_col,borderhi_col,borderlo_col,border_col,
			bgcolor,bar1,bar2,bar3;
long		bgcolors[GRADIENT_STEPS+4];





double roundvalue( double value )
{
	long count=0,val;
	double	newv,i,f;

	// scale 123456 to 1.23456
	while( value > 10 ){
		value /= 10;
		count++;
	}
	f = modf( value, &i );
	val = (long)(5*f)+1;
	newv = i + (val/5.0);
	//val = (long)(value*5)+1;
	//newv = val/5.0;
	if( count < 2 ){
		newv = (i+1);
		count=1;
	}
	while( count>0 ){
		newv *= 10;
		count--;
	}
	return newv;
}


long ScaleRGB( double perc,long rgb )
{
	long r,g,b;

	r = (rgb & 0xff0000) >> 16;
	g = (rgb & 0xff00) >> 8;
	b = (rgb & 0xff);

	r = r * perc / 100.0;
	if( r < 0 ) r = 0;
	if( r > 255 ) r = 255;

	g = g * perc / 100.0;
	if( g < 0 ) g = 0;
	if( g > 255 ) g = 255;

	b = b * perc / 100.0;
	if( b < 0 ) b = 0;
	if( b > 255 ) b = 255;

	return ( r<<16 | g<<8 | b );
}


// Pick a color X percentage between colorA and colorB,   0% being A, and 100% being B.
//            0%                            100%
// 00 |-------A------------------------------B---------| FF
//            0x30                          0xc0
#define	SCALE_COMPONENT(a,b)	(a + ((b-a)*perc) )
#define	LIMIT_COMPONENT(a)		if( a < 0 ) a = 0;   if( a > 255 ) a = 255;

long ScaleRGBtoRGB( double perc /* 0..100 */ ,long rgb, long rgb2 )
{
	long r[3],g[3],b[3];

	r[0] = (rgb & 0xff0000) >> 16;
	g[0] = (rgb & 0xff00) >> 8;
	b[0] = (rgb & 0xff);

	r[1] = (rgb2 & 0xff0000) >> 16;
	g[1] = (rgb2 & 0xff00) >> 8;
	b[1] = (rgb2 & 0xff);

	perc = perc / 100.0;

	r[2] = SCALE_COMPONENT(r[0],r[1]);
	LIMIT_COMPONENT( r[2] );

	g[2] = SCALE_COMPONENT(g[0],g[1]);
	LIMIT_COMPONENT( g[2] );

	b[2] = SCALE_COMPONENT(b[0],b[1]);
	LIMIT_COMPONENT( b[2] );

	return ( r[2]<<16 | g[2]<<8 | b[2] );
}




// ------------------------------------ RASTER RENDERING CODE ---------------------------------------


StandardPlot::StandardPlot()
{
	im = NULL;
	mystrcpy( OSNames[0], "Windows" );
	mystrcpy( OSNames[1], "MacOS" );
	mystrcpy( OSNames[2], "Unix" );
	mystrncpy( OSNames[3], Translate( LABEL_OTHER ), 16 );
	OSNames[3][15] = 0;
}

StandardPlot::~StandardPlot()
{
}

long StandardPlot::PlotImageColorGetRGB( int ct )
{
	return gdImageColorGetRGB( im, ct );
}

int StandardPlot::PlotImageColorAllocate( int r, int g, int b)
{
	return gdImageColorAllocate( im, r, g, b );
}

int StandardPlot::PlotImageColorAllocateRGB( long rgb )
{
	return gdImageColorAllocateRGB( im, rgb );
}

int StandardPlot::PlotImageColorNearest( int r, int g, int b)
{
	return gdImageColorNearest( im, r, g, b );
}

int StandardPlot::PlotImageColorExact( int r, int g, int b )
{
	return gdImageColorExact( im, r, g, b );
}

void StandardPlot::AllocDefaultColors()
{
	// get colors
	black = PlotImageColorAllocate( 0, 0, 0);      
 	dgrey = PlotImageColorAllocate( 85, 85, 85);    
 	lgrey = PlotImageColorAllocate( 221, 221, 221);    
 	white = PlotImageColorAllocate( 255, 255, 255); 
 	red = PlotImageColorAllocate( 255, 0, 0); 
 	green = PlotImageColorAllocate( 0, 255, 0 ); 
 	blue = PlotImageColorAllocate( 0,0,255); 
 	yellow = PlotImageColorAllocate( 255, 255, 0 ); 
}

/*int StandardPlot::PlotImageColorSet(gdImagePtr im, int ct, int r, int g, int b)
{
	return gdImageColorSet(gdImagePtr im, int ct, int r, int g, int b)
}*/


void StandardPlot::PlotImagePolygon( gdPointPtr p, int n, int c, int fill /*= 0*/ )
{
	gdImagePolygon( im, p, n, c );
}

void StandardPlot::PlotString( long fontsize, double x, double y, const char *string, long color, int rotate, int bold, int translate )
{
	if( im )
	{
		if( translate ) //remHTMLTokens )
		{
			std::string convertHTMLTokensStr = ConvertHTMLTokens( string ); 
			gdImagePropString( im, GetFontStruct(fontsize), x, y, convertHTMLTokensStr.c_str(), color );
		}
		else
			gdImagePropString( im, GetFontStruct(fontsize), x, y, string, color );
	}
}

#if DEF_WINDOWS
#include "GraphDrawStr.h"
extern NonISO88591String *nonISO88591String;
#endif // 


void StandardPlot::PlotTransString( long fontsize, double x, double y, const char *string, long color, int rotate, int bold, int translate )
{
#ifndef DEF_WINDOWS
	if( im )
	{
		if( translate ) //remHTMLTokens )
		{
			std::string convertHTMLTokensStr = ConvertHTMLTokens( string ); 
			gdImagePropString( im, GetFontStruct(fontsize), x, y, convertHTMLTokensStr.c_str(), color );
		}
		else
			gdImagePropString( im, GetFontStruct(fontsize), x, y, string, color );
	}
#else 	

	if( !im )
		return;

	if( !nonISO88591String )
	{
		if( translate ) //remHTMLTokens )
		{
			std::string convertHTMLTokensStr = ConvertHTMLTokens( string ); 
			gdImagePropString( im, GetFontStruct(fontsize), x, y, convertHTMLTokensStr.c_str(), color );
		}
		else
			gdImagePropString( im, GetFontStruct(fontsize), x, y, string, color );
		return;
	}

	std::string convertHTMLTokensStr = ConvertHTMLTokens( string ); 
	int l = 0;
	l = strlen(convertHTMLTokensStr.c_str())/2;
	nonISO88591String->DrawString( fontsize, string );

	if( rotate )
	{
		for( int h = 0; h < nonISO88591String->Height(); h++ )
		{
			for ( int w = 0; w < nonISO88591String->Width(); w++ )
			{
				if( !nonISO88591String->Data( nonISO88591String->Width()*h+w ) ) {
					gdImageQuickSetPixel(im, x+h, y-w, color);	
				}
			}
		}
	}
	else
	{
		for( int h = 0; h < nonISO88591String->Height(); h++ )
		{
			for ( int w = 0; w < nonISO88591String->Width(); w++ )
			{
				if( !nonISO88591String->Data( nonISO88591String->Width()*h+w ) ) {
					gdImageQuickSetPixel(im, x+w, y+h, color);	
				}
			}
		}
	}
#endif // DEF_WINDOWS	
}


void StandardPlot::PlotStringConvertHTMLTokens( long fontsize, double x, double y, const char *string, long color, int rotate /*= NO_ROTATE*/, int bold /*= 0*/ )
{
	PlotTransString( fontsize, x, y, string, color, rotate, bold, REM_HTML_TOKENS );
}


void StandardPlot::PlotText(long fontsize, double x, double y, const char *string, long color, int rotate /*= NO_ROTATE*/, int bold /*= 0*/, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	if( im )
	{
		if( remHTMLTokens )
		{
			std::string convertHTMLTokensStr = ConvertHTMLTokens( string ); 
			gdImagePropString( im, GetFontStruct(fontsize), x, y, convertHTMLTokensStr.c_str(), color );
		}
		else
			gdImagePropString( im, GetFontStruct(fontsize), x, y, string, color );
	}
	return;
}

void StandardPlot::PlotStringBold(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	if( im ){
		gdImagePropString( im, GetFontStruct(fontsize), x, y, string, color);
		gdImagePropString( im, GetFontStruct(fontsize), x+1, y, string, color);
	}
}

void StandardPlot::PlotTransStringC(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	PlotTransString( fontsize, x - (GetTransStringLen(string,fontsize)/2), y, string, color, 0, 0, remHTMLTokens );
}

void StandardPlot::PlotStringC(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	gdFont *tf = GetFontStruct(fontsize);
	PlotString( fontsize, x - (GetStringLen(string,fontsize)/2), y, string, color, remHTMLTokens );
}

void StandardPlot::PlotTransStringUp( long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
#if DEF_WINDOWS
	if( nonISO88591String )
		PlotTransString( fontsize, x, y, string, color, ROTATE, 0, remHTMLTokens );
	else
#endif
		PlotTextUp( fontsize, x, y, string, color, remHTMLTokens );
}

void StandardPlot::PlotStringUp(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	PlotTextUp( fontsize, x, y, string, color, remHTMLTokens );
}

void StandardPlot::PlotTextC(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	if( im ){
		gdFont *tf = GetFontStruct(fontsize);
		gdImagePropString( im, tf, x - (gdStringWidth(tf,string)/2), y, string, color);

	}
}

void StandardPlot::PlotTextUp(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	if( im )
		gdImagePropStringUp( im, GetFontStruct(fontsize), x, y, string, color);
}

void StandardPlot::PlotXScatterValues( long mins, long i2, long x, long xsize, long hcount, long ypos, long ysize, long grid_color )
{
	long i,lastx=0,x2;
	char name[64];
	gdFont *tf = GetFontStruct( 2 );

	for( i=0; i< mins; i+=i2 ){
		//x2 = graph->xborder + ((i/i2)*xstep);
		x2 = x + xsize;
		if( (x2-lastx) > 16 ){
			gdImageLine( im, x2, ypos-ysize, x2, ypos+3, label_col );
			sprintf( name, "%d", i );
			PlotNum( 1, x2-6-((-gdStringWidth(&gdFont_Geneva,name))/2), ypos+5, name, label_col );	// draw the current bars NAME/FILE
			lastx = x2;
		}
	}
}
void StandardPlot::PlotHorizbar( long xpos, long ypos, long barsize, long ystep, long bar1, long bar2, long bar3, long depth )
{
	if( MyPrefStruct.graph_style == GRAPH_2DSTYLE )
		PlotBevel( xpos,ypos, barsize, (int)ystep, bar1, bar2, bar3 );	
	else {
		if( MyPrefStruct.paletteType == GRAPH_WEBCOLOR )
			Plot3dHorizBox( xpos,ypos, barsize, (int)ystep, bar1, bar2, bar3, depth );	// draw bar
		else
			Plot3dHorizGradBox( xpos,ypos, barsize, (int)ystep, bar1, bar2, bar3, depth );	// draw bar
	}
}
void StandardPlot::Plot3dHorizBox( long xpos, long ypos, long barsize, long ystep, long bar1, long bar2, long bar3, long depth )
{
	gd3dHorizBox( im, xpos,ypos, barsize, (int)ystep, bar1, bar2, bar3, depth );	// draw bar
}
void StandardPlot::Plot3dHorizGradBox( int x, int y, int xs, int ys, int c1, int c2, int c3, int depth )
{
	gd3dHorizGradBox( im, x, y, xs, ys, c1, c2, c3, depth );	// draw bar
}
void StandardPlot::PlotBevel( int x, int y, int xs, int ys, int c1, int c2, int c3 )
{
	gdBevel( im, x, y, xs, ys, c1, c2, c3 );
}
void StandardPlot::PlotVertbar( double xpos, double ypos, double xs, double ys, long bar1, long bar2, long bar3, long depth )
{
	if( im ){
		if( MyPrefStruct.graph_style == GRAPH_2DSTYLE )
			PlotBevel( xpos,ypos, xs, (int)ys, bar1, bar2, bar3 );	
		else {
			if( MyPrefStruct.paletteType == GRAPH_WEBCOLOR )
				gd3dVertBox( im, xpos,ypos, xs, (int)ys, bar1, bar2, bar3, depth );	// draw bar
			else
				Plot3dVertGradBox( xpos, ypos, xs, (int)ys, bar1, bar2, bar3, depth );	// draw bar
		}
	}
}
void StandardPlot::Plot3dVertGradBox( double x, double y, double xs, double ys, int c1, int c2, int c3, int depth )
{
	gd3dVertGradBox( im, x, y, xs, ys, c1, c2, c3, depth );	// draw bar
}


void StandardPlot::PlotLine( float x, float y, float x2, float y2, long lcol )
{
	if( im )
		gdImageQuickLine(im, x, y, x2, y2, lcol );
}

void StandardPlot::PlotDrawLine( float x, float y, float x2, float y2, long lcol )
{
	if( im )
		gdImageQuickLine(im, x, y, x2, y2, lcol );
}

void StandardPlot::PlotDottedLine( float x, float y, float x2, float y2, long dcol )
{
	int	style[]= {1,0};
	if( im ){
		gdImageSetStyle( im, &style[0] , 2 );
		gdImageDashedLine(im, x, y, x2, y2, dcol );
	}
}
void StandardPlot::PlotPoint( float x, float y, long pcol )
{
	if( im )
		gdImageSetPixel(im, x, y, pcol );
}
void StandardPlot::PlotRect( float x, float y, float x2, float y2, long rcol )
{
	if( im )
		gdImageRectangle(im, x, y, x2-1, y2-1, rcol );
}
void StandardPlot::PlotFilledRect( float x, float y, float x2, float y2, long frcol )
{
	if( im )
		gdImageFilledRectangle(im, x, y, x2, y2, frcol );
}

void StandardPlot::PlotNumRightJustified(long fontsize, double x, double y, const char *string, long color )
{
	double len = GetStringLen( string, fontsize );	// Position of the text
	PlotText( fontsize, x-len, y, string, color );
}

void StandardPlot::PlotStringRightJustified(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	const char *removedHTMLTokensStr;
	if( remHTMLTokens )
		removedHTMLTokensStr = ConvertHTMLTokens( string );
	else
		removedHTMLTokensStr = string;
	double len = GetStringLen( removedHTMLTokensStr, fontsize );	// Position of the text
	PlotString( fontsize, x-len, y, removedHTMLTokensStr, color );
}

void StandardPlot::PlotTransStringRightJustified(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	const char *removedHTMLTokensStr;
	if( remHTMLTokens )
		removedHTMLTokensStr = ConvertHTMLTokens( string );
	else
		removedHTMLTokensStr = string;
	double len = GetTransStringLen( removedHTMLTokensStr, fontsize );	// Position of the text
	PlotTransString( fontsize, x-len, y, removedHTMLTokensStr, color, 0, 0, remHTMLTokens );
}

double StandardPlot::GetTransStringLen( const char* str, int size )
{
#ifndef DEF_WINDOWS
	return GetStringLen( str, size );
#else
	if( !nonISO88591String )
		return GetStringLen( str, size );
	else
		return nonISO88591String->FontSize( size ) * (strlen( str ) / 2);
#endif // DEF_WINDOWS
}

double StandardPlot::GetStringLen( const char* str, int size )
{
	gdFontPtr fontPtr = GetFontStruct( size );
	return gdStringWidth( fontPtr, str );
}

std::string StandardPlot::SetTransStringLen( std::string str, double lenWanted, int size /*= 1*/, short truncateTextLeftOrRight /*= 0*/ )
{
#ifndef DEF_WINDOWS
	return SetStringLen( str, lenWanted, size, truncateTextLeftOrRight );
#else
	if( !nonISO88591String )
		return SetStringLen( str, lenWanted, size, truncateTextLeftOrRight );
	else
	{
		short unicodelen = ( str.length() ) / 2;
		if( ( nonISO88591String->FontSize( size ) * unicodelen ) > lenWanted )
		{
			std::string reducedStr;
			//char *buf = new char[ str.length()+1 ];
			reducedStr = str;
			short charsToCopy = lenWanted / nonISO88591String->FontSize( size );
			reducedStr.resize( charsToCopy*2 );
/*			if( truncateTextLeftOrRight == 0) // LEFT
			{
			}
			else // RIGHT
			{
			}*/
			return reducedStr;
		}
		else
			return str;
		//return TextSizeForNonISO88591[size] * (strlen( str ) / 2);
	}
#endif
}

// return font from size value, 0,1,2   small,normal,large
gdFont *StandardPlot::GetFontStruct( long size )
{
	switch( size ){
		case 0:		return gdFontSmall; break;
		case 2:		return &gdFont_Arial; break;
		case 3:		return gdFontTiny; break;
		case 1:
		default :	return &gdFont_Geneva; break;
	}
}

void StandardPlot::PlotImageFilledPolygon( gdPointPtr p, int n, int c )
{
	gdImageFilledPolygon( im, p, n, c );
}

std::string StandardPlot::SetStringLen( std::string str, double lenWanted, int size /*= 1*/, short truncateTextLeftOrRight /*= 0*/  )
{
	const char *ellipse = "...";
	std::string reducedStr;
	char *buf = new char[ str.length()+1 ];
	if( truncateTextLeftOrRight == 0) // LEFT
	{
		std::string reverseStr;
		reverseStr.reserve(str.length());
		char c;
		int i;
		for( i=str.length()-1; i >= 0; i-- )
		{
			c = str[i];
			reverseStr += c;
		}
		gdFontPtr fontPtr = GetFontStruct( size );
		gdSetStringWidth( fontPtr, reverseStr.c_str(), lenWanted - gdStringWidth( fontPtr, ellipse ), buf );
		reverseStr = buf;
		reducedStr = ellipse;
		for( i=reverseStr.length()-1; i >= 0; i-- )
		{
			c = reverseStr[i];
			reducedStr += c;
		}
	}
	else // RIGHT
	{
		gdFontPtr fontPtr = GetFontStruct( size );
		gdSetStringWidth( fontPtr, str.c_str(), lenWanted - gdStringWidth( fontPtr, ellipse ), buf );
		reducedStr = buf;
		reducedStr += ellipse;
	}
	delete[] buf;
	return reducedStr;
}

const char* StandardPlot::ConvertHTMLTokens( const char *text )
{
	static std::string transText;
	if( MyPrefStruct.language )
	{
		if (text)
			transText = text;
		else
			return "";
		ConvertHTMLCharacterTokens( transText );
		return transText.c_str();
	}
	return text;
}

const char* StandardPlot::Translate( long stringId )
{
	static std::string transText;
	static char *text;

	text = TranslateID( stringId );
	if( text == 0 )
		return EMPTY_STR;
	transText = text;
	transText = ConvertHTMLTokens( transText.c_str() );
	return transText.c_str();
}

void StandardPlot::FixAsciiSymbols( char *txt )
{
	char *p = txt;
	char *out = txt,c;

	while( c=*p++ ){
		if( c == '&' ){
			if( *p == '#' ){
				p++;
				c = myatoi( p );
				p = mystrchr( p, ';' );
				p++;
			}
		}
		*out++ = c;
	}
	*out++ = 0;
}

#include "Winutil.h"

// -------------------------------------------------------------------------------------------------------
void StandardPlot::FW_SaveImage( VDinfoP VDptr, char *name )
{
	char	tempName[512];
	long	image_format;
	
	FILE 	*out = 0;
	if( VDptr && MyPrefStruct.ShowVirtualHostGraph() )
	{
		char	newFile[512];
		sprintf( newFile, "%s%c%s", GetDomainPath(VDptr), PATHSEP, name );
		CopyFilenameUsingPath( tempName, MyPrefStruct.outfile, newFile );
		ReplacePathFromHost( GetDomainPath(VDptr), tempName );
	} else
		CopyFilenameUsingPath( tempName, MyPrefStruct.outfile, name );
	gdImageInterlace(im, 1); //interlace image

	// if file format is RTF make image format PNG
	image_format = MyPrefStruct.GetImageFormat();

	switch( image_format ){
		default:
		case GRAPH_GIF:	strcat( tempName, ".gif");
				if( out = Fopen( tempName, "wb") ) gdImageGif(im, out, 1 ); break;
		case GRAPH_BMP:	strcat( tempName, ".bmp");
				if( out = Fopen( tempName, "wb") ) gdImageBMP(im, out ); break;
		case GRAPH_JPEG:	
				strcat( tempName, ".jpg");
#ifdef JPEGLIB_H
				if( out = Fopen( tempName, "wb") ) { 
					gdImageJPG(im, out, 92 ); 
					SetWorldRegionJPGFilename( tempName ); 
				}
#else
				if( out = Fopen( ".temp.bmp", "wb") ) {
					gdImageBMP(im, out );
					fclose(out);
					Convert_Temp_Image( tempName, 'j' );
				}
#endif
				break;		// Future file formats
		case GRAPH_PNG:	strcat( tempName, ".png");
#ifdef DEF_WINDOWS
				if( out = Fopen( ".temp.bmp", "wb") ) {
					gdImageBMP(im, out );
					Convert_Temp_Image( tempName, 'p' );
				}
#else
				if( out = Fopen( tempName, "wb") ) gdImagePNG(im, out ); 
#endif
				break;
	}
	if( out )
		fclose(out);

	DestroyImage();
}

void StandardPlot::DestroyImage()
{
	if( im )
	{
		gdImageDestroy(im);
		im = NULL;
	}
}


#define MAX_SHADOW_SIZE		64
static long	g_shadowSize = 24;
long ShadowSize()
{
	return g_shadowSize;
}
#ifndef	M_PI
#define	M_PI	3.141592653589
#endif

#define	SHADOW_START	50		//starting darkness level %
#define SHADOW_MATH(level)	(SHADOW_START+(i*((100-SHADOW_START)/(double)g_shadowSize)))

void StandardPlot::MakeBottomShadow( VDinfoP VDptr, long bcolor, long isize )
{
	long			gradcolors[MAX_SHADOW_SIZE], bcol, i , rgb, 
					w,h;
	double	scale;

	if( MyPrefStruct.report_format != 0 )
		return;

	w = isize;
	h = g_shadowSize;
	CreateImage( w, h, MyPrefStruct.paletteType );
	AllocDefaultColors();
	bcol = PlotImageColorAllocateRGB( bcolor );
	for( i=0; i<g_shadowSize;i++ ){
		scale = sin( ((SHADOW_MATH(i)*M_PI)/180) );
		rgb = ScaleRGB( scale*100, bcolor );
		gradcolors[i] = PlotImageColorAllocateRGB( rgb );
	}

	PlotFilledRect( 0, 0, w-1, h-1, bcol );
	
	for( i=0; i<g_shadowSize;i++ ){
		PlotDrawLine( g_shadowSize, i, w, i, gradcolors[i] );	// wide line
		PlotDrawLine( g_shadowSize-i, 0, g_shadowSize, i, gradcolors[i] );	// left curve
	}
	FW_SaveImage( VDptr, "bshad" );
}
void StandardPlot::MakeCornerShadow( VDinfoP VDptr, long bcolor, long isize )
{
	long			gradcolors[MAX_SHADOW_SIZE+8], bcol, i , rgb, 
					w,h;
	double	scale;

	if( MyPrefStruct.report_format != 0 )
		return;

	w = isize;
	h = g_shadowSize;
	CreateImage( g_shadowSize, h, MyPrefStruct.paletteType );
	AllocDefaultColors();
	bcol = PlotImageColorAllocateRGB( bcolor );
	for( i=0; i<g_shadowSize;i++ ){
		scale = sin( ((SHADOW_MATH(i)*M_PI)/180) );
		rgb = ScaleRGB( scale*100, bcolor );
		gradcolors[i] = PlotImageColorAllocateRGB( rgb );
	}
	PlotFilledRect( 0, 0, w, h, bcol );
	for( i=0; i<g_shadowSize;i++ ){
		PlotDrawLine( 0, i, w+i, i, gradcolors[i] );	// wide line
		PlotDrawLine( w+i, 0, w+i, i, gradcolors[i] );	// end
		if( i+1 < g_shadowSize )
			PlotPoint( w+i, i, gradcolors[i+1] );
	}
	FW_SaveImage( VDptr, "cshad" );
}

void StandardPlot::MakeRightShadow( VDinfoP VDptr, long bcolor, long isize )
{
	long			gradcolors[MAX_SHADOW_SIZE], bcol, i , rgb,
					w,h;
	double	scale;

	if( MyPrefStruct.report_format != 0 )
		return;

	h = isize;
	w = g_shadowSize;
	CreateImage( w, h, MyPrefStruct.paletteType );
	AllocDefaultColors();
	bcol = PlotImageColorAllocateRGB( bcolor );
	for( i=0; i<g_shadowSize;i++ ){
		scale = sin( ((SHADOW_MATH(i))*M_PI)/180 );
		rgb = ScaleRGB( scale*100, bcolor );
		gradcolors[i] = PlotImageColorAllocateRGB( rgb );
	}
	PlotFilledRect( 0, 0, 20+w-1, h-1, bcol );
	for( i=0; i<g_shadowSize;i++ ){
		PlotDrawLine( i, g_shadowSize, 0, g_shadowSize-i, gradcolors[i] );	// top curve
		PlotDrawLine( i, g_shadowSize, i, h, gradcolors[i] );		// shadow
	}
	FW_SaveImage( VDptr, "rshad" );
}

#define DONT_DRAW_GRAPH_IN_SEP_FILE -1

FILE *StandardPlot::Image_InitFile( VDinfoP VDptr, struct graphOptions *graph, long imf )
{
	FILE *out;
	char			tempSpec[512];


	//
	// save output to gif on the disk
	//
	mystrcpy(tempSpec,MyPrefStruct.outfile);
	if( graph && MyPrefStruct.ShowVirtualHostGraph() )
	{
		char tmpStr[512], *domName;
		if( graph->type == GRAPH_MULTIVHOST3D )
			domName = graph->filename;
		else
			domName = tmpStr;
			
		sprintf( tmpStr, "%s%c%s", GetDomainPath(VDptr), PATHSEP, graph->filename);
		CopyFilenameUsingPath( tempSpec, MyPrefStruct.outfile, domName );
		if( graph->type != GRAPH_MULTIVHOST3D )
			ReplacePathFromHost( GetDomainPath(VDptr), tempSpec );
	} else
		CopyFilenameUsingPath( tempSpec, MyPrefStruct.outfile, graph->filename );

	out = NULL;

	// if file format is RTF make image format PNG
	switch( MyPrefStruct.report_format )
	{
		case FORMAT_PDF:
		case FORMAT_EXCEL:
		case FORMAT_COMMA: imf = DONT_DRAW_GRAPH_IN_SEP_FILE; break;
		case FORMAT_RTF: imf = FORMAT_PNG; break;
		case FORMAT_HTML: 
			if( imf > FORMAT_PNG )
				imf = FORMAT_GIF;
			break;
	}

	// Special case for virtual hosts graph, 
	if( graph->type == GRAPH_MULTIVHOST3D && imf == DONT_DRAW_GRAPH_IN_SEP_FILE )
		imf = FORMAT_GIF;

	switch( imf )
	{
		case 0:	strcat( tempSpec, ".gif");	out = Fopen( tempSpec, "wb") ; break;
		case 1:	strcat( tempSpec, ".jpg");	out = Fopen( tempSpec, "wb") ; break;
		case 2:	strcat( tempSpec, ".png");	out = Fopen( tempSpec, "wb") ; break;
		case 3:	strcat( tempSpec, ".bmp");	out = Fopen( tempSpec, "wb") ; break;
		case 4:	strcat( tempSpec, ".svg");	out = Fopen( tempSpec, "wb") ; break;
	}
	return out;
}


long SaveandCloseImageFile( FILE *out , gdImagePtr im, long imf )
{
	if( out && im )
	{
		gdImageInterlace(im, 1); //interlace image

		switch( imf )
		{
			case 0:	gdImageGif(im, out, 0 );break;
			case 1:	gdImageJPG(im, out, 95 );break;		// Future file formats
			case 2:	gdImagePNG(im, out ); break;
			case 3:	gdImageBMP(im, out ); break;
		}
		// close Image FILE
		if( out )
			fclose(out);
		gdImageDestroy(im);
	}
	else if( im )
		gdImageDestroy(im);
	return 0;
}

void StandardPlot::Image_CompleteThumbNail( VDinfoP	VDptr, struct graphOptions *graph, gdImagePtr im )
{
	long w,h;

	w = im->sx * 0.333;
	h = im->sy * 0.333;

	if( MyPrefStruct.graph_wider )
	{
		w = w / 1.5;
		h = h / 1.5;
	}

	gdImagePtr newImage = CreateThumbImageFromGD( im, w, h );

	if( newImage )
	{
		// ********************************************************************************
		// We are bastardizing the graph structure's filename, so we are going to assign
		// it to a local variable.  We ARE however setting it back before we return!
		// ********************************************************************************
		char*	szEvilStringPointer = graph->filename;

		char thumbFile[256];
		sprintf( thumbFile, "%s_t", graph->filename );
		graph->filename = thumbFile;
		FILE *fp = Image_InitFile( VDptr, graph, 1 );
		if( fp )
		{
			gdImageJPG(newImage, fp, 75 /*compression level*/ );
			fclose(fp);
			gdImageDestroy(newImage);
		}

		// ********************************************************************************
		// Restore the filename member.
		// ********************************************************************************
		graph->filename = szEvilStringPointer;
	}
}

void StandardPlot::Image_CompleteFile( FILE *out )
{
	SaveandCloseImageFile( out, im, MyPrefStruct.image_format );
	im = NULL;
}

// draw base layer with bevel
void StandardPlot::Graph_DrawBase( struct graphOptions *graph, int w, int h, int fill )
{
	static long basecolors[32];
	int retVal = 1;

	if( fill == 1 )
		PlotFilledRect( 0, 0, w-1, h-1, base_col );
	else
	if( fill == 2 ){
		long gradLevel = 32, lastcol, count;
		long x=0,y=0,x2=w,ysize=h;
		for( count=0; count<gradLevel; count++){
			long newcolor; double scale;
			//graduation percentage
			scale = ((count/(double)gradLevel)-0.5) * 15;
			newcolor = ScaleRGB( 100-scale, graph->base_color );
			basecolors[count] = PlotImageColorAllocate( RGB_RED(newcolor), RGB_GREEN(newcolor), RGB_BLUE(newcolor) );

			lastcol = basecolors[count];
			PlotFilledRect( x, y+(count*(ysize/gradLevel)), x2, y+((1+count)*(ysize/gradLevel)), lastcol );
		}
		if (retVal)
			PlotFilledRect( x, y+(count*(ysize/gradLevel)), x2, y+ysize, lastcol );
	} else
	if( fill == 3 ){
		PlotFilledRect( 0, 0, w-1, h-1, white );			// Should be Prefs->RGBTable[7]
	}
	w--;h--;

	if( MyPrefStruct.corporate_look == FALSE )
	{
		PlotFilledRect( 0, 0, w, 2, borderhi_col ); // Thin Solid Rec across top
		PlotFilledRect( 0, 0, 2, h, borderhi_col ); // Thin Solid Rec down left
		PlotFilledRect( 0, h-1, w, h+1, borderlo_col ); // Thin Solid Rec across bottom
		PlotFilledRect( w-1, 0, w+1, h+1, borderlo_col ); // Thin Solid Rec down right
	}

	PlotRect( 0, 0, w+1, h+1, border_col ); // Rectange around the graph
}


// draw base layer for white corprate look/n/feel
void StandardPlot::Graph_WhiteDrawBase( struct graphOptions *graph, int w, int h, int fill )
{

	{
		long y;

		PlotFilledRect( 0, 0, w-1, h-1, white );

		y = (graph->yborder/4) + 15;
	}
}


#define	SHOWPOLY(pol,count) {int ii;for(ii=0;ii<count;ii++) printf( "POLY %d = (%f,%f)\n", ii,pol[ii].x,pol[ii].y );}

// draw an axis for graph
void StandardPlot::Graph_Draw3DAxis( struct graphOptions *graph, int style, int ixsize, int iysize, int backcolor, int gcol )
{
	float	x,y,xstep,ystep,
			xunits,	yunits,
			xsize,ysize,zsize,
			x2, y2;
	gdPoint	polyb[4], polyl[4], polyr[4];

	xunits = graph->xunits;
	yunits = graph->yunits;
	x = graph->xborder;
	y = graph->yborder;

	xsize = ixsize;
	ysize = ixsize/2.0;
	zsize = MULTI3D_HEIGHT;
	xstep = (xsize) / (float)xunits;
	ystep = (zsize) / (float)yunits;
	x2 = x+(xsize/2.0);
	y2 = y+(ysize/2.0);
	
	polyb[0].x = x2;		polyb[0].y = y;
	polyb[1].x = x+xsize;	polyb[1].y = y2;
	polyb[2].x = x2;		polyb[2].y = y+ysize;
	polyb[3].x = x;			polyb[3].y = y2;
	
	PlotImageFilledPolygon( polyb, 4, backcolor );
	PlotImagePolygon( polyb, 4, border_col );

	
	polyl[0].x = x;			polyl[0].y = y2;
	polyl[1].x = x;			polyl[1].y = y2-zsize;
	polyl[2].x = x2;		polyl[2].y = y-zsize;
	polyl[3].x = x2;		polyl[3].y = y;
	
	PlotImageFilledPolygon( polyl, 4, bar1 );
	PlotImagePolygon( polyl, 4, border_col );

	
	polyr[0].x = x+xsize;	polyr[0].y = y2;
	polyr[1].x = x+xsize;	polyr[1].y = y2-zsize;
	polyr[2].x = x2;		polyr[2].y = y-zsize;
	polyr[3].x = x2;		polyr[3].y = y;

	PlotImageFilledPolygon( polyr, 4, bar2 );
	PlotImagePolygon( polyr, 4, border_col );

	
	// draw X-axis ticks
	{
		float	lastx=0,lastx2=0,xpos, x1,x2,x3,y1,y2,y3;
		long count=0;
		while ( count < xunits ){
			xpos = (count*xstep/2.0);
			x1 = x+xpos; y1 = y+(ysize/2.0)+(xpos/2.0);
			x2 = x+xpos+(xsize/2.0); y2 = y+(xpos/2.0);
			x3 = x+xpos+(xsize/2.0); y3 = y+(xpos/2.0)-zsize;
			if (graph->tickx){
				if( xstep>14 ){
					PlotDottedLine( x1, y1, x2, y2, gcol );
					PlotDottedLine( x2, y2, x3, y3, gcol );
				} else if( xstep>1 ){
					PlotDrawLine( x1, y1, x2, y2, gcol );
					PlotDrawLine( x2, y2, x3, y3, gcol );
				}
				lastx = xpos;
			}
			count++;
		}
	}
	// draw Y-axis ticks
	{
		float ypos;
		long count=0;
		while (count<yunits) {
			ypos = (count*ystep);
			if (graph->ticky) {
				PlotDrawLine( x, y+(ysize/2)-ypos, x+(xsize/2), y-ypos, gcol );
				PlotDrawLine( x+xsize, y+(ysize/2)-ypos, x+(xsize/2), y-ypos, gcol );
			}
			count++;
		}
	}
	return;
}

void StandardPlot::DefineLabels( struct graphOptions *graph,__int64 value,char **xaxis,char **yaxis,long translateIdOverride/*=0*/)
{
	//on entry xaxis contains fixed variable, yaxis the variable
	
	if( graph->type == GRAPH_SCATTER )
		mystrcpy(*yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_PAGES ) );
	else
	switch (graph->scaler_type) {
		case SORT_BYTES:
			mystrcpy( *yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_BYTESDELIVERED ) );
			if (value<1024*10)
			{
				strcat( *yaxis, "(" );
				strcat( *yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_BYTES ) );
				strcat( *yaxis, ")" );
			}
			else
			if (value<(1024*1024))
			{
				strcat( *yaxis, "(" );
				strcat( *yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_KB ) );
				strcat( *yaxis, ")" );
			}
			else
			if (value<(1024*1024*1024))
			{
				strcat( *yaxis, "(" );
				strcat( *yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_MB ) );
				strcat( *yaxis, ")" );
			}
			else
			{
				strcat( *yaxis, "(" );
				strcat( *yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_GB ) );
				strcat( *yaxis, ")" );
			}
			break;
		case SORT_VISITIN:
		case SORT_SESSIONS:
			mystrcpy(*yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_SESSIONS ) );
			break;
		case SORT_COUNTER:
			mystrcpy(*yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_VISITORS ) );
			break;
		case SORT_PAGES:
			mystrcpy(*yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_PAGES ) );
			break;
		case SORT_ERRORS:
			mystrcpy(*yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_ERRORS ) );
			break;

		case SORT_REQUESTS:
		default:
			if( graph->report_id == PAGESFIRST_PAGE )
				mystrcpy( *yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_FIRSTSESSIONS ) );
			else
			if( graph->report_id == PAGESLAST_PAGE )
				mystrcpy( *yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_LASTSESSIONS ) );
			else
				mystrcpy( *yaxis, Translate( translateIdOverride ? translateIdOverride : LABEL_REQUESTS ) );


			if (value>=1000000)
			{
				mystrcat( *yaxis, " (X " );
				mystrcat( *yaxis, Translate( LABEL_MILLIONS ) );
				mystrcat( *yaxis, ")" );
			}
			break;
	}
	if (graph->type==GRAPH_HBAR) //if it is a horizontal bar graph switch labels
		SWAP(*xaxis,*yaxis)
}



void ValuetoString( long scaler_type, __int64 hvalue, __int64 current_value, char *name ){
	switch( scaler_type ){
		case SORT_BYTES:
			if( hvalue < 1024 )
				sprintf( name, "%d", (long)(current_value) );
			else
			if( hvalue < 1024*1024 )
				sprintf (name, "%.1f%s", (double)current_value/1024.0, TranslateID(LABEL_KB));		// sprintf( name, "%.1fKb", (double)current_value/1024.0 );
			else
			if( hvalue < 1024*1024*1024 )
				sprintf( name, "%.1f%s", (double)current_value/(1024*1024), TranslateID(LABEL_MB));			// sprintf( name, "%.1fMb", (double)current_value/(1024*1024) );
			else
				sprintf( name, "%.2f%s", (double)current_value/(1024*1024*1024), TranslateID(LABEL_GB));		// sprintf( name, "%.2fGb", (double)current_value/(1024*1024*1024) );
			break;
		case SORT_PAGES:
		case SORT_ERRORS:
		case SORT_REQUESTS:
			if (hvalue<1000000)
				sprintf( name, "%d", (long)current_value );
			else
				sprintf( name, "%.2fM", current_value/1000000.0 );
			break;
		default:
			sprintf( name, "%d", (long)current_value );
			break;
	}
}



// draw background gradient or plain area setting for the data to go on top of
void StandardPlot::Graph_DrawBackground(struct graphOptions *graph, int style, int xsize, int ysize, int color, int backcolor, int gridcolor )
{
	double	xstep,ystep;
	int		count;
	long	gridx = graph->gridx,
			gridy = graph->gridy,
			tickx = graph->tickx,
			ticky = graph->ticky,
			xunits = graph->xunits,
			yunits = graph->yunits,
			x = graph->xborder,
			y = graph->yborder,
			x2 = graph->xborder + xsize,
			y2 = graph->yborder + ysize;
	long	maxperc = 60;
	long	newcolor;
	int		vertical = 0;
	double	scale;

	xstep = (xsize) / (double)xunits;
	ystep = (ysize) / (double)yunits;
	

	if( graph->type == GRAPH_HBAR )
		vertical = 0;
	else
		vertical = 1;

	//if( MyPrefStruct.corporate_look ){
	//	bgcolors[0] = white; 
	//	PlotFilledRect( x, y, x2, y+ysize, bgcolors[0] );
	//} else
	// draw smooth gradient for background of the graph but only when pallete is custom
	if( MyPrefStruct.paletteType == GRAPH_CUSTOMCOLOR ){
		// define color table
		for( count=0; count<GRADIENT_STEPS; count++)
		{
			scale = ((count/(double)GRADIENT_STEPS)) * maxperc;
			if( MyPrefStruct.corporate_look ){
				newcolor = ScaleRGBtoRGB( 100-(.5*scale), graph->back_color, 0xFFFFFF );
			} else
				newcolor = ScaleRGB( (100+(.5*maxperc))-scale, graph->back_color );
			bgcolors[count] = PlotImageColorAllocate( RGB_RED(newcolor), RGB_GREEN(newcolor), RGB_BLUE(newcolor) );
		}
		// Draw rectangles
		for( count=0; count<GRADIENT_STEPS; count++)
		{
			if( vertical )
				PlotFilledRect( x, y+(count*(ysize/GRADIENT_STEPS)), x2, y+((1+count)*(ysize/GRADIENT_STEPS)), bgcolors[count] );
			else
				PlotFilledRect( x+(count*(xsize/GRADIENT_STEPS)), y, x+((1+count)*(xsize/GRADIENT_STEPS)), y2, bgcolors[GRADIENT_STEPS-1-count] );
		}
		if( vertical )
			PlotFilledRect( x, y+(count*(ysize/GRADIENT_STEPS)), x2, y+ysize, bgcolors[GRADIENT_STEPS-1] );
		else
			PlotFilledRect( x+(count*(xsize/GRADIENT_STEPS)), y, x2, y2, bgcolors[0] );
	} else 
	{
		bgcolors[0] = backcolor;
		PlotFilledRect( x, y, x2, y+ysize, bgcolors[0] );
	}
	PlotRect( x, y, x2, y+ysize, color );

	return;
}


// draw the labels and axis dots
void StandardPlot::Graph_AxisLabels( VDinfoP VDptr, struct graphOptions *graph ,__int64 hvalue,__int64 hcount, short units, int color, int gridcolor )
{
	short 	index=0;
	int		count, indent = GRF_AXISINDENT, minx=3, linecolor;
	__int64	value;
	double	div=1, xstep,ystep;
	char	number[256];
	float	ypos,xpos;
	long	gridx = graph->gridx,
			gridy = graph->gridy;
	float	xunits = graph->xunits,
			yunits = graph->yunits,
			tickx = graph->tickx,
			ticky = graph->ticky,
			x = graph->xborder,
			y = graph->yborder,
			xsize = (graph->width - graph->xborder-10),
			ysize = (graph->height-(2*graph->yborder)),
			x2 = graph->xborder + xsize,
			y2 = graph->yborder + ysize,
			lastx=0;

	// Plot horizontal numbers on graph
	if( graph->xlabels ){
		switch (graph->type) {
			case GRAPH_SCATTER: {
					double meanlen, meanpages;
					long mins, i, i2, x2;
					char name[256];
					meanlen = VDptr->byClient->GetStatListTotalTime()/VDptr->byClient->GetStatListTotalVisits();
					meanpages = VDptr->byClient->GetStatListTotalCounters4()/(double)VDptr->byClient->GetStatListTotalVisits();
					if( meanlen < hcount/5 ) hcount = meanlen*5;
					if( meanpages < hvalue/10 ) hvalue = roundvalue(meanpages*10);
					mins = (hcount)/60.0;
					if( mins ){
						i2 = 1;
						xstep = xsize/(double)(mins/(double)i2);
						if( xstep < 8 ) { i2 = 10; xstep = xsize/(mins/i2); }
						if( xstep < 8 ) { i2 = 60; xstep = xsize/(mins/i2); }
						ypos = (graph->height - graph->yborder);			// calc bar position Y
						for( i=0; i< mins; i+=i2 ){
							x2 = graph->xborder + ((i/i2)*xstep);
							if( (x2-lastx) > 16 ){
								PlotDottedLine( x2, ypos-ysize, x2, ypos+3, gridcolor );
								sprintf( name, "%d", i );
								PlotNum( 1, x2-6-((-GetStringLen(name))/2), ypos+5, name, label_col );	// draw the current bars NAME/FILE
								lastx = x2;
							}
						}
					}
				}
				break;
			case GRAPH_PIE:
				tickx = ticky = 0;
				break;

			case GRAPH_LINE: //line graph
				if( graph->report_id==DATE_PAGE || graph->report_id==RECENTDATE_PAGE ){  //byDate
					long	i, y1;
					struct tm date, datelast;

					linecolor = bar1;

					xstep = (xsize) / (double)xunits;
					ypos = (graph->height - graph->yborder);
					memset( (void*)&datelast, 0xff, sizeof(struct tm) );

					// plot X values on axis
					for( i=0; i<xunits; i++ ){
						xpos = graph->xborder + (i*xstep);
						y1 = ypos+2;
						number[0] = 0;
						DaysDateToStruct( graph->firstTime+(i*ONEDAY), &date ); 
						// plot days
						if( graph->report_id == DATE_PAGE || graph->report_id==RECENTDATE_PAGE ){
							if( xstep>1 ) PlotDrawLine( xpos, ypos, xpos, ypos+2, linecolor );
							if( xpos-lastx>12 ){ //plot days of month
								sprintf( number, "%d", date.tm_mday );
								PlotNumC( 3, xpos, y1, number, label_col );
								lastx = xpos;
							}
							y1+=8;
						}
						// plot months
						if( xstep>(18/31.0) && (date.tm_mon != datelast.tm_mon) ){ //plot months
							if( !number[0] ) { y1+=3; PlotDrawLine( xpos, ypos, xpos, y1-1, linecolor ); }
							mystrncpyNull( number, GetMonthTranslatedByMonthNum(date.tm_mon), BUF16 );
							PlotTransStringC( 3, xpos, y1, number, label_col, REM_HTML_TOKENS ); y1+=9;
						}
						// plot years
						if( xstep>(24/365.0) && (date.tm_year != datelast.tm_year) ){ //plot days of month
							if( !number[0] ) { y1+=2; PlotDrawLine( xpos, ypos, xpos, y1-1, linecolor ); }
							sprintf( number, "%d", date.tm_year );
							PlotNumC( 3, xpos, y1, number, label_col); y1+=9;
						}
						DaysDateToStruct( graph->firstTime+(i*ONEDAY), &datelast );
					}
				}

				break;

			case GRAPH_MULTI3D:
			case GRAPH_MULTIVHOST3D:
				{
					long	i;
					float	x1,y1;
					struct tm date, datelast;

					xstep = (xsize) / (double)xunits;
					ysize = xsize/2.0;
					memset( (void*)&datelast, 0xff, sizeof(struct tm) );

					for( i=0; i<=xunits; i++ ){
						xpos = (i*xstep/2.0);
						x1 = x+xpos; y1 = y+(ysize/2.0)+(xpos/2.0);
						xpos = x1; ypos = y1;

						number[0] = 0;
						DaysDateToStruct( graph->firstTime+(i*ONEDAY), &date ); 
						if( xstep>1 ) PlotDrawLine( xpos, ypos, xpos, ypos+2, border_col );
						if( xpos-lastx>10 ){ //plot days of month
							sprintf( number, "%d", date.tm_mday );
							PlotNumC( 3, xpos, ypos+3, number, label_col ); ypos+=8;
							lastx = xpos;
						}
						if( xstep>(18/31.0) && (date.tm_mon != datelast.tm_mon) ){ //plot months
							if( !number[0] ) { ypos+=9; PlotDrawLine( xpos, y1, xpos, ypos-1, border_col ); }
							mystrncpyNull( number, GetMonthTranslatedByMonthNum( date.tm_mon ), BUF16 );
							PlotTransStringC( 3, xpos, ypos+3, number, label_col, REM_HTML_TOKENS ); ypos+=9;
						}
						if( xstep>(24/365.0) && (date.tm_year != datelast.tm_year) ){ //plot days of month
							if( !number[0] ) { ypos+=2; }
							sprintf( number, "%d", date.tm_year );
							PlotNumC( 3, xpos, ypos+3, number, label_col); ypos+=9;
						}
						DaysDateToStruct( graph->firstTime+(i*ONEDAY), &datelast );
					}
				}
				tickx = ticky = 0;
				break;
		}
	
	}

	// plot vertical numerical labels
	if( graph->ylabels ){
		xstep = xsize/(double)xunits;
		ystep = ysize/(double)yunits;
		while ( index < units) {
			long width;
			value = ((hvalue)*(units-index)) / units;		//calculate values
			ValuetoString( graph->scaler_type, hvalue, value, number );
			switch (graph->type) {
				case GRAPH_MULTIVBAR4: //verticle bar graph 4
				case GRAPH_VBAR: //verticle bar graph
				case GRAPH_LINE: //line graph
				case GRAPH_MULTILINE: //line graph
				case GRAPH_SCATTER:
					ypos = graph->yborder+ (long)(index*ystep);
					PlotNum( 1, graph->xborder-GetStringLen(number)- 5, ypos-3, number, color );
					break;
				case GRAPH_MULTI3D:
				case GRAPH_MULTIVHOST3D:
					width = GetStringLen(number);
					ysize = xsize/2.0;
					ystep = (MULTI3D_HEIGHT) / (double)yunits;
					ypos = (graph->yborder + (ysize/2) - (MULTI3D_HEIGHT)) + (long)(index*ystep);
					PlotNum( 3, graph->xborder+2, ypos - 9, number, color );

					// left numbers
					ypos = (graph->yborder + (ysize/2) - (MULTI3D_HEIGHT)) + (long)(index*ystep);
					PlotNum( 3, (graph->width - graph->xborder+15) - width , ypos-4, number, color);

					// middle numbers
					ypos = (graph->yborder - (MULTI3D_HEIGHT)) + (long)(index*ystep);
					PlotNum( 3, ((xsize/2) + 24) - width, ypos-4, number, color);
					tickx = ticky = 0;
					break;
				case GRAPH_HBAR: //horizontal bar graph
					xpos = graph->xborder+ (long)((units-index)*xstep);
					double xAdjust = GetStringLen(number);
					PlotNum( 1, xpos-xAdjust, graph->yborder+ysize+4, number, color);
					break;
			}
			index++;
		}
	}


	if( MyPrefStruct.paletteType != GRAPH_WEBCOLOR )
		gridcolor = bgcolors[64-1];
	if( MyPrefStruct.corporate_look )
		gridcolor = lgrey;
	
	xstep = xsize/(double)xunits;
	ystep = ysize/(double)yunits;
	// draw Y-axis ticks (horizontal lines)
	if( ticky ) {
		count=yunits;
		while (count>=0) {
			PlotDrawLine( x-GRF_AXISINDENT, y+(count*ystep), x, y+(count*ystep), color);
			if (gridy) PlotDottedLine( x, y+(count*ystep), x2-2, y+(count*ystep), gridcolor );
			count--;
		}
	}

	// draw X-axis ticks (vertical lines)
	if( tickx ) {
		lastx = 0;
		//if (it is the date graph) >31 days  recalculate ticks
		count=xunits;
		while (count>0) {
			xpos = x+(count*xstep);
			if( abs(xpos-lastx)>minx ){
				PlotDrawLine( xpos, y2, xpos, y2+indent, color );
				lastx = xpos;
			}
			if (gridx) PlotDottedLine( x+(count*xstep), y, x+(count*xstep), y2, gridcolor );
			count--;
		}
	}

	// Draw axis border lines
	if( tickx ) PlotDrawLine( x, y, x, y2+GRF_AXISINDENT, color );  			// draw Y-axis 
	if( ticky ) PlotDrawLine( x-GRF_AXISINDENT, y2, x2, y2, color );  	 	// draw X-axis
}






// draw the colored legends showing what each color means
void StandardPlot::Graph_Legend( struct graphOptions *graph, long multi_cols[MULTI_COLS*3] )
{
	long	x,y, lp;
	long	ysize,width,height;

	ysize = (graph->height - (2*graph->yborder));
	width = graph->width;
	height = graph->height;
	x = width-80;
	y = graph->yborder+4;

	switch (graph->type) 
	{
		case GRAPH_LINE:
			long labelIDs[4];

			x = width-120;
			y = 6;

			if( graph->report_id == DATE_PAGE || graph->report_id == RECENTDATE_PAGE )
			{
				labelIDs[0]= LABEL_TRAFFIC;
				labelIDs[1]= LABEL_ERRORS;
				labelIDs[2]= LABEL_AVERAGE;
				labelIDs[3]= 0;
			} else
			{
				labelIDs[0]= LABEL_TRAFFIC;
				labelIDs[1]= LABEL_ERRORS;
				labelIDs[2]= 0;
			}
			lp = 0;

			while( labelIDs[lp] )
			{
				char	labelName[64];
				long	labColors[3];

				mystrcpy( labelName, Translate(labelIDs[lp]) );
				if( MyPrefStruct.corporate_look ){
					labColors[0] = black;
					labColors[1] = red;
					labColors[2] = blue;
				} else {
					labColors[0] = bar1;
					labColors[1] = bar2;
					labColors[2] = bar3;
				}
				PlotFilledRect( x, y+(lp*13)+4, x+23, y+(lp*13)+5, labColors[lp] );

				PlotStringConvertHTMLTokens( 1, x+28, y+(lp*13)-2, labelName, label_col );
				lp++;
			}
			break;

		case GRAPH_MULTIVBAR4:
			x = width-120;
			y = graph->yborder+4;
			for (lp=0; lp<3; lp++)
			{
				PlotFilledRect( x-1, y+(lp*13)-1, x+8+1, 8+y+(lp*13)+1, border_col ); // This will look like a border
				PlotFilledRect( x, y+(lp*13), x+8, 8+y+(lp*13), multi_cols[10+6+lp] ); // This will be the filled color 
				if( MyPrefStruct.corporate_look )
					PlotString( 1, x+13, y+(lp*13)-2, OSNames[lp], black );
				else
					PlotString( 1, x+13, y+(lp*13)-2, OSNames[lp], white );
			}
			break;
	}
}

extern void GetCorrectSiteURL( VDinfoP VDptr, char *host );
//
//
//
//
// Draw the various data graphs here
//
// -------------------------------------------------------------------------------------------
//
void StandardPlot::Graph_HBar( struct graphOptions *graph, long index, long itempos, __int64 value, __int64 current_value, double ystep, long bar1, long bar2, long bar3 )
{
	int		style = TruncateStyle();
	long	xpos,ypos,barsize, barthickness;
	double xsize, wordsize, space;
	char	name[2048];
	std::string text;

	xsize = (graph->width - graph->xborder-10);
	xpos = (graph->xborder);				// calc bar position X
	ypos = graph->yborder+ (int)(index*ystep);		// calc bar position Y
	barsize = ((xsize*value)/100);	// calc bar size
	barthickness = (int)ystep-4;

	if( barthickness > 20 ) barthickness = 20;

	PlotHorizbar( xpos+1,ypos, barsize, barthickness, bar1, bar2, bar3, graph->depth );	// draw bar

	// only do the labels if on and if the value is valid
	if( graph->ylabels && current_value>=0 ) 
	{
		long namelen;
		
		namelen = mystrncpy( name, graph->byStat->GetName(itempos), 2000 );
		name[namelen] = 0;

		switch( graph->report_id )
		{
			// ----- Get remote page titles for these....
			case PAGES_PAGE:
			case PAGEHIST_PAGE:
			case PAGESLEAST_PAGE:
			case PAGESFIRST_PAGE:
				if( MyPrefStruct.page_remotetitle )
				{
					char thishost[DOMAINNAME_LENGTH+1], url[2048+DOMAINNAME_LENGTH+1];
					GetCorrectSiteURL( graph->Vhost, thishost );
					MakeURL( url, thishost, name );
					HTTPURLGetTitle( url, name );
					style = 1;
				}
				break;

			case CLIENT_PAGE:
			case CLIENTSTREAM_PAGE:
			case DOMAIN_PAGE:
			case SECONDDOMAIN_PAGE:
			case KEYVISITORS_PAGE:
				if( !isdigit( *name ) )
					ReverseAddress(name, name );
				break;
			case DOWNLOAD_PAGE:
			case DIRS_PAGE:
			case TOPDIRS_PAGE: 
			case ORGNAMES_PAGE:
			case ERRORS_PAGE:
			case ERRORURL_PAGE:
			case FILE_PAGE:
				if( name[ namelen-1 ] != '/' &&  name[ namelen-1 ] != '\\' ){
					stripURL(name); //incase it is a directory
					FileFromPath(name,name);
				}
				break;
		}
		
		space = ScaleXNum( graph->xborder ) - ScaleXNum( 25 );

		if( strstr( name, "http://" ) )
			mystrcpy( name, name + 7 );

		if( graph->byStat->GetNameTrans ) // We need to use translation string functions
		{
			wordsize = GetTransStringLen( name, 1 );
			if( wordsize > space )
				text = SetTransStringLen( name, space, 1, style );
			else
				text = name;
			PlotTransStringRightJustified( 1, xpos-4, ypos+2, text.c_str(), label_col );
		}
		else
		{
			wordsize = GetStringLen( name, 1 );
			if( wordsize > space )
				text = SetStringLen( name, space, 1, style );
			else
				text = name;
			PlotStringRightJustified( 1, xpos-4, ypos+2, text.c_str(), label_col );
		}


		if( graph->xlabel_value )
		{
			ValuetoString( graph->scaler_type, current_value, current_value, name );
			xpos = xpos + barsize;
			if( value > 90 )
				PlotNumRightJustified( 1, xpos-5, ypos+2, name, label_col );
			else {
				if( MyPrefStruct.corporate_look )
					PlotNum( 1, xpos+4, ypos+2, name, black );
				else
					PlotNum( 1, xpos+4, ypos+2, name, white );
			}
		}
	}
}


void StandardPlot::Graph_VBar( struct graphOptions *graph, long index, long itempos, __int64 value, __int64 current_value, double xstep, long bar1, long bar2, long bar3 )
{
	double	xpos,ypos,barsize;
	long	ysize,width;
	char	name[2048];
	int		barthickness;

	ysize = (graph->height - (2*graph->yborder));
	xpos = graph->xborder + (index*xstep);		// calc bar position X
	ypos = (graph->height - graph->yborder);			// calc bar position Y
	barsize = ((ysize*value)/100);	// calc bar size

	// draw current sample's bar
	barthickness = (int)xstep - 4;
	if( barthickness < 3 ) barthickness = 3;

	PlotVertbar( xpos+1,ypos-barsize, barthickness-graph->bar_space, barsize, bar1, bar2, bar3, graph->depth );

	// only do the labels if on and if the value is valid
	if( graph->xlabels  ) {
		int i = 0;
		char *nameStr;

		if( graph->report_id == WEEK_PAGE )
		{
			// offset the current day index according to user's first day of week setting, so that
			// the specified day of week appears first within the weekly traffic reports
			size_t dayIndex=(itempos+MyPrefStruct.firstDayOfWeek)%7;
			nameStr = GetWeekdayTranslatedByDayNum( dayIndex );
		}
		else if( graph->report_id == MONTH_PAGE )
			nameStr = GetMonthTranslated( graph->byStat->GetDay(itempos) );
		else
			nameStr = graph->byStat->GetName( itempos );
		if( (long)nameStr != 1 && nameStr )
			i = mystrncpy( name,nameStr, 2000 );
		name[i] = 0;

		width = GetStringLen(name);
		if( (width) > (barthickness+graph->bar_space) ){
			char *p = mystrchr( name, ' ' );
			if( p ) {
				*p = 0;
				width = GetStringLen(name);
			} else {
				name[2] = 0;
				width = GetStringLen(name);
			}
			if( (width) > (barthickness+graph->bar_space) ){
				name[3]=0;
				width = GetStringLen(name);
				if( (width) > (barthickness+graph->bar_space) )
					name[1] = 0;
			}
		}
		PlotTransString( 1, xpos+(((int)(xstep-11)-width)/2), ypos+5, name, label_col, 0, 0, REM_HTML_TOKENS );	// draw the current bars NAME/FILE
	}
	// labels on top of the actual Bars
	if( graph->ylabel_value && current_value>=0 )
		PlotNum( 0, xpos+10, ypos-barsize-8, graph->bartitle, label_col );
}


void StandardPlot::Graph_Line(
	VDinfoP	VDptr,
	struct graphOptions *graph,
	long index,
	long itempos,
	__int64 value,
	__int64 value2,
	__int64 value3,
	__int64 sumvalue,
	double xstep,
	long num )
{
static
	long	lastx,lastx2,lastx3, lasty,lasty2,lasty3,lasty4;
	long	xpos,ypos,barsize,ysize, averagecolor;
	char	*namePtr;

	ypos = (graph->height - graph->yborder);
	ysize = (graph->height - (2*graph->yborder));

	if( graph->report_id==DATE_PAGE || graph->report_id==RECENTDATE_PAGE ){  //byDate
		long day;
		day = graph->byStat->GetDay( itempos );
		if( day <= (86400*365*10) || day > (86400*365*40) ) return;

		xpos = (day/ONEDAY) - (graph->firstTime/ONEDAY);
		//xpos = (long)( ((long)(day-0.5)-(long)(graph->firstTime-0.5)));
		xpos *= xstep;
		if( xpos <= 0)	index = 0;
	} else
		xpos = (long)(index*xstep);
	xpos+=graph->xborder;

	if( index == 0 ){
		lasty4 = lasty3 = lasty2 = lasty = ypos;
		lastx = graph->xborder;
		lastx2 = lastx3 = 0;
	}

	if( MyPrefStruct.stat_style != STREAM ) 
	{
		//==== Plot second value data line , in this case its usually bytes value.
		barsize = ((ysize*value2)/100);
		if (index && barsize){
			gdPoint p[5];
			p[0].x = lastx; p[0].y = ypos;
			p[1].x = lastx; p[1].y = ypos-lasty4;
			p[2].x = xpos;	p[2].y = ypos-barsize;
			p[3].x = xpos;	p[3].y = ypos;
			p[4].x = lastx; p[4].y = ypos;
			gdImageFilledPolygon( im, p, 5, blue );
		}
		lasty4 = barsize;
	}


	//==== Plot average value line
	if( graph->report_id!=HOUR_PAGE ){
		barsize = (long)((ysize*(sumvalue/(index+1)))/100);
		if( MyPrefStruct.corporate_look )
			averagecolor = blue;
		else
			averagecolor = bar3;
		if (index)
			PlotDrawLine( lastx, ypos-lasty2, xpos, ypos-barsize, averagecolor );
		lasty2 = barsize;
	}

	//==== Plot Error value line if data exists
	barsize = ((ysize*value3)/100);
	if (index && (barsize || lasty3) )
		PlotDrawLine( lastx, ypos-lasty3, xpos, ypos-barsize, bar2 );
	lasty3 = barsize;

	//==== Plot main value data line
	barsize = ((ysize*value)/100);
	if (index)
		PlotDrawLine( lastx, lasty, xpos, ypos-barsize, bar1 );

	// ----- draw labels
	if( graph->xlabels && xpos>=graph->xborder ){
		if( graph->report_id!=DATE_PAGE && graph->report_id!=RECENTDATE_PAGE ){  //byMonth
			namePtr = graph->byStat->GetName(itempos);
			PlotNum( 1, xpos-10, ypos+4, namePtr, label_col ) ;	// draw the current bars NAME/FILE
		}
	}
	if(index > 0)
		lastx=xpos;
	lasty=ypos-barsize;
}


void StandardPlot::Graph_MultiLine( VDinfoP	VDptr, struct graphOptions *graph,long index, __int64 value, double xstep, long col )
{
static
	long	lastx,lastx2, lasty,lasty2, lastv;
	long	xpos,ypos,barsize,ysize;

	if( index == 0 ){
		lastv = lastx = lastx2 = lasty2 = lasty = 0;
	}

	ysize = (graph->height - (2*graph->yborder));
	xpos = graph->xborder+(int)(index*xstep);
	ypos = graph->height - graph->yborder;
	barsize = ((ysize*value)/100);
	lasty2 = (ypos-barsize);
	ypos = ypos-barsize;

	//only plot after first x,y is obtained
	if (index) {
		//printf( "val=%d, %d,%d   %d,%d\n", value, lastx, lasty, xpos, ypos-barsize );
		if( lastv || value )
			PlotDrawLine( lastx, lasty, xpos, ypos, col );
	}
	lastx=xpos;
	lasty=ypos;
	lastv=value;
}

void StandardPlot::Graph_Multi3DLine( VDinfoP	VDptr, struct graphOptions *graph,long index, long daynum, long maxitems, __int64 value, double xstep, long multi_cols[MULTI_COLS*3] )
{
static
	float	lastx,lastx2, lasty,lasty2, lastv;
	long	col,col2;
	float	xpos,ypos,barsize,ysize,xsize,zsize, bwid = 4,
			x = graph->xborder,
			y = graph->yborder;
	double	ystep, zstep, ntime;

	xsize = (graph->width - graph->xborder-10);
	ysize = xsize/2;
	zsize = MULTI3D_HEIGHT;
	ystep = (zsize) / (double)graph->yunits;
	zstep = (xsize/2) / (double)maxitems;		//15

	col = multi_cols[10+(index%10)];
	col2 = multi_cols[20+(index%10)];

	if( graph->type == GRAPH_MULTIVHOST3D ) {
		ntime = VDptr->byDate->GetDay(daynum);
		if( ntime<=(365*10*ONEDAY) || ntime > (365*40*ONEDAY) )
			return;
		daynum = (long)(ntime/ONEDAY) - (long)(graph->firstTime/ONEDAY);
//		xpos = xpos * (xstep/2);
//		xpos = ((daynum)*(xstep/2));
		//xpos = (long)( ((long)(ntime-0.5)-(long)(graph->firstTime-0.5))* (xstep/2) );
	}

	xpos = ((daynum)*(xstep/2));
	ypos = barsize = ((zsize*value)/100);
	if( xpos < 0 )	index = 0;			// removed <=, that used to cause unwatned blocks at pos 0
	x -= (index*(zstep));		//+4
	y += (index*(zstep/2));		//+2
	
	if( daynum<1 ){
		{
			char txt[64];
			sprintf( txt, "%d", index+1 );
			if( (maxitems>25 && index%2 == 0) || maxitems<=25 )
				PlotNum( 3, x+(xsize)+1, y+(ysize/2), txt, border_col );
		}
		lastx=xpos;		lasty=ypos;		lastv=value;
		//lastv = lasty = lastx = 0;
	}

	//only plot after first x,y is obtained
	if( daynum >= 0 ) {
		gdPoint	poly[4], poly2[4], poly3[4];

		if( maxitems < 15 ) bwid = 150/maxitems;
		x -= bwid;
		y += bwid/2;

		// main
		poly[0].x = poly[1].x = x+lastx+(xsize/2);
		poly[2].x = poly[3].x = x+xpos+(xsize/2);
		poly[0].y = y+(lastx/2);
		poly[1].y = y+(lastx/2)-lasty;
		poly[2].y = y+(xpos/2)-ypos;
		poly[3].y = y+(xpos/2);

		// top     12
		//         03
		poly2[0] = poly2[1] = poly[1];
		poly2[2] = poly2[3] = poly[2];
		poly2[1].x += bwid; poly2[1].y -= bwid/2;
		poly2[2].x += bwid; poly2[2].y -= bwid/2;

		// side
		poly3[0] = poly3[1] = poly[2];
		poly3[2] = poly3[3] = poly[3];
		poly3[1].x += bwid; poly3[1].y -= bwid/2;
		poly3[2].x += bwid; poly3[2].y -= bwid/2;


		if( lastv || value )
		{
			PlotImageFilledPolygon( poly2, 4, col2 );
			if( xstep >3 )
				PlotDrawLine( poly[1].x,poly[1].y,poly2[1].x,poly2[1].y , dgrey );

			PlotImageFilledPolygon( poly3, 4, col2 );
			if( xstep >3 )
				PlotDrawLine( poly[2].x,poly[2].y,poly2[2].x,poly2[2].y , dgrey );

			PlotImageFilledPolygon( poly, 4, col );
			if( xstep >3 )
				PlotDrawLine( poly[0].x,poly[0].y,poly[1].x,poly[1].y , col2 );
		}
		PlotPoint( poly[1].x, poly[1].y, col );
	}
	lastx=xpos;
	lasty=ypos;
	lastv=value;
}

//GRAPH_SCATTER
void StandardPlot::Graph_SessionTimesGetMax( VDinfoP	VDptr, struct graphOptions *graph, long clientnum, long *maxlen, long *maxdepth )
{
	long maxpages, depth=0, sessions=0,page,hashid=SESS_START_PAGE,phashid, item;
	long time, firstDate=0, ptime=0, dur=0;
	Statistic *p;

	p = graph->byStat->GetStat(clientnum);
	maxpages = p->sessionStat->GetNum();
	depth = 1; phashid=SESS_START_PAGE;

	for ( page=0; page < maxpages; page++ ){
		hashid = p->sessionStat->GetSessionPage( page );
		time = p->sessionStat->GetSessionTime( page );
		if( hashid == SESS_START_PAGE ){  // start of session
			firstDate = time;
		} else
		if( hashid == SESS_BYTES ){ // end of session , date
			dur = (ptime-firstDate) + 30;
		} else
		if( hashid == SESS_TIME ){
			if( dur > *maxlen ) *maxlen = dur;
			if( depth > *maxdepth ) *maxdepth = depth;
			depth = 1;
			sessions++;
		}
	
		if( hashid<SESS_START_PAGE || hashid>SESS_ABOVE_IS_PAGE ){
			item = VDptr->byPages->FindHash( hashid );
			if( item >= 0 )
				depth++;
		}
		phashid = hashid;  ptime=time;
	}
	if( hashid!=SESS_TIME ){
		if( dur > *maxlen ) *maxlen = dur;
		if( depth > *maxdepth ) *maxdepth = depth;
	}
}



long StandardPlot::Graph_SessionTimes( VDinfoP	VDptr, struct graphOptions *graph, long clientnum, long maxlen, long maxdepth, long meancolor )
{
	long	maxpages, depth=0, sessions=0,page,hashid=SESS_START_PAGE,phashid, item;
	long	time, firstDate=0, ptime=0, dur=0;
	long	meanpages, meanlen;
	long	xpos,ypos,ysize,xsize;
	long	x,y;
	long	dotcolor;
	Statistic *p;

	ysize = (graph->height - (2*graph->yborder));
	xsize = (graph->width - graph->xborder - 10);
	xpos = graph->xborder;		// calc bar position X
	ypos = (graph->height - graph->yborder);			// calc bar position Y

	p = graph->byStat->GetStat( clientnum );
	if( p->sessionStat == NULL )
		return 0;

	if( MyPrefStruct.corporate_look )
		dotcolor = black;
	else
		dotcolor = white;

	maxpages = p->sessionStat->GetNum();

	meanlen = VDptr->byClient->GetStatListTotalTime()/VDptr->byClient->GetStatListTotalVisits();
	meanpages = VDptr->byClient->GetStatListTotalCounters4()/(__int64)VDptr->byClient->GetStatListTotalVisits();

	if( meanlen < maxlen/5 || maxlen == 0 ) maxlen = meanlen*5;
	if( meanpages < maxdepth/10 ) maxdepth = roundvalue(meanpages*10);

	for ( page=0; page < maxpages; page++ ){

		hashid = p->sessionStat->GetSessionPage( page );
		time = p->sessionStat->GetSessionTime( page );

		if( hashid == SESS_START_PAGE ){  // start of session
			firstDate = time;
		} else
		if( hashid == SESS_BYTES ){ // end of session , date
			dur = (ptime-firstDate) + 30;
		} else
		if( hashid == SESS_TIME ){
			if( dur > 0 && dur<maxlen ){
				x = xpos + ((xsize*dur)/maxlen);
				y = ypos - ((ysize*depth)/maxdepth);
				PlotPoint( x, y, dotcolor );
			}
			depth = 1;
			sessions++;
		}
		if( hashid<SESS_START_PAGE || hashid>SESS_ABOVE_IS_PAGE ){
			item = VDptr->byPages->FindHash( hashid );
			if( item >= 0 ) {
				depth++;
			}
		}
		phashid = hashid; ptime=time;
	}
	if( hashid!=SESS_TIME ){
		if( dur>0 && dur<maxlen ){
			x = xpos + ((xsize*dur)/maxlen);
			y = ypos - ((ysize*depth)/maxdepth);
			PlotPoint( x, y, dotcolor );
		}
	}

	// draw averages.....
	if( clientnum >= VDptr->byClient->GetStatListNum()-1 && maxlen && maxdepth ){
		double	temp;
		long	x2,y2;

		x = xpos + ((xsize*meanlen)/maxlen);
		y = ypos - ((ysize*meanpages)/maxdepth);

		PlotDrawLine( xpos, y, xpos+xsize, y, dgrey );
		PlotDrawLine( x, ypos-ysize, x, ypos, dgrey );

		if( meanpages )
			temp = (maxdepth*meanlen)/meanpages;
		else
			temp = 0;

		x2 = xpos + ((xsize*temp)/maxlen);
		if( x2 < xpos+xsize ){
			PlotDrawLine( xpos, ypos, x2, ypos-ysize, meancolor );
		} else {
			temp = ((maxlen*meanpages)/meanlen);
			y2 = ypos - ((ysize*temp)/maxdepth);
			PlotDrawLine( xpos, ypos, xpos+xsize, y2, meancolor );
		}
	}

	return maxpages;
}

/*
 *  Draw the PIE graph in 3d if possible
 *
 */
void StandardPlot::PlotGraph_Pie( struct graphOptions *graph, long itempos, double current_percent, long num, long pie_hlite[PIE_COLS],long pie_cols[PIE_COLS] )
{
static	long	thecount;
static	double	lastpercent;
		double	angle, angle2;
		long	ysize,width,height, piex, piey, offset = 180;

	if( itempos == 0 ){
		thecount = 0;
		lastpercent = 0;
	}

	ysize = (graph->height - (2*graph->yborder));
	width = graph->width;
	height = graph->height;
	piex = width/2;
	piey = height-PIE_RAD-55;

	angle = ((360*lastpercent)/100.0);
	angle2 = ((360*(lastpercent+current_percent))/100.0);

	int angleInt = angle;
	int angle2Int = angle2 + 0.1;	// Overcomes a stupid compiler error with the following... where 360.0 != 360.0 ???
	//if( angle == 0.0 && angle2 == 360.0 ) angle2 = 359.999;
	if( angleInt == 0 && angle2Int == 360 )
		angle2 = 359.999;

	lastpercent += current_percent;

	//only plot after first x,y is obtained
	if( angle != angle2) {
		Plot3DPieGraph( piex, piey, PIE_RAD, angle+offset, angle2+offset, pie_hlite[thecount%10], pie_cols[thecount%10]);
		//draw pie legend
		PlotFilledRect( PIETAB_X-1, PIETAB_Y+(thecount*15)-1, PIETAB_X+8+1, 8+PIETAB_Y+(thecount*15)+1, label_col );
		PlotFilledRect( PIETAB_X, PIETAB_Y+(thecount*15), PIETAB_X+8, 8+PIETAB_Y+(thecount*15), pie_hlite[thecount%10]);
		PlotString( 1, PIETAB_X+13, PIETAB_Y+(thecount*15)-2, graph->byStat->GetName( itempos ), label_col );
		thecount++;

		if( current_percent > 1 )
		{
			long x,y, da = (angle2-angle)/2; 
			char txt[64];

			x = ArcX( angle+offset+da, 2*PIE_RAD*1.1 );
			y = ArcY( angle+offset+da, PIE_RAD*1.35 );

			sprintf( txt, "%.1f%%", current_percent );
			PlotNum( 1, piex+x-15,piey+y+6, txt, label_col );

			x = ArcX( angle+offset+da, 2*PIE_RAD*.8 );
			y = ArcY( angle+offset+da, PIE_RAD*.8 );
		}
	}
	// other pie data
	if( itempos+1 == num ){
		if( angle2 < 359.999 ){	// draw others
			Plot3DPieGraph( piex, piey, PIE_RAD, angle2+offset, 360+offset, dgrey, black );

			//draw pie legend
			PlotFilledRect( PIETAB_X-1, PIETAB_Y+(thecount*15)-1, PIETAB_X+8+1, 8+PIETAB_Y+(thecount*15)+1, label_col );
			PlotFilledRect( PIETAB_X, PIETAB_Y+(thecount*15), PIETAB_X+8, 8+PIETAB_Y+(thecount*15), black );
			PlotString( 1, PIETAB_X+13, PIETAB_Y+(thecount*15)-2, Translate( LABEL_OTHER ), label_col );
			thecount++;
		}
	}
}




// ------------------------------------------------------------------------------------------


void StandardPlot::SetGraphColors( struct graphOptions *graph )
{
 	label_col = PlotImageColorAllocate( RGB_RED(graph->label_color),RGB_GREEN(graph->label_color),RGB_BLUE(graph->label_color)); 
 	grid_col = PlotImageColorAllocate( RGB_RED(graph->grid_color),RGB_GREEN(graph->grid_color),RGB_BLUE(graph->grid_color)); 
 	base_col = PlotImageColorAllocate( RGB_RED(graph->base_color),RGB_GREEN(graph->base_color),RGB_BLUE(graph->base_color)); 
 	borderhi_col = PlotImageColorAllocate( RGB_RED(graph->borderhi_color),RGB_GREEN(graph->borderhi_color),RGB_BLUE(graph->borderhi_color)); 
 	borderlo_col = PlotImageColorAllocate( RGB_RED(graph->borderlo_color),RGB_GREEN(graph->borderlo_color),RGB_BLUE(graph->borderlo_color)); 

	if( MyPrefStruct.corporate_look )
	 	border_col = PlotImageColorAllocate( 200,200,200 ); 
	else
	 	border_col = PlotImageColorAllocate( RGB_RED(graph->border_color),RGB_GREEN(graph->border_color),RGB_BLUE(graph->border_color)); 

 	bgcolor = PlotImageColorAllocate( RGB_RED(graph->back_color), RGB_GREEN(graph->back_color), RGB_BLUE(graph->back_color));
 	bar1 = PlotImageColorAllocate( RGB_RED(graph->bar_color[0]), RGB_GREEN(graph->bar_color[0]), RGB_BLUE(graph->bar_color[0]));    
 	bar2 = PlotImageColorAllocate( RGB_RED(graph->bar_color[1]), RGB_GREEN(graph->bar_color[1]), RGB_BLUE(graph->bar_color[1]));    
 	bar3 = PlotImageColorAllocate( RGB_RED(graph->bar_color[2]), RGB_GREEN(graph->bar_color[2]), RGB_BLUE(graph->bar_color[2]));    

	int i;
	for ( i=0;i <PIE_COLS; i++ ) {
		pie_cols[i] = PlotImageColorAllocate( RGB_RED(pie_rgb[i]), RGB_GREEN(pie_rgb[i]), RGB_BLUE(pie_rgb[i]));
		pie_hlite[i] = PlotImageColorAllocate( RGB_RED(pie_hlit[i]), RGB_GREEN(pie_hlit[i]), RGB_BLUE(pie_hlit[i]));
	}
	for ( i=0;i <MULTI_COLS; i++ ) {
		multi_cols[i] = PlotImageColorAllocateRGB( ScaleRGB( 150,multi_rgb[i] ) );
		multi_cols[10+i] = PlotImageColorAllocateRGB( multi_rgb[i] );
		multi_cols[20+i] = PlotImageColorAllocateRGB( ScaleRGB( 50,multi_rgb[i] ) );
	}
	
}


#ifndef DEF_MAC
extern "C" void InitSinCos( void );
#endif

//
// draw bar graph of statistics of passed stat list 
//
short StandardPlot::DrawGraph( VDinfoP	VDptr, struct graphOptions *graph )
{	
	__int64 		total_value, highest_value=0, highest_count=0;
	__int64 		current_errors,
					current_value, show_value, value,
					current_value2=0, value2=0,
					sumvalue=0;
	double 			xstep,ystep,current_percent;
	long			index, itempos;
	long 			num,daysTot=0, ysize,xsize;
	long 			scaler_labels;
	long 			width,height,xunits,yunits,yborder,xborder,type,scaler_type;
	long			scaler_units,style,thecount=0;
//	long			pie_cols[PIE_COLS], pie_hlite[PIE_COLS], multi_cols[MULTI_COLS*3];
	char			xaxis[256], yaxis[256], xlate[256];
	char			*xaxisPtr=xaxis;
	char			*yaxisPtr=yaxis;
	FILE 			*out = NULL;
	VDinfoP			Hostptr = 0;


	style = MyPrefStruct.graph_style;
	type = graph->type;
	width = graph->width;
	height = graph->height;
	xunits = graph->xunits;
	yunits = graph->yunits;
	yborder = graph->yborder;
	xborder = graph->xborder;
	scaler_type = graph->scaler_type;
	xsize = (width-xborder-10);				// x/y size of the actual graph region
	ysize = (height-(2*yborder));

	// create image space
	CreateImage( width, height, MyPrefStruct.paletteType );
	AllocDefaultColors();
	SetGraphColors( graph );

	//number of entries in table (# of samples)
	if( graph->byStat )
		num = graph->byStat->GetStatListNum();
	else
		num = graph->firstXsamples;

	switch (scaler_type) {
		case SORT_SESSIONS:	if( graph->byStat ) total_value = graph->byStat->totalVisits/100;
			break;
		case SORT_BYTES:	if( graph->byStat ) total_value = graph->byStat->GetStatListTotalBytes()/100;
			break;
		case SORT_REQUESTS:	if( graph->byStat ) total_value = graph->byStat->GetStatListTotalRequests()/100;
			break;
		case SORT_COUNTER:	if( graph->byStat ) total_value = graph->byStat->GetStatListTotalCounters()/100;
			break;
		case SORT_PAGES:
		case SORT_COUNTER4:	if( graph->byStat ) total_value = graph->byStat->GetStatListTotalCounters4()/100;
			break;
		case SORT_ERRORS:	if( graph->byStat ) total_value = VDptr->totalFailedRequests/100;
			break;
	}

	// get the highest value to scale up the graph to full size!
	// diff case for last 31 days
	itempos = index = 0;	
	total_value = 0;

	if( graph->firstXsamples > 0) {
		if( num > graph->firstXsamples )
			num = graph->firstXsamples;
	}
	if( graph->lastXsamples > 0) {
		if( num > graph->lastXsamples )
			index = num - graph->lastXsamples;
	}
	// if doing Last portion of the list 
	if( num > graph->lastXsamples  &&  graph->lastXsamples )
		itempos = num - graph->lastXsamples;

	if( type == GRAPH_HBAR )
	{
		if( scaler_type == SORT_COUNTER4 || scaler_type == SORT_VISITIN )
		{
			long adjNum = itempos;
			while ( adjNum < num )
			{
				switch ( scaler_type )
				{
					case SORT_COUNTER4:
						current_value = graph->byStat->StatList::GetCounter4( adjNum );
						break;
					case SORT_VISITIN:
						current_value = graph->byStat->StatList::GetVisitIn( adjNum );
						break;
				}
				if( current_value == 0 )
					num = adjNum;
				else
					adjNum++;
			}
		}
		yunits = num;
	}

	if( xunits )	xstep = xsize/(double)xunits; else xsize = 0;
	if( yunits )	ystep = ysize/(double)yunits; else ysize = 0;

	if( type == GRAPH_MULTILINE || type == GRAPH_MULTI3D  || type == GRAPH_MULTIVHOST3D ){
		daysTot = graph->multiLinetotal;
		Hostptr = VDptr;
		if( num > 1 )
			Hostptr = (VDinfo *)Hostptr->next;
	}

	// calc totalsum and highest values from sample data set		//VDinfoP firstVDptr
	int	iOffset = 0;
	while ( index < num ) 
	{
		if( graph->byStat )
		{
			Statistic *p;
			p = graph->byStat->GetStat( index );

			if( graph->type_id == 'tope' && p->counter4 == 0 )	break;		// stop if no more top entry pages
			if( graph->type_id == 'topx' && p->GetVisitIn() == 0 )	break;	// stop if no more top exit pages
		}

		if( graph->byStat )
		{
			if (graph->byStat->m_FilterIndex)
			{
				while ((index+iOffset)<graph->byStat->num && !graph->byStat->m_FilterIndex(graph->byStat->GetStat(index+iOffset)))
				{
					++iOffset;
				}
				if ((index+iOffset)>=graph->byStat->num)
					break;
			}
		}

		if( type == GRAPH_SCATTER ){
			if( graph->byStat->doSessionStat==SESSIONSTAT_CLICKSTREAM ){
				Statistic *p;
				p = graph->byStat->GetStat( (index+iOffset) );
				current_value = p->sessionStat->GetMaxDur();
				if( current_value > highest_count ) highest_count = current_value;

				current_value = p->sessionStat->GetMaxLen();
				if( current_value > highest_value ) highest_value = current_value;
			}
			graph->xunits = (highest_count)/60;
		} else
		if( type == GRAPH_MULTIVHOST3D ){
			long lp;
			while ( Hostptr ){
				graph->byStat = Hostptr->byDate;
				for( lp=0; lp<Hostptr->byDate->GetStatListNum(); lp++ ){	// do all days for each client/page/url
					switch ( scaler_type ) {
						case SORT_BYTES:	current_value = Hostptr->byDate->GetBytes( lp ); break;
						case SORT_REQUESTS:
						default:			current_value = Hostptr->byDate->GetFiles( lp ); break;
					}
					if( current_value > 0 ){
						total_value += current_value;   //used for calculations
						if( current_value > highest_value )
							highest_value = current_value;
						//printf( "%d:%d - cv=%f, hv=%f\n", index,lp,current_value,highest_value);
					}
				}
				Hostptr = (VDinfo *)Hostptr->next;
			}
		} else
		if( type == GRAPH_MULTILINE || type == GRAPH_MULTI3D ){
			long lp;

			for( lp=0; lp<daysTot; lp++ ){	// do all days for each client/page/url
				switch ( scaler_type ) {
					case SORT_BYTES:
						current_value = graph->byStat->GetBytesHistory( (index+iOffset), (long)(VDptr->firstTime/ONEDAY)+lp );
						break;
					case SORT_REQUESTS:
					default:
						current_value = graph->byStat->GetFilesHistory( (index+iOffset), (long)(VDptr->firstTime/ONEDAY)+lp );
						break;
				}
				if( current_value > 0 ){
					total_value += current_value;   //used for calculations
					if( current_value > highest_value )
						highest_value = current_value;
				}
			}
		} else {
			switch ( scaler_type ) 
			{
				case SORT_VISITIN:	current_value = graph->byStat->GetVisitIn((index+iOffset)); break;
				case SORT_PAGES:
				case SORT_COUNTER4:	current_value = graph->byStat->GetCounter4((index+iOffset)); break;
				case SORT_COUNTER:	current_value = graph->byStat->GetCounter((index+iOffset)); break;
				case SORT_SESSIONS:	current_value = graph->byStat->GetVisits((index+iOffset)); break;
				case SORT_ERRORS:	current_value = graph->byStat->GetErrors((index+iOffset)); break;
				case SORT_BYTES:	current_value = graph->byStat->GetBytes((index+iOffset)); 
									if( MyPrefStruct.stat_style != STREAM ){
										current_value2 = graph->byStat->GetBytesIn((index+iOffset));
										if( current_value2 > current_value ) current_value = current_value2;
									}
									break;
				case SORT_REQUESTS:
				default:			current_value = graph->byStat->GetFiles((index+iOffset)); break;
			}
			if( graph->lastXsamples > 0 || graph->firstXsamples > 0)
				total_value += current_value;   //used for calculations
			if( current_value > highest_value )
				highest_value = current_value; 
		}
		index++;
	}
	highest_value = roundvalue( highest_value );

	if( graph->lastXsamples>0 || graph->firstXsamples>0 )
		total_value = total_value/100;
	
	// -----------------------------------------------------------------------------
	switch (type) 
	{
		case GRAPH_HBAR: //horizontal bar graph
			scaler_units = xunits;
			scaler_labels = graph->xlabels;
			break;
		default:
			scaler_units = yunits;
			scaler_labels = graph->ylabels;
			break;
	}
	

	if( MyPrefStruct.report_format == FORMAT_PDF || MyPrefStruct.report_format == FORMAT_EXCEL || MyPrefStruct.report_format == FORMAT_COMMA )
	{
		if( graph->type == GRAPH_MULTIVHOST3D )
			out = Image_InitFile( VDptr, graph, MyPrefStruct.image_format );
	}
	else
		out = Image_InitFile( VDptr, graph, MyPrefStruct.image_format );

	graph->out = out;
	//draw grey backround with a 3d border!
	if( MyPrefStruct.corporate_look )
		Graph_DrawBase( graph, width, height, 3 );			// plain white
	else
	if( MyPrefStruct.paletteType == GRAPH_WEBCOLOR )
		Graph_DrawBase( graph, width, height, 1 );			// plain grey
	else
		Graph_DrawBase( graph, width, height, 2 );			// enhanced graduation/cool look
	//return !stopall;

	if( type==GRAPH_MULTI3D || type==GRAPH_MULTIVHOST3D ){
		ysize = (height-180);
		Graph_Draw3DAxis( graph, style, xsize, ysize, bgcolor, grid_col );
		DefineLabels( graph, highest_value, &xaxisPtr, &yaxisPtr, graph->labelTranslateIdOverride );
		mystrcpy( xaxisPtr, Translate( TIME_DAYS ) );
		PlotTransString( 1, xborder+((width-xborder)/6)-(GetTransStringLen(xaxisPtr)/2), height-(yborder/3), xaxisPtr, label_col, 0, 0, REM_HTML_TOKENS );
		double textLen = GetTransStringLen(yaxisPtr);
		// Trev - have to look at the string length being correct...
		PlotTransStringUp( 1, 3, (height/2)+(textLen/2), yaxisPtr, label_col, REM_HTML_TOKENS  );

	} else
	if( type==GRAPH_PIE ){
		if( !MyPrefStruct.corporate_look )
			PlotBevel( PIETAB_XX-4,PIETAB_YY-4, (width-PIETAB_XX-20), PIETAB_YY + PIETAB_HH, dgrey, lgrey, white );
		DefineLabels( graph, highest_value, &xaxisPtr, &yaxisPtr, graph->labelTranslateIdOverride );
		if( strlen(yaxisPtr) )
		{
			PlotTransString( 1, (width/2)-(GetTransStringLen(yaxisPtr)/2), height-(yborder), yaxisPtr, label_col, 0, 0, REM_HTML_TOKENS );
		}
	} else {
		// draw axis
		Graph_DrawBackground( graph, style, xsize, ysize, border_col, bgcolor, grid_col );
		// draw axis text labels
		if( graph->ftitle )
			mystrcpy( xaxis, ConvertHTMLTokens( graph->ftitle ) );   	//fixed
		else
			xaxis[0] = 0;
		DefineLabels( graph, highest_value, &xaxisPtr, &yaxisPtr, graph->labelTranslateIdOverride );
		PlotTransStringC( 1, xborder+((width-xborder)/2)-(GetStringLen(xaxisPtr)/2), height-(yborder/2), xaxisPtr, label_col, 0 );
		double textLen = GetStringLen(yaxisPtr);
		PlotTransStringUp( 1, 5, (height/2)+(textLen/2), yaxisPtr, label_col, REM_HTML_TOKENS );
		//----- Axis label on the top-left
		//PlotText( 1, (xborder - textLen)/2, yborder-15, yaxisPtr, label_col );
	}
	//draw x labels
	if( scaler_labels ){
		Graph_AxisLabels( VDptr, graph, highest_value, highest_count, scaler_units, label_col, grid_col );
	}

	//draw Title
	mystrcpy( xlate, ConvertHTMLTokens( graph->title ) );
	if( graph->xtitle == 0 )	graph->xtitle = (width/2)-(GetStringLen(xlate)/2);
	if( graph->ytitle == 0 )	graph->ytitle = yborder/4;

	PlotTransString( 2, graph->xtitle, graph->ytitle, xlate, label_col, 0, 0, REM_HTML_TOKENS );

	if( graph->legend )
		Graph_Legend( graph, multi_cols );

	current_errors = index = 0;

	
	if( type == GRAPH_MULTIVHOST3D ){		// restore HostPtr to head of Virtual Pointer List
		Hostptr = VDptr;
		if( num > 1 )
			Hostptr = (VDinfo *)Hostptr->next;
	}

	//
	// draw graph
	//
	iOffset = 0;
	while ( itempos < num ) 
	{
		Statistic *p;

		if( graph->byStat )
		{
			p = graph->byStat->GetStat( index );

			if( graph->type_id == 'tope' && p->counter4 == 0 )	break;		// stop if no more top entry pages
			if( graph->type_id == 'topx' && p->GetVisitIn() == 0 )	break;	// stop if no more top exit pages

			// iOffset set.
			if( graph->byStat->m_FilterIndex)
			{
				while ((itempos+iOffset)<graph->byStat->num && !graph->byStat->m_FilterIndex(graph->byStat->GetStat(itempos+iOffset)))
				{
					++iOffset;
				}
				if ((itempos+iOffset)>=graph->byStat->num)
					break;
			}

			if( graph->byStat ){
				switch ( scaler_type ){
					case SORT_VISITIN:
						current_value = graph->byStat->GetVisitIn((itempos+iOffset));
						break;
					case SORT_COUNTER4: case SORT_PAGES:
						current_value = graph->byStat->GetCounter4((itempos+iOffset));
						break;
					case SORT_COUNTER: 
						current_value = graph->byStat->GetCounter((itempos+iOffset));
						current_percent = current_value/(graph->byStat->totalCounters/100.0);
						break;
					case SORT_ERRORS:
						current_value = graph->byStat->GetErrors((itempos+iOffset));
						current_percent = current_value/(graph->byStat->totalVisits/100.0);
						break;
					case SORT_SESSIONS:
						current_value = graph->byStat->GetVisits((itempos+iOffset));
						current_percent = current_value/(graph->byStat->totalVisits/100.0);
						break;
					case SORT_BYTES:
						current_value = graph->byStat->GetBytes((itempos+iOffset));
						current_percent = current_value/(graph->byStat->GetStatListTotalBytes()/100.0);
						if( MyPrefStruct.stat_style != STREAM ){
							current_value2 = graph->byStat->GetBytesIn((itempos+iOffset));
							value2 = current_value2/(highest_value/100.0);
						}
						break;
					case SORT_REQUESTS:
					default:
						current_value = graph->byStat->GetFiles((itempos+iOffset));
						current_percent = current_value/(graph->byStat->GetStatListTotalRequests()/100.0);
						current_errors = graph->byStat->GetErrors((itempos+iOffset))/(highest_value/100.0);
						break;
				}
			}
		}


		if( highest_value )
			value = current_value/(highest_value/100.0); else value = 0;

		sumvalue += value;
		switch (type) 
		{
			case GRAPH_HBAR: // horizontal bar graph
				Graph_HBar( graph, (index+0), (itempos+iOffset), value, current_value, ystep, bar1, bar2, bar3 );
				if( current_errors )
				{
					int labelStatus = graph->xlabels;
					graph->xlabels = 0;
					Graph_HBar( graph, (index+0), (itempos+iOffset), current_errors, -1, ystep, -1, red, -1 );
					graph->xlabels = labelStatus;
				}
				break;

			case GRAPH_VBAR: // verticle bar graph
				Graph_VBar( graph, (index+0), (itempos+iOffset), value, current_value, xstep, bar1, bar2, bar3 );
				if( current_errors )
				{
					int labelStatus = graph->xlabels;
					graph->xlabels = 0;
					Graph_VBar( graph, (index+0), (itempos+iOffset), current_errors, -1, xstep, -1, red, -1 );
					graph->xlabels = labelStatus;
				}
				break;

			case GRAPH_PIE: // pie graph
				InitSinCos(); // This is for the pie graphs, especially for PDF
				PlotGraph_Pie( graph, (itempos+iOffset), current_percent, num, multi_cols+10, multi_cols+20 );
				break;

			case GRAPH_LINE: // line graph	
				Graph_Line( VDptr, graph, (index+0), (itempos+iOffset), value, value2, current_errors, sumvalue, xstep, num );
				break;

			case GRAPH_MULTILINE:
			case GRAPH_MULTI3D:
				{
					long lp;
					//printf( "name=%s\n", graph->byStat->GetName( itempos ) );
					for( lp=0; lp<daysTot; lp++ ){	// do all days for each client/page/url
						switch( scaler_type ){
							case SORT_BYTES:	current_value = graph->byStat->GetBytesHistory( (itempos+iOffset), (long)(VDptr->firstTime/ONEDAY)+lp );break;
							case SORT_REQUESTS:	
							default:			current_value = graph->byStat->GetFilesHistory( (itempos+iOffset), (long)(VDptr->firstTime/ONEDAY)+lp );break;
						}
						if( current_value < 0 ) current_value=0;
						show_value = current_value/(highest_value/100.0);
						if( show_value == 0 )
							show_value = 1; // Show something, a minimal value
						current_value = show_value;
						value = (long)current_value;
						Graph_Multi3DLine( VDptr, graph, (itempos+iOffset), lp, num, current_value, xstep, multi_cols );
					}
				}
				break;

			case GRAPH_MULTIVHOST3D:
				if( Hostptr && Hostptr->totalRequests ) 
				{
					long lp;
					for( lp=0; lp<daysTot+1; lp++ ){	// do all days for each client/page/url
						switch( scaler_type ){
							case SORT_BYTES:	current_value = Hostptr->byDate->GetBytes( lp );break;
							case SORT_PAGES:	current_value = Hostptr->byDate->GetCounter4( lp );break;
							case SORT_REQUESTS:	
							default:			current_value = Hostptr->byDate->GetFiles( lp );break;
						}

						if( current_value < 0 ) current_value=0;
						show_value = current_value/(highest_value/100.0);
						if( show_value == 0 )
							show_value = 1; // Show something, a minimal value
						current_value = show_value;
						Graph_Multi3DLine( Hostptr, graph, (itempos+iOffset), lp, num, current_value, xstep, multi_cols );
					}
					Hostptr = (VDinfo *)Hostptr->next;
				}
				break;

			// Draw the depth type virtual bars
			case GRAPH_MULTIVBAR4:
				{
					long lp;
					graph->xlabels = 0;
					graph->ylabel_value = 0;
					for( lp=1; lp<=3; lp++ )
					{	// traverse all 3 OSs
						switch( scaler_type ){
							case SORT_BYTES:	current_value = graph->byStat->GetBytesHistory2( (itempos+iOffset), lp ); break;
							case SORT_REQUESTS:	
							default:			current_value = graph->byStat->GetFilesHistory2( (itempos+iOffset), lp ); break;
						}
						value = current_value/(highest_value/100.0);
						graph->xborder = xborder + ((3-lp)*graph->depth);
						if( MyPrefStruct.graph_style )
							graph->yborder = yborder + ((3-lp)*graph->depth);
						if( lp==3 ) graph->xlabels = 1;
						graph->bartitle = OSNames[lp-1];
						Graph_VBar( graph, (index+0), (itempos+iOffset), value, current_value, xstep, multi_cols[5+lp], multi_cols[10+5+lp], multi_cols[20+5+lp] );
					}
					graph->ylabel_value = 0;
				}
				break;

			case GRAPH_SCATTER:
				if( thecount < 100000 )
					thecount += Graph_SessionTimes( VDptr, graph, (index+0), highest_count, highest_value, multi_cols[10+6] );
				break;
		}
		index++; itempos++;
	}

	// if thumb is on, copy/convert/save image as a 30% smaller version with thumb prefix
	if( graph->m_thumbnail )
	{
		Image_CompleteThumbNail( VDptr, graph, im );
	}

	Image_CompleteFile( out );

	return !stopall;	
}



/*
<SCRIPT LANGUAGE=JavaScript>
<!-- begin
    document.write( \"<META NAME=asciigraph CONTENT=\\\"\" );
//-- end -->
</SCRIPT> 

<SCRIPT LANGUAGE=\"JavaScript\">
<!-- begin
    document.write( \"\\\">\" );
//-- end -->
</SCRIPT>

*/
static char asciigraphHead[] = {
"<NOSCRIPT><PRE>\n"
"                       |\n"
"                    ---+----+----+----+----+----+----+----+----+----+----+----\n"
 };

static char asciigraphFoot[] = {
"                    ---+----+----+----+----+----+----+----+----+----+----+----\n"
"                       |\n"
"</PRE></NOSCRIPT>\n"
};

// draw bar graph of statistics of passed stat list 
void DrawAsciiGraph( VDinfoP VDptr, FILE *fp, StatList *array, long type, long depth )
{	
	__int64 		total_value, highest_value=0, 
					current_value, current_percent,
					sumvalue=0;
	int 			width,scaler_type, num;
	long			value,index, itempos;
	char			name[512];

	width = 5*11;
	scaler_type = SORT_REQUESTS;

	if( MyPrefStruct.graph_wider )
		width = (int)(width*1.5);
		

	//number of entries in table (# of samples)
	num = array->GetStatListNum();
	if (num > depth) num=depth;

	switch (scaler_type) {
		case SORT_BYTES:
			total_value = array->GetStatListTotalBytes()/100;
			break;
		case SORT_REQUESTS:
			total_value = (__int64)array->GetStatListTotalRequests()/100;
			break;
	}

	// get the highest value to scale up the graph to full size!
	// diff case for last 31 days
	index = 0;
	index = 0;
	total_value = 0;

	while ( index < num ) {
		switch ( scaler_type ) {
			case SORT_BYTES :
				current_value = array->GetBytes(index); break;
			case SORT_REQUESTS :
				current_value = array->GetFiles(index); break;
		}
		total_value += current_value;   //used for calculations
		if( current_value > highest_value )
			highest_value = current_value; 
		index++;
	}
	
	total_value = total_value/100;
	
	fprintf( fp, asciigraphHead );

	index = itempos = 0;

	// draw graph
	while ( itempos < num ) {
		long loop;
		if( scaler_type == SORT_BYTES ){
			current_value = array->GetBytes(itempos);
			current_percent = current_value/(highest_value/100.0);
		} else {
			current_value = array->GetFiles(itempos);
			current_percent = current_value/(highest_value/100.0);
		}
		value = (long)( (current_percent*width)/100 );		// scale values
		

		loop = mystrncpy( name, array->GetName( itempos ), 511 );
		name[loop] = 0;


		if( type == 6 || type == 7 || type == 12 )
			if( !isdigit( *name ) )
				ReverseAddress( name,name );
		name[22]=0;
		
		fprintf( fp, "%-22s |", name );
		if( value >= 1 && value <= width ){
			for( loop=1; loop<=value; loop++ ){
				if( loop%5==0 && loop )
					fprintf( fp, "+" );
				else
					fprintf( fp, "=" );
			}
		}

		fprintf( fp, "\n" );
		index++; itempos++;
	}

	fprintf( fp, asciigraphFoot );

	return;	
}

/* --------------------------

	Create defaults for common
		graph styles
		
------------------------------*/
long GraphWidth( void )
{
	long w;
	w = GRAPH_WIDTH;
	if( MyPrefStruct.report_format == FORMAT_HTML ){
		if( MyPrefStruct.graph_wider )
			w = w*1.5;


	}
	if( MyPrefStruct.report_format == FORMAT_RTF ){
		w = 732;
	}
	return w;
}
long GraphHeight( void )
{
	long i;
	i = GRAPH_HEIGHT;
	if( MyPrefStruct.report_format == FORMAT_RTF ){
		i = 400;
	} else
		i = GraphWidth() * 72/100;
	return i;
}




// ------------------------------------------------------------------------------------------------------------------

void CopyPrefsColors( GraphColors *g, struct graphOptions *gph )
{
	gph->label_color = g->label;
	gph->grid_color	 = g->grid;
	gph->base_color = g->base;
	gph->borderhi_color = g->border_hilight;
	gph->borderlo_color = g->border_lolight;
	gph->border_color = g->border;
}


void StandardPlot::CreateGraphtypeLine( struct graphOptions *gph )
{
	memset( gph, 0, sizeof( struct graphOptions ) );
	gph->width = GraphWidth();
	gph->height = GraphHeight();

	gph->xborder = 66; 
	gph->yborder = 50;
	gph->xunits = 23;
	gph->xtitle = gph->ytitle = 0;

	gph->yunits = 10;
	gph->xlabels = 1;
	gph->ylabels = 1;
	gph->type = GRAPH_LINE;
	gph->lastXsamples = 0;
	gph->firstXsamples = 0;				// 0 = all samples
	gph->bar_space = 1;
	gph->gridx = 1;
	gph->gridy = 1;
	gph->tickx = 1;
	gph->ticky = 1;	
	gph->xlabel_style = 1;
	gph->xlabel_value = 0;
	gph->legend = 1;
	CopyPrefsColors( &MyPrefStruct.graphcolors[GRAPHCOLORS_LINE], gph );
}



// Not used as the graph looks a little messy
void StandardPlot::CreateGraphtypeMultiLine( struct graphOptions *gph )
{
	CreateGraphtypeLine( gph );
	gph->lastXsamples = 0;
	gph->firstXsamples = 15;
	gph->type = GRAPH_MULTILINE;
	CopyPrefsColors( &MyPrefStruct.graphcolors[GRAPHCOLORS_LINE], gph );
}





void StandardPlot::CreateGraphtypeMulti3D( struct graphOptions *gph )
{
	CreateGraphtypeLine( gph );
	gph->height = GraphWidth() * 72/100;
	gph->xtitle = 20;
	gph->ytitle = 3;
	gph->xborder = 25;
	gph->yborder = 110;
	gph->lastXsamples = 0;
	gph->firstXsamples =50;
	if( gph->firstXsamples > 50 ) gph->firstXsamples = 50;

	gph->type = GRAPH_MULTI3D;
	gph->xlabels = 1;	gph->ylabels = 1;

	if( MyPrefStruct.graph_wider ){
		//gph->height *= 1.2;
		gph->yborder *= 1.5;
	}
	CopyPrefsColors( &MyPrefStruct.graphcolors[GRAPHCOLORS_MULTI3D], gph );
}


void StandardPlot::CreateGraphtypeVBar( struct graphOptions *gph )
{
	memset( gph, 0, sizeof( struct graphOptions ) );
	gph->width = GraphWidth();
	gph->height = GraphHeight();

	gph->xborder = 66;
	gph->yborder = 50;
	gph->xtitle = gph->ytitle = 0;
	gph->xunits = 7;
	gph->yunits = 10;
	gph->xlabels = 1;
	gph->ylabels = 1;
	gph->type = GRAPH_VBAR;
	gph->lastXsamples = 0;
	gph->firstXsamples = 0;
	gph->depth = 5;
	gph->gridx = 0;
	gph->gridy = 1;
	gph->tickx = 0;
	gph->ticky = 1;	
	gph->xlabel_style = 1;
	gph->xlabel_value = 0;
	if( MyPrefStruct.graph_style == GRAPH_2DSTYLE )
		gph->bar_space = 3;
	else
		gph->bar_space = 8;

	CopyPrefsColors( &MyPrefStruct.graphcolors[GRAPHCOLORS_VBAR], gph );
}

void StandardPlot::CreateGraphtype4VBar( struct graphOptions *gph )
{
	CreateGraphtypeVBar( gph );
	gph->yunits = 6;
	gph->xunits = 4;
	gph->bar_space = 30;
	gph->depth = 7;
	gph->firstXsamples = 4;
	gph->legend = 1;
	gph->type = GRAPH_MULTIVBAR4;
	CopyPrefsColors( &MyPrefStruct.graphcolors[GRAPHCOLORS_MULTIVBAR], gph );
}

void StandardPlot::CreateGraphtypeScatterBar( struct graphOptions *gph )
{

	CreateGraphtypeVBar( gph );

	gph->type = GRAPH_SCATTER;
	gph->xborder = 40;

	CopyPrefsColors( &MyPrefStruct.graphcolors[GRAPHCOLORS_SCATTER], gph );

	if( MyPrefStruct.corporate_look )
		gph->grid_color = LGREY;
}


void StandardPlot::CreateGraphtypeHBar( struct graphOptions *gph )
{
	memset( gph, 0, sizeof( struct graphOptions ) );
	gph->width = GraphWidth();
	gph->height = GraphHeight();

	gph->xborder = 180;
	gph->yborder = 50;
	gph->xtitle = gph->ytitle = 0;

	gph->xunits = 5;
	gph->yunits = 25;
	gph->xlabels = 1;
	gph->ylabels = 1;
	gph->type = GRAPH_HBAR;
	gph->lastXsamples = 0;
	gph->firstXsamples = 25;
	gph->depth = 3;
	gph->bar_space = 5;
	gph->gridx = 1;
	gph->gridy = 0;
	gph->tickx = 1;
	gph->ticky = 0;	
	gph->xlabel_style = 1;
	gph->xlabel_value = 1;

	if( MyPrefStruct.graph_wider ){
		gph->xborder *= 1.5;
	}
	CopyPrefsColors( &MyPrefStruct.graphcolors[GRAPHCOLORS_HBAR], gph );
}


void StandardPlot::CreateGraphtypePie( struct graphOptions *gph )
{
	memset( gph, 0, sizeof( struct graphOptions ) );
	gph->width = GraphWidth();
	gph->height = GraphHeight();

	gph->height = GRAPH_HEIGHT;
	gph->xborder = 130;
	gph->yborder = 40;
	gph->xtitle = gph->ytitle = 0;

	gph->xunits = 6;
	gph->xlabels = 1;
	gph->ylabels = 0;
	gph->type = GRAPH_PIE;
	gph->lastXsamples = 0;
	gph->firstXsamples = 15;
	gph->depth = 3;
	gph->bar_space = 5;
	gph->gridx = 1;
	gph->gridy = 0;
	gph->tickx = 1;
	gph->ticky = 0;	
	gph->xlabel_style = 1;
	gph->xlabel_value = 0;

	CopyPrefsColors( &MyPrefStruct.graphcolors[GRAPHCOLORS_VBAR], gph );
}



/* ------------------------

	Draw each type of graph

---------------------------- */

// -------------------------------------------------------------------------------------


short StandardPlot::DrawbyVirtualHostsGraph( VDinfoP VDptr, long tot, short scalerType)
{
	struct graphOptions gph;

	CreateGraphtypeMulti3D( &gph );
	if( (gph.firstXsamples = tot) > 50 ) gph.firstXsamples = 50;
	mystrcpy( gph.filenameStr, FindReportFilenameStr(VHOST_PAGE) ); gph.filename = gph.filenameStr;
	gph.byStat = 0;
	gph.report_id = VHOST_PAGE;
	gph.title = "Virtual Hosts History";
	gph.ftitle = "Days";
	gph.xunits = ((long)(allendTime/ONEDAY)-(long)(allfirstTime/ONEDAY))+1;			// MAKE SURE its UnixTime and not JDseconds math
//	gph.xunits = VDptr->totalDays-1;
	gph.xlabels = 1;
	gph.ylabels = 1;
	gph.multiLinetotal = gph.xunits;
	gph.type = GRAPH_MULTIVHOST3D;
	gph.dataPtr = VDptr;
	gph.firstTime = allfirstTime;

	if( gph.firstXsamples > tot ) gph.firstXsamples = tot;

	if( MyPrefStruct.VDsortby == SORT_SIZES )		scalerType = SORT_BYTES;
	if( MyPrefStruct.VDsortby == SORT_REQUESTS )	scalerType = SORT_REQUESTS;
	if( MyPrefStruct.VDsortby == SORT_PAGES )		scalerType = SORT_PAGES;
		
	gph.scaler_type = scalerType;
	return DrawGraph( VDptr, &gph );
}

















void StandardPlot::CreateImageFromGifMem( unsigned char *gifPtr, long gifLen )
{
	im = gdImageCreateFromGifMem( gifPtr, gifLen );
}

void StandardPlot::CreateImage( int width, int height, int paletteType )
{
	im = gdImageCreate( width, height, paletteType );
}

short StandardPlot::DrawRegionGraph( VDinfoP VDptr, struct graphOptions *graph )
{
	long			num, i, lp, reg, perc, rgb, nrgb, palindex = 16, tc;
	short			regindex[] = { 0, 5, 7, 4, 3, 1, 6, 2 }; //// SA, EU, NM, ASIA, AFRIKA, AU , ANT
	StatList		*byStat;
	
	if( MyPrefStruct.report_format == FORMAT_EXCEL || MyPrefStruct.report_format == FORMAT_COMMA )
		return 1;

	CreateImageFromGifMem( worldmap_gif, WORLDMAP_GIF_LEN );
	Graph_DrawBase( graph, im->sx, im->sy, 0 );
	byStat = graph->byStat;

	num = byStat->GetStatListNum();
	// default all to grays
	for( i=1; i< 8; i++ ){
		palindex = i*16;
		for( lp=0; lp<16; lp++){
			nrgb = ScaleRGB( 100-(lp*5), 0xffffff );
			gdImageColorSetRGB( im, palindex++, nrgb );
		}
	}

	int level = 0;
	for( i=0; i< num; i++ )
	{
		__int64 val;
		reg = byStat->GetCounter2( i );
		if( reg ) {
			if( graph->scaler_type == SORT_BYTES ) { val=byStat->GetBytes( i ); perc = (val*100)/byStat->GetStatListTotalBytes(); }
			if( graph->scaler_type == SORT_REQUESTS ) { val=byStat->GetFiles( i ); perc = (val*100)/byStat->GetStatListTotalRequests(); }
			if( graph->scaler_type == SORT_PAGES ) { val=byStat->GetCounter4( i ); perc = (val*100)/byStat->GetStatListTotalCounters4(); }
			if( val && perc == 0) perc = 1;

			if( perc == 0 )
				rgb = 0xffffff;
			else
				rgb = PlotImageColorGetRGB( 240+level );

			palindex = regindex[ reg ] *16;
			for( lp=0; lp<10; lp++){
				nrgb = ScaleRGB( 100-(lp*3), rgb );
				gdImageColorSetRGB( im, palindex++, nrgb );
			}
			for( lp=0; lp<6; lp++){
				nrgb = ScaleRGB( 70-(lp*6), rgb );
				gdImageColorSetRGB( im, palindex++, nrgb );
			}
			level++;
		}
	}
	tc = PlotImageColorNearest( 0, 0, 0);
	PlotTransString( 1, 40,145, Translate( LABEL_HIGH ), tc, 0, 0, REM_HTML_TOKENS );
	PlotTransString( 1, 40,242, Translate( LABEL_LOW ), tc, 0, 0, REM_HTML_TOKENS );

	if( graph->m_thumbnail )
		Image_CompleteThumbNail( VDptr, graph, im );

	FW_SaveImage( VDptr, graph->filename );

	return 1;
}




short StandardPlot::DrawbyDataStyle( VDinfoP VDptr, StatList *byStat , long reportID, long typeID, char *filename, char *title, char *name, long sort, long depth, long labelTransId/*=0*/ )
{
	GraphDataStylesP graphData;
	short ret;
	graphOptions graphOptions;
	long	tot;

	graphData = FindGraphTypeData( reportID );
	if( !graphData )
		return 0;

	if( byStat )
		tot = byStat->GetStatListNum();

	if( tot > depth && depth )
		tot = depth;

	switch( graphData->style )
	{
		case GRAPH_VBAR:			CreateGraphtypeVBar( &graphOptions ); graphOptions.xunits = tot; break;
		case GRAPH_HBAR:			CreateGraphtypeHBar( &graphOptions ); if( tot>25 ) tot = 25; 	graphOptions.firstXsamples = graphOptions.yunits = tot;break;
		case GRAPH_LINE:			CreateGraphtypeLine( &graphOptions ); break;
		case GRAPH_PIE:				CreateGraphtypePie( &graphOptions ); break;
		case GRAPH_MULTI3D:			CreateGraphtypeMulti3D( &graphOptions ); if( tot>50 ) tot = 50;  break;
		case GRAPH_MULTIVBAR4:		CreateGraphtype4VBar( &graphOptions ); break;
		case GRAPH_MULTIVHOST3D:	CreateGraphtypeMulti3D( &graphOptions ); if( tot>50 ) tot = 50;  byStat = NULL; break;
		case GRAPH_SCATTER:			CreateGraphtypeScatterBar( &graphOptions ); break;
		case GRAPH_WORLD:			CreateGraphtypeHBar( &graphOptions ); break;
	}

	long *dataP;
	dataP = ConfigFindSettingsPtr( &MyPrefStruct, reportID );
	if( dataP )
	{
		long flags = *dataP;
		if( THUMB_ON(flags) )
			graphOptions.m_thumbnail = 1;
	}

	graphOptions.labelTranslateIdOverride=labelTransId;	// set optional translate id of graph label thus parameterising graph labels
	mystrcpy( graphOptions.filenameStr, filename );
	graphOptions.filename = graphOptions.filenameStr;
	graphOptions.firstTime = VDptr->firstTime;
	graphOptions.byStat = byStat;
	graphOptions.Vhost = VDptr;
	graphOptions.report_id = reportID;		// the _PAGE stuff in report.cpp
	graphOptions.type_id = typeID;			// the 4 char IDs, 'page' stuff in report.cpp
	graphOptions.type = graphData->style;
	graphOptions.scaler_type = sort;
	graphOptions.title = (char*)title;
	graphOptions.ftitle = (char*)name;

	if( graphData->col1 != -1 )	graphOptions.bar_color[0] = graphData->col1;
	if( graphData->col2 != -1 )	graphOptions.bar_color[1] = graphData->col2;
	if( graphData->col3 != -1 )	graphOptions.bar_color[2] = graphData->col3;
	if( graphData->bcol != -1 )	graphOptions.back_color = graphData->bcol;
	
	switch( graphData->style )
	{
		case GRAPH_LINE:
			if( reportID == DATE_PAGE )
				graphOptions.xunits = VDptr->totalDays-1;

			if( reportID == RECENTDATE_PAGE )
			{
				graphOptions.xunits = 31;
				graphOptions.lastXsamples = 31;
				//if( graph->byStat )
				//graph->byStat->GetStatListNum();
				//int rubbish = itempos - graphOptions.xunits;
				//if (rubbish > 0)
				//	itempos = itempos - rubbish;
				graphOptions.firstTime = VDptr->lastTime - (ONEDAY*graphOptions.xunits);
			}
			ret = DrawGraph( VDptr, &graphOptions);
			break;

		case GRAPH_MULTIVHOST3D:
			graphOptions.byStat = 0;
			graphOptions.dataPtr = VDptr;
			graphOptions.multiLinetotal = 
			graphOptions.xunits = ((long)(allendTime/ONEDAY)-(long)(allfirstTime/ONEDAY));
			graphOptions.firstTime = allfirstTime;
			graphOptions.dataPtr = VDptr;
			if( VDptr->domainTotal && graphOptions.firstXsamples > VDptr->domainTotal )
				graphOptions.firstXsamples = VDptr->domainTotal;
			else
				graphOptions.firstXsamples = 1;
			if( MyPrefStruct.VDsortby == SORT_SIZES )		graphOptions.scaler_type = SORT_BYTES;
			if( MyPrefStruct.VDsortby == SORT_REQUESTS )	graphOptions.scaler_type = SORT_REQUESTS;
			if( MyPrefStruct.VDsortby == SORT_PAGES )		graphOptions.scaler_type = SORT_PAGES;
			//CreateGraphtypeMulti3D( &graphOptions ); byStat = NULL;
			//DrawbyVirtualHostsGraph( VDptr, VDptr->domainTotal, 0 );
			ret = DrawGraph( VDptr, &graphOptions);
			break;

		case GRAPH_MULTI3D:	
			graphOptions.xunits = VDptr->totalDays-1;
			if( graphOptions.xunits == 0 )
				graphOptions.xunits = 1;
			graphOptions.multiLinetotal = VDptr->totalDays;
			graphOptions.firstXsamples = tot;
			ret = DrawGraph( VDptr, &graphOptions);
			break;

		case GRAPH_WORLD:
			ret = DrawRegionGraph( VDptr, &graphOptions );
			break;

		default:
			ret = DrawGraph( VDptr, &graphOptions);
			break;
	}
	
	//thePlot = NULL;

	return ret;
}



/*--------------------------------------------------------
	Draw all graphs in their own individual styles/colors
	and ways...
----------------------------------------------------------*/
void StandardPlot::WriteShadowImages( VDinfoP VDptr )
{ 
	char *s1;
	long bcol = 0xffffff;
	s1 = strstri( MyPrefStruct.html_head, "bgcolor" );
	if( s1 ){
		s1 = mystrchr( s1, '=' );
		if( s1 ){
			s1++;
			if( *s1 == '\"' ) {
				s1+=1;
			}
			if( *s1 == '#' ) {
				s1+=1;
			}
			bcol = (long)HexStr2Ptr( s1 );
		}
	}
	MakeBottomShadow( VDptr, bcol, GRAPH_WIDTH );
	MakeCornerShadow( VDptr, bcol, 0 );
	MakeRightShadow( VDptr, bcol, GRAPH_HEIGHT/2 );		//	gph->height = GraphWidth() * 72/100;
}


void StandardPlot::Plot3DPieGraph(int cx, int cy, int rad, double a1, double a2, int color, int color2 )
{
	gdImageFilled3DPie(im, cx, cy, rad*2, rad,  a1, a2, color, color2 );
}

