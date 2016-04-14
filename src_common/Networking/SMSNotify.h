//=================================================================================================
// Copyright (C) Software 2001, 2002
//
// File:            smtp.h
// Subsystem:       Networking
// Created By:      Raul, 04/10/2001
// Description:     Public declarations for smtp stuff defined in SMSNotify.c.
//
// $Archive: /iReporter/Dev/src_common/networking/SMSNotify.h $
// $Revision: 1 $
// $Author: Raul $
// $Date: 6/10/2001 5:25p $
//================================================================================================= 

#ifndef	SMTP_H
#define	SMTP_H

#ifdef __cplusplus
extern "C" {
#endif


int SendSMSMessage( char *szPhoneNumber, char *szUrl, char *szUsername, char *szPassword, char *szMessage )


#ifdef __cplusplus
}
#endif

#endif // SMTP_H