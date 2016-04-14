
#ifndef	STRINGLOADER_H
#define	STRINGLOADER_H

#define	MAX_LOADSTRING_LENGTH	511

#include <string>
#include <vector>
using namespace std;

namespace QS {

class CStringLoader
{
public:
	CStringLoader(long lStringId, size_t nStrLen=MAX_LOADSTRING_LENGTH);
	virtual ~CStringLoader();

public:
	const string&	operator[](size_t nIndex) const;
	size_t			size(void) const;

protected:
	vector<string>	m_vstr;
};

}	// end QS namespace

#endif