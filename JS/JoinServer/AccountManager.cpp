// AccountManager.cpp:
#include "Header.h"
#include "AccountManager.h"
#include "Log.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "Util.h"

CAccountManager gAccountManager;

// Construction/Destruction

CAccountManager::CAccountManager()
{

}

CAccountManager::~CAccountManager()
{

}

void CAccountManager::DisconnectProc()
{
	std::vector<ACCOUNT_INFO> disconnectList;

	{
		CCriticalSection::CLock lock(this->m_critical);

		for (auto it = this->m_AccountInfo.begin(); it != this->m_AccountInfo.end();)
		{
			if (it->second.MapServerMove == 0 || (GetTickCount64() - it->second.MapServerMoveTime) < 30000)
			{
				++it;
				continue;
			}

			disconnectList.emplace_back(it->second);
			it = this->m_AccountInfo.erase(it);
		}
	}

	for (const auto& account : disconnectList)
	{
		Log.ToFile(
			LogType::ACCOUNT, "[AccountInfo - DisconnectProc] Cuenta desconectada: (Account: %s, IpAddress: %s, GameServerCode: %d)", account.Account, account.IpAddress, account.GameServerCode);

		gQueryManager.ExecQuery("EXEC WZ_DISCONNECT_MEMB '%s'", account.Account);
		gQueryManager.Close();

		JGExternalDisconnectAccountSend(account.GameServerCode, account.UserIndex, account.Account);
	}
}

void CAccountManager::ClearServerAccountInfo(int serverCode)
{
	std::vector<ACCOUNT_INFO> disconnectList;

	{
		CCriticalSection::CLock lock(this->m_critical);

		for (auto it = this->m_AccountInfo.begin(); it != this->m_AccountInfo.end();)
		{
			if (it->second.GameServerCode != serverCode)
			{
				++it;
				continue;
			}

			disconnectList.emplace_back(it->second);
			it = this->m_AccountInfo.erase(it);
		}
	}

	for (const auto& account : disconnectList)
	{
		Log.ToFile(LogType::ACCOUNT, "[AccountInfo - ClearServerAccountInfo] Cuenta desconectada: (Account: %s, IpAddress: %s, GameServerCode: %d)", account.Account, account.IpAddress, account.GameServerCode);

		gQueryManager.ExecQuery("EXEC WZ_DISCONNECT_MEMB '%s'", account.Account);

		gQueryManager.Close();
	}
}

bool CAccountManager::GetAccountInfo(ACCOUNT_INFO* lpAccountInfo, const char* account)
{
	CCriticalSection::CLock lock(this->m_critical);

	std::string acc(account);

	std::transform(acc.begin(), acc.end(), acc.begin(), CheckAccountCaseSensitive);

	auto it = this->m_AccountInfo.find(acc);

	if (it == this->m_AccountInfo.end())
	{
		return false;
	}

	*lpAccountInfo = it->second;
	return true;
}

void CAccountManager::InsertAccountInfo(const ACCOUNT_INFO& AccountInfo)
{
	CCriticalSection::CLock lock(this->m_critical);

	std::string acc(AccountInfo.Account);

	std::transform(acc.begin(),acc.end(),acc.begin(),CheckAccountCaseSensitive);

	m_AccountInfo[acc] = AccountInfo;
}

void CAccountManager::RemoveAccountInfo(const ACCOUNT_INFO& AccountInfo)
{
	CCriticalSection::CLock lock(this->m_critical);

	std::string acc(AccountInfo.Account);

	std::transform(acc.begin(),acc.end(),acc.begin(),CheckAccountCaseSensitive);

	m_AccountInfo.erase(acc);
}

long CAccountManager::GetAccountCount()
{
	CCriticalSection::CLock lock(this->m_critical);

	return static_cast<long>(this->m_AccountInfo.size());;
}

LONG CheckAccountCaseSensitive(int value)
{
	// Si CaseSensitive == 0 (no distingue mayúsculas), normaliza a minúscula.
	// Si CaseSensitive == 1, deja el valor tal cual.
	return (CaseSensitive == 0)	? tolower((unsigned char)value) : value;
}

DWORD MakeAccountKey(const char* account)
{
	size_t size = strlen(account);
	DWORD key = 0;

	for (size_t n = 0; n < size; n++)
	{
		key += static_cast<BYTE>(account[n]) + 17;
	}

	return ((key + ((10 - size) * 17)) % 256);
}
