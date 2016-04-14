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

#ifdef	__cplusplus
extern "C" {
#endif


void * UnzipOpen( const char *zipfilename, const char *filename );


int UnzipGetData( void *zFile, unsigned char *buffer, size_t bytestoread );


int UnzipClose( void *zFile );


#ifdef	__cplusplus
}
#endif