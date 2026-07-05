// BadSyntax.cpp
#include "Header.h"
#include "BadSyntax.h"
#include "ScriptParser.h"
#include "Util.h"

CBadSyntax gBadSyntax;

// Construction/Destruction

void CBadSyntax::Load(const char* path)
{
	CScriptParser gScriptParser;

	if (gScriptParser.SetBuffer(path) == 0)
	{
		gUtil.ErrorMessageBox(gScriptParser.GetLastError());
		return;
	}

	m_BadSyntaxInfo.clear();

	try
	{
		while (true)
		{
			if (gScriptParser.GetToken() == TOKEN_END)
			{
				break;
			}

			if (std::strcmp(gScriptParser.GetString(), "end") == 0)
			{
				break;
			}

			m_BadSyntaxInfo.push_back({ gScriptParser.GetString() });
		}
	}
	catch (...)
	{
		gUtil.ErrorMessageBox(gScriptParser.GetLastError());
	}
}

bool CBadSyntax::CheckSyntax(const char* text) const
{
	if (text == nullptr)
	{
		return false;
	}

	for (const auto& syntax : m_BadSyntaxInfo)
	{
		if (std::strstr(text, syntax.Syntax.c_str()) != nullptr)
		{
			return false;
		}
	}

	return true;
}
