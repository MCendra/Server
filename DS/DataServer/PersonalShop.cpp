// PersonalShop.cpp
#include "Header.h"
#include "PersonalShop.h"
#include "QueryManager.h"
#include "SocketManager.h"

CPersonalShop gPersonalShop;

void CPersonalShop::GDPShopItemValueRecv(SDHP_PSHOP_ITEM_VALUE_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 802)

	BYTE send[1024];

	SDHP_PSHOP_ITEM_VALUE_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_PERSONAL_SHOP, DS_SUB_PSHOP_ITEM_VALUE, 0);

	int size = sizeof(SDHP_PSHOP_ITEM_VALUE_SEND);

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Count = 0;

	if (gQueryManager.ExecQuery("SELECT * FROM PShopItemValue WHERE Name='%s'", lpMsg->CharacterName) != false)
	{
		SDHP_PSHOP_ITEM_VALUE info{};

		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			info.Slot = gQueryManager.GetAsInteger("Slot");
			info.Serial = gQueryManager.GetAsInteger("Serial");
			info.Value = gQueryManager.GetAsInteger("Value");
			info.JoBValue = gQueryManager.GetAsInteger("JoBValue");
			info.JoSValue = gQueryManager.GetAsInteger("JoSValue");
			info.JoCValue = gQueryManager.GetAsInteger("JoCValue");

			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);

			++pMsg.Count;
		}
	}

	gQueryManager.Close();

	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);

#endif
}

void CPersonalShop::GDPShopItemValueSaveRecv(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	for (int n = 0; n < lpMsg->Count; ++n)
	{
		auto* lpInfo = reinterpret_cast<SDHP_PSHOP_ITEM_VALUE_SAVE*>(
			reinterpret_cast<BYTE*>(lpMsg) +
			sizeof(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV) +
			(sizeof(SDHP_PSHOP_ITEM_VALUE_SAVE) * n));

		if (gQueryManager.ExecQuery(
			"SELECT Name FROM PShopItemValue WHERE Name='%s' AND Slot=%d",
			lpMsg->CharacterName,
			lpInfo->Slot) == false ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"INSERT INTO PShopItemValue "
				"(Name,Slot,Serial,Value,JoBValue,JoSValue,JoCValue) "
				"VALUES ('%s',%d,%d,%d,%d,%d,%d)",
				lpMsg->CharacterName,
				lpInfo->Slot,
				lpInfo->Serial,
				lpInfo->Value,
				lpInfo->JoBValue,
				lpInfo->JoSValue,
				lpInfo->JoCValue);

			gQueryManager.Close();
		}
		else
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"UPDATE PShopItemValue SET "
				"Serial=%d,Value=%d,JoBValue=%d,JoSValue=%d,JoCValue=%d "
				"WHERE Name='%s' AND Slot=%d",
				lpInfo->Serial,
				lpInfo->Value,
				lpInfo->JoBValue,
				lpInfo->JoSValue,
				lpInfo->JoCValue,
				lpMsg->CharacterName,
				lpInfo->Slot);

			gQueryManager.Close();
		}
	}

#endif
}

void CPersonalShop::GDPShopItemValueInsertSaveRecv(SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	if (gQueryManager.ExecQuery(
		"SELECT Name FROM PShopItemValue WHERE Name='%s' AND Slot=%d",
		lpMsg->CharacterName,
		lpMsg->Slot) == false ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO PShopItemValue "
			"(Name,Slot,Serial,Value,JoBValue,JoSValue,JoCValue) "
			"VALUES ('%s',%d,%d,%d,%d,%d,%d)",
			lpMsg->CharacterName,
			lpMsg->Slot,
			lpMsg->Serial,
			lpMsg->Value,
			lpMsg->JoBValue,
			lpMsg->JoSValue,
			lpMsg->JoCValue);

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"UPDATE PShopItemValue SET "
			"Serial=%d,Value=%d,JoBValue=%d,JoSValue=%d,JoCValue=%d "
			"WHERE Name='%s' AND Slot=%d",
			lpMsg->Serial,
			lpMsg->Value,
			lpMsg->JoBValue,
			lpMsg->JoSValue,
			lpMsg->JoCValue,
			lpMsg->CharacterName,
			lpMsg->Slot);

		gQueryManager.Close();
	}

#endif
}

void CPersonalShop::GDPShopItemValueDeleteSaveRecv(SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	gQueryManager.ExecQuery(
		"DELETE FROM PShopItemValue WHERE Name='%s' AND Slot=%d",
		lpMsg->CharacterName,
		lpMsg->Slot);

	gQueryManager.Close();

#endif
}