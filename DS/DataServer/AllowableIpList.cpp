// AllowableIpList.cpp
#include "Header.h"
#include "AllowableIpList.h"
#include "Log.h"
#include "ScriptParser.h"
#include "Util.h"

CAllowableIpList gAllowableIpList;

void CAllowableIpList::Load(const char* path)
{
	CScriptParser gScriptParser;

	if (!gScriptParser.SetBuffer(path))
	{
		Log.ToFile(LogType::GENERAL, gScriptParser.GetLastError());
		return;
	}

	m_AllowableIpInfo.clear();

	try
	{
		while (true)
		{
			if (gScriptParser.GetToken() == TOKEN_END)
			{
				break;
			}

			int section = gScriptParser.GetNumber();

			while (true)
			{
				if (section != 0)
				{
					break;
				}

				if (strcmp("end", gScriptParser.GetAsString()) == 0)
				{
					break;
				}

				ALLOWABLE_IP_INFO info{};

				strcpy_s(info.IpAddr, gScriptParser.GetString());

				m_AllowableIpInfo.emplace(info.IpAddr, info);
			}
		}
	}
	catch (...)
	{
		Log.ToFile(LogType::GENERAL, gScriptParser.GetLastError());
	}
}

bool CAllowableIpList::CheckAllowableIp(const char* ip) const
{
	return (m_AllowableIpInfo.find(ip) != m_AllowableIpInfo.end());
}
