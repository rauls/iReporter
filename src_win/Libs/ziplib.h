/*
 WizZip 1.0 header file
*/
#ifndef _WIZZIP_H
#define _WIZZIP_H

#ifndef MSWIN
#define MSWIN
#endif

/* Porting definations between Win 3.1x and Win32 */
#ifdef WIN32
#  define far
#  define _far
#  define __far
#  define near
#  define _near
#  define __near
#endif


#define IDM_ENCRYPT_VERIFY	      200
#define IDM_ZIP_TARGET	         201
#define IDM_ZIP_SUFFIX	         202
#define IDM_ZIP_ENCRYPT	         203
#define IDM_ZIP_SYSTEM	         204
#define IDM_ZIP_VOLUME	         205
#define IDM_ZIP_EXTRA	         206
#define IDM_ZIP_NO_DIR_ENTRIES   207
#define IDM_ZIP_DATE             208
#define IDM_ZIP_VERBOSE	         209
#define IDM_ZIP_QUIET	         210
#define IDM_ZIP_CRLF_LF          211
#define IDM_ZIP_LF_CRLF	         212
#define IDM_ZIP_JUNKDIR	         213
#define IDM_ZIP_RECURSE	         214
#define IDM_ZIP_GROW             215
#define IDM_ZIP_FORCE	         216
#define IDM_ZIP_MOVE             217
#define IDM_ZIP_DELETE_ENTRIES   218
#define IDM_ZIP_UPDATE	         219
#define IDM_ZIP_FRESHEN	         220
#define IDM_GETFILES_HELP        221
#define IDM_ZIP_JUNKSFX          222
#define IDM_ZIP_TIME             223

/* Keep the following in order */
#define IDM_ZIP_LEVEL0           230
#define IDM_ZIP_LEVEL1           231
#define IDM_ZIP_LEVEL2           232
#define IDM_ZIP_LEVEL3           233
#define IDM_ZIP_LEVEL4           234
#define IDM_ZIP_LEVEL5           235
#define IDM_ZIP_LEVEL6           236
#define IDM_ZIP_LEVEL7           237
#define IDM_ZIP_LEVEL8           238
#define IDM_ZIP_LEVEL9           239

#define IDC_SELECT_ALL           242
#define IDC_DESELECT_ALL         243
#define IDC_ADD	               245
#define IDC_DELETE	            246
#define IDC_FILE_LIST            244

#define IDM_SAVE_ZIP_TO_DIR      250

#ifndef NDEBUG
#  define WinAssert(exp) \
        {\
        if (!(exp))\
            {\
            char szBuffer[40];\
            sprintf(szBuffer, "File %s, Line %d",\
                    __FILE__, __LINE__) ;\
            if (IDABORT == MessageBox((HWND)NULL, szBuffer,\
                "Assertion Error",\
                MB_ABORTRETRYIGNORE|MB_ICONSTOP))\
                    FatalExit(-1);\
            }\
        }

#else
#  define WinAssert(exp)
#endif

#define cchFilesMax 4096

typedef int (far *DLLPRNT) (FILE *, unsigned int, char *);
typedef void (far *DLLSND) (void);

typedef struct {
DLLPRNT print;
DLLSND sound;
FILE *Stdout;
HWND hInst;
/* Zip flag section */
BOOL fEncryptVerify;
BOOL fSuffix;
BOOL fEncrypt;
BOOL fSystem;           /* include system and hidden files */
BOOL fVolume;           /* Include volume label */
BOOL fExtra;            /* Include extra attributes */
BOOL fNoDirEntries;     /* Do not add directory entries */
BOOL fDate;             /* Exclude files earlier than specified date */
BOOL fVerbose;          /* Mention oddities in zip file structure */
BOOL fQuiet;            /* Quiet operation */
char fLevel;            /* Compression level (0 - 9) */
BOOL fCRLF_LF;
BOOL fLF_CRLF;          /* Translate end-of-line */
BOOL fJunkDir;          /* Junk directory names */
BOOL fRecurse;          /* Recurse into subdirectories */
BOOL fGrow;             /* Allow appending to a zip file */
BOOL fForce;            /* Make entries using DOS names (k for Katz) */
BOOL fMove;             /* Delete files added or updated in zip file */
BOOL fDeleteEntries;    /* Delete files from zip file */
BOOL fUpdate;           /* Update zip file--overwrite only if newer */
BOOL fFreshen;          /* Freshen zip file--overwrite only */
BOOL fJunkSFX;          /* Junk SFX prefix */
BOOL fLatestTime;       /* Set zip file time to time of latest file in it */
/* End Zip Flag section */
char Date[7];           /* Date to include after */
int  argc;              /* Count of files to zip */
LPSTR lpszZipFN;
char **FNV;
} ZCL, _far *LPZCL;


#ifndef WIZZIPDLL
/* Zip Flags */
typedef struct
{
unsigned int   fSaveZipToDir : 1;
unsigned int   fSaveUnZipFromDir : 1;
} ZF, *PZF;

extern ZF zf;
extern ZCL zcl;
extern LPZCL lpZCL;
extern void WizZipWndProc(HWND hWnd, WORD wMessage, WPARAM wParam, LPARAM lParam);
extern void GetWizZipOptions(void);
extern void StripDirectory(LPSTR lpDir);
extern void MakeArchive(HWND hWnd);
extern BOOL WINAPI GetFilesProc(HWND hwndDlg, WORD wMessage, WPARAM wParam, LPARAM lParam);
extern void WINAPI DllZipUpFiles(ZCL far *C);
#else
extern WINAPI DllZipUpFiles(ZCL far *C);
#endif /* WIZZIPDLL */

#define	ZipUpFiles	DllZipUpFiles

extern HWND hGetFilesDlg;
extern char szFilesToAdd[80];
extern char rgszFiles[cchFilesMax];


#endif /* _WIZZIP_H */

