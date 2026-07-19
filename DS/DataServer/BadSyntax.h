// BadSyntax.h
#pragma once
#include <string>
#include <vector>

#pragma pack(push,1)
struct BAD_SYNTAX_INFO
{
	std::string Syntax;
};
#pragma pack(pop)

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