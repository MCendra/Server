// LuckyItem.cpp
#include "Header.h"
#include "LuckyItem.h"
#include "QueryManager.h"
#include "SocketManager.h"

CLuckyItem gLuckyItem;

void CLuckyItem::GDLuckyItemRecv(const SDHP_LUCKY_ITEM_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 602)

	if (lpMsg->Count < 0)
	{
		return;
	}

	const int expectedSize =
		sizeof(SDHP_LUCKY_ITEM_RECV) +
		(sizeof(SDHP_LUCKY_ITEM1) * lpMsg->Count);

	if (size != expectedSize)
	{
		return;
	}

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_LUCKY_ITEM_SEND pMsg{};

	pMsg.Header.set(HEAD_LUCKY_ITEM, SUB_LUCKY_ITEM_LOAD, 0);
	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	pMsg.Count = 0;

	int packetSize = sizeof(SDHP_LUCKY_ITEM_SEND);

	constexpr int maxCount =
		(MAX_SEND_PACKET_SIZE - sizeof(SDHP_LUCKY_ITEM_SEND)) /
		sizeof(SDHP_LUCKY_ITEM2);

	const auto* itemData =
		reinterpret_cast<const SDHP_LUCKY_ITEM1*>(
			reinterpret_cast<const BYTE*>(lpMsg) +
			sizeof(SDHP_LUCKY_ITEM_RECV));

	for (int n = 0; n < lpMsg->Count && pMsg.Count < maxCount; ++n)
	{
		const auto& item = itemData[n];

		SDHP_LUCKY_ITEM2 info{};

		info.Slot = item.Slot;
		info.Serial = item.Serial;
		info.DurabilitySmall = 0;

		if (gQueryManager.ExecQuery(
			"SELECT DurabilitySmall FROM LuckyItem WHERE ItemSerial=%u",
			item.Serial))
		{
			const auto sqlRet = gQueryManager.Fetch();

			if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
			{
				info.DurabilitySmall =
					gQueryManager.GetAsInteger("DurabilitySmall");
			}

			gQueryManager.Close();

			if (sqlRet == SQL_NO_DATA)
			{
				gQueryManager.ExecQuery(
					"INSERT INTO LuckyItem (ItemSerial,DurabilitySmall) VALUES (%u,0)",
					item.Serial);

				gQueryManager.Close();
			}
		}
		else
		{
			gQueryManager.Close();
		}

		std::memcpy(
			&send[packetSize],
			&info,
			sizeof(info));

		packetSize += sizeof(info);

		++pMsg.Count;
	}

	pMsg.Header.size[0] = SET_NUMBERHB(packetSize);
	pMsg.Header.size[1] = SET_NUMBERLB(packetSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, packetSize);

#endif
}

void CLuckyItem::GDLuckyItemSaveRecv(const SDHP_LUCKY_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 602)

	if (lpMsg->Count < 0)
	{
		return;
	}

	const int expectedSize =
		sizeof(SDHP_LUCKY_ITEM_SAVE_RECV) +
		(sizeof(SDHP_LUCKY_ITEM_SAVE) * lpMsg->Count);

	if (size != expectedSize)
	{
		return;
	}

	const auto* itemData =
		reinterpret_cast<const SDHP_LUCKY_ITEM_SAVE*>(
			reinterpret_cast<const BYTE*>(lpMsg) +
			sizeof(SDHP_LUCKY_ITEM_SAVE_RECV));

	for (int n = 0; n < lpMsg->Count; ++n)
	{
		const auto& item = itemData[n];

		if (!gQueryManager.ExecQuery(
			"SELECT 1 FROM LuckyItem WHERE ItemSerial=%u",
			item.Serial))
		{
			gQueryManager.Close();
			continue;
		}

		const auto sqlRet = gQueryManager.Fetch();

		gQueryManager.Close();

		if (sqlRet == SQL_NULL_DATA)
		{
			continue;
		}

		if (sqlRet == SQL_NO_DATA)
		{
			gQueryManager.ExecQuery(
				"INSERT INTO LuckyItem (ItemSerial,DurabilitySmall) VALUES (%u,%d)",
				item.Serial,
				item.DurabilitySmall);
		}
		else
		{
			gQueryManager.ExecQuery(
				"UPDATE LuckyItem SET DurabilitySmall=%d WHERE ItemSerial=%u",
				item.DurabilitySmall,
				item.Serial);
		}

		gQueryManager.Close();
	}

#endif
}