//=================================================================================================
//
// File:            OutputMessages.h
// Subsystem:       Networking
// Description:     Moved some common message output functions from the Windows and UNIX versions
//					of remotecontrol.c to here.
//
// $Archive: /iReporter/Dev/src_common/networking/OutputMessages.h $
// $Revision: 1 $
// $Date: 16/07/01 12:28p $
//================================================================================================= 

#ifndef	OUTPUTMESSAGES_H
#define	OUTPUTMESSAGES_H

void ShowRecievedLine( char *pre, char *txt );
void DebugRecievedLine( char *pre, char *txt );

#endif // OUTPUTMESSAGES_H