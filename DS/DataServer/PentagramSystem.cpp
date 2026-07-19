// PentagramSystem.cpp
#include "Header.h"
#include "PentagramSystem.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CPentagramSystem gPentagramSystem;

void CPentagramSystem::GDPentagramJewelInfoRecv(const SDHP_PENTAGRAM_JEWEL_INFO_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 701)

	VALIDATE_PACKET_SIZE(SDHP_PENTAGRAM_JEWEL_INFO_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_PENTAGRAM_JEWEL_INFO_SEND pMsg{};

	pMsg.Header.set(
		HEAD_PENTAGRAM_SYSTEM,
		SUB_PENTAGRAM_JEWEL_INFO,
		0);

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Type = lpMsg->Type;
	pMsg.Count = 0;

	int sendSize = sizeof(pMsg);

	constexpr int maxCount =
		(MAX_SEND_PACKET_SIZE - sizeof(SDHP_PENTAGRAM_JEWEL_INFO_SEND)) /
		sizeof(SDHP_PENTAGRAM_JEWEL_INFO);

	if (gQueryManager.ExecQuery(
		"SELECT Type,[Index],Attribute,ItemSection,ItemType,ItemLevel,"
		"OptionIndexRank1,OptionLevelRank1,"
		"OptionIndexRank2,OptionLevelRank2,"
		"OptionIndexRank3,OptionLevelRank3,"
		"OptionIndexRank4,OptionLevelRank4,"
		"OptionIndexRank5,OptionLevelRank5 "
		"FROM PentagramJewel "
		"WHERE Name='%s' AND Type=%d",
		lpMsg->CharacterName,
		lpMsg->Type))
	{
		auto sqlRet = gQueryManager.Fetch();

		while (sqlRet != SQL_NO_DATA &&
			sqlRet != SQL_NULL_DATA &&
			pMsg.Count < maxCount)
		{
			SDHP_PENTAGRAM_JEWEL_INFO info{};

			info.Type = gQueryManager.GetAsInteger("Type");
			info.Index = gQueryManager.GetAsInteger("Index");
			info.Attribute = gQueryManager.GetAsInteger("Attribute");
			info.ItemSection = gQueryManager.GetAsInteger("ItemSection");
			info.ItemType = gQueryManager.GetAsInteger("ItemType");
			info.ItemLevel = gQueryManager.GetAsInteger("ItemLevel");

			info.OptionIndexRank1 = gQueryManager.GetAsInteger("OptionIndexRank1");
			info.OptionLevelRank1 = gQueryManager.GetAsInteger("OptionLevelRank1");
			info.OptionIndexRank2 = gQueryManager.GetAsInteger("OptionIndexRank2");
			info.OptionLevelRank2 = gQueryManager.GetAsInteger("OptionLevelRank2");
			info.OptionIndexRank3 = gQueryManager.GetAsInteger("OptionIndexRank3");
			info.OptionLevelRank3 = gQueryManager.GetAsInteger("OptionLevelRank3");
			info.OptionIndexRank4 = gQueryManager.GetAsInteger("OptionIndexRank4");
			info.OptionLevelRank4 = gQueryManager.GetAsInteger("OptionLevelRank4");
			info.OptionIndexRank5 = gQueryManager.GetAsInteger("OptionIndexRank5");
			info.OptionLevelRank5 = gQueryManager.GetAsInteger("OptionLevelRank5");

			std::memcpy(
				&send[sendSize],
				&info,
				sizeof(info));

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

void CPentagramSystem::GDPentagramJewelInfoSaveRecv(const SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 701)

	VALIDATE_PACKET_SIZE(SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV);

	if (lpMsg->Count < 0)
	{
		return;
	}

	const int expectedSize =
		sizeof(SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV) +
		(sizeof(SDHP_PENTAGRAM_JEWEL_INFO_SAVE) * lpMsg->Count);

	if (size != expectedSize)
	{
		return;
	}

	const auto* lpMsgBody =
		reinterpret_cast<const SDHP_PENTAGRAM_JEWEL_INFO_SAVE*>(
			reinterpret_cast<const BYTE*>(lpMsg) +
			sizeof(SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV));

	for (int n = 0; n < lpMsg->Count; ++n)
	{
		const auto& info = lpMsgBody[n];

		bool exists = false;

		if (gQueryManager.ExecQuery(
			"SELECT 1 FROM PentagramJewel "
			"WHERE Name='%s' AND Type=%d AND [Index]=%d",
			lpMsg->CharacterName,
			info.Type,
			info.Index))
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
				"INSERT INTO PentagramJewel "
				"(Name,Type,[Index],Attribute,ItemSection,ItemType,ItemLevel,"
				"OptionIndexRank1,OptionLevelRank1,"
				"OptionIndexRank2,OptionLevelRank2,"
				"OptionIndexRank3,OptionLevelRank3,"
				"OptionIndexRank4,OptionLevelRank4,"
				"OptionIndexRank5,OptionLevelRank5) "
				"VALUES "
				"('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
				lpMsg->CharacterName,
				info.Type,
				info.Index,
				info.Attribute,
				info.ItemSection,
				info.ItemType,
				info.ItemLevel,
				info.OptionIndexRank1,
				info.OptionLevelRank1,
				info.OptionIndexRank2,
				info.OptionLevelRank2,
				info.OptionIndexRank3,
				info.OptionLevelRank3,
				info.OptionIndexRank4,
				info.OptionLevelRank4,
				info.OptionIndexRank5,
				info.OptionLevelRank5);
		}
		else
		{
			gQueryManager.ExecQuery(
				"UPDATE PentagramJewel SET "
				"Attribute=%d,"
				"ItemSection=%d,"
				"ItemType=%d,"
				"ItemLevel=%d,"
				"OptionIndexRank1=%d,"
				"OptionLevelRank1=%d,"
				"OptionIndexRank2=%d,"
				"OptionLevelRank2=%d,"
				"OptionIndexRank3=%d,"
				"OptionLevelRank3=%d,"
				"OptionIndexRank4=%d,"
				"OptionLevelRank4=%d,"
				"OptionIndexRank5=%d,"
				"OptionLevelRank5=%d "
				"WHERE Name='%s' AND Type=%d AND [Index]=%d",
				info.Attribute,
				info.ItemSection,
				info.ItemType,
				info.ItemLevel,
				info.OptionIndexRank1,
				info.OptionLevelRank1,
				info.OptionIndexRank2,
				info.OptionLevelRank2,
				info.OptionIndexRank3,
				info.OptionLevelRank3,
				info.OptionIndexRank4,
				info.OptionLevelRank4,
				info.OptionIndexRank5,
				info.OptionLevelRank5,
				lpMsg->CharacterName,
				info.Type,
				info.Index);
		}

		gQueryManager.Close();
	}

#endif
}
void CPentagramSystem::GDPentagramJewelInsertSaveRecv(const SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if(DATASERVER_UPDATE>=701)

	if (!gQueryManager.ExecQuery("SELECT Name FROM PentagramJewel WHERE Name='%s' AND Type=%d AND [Index]=%d",
		lpMsg->CharacterName, lpMsg->Type, lpMsg->JewelIndex) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO PentagramJewel "
			"(Name,Type,[Index],Attribute,ItemSection,ItemType,ItemLevel,"
			"OptionIndexRank1,OptionLevelRank1,OptionIndexRank2,OptionLevelRank2,"
			"OptionIndexRank3,OptionLevelRank3,OptionIndexRank4,OptionLevelRank4,"
			"OptionIndexRank5,OptionLevelRank5) "
			"VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
			lpMsg->CharacterName,
			lpMsg->Type,
			lpMsg->JewelIndex,
			lpMsg->Attribute,
			lpMsg->ItemSection,
			lpMsg->ItemType,
			lpMsg->ItemLevel,
			lpMsg->OptionIndexRank1,
			lpMsg->OptionLevelRank1,
			lpMsg->OptionIndexRank2,
			lpMsg->OptionLevelRank2,
			lpMsg->OptionIndexRank3,
			lpMsg->OptionLevelRank3,
			lpMsg->OptionIndexRank4,
			lpMsg->OptionLevelRank4,
			lpMsg->OptionIndexRank5,
			lpMsg->OptionLevelRank5);

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"UPDATE PentagramJewel SET "
			"Attribute=%d,ItemSection=%d,ItemType=%d,ItemLevel=%d,"
			"OptionIndexRank1=%d,OptionLevelRank1=%d,"
			"OptionIndexRank2=%d,OptionLevelRank2=%d,"
			"OptionIndexRank3=%d,OptionLevelRank3=%d,"
			"OptionIndexRank4=%d,OptionLevelRank4=%d,"
			"OptionIndexRank5=%d,OptionLevelRank5=%d "
			"WHERE Name='%s' AND Type=%d AND [Index]=%d",
			lpMsg->Attribute,
			lpMsg->ItemSection,
			lpMsg->ItemType,
			lpMsg->ItemLevel,
			lpMsg->OptionIndexRank1,
			lpMsg->OptionLevelRank1,
			lpMsg->OptionIndexRank2,
			lpMsg->OptionLevelRank2,
			lpMsg->OptionIndexRank3,
			lpMsg->OptionLevelRank3,
			lpMsg->OptionIndexRank4,
			lpMsg->OptionLevelRank4,
			lpMsg->OptionIndexRank5,
			lpMsg->OptionLevelRank5,
			lpMsg->CharacterName,
			lpMsg->Type,
			lpMsg->JewelIndex);

		gQueryManager.Close();
	}

#endif
}

void CPentagramSystem::GDPentagramJewelDeleteSaveRecv(const SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if(DATASERVER_UPDATE>=701)

	gQueryManager.ExecQuery(
		"DELETE FROM PentagramJewel WHERE Name='%s' AND Type=%d AND [Index]=%d",
		lpMsg->CharacterName,
		lpMsg->Type,
		lpMsg->JewelIndex);

	gQueryManager.Close();

#endif
}