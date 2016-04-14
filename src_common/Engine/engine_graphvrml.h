#ifndef __ENGINE_GRAPHVRML
#define __ENGINE_GRAPHVRML

void VRML_Cyl( FILE *f, long rgb, double x, double y, double z, double h, double rad );
void VRML_Box( FILE *f, long rgb, double x, double y, double z, double xs, double ys, double zs );
void VRML_DrawBase( struct graphOptions *graph, int w, int h, int fill );
void VRML_Complete( struct graphOptions *graph );

#endif
 

