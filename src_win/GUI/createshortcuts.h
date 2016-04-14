#include <Windows.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

long CreateLink(LPCSTR lpszPathObj,LPSTR lpszPathLink,LPSTR lpszDesc);
long GetShortCut( char *linkpath, char *newpath );
long GetURLShortCut( char *linkpath, char *newpath );
//HRESULT ResolveShortCut(HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath);

#ifdef __cplusplus
}
#endif
