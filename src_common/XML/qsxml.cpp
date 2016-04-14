
#pragma warning (disable:4786)	/* identifier was truncated to 255 characters in the debug information */
#include "qsxml.h"
#include <ctype.h>
const char*		TAG_SEPARATOR_TOKEN		= "/"; 
const char*		TAG_INFO_TOKEN			= ":"; 

// ****************************************************************************
// Method:		CQXmlNode::LocateNode
//
// Abstract:	This method returns the child node that matches the topdown tag 
//				tree in szTag (TAG_SEPARATOR_TOKEN separated)
//
// Declaration: CQXmlNode::LocateNode(const char* szTag)
//
// Arguments:	szTag a list of Tags separated by TAG_SEPARATOR_TOKEN.
//				each is expected to be in the object as a topdown tree.//
//
// Returns:		a pointer to the found tag or null. 
//
// ****************************************************************************
CQXmlNode*
CQXmlNode::LocateNode(const char* szTag)
{
	CQXmlNode*	pTag = NULL;
	const char* szToken = strstr(szTag, TAG_SEPARATOR_TOKEN);
	if (!szToken)
		szToken = szTag + strlen(szTag);

	std::string	strTag(szTag, szToken-szTag);

	std::list<class CQXmlNode*>::iterator it;
	for (it=m_listXmlTag.begin(); it!=m_listXmlTag.end(); ++it)
	{
		CQXmlNode* pNode = (*it);				// Create local variable to aid debugging.
		if (pNode->m_strTagName == strTag)
		{
			pTag = pNode;
			break;
		}
	}

	if (*szToken && pTag)
		return pTag->LocateNode(szToken+1);
	return pTag;
}

// ****************************************************************************
// Method:		CQXmlNode::AddNode
//
// Abstract:	This method breaks the 'szTag' string into the tags (separated
//				by TAG_SEPARATOR_TOKEN and adds them to the tree if they do
//				not already exist. It then returns the 'found' node.
//
// Declaration: CQXmlNode::AddNode(const char* szTag)
//
// Arguments:	szTag a list of Tags separated by TAG_SEPARATOR_TOKEN.
//				each is expected to be in the object as a topdown tree.
//
// Returns:		a pointer to the found/created tag or null. 
//
// ****************************************************************************
CQXmlNode*
CQXmlNode::AddNode(const char* szTag)
{
	// I am adding this to this node subtree (Not the node itself).
	CQXmlNode*	pTag = NULL;
	const char* szToken = strstr(szTag, TAG_SEPARATOR_TOKEN);
	if (!szToken)
		szToken = szTag + strlen(szTag);

	std::string	strTag(szTag, szToken-szTag);

	std::list<class CQXmlNode*>::iterator it;
	for (it=m_listXmlTag.begin(); it!=m_listXmlTag.end(); ++it)
	{
		if ((*it)->m_strTagName == strTag)
		{
			pTag = (*it);
			break;
		}
	}

	if (!pTag)
	{
		pTag = new CQXmlNode(strTag.c_str());
		m_listXmlTag.push_back(pTag);
	}

	if (*szToken)
		return pTag->AddNode(szToken+1);
	return pTag;
}


// ****************************************************************************
// Method:		CQXmlNode::GetStartNode
//
// Abstract:	This method supplies access to the subnode of the Xml tree.
//
// Declaration: CQXmlNode::GetStartNode(void)
//
// Arguments:	none
//
// Returns:		The iterator to the start of the list.
//
// ****************************************************************************
CQXmlNode::const_iterator	
CQXmlNode::GetStartNode(void) const
{
	return m_listXmlTag.begin();
}


// ****************************************************************************
// Method:		CQXmlNode::GetEndNode
//
// Abstract:	This method supplies access to the subnode of the Xml tree.
//
// Declaration: CQXmlNode::GetEndNode(void)
//
// Arguments:	none
//
// Returns:		The end() iterator of the list.
//
// ****************************************************************************
CQXmlNode::const_iterator	
CQXmlNode::GetEndNode(void) const
{
	return m_listXmlTag.end();
}


// ****************************************************************************
// Method:		CQXmlNode::countWhiteSpace
//
// Abstract:	This privte method allows us to have spaces separating tags.
//
// Declaration: CQXmlNode::countWhiteSpace(const char* szXml)
//
// Arguments:	The location in the xml which we are parsing where we can expect
//				to have white-space.
//
// Returns:		The number of white-space characters.
//
// ****************************************************************************
size_t	CQXmlNode::countWhiteSpace(const char* szXml)
{
	const char* c;
	for (c=szXml; isspace(*c); ++c);

	return c-szXml;
}

// ****************************************************************************
// Method:		size_t CQXmlNode::readStartTag
//
// Abstract:	This method verifies that the xml we are currently parsing is
//				actually a Start tag and returns the number of characters it
//				consumes includive of the '<' and '>'.
//
// Declaration: CQXmlNode::readStartTag(const char* szXml)
//
// Arguments:	The location in the xml which we are parsing.
//
// Returns:		The number of characters consumed.
// ****************************************************************************
size_t	CQXmlNode::readStartTag(const char* szXml)
{
	const char* sz = szXml; 

	if (*szXml != '<') 
		return -1; 
	++szXml;

	// Ensure its not an EndTag.
	if (*szXml == '/')
		return -1;

	for (; *szXml && *szXml != '>';++szXml);
		
	if (*szXml != '>')
		return -1; 

	++szXml;		// passed to '>'. 

	return szXml-sz; 
} 

// ****************************************************************************
// Method:		size_t CQXmlNode::validateEndTag
//
// Abstract:	This method verifies that the xml we are currently parsing is
//				actually a End tag and returns the number of characters it
//				consumes includive of the '<' and '>'.
//
// Declaration: CQXmlNode::validateEndTag(const char* szXml, const char* szToken)
//
// Arguments:	szXml:		The location in the xml which we are parsing.
//				szToken:	The Tag we are expecting to end.
//
// Returns:		The number of characters consumed.
// ****************************************************************************
size_t	CQXmlNode::validateEndTag(const char* szXml, const char* szToken)
{
	const char* sz = szXml;

	if (*szXml != '<')
		return -1;
	++szXml;
	if (*szXml != '/')
		return -1;
	++szXml;

	for (; *szToken; ++szToken, ++szXml)
	{
		if (*szToken != *szXml)
			return -1;
	}

	if (*szXml != '>')
		return -1;
	++szXml;

	return (szXml-sz);
}
	
// ****************************************************************************
// Method:		size_t CQXmlNode::readNodes
//
// Abstract:	This method parses the xml and builds the tree of tags/data
//				nodes.  If we get a tag then we expect N tags, otherwise we
//				expect data as a string up until the close tag.
//
// Declaration: CQXmlNode::readNodes(const char* szXml, CQXmlNode& rParent)
//
// Arguments:	szXml:		The location in the xml which we are parsing.
//				rParent:	The tag that the current xml will be comtained in.
//
// Returns:		The number of characters consumed.
// ****************************************************************************
size_t	CQXmlNode::readNodes(const char* szXml, CQXmlNode& rParent) 
{ 
	size_t		nChars(0); 
	size_t		nOffset(0); 

	nOffset += countWhiteSpace(szXml+nOffset);

	if (*(szXml+nOffset) == '<')	// Tag list.
	{
		CQXmlNode*	pCQXmlNode = 0;
		nOffset = 0;
		for (;;)
		{
			nOffset += countWhiteSpace(szXml+nOffset);

			nChars	= readStartTag(szXml+nOffset);
			if (nChars == -1)
				return nOffset;
			pCQXmlNode = new CQXmlNode(szXml+nOffset+1,nChars-2);
			nOffset	+= nChars; 

			nChars	= readNodes(szXml+nOffset, *pCQXmlNode);
			if (nChars == -1)
				return -1;
			nOffset	+= nChars; 

			nOffset += countWhiteSpace(szXml+nOffset);
			nChars	= validateEndTag(szXml+nOffset, pCQXmlNode->GetName().c_str());
			if (nChars == -1)
				return -1;
			nOffset	+= nChars; 

			rParent.m_listXmlTag.push_back(pCQXmlNode);
		}
	} 
	else // Data within tag.
	{
		const char*		szParent = rParent.m_strTagName.c_str();
		const char* c;
		for (c=szXml; ;++c)
		{
			if (*c == '<' && validateEndTag(c, szParent) != -1)
				break;
			if (*c == '\\')
				++c;
			if (*c == 0)
				return -1;
		}

		rParent.m_strDataValue = std::string(szXml,c-szXml);

		nOffset = (c - szXml);
	}

	return nOffset; 
} 


// ****************************************************************************
// Method:		CQXmlNode::ReadXml
//
// Abstract:	This method loads full xml into a single node and as such its
//				own name is preserved (defaulted to 'root').  This way we can
//				have a list of tags (which may contain subtags) which will be
//				loaded into this node.
//
// Declaration: CQXmlNode::ReadXml(const char* szXml)
//
// Arguments:	szXml:		The location in the xml which we are parsing.
//
// Returns:		The number of characters consumed.
//
// ****************************************************************************
size_t	CQXmlNode::ReadXml(const char* szXml)
{
	size_t		nChars(0); 
	size_t		nOffset(0); 

	nChars = readNodes(szXml, *this);
	if (nChars == -1)
		return -1;

	return nChars;
}


// ****************************************************************************
// Method:		CQXmlNode::ReadNode
//
// Abstract:	This method loads one tag of xml into a single node.  Its own
//				id will be the 1st tag found.
//
// Declaration: CQXmlNode::ReadNode(const char* szXml)
//
// Arguments:	szXml:		The location in the xml which we are parsing.
//
// Returns:		The number of characters consumed.
//
// ****************************************************************************
size_t	CQXmlNode::ReadNode(const char* szXml)
{
	size_t		nChars(0); 
	size_t		nOffset(0); 

	nOffset += countWhiteSpace(szXml);
	nChars	= readStartTag(szXml+nOffset);
	if (nChars == -1)
		return nOffset;
	m_strTagName = std::string(szXml+nOffset+1,nChars-2);
	nOffset	+= nChars; 

	nChars	= readNodes(szXml+nOffset, *this);
	if (nChars == -1)
		return -1;
	nOffset	+= nChars; 

	nOffset += countWhiteSpace(szXml+nOffset);
	nChars	= validateEndTag(szXml+nOffset, m_strTagName.c_str());
	if (nChars == -1)
		return -1;
	nOffset	+= nChars; 

	return nOffset;
}

// ****************************************************************************
// Method:		CQXmlNode::WriteXml
//
// Abstract:	This method writes out the xml to a std::string.
//
// Declaration: CQXmlNode::WriteXml(std::string& strXml)
//
// Arguments:	strXml is the destination of the xml.
//
// ****************************************************************************
void	CQXmlNode::WriteXml(std::string& strXml)
{
	strXml += "<";
	strXml += m_strTagName;
	strXml += ">";

	if (m_strDataValue.size())
	{
		strXml += m_strDataValue;
	}
	else
	{
		std::list<class CQXmlNode*>::iterator it;
		for (it=m_listXmlTag.begin(); it!=m_listXmlTag.end(); ++it)
		{
			CQXmlNode* p = *it;
			p->WriteXml(strXml);
		}
	}

	strXml += "</";
	strXml += m_strTagName;
	strXml += ">";
}
