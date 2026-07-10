// NpcTalk.cpp
#include "Header.h"
#include "NpcTalk.h"
#include "QueryManager.h"
#include "SocketManager.h"

CNpcTalk gNpcTalk;

void CNpcTalk::GDNpcLeoTheHelperRecv(SDHP_NPC_LEO_THE_HELPER_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 202)

	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_NPC_LEO_THE_HELPER_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_NPC, DS_SUB_NPC_LEO_LOAD, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.status = 0;

	if (gQueryManager.ExecQuery(
		"SELECT Status FROM EventLeoTheHelper WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"INSERT INTO EventLeoTheHelper (Name,Status) VALUES ('%s',0)",
				lpMsg->CharacterName);

			gQueryManager.Close();
		}
		else
		{
			pMsg.status = static_cast<BYTE>(gQueryManager.GetAsInteger("Status"));

			gQueryManager.Close();
		}
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.Header.size);

#endif
}

void CNpcTalk::GDNpcSantaClausRecv(SDHP_NPC_SANTA_CLAUS_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 401)

	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_NPC_SANTA_CLAUS_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_NPC, DS_SUB_NPC_SANTA_LOAD, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.status = 0;

	if (gQueryManager.ExecQuery(
		"SELECT Status FROM EventSantaClaus WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"INSERT INTO EventSantaClaus (Name,Status) VALUES ('%s',0)",
				lpMsg->CharacterName);

			gQueryManager.Close();
		}
		else
		{
			pMsg.status = static_cast<BYTE>(gQueryManager.GetAsInteger("Status"));

			gQueryManager.Close();
		}
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.Header.size);

#endif
}

void CNpcTalk::GDNpcLeoTheHelperSaveRecv(SDHP_NPC_LEO_THE_HELPER_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 202)

	if (lpMsg == nullptr)
	{
		return;
	}

	if (gQueryManager.ExecQuery(
		"SELECT Name FROM EventLeoTheHelper WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		gQueryManager.Close();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			gQueryManager.ExecQuery(
				"INSERT INTO EventLeoTheHelper (Name,Status) VALUES ('%s',%d)",
				lpMsg->CharacterName,
				lpMsg->status);
		}
		else
		{
			gQueryManager.ExecQuery(
				"UPDATE EventLeoTheHelper SET Status=%d WHERE Name='%s'",
				lpMsg->status,
				lpMsg->CharacterName);
		}

		gQueryManager.Close();
	}

#endif
}

void CNpcTalk::GDNpcSantaClausSaveRecv(SDHP_NPC_SANTA_CLAUS_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 401)

	if (lpMsg == nullptr)
	{
		return;
	}

	if (gQueryManager.ExecQuery(
		"SELECT Name FROM EventSantaClaus WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		gQueryManager.Close();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			gQueryManager.ExecQuery(
				"INSERT INTO EventSantaClaus (Name,Status) VALUES ('%s',%d)",
				lpMsg->CharacterName,
				lpMsg->status);
		}
		else
		{
			gQueryManager.ExecQuery(
				"UPDATE EventSantaClaus SET Status=%d WHERE Name='%s'",
				lpMsg->status,
				lpMsg->CharacterName);
		}

		gQueryManager.Close();
	}

#endif
}
