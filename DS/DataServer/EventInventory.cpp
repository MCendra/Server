// EventInventory.cpp
#include "Header.h"
#include "EventInventory.h"
#include "QueryManager.h"
#include "SocketManager.h"

CEventInventory gEventInventory;

// Construction/Destruction

void CEventInventory::GDEventInventoryRecv(SDHP_EVENT_INVENTORY_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 802)

	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_EVENT_INVENTORY_SEND pMsg{};

	pMsg.Header.set(0x26, 0x00, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (!gQueryManager.ExecQuery("SELECT Items FROM EventInventory WHERE Name='%s'", lpMsg->CharacterName))
	{
		gQueryManager.Close();
		memset(pMsg.EventInventory, 0xFF, sizeof(pMsg.EventInventory));

		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	if (gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		memset(pMsg.EventInventory, 0xFF, sizeof(pMsg.EventInventory));

		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	gQueryManager.GetAsBinary("Items", pMsg.EventInventory[0], sizeof(pMsg.EventInventory));

	gQueryManager.Close();

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CEventInventory::GDEventInventorySaveRecv(SDHP_EVENT_INVENTORY_SAVE_RECV* lpMsg)
{
	#if (DATASERVER_UPDATE >= 802)

		if (lpMsg == nullptr)
		{
			return;
		}

		const bool exists =
			gQueryManager.ExecQuery("SELECT Name FROM EventInventory WHERE Name='%s'", lpMsg->CharacterName) &&
			(gQueryManager.Fetch() != SQL_NO_DATA);

		gQueryManager.Close();

		gQueryManager.BindParameterAsBinary(1, lpMsg->EventInventory[0], sizeof(lpMsg->EventInventory));

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
