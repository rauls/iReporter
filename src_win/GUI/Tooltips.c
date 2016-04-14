
#define STRICT

/**************************************************************************
   Include Files
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "tooltips.h"

/**************************************************************************
   Global Variables
**************************************************************************/

extern HINSTANCE   hInst;

static HWND hwndToolTip;


void CreateTooltips( HWND hWnd,TOOLTIPINFO *tips, long total )
{
	long	index = 0;
	
	while( tips[index].id ){
		TOOLINFO ti;

		//add the button to the tooltip
		ZeroMemory(&ti, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;  //TTF_SUBCLASS causes the tooltip to automatically subclass the window and look for the messages it is interested in.
		ti.hwnd = hWnd;
		ti.uId = (UINT)GetDlgItem( hWnd, tips[index].id );
		ti.lpszText = tips[index].text;
		SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		index++;
	}
}

void CreateTooltipsNew( HWND hWnd,TOOLTIPDATA *tips, long total )
{
	long	index = 0;
	
	while( tips[index].id ){
		TOOLINFO ti;

		//add the button to the tooltip
		ZeroMemory(&ti, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;  //TTF_SUBCLASS causes the tooltip to automatically subclass the window and look for the messages it is interested in.
		ti.hwnd = hWnd;
		ti.uId = (UINT)GetDlgItem( hWnd, tips[index].id );
		ti.lpszText = (LPTSTR)tips[index].strid;
		SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		index++;
	}
}





LRESULT CALLBACK ToolTipCreateProc( HWND hWnd,
                              UINT uMessage,
                              WPARAM wParam,
                              LPARAM lParam)
{

switch (uMessage)
   {
   case WM_CREATE:
      {
      /* create the tooltip window - we will use the default delays, but 
      if you want to change that, you can use the TTM_SETDELAYTIME 
      message */
      hwndToolTip = CreateWindowEx( 0, 
                                    TOOLTIPS_CLASS, 
                                    NULL, 
                                    WS_POPUP | TTS_ALWAYSTIP, 
                                    CW_USEDEFAULT, 
                                    CW_USEDEFAULT, 
                                    12, 
                                    12, 
                                    hWnd, 
                                    NULL, 
                                    hInst, 
                                    NULL);
      
      if(NULL == hwndToolTip){
         break;
         }

      CreateTooltips( hWnd, (TOOLTIPINFO *)lParam, (long)wParam );
      
      }
      break;

   case WM_MOUSEMOVE:
      {
      MSG   msg;

      //we need to fill out a message structure and pass it to the tooltip 
      //with the TTM_RELAYEVENT message
      msg.hwnd = hWnd;
      msg.message = uMessage;
      msg.wParam = wParam;
      msg.lParam = lParam;
      GetCursorPos(&msg.pt);
      msg.time = GetMessageTime();
      
      SendMessage(hwndToolTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
      }
      break;
   
   default:
      break;
   }

	return TRUE;
}




LRESULT CALLBACK ToolTipCreateProcNew( HWND hWnd,
                              UINT uMessage,
                              WPARAM wParam,
                              LPARAM lParam)
{

switch (uMessage)
   {
   case WM_CREATE:
      {
      /* create the tooltip window - we will use the default delays, but 
      if you want to change that, you can use the TTM_SETDELAYTIME 
      message */
      hwndToolTip = CreateWindowEx( 0, 
                                    TOOLTIPS_CLASS, 
                                    NULL, 
                                    WS_POPUP | TTS_ALWAYSTIP, 
                                    CW_USEDEFAULT, 
                                    CW_USEDEFAULT, 
                                    12, 
                                    12, 
                                    hWnd, 
                                    NULL, 
                                    hInst, 
                                    NULL);
      
      if(NULL == hwndToolTip){
         break;
         }

      CreateTooltipsNew( hWnd, (TOOLTIPDATA *)lParam, (long)wParam );
      
      }
      break;

   case WM_MOUSEMOVE:
      {
      MSG   msg;

      //we need to fill out a message structure and pass it to the tooltip 
      //with the TTM_RELAYEVENT message
      msg.hwnd = hWnd;
      msg.message = uMessage;
      msg.wParam = wParam;
      msg.lParam = lParam;
      GetCursorPos(&msg.pt);
      msg.time = GetMessageTime();
      
      SendMessage(hwndToolTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
      }
      break;
   
   default:
      break;
   }

	return TRUE;
}
