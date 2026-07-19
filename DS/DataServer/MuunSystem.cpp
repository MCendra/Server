// MuunSystem.cpp
#include "Header.h"
#include "MuunSystem.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CMuunSystem gMuunSystem;

void CMuunSystem::GDMuunInventoryRecv(const SDHP_MUUN_INVENTORY_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 803)

	VALIDATE_PACKET_SIZE(SDHP_MUUN_INVENTORY_RECV);

	SDHP_MUUN_INVENTORY_SEND pMsg{};

	pMsg.Header.set(
		HEAD_MUUN_SYSTEM,
		SUB_MUUN_INVENTORY,
		sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	std::memset(
		pMsg.MuunInventory,
		0xFF,
		sizeof(pMsg.MuunInventory));

	if (gQueryManager.ExecQuery(
		"SELECT Items FROM MuunInventory WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA &&
			sqlRet != SQL_NULL_DATA)
		{
			gQueryManager.GetAsBinary(
				"Items",
				pMsg.MuunInventory[0],
				sizeof(pMsg.MuunInventory));
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CMuunSystem::GDMuunInventorySaveRecv(const SDHP_MUUN_INVENTORY_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 803)

	VALIDATE_PACKET_SIZE(SDHP_MUUN_INVENTORY_SAVE_RECV);

	bool exists = false;

	if (gQueryManager.ExecQuery(
		"SELECT 1 FROM MuunInventory WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		exists =
			(sqlRet != SQL_NO_DATA &&
				sqlRet != SQL_NULL_DATA);
	}

	gQueryManager.Close();

	gQueryManager.BindParameterAsBinary(
		1,
		lpMsg->MuunInventory[0],
		sizeof(lpMsg->MuunInventory));

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE MuunInventory SET Items=? WHERE Name='%s'",
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO MuunInventory (Name,Items) VALUES ('%s',?)",
			lpMsg->CharacterName);
	}

	gQueryManager.Close();

#endif
}