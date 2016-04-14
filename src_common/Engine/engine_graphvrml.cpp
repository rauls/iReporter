
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#if	__dest_os	== __mac_os
	#include "MacUtil.h"
	#include "main.h"
	#include "progress.h"	
	#include "post_comp.h"
#else
	#include "postproc.h"
#endif

#include "myansi.h"
#include "gd.h"
#include "config_struct.h"
#include "engine.h"
#include "engine_graph.h"
#include "editpath.h"
#include "datetime.h"

extern "C" int IsDemoReg( void );
extern "C" struct App_config MyPrefStruct;




static int Fprintf( FILE *fp, const char *fmt, ... )
{
	va_list		args;
static	char lineout[10240];

	va_start( args, fmt );
	vsprintf( lineout, fmt, args );
	va_end( args );

	return fprintf( fp, lineout );
}


static char VRML_header[] = { "#VRML V2.0 utf8\nTransform {\n"};

void VRML_Value( FILE *f, char *t, double x )
{
	fprintf( f, "\t\t %s %f\n", t, x );
}

void VRML_Write( FILE *f, char *t )
{
	fprintf( f, "\t\t%s\n", t );
}

void VRML_End( FILE *f )
{
	fprintf( f, "\t] # end\n}\n" );
}


void VRML_Begin( FILE *f )
{
	fprintf( f, VRML_header );
	fprintf( f, "children [\n" );
	Fprintf( f, "\t\tNavigationInfo { headlight FALSE }\n" );
	Fprintf( f, "\t\tDirectionalLight {\n\t\t direction 0 -5 1 }\n\n" );
}

void VRLM_ObjXYZ( FILE *f, float x, float y, float z )
{
	Fprintf( f, "Transform {\n" );
	Fprintf( f, " translation %f %f %f\n", x, y, z );
}
void VRLM_ObjMaterial( FILE *f, float r, float g, float b )
{
	Fprintf( f, "    appearance Appearance {" );
	Fprintf( f, "     material Material { diffuseColor %f %f %f } }\n", r/256.0,g/256.0,b/256.0 );
}

void VRLM_ObjBox( FILE *f, float xs, float ys, float zs )
{
	Fprintf( f, "   Shape {\n" );
	Fprintf( f, "    geometry Box { size %f %f %f }\n", xs, ys, zs );
}
void VRLM_ObjCyl( FILE *f, float h, float rad )
{
	Fprintf( f, "   Shape {\n" );
	Fprintf( f, "    geometry Cylinder { height %f radius %f }\n", h, rad );
}

void VRLM_ChildStart( FILE *f  )
{
	Fprintf( f, "  children [\n" );
}
void VRLM_ChildEnd( FILE *f  )
{
	Fprintf( f, "  }]\n" );
}
void VRLM_ObjEnd( FILE *f  )
{
	Fprintf( f, " }\n\n" );
}



void VRML_Cyl( FILE *f, long rgb, float x, float y, float z, float h, float rad )
{
	double r = rgb>>16, g = rgb&0xff00 >> 8, b = rgb&0xff;

	VRLM_ObjXYZ( f, x, y, z );
	VRLM_ChildStart( f );
	VRLM_ObjCyl( f, h, rad );
	VRLM_ObjMaterial( f, r, g, b );
	VRLM_ChildEnd( f );
	VRLM_ObjEnd( f );
}

void VRML_Box( FILE *f, long rgb, float x, float y, float z, float xs, float ys, float zs )
{
	double r = rgb>>16, g = rgb&0xff00 >> 8, b = rgb&0xff;

	VRLM_ObjXYZ( f, x, y, z );
	VRLM_ChildStart( f );
	VRLM_ObjBox( f, xs, ys, zs );
	VRLM_ObjMaterial( f, r, g, b );
	VRLM_ChildEnd( f );
	VRLM_ObjEnd( f );
}


void VRML_DrawBase( struct graphOptions *graph, int w, int h, int fill )
{
	if ( graph->out ){
		VRML_Begin( graph->out );

		VRML_Cyl( graph->out, 0xcccccc, 0,0,0, (float)0.001, 5 );
		VRML_Box( graph->out, 0xdddddd, -2,-2,0,  0, 4, 4 );
		VRML_Box( graph->out, 0xdddddd,  2,-2,0,  0, 4, 4 );
		VRML_Box( graph->out, 0x0000ff,  0,-2,-2, 4, 4, 0 );
		VRML_End( graph->out );
	}
}

void VRML_Complete( struct graphOptions *graph )
{
	if( graph->out )
		VRML_End( graph->out );
}

