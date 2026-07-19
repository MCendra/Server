// PersonalShop.cpp
#include "Header.h"
#include "PersonalShop.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CPersonalShop gPersonalShop;

void CPersonalShop::GDPShopItemValueRecv(const SDHP_PSHOP_ITEM_VALUE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(SDHP_PSHOP_ITEM_VALUE_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_PSHOP_ITEM_VALUE_SEND pMsg{};

	pMsg.Header.set(HEAD_PERSONAL_SHOP, SUB_PSHOP_ITEM_VALUE, 0);

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Count = 0;

	int sendSize = sizeof(pMsg);

	constexpr int maxCount =
		(MAX_SEND_PACKET_SIZE - sizeof(SDHP_PSHOP_ITEM_VALUE_SEND)) /
		sizeof(SDHP_PSHOP_ITEM_VALUE);

	if (gQueryManager.ExecQuery(
		"SELECT Slot,Serial,Value,JoBValue,JoSValue,JoCValue "
		"FROM PShopItemValue WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		auto sqlRet = gQueryManager.Fetch();

		while (sqlRet != SQL_NO_DATA &&
			sqlRet != SQL_NULL_DATA &&
			pMsg.Count < maxCount)
		{
			SDHP_PSHOP_ITEM_VALUE info{};

			info.Slot = gQueryManager.GetAsInteger("Slot");
			info.Serial = gQueryManager.GetAsInteger("Serial");
			info.Value = gQueryManager.GetAsInteger("Value");
			info.JoBValue = gQueryManager.GetAsInteger("JoBValue");
			info.JoSValue = gQueryManager.GetAsInteger("JoSValue");
			info.JoCValue = gQueryManager.GetAsInteger("JoCValue");

			std::memcpy(&send[sendSize], &info, sizeof(info));

			sendSize += sizeof(info);
			++pMsg.Count;

			sqlRet = gQueryManager.Fetch();
		}
	}

	gQueryManager.Close();

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);

#endif
}

void CPersonalShop::GDPShopItemValueSaveRecv(const SDHP_PSHOP_ITEM_VALUE_SAVE_RECV* lpMsg, int serverIndex,	int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV);

	if (lpMsg->Count < 0)
	{
		return;
	}

	const int expectedSize =
		sizeof(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV) +
		(sizeof(SDHP_PSHOP_ITEM_VALUE_SAVE) * lpMsg->Count);

	if (size != expectedSize)
	{
		return;
	}

	const auto* lpMsgBody =
		reinterpret_cast<const SDHP_PSHOP_ITEM_VALUE_SAVE*>(
			reinterpret_cast<const BYTE*>(lpMsg) +
			sizeof(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV));

	for (int n = 0; n < lpMsg->Count; ++n)
	{
		const auto& info = lpMsgBody[n];

		bool exists = false;

		if (gQueryManager.ExecQuery(
			"SELECT 1 FROM PShopItemValue "
			"WHERE Name='%s' AND Slot=%d",
			lpMsg->CharacterName,
			info.Slot))
		{
			const auto sqlRet = gQueryManager.Fetch();

			exists =
				(sqlRet != SQL_NO_DATA &&
					sqlRet != SQL_NULL_DATA);
		}

		gQueryManager.Close();

		if (!exists)
		{
			gQueryManager.ExecQuery(
				"INSERT INTO PShopItemValue "
				"(Name,Slot,Serial,Value,JoBValue,JoSValue,JoCValue) "
				"VALUES ('%s',%d,%d,%d,%d,%d,%d)",
				lpMsg->CharacterName,
				info.Slot,
				info.Serial,
				info.Value,
				info.JoBValue,
				info.JoSValue,
				info.JoCValue);
		}
		else
		{
			gQueryManager.ExecQuery(
				"UPDATE PShopItemValue SET "
				"Serial=%d,"
				"Value=%d,"
				"JoBValue=%d,"
				"JoSValue=%d,"
				"JoCValue=%d "
				"WHERE Name='%s' AND Slot=%d",
				info.Serial,
				info.Value,
				info.JoBValue,
				info.JoSValue,
				info.JoCValue,
				lpMsg->CharacterName,
				info.Slot);
		}

		gQueryManager.Close();
	}

#endif
}

void CPersonalShop::GDPShopItemValueInsertSaveRecv(const SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV* lpMsg,	int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV);

	if (gQueryManager.ExecQuery(
		"SELECT 1 FROM PShopItemValue WHERE Name='%s' AND Slot=%d",
		lpMsg->CharacterName,
		lpMsg->Slot))
	{
		const auto sqlRet = gQueryManager.Fetch();

		gQueryManager.Close();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
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
		}
		else
		{
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
		}
	}

	gQueryManager.Close();

#endif
}

void CPersonalShop::GDPShopItemValueDeleteSaveRecv(const SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV* lpMsg,	int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV);

	gQueryManager.ExecQuery(
		"DELETE FROM PShopItemValue WHERE Name='%s' AND Slot=%d",
		lpMsg->CharacterName,
		lpMsg->Slot);

	gQueryManager.Close();

#endif
}