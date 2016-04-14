// ****************************************************************************
// Copyright (C) 2000-2004
//
// File 	:	qsxml.cpp
// Created 	:	Thursday, 4 October 2001
//
// Abstract :	This file implements an XML parser.
//
// ****************************************************************************

#ifndef	QSXML_H
#define	QSXML_H

#include <list> 
#include <string> 

class CQXmlNode
{ 
public: 
	CQXmlNode(const char * szTagName = "root") : m_strTagName(szTagName)	{}
	CQXmlNode(const char * szTagName , unsigned int nLength) : m_strTagName(szTagName, nLength)	{}
	virtual ~CQXmlNode()	{}

	const std::string&	GetName(void) { return m_strTagName;	}
	const std::string&	GetData(void) { return m_strDataValue;	}

	size_t				ReadXml(	const char*	 szXml);
	size_t				ReadNode(	const char*	 szXml);
	void				WriteXml(	std::string& strXml);

	CQXmlNode*			AddNode(	const char* szNode);
	CQXmlNode*			LocateNode(	const char* szNode);

	typedef		std::list<class CQXmlNode*>::const_iterator		const_iterator;
	const_iterator		GetStartNode(void) const;
	const_iterator		GetEndNode(void) const;

protected:
	size_t countWhiteSpace(	const char* szXml);
	size_t readStartTag(	const char* szXml);
	size_t validateEndTag(	const char* szXml, const char* szToken);
	size_t readNodes(		const char* szXml, CQXmlNode& rParent);
	
protected:
	std::string					m_strTagName;			// minus the '<','</','>' 
	std::string					m_strDataValue;			// Is a data node.
	std::list<class CQXmlNode*>	m_listXmlTag;			// Contains the tags ...

private:
	CQXmlNode( const CQXmlNode& rhs);						// disabled. 
	// operator=(const CQXmlNode& rhs);						// disabled. 
}; 

#endif

// ****************************************************************************
// Copyright (C) 2000-2004
// ****************************************************************************

