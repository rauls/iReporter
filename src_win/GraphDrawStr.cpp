

#define UNICODE // need to use unicode functions, rather than ANSI

#include <stdio.h>
#include "GraphDrawStr.h"
#include "translate.h"
#include "myansi.h"

NonISO88591String::NonISO88591String()
{
	width = 0;
	height = 0;
	data = 0;
	TextSizeForNonISO88591[0] = 10;
	TextSizeForNonISO88591[1] = 14;
	TextSizeForNonISO88591[2] = 16;
	TextSizeForNonISO88591[3] = 9;
	NonISO88591W = 300; 
	NonISO88591H = TextSizeForNonISO88591[2]; 
	drawGraphTextDynamicallyForNonISO88591 = 0;
	for( short i = 0; i < 4; i++ )
		hFontSizes[i] = 0;
}

NonISO88591String::~NonISO88591String()
{
	DestroyObjects();
}

short NonISO88591String::CreateObjects( const char *fontName, const char *charSet )
{
	//if ( strcmp( TranslateID( SYSI_CHARSET ), "ISO-8859-1" ) != 0 )
	//{ // We have to create the characters
//		drawGraphTextDynamicallyForNonISO88591 = 1;
		hWndNonOSI = hwndParent;
		mystrncpy( GraphFontName, fontName, 32 );
		GraphFontName[31] = 0;
		if ( GraphFontName[0] == 0 || GraphFontName[0] == ' ' )
		{
			ErrorMsg( "Please select a font to be used for drawing graphs" );
			CHOOSEFONT cf;
			LOGFONT lf;
			memset( &cf, 0, sizeof( CHOOSEFONT ) );
			memset( &lf, 0, sizeof( LOGFONT ) );
			cf.lStructSize = sizeof( CHOOSEFONT );
			cf.hwndOwner = hWndNonOSI;
			cf.hDC = hdc;
			cf.lpLogFont = &lf;
			cf.Flags = CF_BOTH;
			cf.nFontType = REGULAR_FONTTYPE;
			lf.lfCharSet = CharSet( charSet );
			wsprintf( lf.lfFaceName, L"%S", GraphFontName );
			if ( ChooseFont( &cf ) )
			{
				sprintf( GraphFontName, "%S", lf.lfFaceName );
				GraphFontName[31] = 0;
				//HFONT hFont = CreateFontIndirect( cf.lpLogFont );
			}
			else
			{
				ErrorMsg( "Warning, no font has been selected for drawing graphs." );
				return 0;
			}
		}

		WCHAR WGraphFontName[32];
		wsprintf( WGraphFontName, L"%.32S", GraphFontName );
		for( short i = 0; i < 4; i++ )
		{

			hFontSizes[i] = CreateFont( TextSizeForNonISO88591[i], 0, 0, 0, 0, 0, 0, 0, CharSet( charSet ), 0, 0, 0,DEFAULT_PITCH | FF_DONTCARE, WGraphFontName );
			char fontUsed[32];
			sprintf( fontUsed, "%.32S", WGraphFontName );

			if ( !hFontSizes[i] )
			{
				CHOOSEFONT cf;
				LOGFONT lf;
				memset( &cf, 0, sizeof( CHOOSEFONT ) );
				memset( &lf, 0, sizeof( LOGFONT ) );
				cf.lStructSize = sizeof( CHOOSEFONT );
				cf.hwndOwner = hWndNonOSI;
				cf.hDC = hdc;
				cf.lpLogFont = &lf;
				cf.Flags = CF_BOTH;
				cf.nFontType = REGULAR_FONTTYPE;
				lf.lfCharSet = CharSet( charSet );
				wsprintf( lf.lfFaceName, L"%.32S", GraphFontName );
				ErrorMsg( "The font you have specified in your language is not available,\nplease select another font to be used for drawing graphs" );
				while( !ChooseFont( &cf ) )
				{
					//mystrncpy( GraphFontName, lf.lfFaceName, 32 );
					//wsprintf( lf.lfFaceName, L"%S", GraphFontName );
					sprintf( GraphFontName, "%.32S", lf.lfFaceName );
					GraphFontName[31] = 0;
				}
				wsprintf( WGraphFontName, L"%S", GraphFontName );
			}
		}
		drawGraphTextDynamicallyForNonISO88591 = 2;
		hdc = BeginPaint(hWndNonOSI, &ps);
		//hdc = GetDC(hWndNonOSI);
		hdcSrc = CreateCompatibleDC( hdc );
		bgBitMap = CreateCompatibleBitmap( hdc, NonISO88591W, NonISO88591H ); //

		textrc.left = 0; textrc.top = 0;
		textrc.right = 300; textrc.bottom = 100;
		bgBrush = CreateSolidBrush( 0xFFFFFF );
		SetTextColor( hdcSrc, 0x00 );
		data = new char[NonISO88591W*NonISO88591H];
	//}
	return 1;
}

void NonISO88591String::DestroyObjects()
{
//	if ( drawGraphTextDynamicallyForNonISO88591 )
//	{
		delete[] data;
		//drawGraphTextDynamicallyForNonISO88591 = 0;
		for( short i = 0; i < 4; i++ )
		{
			DeleteObject( hFontSizes[i] );
			hFontSizes[i] = 0;
		}

		if ( drawGraphTextDynamicallyForNonISO88591 == 2 )
		{
			DeleteObject( bgBrush );
			DeleteObject( bgBitMap );
			DeleteDC( hdcSrc );
			ReleaseDC( hWndNonOSI, hdc );
			EndPaint( hWndNonOSI, &ps );
		}
//	}
}


long NonISO88591String::CharSet( const char *charSet )
{
	if ( mystrcmpi( charSet, "GB2312" ) == 0 )
		return CHINESEBIG5_CHARSET;
	else if ( mystrcmpi( charSet, "GREEK_CHARSET" ) )
		return GREEK_CHARSET;
	else
		return ANSI_CHARSET;//GetTextCharset( );
}

void NonISO88591String::DrawString( short fontsize, const char *string )
{
	// Convert the single byte string to a Unicode (2 byte) string
	WCHAR uniString[128];
	wsprintf( uniString, L"%S", string );

	int result;
	long xmax=0,ymax=0,x,y, pix, i;

	SelectObject( hdcSrc, bgBitMap );
	FillRect( hdcSrc, &textrc, bgBrush );
	pix = -1;
	int len = wcslen(uniString);
	SelectObject( hdcSrc, hFontSizes[fontsize] );

	result = DrawText( hdcSrc, uniString, wcslen(uniString), &textrc, 0 );

	SelectObject (hdcSrc, bgBitMap);
	x=0;y=0;
	while( (i=GetPixel( hdcSrc, x, y )) != pix )
		x++;
	if ( x > xmax ) xmax = x;
	x=0;
	while( (i=GetPixel( hdcSrc, x, y )) != pix )
		y++;
	if ( y > ymax ) ymax = y;
	width = xmax;
	height = ymax;

	if ( width != 0 && height != 0)
	{
		char *dataPtr = data;
		for ( int h=0; h<height; h++ )
		{
			for ( int w=0; w<width; w++ )
			{
				pix = GetPixel( hdcSrc, w, h );
				if ( pix == 0 )
					*dataPtr = 0;
				else
					*dataPtr = 1;
				dataPtr++;
			}
		}
	}
}
