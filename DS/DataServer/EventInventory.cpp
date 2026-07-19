// EventInventory.cpp
#include "Header.h"
#include "EventInventory.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CEventInventory gEventInventory;

// Construction/Destruction

void CEventInventory::GDEventInventoryRecv(const SDHP_EVENT_INVENTORY_RECV* lpMsg, int serverIndex,	int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(SDHP_EVENT_INVENTORY_RECV);

	SDHP_EVENT_INVENTORY_SEND pMsg{};

	pMsg.Header.set(HEAD_EVENT_INVENTORY, SUB_EVENT_INVENTORY, sizeof(pMsg));
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
		pMsg.EventInventory,
		0xFF,
		sizeof(pMsg.EventInventory));

	if (gQueryManager.ExecQuery(
		"SELECT Items FROM EventInventory WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA &&
			sqlRet != SQL_NULL_DATA)
		{
			gQueryManager.GetAsBinary(
				"Items",
				pMsg.EventInventory[0],
				sizeof(pMsg.EventInventory));
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CEventInventory::GDEventInventorySaveRecv(const SDHP_EVENT_INVENTORY_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(SDHP_EVENT_INVENTORY_SAVE_RECV);

	bool exists = false;

	if (gQueryManager.ExecQuery(
		"SELECT 1 FROM EventInventory WHERE Name='%s'",
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
		lpMsg->EventInventory[0],
		sizeof(lpMsg->EventInventory));

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE EventInventory SET Items=? WHERE Name='%s'",
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO EventInventory (Name,Items) VALUES ('%s',?)",
			lpMsg->CharacterName);
	}

	gQueryManager.Close();

#endif
}
