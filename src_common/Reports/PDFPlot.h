

#ifndef GRAPHPDF_H
#define GRAPHPDF_H

#include <string>
#include "StandardPlot.h"
#include "PDFFont.h"
#include "PDFSettings.h"
#include "gd.h"


#define	TINY_FONT	0
#define	SMALL_FONT	1
#define	NORMAL_FONT	2
#define	LARGE_FONT	3

//
// PDFPlot class - set of "graph" drawing functions to produce PDF "draw" commands
//
//class PDFPlot;
class PDFPlot : public StandardPlot
{
public:
	PDFPlot( VDinfoP VDptr, PDFSettings thePDFSettingsP, PDFGraphSettings *thePDFGraphSettingsP, PDFCharScaleFont *timesNRFontP, float currX, float currY, float drawWidth );
	virtual ~PDFPlot();

	// Functions which actually do the "literal" string writing.  That is, they write the string "as is" with no language translation etc...
	// However, these functions do check for and make amends for invalid characters specific to the "report/graph format" being used
	virtual void PlotText( long fontsize, double x, double y, const char *string, long color, int rotate = NO_ROTATE, int bold = 0, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotTextC( long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotTextUp(long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotStringUp( long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotTransStringC(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ );
	virtual void PlotTransStringUp( long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ );
	virtual void PlotTransString( long fontsize, double x, double y, const char *string, long color, int rotate, int bold, int translate );
	virtual void PlotString( long fontsize, double x, double y, const char *string, long color, int rotate = NO_ROTATE, int bold = 0, int translate = NO_REM_HTML_TOKENS );

	// Functions which must convert the "given"	string somehow, either by a language translation, number conversion, boldness etc...
	virtual void PlotStringBold(long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotStringRightJustified(long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotTransStringRightJustified(long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotNumRightJustified(long fontsize, double x, double y, const char *string, long color );

	// Functions for string alteration & information
	virtual double GetStringLen( const char* str, int size = 1 );
	virtual double GetTransStringLen(const char* str, int size = 1);
	virtual std::string SetStringLen( std::string str, double lenWanted, int size = 1, short truncateTextLeftOrRight = 0  );
	virtual std::string SetTransStringLen( std::string str, double lenWanted, int size = 1, short truncateTextLeftOrRight = 0 );
	virtual short TruncateStyle() { return thePDFSettings.TruncateTextLeftOrRight(); }

	virtual const char* ConvertHTMLTokens( const char *txt );
//	std::string MakePDFString( std::string str ) { return timesNRFont->MakePDFString( str ); }

	// Functions for simple graphics drawing
	virtual void PlotPoint( float x, float y, long pcol );
	virtual void PlotLine( float x, float y, float x2, float y2, long lcol );
	virtual void PlotDrawLine( float x, float y, float x2, float y2, long lcol );
	virtual void PlotDottedLine( float x, float y, float x2, float y2, long dcol );
	virtual void PlotRect( float x, float y, float x2, float y2, long rcol );
	virtual void PlotFilledRect( float x, float y, float x2, float y2, long frcol );
	virtual void PlotImagePolygon(gdPointPtr p, int n, int c, int fill = 0 );
	virtual void PlotImageFilledPolygon( gdPointPtr p, int n, int c );
	void PlotMoveTo( float x, float y );
	void PlotLineTo( float x, float y, long lcol );

	// Functions for more complicated graphics drawing
	virtual void PlotXScatterValues( long mins, long i2, long x, long xsize, long hcount, long ypos, long ysize, long grid_color );
	virtual void Plot3dHorizBox( long xpos, long ypos, long barsize, long ystep, long bar1, long bar2, long bar3, long depth );
	virtual void PlotVertbar( double xpos, double ypos, double xs, double ys, long bar1, long bar2, long bar3, long depth );
	virtual void Plot3dHorizGradBox( int x, int y, int xs, int ys, int c1, int c2, int c3, int depth );
	virtual void Plot3dVertGradBox( double x, double y, double xs, double ys, int c1, int c2, int c3, int depth );
	virtual void Plot3DPieGraph(int cx, int cy, int rad, double a1, double a2, int color, int color2 );
	void CalculateArc( gdPointPtr p, int cx, int cy, int x, int y, double yScale, double a1, double a2, int rad );
	void PlotImagePieSlice( gdPointPtr p, double a1, double a2, int c, int fill = 0 );
	void PlotImagePieSlice3dCylinder( gdPointPtr pie3dCylLook, double a1, double a2, int color, int fill = 0 );
	void PlotArc( gdPointPtr p, double a1, double a2, short reverse = 0 );

	std::string GraphData() { return graphData.c_str(); }
	int GraphHeight( void );
	virtual long PlotImageColorGetRGB( int ct );
	//virtual void AllocDefaultColors();
	virtual int PlotImageColorNearest( int r, int g, int b);
	virtual int PlotImageColorExact( int r, int g, int b );
	virtual int PlotImageColorAllocate( int r, int g, int b );
	virtual int PlotImageColorAllocateRGB( long rgb );
	//virtual int PlotImageColorSet(gdImagePtr im, int ct, int r, int g, int b);
//	virtual void SetGraphColors( struct graphOptions *graph, long& grid_color, long& gcolor, long& bar1, long& bar2, long& bar3 );

	virtual void FW_SaveImage( VDinfoP VDptr, char *name ) {}
	virtual void MakeBottomShadow( VDinfoP VDptr, long bcolor, long isize ) {}
	virtual void MakeCornerShadow( VDinfoP VDptr, long bcolor, long isize ) {}
	virtual void MakeRightShadow( VDinfoP VDptr, long bcolor, long isize ) {}
	virtual void WriteShadowImages( VDinfoP VDptr ) {}

	virtual void CreateImage( int width, int height, int paletteType );
	virtual void CreateImageFromGifMem( unsigned char *gifPtr, long gifLen ) {}
	virtual void DestroyImage() {}
	virtual void PlotBevel( int x, int y, int xs, int ys, int c1, int c2, int c3 );


private:
	// Functions to scale the graphs to fit across the page nicely
	std::string ScaleX( float num ){ return floatToStr( (num * scaleFactor) + xOffset ); }
	virtual double ScaleXNum( double num ) { return (num * scaleFactor) + xOffset ; }
	std::string ScaleY( float num ){ return floatToStr( thePDFSettings.PageHeight() - (num * scaleFactor) - yOffset ); }
	double ScaleYNum( double num ){ return thePDFSettings.PageHeight() - (num * scaleFactor) - yOffset; }

	// Functions for conversion of types
	std::string floatToStr( float num )
	{
		sprintf( buf, "%.7g", num );
		intStr = buf;
		return intStr;
	}

	// Functions to change font size being used
	short UpdateFontSize( long fontsize);
	inline short ConvertFont( int fontsize );

	// Functions to change the color being used
	inline std::string GetRGBFillColorStr( long color );
	inline std::string GetRGBTextColorStr( long color );

	std::string intStr;
	char buf[32];
	std::string graphData;
	//std::string colBuf;
	//long currFillCol;
	//long currStrokeCol;
	int drawWidth;
	double scaleFactor;
	//int currFontSize;
	int yValueMin;
	float xOffset;
	float yOffset;

	int bufS;
	VDinfoP TrevTempVDptr;
	PDFCharScaleFont *timesNRFont;
	PDFSettings thePDFSettings;
	PDFGraphSettings *thePDFGraphSettings;

	gdImageRGBColors colors;
};

struct graphLabels {
	long x;
	long y;
	char *text;
};

#endif

