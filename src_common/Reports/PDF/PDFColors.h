

#ifndef PDFCOLORS_H
#define PDFCOLORS_H

// Individual Font Styles
#define PDF_NORMAL			0x0000
#define PDF_BOLD			0x0001
#define PDF_ITALIC 			0x0002
#define PDF_HTML_FORMATTING	0x0004
#define PDF_UNDERLINE 		0x0008
#define PDF_INVALID_STYLE	-1
#define PDF_CURRENT_STYLE	0x00F0
// Combined Font Styles
#define PDF_BOLDITALIC 	PDF_BOLD | PDF_ITALIC

#define PDF_LEFT			0x0000
#define PDF_RIGHT			0x0001
#define PDF_NOELLIPSE		0x0000
#define PDF_ELLIPSE			0x0002
#define PDF_BREAK_AT_SPACE	0x0004
#define PDF_LEFT_NOELLIPSE	PDF_LEFT | PDF_NOELLIPSE
#define PDF_LEFT_ELLIPSE	PDF_LEFT | PDF_ELLIPSE
#define PDF_RIGHT_NOELLIPSE	PDF_RIGHT | PDF_NOELLIPSE
#define PDF_RIGHT_ELLIPSE	PDF_RIGHT | PDF_ELLIPSE

/*#define PDF_PAGENUMBER_AT_BOTTOM	0x0000
#define PDF_PAGENUMBER_AT_TOP		0x0001

#define PDF_PAGENUMBER_CENTER		0x0000
#define PDF_PAGENUMBER_LEFT			0x0002
#define PDF_PAGENUMBER_RIGHT		0x0004*/


/*extern const int PDF_INVALID_COLOR	; // -1;
extern const int PDF_BLACK			; // 0x000000;
extern const int PDF_RED			; // 0xFF0000;
extern const int PDF_GREEN			; // 0x00FF00;
extern const int PDF_LIGHTGREEN		; // 0x99FF66;
extern const int PDF_DARKGREEN		; // 0x008800;
extern const int PDF_BLUE			; // 0x0000FF;
extern const int PDF_LIGHTBLUE		; // 0x6699FF;
extern const int PDF_DARKBLUE		; // 0x000088;
extern const int PDF_GREY			; // 0xCCCCCC;
extern const int PDF_LIGHTGREY		; // 0x888888;
extern const int PDF_DARKGREY		; // 0x333333;
extern const int PDF_PINK			; // 0xFF00CC;
extern const int PDF_LIGHTPINK		; // 0xFF9999;
extern const int PDF_DARKPINK		; // 0xFF0066;
extern const int PDF_BROWN			; // 0x883300;
extern const int PDF_LIGHTBROWN		; // 0x993300;
extern const int PDF_DARKBROWN		; // 0x993300;
extern const int PDF_YELLOW			; // 0x00FFFF;
extern const int PDF_LIGHTYELLOW	; // 0xFFFF99;
extern const int PDF_ORANGE			; // 0xFF6600;
extern const int PDF_LIGHTORANGE	; // 0xFFCC33;
extern const int PDF_PURPLE			; // 0x9900CC;
extern const int PDF_LIGHTPURPLE	; // 0x9900CC;
extern const int PDF_DARKPURPLE		; // 0x990066;
extern const int PDF_KHAKI			; // 0x666600;
extern const int PDF_AQUA			; // 0x339999;*/


#define PDF_INVALID_COLOR	-1
#define PDF_WHITE			0xFFFFFF
#define PDF_BLACK			0x000000
#define PDF_RED				0xFF0000
#define PDF_GREEN			0x00FF00
#define PDF_LIGHTGREEN		0x99FF66
#define PDF_DARKGREEN		0x008800
#define PDF_BLUE			0x0000FF
#define PDF_LIGHTBLUE		0x6699FF
#define PDF_DARKBLUE		0x000088

#define PDF_GREY			0xCCCCCC
#define PDF_LIGHTGREY		0x888888
#define PDF_DARKGREY		0x333333

#define PDF_PINK			0xFF00CC
#define PDF_LIGHTPINK		0xFF9999
#define PDF_DARKPINK		0xFF0066

#define PDF_BROWN			0x883300
#define PDF_LIGHTBROWN		0x993300
#define PDF_DARKBROWN		0x993300

#define PDF_YELLOW			0xFFFF00
#define PDF_LIGHTYELLOW		0xFFFF99

#define PDF_ORANGE			0xFF6600
#define PDF_LIGHTORANGE		0xFFCC33

#define PDF_PURPLE			0x9900CC
#define PDF_LIGHTPURPLE		0x9900CC
#define PDF_DARKPURPLE		0x990066

#define PDF_KHAKI			0x666600
#define PDF_AQUA			0x339999



#define PDF_BLACK_STR "Black"
#define PDF_RED_STR "Red"
#define PDF_GREEN_STR "Green"
#define PDF_LIGHTGREEN_STR "LightGreen"
#define PDF_DARKGREEN_STR "DarkGreen"
#define PDF_BLUE_STR "Blue"
#define PDF_LIGHTBLUE_STR "LightBlue"
#define PDF_DARKBLUE_STR "DarkBlue"
#define PDF_GREY_STR "Grey"
#define PDF_LIGHTGREY_STR "LightGrey"
#define PDF_DARKGREY_STR "DarkGrey"
#define PDF_PINK_STR "Pink"
#define PDF_LIGHTPINK_STR "LightPink"
#define PDF_DARKPINK_STR "DarkPink"
#define PDF_BROWN_STR "Brown"
#define PDF_LIGHTBROWN_STR "LightBrown"
#define PDF_DARKBROWN_STR "DarkBrown"
#define PDF_YELLOW_STR "Yellow"
#define PDF_LIGHTYELLOW_STR "LightYellow"
#define PDF_ORANGE_STR "Orange"
#define PDF_LIGHTORANGE_STR "LightOrange"
#define PDF_PURPLE_STR "Purple"
#define PDF_LIGHTPURPLE_STR "LightPurple"
#define PDF_DARKPURPLE_STR "DarkPurple"
#define PDF_KHAKI_STR "Khaki"
#define PDF_AQUA_STR "Aqua"


typedef struct ComboBoxColorsStruct
{
	char *colorName;
	int colorValue;
} ComboBoxColors;


#endif // PDFCOLORS_H
