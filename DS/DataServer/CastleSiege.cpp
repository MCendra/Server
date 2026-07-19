// CastleSiege.cpp
#include "Header.h"
#include "CastleSiege.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "Util.h"

CCastleSiege gCastleSiege;

void CCastleSiege::DS_GDReqCastleTotalInfo(const CSP_REQ_CASTLEDATA* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_CASTLEDATA);

	CASTLE_DATA CastleData{};

	CSP_ANS_CASTLEDATA pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_TOTAL_INFO, sizeof(pMsg));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	if (!DB_QueryCastleTotalInfo(lpMsg->MapSvrNum, lpMsg->CastleEventCycle, &CastleData))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = 1;

		pMsg.StartYear = CastleData.StartYear;
		pMsg.StartMonth = CastleData.StartMonth;
		pMsg.StartDay = CastleData.StartDay;
		pMsg.EndYear = CastleData.EndYear;
		pMsg.EndMonth = CastleData.EndMonth;
		pMsg.EndDay = CastleData.EndDay;
		pMsg.IsSiegeGuildList = CastleData.IsSiegeGuildList;
		pMsg.IsSiegeEnded = CastleData.IsSiegeEnded;
		pMsg.IsCastleOccupied = CastleData.IsCastleOccupied;
		pMsg.CastleMoney = CastleData.CastleMoney;
		pMsg.TaxRateChaos = CastleData.TaxRateChaos;
		pMsg.TaxRateStore = CastleData.TaxRateStore;
		pMsg.TaxHuntZone = CastleData.TaxHuntZone;
		pMsg.FirstCreate = CastleData.FirstCreate;

		std::memcpy(pMsg.CastleOwnGuild, CastleData.CastleOwnGuild, sizeof(pMsg.CastleOwnGuild));
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqOwnerGuildMaster(const CSP_REQ_OWNERGUILDMASTER* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_OWNERGUILDMASTER);

	CSP_ANS_OWNERGUILDMASTER pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_OWNER_GUILD_MASTER, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	if (!DB_QueryOwnerGuildMaster(lpMsg->MapSvrNum, &pMsg))
	{
		pMsg.Result = 0;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCastleNpcBuy(const CSP_REQ_NPCBUY* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_NPCBUY);

	CSP_ANS_NPCBUY pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_NPC_BUY, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.NpcNumber = lpMsg->NpcNumber;
	pMsg.NpcIndex = lpMsg->NpcIndex;
	pMsg.BuyCost = lpMsg->BuyCost;

	int queryResult = 0;

	if (DB_QueryCastleNpcBuy(lpMsg->MapSvrNum, lpMsg, &queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCastleNpcRepair(const CSP_REQ_NPCREPAIR* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_NPCREPAIR);

	CSP_ANS_NPCREPAIR pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_NPC_REPAIR, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.NpcNumber = lpMsg->NpcNumber;
	pMsg.NpcIndex = lpMsg->NpcIndex;
	pMsg.RepairCost = lpMsg->RepairCost;

	int queryResult = 0;

	if (DB_QueryCastleNpcRepair(lpMsg->MapSvrNum, lpMsg, &pMsg, &queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCastleNpcUpgrade(const CSP_REQ_NPCUPGRADE* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_NPCUPGRADE);

	CSP_ANS_NPCUPGRADE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_NPC_UPGRADE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.NpcNumber = lpMsg->NpcNumber;
	pMsg.NpcIndex = lpMsg->NpcIndex;
	pMsg.NpcUpType = lpMsg->NpcUpType;
	pMsg.NpcUpValue = lpMsg->NpcUpValue;
	pMsg.NpcUpIndex = lpMsg->NpcUpIndex;

	pMsg.Result = DB_QueryCastleNpcUpgrade(lpMsg->MapSvrNum, lpMsg)	? 1	: 0;

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqTaxInfo(const CSP_REQ_TAXINFO* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_TAXINFO);

	CSP_ANS_TAXINFO pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_TAX_INFO, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	pMsg.Result =
		DB_QueryTaxInfo(lpMsg->MapSvrNum, &pMsg)
		? 1
		: 0;

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqTaxRateChange(const CSP_REQ_TAXRATECHANGE* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_TAXRATECHANGE);

	CSP_ANS_TAXRATECHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_TAX_RATE_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	int queryResult = 0;

	if (DB_QueryTaxRateChange(
		lpMsg->MapSvrNum,
		lpMsg->TaxKind,
		lpMsg->TaxRate,
		&pMsg,
		&queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCastleMoneyChange(const CSP_REQ_MONEYCHANGE* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_MONEYCHANGE);

	CSP_ANS_MONEYCHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_MONEY_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.MoneyChanged = lpMsg->MoneyChanged;

	std::int64_t moneyResult = 0;
	int queryResult = 0;

	if (DB_QueryCastleMoneyChange(
		lpMsg->MapSvrNum,
		lpMsg->MoneyChanged,
		&moneyResult,
		&queryResult))
	{
		pMsg.Result = queryResult;
		pMsg.CastleMoney = moneyResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqSiegeDateChange(const CSP_REQ_SDEDCHANGE* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_SDEDCHANGE);

	CSP_ANS_SDEDCHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_SIEGE_DATE_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	int queryResult = 0;

	if (DB_QuerySiegeDateChange(
		lpMsg->MapSvrNum,
		lpMsg,
		&queryResult))
	{
		pMsg.Result = queryResult;

		pMsg.StartYear = lpMsg->StartYear;
		pMsg.StartMonth = lpMsg->StartMonth;
		pMsg.StartDay = lpMsg->StartDay;
		pMsg.EndYear = lpMsg->EndYear;
		pMsg.EndMonth = lpMsg->EndMonth;
		pMsg.EndDay = lpMsg->EndDay;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqGuildMarkRegInfo(const CSP_REQ_GUILDREGINFO* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_GUILDREGINFO);

	CSP_ANS_GUILDREGINFO pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_GUILD_MARK_INFO, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	char GuildName[9]{};

	std::memcpy(GuildName, lpMsg->GuildName, sizeof(lpMsg->GuildName));

	int queryResult = 0;

	if (DB_QueryGuildMarkRegInfo(
		lpMsg->MapSvrNum,
		GuildName,
		&pMsg,
		&queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqSiegeEndedChange(const CSP_REQ_SIEGEENDCHANGE* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_SIEGEENDCHANGE);

	CSP_ANS_SIEGEENDCHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_SIEGE_ENDED_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	int queryResult = 0;

	if (DB_QuerySiegeEndedChange(
		lpMsg->MapSvrNum,
		lpMsg->IsSiegeEnded,
		&queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCastleOwnerChange(const CSP_REQ_CASTLEOWNERCHANGE* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_CASTLEOWNERCHANGE);

	CSP_ANS_CASTLEOWNERCHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_OWNER_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	int queryResult = 0;

	if (DB_QueryCastleOwnerChange(
		lpMsg->MapSvrNum,
		lpMsg,
		&pMsg,
		&queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqRegAttackGuild(const CSP_REQ_REGATTACKGUILD* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_REGATTACKGUILD);

	CSP_ANS_REGATTACKGUILD pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_REG_ATTACK_GUILD,	sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	int queryResult = 0;

	if (DB_QueryRegAttackGuild(
		lpMsg->MapSvrNum,
		lpMsg,
		&pMsg,
		&queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqRestartCastleState(const CSP_REQ_RESTARTCASTLESTATE* lpMsg, int serverIndex,	int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_RESTARTCASTLESTATE);

	CSP_ANS_CASTLESIEGEEND pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_RESTART_STATE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	int queryResult = 0;

	if (DB_QueryRestartCastleState(lpMsg->MapSvrNum, &queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

//**********************************************//

void CCastleSiege::DS_GDReqMapSvrMsgMultiCast(const CSP_REQ_MAPSVRMULTICAST* lpMsg, int serverIndex, int size)
{
	CSP_ANS_MAPSVRMULTICAST pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_MAPSVR_MULTICAST, sizeof(CSP_ANS_MAPSVRMULTICAST));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	memcpy(pMsg.MsgText, lpMsg->MsgText, sizeof(lpMsg->MsgText));

	for (int n = 0;n < MAX_SERVER;n++)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, (BYTE*)&pMsg, sizeof(CSP_ANS_MAPSVRMULTICAST));
		}
	}
}

void CCastleSiege::DS_GDReqRegGuildMark(const CSP_REQ_GUILDREGMARK* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_GUILDREGMARK);

	CSP_ANS_GUILDREGMARK pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_REG_GUILD_MARK, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.ItemPos = lpMsg->ItemPos;

	char guildName[9]{};
	std::memcpy(guildName, lpMsg->GuildName, 8);

	int queryResult = 0;

	if (DB_QueryGuildMarkRegMark(
		lpMsg->MapSvrNum,
		guildName,
		&pMsg,
		&queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg),sizeof(pMsg));
}

void CCastleSiege::DS_GDReqGuildMarkReset(const CSP_REQ_GUILDRESETMARK* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_GUILDRESETMARK);

	CSP_ANS_GUILDRESETMARK pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_GUILD_MARK_RESET, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	char guildName[9]{};
	std::memcpy(guildName, lpMsg->GuildName, 8);

	DB_QueryGuildMarkReset(
		lpMsg->MapSvrNum,
		guildName,
		&pMsg);

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqGuildSetGiveUp(const CSP_REQ_GUILDSETGIVEUP* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_GUILDSETGIVEUP);

	CSP_ANS_GUILDSETGIVEUP pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_GUILD_GIVEUP, sizeof(pMsg));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	char szGuildName[9]{};
	std::memcpy(szGuildName, lpMsg->GuildName, 8);

	if (!DB_QueryGuildSetGiveUp(lpMsg->MapSvrNum, szGuildName, lpMsg->IsGiveUp, &pMsg))
	{
		pMsg.Result = 0;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCastleNpcRemove(const CSP_REQ_NPCREMOVE* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_NPCREMOVE);

	CSP_ANS_NPCREMOVE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_NPC_REMOVE, sizeof(pMsg));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.NpcNumber = lpMsg->NpcNumber;
	pMsg.NpcIndex = lpMsg->NpcIndex;

	int queryResult = 0;

	if (!DB_QueryCastleNpcRemove(lpMsg->MapSvrNum, lpMsg, &queryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCastleStateSync(const CSP_REQ_CASTLESTATESYNC* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_CASTLESTATESYNC);

	CSP_ANS_CASTLESTATESYNC pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_STATE_SYNC, sizeof(pMsg));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.CastleState = lpMsg->CastleState;
	pMsg.TaxRateChaos = lpMsg->TaxRateChaos;
	pMsg.TaxRateStore = lpMsg->TaxRateStore;
	pMsg.TaxHuntZone = lpMsg->TaxHuntZone;

	std::memcpy(pMsg.OwnerGuildName, lpMsg->OwnerGuildName, sizeof(pMsg.OwnerGuildName));

	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].IsOnline())
		{
			gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void CCastleSiege::DS_GDReqCastleTributeMoney(const CSP_REQ_CASTLETRIBUTEMONEY* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_CASTLETRIBUTEMONEY);

	CSP_ANS_CASTLETRIBUTEMONEY pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_TRIBUTE_MONEY, sizeof(pMsg));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	if (lpMsg->CastleTributeMoney >= 0)
	{
		int queryResult = 0;
		std::int64_t moneyResult = 0;

		if (DB_QueryCastleMoneyChange(
			lpMsg->MapSvrNum,
			lpMsg->CastleTributeMoney,
			&moneyResult,
			&queryResult))
		{
			pMsg.Result = queryResult;
		}
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqResetCastleTaxInfo(const CSP_REQ_RESETCASTLETAXINFO* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_RESETCASTLETAXINFO);

	CSP_ANS_RESETCASTLETAXINFO pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_RESET_TAX_INFO, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	int queryResult = 0;

	if (DB_QueryResetCastleTaxInfo(lpMsg->MapSvrNum, &queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqResetSiegeGuildInfo(const CSP_REQ_RESETSIEGEGUILDINFO* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_RESETSIEGEGUILDINFO);

	CSP_ANS_RESETSIEGEGUILDINFO pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_RESET_SIEGE_GUILD, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	int queryResult = 0;

	if (DB_QueryResetSiegeGuildInfo(lpMsg->MapSvrNum, &queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqResetRegSiegeInfo(const CSP_REQ_RESETSIEGEGUILDINFO* lpMsg, int serverIndex,	int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_RESETSIEGEGUILDINFO);

	CSP_ANS_RESETSIEGEGUILDINFO pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_RESET_REG_INFO, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	int queryResult = 0;

	if (DB_QueryResetRegSiegeInfo(lpMsg->MapSvrNum, &queryResult))
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCastleInitData(const CSP_REQ_CSINITDATA* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_CSINITDATA);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	auto* lpMsgSend =
		reinterpret_cast<CSP_ANS_CSINITDATA*>(send);

	auto* lpMsgSendBody =
		reinterpret_cast<CSP_NPCDATA*>(
			send + sizeof(CSP_ANS_CSINITDATA));

	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;
	lpMsgSend->Count = 0;

	CASTLE_DATA castleData{};

	if (DB_QueryCastleTotalInfo(
		lpMsg->MapSvrNum,
		lpMsg->CastleEventCycle,
		&castleData))
	{
		lpMsgSend->StartYear = castleData.StartYear;
		lpMsgSend->StartMonth = castleData.StartMonth;
		lpMsgSend->StartDay = castleData.StartDay;
		lpMsgSend->EndYear = castleData.EndYear;
		lpMsgSend->EndMonth = castleData.EndMonth;
		lpMsgSend->EndDay = castleData.EndDay;
		lpMsgSend->IsSiegeGuildList = castleData.IsSiegeGuildList;
		lpMsgSend->IsSiegeEnded = castleData.IsSiegeEnded;
		lpMsgSend->IsCastleOccupied = castleData.IsCastleOccupied;
		lpMsgSend->CastleMoney = castleData.CastleMoney;
		lpMsgSend->TaxRateChaos = castleData.TaxRateChaos;
		lpMsgSend->TaxRateStore = castleData.TaxRateStore;
		lpMsgSend->TaxHuntZone = castleData.TaxHuntZone;
		lpMsgSend->FirstCreate = castleData.FirstCreate;

		std::memcpy(
			lpMsgSend->CastleOwnGuild,
			castleData.CastleOwnGuild,
			sizeof(lpMsgSend->CastleOwnGuild));

		int dataCount =
			static_cast<int>(
				(sizeof(send) - sizeof(CSP_ANS_CSINITDATA)) /
				sizeof(CSP_NPCDATA));

		if (DB_QueryCastleNpcInfo(
			lpMsg->MapSvrNum,
			lpMsgSendBody,
			&dataCount))
		{
			lpMsgSend->Result = 1;
			lpMsgSend->Count = dataCount;
		}
	}

	const int sendSize =
		sizeof(CSP_ANS_CSINITDATA) +
		(sizeof(CSP_NPCDATA) * lpMsgSend->Count);

	lpMsgSend->Header.set(0x81, sendSize);

	gSocketManager.DataSend(serverIndex, send, sendSize);
}

void CCastleSiege::DS_GDReqCastleNpcInfo(const CSP_REQ_NPCDATA* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_NPCDATA);

	constexpr int MaxNpcCount = 200;

	std::array<BYTE,
		sizeof(CSP_ANS_NPCDATA) +
		(sizeof(CSP_NPCDATA) * MaxNpcCount)> buffer{};

	auto* lpMsgSend =
		reinterpret_cast<CSP_ANS_NPCDATA*>(buffer.data());

	auto* lpMsgSendBody =
		reinterpret_cast<CSP_NPCDATA*>(
			buffer.data() + sizeof(CSP_ANS_NPCDATA));

	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;

	int dataCount = MaxNpcCount;

	if (DB_QueryCastleNpcInfo(
		lpMsg->MapSvrNum,
		lpMsgSendBody,
		&dataCount))
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = dataCount;
	}

	const int packetSize =
		sizeof(CSP_ANS_NPCDATA) +
		(sizeof(CSP_NPCDATA) * lpMsgSend->Count);

	lpMsgSend->Header.set(0x82, packetSize);

	gSocketManager.DataSend(serverIndex, buffer.data(), packetSize);
}

void CCastleSiege::DS_GDReqAllGuildMarkRegInfo(const CSP_REQ_ALLGUILDREGINFO* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_ALLGUILDREGINFO);

	constexpr int MaxGuildCount = 100;

	std::array<BYTE,
		sizeof(CSP_ANS_ALLGUILDREGINFO) +
		(sizeof(CSP_GUILDREGINFO) * MaxGuildCount)> buffer{};

	auto* lpMsgSend =
		reinterpret_cast<CSP_ANS_ALLGUILDREGINFO*>(buffer.data());

	auto* lpMsgSendBody =
		reinterpret_cast<CSP_GUILDREGINFO*>(
			buffer.data() + sizeof(CSP_ANS_ALLGUILDREGINFO));

	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;
	lpMsgSend->Index = lpMsg->Index;

	int dataCount = MaxGuildCount;

	if (DB_QueryAllGuildMarkRegInfo(lpMsg->MapSvrNum, lpMsgSendBody, &dataCount))
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = dataCount;
	}

	const int packetSize =
		sizeof(CSP_ANS_ALLGUILDREGINFO) +
		(sizeof(CSP_GUILDREGINFO) * lpMsgSend->Count);

	lpMsgSend->Header.set(0x83, packetSize);

	gSocketManager.DataSend(serverIndex, buffer.data(), packetSize);
}

void CCastleSiege::DS_GDReqFirstCreateNPC(const CSP_REQ_NPCSAVEDATA* lpMsg, int serverIndex, int size)
{
	if (size < sizeof(CSP_REQ_NPCSAVEDATA))
	{
		return;
	}

	CSP_ANS_NPCSAVEDATA pMsg{};

	pMsg.Header.set(0x84, sizeof(pMsg));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	const int count = lpMsg->Count;

	const int expectedSize =
		sizeof(CSP_REQ_NPCSAVEDATA) +
		(sizeof(CSP_NPCSAVEDATA) * count);

	if (count < 0 || count > 200 || size != expectedSize)
	{
		pMsg.Result = 0;
	}
	else
	{
		const auto* npcData =
			reinterpret_cast<const CSP_NPCSAVEDATA*>(
				reinterpret_cast<const BYTE*>(lpMsg) +
				sizeof(CSP_REQ_NPCSAVEDATA));

		pMsg.Result =
			DB_QueryFirstCreateNPC(
				lpMsg->MapSvrNum,
				npcData,
				count)
			? 1
			: 0;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCalcRegGuildList(const CSP_REQ_CALCREGGUILDLIST* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(CSP_REQ_CALCREGGUILDLIST);

	constexpr int MaxGuildCount = 100;

	std::array<BYTE,
		sizeof(CSP_ANS_CALCREGGUILDLIST) +
		(sizeof(CSP_CALCREGGUILDLIST) * MaxGuildCount)> buffer{};

	auto* lpMsgSend =
		reinterpret_cast<CSP_ANS_CALCREGGUILDLIST*>(buffer.data());

	auto* lpMsgSendBody =
		reinterpret_cast<CSP_CALCREGGUILDLIST*>(
			buffer.data() + sizeof(CSP_ANS_CALCREGGUILDLIST));

	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;

	int dataCount = MaxGuildCount;

	if (DB_QueryCalcRegGuildList(
		lpMsg->MapSvrNum,
		lpMsgSendBody,
		&dataCount))
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = dataCount;
	}

	const int packetSize =
		sizeof(CSP_ANS_CALCREGGUILDLIST) +
		(sizeof(CSP_CALCREGGUILDLIST) * lpMsgSend->Count);

	lpMsgSend->Header.set(0x85, packetSize);

	gSocketManager.DataSend(serverIndex, buffer.data(), packetSize);
}

void CCastleSiege::DS_GDReqCsGuildUnionInfo(const CSP_REQ_CSGUILDUNIONINFO* lpMsg, int serverIndex, int size)
{
	if (size < sizeof(CSP_REQ_CSGUILDUNIONINFO))
	{
		return;
	}

	const int count = lpMsg->Count;

	if (count < 0 || count > 100)
	{
		return;
	}

	const int expectedSize =
		sizeof(CSP_REQ_CSGUILDUNIONINFO) +
		(sizeof(CSP_CSGUILDUNIONINFO) * count);

	if (size != expectedSize)
	{
		return;
	}

	const auto* lpMsgBody =
		reinterpret_cast<const CSP_CSGUILDUNIONINFO*>(
			reinterpret_cast<const BYTE*>(lpMsg) +
			sizeof(CSP_REQ_CSGUILDUNIONINFO));

	constexpr int MaxGuildCount = 100;

	std::array<BYTE,
		sizeof(CSP_ANS_CSGUILDUNIONINFO) +
		(sizeof(CSP_CSGUILDUNIONINFO) * MaxGuildCount)> buffer{};

	auto* lpMsgSend =
		reinterpret_cast<CSP_ANS_CSGUILDUNIONINFO*>(buffer.data());

	auto* lpMsgSendBody =
		reinterpret_cast<CSP_CSGUILDUNIONINFO*>(
			buffer.data() +
			sizeof(CSP_ANS_CSGUILDUNIONINFO));

	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;

	int resultCount = 0;
	bool success = true;

	for (int n = 0; n < count; ++n)
	{
		char guildName[9]{};

		std::memcpy(
			guildName,
			lpMsgBody[n].GuildName,
			sizeof(lpMsgBody[n].GuildName));

		if (!DB_QueryCsGuildUnionInfo(
			lpMsg->MapSvrNum,
			guildName,
			lpMsgBody[n].CsGuildID,
			lpMsgSendBody,
			&resultCount))
		{
			success = false;
			resultCount = 0;
			break;
		}
	}

	lpMsgSend->Result = success ? 1 : 0;
	lpMsgSend->Count = resultCount;

	const int packetSize =
		sizeof(CSP_ANS_CSGUILDUNIONINFO) +
		(sizeof(CSP_CSGUILDUNIONINFO) * resultCount);

	lpMsgSend->Header.set(0x86, packetSize);

	gSocketManager.DataSend(serverIndex, buffer.data(),	packetSize);
}

void CCastleSiege::DS_GDReqCsSaveTotalGuildInfo(const CSP_REQ_CSSAVETOTALGUILDINFO* lpMsg, int serverIndex,	int size)
{
	CSP_ANS_CSSAVETOTALGUILDINFO pMsg{};

	pMsg.Header.set(0x87, sizeof(pMsg));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	if (lpMsg->Count < 0 || lpMsg->Count > 100)
	{
		pMsg.Result = 0;
		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));
		return;
	}

	const int expectedSize =
		sizeof(CSP_REQ_CSSAVETOTALGUILDINFO) +
		(sizeof(CSP_CSSAVETOTALGUILDINFO) * lpMsg->Count);

	if (size != expectedSize)
	{
		pMsg.Result = 0;
		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));
		return;
	}

	const auto* guildData =
		reinterpret_cast<const CSP_CSSAVETOTALGUILDINFO*>(
			reinterpret_cast<const BYTE*>(lpMsg) +
			sizeof(CSP_REQ_CSSAVETOTALGUILDINFO));

	if (!DB_QueryCsClearTotalGuildInfo(lpMsg->MapSvrNum))
	{
		pMsg.Result = 0;
		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));
		return;
	}

	for (int n = 0; n < lpMsg->Count; ++n)
	{
		char guildName[9]{};
		memcpy(guildName, guildData[n].GuildName, 8);

		if (!DB_QueryCsSaveTotalGuildInfo(
			lpMsg->MapSvrNum,
			guildName,
			guildData[n].CsGuildID,
			guildData[n].GuildInvolved,
			guildData[n].GuildScore))
		{
			pMsg.Result = 0;
			gSocketManager.DataSend(
				serverIndex,
				reinterpret_cast<BYTE*>(&pMsg),
				sizeof(pMsg));
			return;
		}
	}

	int queryResult = 0;

	if (!DB_QueryCsSaveTotalGuildOK(lpMsg->MapSvrNum, &queryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = queryResult;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCastleSiege::DS_GDReqCsLoadTotalGuildInfo(const CSP_REQ_CSLOADTOTALGUILDINFO* lpMsg, int serverIndex,	int size)
{
	char buffer[sizeof(CSP_ANS_CSLOADTOTALGUILDINFO) +
		(sizeof(CSP_CSLOADTOTALGUILDINFO) * 100)]{};

	auto* lpMsgSend =
		reinterpret_cast<CSP_ANS_CSLOADTOTALGUILDINFO*>(buffer);

	auto* lpMsgSendBody =
		reinterpret_cast<CSP_CSLOADTOTALGUILDINFO*>(
			buffer + sizeof(CSP_ANS_CSLOADTOTALGUILDINFO));

	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;
	lpMsgSend->Count = 0;

	int dataCount = 100;

	if (DB_QueryCsLoadTotalGuildInfo(
		lpMsg->MapSvrNum,
		lpMsgSendBody,
		&dataCount))
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = dataCount;
	}
	else
	{
		lpMsgSend->Result = 0;
	}

	const int packetSize =
		sizeof(CSP_ANS_CSLOADTOTALGUILDINFO) +
		(sizeof(CSP_CSLOADTOTALGUILDINFO) * lpMsgSend->Count);

	lpMsgSend->Header.set(0x88, packetSize);

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(lpMsgSend), packetSize);
}

void CCastleSiege::DS_GDReqCastleNpcUpdate(const CSP_REQ_NPCUPDATEDATA* lpMsg, int serverIndex,	int size)
{
	CSP_ANS_NPCSAVEDATA pMsg{};

	pMsg.Header.set(0x89, sizeof(pMsg));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	if (lpMsg->Count < 0 || lpMsg->Count > 200)
	{
		pMsg.Result = 0;
	}
	else
	{
		const int expectedSize =
			sizeof(CSP_REQ_NPCUPDATEDATA) +
			(sizeof(CSP_NPCUPDATEDATA) * lpMsg->Count);

		if (size != expectedSize)
		{
			pMsg.Result = 0;
		}
		else
		{
			const auto* npcData =
				reinterpret_cast<const CSP_NPCUPDATEDATA*>(
					reinterpret_cast<const BYTE*>(lpMsg) +
					sizeof(CSP_REQ_NPCUPDATEDATA));

			pMsg.Result =
				DB_QueryCastleNpcUpdate(
					lpMsg->MapSvrNum,
					npcData,
					lpMsg->Count)
				? 1
				: 0;
		}
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

//**********************************************//

bool CCastleSiege::DB_QueryCastleTotalInfo(int iMapSvrGroup, int iCastleEventCycle, CASTLE_DATA* lpCastleData)
{
    if (lpCastleData == nullptr)
    {
        return false;
    }

    if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetCastleTotalInfo %d, %d", iMapSvrGroup, iCastleEventCycle))
    {
        gQueryManager.Close();
        return false;
    }

    const auto sqlRet = gQueryManager.Fetch();

    if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
    {
        gQueryManager.Close();
        return false;
    }

    lpCastleData->StartYear = gQueryManager.GetAsInteger("SYEAR");
    lpCastleData->StartMonth = gQueryManager.GetAsInteger("SMONTH");
    lpCastleData->StartDay = gQueryManager.GetAsInteger("SDAY");
    lpCastleData->EndYear = gQueryManager.GetAsInteger("EYEAR");
    lpCastleData->EndMonth = gQueryManager.GetAsInteger("EMONTH");
    lpCastleData->EndDay = gQueryManager.GetAsInteger("EDAY");
    lpCastleData->IsSiegeGuildList = gQueryManager.GetAsInteger("SIEGE_GUILDLIST_SETTED");
    lpCastleData->IsSiegeEnded = gQueryManager.GetAsInteger("SIEGE_ENDED");
    lpCastleData->IsCastleOccupied = gQueryManager.GetAsInteger("CASTLE_OCCUPY");
    lpCastleData->CastleMoney = gQueryManager.GetAsInteger64("MONEY");
    lpCastleData->TaxRateChaos = gQueryManager.GetAsInteger("TAX_RATE_CHAOS");
    lpCastleData->TaxRateStore = gQueryManager.GetAsInteger("TAX_RATE_STORE");
    lpCastleData->TaxHuntZone = gQueryManager.GetAsInteger("TAX_HUNT_ZONE");
    lpCastleData->FirstCreate = gQueryManager.GetAsInteger("FIRST_CREATE");
    gQueryManager.GetAsString("OWNER_GUILD", lpCastleData->CastleOwnGuild, sizeof(lpCastleData->CastleOwnGuild));
    gQueryManager.Close();
    return true;
}

bool CCastleSiege::DB_QueryOwnerGuildMaster(int iMapSvrGroup, CSP_ANS_OWNERGUILDMASTER* lpOwnerGuildMaster)
{
	if (lpOwnerGuildMaster == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_GetOwnerGuildMaster %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();

		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		return false;
	}

	lpOwnerGuildMaster->Result =
		gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.GetAsString(
		"OwnerGuild",
		lpOwnerGuildMaster->CastleOwnGuild,
		sizeof(lpOwnerGuildMaster->CastleOwnGuild));

	gQueryManager.GetAsString(
		"OwnerGuildMaster",
		lpOwnerGuildMaster->CastleOwnGuildMaster,
		sizeof(lpOwnerGuildMaster->CastleOwnGuildMaster));

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryCastleNpcBuy(
	int iMapSvrGroup,
	const CSP_REQ_NPCBUY* lpNpcBuy,
	int* lpiResult)
{
	if (lpNpcBuy == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ReqNpcBuy %d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		iMapSvrGroup,
		lpNpcBuy->NpcNumber,
		lpNpcBuy->NpcIndex,
		lpNpcBuy->NpcDfLevel,
		lpNpcBuy->NpcRgLevel,
		lpNpcBuy->NpcMaxHp,
		lpNpcBuy->NpcHp,
		lpNpcBuy->NpcX,
		lpNpcBuy->NpcY,
		lpNpcBuy->NpcDIR))
	{
		gQueryManager.Close();

		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryCastleNpcRepair(int iMapSvrGroup, const CSP_REQ_NPCREPAIR* lpNpcRepair, CSP_ANS_NPCREPAIR* lpNpcRepairResult, int* lpiResult)
{
	if (lpNpcRepair == nullptr ||
		lpNpcRepairResult == nullptr ||
		lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ReqNpcRepair %d,%d,%d",
		iMapSvrGroup,
		lpNpcRepair->NpcNumber,
		lpNpcRepair->NpcIndex))
	{
		gQueryManager.Close();

		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	lpNpcRepairResult->NpcHp =
		gQueryManager.GetAsInteger("NPC_HP");

	lpNpcRepairResult->NpcMaxHp =
		gQueryManager.GetAsInteger("NPC_MAXHP");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryCastleNpcUpgrade(int iMapSvrGroup, const CSP_REQ_NPCUPGRADE* lpNpcUpgrade)
{
	if (lpNpcUpgrade == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ReqNpcUpgrade %d,%d,%d,%d,%d",
		iMapSvrGroup,
		lpNpcUpgrade->NpcNumber,
		lpNpcUpgrade->NpcIndex,
		lpNpcUpgrade->NpcUpType,
		lpNpcUpgrade->NpcUpValue))
	{
		gQueryManager.Close();

		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		return false;
	}

	const int queryResult =
		gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return (queryResult != 0);
}

bool CCastleSiege::DB_QueryTaxInfo(int iMapSvrGroup, CSP_ANS_TAXINFO* lpTaxInfo)
{
	if (lpTaxInfo == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetCastleTaxInfo %d", iMapSvrGroup))
	{
		gQueryManager.Close();

		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		return false;
	}

	lpTaxInfo->CastleMoney =
		gQueryManager.GetAsInteger64("MONEY");

	lpTaxInfo->TaxRateChaos =
		gQueryManager.GetAsInteger("TAX_RATE_CHAOS");

	lpTaxInfo->TaxRateStore =
		gQueryManager.GetAsInteger("TAX_RATE_STORE");

	lpTaxInfo->TaxHuntZone =
		gQueryManager.GetAsInteger("TAX_HUNT_ZONE");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryTaxRateChange(int iMapSvrGroup, int iTaxType, int iTaxRate, CSP_ANS_TAXRATECHANGE* lpTaxRateChange,	int* lpiResult)
{
	if (lpTaxRateChange == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ModifyTaxRate %d,%d,%d",
		iMapSvrGroup,
		iTaxType,
		iTaxRate))
	{
		gQueryManager.Close();

		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		return false;
	}

	*lpiResult =
		gQueryManager.GetAsInteger("QueryResult");

	lpTaxRateChange->TaxKind =
		gQueryManager.GetAsInteger("TaxKind");

	lpTaxRateChange->TaxRate =
		gQueryManager.GetAsInteger("TaxRate");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryCastleMoneyChange(int iMapSvrGroup, int iMoneyChange, std::int64_t* resultMoney, int* result)
{
	if (resultMoney == nullptr || result == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ModifyMoney %d, %d",
		iMapSvrGroup,
		iMoneyChange))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*result = gQueryManager.GetAsInteger("QueryResult");
	*resultMoney = gQueryManager.GetAsInteger64("MONEY");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QuerySiegeDateChange(int iMapSvrGroup, const CSP_REQ_SDEDCHANGE* lpSdEdChange, int* result)
{
	if (lpSdEdChange == nullptr || result == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ModifyCastleSchedule %d, '%d-%d-%d 00:00:00', '%d-%d-%d 00:00:00'",
		iMapSvrGroup,
		lpSdEdChange->StartYear,
		lpSdEdChange->StartMonth,
		lpSdEdChange->StartDay,
		lpSdEdChange->EndYear,
		lpSdEdChange->EndMonth,
		lpSdEdChange->EndDay))
	{
		gQueryManager.Close();
		return false;
	}

	const short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*result = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryGuildMarkRegInfo(int iMapSvrGroup, const char* lpszGuildName, CSP_ANS_GUILDREGINFO* lpGuildRegInfo, int* lpiResult)
{
	if (lpszGuildName == nullptr ||
		lpGuildRegInfo == nullptr ||
		lpiResult == nullptr)
	{
		return false;
	}

	const size_t guildNameLength = std::strlen(lpszGuildName);

	if (!gUtil.CheckTextSyntax(lpszGuildName, guildNameLength))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_GetGuildMarkRegInfo %d, '%s'",
		iMapSvrGroup,
		lpszGuildName))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	std::memcpy(
		lpGuildRegInfo->GuildName,
		lpszGuildName,
		sizeof(lpGuildRegInfo->GuildName));

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		lpGuildRegInfo->RegMarkCount = 0;
		*lpiResult = 2;
	}
	else
	{
		lpGuildRegInfo->RegMarkCount =
			gQueryManager.GetAsInteger("REG_MARKS");

		lpGuildRegInfo->RegRank =
			gQueryManager.GetAsInteger("SEQ_NUM");

		lpGuildRegInfo->IsGiveUp =
			(gQueryManager.GetAsInteger("IS_GIVEUP") > 0);

		*lpiResult = 1;
	}

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QuerySiegeEndedChange(int iMapSvrGroup, int bIsCastleSiegeEnded, int* lpiResult)
{
	if (lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ModifySiegeEnd %d, %d",
		iMapSvrGroup,
		bIsCastleSiegeEnded))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryCastleOwnerChange(int iMapSvrGroup, const CSP_REQ_CASTLEOWNERCHANGE* lpCastleOwnerInfo, CSP_ANS_CASTLEOWNERCHANGE* lpCastleOwnerInfoResult, int* lpiResult)
{
	if (lpCastleOwnerInfo == nullptr ||
		lpCastleOwnerInfoResult == nullptr ||
		lpiResult == nullptr)
	{
		return false;
	}

	char guildName[9]{};

	std::memcpy(
		guildName,
		lpCastleOwnerInfo->OwnerGuildName,
		sizeof(lpCastleOwnerInfo->OwnerGuildName));

	if (!gUtil.CheckTextSyntax(
		guildName,
		std::strlen(guildName)))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ModifyCastleOwnerInfo %d, %d, '%s'",
		iMapSvrGroup,
		lpCastleOwnerInfo->IsCastleOccupied,
		guildName))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	lpCastleOwnerInfoResult->IsCastleOccupied =
		lpCastleOwnerInfo->IsCastleOccupied;

	std::memcpy(
		lpCastleOwnerInfoResult->OwnerGuildName,
		lpCastleOwnerInfo->OwnerGuildName,
		sizeof(lpCastleOwnerInfoResult->OwnerGuildName));

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryRegAttackGuild(int iMapSvrGroup,	const CSP_REQ_REGATTACKGUILD* lpRegAttackGuild,	CSP_ANS_REGATTACKGUILD* lpRegAttackGuildResult,	int* lpiResult)
{
	if (lpRegAttackGuild == nullptr ||
		lpRegAttackGuildResult == nullptr ||
		lpiResult == nullptr)
	{
		return false;
	}

	char guildName[9]{};

	std::memcpy(
		guildName,
		lpRegAttackGuild->EnemyGuildName,
		sizeof(lpRegAttackGuild->EnemyGuildName));

	if (!gUtil.CheckTextSyntax(
		guildName,
		std::strlen(guildName)))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ReqRegAttackGuild %d, '%s'",
		iMapSvrGroup,
		guildName))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	std::memcpy(
		lpRegAttackGuildResult->EnemyGuildName,
		lpRegAttackGuild->EnemyGuildName,
		sizeof(lpRegAttackGuildResult->EnemyGuildName));

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryRestartCastleState(int iMapSvrGroup, int* lpiResult)
{
	if (lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ResetCastleSiege %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryGuildMarkRegMark(int iMapSvrGroup, const char* lpszGuildName, CSP_ANS_GUILDREGMARK* lpGuildRegMark, int* lpiResult)
{
	if (lpszGuildName == nullptr || lpGuildRegMark == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gUtil.CheckTextSyntax(lpszGuildName, strlen(lpszGuildName)))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ReqRegGuildMark %d, '%s'", iMapSvrGroup, lpszGuildName))
	{
		gQueryManager.Close();
		return false;
	}

	const short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	memcpy(lpGuildRegMark->GuildName, lpszGuildName, 8);

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	lpGuildRegMark->RegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryGuildMarkReset(int iMapSvrGroup, const char* lpszGuildName, CSP_ANS_GUILDRESETMARK* lpGuildResetMark)
{
	if (lpszGuildName == nullptr || lpGuildResetMark == nullptr)
	{
		return false;
	}

	if (!gUtil.CheckTextSyntax(lpszGuildName, strlen(lpszGuildName)))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ModifyGuildMarkReset %d, '%s'", iMapSvrGroup, lpszGuildName))
	{
		gQueryManager.Close();
		return false;
	}

	const short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	memcpy(lpGuildResetMark->GuildName, lpszGuildName, 8);

	lpGuildResetMark->Result = gQueryManager.GetAsInteger("QueryResult");
	lpGuildResetMark->RegMarkCount = gQueryManager.GetAsInteger("DEL_MARKS");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryGuildSetGiveUp(int iMapSvrGroup, const char* lpszGuildName, int bIsGiveUp, CSP_ANS_GUILDSETGIVEUP* lpGuildSetGiveUp)
{
	if (lpszGuildName == nullptr || lpGuildSetGiveUp == nullptr)
	{
		return false;
	}

	if (!gUtil.CheckTextSyntax(lpszGuildName, std::strlen(lpszGuildName)))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ModifyGuildGiveUp %d, '%s', %d",
		iMapSvrGroup,
		lpszGuildName,
		bIsGiveUp))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	std::memcpy(lpGuildSetGiveUp->GuildName, lpszGuildName, 8);

	lpGuildSetGiveUp->IsGiveUp = bIsGiveUp;
	lpGuildSetGiveUp->Result = gQueryManager.GetAsInteger("QueryResult");
	lpGuildSetGiveUp->RegMarkCount = gQueryManager.GetAsInteger("DEL_MARKS");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryCastleNpcRemove(int iMapSvrGroup, const CSP_REQ_NPCREMOVE* lpNpcRemove, int* lpiResult)
{
	if (lpNpcRemove == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ReqNpcRemove %d, %d, %d",
		iMapSvrGroup,
		lpNpcRemove->NpcNumber,
		lpNpcRemove->NpcIndex))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryResetCastleTaxInfo(int iMapSvrGroup, int* result)
{
	if (result == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ResetCastleTaxInfo %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*result = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryResetSiegeGuildInfo(int iMapSvrGroup, int* result)
{
	if (result == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ResetSiegeGuildInfo %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*result = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryResetRegSiegeInfo(int iMapSvrGroup, int* result)
{
	if (result == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ResetRegSiegeInfo %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*result = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

bool CCastleSiege::DB_QueryCastleNpcInfo(int iMapSvrGroup, CSP_NPCDATA* npcData, int* count)
{
	if (npcData == nullptr || count == nullptr || *count <= 0)
	{
		return false;
	}

	const int maxCount = *count;
	*count = 0;

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_GetCastleNpcInfo %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	auto sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*count >= maxCount)
		{
			break;
		}

		auto& npc = npcData[*count];

		npc.NpcNumber = gQueryManager.GetAsInteger("NPC_NUMBER");
		npc.NpcIndex = gQueryManager.GetAsInteger("NPC_INDEX");
		npc.NpcDfLevel = gQueryManager.GetAsInteger("NPC_DF_LEVEL");
		npc.NpcRgLevel = gQueryManager.GetAsInteger("NPC_RG_LEVEL");
		npc.NpcMaxHp = gQueryManager.GetAsInteger("NPC_MAXHP");
		npc.NpcHp = gQueryManager.GetAsInteger("NPC_HP");
		npc.NpcX = gQueryManager.GetAsInteger("NPC_X");
		npc.NpcY = gQueryManager.GetAsInteger("NPC_Y");
		npc.NpcDIR = gQueryManager.GetAsInteger("NPC_DIR");

		++(*count);

		sqlRet = gQueryManager.Fetch();
	}

	const bool success = (sqlRet != SQL_NULL_DATA);

	gQueryManager.Close();

	return success;
}

bool CCastleSiege::DB_QueryAllGuildMarkRegInfo(int iMapSvrGroup, CSP_GUILDREGINFO* guildRegInfo, int* count)
{
	if (guildRegInfo == nullptr || count == nullptr || *count <= 0)
	{
		return false;
	}

	const int maxCount = (*count > 100) ? 100 : *count;
	*count = 0;

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_GetAllGuildMarkRegInfo %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	auto sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*count >= maxCount)
		{
			break;
		}

		auto& guild = guildRegInfo[*count];

		gQueryManager.GetAsString(
			"REG_SIEGE_GUILD",
			guild.GuildName,
			sizeof(guild.GuildName));

		guild.RegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");
		guild.IsGiveUp = (gQueryManager.GetAsInteger("IS_GIVEUP") > 0);
		guild.RegRank = gQueryManager.GetAsInteger("SEQ_NUM");

		++(*count);

		sqlRet = gQueryManager.Fetch();
	}

	const bool success = (sqlRet != SQL_NULL_DATA);

	gQueryManager.Close();

	return success;
}

bool CCastleSiege::DB_QueryFirstCreateNPC(int iMapSvrGroup, const CSP_NPCSAVEDATA* npcData,	int count)
{
	if (npcData == nullptr || count < 0 || count > 200)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"DELETE FROM MuCastle_NPC WHERE MAP_SVR_GROUP=%d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	for (int n = 0; n < count; ++n)
	{
		const auto& npc = npcData[n];

		if (!gQueryManager.ExecQuery(
			"EXEC WZ_CS_ReqNpcBuy %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			iMapSvrGroup,
			npc.NpcNumber,
			npc.NpcIndex,
			npc.NpcDfLevel,
			npc.NpcRgLevel,
			npc.NpcMaxHp,
			npc.NpcHp,
			npc.NpcX,
			npc.NpcY,
			npc.NpcDIR))
		{
			gQueryManager.Close();
			return false;
		}

		gQueryManager.Close();
	}

	return true;
}

bool CCastleSiege::DB_QueryCalcRegGuildList(int iMapSvrGroup, CSP_CALCREGGUILDLIST* guildList, int* count)
{
	if (guildList == nullptr || count == nullptr || *count <= 0)
	{
		return false;
	}

	const int maxCount = (*count > 100) ? 100 : *count;
	*count = 0;

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_GetCalcRegGuildList %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	auto sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*count >= maxCount)
		{
			break;
		}

		auto& guild = guildList[*count];

		gQueryManager.GetAsString(
			"REG_SIEGE_GUILD",
			guild.GuildName,
			sizeof(guild.GuildName));

		guild.RegMarkCount =
			gQueryManager.GetAsInteger("REG_MARKS");

		guild.GuildMemberCount =
			gQueryManager.GetAsInteger("GUILD_MEMBER");

		guild.GuildMasterLevel =
			gQueryManager.GetAsInteger("GM_LEVEL");

		guild.SeqNum =
			gQueryManager.GetAsInteger("SEQ_NUM");

		++(*count);

		sqlRet = gQueryManager.Fetch();
	}

	const bool success = (sqlRet != SQL_NULL_DATA);

	gQueryManager.Close();

	return success;
}

bool CCastleSiege::DB_QueryCsGuildUnionInfo(int iMapSvrGroup, const char* guildName, int csGuildID,	CSP_CSGUILDUNIONINFO* guildUnionInfo, int* count)
{
	if (guildName == nullptr ||
		guildUnionInfo == nullptr ||
		count == nullptr ||
		*count < 0 ||
		*count > 100)
	{
		return false;
	}

	if (!gUtil.CheckTextSyntax(guildName, std::strlen(guildName)))
	{
		return false;
	}

	int resultCount = *count;

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_GetCsGuildUnionInfo '%s'",
		guildName))
	{
		gQueryManager.Close();
		return false;
	}

	auto sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (resultCount >= 100)
		{
			break;
		}

		auto& guild = guildUnionInfo[resultCount];

		gQueryManager.GetAsString(
			"GUILD_NAME",
			guild.GuildName,
			sizeof(guild.GuildName));

		guild.CsGuildID = csGuildID;

		++resultCount;

		sqlRet = gQueryManager.Fetch();
	}

	const bool success = (sqlRet != SQL_NULL_DATA);

	gQueryManager.Close();

	if (success)
	{
		*count = resultCount;
	}

	return success;
}

bool CCastleSiege::DB_QueryCsClearTotalGuildInfo(int iMapSvrGroup)
{
	if (!gQueryManager.ExecQuery(
		"DELETE FROM MuCastle_SIEGE_GUILDLIST WHERE MAP_SVR_GROUP = %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();
	return true;
}

bool CCastleSiege::DB_QueryCsSaveTotalGuildInfo(int iMapSvrGroup, const char* lpszGuildName, int iCsGuildID, int iCsGuildInvolved, int iCsGuildScore)
{
	if (lpszGuildName == nullptr)
	{
		return false;
	}

	const size_t guildNameLength = strlen(lpszGuildName);

	if (!gUtil.CheckTextSyntax(lpszGuildName, guildNameLength))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_SetSiegeGuildInfo %d, '%s', %d, %d, %d",
		iMapSvrGroup,
		lpszGuildName,
		iCsGuildID,
		iCsGuildInvolved,
		iCsGuildScore))
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();
	return true;
}

bool CCastleSiege::DB_QueryCsSaveTotalGuildOK(int iMapSvrGroup,	int* lpiResult)
{
	if (lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_SetSiegeGuildOK %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();
	return true;
}

bool CCastleSiege::DB_QueryCsLoadTotalGuildInfo(int iMapSvrGroup, CSP_CSLOADTOTALGUILDINFO* lpLoadTotalGuildInfo, int* lpiCount)
{
	if (lpLoadTotalGuildInfo == nullptr ||
		lpiCount == nullptr ||
		*lpiCount <= 0)
	{
		return false;
	}

	const int maxCount = (*lpiCount > 100) ? 100 : *lpiCount;
	*lpiCount = 0;

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_GetSiegeGuildInfo %d",
		iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	auto sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount >= maxCount)
		{
			break;
		}

		auto& guild = lpLoadTotalGuildInfo[*lpiCount];

		gQueryManager.GetAsString(
			"GUILD_NAME",
			guild.GuildName,
			sizeof(guild.GuildName));

		guild.CsGuildID =
			gQueryManager.GetAsInteger("GUILD_ID");

		guild.GuildInvolved =
			gQueryManager.GetAsInteger("GUILD_INVOLVED");

		guild.GuildScore =
			gQueryManager.GetAsInteger("GUILD_SCORE");

		++(*lpiCount);

		sqlRet = gQueryManager.Fetch();
	}

	const bool success = (sqlRet != SQL_NULL_DATA);

	gQueryManager.Close();

	return success;
}


bool CCastleSiege::DB_QueryCastleNpcUpdate(int iMapSvrGroup, const CSP_NPCUPDATEDATA* npcData, int count)
{
	if (npcData == nullptr || count < 0 || count > 200)
	{
		return false;
	}

	for (int n = 0; n < count; ++n)
	{
		const auto& npc = npcData[n];

		if (!gQueryManager.ExecQuery(
			"EXEC WZ_CS_ReqNpcUpdate %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			iMapSvrGroup,
			npc.NpcNumber,
			npc.NpcIndex,
			npc.NpcDfLevel,
			npc.NpcRgLevel,
			npc.NpcMaxHp,
			npc.NpcHp,
			npc.NpcX,
			npc.NpcY,
			npc.NpcDIR))
		{
			gQueryManager.Close();
			return false;
		}

		gQueryManager.Close();
	}

	return true;
}
