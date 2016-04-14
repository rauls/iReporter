#ifndef __GD_H
#define __GD_H

#ifdef __cplusplus
extern "C" {
#endif

/* stdio is needed for file I/O. */
#include <stdio.h>

/* This can't be changed, it's part of the GIF specification. */

#define gdMaxColors 256

#define gdAlphaMax 127
#define gdAlphaOpaque 0
#define gdAlphaTransparent 127
#define gdRedMax 255
#define gdGreenMax 255
#define gdBlueMax 255
#define gdTrueColorGetAlpha(c) (((c) & 0x7F000000) >> 24)
#define gdTrueColorGetRed(c) (((c) & 0xFF0000) >> 16)
#define gdTrueColorGetGreen(c) (((c) & 0x00FF00) >> 8)
#define gdTrueColorGetBlue(c) ((c) & 0x0000FF)

/* A simpler way to obtain an opaque truecolor value for drawing on a
	truecolor image. Not for use with palette images! */

#define gdTrueColor(r, g, b) (((r) << 16) + \
	((g) << 8) + \
	(b))

/* Returns a truecolor value with an alpha channel component.
	gdAlphaMax (127, **NOT 255**) is transparent, 0 is completely
	opaque. */

#define gdTrueColorAlpha(r, g, b, a) (((a) << 24) + \
	((r) << 16) + \
	((g) << 8) + \
	(b))



typedef struct gdImageRGBColorsStruct {
	int colorsTotal;
	int red[gdMaxColors];
	int green[gdMaxColors];
	int blue[gdMaxColors]; 
	int open[gdMaxColors];
	int	useDefaultPalette;
} gdImageRGBColors;

/* Image type. See functions below; you will not need to change
	the elements directly. Use the provided macros to
	access sx, sy, the color table, and colorsTotal for 
	read-only purposes. */

typedef struct gdImageStruct {
	unsigned char ** pixels;
	int sx;
	int sy;
//	int red[gdMaxColors];
//	int green[gdMaxColors];
//	int blue[gdMaxColors]; 
//	int open[gdMaxColors];
	gdImageRGBColors colors;
	int transparent;
	float *polyInts;
	int polyAllocated;
	struct gdImageStruct *brush;
	struct gdImageStruct *tile;	
	int brushColorMap[gdMaxColors];
	int tileColorMap[gdMaxColors];
	int styleLength;
	int stylePos;
	int *style;
	int interlace;
	int	useRLE;
	int truecolor;
//	int	useDefaultPalette;
} gdImage;

typedef gdImage * gdImagePtr;


typedef struct {
	int	leftx;
	int	width;
} gdCharSizes;

typedef struct {
	/* # of characters in font */
	int nchars;
	/* First character is numbered... (usually 32 = space) */
	int offset;
	/* Character width and height */
	int w;
	int h;
	/* Font data; array of characters, one row after another.
		Easily included in code, also easily loaded from
		data files. */
	char *data;
	/* store proportional font char details here */
	int				propStatus;
	gdCharSizes		propValues[256];
} gdFont;
/* Text functions take these. */
typedef gdFont *gdFontPtr;


/* For backwards compatibility only. Use gdImageSetStyle()
	for MUCH more flexible line drawing. Also see
	gdImageSetBrush(). */
#define gdDashSize 1

/* Special colors. */

#define gdStyled (-2)
#define gdBrushed (-3)
#define gdStyledBrushed (-4)
#define gdTiled (-5)

/* NOT the same as the transparent color index.
	This is used in line styles only. */
#define gdTransparent (-6)





void InitSinCos( void );


long ArcX( long deg, long rad );
long ArcY( long deg, long rad );



void Convert32to24bit( gdImagePtr im, long line, char *rowdata );
void Convert8to24bit( gdImagePtr im, long line, char *rowdata );

int gdImageColorSet(gdImagePtr im, int ct, int r, int g, int b);
int gdImageColorSetRGB(gdImagePtr im, int ct, long rgb );
long gdImageColorGetRGB(gdImagePtr im, int ct );

void gdImageJPG( gdImagePtr im, FILE *outfile, int quality);
void gdImagePNG( gdImagePtr im, FILE *outfile );

/* Functions to manipulate images. */
void gdImageWebColors( gdImageRGBColors* colors, int useDefaultPalette );
gdImagePtr gdImageCreate(int sx, int sy, int x );
gdImagePtr gdImageCreateFromGif(FILE *fd);
gdImagePtr gdImageCreateFromGifMem( unsigned char *gifPtr, long l );
gdImagePtr gdImageCreateFromGd(FILE *in);
gdImagePtr gdImageCreateFromXbm(FILE *fd);
gdImagePtr gdImageCreateFromBmpRam( void *bmp );

void gdImageDestroy(gdImagePtr im);
void gdImageSetPixel(gdImagePtr im, int x, int y, int color);
int gdImageGetPixel(gdImagePtr im, int x, int y);
void gdImageLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
void gdImageQuickLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
/* For backwards compatibility only. Use gdImageSetStyle()
	for much more flexible line drawing. */
void gdImageDashedLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
/* Corners specified (not width and height). Upper left first, lower right
 	second. */
void gdImageRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
/* Solid bar. Upper left corner first, lower right corner second. */
void gdImageFilledRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
int gdImageBoundsSafe(gdImagePtr im, int x, int y);

void gdImageChar(gdImagePtr im, gdFontPtr f, int x, int y, int c, int color);
void gdImageCharUp(gdImagePtr im, gdFontPtr f, int x, int y, unsigned char c, int color);
void gdImageString(gdImagePtr im, gdFontPtr f, int x, int y, char *s, int color);
void gdImageStringUp(gdImagePtr im, gdFontPtr f, int x, int y, char *s, int color);

/* Proportional font support added .. Raul Sobon Nov-1997 */
void gdMakefontProp( gdFontPtr f );
void gdImagePropChar(gdImagePtr im, gdFontPtr f, int x, int y, unsigned char c, int color);
void gdImagePropCharUp(gdImagePtr im, gdFontPtr f, int x, int y, unsigned char c, int color);
void gdImagePropString(gdImagePtr im, gdFontPtr f, int x, int y, const char *s, int color);
void gdImagePropStringUp(gdImagePtr im, gdFontPtr f, int x, int y, const char *s, int color);
int gdStringWidth( gdFontPtr f, const char *s );
int gdSetStringWidth( gdFontPtr f, const char *s, int len, char *buf );

/* Point type for use in polygon drawing. */

typedef struct {
	float x;
	float y;
} gdPoint, *gdPointPtr;

void gdImagePolygon(gdImagePtr im, gdPointPtr p, int n, int c);
void gdImageFilledPolygon(gdImagePtr im, gdPointPtr p, int n, int c);

int gdImageColorAllocate(gdImagePtr im, int r, int g, int b);
int gdImageColorAllocateRGB(gdImagePtr im, long rgb );
int gdImageColorNearest(gdImagePtr im, int r, int g, int b);
int gdImageColorClosest(gdImagePtr im, int r, int g, int b);
int gdImageColorExact(gdImagePtr im, int r, int g, int b);
void gdImageColorDeallocate(gdImagePtr im, int color);
void gdImageColorTransparent(gdImagePtr im, int color);
void gdImageGif(gdImagePtr im, FILE *out, int useRLE );
void gdImageBMP(gdImagePtr im, FILE *out );
void gdImageGd(gdImagePtr im, FILE *out);
void gdImageArc(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int , int color);
void gdImagePie(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color);
void gdImage3DPie(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color, int color2 );
void gdImageFilledPie(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color);
void gdImageFilled3DPie(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color, int color2 );
void gdImageFillToBorder(gdImagePtr im, int x, int y, int border, int color);
void gdImageFill(gdImagePtr im, int x, int y, int color);
void gdImageCopy(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h);
void gdImageQuickSetPixel(gdImagePtr im, int x, int y, int color);
/* Stretches or shrinks to fit, as needed */
void gdImageCopyResized(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int dstW, int dstH, int srcW, int srcH);
void gdImageSetBrush(gdImagePtr im, gdImagePtr brush);
void gdImageSetTile(gdImagePtr im, gdImagePtr tile);
void gdImageSetStyle(gdImagePtr im, int *style, int noOfPixels);
/* On or off (1 or 0) */
void gdImageInterlace(gdImagePtr im, int interlaceArg);

void gdBevel( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3 );
void gdBevelThick( gdImagePtr im, int x1, int y1, int xs, int ys, int c1, int c2, int c3 );
void gdPieGraph(gdImagePtr im, int cx, int cy, int rad, int a1, int a2, int color);
void gd3DPieGraph(gdImagePtr im, int cx, int cy, int rad, int a1, int a2, int color, int color2 );
void gd3dVertBox( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3, int depth );
void gd3dHorizBox( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3, int depth );
void gd3dVertGradBox( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3, int depth );
void gd3dHorizGradBox( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3, int depth );

long Scale_RGB( double perc,long rgb );

/* Macros to access information about images. READ ONLY. Changing
	these values will NOT have the desired result. */
#define gdImageSX(im) ((im)->sx)
#define gdImageSY(im) ((im)->sy)
#define gdImageColorsTotal(im) ((im)->colors.colorsTotal)
#define gdImageRed(im, c) ((im)->colors.red[(c)])
#define gdImageGreen(im, c) ((im)->colors.green[(c)])
#define gdImageBlue(im, c) ((im)->colors.blue[(c)])
#define gdImageGetTransparent(im) ((im)->transparent)
#define gdImageGetInterlaced(im) ((im)->interlace)


gdImagePtr CreateThumbImageFromGD( gdImagePtr sourceImage, long w, long h );

#ifdef __cplusplus
}
#endif

#endif
 
