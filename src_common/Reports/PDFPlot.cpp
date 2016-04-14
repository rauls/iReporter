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
#include "PDFPlot.h"
#include "gd.h"
#include "gdfont_Geneva.h"
#include "gdfont_Arial.h"
#include "gdfonts.h"
#include "gdfontt.h"
#include "config_struct.h"
#include "editpath.h"
#include "datetime.h"
#include "translate.h"
#include "PDFFont.h"

#include "config.h"

static const long ONEK = 1024;
static const long HUNDREDK = 100*ONEK;
static const long ONEMEG = ONEK*ONEK;
static const long FIVEMEG = 5*ONEMEG;

extern long	black;

long PDFPlot::PlotImageColorGetRGB( int ct )
{
	long r,g,b, rgb;

	// Range check 
	if ( ct < 0 || ct > gdMaxColors )
		ct = 0;

	r = colors.red[ct]<< 16;
	g = colors.green[ct]<< 8;
	b = colors.blue[ct];
	rgb = r | g | b;

	return rgb;
}

#define ABS(a,b) ( (a>b) ? a-b : b-a )

int PDFPlot::PlotImageColorNearest( int r, int g, int b)
{
	int closest=0;
	int i;

	unsigned short closedist=0xffff,dist,d;

	for (i=0; (i<(colors.colorsTotal)); i++) {
		if (colors.open[i]) {
			continue;
		}
		dist=ABS(r,colors.red[i]);
		if (dist >= closedist) continue;
		d   =ABS(g,colors.green[i]);
		if (d>dist) dist=d;
		if (dist >= closedist) continue;
		d   =ABS(b,colors.blue[i]);
		if (d>dist) dist=d;
		if (dist >= closedist) continue;
		closest=i;
		if (dist==0) break;
		closedist=dist;
	}
	return closest;
}

int PDFPlot::PlotImageColorExact( int r, int g, int b )
{
	int i;
	for (i=0; (i<(colors.colorsTotal)); i++) {
		if (colors.open[i]) {
			continue;
		}
		if ((colors.red[i] == r) && 
			(colors.green[i] == g) &&
			(colors.blue[i] == b)) {
			return i;
		}
	}
	return -1;
}

int PDFPlot::PlotImageColorAllocate( int r, int g, int b )
{
	int i;
	int ct = (-1);

	if ( colors.useDefaultPalette ){
		return PlotImageColorNearest( r, g, b );
	}

	i = PlotImageColorExact( r, g, b );
	if ( i>=0 )
		return i;
	
	for (i=0; (i<(colors.colorsTotal)); i++) {
		if (colors.open[i]) {
			ct = i;
			break;
		}
	}	
	if (ct == (-1)) {
		ct = colors.colorsTotal;
		if (ct == gdMaxColors) {
			return -1;
		}
		colors.colorsTotal++;
	}
	colors.red[ct] = r;
	colors.green[ct] = g;
	colors.blue[ct] = b;
	colors.open[ct] = 0;
	return ct;
}

int PDFPlot::PlotImageColorAllocateRGB( long rgb )
{
	long r,g,b;
	r = (rgb & 0xff0000) >> 16;
	g = (rgb & 0xff00) >> 8;
	b = (rgb & 0xff);

	return PlotImageColorAllocate( r, g, b );
}

extern void WriteTimeTakenInfo( VDinfoP VDptr, std::string infoDesc, int info );

int pdf_currHeight;


PDFPlot::PDFPlot( VDinfoP VDptr, PDFSettings thePDFSettingsP, PDFGraphSettings *thePDFGraphSettingsP, PDFCharScaleFont *timesNRFontP, float currX, float currY, float drawWidth )
: StandardPlot()
{
	thePDFSettings = thePDFSettingsP;
	thePDFGraphSettings = thePDFGraphSettingsP;
	timesNRFont = timesNRFontP;
	timesNRFont->ResetFont();
	scaleFactor = drawWidth / (double)GRAPH_WIDTH;

	// Set the offset's for the position of the graph.
	xOffset = currX;
	if ( currY == 0 )
		yOffset = thePDFSettings.TopMargin() + pdf_currHeight;
	else
		yOffset = currY + pdf_currHeight;

	TrevTempVDptr = VDptr;

	graphData.reserve( FIVEMEG );

	graphData += "0 J 0 j 0.005 w 10 M []0 d\r"; // PDF line thickness (0.005)... dunno about the other stuff??

	// Debug Info...
	bufS = graphData.capacity(); 
	WriteTimeTakenInfo( TrevTempVDptr, "graphData.capacity()", bufS );
}

int PDFPlot::GraphHeight( void )
{	return 420 * scaleFactor;
}

PDFPlot::~PDFPlot()
{
	bufS = graphData.size(); 
	WriteTimeTakenInfo( TrevTempVDptr, "graphData.size()", bufS );
}


void PDFPlot::PlotImageFilledPolygon( gdPointPtr p, int n, int c )
{
	if ( graphData.capacity() - HUNDREDK < graphData.size()  )
		graphData.reserve( graphData.capacity() + HUNDREDK );

	if ( thePDFSettings.CommentsOn() )
		graphData += "% Filled\r";
	graphData += GetRGBFillColorStr( c );
	PlotImagePolygon( p, n, c, 1 );
}

void PDFPlot::PlotImagePolygon( gdPointPtr p, int n, int c, int fill /*= 0*/ )
{
	int i;
	float lx, ly;

	if (!n) {
		if ( thePDFSettings.CommentsOn() )
			graphData += "% Attempted Polygon draw";
		return;
	}
	
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Polygon\r";
	lx = p->x;
	ly = p->y;
	PlotLine( lx, ly, p[1].x, p[1].y, c);
	for (i=2; (i < n); i++) {
		PlotLineTo( p[i].x, p[i].y, c);
	}
	PlotLineTo( lx, ly, c);
	if ( fill )
		graphData += "B\r"; // Closes, fills and draws the polygon
	else
		graphData += "s\r"; // Closes and draws the polygon
}

std::string PDFPlot::GetRGBTextColorStr( long color )
{
	return timesNRFont->TextColor( PlotImageColorGetRGB( color ) );
}

std::string PDFPlot::GetRGBFillColorStr( long color )
{
	
	return timesNRFont->FillColor( PlotImageColorGetRGB( color ) );
}

short PDFPlot::ConvertFont( int fontsize )
{
	switch ( fontsize )
	{
	case 0:
		return (thePDFGraphSettings->smallFontSize*scaleFactor)+1; // small size
	case 1:
		return (thePDFGraphSettings->normalFontSize*scaleFactor)+1; // normal size
	case 2:
		return (thePDFGraphSettings->largeFontSize*scaleFactor)+1; // large size
	case 3:
		return (thePDFGraphSettings->tinyFontSize*scaleFactor)+1; // tiny size
	}
	// Maybe the fontsize has already been converted
	return fontsize;
}

short PDFPlot::UpdateFontSize( long fontsize )
{
	return ConvertFont( fontsize );
}


void PDFPlot::PlotText(long fontsize, double x, double y, const char *string, long color, int rotate /*= NO_ROTATE*/, int bold /*= 0*/, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	graphData += GetRGBTextColorStr( color );
	graphData += GetRGBFillColorStr( color );

	graphData +=  "BT\r";
	short font = ConvertFont( fontsize );
	
	graphData += timesNRFont->FontChangeStr( bold ? PDF_BOLD : PDF_NORMAL,
		                                     font,
											 PlotImageColorGetRGB( color )
											 );

	if ( rotate ) // If we are rotating the text vertically we need to make adjustments
	{
		graphData += "0 1 -1 0 ";
		x = x + font;
		y = y - font;
	}

	// Position of the text
	graphData += ScaleX( x );
	graphData += " ";
	graphData += ScaleY( y + font );

	if ( rotate ) 
		graphData += " Tm\r"; // Need to change the text matrix
	else
		graphData += " Td\r"; // Standard text positioning
	graphData += "0 Tc\r(";

	// The text to be shown
	const char *removedHTMLTokensStr;
	if ( remHTMLTokens )
	{
		removedHTMLTokensStr = ConvertHTMLTokens( string );
		graphData += timesNRFont->MakePDFString( removedHTMLTokensStr );
	}
	else
		graphData += timesNRFont->MakePDFString( string );
	graphData += ")Tj\r";
	graphData += "ET\r";
}


void PDFPlot::PlotNumRightJustified(long fontsize, double x, double y, const char *string, long color )
{
	PlotStringRightJustified( fontsize, x, y, string, color );
}

void PDFPlot::PlotTransStringRightJustified(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	PlotStringRightJustified( fontsize, x, y, string, color, remHTMLTokens );
}

void PDFPlot::PlotStringRightJustified(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	graphData += GetRGBTextColorStr( PDF_BLACK );
	graphData += GetRGBFillColorStr( PDF_BLACK );
	graphData +=  "BT\r";
	short font = ConvertFont( fontsize );
	graphData += timesNRFont->FontChangeStr( PDF_NORMAL, font );
//	graphData += timesNRFont->FontChangeStr( testStyle, font );

	// Position of the text
	const char *removedHTMLTokensStr;
//	static std::string removedHTMLTokensStr;
	if ( remHTMLTokens )
		removedHTMLTokensStr = ConvertHTMLTokens( string );
	else
		removedHTMLTokensStr = string;

	double len = GetStringLen( removedHTMLTokensStr, fontsize );
	graphData += floatToStr( ScaleXNum( x ) - len );
	graphData += " ";
	graphData += ScaleY( y + ConvertFont(font) );

	graphData += " Td\r"; // Standard text positioning
	graphData += "0 Tc\r(";

	// The text to be shown
	graphData += timesNRFont->MakePDFString( removedHTMLTokensStr );
	graphData += ")Tj\r";
	graphData += "ET\r";
}

double PDFPlot::GetTransStringLen( const char* str, int size )
{
	return GetStringLen( str, size );
}

double PDFPlot::GetStringLen( const char* str, int size )
{
	short font = ConvertFont( size );
	return timesNRFont->GetStringLen( str, font, PDF_CURRENT_STYLE );
}

std::string PDFPlot::SetTransStringLen( std::string str, double lenWanted, int size /*= 1*/, short truncateTextLeftOrRight /*= 0*/ )
{
	return SetStringLen( str, lenWanted, size, truncateTextLeftOrRight );
}

std::string PDFPlot::SetStringLen( std::string str, double lenWanted, int size /*= 1*/, short truncateTextLeftOrRight /*= 0*/ )
{
	short font = ConvertFont( size );
	return timesNRFont->SetStringLen(str, lenWanted, font, truncateTextLeftOrRight, PDF_CURRENT_STYLE );
}

const char* PDFPlot::ConvertHTMLTokens( const char *text )
{
	static std::string transTextStr;
	if ( MyPrefStruct.language )
	{
		transTextStr = text;
		timesNRFont->TranslateHTMLSpecialTokens( transTextStr );
		return transTextStr.c_str();
	}
	return text;
}

void PDFPlot::PlotStringBold(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	PlotString( fontsize, x, y, string, color, 0, remHTMLTokens );
}

void PDFPlot::PlotTextC(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	const char *removedHTMLTokensStr;
	short font = ConvertFont( fontsize );
	int centerPos;
	if ( remHTMLTokens )
	{
		removedHTMLTokensStr = ConvertHTMLTokens( string );
		centerPos = x - (GetStringLen(removedHTMLTokensStr, fontsize) / (double)2);
		PlotText( font, centerPos, y, removedHTMLTokensStr, color );
	}
	else
	{
		centerPos = x - (GetStringLen(string, fontsize) / (double)2);
		PlotText( font, centerPos, y, string, color );
	}
}

void PDFPlot::PlotTransStringC(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	gdFont *tf = GetFontStruct(fontsize);
	PlotTransString( fontsize, x - (GetTransStringLen(string,fontsize)/2), y, string, color, 0, 0, remHTMLTokens );
}


void PDFPlot::PlotTextUp(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	PlotText( fontsize, x, y, string, color, ROTATE, remHTMLTokens );
}



void PDFPlot::PlotStringUp(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	PlotText( fontsize, x, y, string, color, ROTATE, remHTMLTokens );
}

void PDFPlot::PlotTransStringUp(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	PlotText( fontsize, x, y, string, color, ROTATE, remHTMLTokens );
}

void PDFPlot::PlotString(long fontsize, double x, double y, const char *string, long color, int rotate /*= NO_ROTATE*/, int bold /*= 0*/, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ )
{
	PlotText( fontsize, x, y, string, color, rotate, bold, remHTMLTokens );
}

void PDFPlot::PlotTransString( long fontsize, double x, double y, const char *string, long color, int rotate, int bold, int translate )
{
	PlotText( fontsize, x, y, string, color, rotate, bold, translate );
}

void PDFPlot::PlotXScatterValues( long mins, long i2, long x, long xsize, long hcount, long ypos, long ysize, long grid_color )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Scatter\r";
	short font = ConvertFont( thePDFGraphSettings->normalFontSize );
	graphData += timesNRFont->FontChangeStr( PDF_NORMAL, font );
//	graphData += timesNRFont->FontChangeStr( testStyle, font );
	long i,lastx=0,x2; char name[64];
	for( i=0; i< mins; i+=i2 ){
		x2 = x + xsize;
		if ( (x2-lastx) > 16 ){
			PlotDrawLine( x2, ypos-ysize, x2, ypos+3, grid_color );
			sprintf( name, "%d", i );
			PlotNum( 1, x2-6+(GetStringLen( name, timesNRFont->FontSize() )/2), ypos+5, name, black );	// draw the current bars NAME/FILE
			lastx = x2;
		}
	}
}

void PDFPlot::Plot3dHorizBox( long x, long y, long barsize, long ystep, long c1, long c2, long c3, long depth )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Horz Bar...\r";

	y += depth;

	// Horizontal rectangle
	PlotFilledRect( x, y+1, x+barsize, (int)y+ystep, c2 );	// draw bar
	int xpos = x, ypos = y+1;
	gdPoint	poly[4];
	poly[0].x = xpos;				poly[0].y = ypos;
	poly[1].x = xpos+barsize;		poly[1].y = ypos;
	poly[2].x = xpos+barsize+depth;	poly[2].y = ypos-depth;
	poly[3].x = xpos+depth;			poly[3].y = ypos-depth;
	
	if ( c1 >= 0 )
		PlotImageFilledPolygon( poly, 4, c1 ); // Polygon above rectangle

	xpos = x+barsize;				ypos = y+ystep;
	poly[0].x = xpos;				poly[0].y = ypos;
	poly[1].x = xpos;				poly[1].y = y+1;
	poly[2].x = xpos+depth;			poly[2].y = y-depth+1;
	poly[3].x = xpos+depth;			poly[3].y = ypos-depth;
	
	if ( c3 >= 0 )
		PlotImageFilledPolygon( poly, 4, c3 ); // Polygon next to rectangle
}
void PDFPlot::Plot3dHorizGradBox( int x, int y, int xs, int ys, int c1, int c2, int c3, int depth )
{
	Plot3dHorizBox( x, y, xs, ys, c1, c2, c3, depth );	// draw bar
	return;
}

void PDFPlot::Plot3dVertGradBox( double x, double y, double xs, double ys, int c1, int c2, int c3, int depth )
{
	if( ys < 0 || ys > 1000 || y < 0) return;

	if ( xs > 640 ) xs = 640;
	if ( ys > 400 ) ys = 400;
	if ( xs < 0 ) xs=0;

	int xpos = x, ypos = y;
	gdPoint	poly[5];

	// Top side of shadow
	poly[0].x = xpos;			poly[0].y = ypos;
	poly[1].x = xpos+xs;		poly[1].y = ypos;
	poly[2].x = xpos+xs+depth;	poly[2].y = ypos-depth;
	poly[3].x = xpos+depth;		poly[3].y = ypos-depth;
	poly[4].x = xpos;			poly[4].y = ypos;
	if ( c1 >= 0 )
		PlotImageFilledPolygon( poly, 5, c1 );

	// Right side of shadow
	xpos = x+xs;				ypos = y+ys;
	poly[0].x = xpos;			poly[0].y = ypos;
	poly[1].x = xpos+depth;		poly[1].y = ypos-depth;
	poly[2].x = xpos+depth;		poly[2].y = y-depth;
	poly[3].x = xpos;			poly[3].y = y;
	poly[4].x = xpos;			poly[4].y = ypos;
	if ( c3 >= 0 )
		PlotImageFilledPolygon( poly, 5, c3 );

	// The column
	PlotFilledRect( x, y, x+xs, y+ys, c2 );	// draw bar
}

void PDFPlot::PlotVertbar( double xpos, double ypos, double xs, double ys, long bar1, long bar2, long bar3, long depth )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Vert Bar...\r"; // 

	Plot3dVertGradBox( xpos, ypos, xs, (int)ys, bar1, bar2, bar3, depth );	// draw bar
}

void PDFPlot::PlotMoveTo( float x, float y )
{
	graphData += ScaleX( x );
	graphData += " ";
	graphData += ScaleY( y );
	graphData += " m\r";
}

void PDFPlot::PlotLineTo( float x, float y, long lcol )
{
	graphData += GetRGBTextColorStr( lcol );
	graphData += ScaleX( x );
	graphData += " ";
	graphData += ScaleY( y );
	graphData += " l\r";
}

void PDFPlot::PlotLine( float x, float y, float x2, float y2, long lcol )
{
	PlotMoveTo( x, y );
	PlotLineTo( x2, y2, lcol );
}

void PDFPlot::PlotDrawLine( float x, float y, float x2, float y2, long lcol )
{
	graphData += GetRGBTextColorStr( lcol );
	PlotLine( x, y, x2, y2, lcol );
	graphData += "S\r";
}

void PDFPlot::PlotDottedLine( float x, float y, float x2, float y2, long dcol )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Dashed Line\r";
	// Set dash format 6 pixels of dash, 3 pixels off
	graphData += "[";
	graphData += floatToStr( thePDFGraphSettings->dottedLineStokeSize );
	graphData += " ";
	graphData += floatToStr( thePDFGraphSettings->dottedLineNonStokeSize );
	graphData += "] 0 d\r";
	PlotDrawLine( x, y, x2, y2, dcol ); 
	graphData += "[ ] 0 d\r"; // Change the line output back to solid line
}
void PDFPlot::PlotPoint( float x, float y, long pcol )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Point\r";

	PlotDrawLine( x, y, x, y, pcol ); 
}

void PDFPlot::PlotRect( float x, float y, float x2, float y2, long rcol )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Rect\r";

	graphData += GetRGBTextColorStr( rcol );

	graphData += ScaleX( x );
	graphData += " ";
	graphData += ScaleY( y );
	graphData += " m\r";

	graphData += ScaleX( x2 );
	graphData += " ";
	graphData += ScaleY( y );
	graphData += " l\r";

	graphData += ScaleX( x2 );
	graphData += " ";
	graphData += ScaleY( y2 );
	graphData += " l\r";

	graphData += ScaleX( x );
	graphData += " ";
	graphData += ScaleY( y2 );
	graphData += " l\r";

	graphData += ScaleX( x );
	graphData += " ";
	graphData += ScaleY( y );
	graphData += " l\rS\r";
}

void PDFPlot::PlotFilledRect( float x, float y, float x2, float y2, long frcol )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Filled Rect\r";

	graphData += GetRGBFillColorStr( frcol );
	graphData += ScaleX( x );
	graphData += " ";
	graphData += ScaleY( y );
	graphData += " ";
	graphData += floatToStr( ((x2-x) * scaleFactor) );
	graphData += " ";
	graphData += floatToStr( (y-y2) * scaleFactor );
	graphData += " re\rf\r";
}

void PDFPlot::CreateImage( int width, int height, int paletteType )
{
	gdImageWebColors( &colors, paletteType );
}


#define	PI				3.1415926535897932384626433832795
#define	RADTODEG(x)		(x*180/PI)
#define	DEGTORAD(x)		(x*PI/180)

static const double NINTY_DBLE = 90.000000000000;
static const float SMALL_FLOAT = static_cast<float>(0.0001);
static const double THREESIXTY_DBLE = 360.000000000000;
static long Plot3DPieGraphCount = 0;
void PDFPlot::Plot3DPieGraph(int cx, int cy, int rad, double a1, double a2, int color, int color2 )
{
	double yScale = 0.5;
	double x, y;
	double rad_1;
	double angleDiff = (float)(a2-a1); // This needs to be a float for rounding purposes!!!

	if ( Plot3DPieGraphCount >= 5 )
	{
		OutDebug( "PDFPlot::Plot3DPieGraph - Entered recursive function too many times, please look at!" );
		return;
	}

	Plot3DPieGraphCount++;

	OutDebugs( "Angle 1 = %.12f, 2 = %.12f, diff = %.12f", a1, a2, angleDiff );

	if ( thePDFSettings.CommentsOn() )
	{
		if ( angleDiff <= NINTY_DBLE )
			graphData += "% Pie 3d Slice\r";
	}
	// Make sure both a1 and a2 are less than are only at most 90 degrees apart,
	// otherwise the drawing will be stuffed...

	if ( (float)angleDiff > (float)NINTY_DBLE + SMALL_FLOAT )
	{
		Plot3DPieGraph( cx, cy, rad, a1, a1+NINTY_DBLE, color, color2 );
		Plot3DPieGraph( cx, cy, rad, a1+NINTY_DBLE, a2, color, color2 );
		Plot3DPieGraphCount--;
		return;
	}
	// Make sure that 360 degreee is not between a1 and a2,
	// as we have to put a the bottom cyclinder on pie segments where it goes over 360 degrees
	if ( (float)a1 < (float)THREESIXTY_DBLE && (float)a2 > (float)THREESIXTY_DBLE )
	{
		Plot3DPieGraph( cx, cy, rad, a1, THREESIXTY_DBLE, color, color2 );
		Plot3DPieGraph( cx, cy, rad, THREESIXTY_DBLE, a2, color, color2 );
		Plot3DPieGraphCount--;
		return;
	}
	
	rad_1 = DEGTORAD(a1);

	gdPoint	polyr[5]; // Structure to hold the Arc points

	// Center of the pie
	polyr[0].x = cx;		polyr[0].y = cy; // Center
	
	// Line from centre of the "Pie" to the start of the Arc
	x = cos( rad_1 )*rad;
	y = sin( rad_1 )*rad;
	polyr[1].x = cx + x;	polyr[1].y = cy + y * yScale; // Start of Arc

	// Calculate the arc points
	CalculateArc( &polyr[2], cx, cy, x, y, yScale, a1, a2, rad );

	// Draw the Pie slice
	PlotImagePieSlice( polyr, a1, a2, color, 1 );

	if ( a1 >= 180 && a1 < 360 )
	{
		Plot3DPieGraphCount--;
		return;	// Don't draw the drop down 3d cyclinder as we need to be above 180 degrees
	}

	// Now draw the drop down 3d cyclinder look
	gdPoint	pie3dCylLook[8]; // Structure to hold the Arc points
	pie3dCylLook[0].x = cx + x;	pie3dCylLook[0].y = cy + y * yScale;
	CalculateArc( &pie3dCylLook[1], cx, cy, x, y, yScale, a1, a2, rad );
	yScale *= 1.05;
	cy += rad*0.1;
	pie3dCylLook[4].x = cx + x;	pie3dCylLook[4].y = cy + y * yScale;
	CalculateArc( &pie3dCylLook[5], cx, cy, x, y, yScale, a1, a2, rad );
	PlotImagePieSlice3dCylinder( pie3dCylLook, a1, a2, color2, 1 );
	Plot3DPieGraphCount--;
}


void PDFPlot::CalculateArc( gdPointPtr p, int cxP, int cyP, int xP, int yP, double yScale, double a1, double a2, int rad ) 
{
	//
	// The Arc of the "Pie Slice"
	//
	double angle1, angle2;

	double rangle1, rangle2;
	double diffRad1, diffRad2;
	double lenPtFromCirRad1, lenPtFromCirRad2;
	double realCirPosX1, realCirPosY1, realCirPosX2, realCirPosY2;
	double extraX1, extraY1, extraX2, extraY2;
	double x = xP, y = yP, cx = cxP, cy = cyP;

	// Start point of the Arc
	p[0].x = cx + x;			p[0].y = cy + y * yScale;

	//
	// To draw the Arc, we need to have two points which "guide" the drawing to the Arc from
	// the beginning point to the end point
	//

	// 1st guide point of Arc
	angle1 = a2-((a2-a1)/3*2);
	rangle1 = DEGTORAD(angle1);
	diffRad1 = DEGTORAD( (a1 - angle1) );
	lenPtFromCirRad1 = (1-cos(diffRad1))*rad; // Distance of tangent of "air" point from radius
	realCirPosX1 = (cos(rangle1)*rad);
	extraX1 = cos( rangle1 ) * lenPtFromCirRad1 ;
	x = realCirPosX1 + extraX1; // Radius + "x" value from Radius 
	realCirPosY1 = (sin(rangle1)*rad) ;
	extraY1 = sin( rangle1 ) * lenPtFromCirRad1;
	y = realCirPosY1 + extraY1; // Radius + "y" value from Radius 

	p[0].x = cx + x;			p[0].y = cy + y * yScale;

	// 2nd guide point of Arc
	angle2 = a2-((a2-a1)/3);
	rangle2 = DEGTORAD(angle2);
	diffRad2 = DEGTORAD( (angle1 - angle2) );
	lenPtFromCirRad2 = (1-cos(diffRad2))*rad; // Distance of tangent of "air" point from radius
	realCirPosX2 = (cos(rangle2)*rad);
	extraX2 = cos( rangle2 ) * lenPtFromCirRad2 ;
	x = realCirPosX2 + extraX2; // Radius + "x" value from Radius 
	realCirPosY2 = (sin(rangle2)*rad) ;
	extraY2 = sin( rangle2 ) * lenPtFromCirRad2;
	y = realCirPosY2 + extraY2; // Radius + "y" value from Radius 

	p[1].x = cx + x;			p[1].y = cy + y * yScale;

	// End point of Arc
	double rad_2 = DEGTORAD(a2);
	x = cos(rad_2)*rad;
	y = sin(rad_2)*rad;

	p[2].x = cx + x;			p[2].y = cy + y * yScale;
}


void PDFPlot::PlotImagePieSlice( gdPointPtr p, double a1, double a2, int color, int fill /*= 0*/ )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Pie Flat Slice\r";

	graphData += GetRGBTextColorStr( color );
	graphData += GetRGBFillColorStr( color );

	// Plot the line from the center to the start of the arc
	PlotLine( p[0].x, p[0].y, p[1].x, p[1].y, color);

	// Plot the Arc
	PlotArc( &p[2], a1, a2 );

	// Plot the line from the end of the arc to the center 
	PlotLineTo( p[0].x, p[0].y, color );

	if ( fill )
		graphData += "b\r"; // Closes, fills and draws the polygon
	else
		graphData += "s\r"; // Closes and draws the polygon
}


void PDFPlot::PlotArc( gdPointPtr p, double a1, double a2, short reverse /*= 0*/ )
{
	// PDF Comments
	if ( thePDFSettings.CommentsOn() )
	{
		graphData += "% Arc ";
		graphData += floatToStr( a1 );
		graphData += " ";
		graphData += floatToStr( a2 );
		graphData += "\r";
	}

	// Plot the Arc
	if ( !reverse )
	{
		for (int i=0; (i <= 2); i++) {
			graphData += ScaleX( p[i].x );
			graphData += " ";
			graphData += ScaleY( p[i].y );
			graphData += " ";
		}
	}
	else
	{
		for (int i=2; (i >= 0); i--) {
			graphData += ScaleX( p[i].x );
			graphData += " ";
			graphData += ScaleY( p[i].y );
			graphData += " ";
		}
	}
	graphData += "c\r";
}


void PDFPlot::PlotImagePieSlice3dCylinder( gdPointPtr pie3dCylLook, double a1, double a2, int color, int fill /*= 0*/ )
{
	if ( thePDFSettings.CommentsOn() )
		graphData += "% Pie Bottom Cylinder\r";

	graphData += GetRGBTextColorStr( color );
	graphData += GetRGBFillColorStr( color );

	// Plot the Arc which goes along the bottom of the pie slice
	PlotMoveTo( pie3dCylLook[0].x, pie3dCylLook[0].y ); 
	PlotArc( &pie3dCylLook[1], a1, a2 );

	// Plot the 2nd Arc which is offset from the first so that we get the 3d cyclinder look
	PlotLineTo( pie3dCylLook[7].x, pie3dCylLook[7].y, color ); 
	PlotArc( &pie3dCylLook[4], a1, a2, 1 );

	if ( fill )
		graphData += "b\r"; // Closes, fills and draws the polygon
	else
		graphData += "s\r"; // Closes and draws the polygon
}

void PDFPlot::PlotBevel( int x, int y, int xs, int ys, int c1, int c2, int c3 )
{
	int x2,y2;
	if ( xs > 640 ) xs = 640;
	if ( ys > 400 ) ys = 400;

	x2 = x + (xs-1);
	y2 = y + (ys-1);

	PlotFilledRect(x, y, x2, y2, c3);			/* dark */
	PlotFilledRect(x, y, x2-1, y2-1, c1);		/* bright */
	PlotFilledRect(x+1, y+1, x2-1, y2-1, c2);	/* middle */
}



