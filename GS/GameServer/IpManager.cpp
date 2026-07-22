// IpManager.cpp
#include "Header.h"
#include "IpManager.h"
#include "ServerInfo.h"
#include "Log.h"
#include "Util.h"

CIpManager gIpManager;

bool CIpManager::CheckIpAddress(char* IpAddress)
{
	auto it = m_IpAddressInfo.find(IpAddress);

	if (it == m_IpAddressInfo.end())
	{
		return (gServerInfo.m_MaxIpConnection != 0);
	}

	IP_ADDRESS_INFO& info = it->second;
	const ULONGLONG currentTick = GetTickCount64();

	if (info.IpBlocked != 0)
	{
		if (info.IpRealUser != 0 || (currentTick - info.IpBlockedTime) > gServerInfo.m_IpConnectionBlockedTime)
		{
			Log.ToDispAndFile(LOG_RED, LogType::HACK, "[IpManager - CheckIpAddress] IP desbaneada: %s", IpAddress);
			info.IpBlocked = 0;
			info.IpBlockedTime = 0;
		}
		else
		{
			return false;
		}
	}

	if (info.IpRealUser == 0 && info.IpTick != 0 && (currentTick - info.IpTick) < 1000ULL)
	{
		info.IpBlocked = 1;
		info.IpBlockedTime = currentTick;

		Log.ToDispAndFile(LOG_RED, LogType::HACK, "[IpManager - CheckIpAddress] Intento de flood detectado - Banear IP: %s", IpAddress);
		return false;
	}

	info.IpTick = currentTick;

	if (info.IpAddressCount >= gServerInfo.m_MaxIpConnection)
	{
		if (info.IpRealUser == 0)
		{
			info.IpBlocked = 1;
			info.IpBlockedTime = currentTick;

			Log.ToDispAndFile(LOG_RED, LogType::HACK, "[IpManager - CheckIpAddress] Intento de flood detectado - Banear IP: %s", IpAddress);
		}

		return false;
	}

	return true;
}

void CIpManager::InsertIpAddress(char* IpAddress)
{
	auto it = m_IpAddressInfo.find(IpAddress);

	if (it != m_IpAddressInfo.end())
	{
		++it->second.IpAddressCount;
		return;
	}

	IP_ADDRESS_INFO info{};

	strcpy_s(info.IpAddress, IpAddress);

	info.IpAddressCount = 1;
	info.IpTick = GetTickCount64();
	info.IpBlocked = 0;
	info.IpBlockedTime = 0;
	info.IpFloodAttemps = 0;
	info.IpFloodLastTime = 0;
	info.IpRealUser = 0;

	m_IpAddressInfo.emplace(IpAddress, info);
}

void CIpManager::RemoveIpAddress(char* IpAddress)
{
	auto it = m_IpAddressInfo.find(IpAddress);

	if (it == m_IpAddressInfo.end())
	{
		return;
	}

	IP_ADDRESS_INFO& info = it->second;

	if (info.IpBlocked == 0 && --info.IpAddressCount == 0)
	{
		m_IpAddressInfo.erase(it);
	}
}

void CIpManager::InsertRealUser(char* IpAddress)
{
	auto it = m_IpAddressInfo.find(IpAddress);

	if (it != m_IpAddressInfo.end())
	{
		it->second.IpRealUser = 1;
	}
}
