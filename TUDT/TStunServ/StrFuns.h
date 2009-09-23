#pragma once
#include <string>
#include <vector>

class StrFuns
{
public:
	StrFuns(void);
	~StrFuns(void);

	static void Tokenize(const std::string& str,
		std::vector<std::string>& tokens,
		const std::string& delimiters=" ,\t\n");
};
