// ****************************************************************************
// Copyright (C) 2000-2004
//
// File 	:	StringLoader.cpp
// Created 	:	Tuesday, 24 July 2001
// Author 	:	Julien Crawford [JMC]
//
// Abstract :	This file implements a class for loading comma separated
//				strings form the .RC file.
//
// ****************************************************************************


#include "compiler.h"			// always included
#include "StringLoader.h"		// class def.

#include <cstdlib>				// for size_t
#include <malloc.h>				// for alloca
#include <windows.h>			// for HINSTANCE

#include "WinMain.h"			// for hInst

QS::CStringLoader::CStringLoader(
		long		lStringId,
		size_t		nStrLen		// = MAX_LOADSTRING_LENGTH
		)
{
	char*	szBuffer = 0;

	try
	{
		// alloca throws an exception on failure.
		szBuffer = (char*)alloca((nStrLen+1)*sizeof(char));
	}	catch(...) {}
	if (szBuffer == NULL)
		return;	// Can not proceed!

	
	// ************************************************************
	// If the string has been deleted from the RC file then we can
	// not proceed, just return NULL.
	// ************************************************************
	int iNChars = ::LoadString( hInst, lStringId, szBuffer, nStrLen );
	if (!iNChars)
		return;
	szBuffer[iNChars] = NULL;	// Ensure NULL termination


	// ************************************************************
	// Traverse the string.
	// ************************************************************
	char*	szString = szBuffer;
	for (int i=0;i<iNChars;++i)
	{
		if (szBuffer[i] == ',')
		{
			szBuffer[i] = NULL;
			m_vstr.push_back(szString);
			szString = szBuffer+i+1;
		}
	}

	m_vstr.push_back(szString);
}

QS::CStringLoader::~CStringLoader()
{
}

const string&
QS::CStringLoader::operator[](size_t nIndex) const
{
	DEF_PRECONDITION2(nIndex<m_vstr.size(),"Operator out of range");

	return m_vstr[nIndex];
}

size_t
QS::CStringLoader::size(void) const
{
	return m_vstr.size();
}

// ****************************************************************************
// Copyright (C) 2000-2004
// ****************************************************************************

