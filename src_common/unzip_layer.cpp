// ****************************************************************************
// Copyright (C) 2000-2004
//
// File 	:	unzip_layer.c
// Created 	:	Friday, 16 Aug 2001
// Author 	:	Raul Sobon
//
// Abstract :	This is the layer api used to read files from a zip file as if
//              its a normal file stream.
//
// ****************************************************************************

// ****************************************************************************
// Default Includes.
// ****************************************************************************
#include "FWA.h"

#include "unzip.h"
#include "unzip_layer.h"

void * UnzipOpen( const char *zipfilename, const char *filename )
{
	void * zFile;
	int zStatus;

	zFile = unzOpen( zipfilename );
	zStatus = unzGoToFirstFile( zFile );
	zStatus = unzOpenCurrentFile( zFile );
	return zFile;
}



int UnzipGetData( void *zFile, unsigned char *buffer, size_t bytestoread )
{
	int zStatus;
	zStatus = unzReadCurrentFile( zFile , buffer, bytestoread );
	if ( zStatus <= 0 )
	{
		unzCloseCurrentFile( zFile );
		if ( unzGoToNextFile( zFile ) == UNZ_OK )
		{
			if ( unzOpenCurrentFile( zFile ) == UNZ_OK )
				zStatus = UnzipGetData( zFile, buffer, bytestoread );
		}
	}
	return zStatus;
}


int UnzipClose( void *zFile )
{
	return unzClose( zFile );
}
