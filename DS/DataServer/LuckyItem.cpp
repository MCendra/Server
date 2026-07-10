// LuckyItem.cpp
#include "Header.h"
#include "LuckyItem.h"
#include "QueryManager.h"
#include "SocketManager.h"

CLuckyItem gLuckyItem;

void CLuckyItem::GDLuckyItemRecv(SDHP_LUCKY_ITEM_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 602)

	if (lpMsg == nullptr)
	{
		return;
	}

	BYTE send[4096]{};

	SDHP_LUCKY_ITEM_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_LUCKY_ITEM, DS_SUB_LUCKY_ITEM_LOAD, 0);

	int size = sizeof(SDHP_LUCKY_ITEM_SEND);

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.Count = 0;

	constexpr int maxCount =
		(sizeof(send) - sizeof(SDHP_LUCKY_ITEM_SEND)) / sizeof(SDHP_LUCKY_ITEM2);

	for (int n = 0; n < lpMsg->Count && pMsg.Count < maxCount; n++)
	{
		auto* lpInfo = reinterpret_cast<SDHP_LUCKY_ITEM1*>(
			reinterpret_cast<BYTE*>(lpMsg) +
			sizeof(SDHP_LUCKY_ITEM_RECV) +
			(sizeof(SDHP_LUCKY_ITEM1) * n));

		SDHP_LUCKY_ITEM2 info{};

		info.Slot = lpInfo->Slot;
		info.Serial = lpInfo->Serial;
		info.DurabilitySmall = 0;

		if (gQueryManager.ExecQuery(
			"SELECT DurabilitySmall FROM LuckyItem WHERE ItemSerial=%u",
			lpInfo->Serial))
		{
			const auto sqlRet = gQueryManager.Fetch();

			if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
			{
				info.DurabilitySmall = gQueryManager.GetAsInteger("DurabilitySmall");
			}
			else
			{
				gQueryManager.Close();

				gQueryManager.ExecQuery(
					"INSERT INTO LuckyItem (ItemSerial,DurabilitySmall) VALUES (%u,0)",
					lpInfo->Serial);
			}
		}

		gQueryManager.Close();

		std::memcpy(&send[size], &info, sizeof(info));

		size += sizeof(info);

		++pMsg.Count;
	}

	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);

#endif
}

void CLuckyItem::GDLuckyItemSaveRecv(SDHP_LUCKY_ITEM_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 602)

	if (lpMsg == nullptr)
	{
		return;
	}

	for (int n = 0; n < lpMsg->Count; n++)
	{
		auto* lpInfo = reinterpret_cast<SDHP_LUCKY_ITEM_SAVE*>(
			reinterpret_cast<BYTE*>(lpMsg) +
			sizeof(SDHP_LUCKY_ITEM_SAVE_RECV) +
			(sizeof(SDHP_LUCKY_ITEM_SAVE) * n));

		if (gQueryManager.ExecQuery(
			"SELECT 1 FROM LuckyItem WHERE ItemSerial=%u",
			lpInfo->Serial))
		{
			const auto sqlRet = gQueryManager.Fetch();

			gQueryManager.Close();

			if (sqlRet == SQL_NO_DATA)
			{
				gQueryManager.ExecQuery(
					"INSERT INTO LuckyItem (ItemSerial,DurabilitySmall) VALUES (%u,%d)",
					lpInfo->Serial,
					lpInfo->DurabilitySmall);
			}
			else
			{
				gQueryManager.ExecQuery(
					"UPDATE LuckyItem SET DurabilitySmall=%d WHERE ItemSerial=%u",
					lpInfo->DurabilitySmall,
					lpInfo->Serial);
			}
		}

		gQueryManager.Close();
	}

#endif
}