// NpcTalk.cpp
#include "Header.h"
#include "NpcTalk.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CNpcTalk gNpcTalk;

void CNpcTalk::GDNpcLeoTheHelperRecv(const SDHP_NPC_LEO_THE_HELPER_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 202)

	VALIDATE_PACKET_SIZE(SDHP_NPC_LEO_THE_HELPER_RECV);

	SDHP_NPC_LEO_THE_HELPER_SEND pMsg{};
	pMsg.Header.set(HEAD_NPC, SUB_NPC_LEO_LOAD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Status = 0;

	const bool dataFound =
		gQueryManager.ExecQuery(
			"SELECT Status FROM EventLeoTheHelper WHERE Name='%s'",
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.Status = static_cast<BYTE>(
			gQueryManager.GetAsInteger("Status"));
	}

	gQueryManager.Close();

	if (!dataFound)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO EventLeoTheHelper (Name,Status) VALUES ('%s',0)",
			lpMsg->CharacterName);

		gQueryManager.Close();
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CNpcTalk::GDNpcSantaClausRecv(const SDHP_NPC_SANTA_CLAUS_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 401)

	VALIDATE_PACKET_SIZE(SDHP_NPC_SANTA_CLAUS_RECV);

	SDHP_NPC_SANTA_CLAUS_SEND pMsg{};
	pMsg.Header.set(HEAD_NPC, SUB_NPC_SANTA_LOAD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Status = 0;

	const bool dataFound =
		gQueryManager.ExecQuery(
			"SELECT Status FROM EventSantaClaus WHERE Name='%s'",
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.Status = static_cast<BYTE>(
			gQueryManager.GetAsInteger("Status"));
	}

	gQueryManager.Close();

	if (!dataFound)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO EventSantaClaus (Name,Status) VALUES ('%s',0)",
			lpMsg->CharacterName);

		gQueryManager.Close();
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CNpcTalk::GDNpcLeoTheHelperSaveRecv(const SDHP_NPC_LEO_THE_HELPER_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 202)

	VALIDATE_PACKET_SIZE(SDHP_NPC_LEO_THE_HELPER_SAVE_RECV);

	gQueryManager.ExecQuery(
		"UPDATE EventLeoTheHelper SET Status=%d WHERE Name='%s'",
		lpMsg->Status,
		lpMsg->CharacterName);

	const SQLLEN affectedRows = gQueryManager.GetAffectedRows();

	gQueryManager.Close();

	if (affectedRows == 0)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO EventLeoTheHelper (Name,Status) VALUES ('%s',%d)",
			lpMsg->CharacterName,
			lpMsg->Status);

		gQueryManager.Close();
	}

#endif
}

void CNpcTalk::GDNpcSantaClausSaveRecv(const SDHP_NPC_SANTA_CLAUS_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 401)

	VALIDATE_PACKET_SIZE(SDHP_NPC_SANTA_CLAUS_SAVE_RECV);

	gQueryManager.ExecQuery(
		"UPDATE EventSantaClaus SET Status=%d WHERE Name='%s'",
		lpMsg->Status,
		lpMsg->CharacterName);

	const SQLLEN affectedRows = gQueryManager.GetAffectedRows();

	gQueryManager.Close();

	if (affectedRows == 0)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO EventSantaClaus (Name,Status) VALUES ('%s',%d)",
			lpMsg->CharacterName,
			lpMsg->Status);

		gQueryManager.Close();
	}

#endif
}
