//=================================================================================================
// Copyright © Analysis Software 2001, 2002
//
// File:            IceStr.cpp
// Subsystem:       Encryption
// Created By:      Zoltan, 07/01/02
// Description:     Implements functions that encrypt/decrypt a C string.  These functions 
//					represent a layer over the lower level ice functions defined within ice.h
//
// $Archive: $
// $Revision: $
// $Author: $
// $Date: $
//================================================================================================= 

//==========================================================================================
// System Includes
//==========================================================================================
#include "FWA.h"
#include <cstring>	// for strlen(), memcpy() etc
#include "string.h" // for strlen(), memcpy(), etc.
//==========================================================================================
// Project Includes
//==========================================================================================
#include "IceStr.h"
#include "ice.h"

//==========================================================================================
// Initialisations & Declarations
//==========================================================================================

static const char s_my2ndkey[]="shutdown";
static const size_t s_iceChunkSize(8);		// ice routines encrypt/decrypt 8-byte chunks
static const char s_paddingChar(1);			// Used to pad strings to above size before -
											// encryption.  The idea here is to use a char
											// that isn't likely to appear at the end of 
											// a string passed for encryption.
											// NOTE: Should not be zero!!

//==========================================================================================
// Implementations
//==========================================================================================

//------------------------------------------------------------------------------------------
// Encrypt the string <str> and return the result within a std::string.  The optional
// <length> parameter should be used if you know the length of the string or if you don't 
// want to encrypt the entire string.  If <length> is not specified (or zero) then strlen()
// will be used to determine the length of <str>.  Note that the resulting encrypted string
// that's returned may end up being a bit longer than the original.  This is because the
// ice encryption algorithm only encrypts/decrypts 8-byte chunks and hence the original
// string may end up being padded before encryption.
//------------------------------------------------------------------------------------------
std::string IceEncryptStr( const char* str, size_t length/*=0*/ )
{
	DEF_PRECONDITION( str ); 

	// create encyrption key
	ICE_KEY* key=ice_key_create( 0 );
	if( !key )
	{
		return "";
	}

	ice_key_set( key, (unsigned char *)s_my2ndkey );
	
	size_t copyLength(s_iceChunkSize);						// actual copy length for current iteration
	size_t bytesLeft( length ? length : strlen( str ) );	// bytes left to encrypt
	const size_t newLength(bytesLeft+s_iceChunkSize-bytesLeft%s_iceChunkSize); 

	std::string retVal;
	retVal.reserve(newLength);

	// while there are more bytes in str to encrypt
	while( bytesLeft>0 )
	{
		// because ice can only encrypt an 8-byte chunk at a time,
		// we use temp buffers and we zero pad the last chunk 
		unsigned char src[s_iceChunkSize];
		unsigned char dst[s_iceChunkSize];

		// handle terminating boundary condition
		if( bytesLeft<s_iceChunkSize )
		{
			copyLength=bytesLeft;												// only copy real bytes
			memset( src+copyLength, s_paddingChar, s_iceChunkSize-copyLength );	// pad out temp buffer
		}

		// copy chunk to temp buffer
		memcpy( src, str, copyLength );			

		// encrypt from temp buffer into dest string
		ice_key_encrypt( key, src, dst );

		// copy encrypted chunk to retVal string
		retVal.append( reinterpret_cast<char*>(dst), s_iceChunkSize );

		cycle_key( key );
		
		// point to next chunk to be encrypted
		str+=copyLength;
		bytesLeft-=copyLength;
	}

	// clean up encryption key
	ice_key_destroy ( key );

	return retVal;
}


//------------------------------------------------------------------------------------------
// Decrypt the string <str> and return the result within a std::string.  The optional
// <length> parameter should be used if you know the length of the string or if you don't 
// want to decrypt the entire string.  If <length> is not specified (or zero) then strlen()
// will be used to determine the length of <str>.  At any rate, the actual encrypted
// string's length (or the <length> specified) MUST be a multiple of 8.  This is because
// the ice encryption algorithm can only encrypt/decrypt 8 byte-chunks and hence the
// resulting encrypted string may actually end up longer than the original (i.e. rounded
// up to next 8-char multiple).
//------------------------------------------------------------------------------------------
std::string IceDecryptStr( const char* str, size_t length )
{
	DEF_PRECONDITION( str ); 

	// create decyrption key
	ICE_KEY* key=ice_key_create( 0 );
	if( !key )
	{
		return "";
	}

	ice_key_set( key, (unsigned char *)s_my2ndkey );
	
	size_t copyLength(s_iceChunkSize);						// actual copy length for current iteration
	size_t bytesLeft( length ? length : strlen( str ) );	// bytes left to encrypt
	DEF_ASSERT( bytesLeft%s_iceChunkSize==0 );				// must be integral multiple of 8

	std::string retVal;
	retVal.reserve(bytesLeft);

	// while there are more bytes in str to decrypt
	while( bytesLeft>0 )
	{
		// temp buffer for decryption destination
		unsigned char dst[s_iceChunkSize];

		// decrypt chunk into temp buffer
		ice_key_decrypt( key, reinterpret_cast<const unsigned char*>(str), dst );

		// handle terminating boundary condition
		if( bytesLeft==s_iceChunkSize )
		{
			// if we find a null terminator then adjust our copyLength
			unsigned char* nullCharPtr=reinterpret_cast<unsigned char*>( memchr( dst, 0, s_iceChunkSize ) );

			if( nullCharPtr )
			{
				copyLength=nullCharPtr-dst;
			}
		}

		// copy decrypted chunk to retVal string
		retVal.append( reinterpret_cast<char*>(dst), copyLength );

		cycle_key( key );
		
		// point to next chunk to be decrypted
		str+=s_iceChunkSize;
		bytesLeft-=s_iceChunkSize;
	}

	// clean up decryption key
	ice_key_destroy ( key );

	// remove extra padding that we may have added during encryption
	while( retVal.size() && retVal[retVal.size()-1]==s_paddingChar )
	{
		retVal.resize( retVal.size()-1 );
	}

	return retVal;
}
