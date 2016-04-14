#ifndef StandardPlot_H
#define StandardPlot_H

#include <string>
#include "VirtDomInfo.h"	// only for VDinfoP!
#include "gd.h"


#define	PIE_COLS	9
#define	MULTI_COLS	10

#define NO_ROTATE		0
#define ROTATE			1

#define NO_REM_HTML_TOKENS	0
#define REM_HTML_TOKENS		1

//
// StandardPlot class - class to encapsulate previous "C" function calls for the existing image formats
// Created this class so that we could override the classes function set for creating PDF graphs
//
class StandardPlot
{
public:
	StandardPlot();
	gdImagePtr im;
	virtual ~StandardPlot();

	virtual void CreateImage( int width, int height, int paletteType );
	virtual void CreateImageFromGifMem( unsigned char *gifPtr, long gifLen );
	virtual void DestroyImage();

	// Functions which actually do the "literal" string writing.  That is, they write the string "as is" with no language translation etc...
	// However, these functions do check for and make amends for invalid characters specific to the "report/graph format" being used
	virtual void PlotText( long fontsize, double x, double y, const char *string, long color, int rotate = NO_ROTATE, int bold = 0, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotTextC( long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotTextUp(long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	
	// Functions which must convert the "given"	string somehow, either by a language translation, number conversion, boldness etc...
	void PlotNum( long fontsize, double x, double y, const char *string, long color ){
		PlotText( fontsize, x, y, string, color ); }
	void PlotNumC( long fontsize, double x, double y, const char *string, long color ){
		PlotTextC( fontsize, x, y, string, color ); }
	virtual void PlotTransStringC(long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ );
	virtual void PlotTransStringUp( long fontsize, double x, double y, const char *string, long color, int remHTMLTokens /*= NO_REM_HTML_TOKENS*/ );
	virtual void PlotTransString( long fontsize, double x, double y, const char *string, long color, int rotate, int bold, int translate );
	virtual void PlotString( long fontsize, double x, double y, const char *string, long color, int rotate = NO_ROTATE, int bold = 0, int translate = NO_REM_HTML_TOKENS );
	void PlotStringConvertHTMLTokens( long fontsize, double x, double y, const char *string, long color, int rotate = NO_ROTATE, int bold = 0 );
	void PlotStringC( long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotStringUp( long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotStringBold( long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotStringRightJustified(long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotTransStringRightJustified(long fontsize, double x, double y, const char *string, long color, int translate = NO_REM_HTML_TOKENS );
	virtual void PlotNumRightJustified(long fontsize, double x, double y, const char *string, long color );

	// Functions for string alteration & information
	virtual double GetStringLen(const char* str, int size = 1);
	virtual double GetTransStringLen(const char* str, int size = 1);
	virtual std::string SetStringLen( std::string str, double lenWanted, int size = 1, short truncateTextLeftOrRight = 0 );
	virtual std::string SetTransStringLen( std::string str, double lenWanted, int size = 1, short truncateTextLeftOrRight = 0 );
	virtual short TruncateStyle() { return 0; }
	virtual const char* ConvertHTMLTokens( const char *txt );
	const char* Translate( long stringId );
	void FixAsciiSymbols( char *txt );

	// Functions for simple graphics drawing
	virtual void PlotPoint( float x, float y, long pcol );
	virtual void PlotLine( float x, float y, float x2, float y2, long lcol );
	virtual void PlotDrawLine( float x, float y, float x2, float y2, long lcol );
	virtual void PlotDottedLine( float x, float y, float x2, float y2, long dcol );
	virtual void PlotRect( float x, float y, float x2, float y2, long rcol );
	virtual void PlotFilledRect( float x, float y, float x2, float y2, long frcol );
	virtual void PlotImagePolygon(gdPointPtr p, int n, int c, int fill = 0 );
	virtual void PlotImageFilledPolygon( gdPointPtr p, int n, int c );


	// Functions for more complicated graphics drawing
	virtual void PlotXScatterValues( long mins, long i2, long x, long xsize, long hcount, long ypos, long ysize, long grid_color );
	virtual void PlotHorizbar( long xpos, long ypos, long barsize, long ystep, long bar1, long bar2, long bar3, long depth );
	virtual void Plot3dHorizBox( long xpos, long ypos, long barsize, long ystep, long bar1, long bar2, long bar3, long depth );
	virtual void PlotVertbar( double xpos, double ypos, double xs, double ys, long bar1, long bar2, long bar3, long depth );
	virtual void Plot3dHorizGradBox( int x, int y, int xs, int ys, int c1, int c2, int c3, int depth );
	virtual void Plot3dVertGradBox( double x, double y, double xs, double ys, int c1, int c2, int c3, int depth );
	virtual void Plot3DPieGraph(int cx, int cy, int rad, double a1, double a2, int color, int color2 );
	virtual void PlotGraph_Pie( struct graphOptions *graph, long itempos, double current_percent, long num, long pie_hlite[PIE_COLS],long pie_cols[PIE_COLS] );
	void Graph_Draw3DAxis( struct graphOptions *graph, int style, int xsize, int ysize, int backcolor, int gcol );

	// Functions for drawing specific complete graph types
	void Graph_HBar( struct graphOptions *graph, long index, long itempos, __int64 value, __int64 current_value, double ystep, long bar1, long bar2, long bar3 );

	short DrawbyDataStyle( VDinfoP VDptr, StatList *byStat, long id, long typeID, char *filename, char *title, char *name, long sort, long depth, long labelTransId=0 );
	short DrawGraph( VDinfoP	VDptr, struct graphOptions *graph );
	short DrawbyVirtualHostsGraph( VDinfoP VDptr, long tot, short scalerType);
	short DrawRegionGraph( VDinfoP VDptr, struct graphOptions *graph );

	virtual long PlotImageColorGetRGB( int ct );
	virtual void AllocDefaultColors();
	virtual int PlotImageColorNearest( int r, int g, int b);
	virtual int PlotImageColorExact( int r, int g, int b );
	virtual int PlotImageColorAllocate( int r, int g, int b );
	virtual int PlotImageColorAllocateRGB( long rgb );
	//virtual int PlotImageColorSet(gdImagePtr im, int ct, int r, int g, int b);
	virtual void SetGraphColors( struct graphOptions *graph );
	long			pie_cols[PIE_COLS], pie_hlite[PIE_COLS], multi_cols[MULTI_COLS*3];
	
	gdFont *GetFontStruct( long size );
	virtual void FW_SaveImage( VDinfoP VDptr, char *name );
	virtual void MakeBottomShadow( VDinfoP VDptr, long bcolor, long isize );
	virtual void MakeCornerShadow( VDinfoP VDptr, long bcolor, long isize );
	virtual void MakeRightShadow( VDinfoP VDptr, long bcolor, long isize );
	virtual void WriteShadowImages( VDinfoP VDptr );
	FILE *Image_InitFile( VDinfoP VDptr, struct graphOptions *graph, long image_format );
	void Image_CompleteFile( FILE *out );
	void Image_CompleteThumbNail( VDinfoP	VDptr, struct graphOptions *graph, gdImagePtr im );
	void Graph_DrawBase( struct graphOptions *graph, int w, int h, int fill );
	virtual void PlotBevel( int x, int y, int xs, int ys, int c1, int c2, int c3 );
	void Graph_WhiteDrawBase( struct graphOptions *graph, int w, int h, int fill );
	void DefineLabels(struct graphOptions *graph,__int64 value,char **xaxis,char **yaxis,long translateIdOverride=0);
	void Graph_DrawBackground(struct graphOptions *graph, int style, int xsize, int ysize, int color, int backcolor, int gridcolor );
	void Graph_AxisLabels( VDinfoP VDptr, struct graphOptions *graph ,__int64 hvalue,__int64 hcount, short units, int color, int gridcolor );
	void Graph_Legend( struct graphOptions *graph, long multi_cols[MULTI_COLS*3] );
	void Graph_VBar( struct graphOptions *graph, long index, long itempos, __int64 value, __int64 current_value, double xstep, long bar1, long bar2, long bar3 );
	void Graph_Line(
		VDinfoP	VDptr,
		struct graphOptions *graph,
		long index,
		long itempos,
		__int64 value,
		__int64 value2,
		__int64 value3,
		__int64 sumvalue,
		double xstep,
		long num );
	void Graph_MultiLine( VDinfoP	VDptr, struct graphOptions *graph,long index, __int64 value, double xstep, long col );
	void Graph_Multi3DLine( VDinfoP	VDptr, struct graphOptions *graph,long index, long daynum, long maxitems, __int64 value, double xstep, long multi_cols[MULTI_COLS*3] );
	void Graph_SessionTimesGetMax( VDinfoP	VDptr, struct graphOptions *graph, long clientnum, long *maxlen, long *maxdepth );
	long Graph_SessionTimes( VDinfoP	VDptr, struct graphOptions *graph, long clientnum, long maxlen, long maxdepth, long meancolor );
	//void DrawAsciiGraph( VDinfoP VDptr, FILE *fp, StatList *array, long type );
	//long GraphWidth( void );
	//long GraphHeight( void );
	void CreateGraphtypeLine( struct graphOptions *gph );
	void CreateGraphtypeMultiLine( struct graphOptions *gph );
	void CreateGraphtypeMulti3D( struct graphOptions *gph );
	void CreateGraphtypeVBar( struct graphOptions *gph );
	void CreateGraphtype4VBar( struct graphOptions *gph );
	void CreateGraphtypeScatterBar( struct graphOptions *gph );
	void CreateGraphtypeHBar( struct graphOptions *gph );
	void CreateGraphtypePie( struct graphOptions *gph );
	void SetWorldRegionJPGFilename( const char *filename ) { worldRegionJPGFilename = filename; }
	std::string& GetWorldRegionJPGFilename() { return worldRegionJPGFilename; }
private:
	virtual double ScaleXNum( double num ) { return num; }
	char OSNames[4][16];
	std::string worldRegionJPGFilename;
};


struct graphOptions {
	StatList		*byStat;
	VDinfo			*Vhost;
	char 			*title;
	char 			*ftitle;
	char			*filename;
	FILE			*out;
	char			*bartitle;
	void			*dataPtr;
	long			firstTime;
	char			filenameStr[256];
	short		m_thumbnail;		// do a thumb nail 30% size of graph as well, "thumb_filename.GIF"
	short		scaler_type;			// SORT_SIZES or  SORT_REQUESTS
	short		depth;				//3d bar depth
	short		bar_space;			//the width that the bar is to be reduced by!
	long		report_id;			//0=byDate,1=byHour,2=Weekday,3=Domain,4=byFile,5=Browser,6=Opersys,7=FileType
	long		type_id;			//stuff like 'page' 'clie' etc... the column type ID
	short		xlabel_style;		//AXIS LABELS 0=internal,1=external  (HBAR only)
	short		xtitle;
	short		ytitle;
	short		width;
	short		height;
	short		xborder;
	short		yborder;
	long		xunits;
	long		yunits;
	int			xlabel_value;		// put a value per HBAR block
	int			ylabel_value;		// put a value per HBAR block
	int			xlabels;
	int			ylabels;
	int			legend;				// use legend or not
	short		type;
	short		gridx;
	short		gridy;
	short		tickx;
	short		ticky;
	int			lastXsamples;			//only go thru the last X samples of data and not all of it
	int			firstXsamples;			//only go thru the first X samples of data and not all of it
	int			multiLinetotal;

	long		bar_color[3];			//long 0x00rrggbb colors
	long		back_color;			//long 0x00rrggbb color

	long		label_color;			//long 0x00rrggbb color
	long		grid_color;			// grid/axis lines color
	long		base_color;			// the graph outside region color (gray gradient)
	long		borderhi_color;		// border hi light color
	long		borderlo_color;		// border lo light (shadow color)
	long		border_color;		// complete outside border square color

	long		labelTranslateIdOverride;	// optional tranlate id of graph label - used to specify non default label - defaults to 0
} ;	


long GraphWidth( void );
long GraphHeight( void );
long ShadowSize( void );

void DrawAsciiGraph( VDinfoP VDptr, FILE *fp, StatList *array, long type, long depth );
void ValuetoString( long scaler_type, __int64 hvalue, __int64 current_value, char *name );
long ScaleRGB( double perc,long rgb );

#endif
 

