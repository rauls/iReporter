/*


	http://www.amazing.ch/gfx-formats/   COOL FORMATS SITE
	
	
*/

#include "FWA.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"
#include "myansi.h"
#include "webpalette.h"


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#if (!DEF_UNIX)
#define	USEJPEG
#define USEPNG
#endif

#ifndef RGB_RED
#define RGB_RED(x) 				((x & 0xff0000)>>16)
#endif

#ifndef RGB_GREEN
#define	RGB_GREEN(x) 			((x & 0x00ff00)>>8)
#endif

#ifndef RGB_BLUE
#define RGB_BLUE(x)				( x & 0x0000ff)
#endif

#define	PI				3.1415926535897932384626433832795
#define	RADTODEG(x)		(x*180/PI)
#define	DEGTORAD(x)		(x*PI/180)
#define	SINETABLESIZE	360
#define COSTSCALE		100000
#define SINTSCALE		COSTSCALE



//#include "mtables.c"
long	cost[SINETABLESIZE];
long	sint[SINETABLESIZE];
void InitSinCos( void )
{
	static int	tabledone=0;
	long		i;

	if ( !tabledone ){
		for(i=0;i<SINETABLESIZE;i++){
			sint[i] = (long)(sin( DEGTORAD(i) ) * COSTSCALE);
			cost[i] = (long)(cos( DEGTORAD(i) ) * COSTSCALE);
		}
		tabledone = 1;
	}
}

#define	RANGE(x)	(x % SINETABLESIZE)
#define	SIN(x)		(sint[ RANGE(x) ])
#define	COS(x)		(cost[ RANGE(x) ])

#define	ARCX(deg,rad)		(COS(deg) * rad / COSTSCALE)
#define	ARCY(deg,rad)		(SIN(deg) * rad / SINTSCALE)

//	r1 = w/2;  r2 = h/2;
//	x = ((long)cost[s % 360] * (long)r1 / COSTSCALE) + cx; 
//	y = ((long)sint[s % 360] * (long)r2 / SINTSCALE) + cy;


long ArcX( long deg, long rad )
{
	deg = deg%360;
	rad = rad/2;
	return ((long)cost[deg % 360] * (long)rad / COSTSCALE);
}
long ArcY( long deg, long rad )
{
	deg = deg%360;
	rad = rad/2;
	return ((long)sint[deg % 360] * (long)rad / SINTSCALE);
}

void gdImageQuickSetPixel(gdImagePtr im, int x, int y, int color);
static void gdImageBrushApply(gdImagePtr im, int x, int y);
static void gdImageTileApply(gdImagePtr im, int x, int y);
int gdGetWord(int *result, FILE *in);
void gdPutWord(int w, FILE *out);
int gdGetByte(int *result, FILE *in);
void Line2D (gdImagePtr im, int x0, int y0, int x1, int y1, int color);

#define UNDEFINED -1

/*

void RGB_to_HWB( short *rgb, short *hwb ) {
 // RGB are each on [0, 1]. W and B are returned on [0, 1] and H is  
 // returned on [0, 6]. Exception: H is returned UNDEFINED if W == 1 - B.  
 float R = rgb[0], G = rgb[1], B = rgb[2], w, v, b, f;  
 int i;

 w = min(R, G, B);  
 v = max(R, G, B);  
 b = 1 - v;  

 if (v == w) {
	 hwb[0]=0; hwb[1]=w; hwb[2]=b;
 } else {
	 f = (R == w) ? G - B : ((G == w) ? B - R : R - G);  
	 i = (R == w) ? 3 : ((G == w) ? 5 : 1);  
 	 hwb[0]=(i - f)/(v - w); hwb[1]=w; hwb[2]=b;
 }
}

*/

/*void HWB_to_RGB( short *hwb, short *rgb ) {
 // H is given on [0, 6] or UNDEFINED. W and B are given on [0, 1].  
 // RGB are each returned on [0, 1].  
 float h = hwb[0], w = hwb[1], b = hwb[2], v, n, f;  
 int i;  

 v = 1 - b;  
 if (h == UNDEFINED) {
	 rgb[0] = v; rgb[1] = v; rgb[2] = v;
	 return;
 }

 i = floor(h);  
 f = h - i;  
 if (i & 1) f = 1 - f; // if i is odd  
 n = w + f * (v - w); // linear interpolation between w and v  

 switch (i) {  
  case 6:  
  case 0: rgb[0] = v; rgb[1] = n; rgb[2] = w; break;
  case 1: rgb[0] = n; rgb[1] = v; rgb[2] = w; break;
  case 2: rgb[0] = w; rgb[1] = v; rgb[2] = n; break;
  case 3: rgb[0] = w; rgb[1] = n; rgb[2] = v; break;
  case 4: rgb[0] = n; rgb[1] = w; rgb[2] = v; break;
  case 5: rgb[0] = v; rgb[1] = w; rgb[2] = n; break;
 }  
}*/




long Scale_RGB( double perc,long rgb )
{
	long r,g,b;

	r = (rgb & 0xff0000) >> 16;
	g = (rgb & 0xff00) >> 8;
	b = (rgb & 0xff);

	r = (long)(r * perc / 100.0);
	if ( r < 0 ) r = 0;
	if ( r > 255 ) r = 255;

	g = (long)(g * perc / 100.0);
	if ( g < 0 ) g = 0;
	if ( g > 255 ) g = 255;

	b = (long)(b * perc / 100.0);
	if ( b < 0 ) b = 0;
	if ( b > 255 ) b = 255;

	return ( r<<16 | g<<8 | b );
}


void gdImageWebColors( gdImageRGBColors* colors, int useDefaultPalette )
{
	int i;

	if( colors->useDefaultPalette=useDefaultPalette )
	{
		for( i=0; i<gdMaxColors; i++)
		{
			colors->red[i]=   webpalette[ (i*3)+0 ];
			colors->green[i]= webpalette[ (i*3)+1 ];
			colors->blue[i]=  webpalette[ (i*3)+2 ];
			colors->open[i]=  0;
		}

		colors->colorsTotal=gdMaxColors;
	}
	else
	{
		// initialise colours to avoid bounds checker unitialised memory warnings in gdImageGif()
		for( i=0; i<gdMaxColors; i++)
		{
			colors->red[i]=   0;
			colors->green[i]= 0;
			colors->blue[i]=  0;
			colors->open[i]=  0;
		}

		colors->colorsTotal=0;
	}
}


gdImagePtr gdImageCreate(int sx, int sy, int useDefaultPalette )
{
	int i;
	gdImagePtr im;

	InitSinCos();

	im = (gdImage *) malloc(sizeof(gdImage));
	if ( im ){
		im->pixels = (unsigned char **) malloc(sizeof(unsigned char *) * (sy+32));
		if ( !im->pixels ) return 0;
		im->polyInts = 0;
		im->polyAllocated = 0;
		im->brush = 0;
		im->tile = 0;
		im->style = 0;
		
		for (i=0; (i<sy); i++) {
			//OutDebugs( "debug %d, i=%d\n", __LINE__, i );
			im->pixels[i] = (unsigned char *) malloc( sx * sizeof(unsigned char) );
		}

		im->sx = sx;
		im->sy = sy;
		im->transparent = (-1);
		im->interlace = 0;
		im->truecolor = 0;

		gdImageWebColors( &im->colors, useDefaultPalette );
	}

	return im;
}


gdImagePtr gdImageCreate24bit(int sx, int sy )
{
	int i;
	gdImagePtr im;

	sx &= 0x7ff;
	sy &= 0x7ff;

	InitSinCos();

	im = (gdImage *) malloc(sizeof(gdImage));
	if ( im ){
		im->pixels = (unsigned char **) malloc(sizeof(unsigned char *) * (sy+32));
		if ( !im->pixels ) return 0;
		im->polyInts = 0;
		im->polyAllocated = 0;
		im->brush = 0;
		im->tile = 0;
		im->style = 0;
		
		for (i=0; (i<sy); i++) {
			im->pixels[i] = (unsigned char *) malloc( (sx+32) * sizeof(unsigned char) * 4 );
			memset( im->pixels[i], 0xffffffff, sx * sizeof(unsigned char) * 4 );
		}

		im->sx = sx;
		im->sy = sy;
		im->transparent = 0;
		im->interlace = 0;
		im->truecolor = 1;
		gdImageWebColors( &im->colors, 0 );
	}

	return im;
}


void gdImageDestroy(gdImagePtr im)
{
	int i;
	if ( im ){
		for (i=0; (i<im->sy); i++) {
			if ( im->pixels[i] )
				free(im->pixels[i]);
		}	
		free(im->pixels);

		if (im->polyInts) {
			free(im->polyInts);
		}

		if (im->style) {
			free(im->style);
		}
		free(im);
	}
}

int gdImageColorClosest(gdImagePtr im, int r, int g, int b)
{
	int i;
	long rd, gd, bd;
	int ct = (-1);
	long mindist = 0;
	for (i=0; (i<(im->colors.colorsTotal)); i++) {
		long dist;
		if (im->colors.open[i]) {
			continue;
		}
		rd = (im->colors.red[i] - r);	
		gd = (im->colors.green[i] - g);
		bd = (im->colors.blue[i] - b);
		dist = rd * rd + gd * gd + bd * bd;
		if ((i == 0) || (dist < mindist)) {
			mindist = dist;	
			ct = i;
		}
	}
	return ct;
}

/* Do closest-match-mapping. I use a || x ||_1 norm here. This is faster
 * and the measure for "closest" istn't well defined for RGB anyway.
 */

#define ABS(a,b) ( (a>b) ? a-b : b-a )

int gdImageColorNearest(gdImagePtr im, int r, int g, int b)
{
	int closest=0;
	int i;

	unsigned short closedist=0xffff,dist,d;

	for (i=0; (i<(im->colors.colorsTotal)); i++) {
		if (im->colors.open[i]) {
			continue;
		}
		dist=ABS(r,im->colors.red[i]);
		if (dist >= closedist) continue;
		d   =ABS(g,im->colors.green[i]);
		if (d>dist) dist=d;
		if (dist >= closedist) continue;
		d   =ABS(b,im->colors.blue[i]);
		if (d>dist) dist=d;
		if (dist >= closedist) continue;
		closest=i;
		if (dist==0) break;
		closedist=dist;
	}
	return closest;
}

int gdImageColorExact(gdImagePtr im, int r, int g, int b)
{
	int i;
	for (i=0; (i<(im->colors.colorsTotal)); i++) {
		if (im->colors.open[i]) {
			continue;
		}
		if ((im->colors.red[i] == r) && 
			(im->colors.green[i] == g) &&
			(im->colors.blue[i] == b)) {
			return i;
		}
	}
	return -1;
}

int gdImageColorAllocate(gdImagePtr im, int r, int g, int b)
{
	int i;
	int ct = (-1);

	if ( im->colors.useDefaultPalette ){
		return gdImageColorNearest( im, r, g, b );
		//return gdImageColorClosest( im, r, g, b );
	}	

	i = gdImageColorExact(im,r,g,b);
	if ( i>=0 )
		return i;
	
	for (i=0; (i<(im->colors.colorsTotal)); i++) {
		if (im->colors.open[i]) {
			ct = i;
			break;
		}
	}	
	if (ct == (-1)) {
		ct = im->colors.colorsTotal;
		if (ct == gdMaxColors) {
			return gdImageColorNearest( im, r, g, b );
		}
		im->colors.colorsTotal++;
	}
	im->colors.red[ct] = r;
	im->colors.green[ct] = g;
	im->colors.blue[ct] = b;
	im->colors.open[ct] = 0;
	return ct;
}

int gdImageColorAllocateRGB(gdImagePtr im, long rgb )
{
	long r,g,b;
	r = (rgb & 0xff0000) >> 16;
	g = (rgb & 0xff00) >> 8;
	b = (rgb & 0xff);

	return gdImageColorAllocate(im, r, g, b);
}
int gdImageColorSet(gdImagePtr im, int ct, int r, int g, int b)
{
	im->colors.red[ct] = r;
	im->colors.green[ct] = g;
	im->colors.blue[ct] = b;
	im->colors.open[ct] = 0;

	return ct;
}
int gdImageColorSetRGB(gdImagePtr im, int ct, long rgb )
{
	long r,g,b;

	r = (rgb & 0xff0000) >> 16;
	g = (rgb & 0xff00) >> 8;
	b = (rgb & 0xff);

	return gdImageColorSet(im, ct, r, g, b);
}
long gdImageColorGetRGB(gdImagePtr im, int ct )
{
	long r,g,b, rgb;

	/* Range check */
	if ( ct < 0 || ct > gdMaxColors )
		ct = 0;

	r = im->colors.red[ct]<< 16;
	g = im->colors.green[ct]<< 8;
	b = im->colors.blue[ct];
	rgb = r | g | b;

	return rgb;
}
void gdImageColorDeallocate(gdImagePtr im, int color)
{
	/* Mark it open. */
	im->colors.open[color] = 1;
}

void gdImageColorTransparent(gdImagePtr im, int color)
{
	im->transparent = color;
}

void gdImageQuickSetPixel(gdImagePtr im, int x, int y, int color)
{
	if (!im)
		return;

	if (gdImageBoundsSafe(im, x, y))
	{
		if ( im->truecolor )
		{
			unsigned char *rgbPtr = &im->pixels[y][x*4];
			long *pixel;
			pixel = (long*)rgbPtr;
			*pixel = color;
		} else
		 im->pixels[y][x] = color;
	}
}

void gdImageSetPixel(gdImagePtr im, int x, int y, int color)
{
	int p;
	switch(color) {
		case gdStyled:
			if (!im->style) {
				/* Refuse to draw if no style is set. */
				return;
			} else {
				p = im->style[im->stylePos++];
			}
			if (p != (gdTransparent)) {
				gdImageSetPixel(im, x, y, p);
			}
			im->stylePos = im->stylePos %  im->styleLength;
			break;
		case gdStyledBrushed:
			if (!im->style) {
				/* Refuse to draw if no style is set. */
				return;
			}
			p = im->style[im->stylePos++];
			if ((p != gdTransparent) && (p != 0)) {
				gdImageSetPixel(im, x, y, gdBrushed);
			}
			im->stylePos = im->stylePos %  im->styleLength;
			break;
		case gdBrushed:
			gdImageBrushApply(im, x, y);
			break;
		case gdTiled:
			gdImageTileApply(im, x, y);
			break;
		default:
			gdImageQuickSetPixel( im, x, y, color );
			break;
	}
}



static void gdImageBrushApply(gdImagePtr im, int x, int y)
{
	int lx, ly;
	int hy;
	int hx;
	int x1, y1, x2, y2;
	int srcx, srcy;
	if (!im->brush) {
		return;
	}
	hy = gdImageSY(im->brush)/2;
	y1 = y - hy;
	y2 = y1 + gdImageSY(im->brush);	
	hx = gdImageSX(im->brush)/2;
	x1 = x - hx;
	x2 = x1 + gdImageSX(im->brush);
	srcy = 0;
	for (ly = y1; (ly < y2); ly++) {
		srcx = 0;
		for (lx = x1; (lx < x2); lx++) {
			int p;
			p = gdImageGetPixel(im->brush, srcx, srcy);
			/* Allow for non-square brushes! */
			if (p != gdImageGetTransparent(im->brush)) {
				gdImageSetPixel(im, lx, ly,
					im->brushColorMap[p]);
			}
			srcx++;
		}
		srcy++;
	}	
}		

static void gdImageTileApply(gdImagePtr im, int x, int y)
{
	int srcx, srcy;
	int p;
	if (!im->tile) {
		return;
	}
	srcx = x % gdImageSX(im->tile);
	srcy = y % gdImageSY(im->tile);
	p = gdImageGetPixel(im->tile, srcx, srcy);
	/* Allow for transparency */
	if (p != gdImageGetTransparent(im->tile)) {
		gdImageSetPixel(im, x, y,
			im->tileColorMap[p]);
	}
}		

int gdImageGetPixel(gdImagePtr im, int x, int y)
{
	if (gdImageBoundsSafe(im, x, y)) 
	{
		if ( im->truecolor )
		{
			unsigned char *rgbPtr = &im->pixels[y][x*4];
			long *pixel;
			pixel = (long*)rgbPtr;
			return *pixel;
		} else
			return im->pixels[y][x];
	} else {
		return 0;
	}
}


int gdImageGetPixelRGB(gdImagePtr im, int x, int y)
{
	if ( gdImageBoundsSafe(im, x, y)) 
	{
		long i = gdImageGetPixel( im, x,y );
		if ( i>255 )
			return i;
		else
			return gdImageColorGetRGB( im, i );
	} else {
		return -1;
	}
}



/* Bresenham as presented in Foley & Van Dam */

void gdImageLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	Line2D (im, x1, y1, x2, y2, color);
}

// new Bresenham variant
// D. Williams
void Line2D (gdImagePtr im, int x0, int y0, int x1, int y1, int color)
{
    int ax, ay, max, var;
    int decx, decy;
    // starting point of line
    int x = x0, y = y0;

    // direction of line
    int dx = x1-x0, dy = y1-y0;

    // increment or decrement depending on direction of line
    int sx = (dx > 0 ? 1 : (dx < 0 ? -1 : 0));
    int sy = (dy > 0 ? 1 : (dy < 0 ? -1 : 0));

    // decision parameters for voxel selection
    if ( dx < 0 ) dx = -dx;
    if ( dy < 0 ) dy = -dy;
    ax = 2*dx;
    ay = 2*dy;

    // determine largest direction component, single-step related variable
    max = dx;
    var = 0;
    if ( dy > max ) { var = 1; }

    // traverse Bresenham line
    switch ( var )
    {
    case 0:  // single-step in x-direction
        for (decy=ay-dx; /**/; x += sx, decy += ay)
        {
            // process pixel
            gdImageQuickSetPixel(im, x, y, color);

            // take Bresenham step
            if ( x == x1 ) break;
            if ( decy >= 0 ) { decy -= ax; y += sy; }
        }
        break;
    case 1:  // single-step in y-direction
        for (decx=ax-dy; /**/; y += sy, decx += ax)
        {
            // process pixel
            gdImageQuickSetPixel(im, x, y, color);

            // take Bresenham step
            if ( y == y1 ) break;
            if ( decx >= 0 ) { decx -= ay; x += sx; }
        }
        break;
    }
}
void Circle2D (gdImagePtr im, int xc, int yc, int r, int color)
{
    int x,y,dec;
    
    for (x = 0, y = r, dec = 3-2*r; x <= y; x++)
    {
        gdImageQuickSetPixel(im, xc+x,yc+y, color);
        gdImageQuickSetPixel(im, xc+x,yc-y, color);
        gdImageQuickSetPixel(im, xc-x,yc+y, color);
        gdImageQuickSetPixel(im, xc-x,yc-y, color);
        gdImageQuickSetPixel(im, xc+y,yc+x, color);
        gdImageQuickSetPixel(im, xc+y,yc-x, color);
        gdImageQuickSetPixel(im, xc-y,yc+x, color);
        gdImageQuickSetPixel(im, xc-y,yc-x, color);

        if ( dec >= 0 )
            dec += -4*(y--)+4;
        dec += 4*x+6;
    }
}
// Draw an Ellipse
// Center xc,yc
// 
void Ellipse2D (gdImagePtr im, int xc, int yc, int a, int b, int s, int e, int color)
{
    int x, y, dec;
    int a2 = a*a;
    int b2 = b*b;
    
    for (x = 0, y = b, dec = 2*b2+a2*(1-2*b); b2*x <= a2*y; x++)
    {
        gdImageQuickSetPixel(im, xc+x,yc+y, color);
        gdImageQuickSetPixel(im, xc-x,yc+y, color);
        gdImageQuickSetPixel(im, xc+x,yc-y, color);
        gdImageQuickSetPixel(im, xc-x,yc-y, color);

        if ( dec >= 0 )
            dec += 4*a2*(1-(y--));
        dec += b2*(4*x+6);
    }

    for (x = a, y = 0, dec = 2*a2+b2*(1-2*a); a2*y <= b2*x; y++)
    {
        gdImageQuickSetPixel(im, xc+x,yc+y, color);
        gdImageQuickSetPixel(im, xc-x,yc+y, color);
        gdImageQuickSetPixel(im, xc+x,yc-y, color);
        gdImageQuickSetPixel(im, xc-x,yc-y, color);

        if ( dec >= 0 )
            dec += 4*b2*(1-(x--));
        dec += a2*(4*y+6);
    }
}
static void SelectEllipsePoint (int a2, int b2, float x, float y, int *ix,int *iy)
{
    int xfloor = (int)floor(x);
    int yfloor = (int)floor(y);
    int xincr = b2*(2*xfloor+1);
    int yincr = a2*(2*yfloor+1);
    int base = b2*xfloor*xfloor+a2*yfloor*yfloor-a2*b2;
    int a00 = abs(base);
    int a10 = abs(base+xincr);
    int a01 = abs(base+yincr);
    int a11 = abs(base+xincr+yincr);

    int min = a00;
    *ix = xfloor;
    *iy = yfloor;
    if ( a10 < min )
    {
        min = a10;
        *ix = xfloor+1;
        *iy = yfloor;
    }
    if ( a01 < min )
    {
        min = a01;
        *ix = xfloor;
        *iy = yfloor+1;
    }
    if ( a11 < min )
    {
        min = a11;
        *ix = xfloor+1;
        *iy = yfloor+1;
    }
}
//---------------------------------------------------------------------------
static int WhichArc (int a2, int b2, int x, int y)
{
    if ( x > 0 )
    {
        if ( y > 0 )
            return ( b2*x <  a2*y ? 0 : 1 );
        else if ( y < 0 )
            return ( b2*x > -a2*y ? 2 : 3 );
        else
            return 2;
    }
    else if ( x < 0 )
    {
        if ( y < 0 )
            return ( a2*y <  b2*x ? 4 : 5 );
        else if ( y > 0 )
            return ( a2*y < -b2*x ? 6 : 7 );
        else
            return 6;
    }
    else
    {
        return ( y > 0 ? 0 : 4 );
    }
}

void EllipseArc2D (gdImagePtr im, int xc, int yc, int a, int b, float fx0, float fy0, float fx1, float fy1, int color)
{
    // Assert (within floating point roundoff errors):
    //   (fx0-xc)^2/a^2 + (fy0-yc)^2/b^2 = 1
    //   (fx1-xc)^2/a^2 + (fy1-yc)^2/b^2 = 1
    // Assume if (fx0,fy0) == (fx1,fy1), then entire ellipse should be drawn.
    //
    // Integer points on arc are guaranteed to be traversed clockwise.
    int dx,dy,sqrlen,arc,sigma;
    const int a2 = a*a, b2 = b*b;

    // get integer end points for arc
    int x0, y0, x1, y1;
    SelectEllipsePoint(a2,b2,fx0-xc,fy0-yc,&x0,&y0);
    SelectEllipsePoint(a2,b2,fx1-xc,fy1-yc,&x1,&y1);

    dx = x0 - x1;
    dy = y0 - y1;
    sqrlen = dx*dx+dy*dy;
    if ( sqrlen == 1 || ( sqrlen == 2 && abs(dx) == 1 ) )
    {
        gdImageQuickSetPixel(im, xc+x0,yc+y0,color);
        gdImageQuickSetPixel(im, xc+x1,yc+y1,color);
        return;
    }

    // determine initial case for arc drawing
    arc = WhichArc(a2,b2,x0,y0);
    while ( 1 )
    {
        // process the pixel
        gdImageQuickSetPixel(im, xc+x0,yc+y0,color);

        // Determine next pixel to process.  Notation <(x,y),dy/dx>
        // indicates point on ellipse and slope at that point.
        switch ( arc )
        {
        case 0:  // <(0,b),0> to <(u0,v0),-1>
            x0++;
            dx++;
            sigma = b2*x0*x0+a2*(y0-1)*(y0-1)-a2*b2;
            if ( sigma >= 0 )
            {
                y0--;
                dy--;
            }
            if ( b2*x0 >= a2*y0 )
                arc = 1;
            break;
        case 1:  // <(u0,v0),-1> to <(a,0),infinity>
            y0--;
            dy--;
            sigma = b2*x0*x0+a2*y0*y0-a2*b2;
            if ( sigma < 0 )
            {
                x0++;
                dx++;
            }
            if ( y0 == 0 )
                arc = 2;
            break;
        case 2:  // <(a,0),infinity> to <(u1,v1),+1>
            y0--;
            dy--;
            sigma = b2*(x0-1)*(x0-1)+a2*y0*y0-a2*b2;
            if ( sigma >= 0 )
            {
                x0--;
                dx--;
            }
            if ( b2*x0 <= -a2*y0 )
                arc = 3;
            break;
        case 3:  // <(u1,v1),+1> to <(0,-b),0>
            x0--;
            dx--;
            sigma = b2*x0*x0+a2*y0*y0-a2*b2;
            if ( sigma < 0 )
            {
                y0--;
                dy--;
            }
            if ( x0 == 0 )
                arc = 4;
            break;
        case 4:  // <(0,-b),0> to <(u2,v2,-1)>
            x0--;
            dx--;
            sigma = b2*x0*x0+a2*(y0+1)*(y0+1)-a2*b2;
            if ( sigma >= 0 )
            {
                y0++;
                dy++;
            }
            if ( a2*y0 >= b2*x0 )
                arc = 5;
            break;
        case 5:  // <(u2,v2,-1)> to <(-a,0),infinity>
            y0++;
            dy++;
            sigma = b2*x0*x0+a2*y0*y0-a2*b2;
            if ( sigma < 0 )
            {
                x0--;
                dx--;
            }
            if ( y0 == 0 )
                arc = 6;
            break;
        case 6:  // <(-a,0),infinity> to <(u3,v3),+1>
            y0++;
            dy++;
            sigma = b2*(x0+1)*(x0+1)+a2*y0*y0-a2*b2;
            if ( sigma >= 0 )
            {
                x0++;
                dx++;
            }
            if ( a2*y0 >= -b2*x0 )
                arc = 7;
            break;
        case 7:  // <(u3,v3),+1> to <(0,b),0>
            x0++;
            dx++;
            sigma = b2*x0*x0+a2*y0*y0-a2*b2;
            if ( sigma < 0 )
            {
                y0++;
                dy++;
            }
            if ( x0 == 0 )
                arc = 0;
            break;
        }

        sqrlen = dx*dx+dy*dy;
        if ( sqrlen <= 1 )
            break;
    }
}
void gdImageQuickLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	Line2D (im, x1, y1, x2, y2, color);
	/*
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;

	dx = abs(x2-x1);
	dy = abs(y2-y1);
	if (dy <= dx) {
		d = 2*dy - dx;
		incr1 = 2*dy;
		incr2 = 2 * (dy - dx);
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}
		gdImageQuickSetPixel(im, x, y, color);

		if (((y2 - y1) * ydirflag) > 0) {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y++;
					d+=incr2;
				}
				gdImageQuickSetPixel(im, x, y, color);
			}

		} else {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y--;
					d+=incr2;
				}
				gdImageQuickSetPixel(im, x, y, color);
			}
		}	
	} else {
		d = 2*dx - dy;
		incr1 = 2*dx;
		incr2 = 2 * (dx - dy);
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}
		gdImageQuickSetPixel(im, x, y, color);

		if (((x2 - x1) * xdirflag) > 0) {
			while (y < yend) {
				y++;

				if (d <0) {
					d+=incr1;
				} else {
					x++;
					d+=incr2;
				}
				gdImageQuickSetPixel(im, x, y, color);
			}
		} else {
			while (y < yend) {
				y++;

				if (d <0) {
					d+=incr1;
				} else {
					x--;
					d+=incr2;
				}
				gdImageQuickSetPixel(im, x, y, color);
			}
		}
	}*/
}
/* As above, plus dashing */

#define dashedSet 	dashStep++; if (dashStep == gdDashSize) { dashStep = 0; on = !on; } if (on) { gdImageSetPixel(im, x, y, color); } 


void gdImageDashedLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;
	int dashStep = 0;
	int on = 1;
	dx = abs(x2-x1);
	dy = abs(y2-y1);
	if (dy <= dx) {
		d = 2*dy - dx;
		incr1 = 2*dy;
		incr2 = 2 * (dy - dx);
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}
		dashedSet;
		if (((y2 - y1) * ydirflag) > 0) {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y++;
					d+=incr2;
				}
				dashedSet;
			}
		} else {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y--;
					d+=incr2;
				}
				dashedSet;
			}
		}		
	} else {
		d = 2*dx - dy;
		incr1 = 2*dx;
		incr2 = 2 * (dx - dy);
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}
		dashedSet;
		if (((x2 - x1) * xdirflag) > 0) {
			while (y < yend) {
				y++;
				if (d <0) {
					d+=incr1;
				} else {
					x++;
					d+=incr2;
				}
				dashedSet;
			}
		} else {
			while (y < yend) {
				y++;
				if (d <0) {
					d+=incr1;
				} else {
					x--;
					d+=incr2;
				}
				dashedSet;
			}
		}
	}
}

int gdImageBoundsSafe(gdImagePtr im, int x, int y)
{
	if (!im)
		return 0;

	return (!(((y < 0) || (y >= im->sy)) ||
		((x < 0) || (x >= im->sx))));
}

#define CHECKXYBOUNDS( x,y ) ( x>0 && y>0 && x<im->sx && y<im->sy )


void gdMakefontProp( gdFontPtr f )
{

	int cx, cy;
	int px, py;
	int fline;
	int highestx,smallestx,firstpixel,width;
	int c;
	cx = cy = 0;

	/* for each character */
	for ( c=f->offset ; c<(f->offset + f->nchars); c++ ){
		fline = (c - f->offset) * f->h * f->w;
		cx = cy = 0;
		highestx = 0;
		smallestx = f->w;
		/* for each line */
		for (py = 0; (py < (f->h)); py++) {
			firstpixel = -1;
			/* for each pixel */
			for (px = 0; (px < (f->w)); px++) {
				if (f->data[fline + (cy * f->w) + cx]) {
					if ( firstpixel < 0 )
						firstpixel = px;
					if ( px > highestx )
						highestx = px;
				}
				cx++;
			}
			if ( firstpixel >= 0 )
				if ( firstpixel < smallestx )
					  smallestx = firstpixel;
			cx = 0;
			cy++;
		}
		if ( c == 32 ){
			smallestx = 0;
			highestx = f->w/5;
		}
		width = (highestx - smallestx)+1;
		if ( width < 0 ){
			width = 0;
		}
		/* store smallestx,width for proportional details */
		f->propValues[c].leftx = smallestx;
		f->propValues[c].width = width;
	}
	f->propStatus = 1;
}


/* proportional draw char */
void gdImagePropChar(gdImagePtr im, gdFontPtr f, int x, int y, unsigned char c, int color)
{
	int cx, cy;
	int px, py;
	int fline, yoffset;

	cx = 0;
	cy = 0;
	if ((c < f->offset) || (c >= (f->offset + f->nchars)))
		return;

	fline = (c - f->offset) * f->h * f->w;
	for (py = y; (py < (y + f->h)); py++) {
		cx = f->propValues[c].leftx;
		yoffset = fline + (cy * f->w);
		for (px = x; (px < (x + f->propValues[c].width)); px++) {
			if (f->data[yoffset + cx]) {
				gdImageQuickSetPixel(im, px, py, color);	
			}
			cx++;
		}
		cy++;
	}
}

void gdImageChar(gdImagePtr im, gdFontPtr f, int x, int y, int c, int color)
{
	int cx, cy;
	int px, py;
	int fline, yoffset;

	cx = 0;
	cy = 0;
	if ((c < f->offset) || (c >= (f->offset + f->nchars)))
		return;

	fline = (c - f->offset) * f->h * f->w;
	for (py = y; (py < (y + f->h)); py++) {
		yoffset = fline + (cy * f->w);
		for (px = x; (px < (x + f->w)); px++) {
			if (f->data[yoffset + cx]) {
				gdImageQuickSetPixel(im, px, py, color);	
			}
			cx++;
		}
		cx = 0;
		cy++;
	}
}

void gdImagePropCharUp(gdImagePtr im, gdFontPtr f, int x, int y, unsigned char c, int color)
{
	int cx, cy;
	int px, py;
	int fline;
	cx = 0;
	cy = 0;
	if ((c < f->offset) || (c >= (f->offset + f->nchars))) {
		return;
	}
	fline = (c - f->offset) * f->h * f->w;
	cx = f->propValues[c].leftx;
	for (py = y; (py > (y - f->propValues[c].width)); py--) {
		for (px = x; (px < (x + f->h)); px++) {
			if (f->data[fline + (cy * f->w) + cx]) {
				gdImageQuickSetPixel(im, px, py, color);	
			}
			cy++;
		}
		cy = 0;
		cx++;
	}
}

void gdImageCharUp(gdImagePtr im, gdFontPtr f, int x, int y, unsigned char c, int color)
{
	int cx, cy;
	int px, py;
	int fline;
	cx = 0;
	cy = 0;
	if ((c < f->offset) || (c >= (f->offset + f->nchars))) {
		return;
	}
	fline = (c - f->offset) * f->h * f->w;
	for (py = y; (py > (y - f->w)); py--) {
		for (px = x; (px < (x + f->h)); px++) {
			if (f->data[fline + (cy * f->w) + cx]) {
				gdImageQuickSetPixel(im, px, py, color);	
			}
			cy++;
		}
		cy = 0;
		cx++;
	}
}

void gdImagePropString(gdImagePtr im, gdFontPtr f, int x, int y, const char *s, int color)

{
	int i;
	int l;
	unsigned char c;

	l = mystrlen ((char *)s);
	if ( l ){
		if ( f->propStatus == 0 )
			gdMakefontProp( f );
		for (i=0; (i<l); i++) {
			c = s[i];
			gdImagePropChar(im, f, x, y, c, color);
			x += (f->propValues[c].width + 1);
		}
	}
}

int gdStringWidth( gdFontPtr f, const char *s )
{
	int i, x=0;
	int l;
	unsigned char c;

	l = mystrlen ((char *)s);
	if ( l ){
		if ( f->propStatus == 0 )
			gdMakefontProp( f );
		for (i=0; (i<l); i++) {
			c = s[i];
			x += (f->propValues[c].width + 1);
		}
	}
	return x;
}

int gdSetStringWidth( gdFontPtr f, const char *s, int len, char *buf )
{
	int i, x=0;
	int l;
	unsigned char c;
	char *ptr = buf;

	l = mystrlen ((char *)s);
	if ( l ){
		if ( f->propStatus == 0 )
			gdMakefontProp( f );
		for (i=0; (i<l); i++)
		{
			// Get the current character
			c = s[i];

			// Check it's length and see if we have room left to add it...
			if( x + f->propValues[c].width <= len )
				x += (f->propValues[c].width + 1);
			else
				break;

			// Copy the string across...
			*ptr = c; 
			ptr++;
		}
		*ptr = 0;
	}
	return x;
}

void gdImageString (gdImagePtr im, gdFontPtr f, int x, int y, char *s, int color)
{
	int i;
	int l;
	l = mystrlen(s);
	for (i=0; (i<l); i++) {
		gdImageChar(im, f, x, y, s[i], color);
		x += f->w;
	}
}

void gdImagePropStringUp(gdImagePtr im, gdFontPtr f, int x, int y, const char *s, int color)
{
	int i;
	int l;
	unsigned char c;

	l = mystrlen ((char *)s);
	if ( l ){
		if ( f->propStatus == 0 )
			gdMakefontProp( f );
		for (i=0; (i<l); i++) {
			c = s[i];
			gdImageCharUp(im, f, x, y, c, color);
			y -= (f->propValues[c].width +1);
		}
	}
}

void gdImageStringUp(gdImagePtr im, gdFontPtr f, int x, int y, char *s, int color)
{
	int i;
	int l;
	l = mystrlen(s);
	for (i=0; (i<l); i++) {
		gdImageCharUp(im, f, x, y, s[i], color);
		y -= f->w;
	}
}

/* s and e are integers modulo 360 (degrees), with 0 degrees
  being the rightmost extreme and degrees changing clockwise.
  cx and cy are the center in pixels; w and h are the horizontal 
  and vertical diameter in pixels. Nice interface, but slow, since
  I don't yet use Bresenham (I'm using an inefficient but
  simple solution with too much work going on in it; generalizing
  Bresenham to ellipses and partial arcs of ellipses is non-trivial,
  at least for me) and there are other inefficiencies (small circles
  do far too much work). */

void gdImageArc(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int thik, int color)
{
	//Ellipse2D(im,cx,cy,w,h,s,e,color);
	//EllipseArc2D (im, 100, 100, 100, 100, 0, 0, 0, 0, color);
	
	int i;
	int lx = 0, ly = 0;
	int r1, r2;
	r1 = w/2;
	r2 = h/2;
	while (e < s) {
		e += 360;
	}
	for (i=s; (i <= e); i++) {
		int x, y;
		x = ((long)cost[i % 360] * (long)r1 / COSTSCALE) + cx; 
		y = ((long)sint[i % 360] * (long)r2 / SINTSCALE) + cy;
		if (i != s) {
			if ( (x!=lx) || (y!=ly) ){
				if ( thik > 1 ){
					gdPoint	pxy[4];	// = { lx,ly,x,y,  x,y-thik, lx,y-thik};
					pxy[0].x = (float)lx; pxy[0].y = (float)ly;
					pxy[1].x = (float)x; pxy[1].y = (float)y;
					pxy[2].x = (float)x; pxy[2].y = (float)y-thik;
					pxy[3].x = (float)lx; pxy[3].y = (float)y-thik;
					gdImageFilledPolygon( im, pxy, 4, color );
				} else
					gdImageLine(im, lx, ly, x, y, color );	
			}
		}
		lx = x;
		ly = y;
	}
}



void gdImageFilledArc(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color)
{
	//Ellipse2D(im,cx,cy,w,h,s,e,color);
	//EllipseArc2D (im, 100, 100, 100, 100, 0, 0, 0, 0, color);
	
	int i;
	int lx = 0, ly = 0;
	int r1, r2;
	r1 = w/2;
	r2 = h/2;
	while (e < s) {
		e += 360;
	}
	for (i=s; (i <= e); i++) {
		int x, y;
		x = ((long)cost[i % 360] * (long)r1 / COSTSCALE) + cx; 
		y = ((long)sint[i % 360] * (long)r2 / SINTSCALE) + cy;
		if (i != s) {
			if ( (x!=lx) || (y!=ly) ){
				gdPoint	pxy[4];// = { lx,ly,x,y,  cx,cy };
				pxy[0].x = (float)lx; pxy[0].y = (float)ly;
				pxy[1].x = (float)x; pxy[1].y = (float)y;
				pxy[2].x = (float)cx; pxy[2].y = (float)cy;
				gdImageFilledPolygon( im, pxy, 3, color );
			}
		}
		lx = x;
		ly = y;
	}
}

void gdImagePie(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color)
{
	int r1,r2,x,y;
	
	r1 = w/2;  r2 = h/2;
	gdImageArc(im, cx, cy, w, h, s, e, 1, color);

	x = ((long)cost[s % 360] * (long)r1 / COSTSCALE) + cx; 
	y = ((long)sint[s % 360] * (long)r2 / SINTSCALE) + cy;
	gdImageLine(im, cx, cy, x, y, color);

	x = ((long)cost[e % 360] * (long)r1 / COSTSCALE) + cx; 
	y = ((long)sint[e % 360] * (long)r2 / SINTSCALE) + cy;
	gdImageLine(im, cx, cy, x, y, color);
}

void gdImage3DPie(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color, int color2 )
{
	int r1,r2,depth=20;

	r1 = w/2;  r2 = h/2;
	gdImageFilledArc(im, cx, cy, w, h, s, e, color);

	if ( s>=180 && e<180 ) s=0;
	if ( e>=180 ) e = 180;
	if ( s>=0 && s<=180 && e>=0 && e<=180 )
		gdImageArc(im, cx, cy+depth, w, h, s, e, depth, color2 );
}

void gdImageFilledPie(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color)
{
	int fx,fy, ma, r1, r2;

	s=s%360;  e=e%360;

	gdImagePie( im,  cx,  cy,  w,  h,  s,  e,  color);

	ma = s + ((e-s)/2);
	r1 = w/4;   r2 = h/4;
	fx = ((long)cost[ma % 360] * (long)r1 / COSTSCALE) + cx; 
	fy = ((long)sint[ma % 360] * (long)r2 / SINTSCALE) + cy;
	gdImageFill( im, fx, fy, color );
}

void gdImageFilled3DPie(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color, int color2 )
{
	s=s%360;  e=e%360;

	gdImage3DPie( im,  cx,  cy,  w,  h,  s,  e,  color,color2 );
}

void gdImageFillToBorder(gdImagePtr im, int x, int y, int border, int color)
{
	int lastBorder;
	/* Seek left */
	int leftLimit, rightLimit;
	int i;
	leftLimit = (-1);
	if (border < 0) {
		/* Refuse to fill to a non-solid border */
		return;
	}
	for (i = x; (i >= 0); i--) {
		if (gdImageGetPixel(im, i, y) == border) {
			break;
		}
		gdImageSetPixel(im, i, y, color);
		leftLimit = i;
	}
	if (leftLimit == (-1)) {
		return;
	}
	/* Seek right */
	rightLimit = x;
	for (i = (x+1); (i < im->sx); i++) {	
		if (gdImageGetPixel(im, i, y) == border) {
			break;
		}
		gdImageSetPixel(im, i, y, color);
		rightLimit = i;
	}
	/* Look at lines above and below and start paints */
	/* Above */
	if (y > 0) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			int c;
			c = gdImageGetPixel(im, i, y-1);
			if (lastBorder) {
				if ((c != border) && (c != color)) {	
					gdImageFillToBorder(im, i, y-1, 
						border, color);		
					lastBorder = 0;
				}
			} else if ((c == border) || (c == color)) {
				lastBorder = 1;
			}
		}
	}
	/* Below */
	if (y < ((im->sy) - 1)) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			int c;
			c = gdImageGetPixel(im, i, y+1);
			if (lastBorder) {
				if ((c != border) && (c != color)) {	
					gdImageFillToBorder(im, i, y+1, 
						border, color);		
					lastBorder = 0;
				}
			} else if ((c == border) || (c == color)) {
				lastBorder = 1;
			}
		}
	}
}

void gdImageFill(gdImagePtr im, int x, int y, int color)
{
	int lastBorder;
	int old;
	int leftLimit, rightLimit;
	int i;
	old = gdImageGetPixel(im, x, y);
	if (color == gdTiled) {
		/* Tile fill -- got to watch out! */
		int p, tileColor;	
		int srcx, srcy;
		if (!im->tile) {
			return;
		}
		/* Refuse to flood-fill with a transparent pattern --
			I can't do it without allocating another image */
		if (gdImageGetTransparent(im->tile) != (-1)) {
			return;
		}	
		srcx = x % gdImageSX(im->tile);
		srcy = y % gdImageSY(im->tile);
		p = gdImageGetPixel(im->tile, srcx, srcy);
		tileColor = im->tileColorMap[p];
		if (old == tileColor) {
			/* Nothing to be done */
			return;
		}
	} else {
		if (old == color) {
			/* Nothing to be done */
			return;
		}
	}
	/* Seek left */
	leftLimit = (-1);
	for (i = x; (i >= 0); i--) {
		if (gdImageGetPixel(im, i, y) != old) {
			break;
		}
		gdImageSetPixel(im, i, y, color);
		leftLimit = i;
	}
	if (leftLimit == (-1)) {
		return;
	}
	/* Seek right */
	rightLimit = x;
	for (i = (x+1); (i < im->sx); i++) {	
		if (gdImageGetPixel(im, i, y) != old) {
			break;
		}
		gdImageSetPixel(im, i, y, color);
		rightLimit = i;
	}
	/* Look at lines above and below and start paints */
	/* Above */
	if (y > 0) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			int c;
			c = gdImageGetPixel(im, i, y-1);
			if (lastBorder) {
				if (c == old) {	
					gdImageFill(im, i, y-1, color);		
					lastBorder = 0;
				}
			} else if (c != old) {
				lastBorder = 1;
			}
		}
	}
	/* Below */
	if (y < ((im->sy) - 1)) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			int c;
			c = gdImageGetPixel(im, i, y+1);
			if (lastBorder) {
				if (c == old) {
					gdImageFill(im, i, y+1, color);		
					lastBorder = 0;
				}
			} else if (c != old) {
				lastBorder = 1;
			}
		}
	}
}
	
#ifdef TEST_CODE
void gdImageDump(gdImagePtr im)
{
	int i, j;
	for (i=0; (i < im->sy); i++) {
		for (j=0; (j < im->sx); j++) {
			printf("%d", im->pixels[i][j]);
		}
		printf("\n");
	}
}
#endif

/* Code drawn from ppmtogif.c, from the pbmplus package
**
** Based on GIFENCOD by David Rowley <mgardi@watdscu.waterloo.edu>. A
** Lempel-Zim compression based on "compress".
**
** Modified by Marcel Wijkstra <wijkstra@fwi.uva.nl>
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** The Graphics Interchange Format(c) is the Copyright property of
** CompuServe Incorporated.  GIF(sm) is a Service Mark property of
** CompuServe Incorporated.
*/

/*
 * a code_int must be able to hold 2**GIFBITS values of type int, and also -1
 */
typedef int             code_int;

#ifdef SIGNED_COMPARE_SLOW
typedef unsigned long int count_int;
typedef unsigned short int count_short;
#else /*SIGNED_COMPARE_SLOW*/
typedef long int          count_int;
#endif /*SIGNED_COMPARE_SLOW*/

static int colorstobpp(int colors);
static void BumpPixel (void);
static int GIFNextPixel (gdImagePtr im);
static void GIFEncode (FILE *fp, int GWidth, int GHeight, int GInterlace, int Background, int Transparent, int BitsPerPixel, int *Red, int *Green, int *Blue, gdImagePtr im);
static void BMPWrite(FILE *fp, int xsize, int ysize, int GInterlace, int Background, int Transparent, int depth, int *Red, int *Green, int *Blue, gdImagePtr im);

static void Putword (int w, FILE *fp);
static void compressLZW(int init_bits, FILE *outfile, gdImagePtr im);
static void compressRLE(int init_bits, FILE *outfile, gdImagePtr im, int background);

static void output (code_int code);
static void cl_block (void);
static void cl_hash (register count_int hsize);
static void char_init (void);
static void char_out (int c);
static void flush_char (void);
/* Allows for reuse */
static void init_statics(void);

void gdImageGif(gdImagePtr im, FILE *out, int useRLE )
{
	int interlace, transparent, BitsPerPixel;
	interlace = im->interlace;
	transparent = im->transparent;

	BitsPerPixel = colorstobpp(im->colors.colorsTotal);
	/* Clear any old values in statics strewn through the GIF code */
	init_statics();

	im->useRLE = useRLE;
	/* All set, let's do it. */
	GIFEncode(
		out, im->sx, im->sy, interlace, 0, transparent, BitsPerPixel,
		im->colors.red, im->colors.green, im->colors.blue, im);
}

void gdImageBMP(gdImagePtr im, FILE *out )
{
	int interlace, transparent, BitsPerPixel;

	interlace = im->interlace;
	transparent = im->transparent;

	BitsPerPixel = 8;

	/* Clear any old values in statics strewn through the GIF code */
	init_statics();

	/* All set, let's do it. */
	BMPWrite(out, im->sx, im->sy, interlace, 0, transparent, BitsPerPixel,
		im->colors.red, im->colors.green, im->colors.blue, im);

}

static int colorstobpp(int colors)
{
    int bpp = 0;

    if ( colors <= 2 )
        bpp = 1;
    else if ( colors <= 4 )
        bpp = 2;
    else if ( colors <= 8 )
        bpp = 3;
    else if ( colors <= 16 )
        bpp = 4;
    else if ( colors <= 32 )
        bpp = 5;
    else if ( colors <= 64 )
        bpp = 6;
    else if ( colors <= 128 )
        bpp = 7;
    else if ( colors <= 256 )
        bpp = 8;
    return bpp;
    }

short swapw( short word );
long swapl( long dword );
/* swap word little -> big endian */

short swapw( short word )
{
	short a,b;

	a = word;
	b = a;
	a = a>>8;
	a |= (b<<8);

	return a;
}

/* swap long little -> big endian */
long swapl( long dword )
{
	long x;
	short a,b;

	x = dword;
	a = (x&0xffff);
	b = (short)(x>>16);
	a = swapw( a ); b = swapw( b );

	return ((a<<16) | (b));
}

/*
 * Write out a word to the GIF file
 */
static void Putword(int w, FILE *fp)
{
        fputc( w & 0xff, fp );
        fputc( (w / 256) & 0xff, fp );
}

static void PutLongword(long lw, FILE *fp)
{
	int	w1,w2;

	w1 = lw & 0xffff;
	w2 = lw >> 16;

	Putword( w1, fp );
	Putword( w2, fp );
}
/*

 *  .BMP Structures and keywords...

 */
typedef unsigned long DWORD;
typedef long LONG;
typedef short WORD;
typedef short UINT;
typedef unsigned char BYTE;

#define BMFH_SIZE		(14)

typedef struct tagBITMAPFILEHEADER {    /* bmfh (14) */
    UINT    bfType;		/*Specifies the type of file. This member must be BM. */
    DWORD   bfSize;		/*Specifies the size of the file, in bytes. */
    UINT    bfReserved1;	/* 0 */
    UINT    bfReserved2;	/* 0 */
    DWORD   bfOffBits;		/*Specifies the byte offset from the BITMAPFILEHEADER structure*/
    				/*to the actual bitmap data in the file.*/

} BITMAPFILEHEADER;                   

#define BMIH_SIZE		(40)

typedef struct tagBITMAPINFOHEADER {    /* bmih */
    DWORD   biSize;		/*Specifies the number of bytes required by the BITMAPINFOHEADER structure.*/
    LONG    biWidth;		/*pixels*/
    LONG    biHeight;
    WORD    biPlanes;		/*must be 1 */
    WORD    biBitCount;		/*1,4,8,24 */
    DWORD   biCompression;	/*0 */
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAPINFOHEADER;

#define RGB_SIZE		(4*256)

typedef struct tagRGBQUAD {     /* rgbq */
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;

#define	TOTAL_HEADER_SIZE	(BMFH_SIZE + BMIH_SIZE + RGB_SIZE)

typedef struct tagBMP {
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
} BMP, *BMPptr;

#define WRITEVAR(value,fd)	fwrite( &value, sizeof(value), 1, fd )

static void BMPWrite(FILE *fd, int xsize, int ysize, int GInterlace, int Background, int Transparent, int depth, int *Red, int *Green, int *Blue, gdImagePtr im)
{
	short	col, xl, yl;
	BMP		main_bmp;

	//main_bmp.bmfh.bfType = swapw('BM');
	strncpy( (char*)&main_bmp.bmfh.bfType, "BM", 2 );
	main_bmp.bmfh.bfSize = TOTAL_HEADER_SIZE + (xsize*ysize*depth)/8;
	main_bmp.bmfh.bfReserved1 = main_bmp.bmfh.bfReserved2 = 0;
	main_bmp.bmfh.bfOffBits = TOTAL_HEADER_SIZE;

	WRITEVAR( main_bmp.bmfh.bfType, fd );
	PutLongword( main_bmp.bmfh.bfSize, fd );
	Putword( main_bmp.bmfh.bfReserved1, fd );
	Putword( main_bmp.bmfh.bfReserved2, fd );
	PutLongword( main_bmp.bmfh.bfOffBits, fd );

	main_bmp.bmih.biSize = BMIH_SIZE;
	main_bmp.bmih.biWidth = xsize;
	main_bmp.bmih.biHeight = ysize;
	main_bmp.bmih.biPlanes = 1;
	main_bmp.bmih.biBitCount = depth;
	main_bmp.bmih.biCompression = 0;
	main_bmp.bmih.biSizeImage = (xsize*ysize*depth)/8;
	main_bmp.bmih.biXPelsPerMeter = 2835;
	main_bmp.bmih.biYPelsPerMeter = 2835;

	if ( depth <= 4 )
		main_bmp.bmih.biClrUsed = 16;
	else
	if ( depth <= 8 )
		main_bmp.bmih.biClrUsed = 256;
	else
	if ( depth > 8 )
		main_bmp.bmih.biClrUsed = 0;

	main_bmp.bmih.biClrImportant = 0;
	PutLongword( main_bmp.bmih.biSize, fd );
	PutLongword( main_bmp.bmih.biWidth, fd );
	PutLongword( main_bmp.bmih.biHeight, fd );
	Putword( main_bmp.bmih.biPlanes, fd );
	Putword( main_bmp.bmih.biBitCount, fd );
	PutLongword( main_bmp.bmih.biCompression, fd );
	PutLongword( main_bmp.bmih.biSizeImage, fd );
	PutLongword( main_bmp.bmih.biXPelsPerMeter, fd );
	PutLongword( main_bmp.bmih.biYPelsPerMeter, fd );
	PutLongword( main_bmp.bmih.biClrUsed, fd );
	PutLongword( main_bmp.bmih.biClrImportant, fd );

	for ( col=0; col <256; col++){
		BYTE	rgbq[4];
		rgbq[0] = Blue[col];
		rgbq[1] = Green[col];
		rgbq[2] = Red[col];
		rgbq[3]=0;
		WRITEVAR( rgbq,fd );
	}
	for ( yl=ysize-1; yl>=0 ; yl--){
	//for ( yl=0; yl < ysize; yl++){
		for ( xl=0; xl < xsize; xl++){
			fputc( im->pixels[yl][xl], fd );
		}
	}
}

/*****************************************************************************
 *
 * GIFENCODE.C    - GIF Image compression interface
 *
 * GIFEncode( FName, GHeight, GWidth, GInterlace, Background, Transparent,
 *            BitsPerPixel, Red, Green, Blue, gdImagePtr )
 *
 *****************************************************************************/

static int Width, Height;
static int curx, cury;
static long CountDown;
static int Pass = 0;
static int Interlace;

/*
 * Bump the 'curx' and 'cury' to point to the next pixel
 */
static void BumpPixel(void)
{
        /*
         * Bump the current X position
         */
        ++curx;

        /*
         * If we are at the end of a scan line, set curx back to the beginning
         * If we are interlaced, bump the cury to the appropriate spot,
         * otherwise, just increment it.
         */
        if( curx == Width ) {
                curx = 0;

                if( !Interlace )
                        ++cury;
                else {
                     switch( Pass ) {

                       case 0:
                          cury += 8;
                          if( cury >= Height ) {
                                ++Pass;
                                cury = 4;
                          }
                          break;

                       case 1:
                          cury += 8;
                          if( cury >= Height ) {
                                ++Pass;
                                cury = 2;
                          }
                          break;

                       case 2:
                          cury += 4;
                          if( cury >= Height ) {
                             ++Pass;
                             cury = 1;
                          }
                          break;

                       case 3:
                          cury += 2;
                          break;
                        }
                }
        }
}

/*
 * Return the next pixel from the image
 */
static int GIFNextPixel(gdImagePtr im)
{
        int r;

        if( CountDown == 0 )
                return EOF;

        --CountDown;

        r = gdImageGetPixel(im, curx, cury);

        BumpPixel();

        return r;
}

/* public */

static void GIFEncode(FILE *fp, int GWidth, int GHeight, int GInterlace, int Background, int Transparent, int BitsPerPixel, int *Red, int *Green, int *Blue, gdImagePtr im)
{
        int B;
        int RWidth, RHeight;
        int LeftOfs, TopOfs;
        int Resolution;
        int ColorMapSize;
        int InitCodeSize;
        int i;

        Interlace = GInterlace;

        ColorMapSize = 1 << BitsPerPixel;

        RWidth = Width = GWidth;
        RHeight = Height = GHeight;
        LeftOfs = TopOfs = 0;

        Resolution = BitsPerPixel;

        /*
         * Calculate number of bits we are expecting
         */
        CountDown = (long)Width * (long)Height;

        /*
         * Indicate which pass we are on (if interlace)
         */
        Pass = 0;

        /*
         * The initial code size
         */
        if( BitsPerPixel <= 1 )
                InitCodeSize = 2;
        else
                InitCodeSize = BitsPerPixel;

        /*
         * Set up the current x and y position
         */
        curx = cury = 0;

        /*
         * Write the Magic header
         */
        fwrite( Transparent < 0 ? "GIF87a" : "GIF89a", 1, 6, fp );

        /*
         * Write out the screen width and height
         */
        Putword( RWidth, fp );
        Putword( RHeight, fp );

        /*
         * Indicate that there is a global colour map
         */
        B = 0x80;       /* Yes, there is a color map */

        /*
         * OR in the resolution
         */
        B |= (Resolution - 1) << 4;

        /*
         * OR in the Bits per Pixel
         */
        B |= (BitsPerPixel - 1);

        /*
         * Write it out
         */
        fputc( B, fp );

        /*
         * Write out the Background colour
         */
        fputc( Background, fp );

        /*
         * Byte of 0's (future expansion)
         */
        fputc( 0, fp );

        /*
         * Write out the Global Colour Map
         */
        for( i=0; i<ColorMapSize; ++i ) {
                fputc( Red[i], fp );
                fputc( Green[i], fp );
                fputc( Blue[i], fp );
        }

	/*
	 * Write out extension for transparent colour index, if necessary.
	 */
	if ( Transparent >= 0 ) {
	    fputc( '!', fp );
	    fputc( 0xf9, fp );
	    fputc( 4, fp );
	    fputc( 1, fp );
	    fputc( 0, fp );
	    fputc( 0, fp );
	    fputc( (unsigned char) Transparent, fp );
	    fputc( 0, fp );
	}

        /*
         * Write an Image separator
         */
        fputc( ',', fp );

        /*
         * Write the Image header
         */

        Putword( LeftOfs, fp );
        Putword( TopOfs, fp );
        Putword( Width, fp );
        Putword( Height, fp );

        /*
         * Write out whether or not the image is interlaced
         */
        if( Interlace )
                fputc( 0x40, fp );
        else
                fputc( 0x00, fp );

        /*
         * Write out the initial code size
         */
        fputc( InitCodeSize, fp );

        /*
         * Go and actually compress the data
         */
		if ( im->useRLE )
			compressRLE( InitCodeSize+1, fp, im, Background );
		else
	        	compressLZW( InitCodeSize+1, fp, im );			// USE stupid patented LZW crap, who cares

        /*
         * Write out a Zero-length packet (to end the series)
         */
        fputc( 0, fp );

        /*
         * Write the GIF file terminator
         */
        fputc( ';', fp );
}




/***************************************************************************
 *
 *  GIFCOMPR.C       - GIF Image compression routines
 *
 *  Lempel-Ziv compression based on 'compress'.  GIF modifications by
 *  David Rowley (mgardi@watdcsu.waterloo.edu)
 *
 ***************************************************************************/

/*
 * General DEFINEs
 */

#define GIFBITS    12

#define HSIZE  5003            /* 80% occupancy */

#ifdef NO_UCHAR
 typedef char   char_type;
#else /*NO_UCHAR*/
 typedef        unsigned char   char_type;
#endif /*NO_UCHAR*/

/*
 *
 * GIF Image compression - modified 'compress'
 *
 * Based on: compress.c - File compression ala IEEE Computer, June 1984.
 *
 * By Authors:  Spencer W. Thomas       (decvax!harpo!utah-cs!utah-gr!thomas)
 *              Jim McKie               (decvax!mcvax!jim)
 *              Steve Davies            (decvax!vax135!petsd!peora!srd)
 *              Ken Turkowski           (decvax!decwrl!turtlevax!ken)
 *              James A. Woods          (decvax!ihnp4!ames!jaw)
 *              Joe Orost               (decvax!vax135!petsd!joe)
 *
 */
#include <ctype.h>

#define ARGVAL() (*++(*argv) || (--argc && *++argv))

static int n_bits;                        /* number of bits/code */
static int maxbits = GIFBITS;                /* user settable max # bits/code */
static code_int maxcode;                  /* maximum code, given n_bits */
static code_int maxmaxcode = (code_int)1 << GIFBITS; /* should NEVER generate this code */
#ifdef COMPATIBLE               /* But wrong! */
# define MAXCODE(n_bits)        ((code_int) 1 << (n_bits) - 1)
#else /*COMPATIBLE*/
# define MAXCODE(n_bits)        (((code_int) 1 << (n_bits)) - 1)
#endif /*COMPATIBLE*/

static count_int htab [HSIZE];
static unsigned short codetab [HSIZE];
#define HashTabOf(i)       htab[i]
#define CodeTabOf(i)    codetab[i]

static code_int hsize = HSIZE;                 /* for dynamic table sizing */

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**GIFBITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i) CodeTabOf(i)
#define tab_suffixof(i)        ((char_type*)(htab))[i]
#define de_stack               ((char_type*)&tab_suffixof((code_int)1<<GIFBITS))

static code_int free_ent = 0;                  /* first unused entry */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
static int clear_flg = 0;

static int offset;
static long int in_count = 1;            /* length of input */
static long int out_count = 0;           /* # of codes output (for debugging) */

/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

static int g_init_bits;
static FILE* g_outfile;

static int ClearCode;
static int EOFCode;

static void compressLZW(int init_bits, FILE *outfile, gdImagePtr im)
{
    register long fcode;
    register code_int i /* = 0 */;
    register int c;
    register code_int ent;
    register code_int disp;
    register code_int hsize_reg;
    register int hshift;

    /*
     * Set up the globals:  g_init_bits - initial number of bits
     *                      g_outfile   - pointer to output file
     */
    g_init_bits = init_bits;
    g_outfile = outfile;

    /*
     * Set up the necessary values
     */
    offset = 0;
    out_count = 0;
    clear_flg = 0;
    in_count = 1;
    maxcode = MAXCODE(n_bits = g_init_bits);

    ClearCode = (1 << (init_bits - 1));
    EOFCode = ClearCode + 1;
    free_ent = ClearCode + 2;

    char_init();

    ent = GIFNextPixel( im );

    hshift = 0;
    for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L )
        ++hshift;
    hshift = 8 - hshift;                /* set hash code range bound */

    hsize_reg = hsize;
    cl_hash( (count_int) hsize_reg);            /* clear hash table */

    output( (code_int)ClearCode );

#ifdef SIGNED_COMPARE_SLOW
    while ( (c = GIFNextPixel( im )) != (unsigned) EOF ) {
#else /*SIGNED_COMPARE_SLOW*/
    while ( (c = GIFNextPixel( im )) != EOF ) {  /* } */
#endif /*SIGNED_COMPARE_SLOW*/

        ++in_count;

        fcode = (long) (((long) c << maxbits) + ent);
        i = (((code_int)c << hshift) ^ ent);    /* xor hashing */

        if ( HashTabOf (i) == fcode ) {
            ent = CodeTabOf (i);
            continue;
        } else if ( (long)HashTabOf (i) < 0 )      /* empty slot */
            goto nomatch;
        disp = hsize_reg - i;           /* secondary hash (after G. Knott) */
        if ( i == 0 )
            disp = 1;
probe:
        if ( (i -= disp) < 0 )
            i += hsize_reg;

        if ( HashTabOf (i) == fcode ) {
            ent = CodeTabOf (i);
            continue;
        }
        if ( (long)HashTabOf (i) > 0 )
            goto probe;
nomatch:
        output ( (code_int) ent );
        ++out_count;
        ent = c;
#ifdef SIGNED_COMPARE_SLOW
        if ( (unsigned) free_ent < (unsigned) maxmaxcode) {
#else /*SIGNED_COMPARE_SLOW*/
        if ( free_ent < maxmaxcode ) {  /* } */
#endif /*SIGNED_COMPARE_SLOW*/
            CodeTabOf (i) = free_ent++; /* code -> hashtable */
            HashTabOf (i) = fcode;
        } else
                cl_block();
    }
    /*
     * Put out the final code.
     */
    output( (code_int)ent );
    ++out_count;
    output( (code_int) EOFCode );
}

/*****************************************************************
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 *      code:   A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *              that n_bits =< (long)wordsize - 1.
 * Outputs:
 *      Outputs code to the file.
 * Assumptions:
 *      Chars are 8 bits long.
 * Algorithm:
 *      Maintain a GIFBITS character long buffer (so that 8 codes will
 * fit in it exactly).  Use the VAX insv instruction to insert each
 * code in turn.  When the buffer fills up empty it and start over.
 */

static unsigned long cur_accum = 0;
static int cur_bits = 0;

static unsigned long masks[] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
                                  0x001F, 0x003F, 0x007F, 0x00FF,
                                  0x01FF, 0x03FF, 0x07FF, 0x0FFF,
                                  0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

static void output(code_int code)
{
    cur_accum &= masks[ cur_bits ];

    if( cur_bits > 0 )
        cur_accum |= ((long)code << cur_bits);
    else
        cur_accum = code;

    cur_bits += n_bits;

    while( cur_bits >= 8 ) {
        char_out( (unsigned int)(cur_accum & 0xff) );
        cur_accum >>= 8;
        cur_bits -= 8;
    }

    /*
     * If the next entry is going to be too big for the code size,
     * then increase it, if possible.
     */
   if ( free_ent > maxcode || clear_flg ) {

            if( clear_flg ) {

                maxcode = MAXCODE (n_bits = g_init_bits);
                clear_flg = 0;

            } else {

                ++n_bits;
                if ( n_bits == maxbits )
                    maxcode = maxmaxcode;
                else
                    maxcode = MAXCODE(n_bits);
            }
        }

    if( code == EOFCode ) {
        /*
         * At EOF, write the rest of the buffer.
         */
        while( cur_bits > 0 ) {
                char_out( (unsigned int)(cur_accum & 0xff) );
                cur_accum >>= 8;
                cur_bits -= 8;
        }

        flush_char();

        fflush( g_outfile );

        if( ferror( g_outfile ) )
		return;
    }
}

/*
 * Clear out the hash table
 */
static void cl_block (void)             /* table clear for block compress */
{

        cl_hash ( (count_int) hsize );
        free_ent = ClearCode + 2;
        clear_flg = 1;

        output( (code_int)ClearCode );
}

static void cl_hash(register count_int hsize)          /* reset code table */                         
{
        register count_int *htab_p = htab+hsize;

        register long i;
        register long m1 = -1;

        i = hsize - 16;
        do {                            /* might use Sys V memset(3) here */
                *(htab_p-16) = m1;
                *(htab_p-15) = m1;
                *(htab_p-14) = m1;
                *(htab_p-13) = m1;
                *(htab_p-12) = m1;
                *(htab_p-11) = m1;
                *(htab_p-10) = m1;
                *(htab_p-9) = m1;
                *(htab_p-8) = m1;
                *(htab_p-7) = m1;
                *(htab_p-6) = m1;
                *(htab_p-5) = m1;
                *(htab_p-4) = m1;
                *(htab_p-3) = m1;
                *(htab_p-2) = m1;
                *(htab_p-1) = m1;
                htab_p -= 16;
        } while ((i -= 16) >= 0);

        for ( i += 16; i > 0; --i )
                *--htab_p = m1;
}

/******************************************************************************
 *
 * GIF Specific routines
 *
 ******************************************************************************/

/*
 * Number of characters so far in this 'packet'
 */
static int a_count;

/*
 * Set up the 'byte output' routine
 */
static void
char_init(void)
{
        a_count = 0;
}

/*
 * Define the storage for the packet accumulator
 */
static char accum[ 256 ];

/*
 * Add a character to the end of the current packet, and if it is 254
 * characters, flush the packet to disk.
 */
static void
char_out(int c)
{
        accum[ a_count++ ] = c;
        if( a_count >= 254 )
                flush_char();
}

/*
 * Flush the packet to disk, and reset the accumulator
 */
static void flush_char(void)
{
        if( a_count > 0 ) {
                fputc( a_count, g_outfile );
                fwrite( accum, 1, a_count, g_outfile );
                a_count = 0;
        }
}

static void init_statics(void) {
	/* Some of these are properly initialized later. What I'm doing
		here is making sure code that depends on C's initialization
		of statics doesn't break when the code gets called more
		than once. */
	Width = 0;
	Height = 0;
	curx = 0;
	cury = 0;
	CountDown = 0;
	Pass = 0;
	Interlace = 0;
	a_count = 0;
	cur_accum = 0;
	cur_bits = 0;
	g_init_bits = 0;
	g_outfile = 0;
	ClearCode = 0;
	EOFCode = 0;
	free_ent = 0;
	clear_flg = 0;
	offset = 0;
	in_count = 1;
	out_count = 0;	
	hsize = HSIZE;
	n_bits = 0;
	maxbits = GIFBITS;
	maxcode = 0;
	maxmaxcode = (code_int)1 << GIFBITS;
}

/*---------------------------------------------------------------




-----------------------------------------------------------------*/


/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, 1991, 1993, David Koblas.  (koblas@netcom.com)    | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */


#define        MAXCOLORMAPSIZE         256

#define CM_RED         0
#define CM_GREEN       1
#define CM_BLUE                2

#define        MAX_LWZ_BITS            12

#define INTERLACE              0x40
#define LOCALCOLORMAP  0x80
#define BitSet(byte, bit)      (((byte) & (bit)) == (bit))

#define ReadOK(file,buffer,len) (fread(buffer, len, 1, file))

#define LM_to_uint(a,b)                        (((b)<<8)|(a))

/* We may eventually want to use this information, but def it out for now */
#if 0
static struct {
       unsigned int    Width;
       unsigned int    Height;
       unsigned char   ColorMap[3][MAXCOLORMAPSIZE];
       unsigned int    BitPixel;
       unsigned int    ColorResolution;
       unsigned int    Background;
       unsigned int    AspectRatio;
} GifScreen;
#endif

static struct {
       int     transparent;
       int     delayTime;
       int     inputFlag;
       int     disposal;
} Gif89 = { -1, -1, -1, 0 };

static int ReadColorMap (FILE *fd, int number, unsigned char (*buffer)[256]);
static int DoExtension (FILE *fd, int label, int *Transparent);
static int GetDataBlock (FILE *fd, unsigned char *buf);
static int GetCode (FILE *fd, int code_size, int flag);
static int LWZReadByte (FILE *fd, int flag, int input_code_size);
static void ReadImage (gdImagePtr im, FILE *fd, int len, int height, unsigned char (*cmap)[256], int interlace, int ignore);

static int MemReadColorMap ( unsigned char **fd, int number, unsigned char (*buffer)[256]);
static int MemDoExtension (unsigned char **fd, int label, int *Transparent);
static int MemGetDataBlock (unsigned char **fd, unsigned char *buf);
static int MemGetCode (unsigned char **fd, int code_size, int flag);
static int MemLWZReadByte (unsigned char **fd, int flag, int input_code_size);
static void MemReadImage (gdImagePtr im, unsigned char **fd, int len, int height, unsigned char (*cmap)[256], int interlace, int ignore);
//void gdImageQuickSetPixel(gdImagePtr im, int x, int y, int color);
int MemRead( unsigned char **fd, unsigned char *dest, long len );

int ZeroDataBlock;

gdImagePtr gdImageCreateFromGif(FILE *fd)
{
	int imageNumber;
	int BitPixel;
	int ColorResolution;
	int Background;
	int AspectRatio;
	int Transparent = (-1);
	unsigned char   buf[16];
	unsigned char   c;
	unsigned char   ColorMap[3][MAXCOLORMAPSIZE];
	unsigned char   localColorMap[3][MAXCOLORMAPSIZE];
	int             imw, imh;
	int             useGlobalColormap;
	int             bitPixel;
	int             imageCount = 0;
	char            version[4];
	gdImagePtr im = 0;
	ZeroDataBlock = FALSE;
	
	imageNumber = 1;
	if (! ReadOK(fd,buf,6)) {
		return 0;
	}
	if (strncmp((char *)buf,"GIF",3) != 0) {
		return 0;
	}
	strncpy(version, (char *)buf + 3, 3);
	version[3] = '\0';
	
	if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0)) {
		return 0;
	}
	if (! ReadOK(fd,buf,7)) {
		return 0;
	}
	BitPixel        = 2<<(buf[4]&0x07);
	ColorResolution = (int) (((buf[4]&0x70)>>3)+1);
	Background      = buf[5];
	AspectRatio     = buf[6];
	
	if (BitSet(buf[4], LOCALCOLORMAP)) {    /* Global Colormap */
		if (ReadColorMap(fd, BitPixel, ColorMap)) {
			return 0;
		}
	}

	for (;;) {
		int pos = ftell( fd );
		
	   if (! ReadOK(fd,&c,1)) {
           return 0;
	   }
	   if (c == ';') {         /* GIF terminator */
           int i;
           if (imageCount < imageNumber) {
                   return 0;
           }
           /* Terminator before any image was declared! */
           if (!im) {
                  return 0;
           }
		   /* Check for open colors at the end, so
              we can reduce colorsTotal and ultimately
              BitsPerPixel */
           for (i=((im->colors.colorsTotal-1)); (i>=0); i--) {
               if (im->colors.open[i]) {
                       im->colors.colorsTotal--;
               } else {
                       break;
               }
           } 
           return im;
	   }
	
	   if (c == '!') {         /* Extension */
           if (! ReadOK(fd,&c,1)) {
                   return 0;
           }
           DoExtension(fd, c, &Transparent);
           continue;
	   }
	
	   if (c != ',') {         /* Not a valid start character */
           continue;
	   }
	
	   ++imageCount;
	
	   if (! ReadOK(fd,buf,9)) {
	       return 0;
	   }
	
	   useGlobalColormap = ! BitSet(buf[8], LOCALCOLORMAP);
	
	   bitPixel = 1<<((buf[8]&0x07)+1);
	
	   imw = LM_to_uint(buf[4],buf[5]);
	   imh = LM_to_uint(buf[6],buf[7]);
	   im = gdImageCreate(imw, imh,0);
	
	   if (!im) {
		 return 0;
	   }
	   im->interlace = BitSet(buf[8], INTERLACE);
	   if (! useGlobalColormap) {
	           if (ReadColorMap(fd, bitPixel, localColorMap)) { 
	                 return 0;
	           }
	           ReadImage(im, fd, imw, imh, localColorMap, 
	                     BitSet(buf[8], INTERLACE), 
	                     imageCount != imageNumber);
	   } else {
	           ReadImage(im, fd, imw, imh,
	                     ColorMap, 
	                     BitSet(buf[8], INTERLACE), 
	                     imageCount != imageNumber);
	   }
	   if (Transparent != (-1)) {
	           gdImageColorTransparent(im, Transparent);
	   }	   
	}
}


int MemRead( unsigned char **fd, unsigned char *dest, long len )
{
	static int	curpos,length;

	if ( dest ){
		if ( curpos > length )
			return 0;

		if ( *fd ){
			if( len ){
				memcpy( dest, *fd, len );
				*fd += len;
				curpos += len;
			}
			return len;
		}
	} else {
		length = len;
		curpos = 0;
	}
	return 0;
}

gdImagePtr gdImageCreateFromGifMem( unsigned char *gifPtr, long gifLen )
{
       int imageNumber;
       int BitPixel;
       int ColorResolution;
       int Background;
       int AspectRatio;
       int Transparent = (-1);
       unsigned char   buf[16];
       unsigned char   c, **fd, *fdp;
       unsigned char   ColorMap[3][MAXCOLORMAPSIZE];
       unsigned char   localColorMap[3][MAXCOLORMAPSIZE];
       int             imw, imh;
       int             useGlobalColormap;
       int             bitPixel;
       int             imageCount = 0;
       char            version[4];
       gdImagePtr im = 0;
       ZeroDataBlock = FALSE;

		fdp = gifPtr;
		fd = &fdp;
		
       imageNumber = 1;
       MemRead( 0, 0, gifLen );
       MemRead( fd, buf, 6 );
       if (strncmp((char *)buf,"GIF",3) != 0) {
		return 0;
	}
       strncpy(version, (char *)buf + 3, 3);
       version[3] = '\0';

       if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0)) {
		return 0;
	}
		MemRead( fd, buf, 7 );
       BitPixel        = 2<<(buf[4]&0x07);
       ColorResolution = (int) (((buf[4]&0x70)>>3)+1);
       Background      = buf[5];
       AspectRatio     = buf[6];

       if (BitSet(buf[4], LOCALCOLORMAP)) {    /* Global Colormap */
               if (MemReadColorMap(fd, BitPixel, ColorMap)) {
			return 0;
		}
       }
       for (;;) {
               if (! MemRead(fd,&c,1)) {
                       return 0;
               }
              if (c == ';') {         /* GIF terminator */
                       int i;
                       if (imageCount < imageNumber) {
                               return 0;
                       }
                       /* Terminator before any image was declared! */
                       if (!im) {
                              return 0;
                       }
		       /* Check for open colors at the end, so
                          we can reduce colorsTotal and ultimately
                          BitsPerPixel */
                       for (i=((im->colors.colorsTotal-1)); (i>=0); i--) {
                               if (im->colors.open[i]) {
                                       im->colors.colorsTotal--;
                               } else {
                                       break;
                               }
                       } 
                       return im;
               }

               if (c == '!') {         /* Extension */
				       if (! MemRead(fd,&c,1)) {
			                   return 0;
		               }
                       MemDoExtension(fd, c, &Transparent);
                       continue;
               }

               if (c != ',') {         /* Not a valid start character */
                       continue;
               }

               ++imageCount;

			   if ( imageCount > 1 ){
				   return im;
			   }

               if (! MemRead(fd,buf,9)) {
	               return 0;
               }

               useGlobalColormap = ! BitSet(buf[8], LOCALCOLORMAP);

               bitPixel = 1<<((buf[8]&0x07)+1);

               imw = LM_to_uint(buf[4],buf[5]);
               imh = LM_to_uint(buf[6],buf[7]);
               im = gdImageCreate(imw, imh,0);
		       if (!im) {
				 return 0;
		       }
               im->interlace = BitSet(buf[8], INTERLACE);
               if (! useGlobalColormap) {
                       if (MemReadColorMap(fd, bitPixel, localColorMap)) { 
                                 return 0;
                       }
                       MemReadImage(im, fd, imw, imh, localColorMap, 
                                 BitSet(buf[8], INTERLACE), 
                                 imageCount != imageNumber);
               } else {
                       MemReadImage(im, fd, imw, imh,
                                 ColorMap, 
                                 BitSet(buf[8], INTERLACE), 
                                 imageCount != imageNumber);
               }
               if (Transparent != (-1)) {
                       gdImageColorTransparent(im, Transparent);
               }	   
       }
}

static int ReadColorMap(FILE *fd, int number, unsigned char (*buffer)[256])
{
       int             i;
       unsigned char   rgb[3];


       for (i = 0; i < number; ++i) {
               if (! ReadOK(fd, rgb, sizeof(rgb))) {
                       return TRUE;
               }
               buffer[CM_RED][i] = rgb[0] ;
               buffer[CM_GREEN][i] = rgb[1] ;
               buffer[CM_BLUE][i] = rgb[2] ;
       }


       return FALSE;
}

static int MemReadColorMap( unsigned char **fd, int number, unsigned char (*buffer)[256])
{
       int             i;
       unsigned char   rgb[3];

       for (i = 0; i < number; ++i) {
               if (! MemRead(fd, rgb, sizeof(rgb))) {
                       return TRUE;
               }
               buffer[CM_RED][i] = rgb[0] ;
               buffer[CM_GREEN][i] = rgb[1] ;
               buffer[CM_BLUE][i] = rgb[2] ;
       }

       return FALSE;
}


static int DoExtension(FILE *fd, int label, int *Transparent)
{
       static unsigned char     buf[256];

       switch (label) {
       case 0xf9:              /* Graphic Control Extension */
               (void) GetDataBlock(fd, (unsigned char*) buf);
               Gif89.disposal    = (buf[0] >> 2) & 0x7;
               Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
               Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
               if ((buf[0] & 0x1) != 0)
                       *Transparent = buf[3];

               while (GetDataBlock(fd, (unsigned char*) buf) != 0)
                       ;
               return FALSE;
       default:
               break;
       }
       while (GetDataBlock(fd, (unsigned char*) buf) != 0)
               ;

       return FALSE;
}


static int MemDoExtension( unsigned char **fd, int label, int *Transparent)
{
       static unsigned char     buf[256];

       switch (label) {
       case 0xf9:              /* Graphic Control Extension */
               (void) MemGetDataBlock(fd, (unsigned char*) buf);
               Gif89.disposal    = (buf[0] >> 2) & 0x7;
               Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
               Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
               if ((buf[0] & 0x1) != 0)
                       *Transparent = buf[3];

               while (MemGetDataBlock(fd, (unsigned char*) buf) != 0)
                       ;
               return FALSE;
       default:
               break;
       }
       while (MemGetDataBlock(fd, (unsigned char*) buf) != 0)
               ;

       return FALSE;
}


static int GetDataBlock(FILE *fd, unsigned char *buf)
{
       unsigned char   count;

       if (! ReadOK(fd,&count,1)) {
               return -1;
       }

       ZeroDataBlock = count == 0;

       if ((count != 0) && (! ReadOK(fd, buf, count))) {
               return -1;
       }

       return count;
}


static int MemGetDataBlock( unsigned char **fd, unsigned char *buf)
{
       unsigned char   count;

       if (! MemRead(fd,&count,1)) {
               return -1;
       }

       ZeroDataBlock = count == 0;

       if ((count != 0) && (! MemRead(fd, buf, count))) {
               return -1;
       }

       return count;
}

static int GetCode(FILE *fd, int code_size, int flag)
{
       static unsigned char    buf[280];
       static int              curbit, lastbit, done, last_byte;
       int                     i, j, ret;
       unsigned char           count;

       if (flag) {
               curbit = 0;
               lastbit = 0;
               done = FALSE;
			   last_byte = 0;
               return 0;
       }

       if ( (curbit+code_size) >= lastbit) {
               if (done) {
                       if (curbit >= lastbit) {
                                /* Oh well */
                       }                        
                       return -1;
               }

			   if( last_byte )
			   {
					buf[0] = buf[last_byte-2];
					buf[1] = buf[last_byte-1];
			   }
			   else
			   {
					buf[0] = 0;
					buf[1] = 0;
			   }

               if ((count = GetDataBlock(fd, &buf[2])) == 0)
                       done = TRUE;

               last_byte = 2 + count;
               curbit = (curbit - lastbit) + 16;
               lastbit = (2+count)*8 ;
       }

       ret = 0;
       for (i = curbit, j = 0; j < code_size; ++i, ++j)
               ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;

       curbit += code_size;

       return ret;
}



static int MemGetCode(unsigned char **fd, int code_size, int flag)
{
       static unsigned char    buf[280];
       static int              curbit, lastbit, done, last_byte;
       int                     i, j, ret;
       unsigned char           count;

       if (flag) {
               curbit = 0;
               lastbit = 0;
               done = FALSE;
			   last_byte = 0;
               return 0;
       }

       if ( (curbit+code_size) >= lastbit) {
               if (done) {
                       if (curbit >= lastbit) {
                                /* Oh well */
                       }                        
                       return -1;
               }

			   if( last_byte )
			   {
					buf[0] = buf[last_byte-2];
					buf[1] = buf[last_byte-1];
			   }
			   else
			   {
					buf[0] = 0;
					buf[1] = 0;
			   }

               if ((count = MemGetDataBlock(fd, &buf[2])) == 0)
                       done = TRUE;

               last_byte = 2 + count;
               curbit = (curbit - lastbit) + 16;
               lastbit = (2+count)*8 ;
       }

       ret = 0;
       for (i = curbit, j = 0; j < code_size; ++i, ++j)
               ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;

       curbit += code_size;

       return ret;
}


static int LWZReadByte(FILE *fd, int flag, int input_code_size)
{
       static int      fresh = FALSE;
       int             code, incode;
       static int      code_size, set_code_size;
       static int      max_code, max_code_size;
       static int      firstcode, oldcode;
       static int      clear_code, end_code;
       static int      table[2][(1<< MAX_LWZ_BITS)];
       static int      stack[(1<<(MAX_LWZ_BITS))*2], *sp;
       register int    i;

       if (flag) {
               set_code_size = input_code_size;
               code_size = set_code_size+1;
               clear_code = 1 << set_code_size ;
               end_code = clear_code + 1;
               max_code_size = 2*clear_code;
               max_code = clear_code+2;

               GetCode(fd, 0, TRUE);
               
               fresh = TRUE;

               for (i = 0; i < clear_code; ++i) {
                       table[0][i] = 0;
                       table[1][i] = i;
               }
               for (; i < (1<<MAX_LWZ_BITS); ++i)
                       table[0][i] = table[1][0] = 0;

               sp = stack;

               return 0;
       } else if (fresh) {
               fresh = FALSE;
               do {
                       firstcode = oldcode =
                               GetCode(fd, code_size, FALSE);
               } while (firstcode == clear_code);
               return firstcode;
       }

       if (sp > stack)
               return *--sp;

       while ((code = GetCode(fd, code_size, FALSE)) >= 0) {
               if (code == clear_code) {
                       for (i = 0; i < clear_code; ++i) {
                               table[0][i] = 0;
                               table[1][i] = i;
                       }
                       for (; i < (1<<MAX_LWZ_BITS); ++i)
                               table[0][i] = table[1][i] = 0;
                       code_size = set_code_size+1;
                       max_code_size = 2*clear_code;
                       max_code = clear_code+2;
                       sp = stack;
                       firstcode = oldcode =
                                       GetCode(fd, code_size, FALSE);
                       return firstcode;
               } else if (code == end_code) {
                       int             count;
                       unsigned char   buf[260];

                       if (ZeroDataBlock)
                               return -2;

                       while ((count = GetDataBlock(fd, buf)) > 0)
                               ;

                       if (count != 0)
                       return -2;
               }

               incode = code;

               if (code >= max_code) {
                       *sp++ = firstcode;
                       code = oldcode;
               }

               while (code >= clear_code) {
                       *sp++ = table[1][code];
                       if (code == table[0][code]) {
                               /* Oh well */
                       }
                       code = table[0][code];
               }

               *sp++ = firstcode = table[1][code];

               if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
                       table[0][code] = oldcode;
                       table[1][code] = firstcode;
                       ++max_code;
                       if ((max_code >= max_code_size) &&
                               (max_code_size < (1<<MAX_LWZ_BITS))) {
                               max_code_size *= 2;
                               ++code_size;
                       }
               }

               oldcode = incode;

               if (sp > stack)
                       return *--sp;
       }
       return code;
}

static int MemLWZReadByte(unsigned char **fd, int flag, int input_code_size)
{
       static int      fresh = FALSE;
       int             code, incode;
       static int      code_size, set_code_size;
       static int      max_code, max_code_size;
       static int      firstcode, oldcode;
       static int      clear_code, end_code, *sp;
       static int      table[2][(1<< MAX_LWZ_BITS)];
       static int      stack[(1<<(MAX_LWZ_BITS))*2];
       register int    i;

       if (flag) {
               set_code_size = input_code_size;
               code_size = set_code_size+1;
               clear_code = 1 << set_code_size ;
               end_code = clear_code + 1;
               max_code_size = 2*clear_code;
               max_code = clear_code+2;

               MemGetCode(fd, 0, TRUE);
               
               fresh = TRUE;

               for (i = 0; i < clear_code; ++i) {
                       table[0][i] = 0;
                       table[1][i] = i;
               }
               for (; i < (1<<MAX_LWZ_BITS); ++i)
                       table[0][i] = table[1][0] = 0;

               sp = stack;

               return 0;
       } else if (fresh) {
               fresh = FALSE;
               do {
                       firstcode = oldcode =
                               MemGetCode(fd, code_size, FALSE);
               } while (firstcode == clear_code);
               return firstcode;
       }

       if (sp > stack)
               return *--sp;

       while ((code = MemGetCode(fd, code_size, FALSE)) >= 0) {
               if (code == clear_code) {
                       for (i = 0; i < clear_code; ++i) {
                               table[0][i] = 0;
                               table[1][i] = i;
                       }
                       for (; i < (1<<MAX_LWZ_BITS); ++i)
                               table[0][i] = table[1][i] = 0;
                       code_size = set_code_size+1;
                       max_code_size = 2*clear_code;
                       max_code = clear_code+2;
                       sp = stack;
                       firstcode = oldcode =
                                       MemGetCode(fd, code_size, FALSE);
                       return firstcode;
               } else if (code == end_code) {
                       int             count;
                       unsigned char   buf[260];

                       if (ZeroDataBlock)
                               return -2;

                       while ((count = MemGetDataBlock(fd, buf)) > 0)
                               ;

                       if (count != 0)
                       return -2;
               }

               incode = code;

               if (code >= max_code) {
                       *sp++ = firstcode;
                       code = oldcode;
               }

               while (code >= clear_code) {
                       *sp++ = table[1][code];
                       if (code == table[0][code]) {
                               /* Oh well */
                       }
                       code = table[0][code];
               }

               *sp++ = firstcode = table[1][code];

               if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
                       table[0][code] = oldcode;
                       table[1][code] = firstcode;
                       ++max_code;
                       if ((max_code >= max_code_size) &&
                               (max_code_size < (1<<MAX_LWZ_BITS))) {
                               max_code_size *= 2;
                               ++code_size;
                       }
               }

               oldcode = incode;

               if (sp > stack)
                       return *--sp;
       }
       return code;
}

static void ReadImage(gdImagePtr im, FILE *fd, int len, int height, unsigned char (*cmap)[256], int interlace, int ignore)
{
       unsigned char   c;      
       int             v;
       int             xpos = 0, ypos = 0, pass = 0;
       int i;
       /* Stash the color map into the image */
       for (i=0; (i<gdMaxColors); i++) {
               im->colors.red[i] = cmap[CM_RED][i];	
               im->colors.green[i] = cmap[CM_GREEN][i];	
               im->colors.blue[i] = cmap[CM_BLUE][i];	
               im->colors.open[i] = 1;
       }
       /* Many (perhaps most) of these colors will remain marked open. */
       im->colors.colorsTotal = gdMaxColors;
       /*
       **  Initialize the Compression routines
       */
       if (! ReadOK(fd,&c,1)) {
               return; 
       }
       if (LWZReadByte(fd, TRUE, c) < 0) {
               return;
       }

       /*
       **  If this is an "uninteresting picture" ignore it.
       */
       if (ignore) {
               while (LWZReadByte(fd, FALSE, c) >= 0)
                       ;
               return;
       }

       while ((v = LWZReadByte(fd,FALSE,c)) >= 0 ) {
               /* This how we recognize which colors are actually used. */
               if (im->colors.open[v]) {
                       im->colors.open[v] = 0;
               }
               gdImageSetPixel(im, xpos, ypos, v);
               ++xpos;
               if (xpos == len) {
                       xpos = 0;
                       if (interlace) {
                               switch (pass) {
                               case 0:
                               case 1:
                                       ypos += 8; break;
                               case 2:
                                       ypos += 4; break;
                               case 3:
                                       ypos += 2; break;
                               }

                               if (ypos >= height) {
                                       ++pass;
                                       switch (pass) {
                                       case 1:
                                               ypos = 4; break;
                                       case 2:
                                               ypos = 2; break;
                                       case 3:
                                               ypos = 1; break;
                                       default:
                                               goto fini;
                                       }
                               }
                       } else {
                               ++ypos;
                       }
               }
               if (ypos >= height)
                       break;
       }

fini:
       if (LWZReadByte(fd,FALSE,c)>=0) {
               /* Ignore extra */
       }
}

static void MemReadImage(gdImagePtr im, unsigned char **fd, int len, int height, unsigned char (*cmap)[256], int interlace, int ignore)
{
       unsigned char   c;      
       int             v;
       int             xpos = 0, ypos = 0, pass = 0;
       int i;
       /* Stash the color map into the image */
       for (i=0; (i<gdMaxColors); i++) {
               im->colors.red[i] = cmap[CM_RED][i];	
               im->colors.green[i] = cmap[CM_GREEN][i];	
               im->colors.blue[i] = cmap[CM_BLUE][i];	
               im->colors.open[i] = 1;
       }
       /* Many (perhaps most) of these colors will remain marked open. */
       im->colors.colorsTotal = gdMaxColors;
       /*
       **  Initialize the Compression routines
       */
       if (! MemRead(fd,&c,1)) {
               return; 
       }
       if (MemLWZReadByte(fd, TRUE, c) < 0) {
               return;
       }

       /*
       **  If this is an "uninteresting picture" ignore it.
       */
       if (ignore) {
               while (MemLWZReadByte(fd, FALSE, c) >= 0)
                       ;
               return;
       }

       while ((v = MemLWZReadByte(fd,FALSE,c)) >= 0 ) {
               /* This how we recognize which colors are actually used. */
               if (im->colors.open[v]) {
                       im->colors.open[v] = 0;
               }
               gdImageSetPixel(im, xpos, ypos, v);
               ++xpos;
               if (xpos == len) {
                       xpos = 0;
                       if (interlace) {
                               switch (pass) {
                               case 0:
                               case 1:
                                       ypos += 8; break;
                               case 2:
                                       ypos += 4; break;
                               case 3:
                                       ypos += 2; break;
                               }

                               if (ypos >= height) {
                                       ++pass;
                                       switch (pass) {
                                       case 1:
                                               ypos = 4; break;
                                       case 2:
                                               ypos = 2; break;
                                       case 3:
                                               ypos = 1; break;
                                       default:
                                               goto fini2;
                                       }
                               }
                       } else {
                               ++ypos;
                       }
               }
               if (ypos >= height)
                       break;
       }

fini2:
       if (MemLWZReadByte(fd,FALSE,c)>=0) {
               /* Ignore extra */
       }
}



void gdImageRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	gdImageQuickLine(im, x1, y1, x2, y1, color);		
	gdImageQuickLine(im, x1, y2, x2, y2, color);		
	gdImageQuickLine(im, x1, y1, x1, y2, color);
	gdImageQuickLine(im, x2, y1, x2, y2, color);
}

void gdImageFilledRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	int x, y;

	if (!im)
		return;

	for (y=y1; (y<=y2); y++) {
		for (x=x1; (x<=x2); x++) {
			gdImageQuickSetPixel(im, x, y, color);
		}
	}
}

/*---------------------------------------------------------------


-----------------------------------------------------------------*/

void gdImageCopy(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h)
{
	int c;
	int x, y;
	int tox, toy;
	int i;
	int colorMap[gdMaxColors];
	for (i=0; (i<gdMaxColors); i++) {
		colorMap[i] = (-1);
	}
	toy = dstY;
	for (y=srcY; (y < (srcY + h)); y++) {
		tox = dstX;
		for (x=srcX; (x < (srcX + w)); x++) {
			int nc;
			c = gdImageGetPixel(src, x, y);
			/* Added 7/24/95: support transparent copies */
			if (gdImageGetTransparent(src) == c) {
				tox++;
				continue;
			}
			/* Have we established a mapping for this color? */
			if (colorMap[c] == (-1)) {
				/* If it's the same image, mapping is trivial */
				if (dst == src) {
					nc = c;
				} else { 
					/* First look for an exact match */
					nc = gdImageColorExact(dst,
						src->colors.red[c], src->colors.green[c],
						src->colors.blue[c]);
				}	
				if (nc == (-1)) {
					/* No, so try to allocate it */
					nc = gdImageColorAllocate(dst,
						src->colors.red[c], src->colors.green[c],
						src->colors.blue[c]);
					/* If we're out of colors, go for the
						closest color */
					if (nc == (-1)) {
						nc = gdImageColorClosest(dst,
							src->colors.red[c], src->colors.green[c],
							src->colors.blue[c]);
					}
				}
				colorMap[c] = nc;
			}
			gdImageSetPixel(dst, tox, toy, colorMap[c]);
			tox++;
		}
		toy++;
	}
}			

void gdImageCopyResized(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int dstW, int dstH, int srcW, int srcH)
{
	int c;
	int x, y;
	int tox, toy;
	int ydest;
	int i;
	int colorMap[gdMaxColors];
	/* Stretch vectors */
	int *stx;
	int *sty;
	/* We only need to use floating point to determine the correct
		stretch vector for one line's worth. */
	double accum;
	stx = (int *) malloc(sizeof(int) * srcW);
	sty = (int *) malloc(sizeof(int) * srcH);
	accum = 0;
	for (i=0; (i < srcW); i++) {
		int got;
		accum += ((double)dstW/(double)srcW);
		got = (int)floor(accum);
		stx[i] = got;
		accum -= got;
	}
	accum = 0;
	for (i=0; (i < srcH); i++) {
		int got;
		accum += ((double)dstH/(double)srcH);
		got = (int)floor(accum);
		sty[i] = got;
		accum -= got;
	}
	for (i=0; (i<gdMaxColors); i++) {
		colorMap[i] = (-1);
	}
	toy = dstY;
	for (y=srcY; (y < (srcY + srcH)); y++) {
		for (ydest=0; (ydest < sty[y-srcY]); ydest++) {
			tox = dstX;
			for (x=srcX; (x < (srcX + srcW)); x++) {
				int nc;
				if (!stx[x - srcX]) {
					continue;
				}
				c = gdImageGetPixel(src, x, y);
				/* Added 7/24/95: support transparent copies */
				if (gdImageGetTransparent(src) == c) {
					tox += stx[x-srcX];
					continue;
				}
				/* Have we established a mapping for this color? */
				if (colorMap[c] == (-1)) {
					/* If it's the same image, mapping is trivial */
					if (dst == src) {
						nc = c;
					} else { 
						/* First look for an exact match */
						nc = gdImageColorExact(dst,
							src->colors.red[c], src->colors.green[c],
							src->colors.blue[c]);
					}	
					if (nc == (-1)) {
						/* No, so try to allocate it */
						nc = gdImageColorAllocate(dst,
							src->colors.red[c], src->colors.green[c],
							src->colors.blue[c]);
						/* If we're out of colors, go for the
							closest color */
						if (nc == (-1)) {
							nc = gdImageColorClosest(dst,
								src->colors.red[c], src->colors.green[c],
								src->colors.blue[c]);
						}
					}
					colorMap[c] = nc;
				}
				for (i=0; (i < stx[x - srcX]); i++) {
					gdImageSetPixel(dst, tox, toy, colorMap[c]);
					tox++;
				}
			}
			toy++;
		}
	}
	free(stx);
	free(sty);
}

int gdGetWord(int *result, FILE *in)
{
	int r;
	r = getc(in);
	if (r == EOF) {
		return 0;
	}
	*result = r << 8;
	r = getc(in);	
	if (r == EOF) {
		return 0;
	}
	*result += r;
	return 1;
}

void gdPutWord(int w, FILE *out)
{
	putc((unsigned char)(w >> 8), out);
	putc((unsigned char)(w & 0xFF), out);
}

int gdGetByte(int *result, FILE *in)
{
	int r;
	r = getc(in);
	if (r == EOF) {
		return 0;
	}
	*result = r;
	return 1;
}

gdImagePtr gdImageCreateFromGd(FILE *in)
{
	int sx, sy;
	int x, y;
	int i;
	gdImagePtr im;
	if (!gdGetWord(&sx, in)) {
		goto fail1;
	}
	if (!gdGetWord(&sy, in)) {
		goto fail1;
	}
	im = gdImageCreate(sx, sy,0);
	if (!gdGetByte(&im->colors.colorsTotal, in)) {
		goto fail2;
	}
	if (!gdGetWord(&im->transparent, in)) {
		goto fail2;
	}
	if (im->transparent == 257) {
		im->transparent = (-1);
	}
	for (i=0; (i<gdMaxColors); i++) {
		if (!gdGetByte(&im->colors.red[i], in)) {
			goto fail2;
		}
		if (!gdGetByte(&im->colors.green[i], in)) {
			goto fail2;
		}
		if (!gdGetByte(&im->colors.blue[i], in)) {
			goto fail2;
		}
	}	
	for (y=0; (y<sy); y++) {
		for (x=0; (x<sx); x++) {	
			int ch;
			ch = getc(in);
			if (ch == EOF) {
				gdImageDestroy(im);
				return 0;
			}
			im->pixels[y][x] = ch;
		}
	}
	return im;
fail2:
	gdImageDestroy(im);
fail1:
	return 0;
}
	
void gdImageGd(gdImagePtr im, FILE *out)
{
	int x, y;
	int i;
	int trans;
	gdPutWord(im->sx, out);
	gdPutWord(im->sy, out);
	putc((unsigned char)im->colors.colorsTotal, out);
	trans = im->transparent;
	if (trans == (-1)) {
		trans = 257;
	}	
	gdPutWord(trans, out);
	for (i=0; (i<gdMaxColors); i++) {
		putc((unsigned char)im->colors.red[i], out);
		putc((unsigned char)im->colors.green[i], out);	
		putc((unsigned char)im->colors.blue[i], out);	
	}
	for (y=0; (y < im->sy); y++) {	
		for (x=0; (x < im->sx); x++) {	
			putc((unsigned char)im->pixels[y][x], out);
		}
	}
}


gdImagePtr gdImageCreateFromXbm(FILE *fd)
{
	gdImagePtr im;	
	int bit;
	int w, h;
	int bytes;
	int ch;
	int i, x, y;
	char *sp;
	char s[161];
	if (!fgets(s, 160, fd)) {
		return 0;
	}
	sp = &s[0];
	/* Skip #define */
	sp = mystrchr(sp, ' ');
	if (!sp) {
		return 0;
	}
	/* Skip width label */
	sp++;
	sp = mystrchr(sp, ' ');
	if (!sp) {
		return 0;
	}
	/* Get width */
	w = atoi(sp + 1);
	if (!w) {
		return 0;
	}
	if (!fgets(s, 160, fd)) {
		return 0;
	}
	sp = s;
	/* Skip #define */
	sp = mystrchr(sp, ' ');
	if (!sp) {
		return 0;
	}
	/* Skip height label */
	sp++;
	sp = mystrchr(sp, ' ');
	if (!sp) {
		return 0;
	}
	/* Get height */
	h = atoi(sp + 1);
	if (!h) {
		return 0;
	}
	/* Skip declaration line */
	if (!fgets(s, 160, fd)) {
		return 0;
	}
	bytes = (w * h / 8) + 1;
	im = gdImageCreate(w, h,0);
	gdImageColorAllocate(im, 255, 255, 255);
	gdImageColorAllocate(im, 0, 0, 0);
	x = 0;
	y = 0;
	for (i=0; (i < bytes); i++) {
		char h[3];
		int b;
		/* Skip spaces, commas, CRs, 0x */
		while(1) {
			ch = getc(fd);
			if (ch == EOF) {
				goto fail;
			}
			if (ch == 'x') {
				break;
			}	
		}
		/* Get hex value */
		ch = getc(fd);
		if (ch == EOF) {
			goto fail;
		}
		h[0] = ch;
		ch = getc(fd);
		if (ch == EOF) {
			goto fail;
		}
		h[1] = ch;
		h[2] = '\0';
		sscanf(h, "%x", &b);		
		for (bit = 1; (bit <= 128); (bit = bit << 1)) {
			gdImageSetPixel(im, x++, y, (b & bit) ? 1 : 0);	
			if (x == im->sx) {
				x = 0;
				y++;
				if (y == im->sy) {
					return im;
				}
				/* Fix 8/8/95 */
				break;
			}
		}
	}
	/* Shouldn't happen */
	fprintf(stderr, "Error: bug in ImageCreateFromXbm()!\n");
	return 0;
fail:
	gdImageDestroy(im);
	return 0;
}

void gdImagePolygon(gdImagePtr im, gdPointPtr p, int n, int c)
{
	long i, x,y, lx, ly;

	if (n==0 || !im || !p ) {
		return;
	}
	lx = (long)p->x;
	ly = (long)p->y;

	// last point
	x = (long)p[n-1].x;
	y = (long)p[n-1].y;
//printf( "gdImagePolygon1(%d): lx=%d..ly=%d, x=%d, y=%d n=%d\n", __LINE__, lx, ly, x, y, n );	
	
	gdImageQuickLine(im, lx, ly, x, y, c);
	for (i=1; (i < n); i++) {
		p++;
		x = (long)p->x;
		y = (long)p->y;
//printf( "gdImagePolygon1(%d): lx=%d..ly=%d, x=%d, y=%d, n=%d\n", __LINE__, lx, ly, x, y, n );	
		gdImageQuickLine(im, lx, ly, x, y, c);
		lx = x;
		ly = y;
	}
}	
	
int gdCompareInt(const void *a, const void *b)
{
	return (*(const int *)a) - (*(const int *)b);
}

#ifdef DEF_MAC
int gdCompareFloat(const void *a, const void *b)

{

	return (*(const float *)a) - (*(const float *)b);

}
#else
int gdCompareFloat(const float *a, const float *b)
{

	return (int)((*a) - (*b));
}
#endif

#define	SHOWPOLY(pol,count) {int ii;for(ii=0;ii<count;ii++) printf( "POINT %d = (%f)\n", ii,pol[ii] );}
	
void gdImageFilledPolygon(gdImagePtr im, gdPointPtr p, int n, int c)
{
	int i;
	int y;
	int y1, y2;
	int ints;
	if (!n) {
		return;
	}
	if (!im->polyAllocated) {
		im->polyInts = (float *) malloc(sizeof(gdPoint) * n * 2);
		im->polyAllocated = n;
	}		
	if (im->polyAllocated < n) {
		while (im->polyAllocated < n) {
			im->polyAllocated *= 2;
		}	
		im->polyInts = (float *) realloc(im->polyInts, sizeof(gdPoint) * im->polyAllocated);
	}
	y1 = (int)p[0].y;
	y2 = (int)p[0].y;

	for (i=1; (i < n); i++) {
		if (p[i].y < y1) {
			y1 = (int)p[i].y;
		}
		if (p[i].y > y2) {
			y2 = (int)p[i].y;
		}
	}
	if ( y1<0 ) y1 = 0;
	if ( y2<0 ) y2 = 0;

	for (y=y1; (y < y2); y++) {
		int interLast = 0;
		int dirLast = 0;
		int interFirst = 1;
		ints = 0;

		for (i=0; (i <= n); i++) {
			int x1, x2;
			int y1, y2;
			int dir;
			int ind1, ind2;
			int lastInd1 = 0;


			if ((i == n) || (!i)) {
				ind1 = n-1;
				ind2 = 0;
			} else {
				ind1 = i-1;
				ind2 = i;
			}
			y1 = (int)p[ind1].y;
			y2 = (int)p[ind2].y;
			if (y1 < y2) {
				y1 = (int)p[ind1].y;
				y2 = (int)p[ind2].y;
				x1 = (int)p[ind1].x;
				x2 = (int)p[ind2].x;
				dir = -1;
			} else if (y1 > y2) {
				y2 = (int)p[ind1].y;
				y1 = (int)p[ind2].y;
				x2 = (int)p[ind1].x;
				x1 = (int)p[ind2].x;
				dir = 1;
			} else {
				/* Horizontal; just draw it */
				if ( y1>=0 && y1>=0 && ind2<n ){
					gdImageQuickLine(im, (int)p[ind1].x, y1, (int)p[ind2].x, y1, c);
				}
				continue;
			}
			if ((y >= y1) && (y <= y2)) {
				int inter = (y-y1) * (x2-x1) / (y2-y1) + x1;
				/* Only count intersections once
					except at maxima and minima. Also, 
					if two consecutive intersections are
					endpoints of the same horizontal line
					that is not at a maxima or minima,	
					discard the leftmost of the two. */
				if (!interFirst) {
					if ((p[ind1].y == p[lastInd1].y) &&
						(p[ind1].x != p[lastInd1].x)) {
						if (dir == dirLast) {
							if (inter > interLast) {
								/* Replace the old one */
								im->polyInts[ints] = (float)inter;
							} else {
								/* Discard this one */
							}	
							continue;
						}
					}
					if (inter == interLast) {
						if (dir == dirLast) {
							continue;
						}
					}
				} 
				if (i > 0) {
					im->polyInts[ints++] = (float)inter;
				}
				lastInd1 = i;
				dirLast = dir;
				interLast = inter;
				interFirst = 0;
			}
		}
		// qsort(im->polyInts, ints, sizeof(float), gdCompareFloat);
		qsort((void *)im->polyInts, ints, sizeof(float), gdCompareFloat);


		for (i=0; (i < (ints-1)); i+=2) {
			long x1,x2;

			x1 = (long)im->polyInts[i];
			x2 = (long)im->polyInts[i+1];
			if( x2>=0 && x2<9999 ){
				gdImageQuickLine(im, x1, y, x2, y, c );
			}
		}
	}
}
	

void gdImageSetStyle(gdImagePtr im, int *style, int noOfPixels)
{
	if (im->style) {
		free(im->style);
	}
	im->style = (int *) 
		malloc(sizeof(int) * noOfPixels);
	memcpy(im->style, style, sizeof(int) * noOfPixels);
	im->styleLength = noOfPixels;
	im->stylePos = 0;
}

void gdImageSetBrush(gdImagePtr im, gdImagePtr brush)
{
	int i;
	im->brush = brush;
	for (i=0; (i < gdImageColorsTotal(brush)); i++) {
		int index;
		index = gdImageColorExact(im, 
			gdImageRed(brush, i),
			gdImageGreen(brush, i),
			gdImageBlue(brush, i));
		if (index == (-1)) {
			index = gdImageColorAllocate(im,
				gdImageRed(brush, i),
				gdImageGreen(brush, i),
				gdImageBlue(brush, i));
			if (index == (-1)) {
				index = gdImageColorClosest(im,
					gdImageRed(brush, i),
					gdImageGreen(brush, i),
					gdImageBlue(brush, i));
			}
		}
		im->brushColorMap[i] = index;
	}
}
	
void gdImageSetTile(gdImagePtr im, gdImagePtr tile)
{
	int i;
	im->tile = tile;
	for (i=0; (i < gdImageColorsTotal(tile)); i++) {
		int index;
		index = gdImageColorExact(im, 
			gdImageRed(tile, i),
			gdImageGreen(tile, i),
			gdImageBlue(tile, i));
		if (index == (-1)) {
			index = gdImageColorAllocate(im,
				gdImageRed(tile, i),
				gdImageGreen(tile, i),
				gdImageBlue(tile, i));
			if (index == (-1)) {
				index = gdImageColorClosest(im,
					gdImageRed(tile, i),
					gdImageGreen(tile, i),
					gdImageBlue(tile, i));
			}
		}
		im->tileColorMap[i] = index;
	}
}

void gdImageInterlace(gdImagePtr im, int interlaceArg)
{
	im->interlace = interlaceArg;
}

/* ----------------------------------------------

	load BMP into ram struct

	(c) raul Sobon add on

------------------------------------------------*/
gdImagePtr gdImageCreateFromBmpRam( void *bmp )
{
	gdImagePtr im;	
	BMPptr		mybmp;
	unsigned	char *data;
	int w, h, col;
	int bytes;
	int i, x, y;
	
	mybmp = (BMPptr) bmp;
	data = (unsigned	char *) bmp;
	memcpy( &w, data+14+4, 4 );			/*	w = mybmp->bmih.biWidth;  */
	memcpy( &h, data+14+4+4, 4 );		/*	h = mybmp->bmih.biHeight; */
#ifdef DEF_MAC
	w = swapl(w);	h = swapl(h);
#endif
	data = (unsigned char *)mybmp + TOTAL_HEADER_SIZE;
	
	bytes = (w * h ) + 1;
	im = gdImageCreate(w, h,0);
	gdImageColorAllocate(im, 255, 255, 255);
	gdImageColorAllocate(im, 0, 0, 0);

	i = 0;
	for( y=h-1; y>=0; y-- ){
		for( x=0; x<w; x++ ){
			col = data[i++];
			gdImageSetPixel(im, x, y, col );
		}
	}		

	return im;

	gdImageDestroy(im);
	return 0;
}
 
/*------------------------------------------------------

	Various graphics drawing funcs like axis and 3dboxes

-------------------------------------------------------*/
/*
 * draw 3d isometric view box
 */
void gd3dHorizBox( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3, int depth )
{
	int x2,y2,x3,x4,lp;
	
	if( ys < 0 || y < 0) return;

	if ( xs > im->sx ) xs = im->sx;
	if ( ys > im->sy ) ys = im->sy;
	if ( xs < 0 ) xs=0;

	ys -= depth;
	y += depth;

	x2 = x + xs;
	y2 = y + ys;
	gdImageFilledRectangle(im, x, y+1, x2-1, y2-1, c2);		/* middle */

	for(lp=0; lp<depth; lp++) {
		x3 = x+lp;
		x4 = x2+lp;
		if ( x3 < x ) x3 = x;
		if ( x4 < x ) x4 = x;
		if ( c1 >= 0 )
		gdImageQuickLine(im, x3, y-lp,  x4, y-lp, c1 );  			/* draw X-axis */
		if ( c3 >= 0 )
		gdImageQuickLine(im, x4, y-lp,  x4, y2-lp-1, c3 );  			/* draw Y-axis */
	}
}

/*
 draw 3d isometric view box
 */
void gd3dVertBox( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3, int depth )
{
	int x2,y2,x3,y3,y4,lp;
	
	if( ys < 0 || y < 0) return;

	if ( xs > im->sx ) xs = im->sx;
	if ( ys > im->sy ) ys = im->sy;
	if ( xs < 0 ) xs=0;

	x2 = x + xs;
	y2 = y + ys;
	gdImageFilledRectangle(im, x, y+1, x2-1, y2-1, c2);		/* middle */

	for( lp=0; lp<depth; lp++){
		x3 = x2 + lp;
		y3 = y - lp;
		y4 = y3 + ys;
		if ( c1 >= 0 )
		gdImageQuickLine(im, x+lp, y3,   x3, y3, c1 );  		/* draw X-axis  */
		if ( c3 >= 0 )
		gdImageQuickLine(im, x3, y3,  x3, y4, c3 );  			/* draw Y-axis  */
	}
}





static	long coolbarcolors[32];

void gd3dHorizGradBox( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3, int depth )
{
	int x2,y2,x3,x4,lp;
	
	if( ys < 0 ||  y < 0) return;

	if ( xs > im->sx ) xs = im->sx;
	if ( ys > im->sy ) ys = im->sy;
	if ( xs < 0 ) xs=0;

	ys -= depth;
	y += depth;

	x2 = x + xs;
	y2 = y + ys;

	
	{
		long gradLevel = 20, lastcol, count;
		long ysize=ys, y2,y3,y4;
		c2 = gdImageColorGetRGB( im, c2 );
		y4 = 0;
		for( count=0; count<gradLevel; count++){
			long newcolor; double scale;
			scale = ((count/(double)gradLevel)-0.5) * 30;
			newcolor = Scale_RGB( 100-scale, c2 );
			y2 = y+(count*ysize/gradLevel);
			y3 = y+(((1+count)*ysize)/gradLevel);
			if ( y2>y4 ){
				coolbarcolors[count] = gdImageColorAllocate(im, RGB_RED(newcolor), RGB_GREEN(newcolor), RGB_BLUE(newcolor) );
				lastcol = coolbarcolors[count];
				gdImageFilledRectangle(im, x, y2, x2, y3, lastcol );
				y4 = y2;
			}
		}
		gdImageFilledRectangle(im, x, y+((count*ysize)/gradLevel), x2, y+ysize, lastcol );
	}

	for(lp=0; lp<depth; lp++) {
		x3 = x+lp;
		x4 = x2+lp;
		if ( x3 < x ) x3 = x;
		if ( x4 < x ) x4 = x;
		if ( c1 >= 0 )
		gdImageQuickLine(im, x3, y-lp,  x4, y-lp, c1 );  			/* draw X-axis */
		if ( c3 >= 0 )
		gdImageQuickLine(im, x4, y-lp,  x4, y2-lp, c3 );  			/* draw Y-axis */
	}
}

/*
 draw 3d isometric view box
 */
void gd3dVertGradBox( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3, int depth )
{
	int x2,y2,x3,y3,y4,lp;
	
	if( ys < 0 || ys > 1000 || y < 0) return;

	if ( xs > im->sx ) xs = im->sx;
	if ( ys > im->sy ) ys = im->sy;

	x2 = x + xs;
	y2 = y + ys;

	{
		long gradLevel = 32, lastcol, count;
		long ysize=ys;
		c2 = gdImageColorGetRGB( im, c2 );

		y4 = 0;
		for( count=0; count<gradLevel; count++){
			long newcolor; double scale;
			scale = ((count/(double)gradLevel)-0.5) * 30;
			newcolor = Scale_RGB( 100-scale, c2 );
			y2 = y+(count*ysize/gradLevel);
			y3 = y+(((1+count)*ysize)/gradLevel);
			if ( y2>y4 ){
				coolbarcolors[count] = gdImageColorAllocate(im, RGB_RED(newcolor), RGB_GREEN(newcolor), RGB_BLUE(newcolor) );
				lastcol = coolbarcolors[count];
				gdImageFilledRectangle(im, x, y2, x2, y3, lastcol );
				y4 = y2;
			}
		}
		gdImageFilledRectangle(im, x, y+((count*ysize)/gradLevel), x2, y+ysize, lastcol );
	}

	for( lp=0; lp<depth; lp++){
		x3 = x2 + lp + 1;
		y3 = y - lp;
		y4 = y3 + ys;
		if ( c1 >= 0 )
		gdImageQuickLine(im, x+lp, y3,   x3, y3, c1 );  		/* draw X-axis  */
		if ( c3 >= 0 )
		gdImageQuickLine(im, x3, y3,  x3, y4, c3 );  			/* draw Y-axis  */
	}
}






/*
 draw pie graph

 im, centre,centre,radius,deg1,deg2,pie color
 */
void gdPieGraph(gdImagePtr im, int cx, int cy, int rad, int a1, int a2, int color)
{
	gdImageFilledPie(im, cx, cy, rad*2, rad*2,  a1, a2, color);
}

void gd3DPieGraph(gdImagePtr im, int cx, int cy, int rad, int a1, int a2, int color, int color2 )
{
	gdImageFilled3DPie(im, cx, cy, rad*2, rad,  a1, a2, color, color2 );
}

/*
 draw bevel with 1 pixel thick border
 */
void gdBevel( gdImagePtr im, int x, int y, int xs, int ys, int c1, int c2, int c3 )
{
	int x2,y2;

	if (!im)
		return;
	
	if ( xs > im->sx ) xs = im->sx;
	if ( ys > im->sy ) ys = im->sy;

	x2 = x + (xs-1);
	y2 = y + (ys-1);

	gdImageFilledRectangle(im, x, y, x2, y2, c3);			/* dark */
	gdImageFilledRectangle(im, x, y, x2-1, y2-1, c1);		/* bright */
	gdImageFilledRectangle(im, x+1, y+1, x2-1, y2-1, c2);	/* middle */
}

/*-----------------------------------------------------------------------
 *
 * miGIF Compression - mouse and ivo's GIF-compatible compression
 *
 *          -run length encoding compression routines-
 *
 * Copyright (C) 1998 Hutchison Avenue Software Corporation
 *               http://www.hasc.com
 *               info@hasc.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "AS IS." The Hutchison Avenue 
 * Software Corporation disclaims all warranties, either express or implied, 
 * including but not limited to implied warranties of merchantability and 
 * fitness for a particular purpose, with respect to this code and accompanying
 * documentation. 
 * 
 * The miGIF compression routines do not, strictly speaking, generate files 
 * conforming to the GIF spec, since the image data is not LZW-compressed 
 * (this is the point: in order to avoid transgression of the Unisys patent 
 * on the LZW algorithm.)  However, miGIF generates data streams that any 
 * reasonably sane LZW decompresser will decompress to what we want.
 *
 * miGIF compression uses run length encoding. It compresses horizontal runs 
 * of pixels of the same color. This type of compression gives good results
 * on images with many runs, for example images with lines, text and solid 
 * shapes on a solid-colored background. It gives little or no compression 
 * on images with few runs, for example digital or scanned photos.
 *
 *                               der Mouse
 *                      mouse@rodents.montreal.qc.ca
 *            7D C8 61 52 5D E7 2D 39  4E F1 31 3E E8 B3 27 4B
 *
 *                             ivo@hasc.com
 *
 * The Graphics Interchange Format(c) is the Copyright property of
 * CompuServe Incorporated.  GIF(sm) is a Service Mark property of
 * CompuServe Incorporated.
 *
 */

static int rl_pixel;
static int rl_basecode;
static int rl_count;
static int rl_table_pixel;
static int rl_table_max;
static int just_cleared;
static int out_bits;
static int out_bits_init;
static int out_bump;
static int out_bump_init;
static int out_clear;
static int out_clear_init;
static int max_ocodes;
static int code_clear;
static int code_eof;
static unsigned int obuf;
static int obits;
static FILE *ofile;
static unsigned char oblock[256];
static int oblen;

#define VERBOSE 0

static const char *binformat(unsigned int v, int nbits)
{
 static char bufs[8][64];
 static int bhand = 0;
 unsigned int bit;
 int bno;
 char *bp;
 
 bhand --;

 if (bhand < 0) bhand = (sizeof(bufs)/sizeof(bufs[0]))-1;

 bp = &bufs[bhand][0];

 for (bno=nbits-1,bit=1U<<bno;bno>=0;bno--,bit>>=1)
  { *bp++ = (v & bit) ? '1' : '0';
    if (((bno&3) == 0) && (bno != 0)) *bp++ = '.';
  }
 *bp = '\0';

 return(&bufs[bhand][0]);
}

static void write_block(void)
{
 fputc(oblen,ofile);
 if ( oblen )
  fwrite(&oblock[0],1,oblen,ofile);
 oblen = 0;
}

static void block_out(unsigned char c)
{
 //if (VERBOSE) printf("block_out %s\n",binformat(c,8));

 oblock[oblen++] = c;

 if (oblen >= 255) write_block();

}

static void block_flush(void)
{
 //if (VERBOSE) printf("block_flush\n");
 if (oblen > 0) write_block();
}

static void block_output(unsigned int val)
{
 //if (VERBOSE) printf("output %s [%s %d %d]\n",binformat(val,out_bits),binformat(obuf,obits),obits,out_bits);

 obuf |= val << obits;
 obits += out_bits;
 while (obits >= 8)
  {
	unsigned char c = (unsigned char)( (unsigned char)(obuf)&(unsigned char)(0xff) );
	block_out(c);
    obuf >>= 8;
    obits -= 8;
  }
 //if (VERBOSE) printf("output leaving [%s %d]\n",binformat(obuf,obits),obits);
}

static void output_flush(void)
{
 //if (VERBOSE) printf("output_flush\n");
 if (obits > 0) block_out((unsigned char)obuf);
 block_flush();
}

static void did_clear(void)
{
 //if (VERBOSE) printf("did_clear\n");
 out_bits = out_bits_init;
 out_bump = out_bump_init;
 out_clear = out_clear_init;
 out_count = 0;
 rl_table_max = 0;
 just_cleared = 1;
}

static void output_plain(int c)
{
 //if (VERBOSE) printf("output_plain %s\n",binformat(c,out_bits));
 just_cleared = 0;
 block_output(c);
 out_count ++;
 if (out_count >= out_bump)
  { out_bits ++;
    out_bump += 1 << (out_bits - 1);
  }

 if (out_count >= out_clear)
  { block_output(code_clear);
    did_clear();
  }

}

static unsigned int isqrt(unsigned int x)
{
 unsigned int r;
 unsigned int v;

 if (x < 2) return(x);
 for (v=x,r=1;v;v>>=2,r<<=1) ;
 while (1)
  { v = ((x / r) + r) / 2;
    if ((v == r) || (v == r+1)) return(r);
    r = v;
  }
}

static unsigned int compute_triangle_count(unsigned int count, unsigned int nrepcodes)
{
 unsigned int perrep;
 unsigned int cost;

 cost = 0;

 perrep = (nrepcodes * (nrepcodes+1)) / 2;
 while (count >= perrep)
  { cost += nrepcodes;
    count -= perrep;
  }

 if (count > 0)
  { unsigned int n;
    n = isqrt(count);
    while ((n*(n+1)) >= 2*count) n --;
    while ((n*(n+1)) < 2*count) n ++;
    cost += n;
  }

 return(cost);

}

static void max_out_clear(void)
{
 out_clear = max_ocodes;
}

static void reset_out_clear(void)
{
 out_clear = out_clear_init;
 if (out_count >= out_clear)
  { block_output(code_clear);
    did_clear();
  }
}

static void rl_flush_fromclear(int count)
{
 int n;

 //if (VERBOSE) printf("rl_flush_fromclear %d\n",count);
 max_out_clear();
 rl_table_pixel = rl_pixel;
 n = 1;
 while (count > 0)
  { if (n == 1)
     { rl_table_max = 1;
       output_plain(rl_pixel);
       count --;
     }
    else if (count >= n)
     { rl_table_max = n;
       output_plain(rl_basecode+n-2);
       count -= n;
     }
    else if (count == 1)
     { rl_table_max ++;
       output_plain(rl_pixel);
       count = 0;
     }

    else
     { rl_table_max ++;
       output_plain(rl_basecode+count-2);
       count = 0;
     }

    if (out_count == 0) n = 1; else n ++;
  }
 reset_out_clear();

 //if (VERBOSE) printf("rl_flush_fromclear leaving table_max=%d\n",rl_table_max);

}

static void rl_flush_clearorrep(int count)
{
 int withclr;

 //if (VERBOSE) printf("rl_flush_clearorrep %d\n",count);
 withclr = 1 + compute_triangle_count(count,max_ocodes);
 if (withclr < count)
  { block_output(code_clear);
    did_clear();
    rl_flush_fromclear(count);
  }
 else
  { for (;count>0;count--) output_plain(rl_pixel);

  }
}

static void rl_flush_withtable(int count)
{
 int repmax;
 int repleft;
 int leftover;

 //if (VERBOSE) printf("rl_flush_withtable %d\n",count);

 repmax = count / rl_table_max;
 leftover = count % rl_table_max;
 repleft = (leftover ? 1 : 0);
 if (out_count+repmax+repleft > max_ocodes)
  { repmax = max_ocodes - out_count;
    leftover = count - (repmax * rl_table_max);
    repleft = 1 + compute_triangle_count(leftover,max_ocodes);
  }

 //if (VERBOSE) printf("rl_flush_withtable repmax=%d leftover=%d repleft=%d\n",repmax,leftover,repleft);

 if (1+compute_triangle_count(count,max_ocodes) < (unsigned int)(repmax+repleft))
  { block_output(code_clear);
    did_clear();
    rl_flush_fromclear(count);
    return;
  }

 max_out_clear();
 for (;repmax>0;repmax--) output_plain(rl_basecode+rl_table_max-2);

 if (leftover)
  { if (just_cleared)
     { rl_flush_fromclear(leftover);
     }
    else if (leftover == 1)
     { output_plain(rl_pixel);
     }
    else
     { output_plain(rl_basecode+leftover-2);
     }
  }
 reset_out_clear();
}

static void rl_flush(void)
{
 //if (VERBOSE) printf("rl_flush [ %d %d\n",rl_count,rl_pixel);

 if (rl_count == 1)
  { output_plain(rl_pixel);
    rl_count = 0;
    //if (VERBOSE) printf("rl_flush ]\n");
    return;

  }
 if (just_cleared)
  { rl_flush_fromclear(rl_count);

  }
 else if ((rl_table_max < 2) || (rl_table_pixel != rl_pixel))
  { rl_flush_clearorrep(rl_count);

  }
 else
  { rl_flush_withtable(rl_count);

  }
 //if (VERBOSE) printf("rl_flush ]\n");
 rl_count = 0;
}

static void compressRLE(int init_bits, FILE *outfile, gdImagePtr im, int background)
{
 int c;

 ofile = outfile;
 obuf = 0;
 obits = 0;
 oblen = 0;
 code_clear = 1 << (init_bits - 1);
 code_eof = code_clear + 1;
 rl_basecode = code_eof + 1;
 out_bump_init = (1 << (init_bits - 1)) - 1;

 /* for images with a lot of runs, making out_clear_init larger will
    give better compression. */ 

 out_clear_init = (init_bits <= 3) ? 9 : (out_bump_init-1);
 out_bits_init = init_bits;
 max_ocodes = (1 << GIFBITS) - ((1 << (out_bits_init - 1)) + 3);
 did_clear();
 block_output(code_clear);
 rl_count = 0;

 while (1)
  { c = GIFNextPixel(im);

    if ((rl_count > 0) && (c != rl_pixel)) rl_flush();
    if (c == EOF) break;
    if (rl_pixel == c)
     { rl_count ++;

     }
    else
     { rl_pixel = c;
       rl_count = 1;
     }

  }
 block_output(code_eof);
 output_flush();
}

void Convert32to24bit( gdImagePtr im, long line, char *rowdata )
{
	long lp, r,g,b, i;
	char *p = rowdata;

	for ( lp = 0; lp < im->sx; lp++ )
	{
		i = gdImageGetPixelRGB( im, lp, line );
		r = RGB_RED(i);
		g = RGB_GREEN(i);
		b = RGB_BLUE(i);
		*(rowdata++) = (char)r;
		*(rowdata++) = (char)g;
		*(rowdata++) = (char)b;
	}
}

void Convert8to24bit( gdImagePtr im, long line, char *rowdata )
{
	long lp, r,g,b, i;
	char *p = rowdata;

	for ( lp = 0; lp < im->sx; lp++ ){
		i = gdImageGetPixel( im, lp, line );
		r = im->colors.red[ i ];
		g = im->colors.green[ i ];
		b = im->colors.blue[ i ];
		*(rowdata++) = (char)r;
		*(rowdata++) = (char)g;
		*(rowdata++) = (char)b;
	}
}


#ifdef USEJPEGLIB
/*    JPEG  saving */
#include "jpeglib.h"
//#include <setjmp.h>
#endif

void gdImageJPG( gdImagePtr im, FILE *outfile, int quality)
{
	char *rowdata;

#ifdef USEJPEGLIB
	/* This struct contains the JPEG compression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	* It is possible to have several such structures, representing multiple
	* compression/decompression processes, in existence at once.  We refer
	* to any one struct (and its associated working data) as a "JPEG object".
	*/
	struct jpeg_compress_struct cinfo;
	
	/* This struct represents a JPEG error handler.  It is declared separately
	* because applications often want to supply a specialized error handler
	* (see the second half of this file for an example).  But here we just
	* take the easy way out and use the standard error handler, which will
	* print a message on stderr and call exit() if compression fails.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct jpeg_error_mgr jerr;

	/* More stuff */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	/* Step 1: allocate and initialize JPEG compression object */
	/* We have to set up the error handler first, in case the initialization
	* step fails.  (Unlikely, but it could happen if you are out of memory.)
	* This routine fills in the contents of struct jerr, and returns jerr's
	* address which we place into the link field in cinfo.
	*/

	cinfo.err = jpeg_std_error(&jerr);

	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */
	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/

	cinfo.image_width = im->sx; 	/* image width and height, in pixels */
	cinfo.image_height = im->sy;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/

	jpeg_set_defaults(&cinfo);

	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */
	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */
	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = im->sx * 3;	/* JSAMPLEs per row in image_buffer */
	rowdata = (char *)malloc( row_stride );
	
	if ( rowdata )
	{
		while (cinfo.next_scanline < cinfo.image_height) 
		{
			/* jpeg_write_scanlines expects an array of pointers to scanlines.
			 * Here the array is only one element long, but you could pass
			 * more than one scanline at a time if that's more convenient.
			 */
			if( im->truecolor == 1 )
				Convert32to24bit( im, cinfo.next_scanline, rowdata );
			else
				Convert8to24bit( im, cinfo.next_scanline, rowdata );

			row_pointer[0] = (unsigned char *)rowdata;
			(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}
		free( rowdata );
	}

	/* Step 6: Finish compression */
	jpeg_finish_compress(&cinfo);

	/* After finish_compress, we can close the output file. */
	/* Step 7: release JPEG compression object */
	/* This is an important step since it will release a good deal of memory. */

	jpeg_destroy_compress(&cinfo);
	/* And we're done! */
#endif
}

/* write a png file */
#ifdef USEPNG
#include "png.h"
#endif


void gdImagePNG_old( gdImagePtr im, FILE *fp )
{
#ifdef USEPNGOLD
	long i;
	png_structp png_ptr;
	png_infop info_ptr;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED.
	*/

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if (png_ptr == NULL){
	  fclose(fp);
	  return;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL){
	  fclose(fp);
	  png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
	  return;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error hadnling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_ptr->jmpbuf)){
	  /* If we get here, we had a problem reading the file */
	  fclose(fp);
	  png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
	  return;
	}

	png_init_io(png_ptr, fp);

	/* Set the image information here.  Width and height are up to 2^31,
	* bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	* the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	* PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	* or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	* PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	* currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
	*/
	{
		png_colorp palette;

		png_set_IHDR(png_ptr, info_ptr, im->sx, im->sy, 8, PNG_COLOR_TYPE_PALETTE,
		  PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		/* set the palette if there is one.  REQUIRED for indexed-color images */
		palette = (png_colorp)png_malloc(png_ptr, 256 * sizeof (png_color));
		/* ... set palette colors ... */
		for ( i=0; i<256; i++){
			palette[i].red = im->colors.red[i];
			palette[i].green = im->colors.green[i];
			palette[i].blue = im->colors.blue[i];
		}
		png_set_PLTE(png_ptr, info_ptr, palette, 256);
	}

	/* Optional gamma chunk is strongly suggested if you have any guess
	* as to the correct gamma of the image.
	*/
	//png_set_gAMA(png_ptr, info_ptr, 2.1 );

	{	png_text  text[3];
		png_textp text_ptr;

		text_ptr = &text[0];
		/* Optionally write comments into the image */
		text_ptr[0].key = "Title";
		text_ptr[0].text = "Image";
		text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[1].key = "Author";
		text_ptr[1].text = "Bob";
		text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[2].key = "Description";
		text_ptr[2].text = "graph image data";
		text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
		png_set_text(png_ptr, info_ptr, text_ptr, 3);
	}

	/* other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs, */
	/* note that if sRGB is present the cHRM chunk must be ignored
	* on read and must be written in accordance with the sRGB profile */
	/* Write the file header information.  REQUIRED */

	png_write_info(png_ptr, info_ptr);

	/* Once we write out the header, the compression type on the text
	* chunks gets changed to PNG_TEXT_COMPRESSION_NONE_WR or
	* PNG_TEXT_COMPRESSION_zTXt_WR, so it doesn't get written out again
	* at the end.
	*/

	/* set up the transformations you want.  Note that these are
	* all optional.  Only call them if you want them.
	*/

	/* pack pixels into bytes */
	//png_set_packing(png_ptr);

	/* swap location of alpha bytes from ARGB to RGBA */
	//png_set_swap_alpha(png_ptr);

	/* Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
	* RGB (4 channels -> 3 channels). The second parameter is not used.
	*/
	//png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE);

	/* flip BGR pixels to RGB */
	//png_set_bgr(png_ptr);

	/* swap bytes of 16-bit files to most significant byte first */
	//png_set_swap(png_ptr);

	/* swap bits of 1, 2, 4 bit packed pixel formats */
	//png_set_packswap(png_ptr);
	{	long number_passes, pass,y;
		/* turn on interlace handling if you are not using png_write_image() */
		number_passes = png_set_interlace_handling(png_ptr);

		/* The number of passes is either 1 for non-interlaced images,
		* or 7 for interlaced images.
		*/
		for (pass = 0; pass < number_passes; pass++){

		  png_byte *row_pointers[2];

		  /* If you are only writing one row at a time, this works */
		  for (y = 0; y < im->sy; y++)  {
			row_pointers[0] = &im->pixels[y][0];
			png_write_rows(png_ptr, row_pointers, 1);
		  }

		}

	}

	/* You can write optional chunks like tEXt, zTXt, and tIME at the end
	* as well.
	*/
	/* It is REQUIRED to call this to finish writing the rest of the file */

	png_write_end(png_ptr, info_ptr);
	/* if you malloced the palette, free it here */

	free(info_ptr->palette);

	/* if you allocated any text comments, free them here */
	/* clean up after the write, and free any memory allocated */

	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

	/* close the file */
	fclose(fp);
	/* that's it */
	return;
#endif
}

#ifdef USEPNGLIB
//#include <setjmp.h>
#endif

void gdImagePNG( gdImagePtr im, FILE *fp )
{
#ifdef USEPNGLIB
	long number_passes,pass,y,i;
	png_structp png_ptr;
	png_infop info_ptr;
    png_colorp palette;
	
	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED.
	*/

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if (png_ptr == NULL){
	  return;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL){
	  png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
	  return;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error hadnling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png_ptr))){
	  /* If we get here, we had a problem reading the file */
	  png_destroy_write_struct(&png_ptr,&info_ptr);
	  return;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, im->sx, im->sy, 8, PNG_COLOR_TYPE_PALETTE,
	  PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* set the palette if there is one.  REQUIRED for indexed-color images */
	palette = (png_colorp)png_malloc(png_ptr, 256 * sizeof (png_color));
	/* ... set palette colors ... */
	for ( i=0; i<256; i++){
		palette[i].red = im->colors.red[i];
		palette[i].green = im->colors.green[i];
		palette[i].blue = im->colors.blue[i];
	}
	png_set_PLTE(png_ptr, info_ptr, palette, 256);

	/* Optional gamma chunk is strongly suggested if you have any guess
	* as to the correct gamma of the image.
	*/
	//png_set_gAMA(png_ptr, info_ptr, 2.1 );

	{	png_text  text[3];
		png_textp text_ptr;

		text_ptr = &text[0];
		/* Optionally write comments into the image */
		text_ptr[0].key = "Title";
		text_ptr[0].text = "Image";
		text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[1].key = "Author";
		text_ptr[1].text = "Bob";
		text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[2].key = "Description";
		text_ptr[2].text = "graph image data";
		text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
	#ifdef PNG_iTXt_SUPPORTED
	    text_ptr[0].lang = NULL;
	    text_ptr[1].lang = NULL;
	    text_ptr[2].lang = NULL;
	#endif
		png_set_text(png_ptr, info_ptr, text_ptr, 3);
	}


	png_write_info(png_ptr, info_ptr);

	
	/* turn on interlace handling if you are not using png_write_image() */
    if (im->interlace)
    	number_passes = png_set_interlace_handling(png_ptr);
    else
      	number_passes = 1;

	/* The number of passes is either 1 for non-interlaced images,
	* or 7 for interlaced images.
	*/
	for (pass = 0; pass < number_passes; pass++){

	  png_byte *row_pointers[2];

	  /* If you are only writing one row at a time, this works */
	  for (y = 0; y < im->sy; y++) {
		row_pointers[0] = &im->pixels[y][0];
		png_write_rows(png_ptr, row_pointers, 1);
	  }

	}

	png_write_end(png_ptr, info_ptr);
	/* if you malloced the palette, free it here */

	png_free(png_ptr,palette);
	palette=NULL;

	/* if you allocated any text comments, free them here */
	/* clean up after the write, and free any memory allocated */

	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* that's it */
#endif
	return;
}

/*-----------------------------------------------------------------------
 *
 * End of miGIF section  - See copyright notice at start of section.
 *
/*----------------------------------------------------------------------- */




/* When gd 1.x was first created, floating point was to be avoided.
   These days it is often faster than table lookups or integer
   arithmetic. The routine below is shamelessly, gloriously
   floating point. TBB */

void
gdImageCopyResampled (gdImagePtr dst,
		      gdImagePtr src,
		      int dstX, int dstY,
		      int srcX, int srcY,
		      int dstW, int dstH,
		      int srcW, int srcH)
{
  int x, y;
  if (!dst->truecolor)
    {
      gdImageCopyResized (
			   dst, src, dstX, dstY, srcX, srcY, dstW, dstH,
			   srcW, srcH);
      return;
    }
  for (y = dstY; (y < dstY + dstH); y++)
    {
      for (x = dstX; (x < dstX + dstW); x++)
	{
	  int pd = gdImageGetPixel (dst, x, y);
	  double sy1, sy2, sx1, sx2;
	  double sx, sy;
	  double spixels = 0;
	  double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;
	  sy1 = ((double) y - (double) dstY) * (double) srcH /
	    (double) dstH;
	  sy2 = ((double) (y + 1) - (double) dstY) * (double) srcH /
	    (double) dstH;
	  sy = sy1;
	  do
	    {
	      double yportion;
	      if (floor (sy) == floor (sy1))
		{
		  yportion = 1.0 - (sy - floor (sy));
		  if (yportion > sy2 - sy1)
		    {
		      yportion = sy2 - sy1;
		    }
		  sy = floor (sy);
		}
	      else if (sy == floor (sy2))
		{
		  yportion = sy2 - floor (sy2);
		}
	      else
		{
		  yportion = 1.0;
		}
	      sx1 = ((double) x - (double) dstX) * (double) srcW /
		dstW;
	      sx2 = ((double) (x + 1) - (double) dstX) * (double) srcW /
		dstW;
	      sx = sx1;
	      do
		{
		  double xportion;
		  double pcontribution;
		  int p;
		  if (floor (sx) == floor (sx1))
		    {
		      xportion = 1.0 - (sx - floor (sx));
		      if (xportion > sx2 - sx1)
			{
			  xportion = sx2 - sx1;
			}
		      sx = floor (sx);
		    }
		  else if (sx == floor (sx2))
		    {
		      xportion = sx2 - floor (sx2);
		    }
		  else
		    {
		      xportion = 1.0;
		    }
		  pcontribution = xportion * yportion;
		  p = gdImageGetPixelRGB (
						 src,
						 (int) sx,
						 (int) sy);
		  red += gdTrueColorGetRed (p) * pcontribution;
		  green += gdTrueColorGetGreen (p) * pcontribution;
		  blue += gdTrueColorGetBlue (p) * pcontribution;
		  alpha += gdTrueColorGetAlpha (p) * pcontribution;
		  spixels += xportion * yportion;
		  sx += 1.0;
		}
	      while (sx < sx2);
	      sy += 1.0;
	    }
	  while (sy < sy2);
	  if (spixels != 0.0)
	    {
	      red /= spixels;
	      green /= spixels;
	      blue /= spixels;
	      alpha /= spixels;
	    }
	  /* Clamping to allow for rounding errors above */
	  if (red > 255.0)
	    {
	      red = 255.0;
	    }
	  if (green > 255.0)
	    {
	      green = 255.0;
	    }
	  if (blue > 255.0)
	    {
	      blue = 255.0;
	    }
	  if (alpha > gdAlphaMax)
	    {
	      alpha = gdAlphaMax;
	    }
	  gdImageSetPixel (dst,
			   x, y,
			   gdTrueColorAlpha ((int) red,
					     (int) green,
					     (int) blue,
					     (int) alpha));
	}
    }
}

gdImagePtr CreateThumbImageFromGD( gdImagePtr sourceImage, long nx, long ny )
{
	gdImagePtr newImage = NULL;

	if ( sourceImage )
	{
		if ( nx == 0 || ny == 0 )
		{
			nx = (int)(sourceImage->sx * 0.33);
			ny = (int)(sourceImage->sy * 0.33);
		}

		newImage = gdImageCreate24bit( nx, ny );
		gdImageCopyResampled( newImage, sourceImage, 0,0,   0,0,     nx,ny,     sourceImage->sx, sourceImage->sy );
	}
	return newImage;
}


