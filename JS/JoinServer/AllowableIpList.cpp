// AllowableIpList.cpp
#include "Header.h"
#include "AllowableIpList.h"
#include "Log.h"
#include "ScriptParser.h"
#include "Util.h"

CAllowableIpList gAllowableIpList;

// Construction/Destruction

CAllowableIpList::CAllowableIpList() = default;

CAllowableIpList::~CAllowableIpList() = default;

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
		while (true)
		{
			if (scriptParser.GetToken() == TOKEN_END)
			{
				break;
			}

			int section = scriptParser.GetNumber();

			while (true)
			{
				if (section != 0)
				{
					break;
				}

				if (strcmp("end", scriptParser.GetAsString()) == 0)
				{
					break;
				}

				ALLOWABLE_IP_INFO info{};

				strcpy_s(info.IpAddr, scriptParser.GetString());

				m_AllowableIpInfo.emplace(info.IpAddr, info);
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
