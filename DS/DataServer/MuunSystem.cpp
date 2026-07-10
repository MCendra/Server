// MuunSystem.cpp
#include "Header.h"
#include "MuunSystem.h"
#include "QueryManager.h"
#include "SocketManager.h"

CMuunSystem gMuunSystem;

void CMuunSystem::GDMuunInventoryRecv(SDHP_MUUN_INVENTORY_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 803)

	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_MUUN_INVENTORY_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_MUUN_SYSTEM, DS_SUB_MUUN_INVENTORY, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"SELECT Items FROM MuunInventory WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			std::memset(pMsg.MuunInventory, 0xFF, sizeof(pMsg.MuunInventory));
		}
		else
		{
			gQueryManager.GetAsBinary(
				"Items",
				pMsg.MuunInventory[0],
				sizeof(pMsg.MuunInventory));
		}
	}
	else
	{
		std::memset(pMsg.MuunInventory, 0xFF, sizeof(pMsg.MuunInventory));
	}

	gQueryManager.Close();

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

#endif
}

void CMuunSystem::GDMuunInventorySaveRecv(SDHP_MUUN_INVENTORY_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 803)

	if (lpMsg == nullptr)
	{
		return;
	}

	if (gQueryManager.ExecQuery(
		"SELECT Name FROM MuunInventory WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		gQueryManager.Close();

		gQueryManager.BindParameterAsBinary(
			1,
			lpMsg->MuunInventory[0],
			sizeof(lpMsg->MuunInventory));

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			gQueryManager.ExecQuery(
				"INSERT INTO MuunInventory (Name,Items) VALUES ('%s',?)",
				lpMsg->CharacterName);
		}
		else
		{
			gQueryManager.ExecQuery(
				"UPDATE MuunInventory SET Items=? WHERE Name='%s'",
				lpMsg->CharacterName);
		}

		gQueryManager.Close();
	}

#endif
}
