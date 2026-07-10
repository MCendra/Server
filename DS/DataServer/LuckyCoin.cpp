// LuckyCoin.cpp
#include "Header.h"
#include "LuckyCoin.h"
#include "QueryManager.h"
#include "SocketManager.h"

CLuckyCoin gLuckyCoin;

void CLuckyCoin::GDLuckyCoinCountRecv(SDHP_LUCKY_COIN_COUNT_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 402)

	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_LUCKY_COIN_COUNT_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_LUCKY_COIN, DS_SUB_LUCKY_COIN_COUNT, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.Result = 0;
	pMsg.LuckyCoin = 0;

	if (!gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		if (!gQueryManager.ExecQuery(
			"INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',0)",
			lpMsg->Account))
		{
			gQueryManager.Close();

			pMsg.Result = 1;
		}
		else
		{
			gQueryManager.Close();
		}
	}
	else
	{
		pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);

#endif
}

void CLuckyCoin::GDLuckyCoinRegisterRecv(SDHP_LUCKY_COIN_REGISTER_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 402)

	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_LUCKY_COIN_REGISTER_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_LUCKY_COIN, DS_SUB_LUCKY_COIN_REGISTER, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.Result = 0;
	pMsg.Slot = lpMsg->Slot;
	pMsg.LuckyCoin = 0;

	if (!gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;
	}
	else
	{
		pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);

#endif
}

void CLuckyCoin::GDLuckyCoinExchangeRecv(SDHP_LUCKY_COIN_EXCHANGE_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 402)

	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_LUCKY_COIN_EXCHANGE_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_LUCKY_COIN, DS_SUB_LUCKY_COIN_EXCHANGE, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.Result = 0;
	pMsg.TradeCoin = lpMsg->TradeCoin;
	pMsg.LuckyCoin = 0;

	if (!gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;
	}
	else
	{
		pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);

#endif
}

void CLuckyCoin::GDLuckyCoinAddCountSaveRecv(SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 402)

	if (lpMsg == nullptr)
	{
		return;
	}

	if (!gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',%d)",
			lpMsg->Account,
			lpMsg->AddLuckyCoin);

		gQueryManager.Close();
	}
	else
	{
		const DWORD luckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();

		const DWORD newLuckyCoin =
			((luckyCoin + lpMsg->AddLuckyCoin) > static_cast<DWORD>(INT_MAX))
			? static_cast<DWORD>(INT_MAX)
			: (luckyCoin + lpMsg->AddLuckyCoin);

		gQueryManager.ExecQuery(
			"UPDATE LuckyCoin SET LuckyCoin=%d WHERE AccountID='%s'",
			newLuckyCoin,
			lpMsg->Account);

		gQueryManager.Close();
	}

#endif
}

void CLuckyCoin::GDLuckyCoinSubCountSaveRecv(SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 402)

	if (lpMsg == nullptr)
	{
		return;
	}

	if (!gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',%d)",
			lpMsg->Account,
			0);

		gQueryManager.Close();
	}
	else
	{
		const DWORD luckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();

		const DWORD newLuckyCoin =
			(lpMsg->SubLuckyCoin > luckyCoin)
			? 0
			: (luckyCoin - lpMsg->SubLuckyCoin);

		gQueryManager.ExecQuery(
			"UPDATE LuckyCoin SET LuckyCoin=%d WHERE AccountID='%s'",
			newLuckyCoin,
			lpMsg->Account);

		gQueryManager.Close();
	}

#endif
}