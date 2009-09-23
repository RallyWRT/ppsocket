#include "stdafx.h"
#include ".\strfuns.h"
#include <assert.h>
using namespace std;

StrFuns::StrFuns(void)
{
}

StrFuns::~StrFuns(void)
{
}

void StrFuns::Tokenize( const string& str, vector<string>& tokens, const string& delimiters/*=" ,\t\n"*/ )
{
	//skip delimiters at beginning
	string::size_type lastPos=str.find_first_not_of(delimiters,0);
	//find first "non-delimiter".
	string::size_type pos=str.find_first_of(delimiters,lastPos);

	while(string::npos!=lastPos)
	{
		if (string::npos==pos)
		{
			string strSub = str.substr(lastPos,str.size()-lastPos);
			tokens.push_back(strSub);
			break;
		}
		//found a token, add it to the vector
		assert(pos-lastPos>0);
		assert(pos-lastPos<str.size());
		string strSub = str.substr(lastPos,pos-lastPos);
		tokens.push_back(strSub);
		//skip delimiters. Note the "not_of"
		lastPos=str.find_first_not_of(delimiters,pos);
		//find next "non-delimiter"
		pos=str.find_first_of(delimiters,lastPos);
	}
}