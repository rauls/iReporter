/*****************************************************************************
*   C/C++ Header File for STATISTICA Development Environment                 *
*   Copyright (c) 1997 by StatSoft, Inc.                                     *
*****************************************************************************/

#ifndef _STADEV_H
#define _STADEV_H

#ifdef _WIN32
#define     API_EXP  __stdcall
#else
#define     API_EXP  FAR PASCAL
#endif

typedef		long  HSTAFILE;
typedef		short HRES;

#define     STAMAX_NVARS            4092
#define     STAMAX_VARNAMELEN       8
#define     STAMAX_LONGVARNAMELEN   128
#define     STAMAX_CASENAMELEN      20
#define     STAMAX_SLABELLEN        8
#define     STAMAX_LLABELLEN        40
#define     STAMAX_HEADERLEN        80
#define     STAMAX_NB_SLAB          2200
#define     STAMAX_NB_LLAB          720

#define     RES_OK   1
#define     RES_ERR  0

long API_EXP StaDevVersion ();
HSTAFILE API_EXP StaOpenFile (LPCSTR szFileName);
HSTAFILE API_EXP StaCreateFile (short NVars, long NCases, LPCSTR szFileName); 
HRES API_EXP StaCloseFile (HSTAFILE hSF);

short API_EXP StaGetNVars (HSTAFILE hSF);
long API_EXP StaGetNCases (HSTAFILE hSF);
HRES API_EXP StaAddVars (HSTAFILE hSF, short After, short HowMany);
HRES API_EXP StaDeleteVars (HSTAFILE hSF, short From, short To);
HRES API_EXP StaAddCases (HSTAFILE hSF, long After, long HowMany);
HRES API_EXP StaDeleteCases (HSTAFILE hSF, long From, long To);

HRES API_EXP StaSetFileHeader (HSTAFILE hSF, LPCSTR szHeader);
HRES API_EXP StaGetFileHeader (HSTAFILE hSF, LPSTR szHeader, short BL);

HRES API_EXP StaSetVarName (HSTAFILE hSF, short Var, LPCSTR szName);
HRES API_EXP StaGetVarName (HSTAFILE hSF, short Var, LPSTR szName);
HRES API_EXP StaSetVarLongName (HSTAFILE hSF, short Var, LPCSTR szLongName);
HRES API_EXP StaGetVarLongName (HSTAFILE hSF, short Var, LPSTR szLongName, short BL);

HRES API_EXP StaSetVarFormat (HSTAFILE hSF, short Var, short width, short dec, short categ, short display);
HRES API_EXP StaGetVarFormat (HSTAFILE hSF, short Var, short FAR * lpWidth, short FAR * lpDec, short FAR * lpCateg, short FAR * lpDisplay);

HRES API_EXP StaSetVarMD (HSTAFILE hSF, short Var, double MDValue);
HRES API_EXP StaGetVarMD (HSTAFILE hSF, short Var, double FAR * lpMDVal);
HRES API_EXP StaGetAllMD (HSTAFILE hSF, double FAR * lpMD);

HRES API_EXP StaSetCaseNameLength (HSTAFILE hSF, short CNLen);
short API_EXP StaGetCaseNameLength (HSTAFILE hSF);

HRES API_EXP StaSetCaseName (HSTAFILE hSF, long Case, LPCSTR szName);
HRES API_EXP StaGetCaseName (HSTAFILE hSF, long Case, LPSTR szName, short BL);

HRES API_EXP StaSetData (HSTAFILE hSF, short Var, long Case, double Value);
HRES API_EXP StaGetData (HSTAFILE hSF, short Var, long Case, double FAR * lpValue);
HRES API_EXP StaSetCaseData (HSTAFILE hSF, long Case, const double FAR * lpCase);
HRES API_EXP StaGetCaseData (HSTAFILE hSF, long Case, double FAR * lpCase);

HRES API_EXP StaGetLabelForValue (HSTAFILE hSF, short Var, double Value, LPSTR szLabel);
HRES API_EXP StaGetLongLabelForValue (HSTAFILE hSF, short Var, double Value, LPSTR szLLabel, short BL);
HRES API_EXP StaGetValueForLabel (HSTAFILE hSF, short Var, LPCSTR szLabel, double FAR * lpValue);

HRES API_EXP StaAddLabel (HSTAFILE hSF, short Var, double Value, LPCSTR szLabel, LPCSTR szLongLabel);
HRES API_EXP StaDeleteLabelForValue (HSTAFILE hSF, short Var, double Value);
HRES API_EXP StaDeleteLabel (HSTAFILE hSF, short Var, LPCSTR szLabel);
HRES API_EXP StaGetNumVarLabels (HSTAFILE hSF, short Var, short FAR * lpNumLabels);
HRES API_EXP StaGetVarLabelByIndex (HSTAFILE hSF, short Var, short index,
                                    double FAR * lpValue, LPSTR szLabel,
                                    LPSTR szLongLabel);

#endif