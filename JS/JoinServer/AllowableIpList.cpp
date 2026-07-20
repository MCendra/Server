// AllowableIpList.cpp
#include "Header.h"
#include "AllowableIpList.h"
#include "Log.h"
#include "ScriptParser.h"
#include "Util.h"

CAllowableIpList gAllowableIpList;

void CAllowableIpList::Load(const char* path)
{
	CScriptParser scriptParser;

	if (!scriptParser.SetBuffer(path))
	{
		Log.ToFile(LogType::GENERAL, scriptParser.GetLastError());
		return;
	}

	m_AllowableIpInfo.clear();

	try
	{
		while (scriptParser.GetToken() != TOKEN_END)
		{
			const int section = scriptParser.GetNumber();

			if (section != 0)
			{
				continue;
			}

			while (true)
			{
				if (strcmp("end", scriptParser.GetAsString()) == 0)
				{
					break;
				}

				ALLOWABLE_IP_INFO info{};

				const char* ip = scriptParser.GetString();

				if (ip[0] == '\0')
				{
					continue;
				}

				strcpy_s(info.IpAddr, ip);

				m_AllowableIpInfo.try_emplace(info.IpAddr, info);
			}
		}
	}
	catch (...)
	{
		Log.ToFile(LogType::GENERAL, scriptParser.GetLastError());
	}
}

bool CAllowableIpList::CheckAllowableIp(const char* ip) const
{
	return (m_AllowableIpInfo.find(ip) != m_AllowableIpInfo.end());
}
