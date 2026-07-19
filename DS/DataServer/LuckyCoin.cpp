// LuckyCoin.cpp
#include "Header.h"
#include "LuckyCoin.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CLuckyCoin gLuckyCoin;

void CLuckyCoin::GDLuckyCoinCountRecv(const SDHP_LUCKY_COIN_COUNT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 402)

	VALIDATE_PACKET_SIZE(SDHP_LUCKY_COIN_COUNT_RECV);

	SDHP_LUCKY_COIN_COUNT_SEND pMsg{};

	pMsg.Header.set(HEAD_LUCKY_COIN, SUB_LUCKY_COIN_COUNT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	if (!gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account))
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

		return;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		if (!gQueryManager.ExecQuery(
			"INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',0)",
			lpMsg->Account))
		{
			pMsg.Result = 1;
		}
	}
	else
	{
		pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CLuckyCoin::GDLuckyCoinRegisterRecv(const SDHP_LUCKY_COIN_REGISTER_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 402)

	VALIDATE_PACKET_SIZE(SDHP_LUCKY_COIN_REGISTER_RECV);

	SDHP_LUCKY_COIN_REGISTER_SEND pMsg{};

	pMsg.Header.set(HEAD_LUCKY_COIN, SUB_LUCKY_COIN_REGISTER, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.Slot = lpMsg->Slot;

	if (gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");
		}
		else
		{
			pMsg.Result = 1;
		}
	}
	else
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CLuckyCoin::GDLuckyCoinExchangeRecv(
	const SDHP_LUCKY_COIN_EXCHANGE_RECV* lpMsg,
	int serverIndex,
	int size)
{
#if (DATASERVER_UPDATE >= 402)

	VALIDATE_PACKET_SIZE(SDHP_LUCKY_COIN_EXCHANGE_RECV);

	SDHP_LUCKY_COIN_EXCHANGE_SEND pMsg{};

	pMsg.Header.set(HEAD_LUCKY_COIN, SUB_LUCKY_COIN_EXCHANGE, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.TradeCoin = lpMsg->TradeCoin;

	if (gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");
		}
		else
		{
			pMsg.Result = 1;
		}
	}
	else
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CLuckyCoin::GDLuckyCoinAddCountSaveRecv(
	const SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV* lpMsg,
	int serverIndex,
	int size)
{
#if (DATASERVER_UPDATE >= 402)

	VALIDATE_PACKET_SIZE(SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV);

	if (!gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account))
	{
		gQueryManager.Close();
		return;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		const DWORD newLuckyCoin =
			(lpMsg->AddLuckyCoin > static_cast<DWORD>(INT_MAX))
			? static_cast<DWORD>(INT_MAX)
			: lpMsg->AddLuckyCoin;

		gQueryManager.ExecQuery(
			"INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',%u)",
			lpMsg->Account,
			newLuckyCoin);

		gQueryManager.Close();
		return;
	}

	const DWORD luckyCoin = static_cast<DWORD>(
		gQueryManager.GetAsInteger("LuckyCoin"));

	gQueryManager.Close();

	const DWORD maxLuckyCoin = static_cast<DWORD>(INT_MAX);

	const DWORD newLuckyCoin =
		(luckyCoin >= maxLuckyCoin ||
			lpMsg->AddLuckyCoin > (maxLuckyCoin - luckyCoin))
		? maxLuckyCoin
		: (luckyCoin + lpMsg->AddLuckyCoin);

	gQueryManager.ExecQuery(
		"UPDATE LuckyCoin SET LuckyCoin=%u WHERE AccountID='%s'",
		newLuckyCoin,
		lpMsg->Account);

	gQueryManager.Close();

#endif
}

void CLuckyCoin::GDLuckyCoinSubCountSaveRecv(
	const SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV* lpMsg,
	int serverIndex,
	int size)
{
#if (DATASERVER_UPDATE >= 402)

	VALIDATE_PACKET_SIZE(SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV);

	if (!gQueryManager.ExecQuery(
		"SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",
		lpMsg->Account))
	{
		gQueryManager.Close();
		return;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',0)",
			lpMsg->Account);

		gQueryManager.Close();
		return;
	}

	const DWORD luckyCoin = static_cast<DWORD>(
		gQueryManager.GetAsInteger("LuckyCoin"));

	gQueryManager.Close();

	const DWORD newLuckyCoin =
		(lpMsg->SubLuckyCoin > luckyCoin)
		? 0
		: (luckyCoin - lpMsg->SubLuckyCoin);

	gQueryManager.ExecQuery(
		"UPDATE LuckyCoin SET LuckyCoin=%u WHERE AccountID='%s'",
		newLuckyCoin,
		lpMsg->Account);

	gQueryManager.Close();

#endif
}