//=================================================================================================
// Copyright © RedFlag Software 2001, 2002
//
// File:            IceStr.h
// Subsystem:       Encryption
// Description:     Defines functions that encrypt/decrypt a C string.  These functions 
//					represent a layer over the lower level ice functions defined within ice.h
//
// $Archive: $
// $Revision: $
// $Author: $
// $Date: $
//================================================================================================= 

#ifndef	ICESTR_H
#define	ICESTR_H

//------------------------------------------------------------------------------------------
// System Includes
//------------------------------------------------------------------------------------------
#include "FWA.h"
#include <cstddef>	// for size_t
#include <string>

//------------------------------------------------------------------------------------------
// Project Includes
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------------------
std::string IceEncryptStr( const char* str, size_t length=0 );
std::string IceDecryptStr( const char* str, size_t length=0 );

#endif // ICESTR_H












