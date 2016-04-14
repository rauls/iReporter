#define INC_OLE2
 
//ole32.lib uuid.lib url.lib

#include <windows.h>
#include <ole2.h>
#include <shlobj.h>
#include <objidl.h>
//#include <Objbase.h>

#include "createshortcuts.h"
#include "intshcut.h"

long CreateLink(LPCSTR lpszPathObj,LPSTR lpszPathLink,LPSTR lpszDesc)
{
	HRESULT hres;
	IShellLink* psl;
    
	CoInitialize(NULL);
	hres = CoCreateInstance(CLSID_ShellLink, NULL, 
        CLSCTX_INPROC_SERVER, IID_IShellLink,(void**)&psl); 
    if (SUCCEEDED(hres)) 
	{
		IPersistFile *ppf;
		hres = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);
		if (SUCCEEDED(hres)) 
		{ 
            WORD wsz[MAX_PATH];
			ZeroMemory(wsz,sizeof(WORD)*MAX_PATH);
			hres = psl->SetPath(lpszPathObj); 
			hres = psl->SetDescription(lpszDesc);  
       		hres = psl->SetIconLocation(lpszPathObj,0);
			MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);
			hres = ppf->Save(wsz, TRUE); 
            ppf->Release();
			if(!SUCCEEDED(hres))
				return false;
		} 
		else
			return false;
        psl->Release();
	}
	else
		return false;
	CoUninitialize();
	return true;
}

long GetShortCut( char *linkpath, char *newpath )
{
	HRESULT hres;
	IShellLink* psl;
	WIN32_FIND_DATA fd; 
    
	CoInitialize(NULL);
	hres = CoCreateInstance(CLSID_ShellLink, NULL, 
        CLSCTX_INPROC_SERVER, IID_IShellLink,(void**)&psl); 
    if (SUCCEEDED(hres)) {
		IPersistFile *ppf;
		hres = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);		//IID_IPersistFile
		if (SUCCEEDED(hres)) { 
            WORD wsz[MAX_PATH];

            MultiByteToWideChar(CP_ACP, 0, linkpath, -1, wsz,  MAX_PATH);              // Load the shortcut. 
			hres = ppf->Load(wsz, TRUE); 
			if (SUCCEEDED(hres)) {
				psl->GetPath(newpath ,256, &fd, SLGP_SHORTPATH );
			}
            ppf->Release();
			if(!SUCCEEDED(hres))
				return false;
		} else
			return false;
        psl->Release();
	} else
		return false;
	CoUninitialize();
	return true;
}

/*
	FILE *fp;
	char szText[4096];
	if ( fp = fopen( linkpath, "r" ) ){

		while( !feof( fp ) ){
			szText[0]=0;
			fgets( szText, 4096, fp );
			if ( !strcmpd( "URL=", szText ) ){
				mystrcpy( out, szText+5 );
				break;
			}
		}
*/

long GetURLShortCut( char *linkpath, char *newpath )
{
	HRESULT hres;
	IUniformResourceLocator *pHook;
	WIN32_FIND_DATA fd; 
    
	CoInitialize(NULL);
	hres = CoCreateInstance(CLSID_InternetShortcut, NULL, 
        CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator,(void**)&pHook); 
    if (SUCCEEDED(hres)) {
		IPersistFile *ppf;
		IShellLink *psl;

		hres = pHook->QueryInterface (IID_IPersistFile, (void **)&ppf);		//IID_IPersistFile
		hres = pHook->QueryInterface (IID_IShellLink, (void **)&psl);
		if (SUCCEEDED(hres)) { 
            WORD wsz[MAX_PATH];
			char *lpsz;

            MultiByteToWideChar(CP_ACP, 0, linkpath, -1, wsz,  MAX_PATH);              // Load the shortcut. 
			hres = ppf->Load (wsz, TRUE);
			if (SUCCEEDED(hres)) { 
				pHook->GetURL(&lpsz);
				if ( lpsz )
					strcpy( newpath, lpsz );
			}
	        psl->Release();
            ppf->Release();
			if(!SUCCEEDED(hres))
				return false;
		} else
			return false;
		pHook->Release ();
	} else
		return false;
	CoUninitialize();
	return true;
}
