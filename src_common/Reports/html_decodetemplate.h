/*=================================================================================================
 Copyright © Analysis Software 2001, 2002

 File:            html_decodetemplate.cpp
 Subsystem:       Report
 Created By:      RSOBON, 12/Nov/2002
 Description:
 This segment loads and decodes a summary page template file for html only, it will
 fill in all variables in [$VAR] format, and also decode which segments to keep or ignore.

 $Archive: /iReporter/Dev/src_common/engine/html_decodetemplate.cpp $
 $Revision: 4 $
 $Author: RAUL $
 $Date: 12/11/01 11:11a $
*/ //================================================================================================= 



long FilterSections( VDinfoP VDptr, char *source_html );
char *ResolveAllVariables( VDinfoP VDptr, char *htmldata, long datalen );
long SwitchonAllThumbnails( VDinfoP VDptr );
// ------------------------------------------------ MAIN ENTRY POINT ------------------------------------------------
long CreateSummaryPage( VDinfoP VDptr, FILE *outfp, char *outputPath, long logNum );





























