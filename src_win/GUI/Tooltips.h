
#ifndef	__TOOLTIPS_H
#define	__TOOLTIPS_H

#ifdef __cplusplus
extern "C"{
#endif

typedef struct	TooltipInfo {
	long	id;
	char	text[128];
} TOOLTIPINFO, *TOOLTIPINFO_PTR;

typedef struct	TooltipData {
	long	id;
	long	strid;
} TOOLTIPDATA, *TOOLTIPDATA_PTR;



void CreateTooltips( HWND hWnd,TOOLTIPINFO *tips, long total );

void CreateTooltipsNew( HWND hWnd,TOOLTIPDATA *tips, long total );


LRESULT CALLBACK ToolTipCreateProc( HWND hWnd,
                              UINT uMessage,
                              WPARAM wParam,
                              LPARAM lParam);

LRESULT CALLBACK ToolTipCreateProcNew( HWND hWnd,
                              UINT uMessage,
                              WPARAM wParam,
                              LPARAM lParam);

#ifdef __cplusplus
}
#endif


#endif
