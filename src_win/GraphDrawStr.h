

#ifndef GRAPH_STR_DRAW_H
#define GRAPH_STR_DRAW_H


#include <windows.h>

extern "C" HWND	hwndParent;

class NonISO88591String
{
public:
	NonISO88591String();
	~NonISO88591String();
	void DrawString( short fontsize, const char *string );
	short Width() { return width; }
	short Height() { return height; }
	char Data( short pos ) { return data[pos]; }
	//char DrawGraphText() { return drawGraphTextDynamicallyForNonISO88591; }
	short FontSize( short size ) { return TextSizeForNonISO88591[size]; }
	short CreateObjects( const char *fontName, const char *charSet );
private:
	void DestroyObjects();
	long CharSet( const char *charSet );
private:
	short width;
	short height;
	char *data;
	HFONT hFontSizes[4];
	HBITMAP bgBitMap;
	HDC		hdcSrc;
	HBRUSH bgBrush;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT textrc;
	HWND hWndNonOSI;
	char drawGraphTextDynamicallyForNonISO88591;
	char GraphFontName[32];

	short TextSizeForNonISO88591[4];// = { 10, 14, 16, 9 };
	short NonISO88591W;// = 300; 
	short NonISO88591H;// = TextSizeForNonISO88591[2]; 
	char *NonISO88591Buf;
};

/*class GraphStrDraw
{
public:
	GraphStrDraw();
	~GraphStrDraw();
};*/


#endif // GRAPH_STR_DRAW_H
