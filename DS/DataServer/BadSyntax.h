// BadSyntax.h
#pragma once

#include <string>
#include <vector>

struct BAD_SYNTAX_INFO
{
	std::string syntax;
};

class CBadSyntax
{
public:
	CBadSyntax() = default;
	~CBadSyntax() = default;

	void Load(const char* path);
	bool CheckSyntax(const char* text) const;

private:
	std::vector<BAD_SYNTAX_INFO> m_BadSyntaxInfo;
};

extern CBadSyntax gBadSyntax;