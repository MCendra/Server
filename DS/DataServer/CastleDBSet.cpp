// CastleDBSet.cpp
#include "Header.h"
#include "CastleDBSet.h"
#include "QueryManager.h"
#include "Util.h"

CCastleDBSet gCastleDBSet;

BOOL CCastleDBSet::DB_QueryCastleTotalInfo(int iMapSvrGroup, int iCastleEventCycle, CASTLE_DATA* lpCastleData)
{
	if(lpCastleData == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetCastleTotalInfo %d, %d", iMapSvrGroup, iCastleEventCycle))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if(sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
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
	gQueryManager.GetAsString("OWNER_GUILD", lpCastleData->CastleOwnGuild,sizeof(lpCastleData->CastleOwnGuild));
	gQueryManager.Close();
	return true;
}

BOOL CCastleDBSet::DSDB_QueryOwnerGuildMaster(int iMapSvrGroup, CSP_ANS_OWNERGUILDMASTER* lpOwnerGuildMaster)
{
	if (lpOwnerGuildMaster == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetOwnerGuildMaster %d", iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();
	
	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA )
	{
		gQueryManager.Close();
		return false;
	}
	
	char szOwnerGuild[8+8] = {'\0'};
	char szOwnerGuildMaster[8+8] = {'\0'};
	
	lpOwnerGuildMaster->Result = gQueryManager.GetAsInteger("QueryResult");
	
	gQueryManager.GetAsString("OwnerGuild", szOwnerGuild,sizeof(szOwnerGuild));
	gQueryManager.GetAsString("OwnerGuildMaster", szOwnerGuildMaster,sizeof(szOwnerGuildMaster));
	
	memcpy(lpOwnerGuildMaster->CastleOwnGuild, szOwnerGuild, 8);
	memcpy(lpOwnerGuildMaster->CastleOwnGuildMaster, szOwnerGuildMaster, 10);
	
	gQueryManager.Close();
	return true;	
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcBuy(int iMapSvrGroup, CSP_REQ_NPCBUY* lpNpcBuy, int* lpiResult)
{
	if (lpNpcBuy == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ReqNpcBuy %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
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

	const short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();
	return true;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcRepair(int iMapSvrGroup, CSP_REQ_NPCREPAIR* lpNpcRepair, CSP_ANS_NPCREPAIR* lpNpcRepairResult, int* lpiResult)
{
	if (lpNpcRepair == nullptr || lpNpcRepairResult == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ReqNpcRepair %d, %d, %d",
		iMapSvrGroup,
		lpNpcRepair->NpcNumber,
		lpNpcRepair->NpcIndex))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	lpNpcRepairResult->NpcHp = gQueryManager.GetAsInteger("NPC_HP");
	lpNpcRepairResult->NpcMaxHp = gQueryManager.GetAsInteger("NPC_MAXHP");

	gQueryManager.Close();
	return true;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcUpgrade(int iMapSvrGroup, CSP_REQ_NPCUPGRADE* lpNpcUpgrade)
{
	if (lpNpcUpgrade == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ReqNpcUpgrade %d, %d, %d, %d, %d",
		iMapSvrGroup,
		lpNpcUpgrade->NpcNumber,
		lpNpcUpgrade->NpcIndex,
		lpNpcUpgrade->NpcUpType,
		lpNpcUpgrade->NpcUpValue))
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

	const int iResult = gQueryManager.GetAsInteger("QueryResult");

	if (iResult == 0)
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryTaxInfo(int iMapSvrGroup, CSP_ANS_TAXINFO* lpTaxInfo)
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

	const short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	lpTaxInfo->CastleMoney = gQueryManager.GetAsInteger64("MONEY");
	lpTaxInfo->TaxRateChaos = gQueryManager.GetAsInteger("TAX_RATE_CHAOS");
	lpTaxInfo->TaxRateStore = gQueryManager.GetAsInteger("TAX_RATE_STORE");
	lpTaxInfo->TaxHuntZone = gQueryManager.GetAsInteger("TAX_HUNT_ZONE");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryTaxRateChange(int iMapSvrGroup, int iTaxType, int iTaxRate, CSP_ANS_TAXRATECHANGE* lpTaxRateChange, int* lpiResult)
{
	if (lpTaxRateChange == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ModifyTaxRate %d, %d, %d", iMapSvrGroup, iTaxType, iTaxRate))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	lpTaxRateChange->TaxKind = gQueryManager.GetAsInteger("TaxKind");
	lpTaxRateChange->TaxRate = gQueryManager.GetAsInteger("TaxRate");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCastleMoneyChange(int iMapSvrGroup, int iMoneyChange, __int64* i64ResultMoney, int* lpiResult)
{
	if (i64ResultMoney == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ModifyMoney %d, %d", iMapSvrGroup, iMoneyChange))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");
	*i64ResultMoney = gQueryManager.GetAsInteger64("MONEY");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QuerySiegeDateChange(int iMapSvrGroup, CSP_REQ_SDEDCHANGE* lpSdEdChange, int* lpiResult)
{
	if (lpSdEdChange == nullptr || lpiResult == nullptr)
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryGuildMarkRegInfo(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDREGINFO* lpGuildRegInfo, int* lpiResult)
{
	if (lpszGuildName == nullptr || lpGuildRegInfo == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gUtil.CheckTextSyntax(lpszGuildName, strlen(lpszGuildName)))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetGuildMarkRegInfo %d, '%s'", iMapSvrGroup, lpszGuildName))
	{
		gQueryManager.Close();
		return false;
	}

	const short sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	memcpy(lpGuildRegInfo->GuildName, lpszGuildName, 8);

	if (sqlRet == SQL_NO_DATA)
	{
		lpGuildRegInfo->RegMarkCount = 0;
		*lpiResult = 2;
	}
	else
	{
		*lpiResult = 1;
		lpGuildRegInfo->RegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");
		lpGuildRegInfo->RegRank = gQueryManager.GetAsInteger("SEQ_NUM");
		lpGuildRegInfo->IsGiveUp = (gQueryManager.GetAsInteger("IS_GIVEUP") > 0);
	}

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QuerySiegeEndedChange(int iMapSvrGroup, int bIsCastleSiegeEnded, int* lpiResult)
{
	if (lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ModifySiegeEnd %d, %d", iMapSvrGroup, bIsCastleSiegeEnded))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCastleOwnerChange(int iMapSvrGroup, CSP_REQ_CASTLEOWNERCHANGE* lpCastleOwnerInfo, CSP_ANS_CASTLEOWNERCHANGE* lpCastleOwnerInfoResult, int* lpiResult)
{
	if (lpCastleOwnerInfo == nullptr || lpCastleOwnerInfoResult == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	char szGuildName[9]{};
	memcpy(szGuildName, lpCastleOwnerInfo->OwnerGuildName, 8);

	if (!gUtil.CheckTextSyntax(szGuildName, strlen(szGuildName)))
	{
		return false;
	}

	lpCastleOwnerInfoResult->IsCastleOccupied = lpCastleOwnerInfo->IsCastleOccupied;
	memcpy(lpCastleOwnerInfoResult->OwnerGuildName, lpCastleOwnerInfo->OwnerGuildName, 8);

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_CS_ModifyCastleOwnerInfo %d, %d, '%s'",
		iMapSvrGroup,
		lpCastleOwnerInfo->IsCastleOccupied,
		szGuildName))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryRegAttackGuild(int iMapSvrGroup, CSP_REQ_REGATTACKGUILD* lpRegAttackGuild, CSP_ANS_REGATTACKGUILD* lpRegAttackGuildResult, int* lpiResult)
{
	if (lpRegAttackGuild == nullptr || lpRegAttackGuildResult == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	char szGuildName[9]{};
	memcpy(szGuildName, lpRegAttackGuild->EnemyGuildName, 8);

	if (!gUtil.CheckTextSyntax(szGuildName, strlen(szGuildName)))
	{
		return false;
	}

	memcpy(lpRegAttackGuildResult->EnemyGuildName, lpRegAttackGuild->EnemyGuildName, 8);

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ReqRegAttackGuild %d, '%s'", iMapSvrGroup, szGuildName))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryRestartCastleState(int iMapSvrGroup, CSP_REQ_CASTLESIEGEEND* lpCastleSiegeEnd, int* lpiResult)
{
	if (lpCastleSiegeEnd == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ResetCastleSiege %d", iMapSvrGroup))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryGuildMarkRegMark(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDREGMARK* lpGuildRegMark, int* lpiResult)
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

BOOL CCastleDBSet::DSDB_QueryGuildMarkReset(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDRESETMARK* lpGuildResetMark)
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

BOOL CCastleDBSet::DSDB_QueryGuildSetGiveUp(int iMapSvrGroup, char* lpszGuildName, int bIsGiveUp, CSP_ANS_GUILDSETGIVEUP* lpGuildSetGiveUp)
{
	if (lpszGuildName == nullptr || lpGuildSetGiveUp == nullptr)
	{
		return false;
	}

	if (!gUtil.CheckTextSyntax(lpszGuildName, strlen(lpszGuildName)))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ModifyGuildGiveUp %d, '%s', %d", iMapSvrGroup, lpszGuildName, bIsGiveUp))
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

	memcpy(lpGuildSetGiveUp->GuildName, lpszGuildName, 8);

	lpGuildSetGiveUp->IsGiveUp = bIsGiveUp;
	lpGuildSetGiveUp->Result = gQueryManager.GetAsInteger("QueryResult");
	lpGuildSetGiveUp->RegMarkCount = gQueryManager.GetAsInteger("DEL_MARKS");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcRemove(int iMapSvrGroup, CSP_REQ_NPCREMOVE* lpNpcRemove, int* lpiResult)
{
	if (lpNpcRemove == nullptr || lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ReqNpcRemove %d, %d, %d",
		iMapSvrGroup,
		lpNpcRemove->NpcNumber,
		lpNpcRemove->NpcIndex))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryResetCastleTaxInfo(int iMapSvrGroup, int* lpiResult)
{
	if (lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ResetCastleTaxInfo %d", iMapSvrGroup))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryResetSiegeGuildInfo(int iMapSvrGroup, int* lpiResult)
{
	if (lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ResetSiegeGuildInfo %d", iMapSvrGroup))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryResetRegSiegeInfo(int iMapSvrGroup, int* lpiResult)
{
	if (lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_ResetRegSiegeInfo %d", iMapSvrGroup))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcInfo(int iMapSvrGroup, CSP_NPCDATA* lpNpcData, int* lpiCount)
{
	if (lpNpcData == nullptr || lpiCount == nullptr)
	{
		return false;
	}

	const int iMaxCount = *lpiCount;
	*lpiCount = 0;

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetCastleNpcInfo %d", iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	short sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount >= iMaxCount)
		{
			break;
		}

		lpNpcData[*lpiCount].NpcNumber = gQueryManager.GetAsInteger("NPC_NUMBER");
		lpNpcData[*lpiCount].NpcIndex = gQueryManager.GetAsInteger("NPC_INDEX");
		lpNpcData[*lpiCount].NpcDfLevel = gQueryManager.GetAsInteger("NPC_DF_LEVEL");
		lpNpcData[*lpiCount].NpcRgLevel = gQueryManager.GetAsInteger("NPC_RG_LEVEL");
		lpNpcData[*lpiCount].NpcMaxHp = gQueryManager.GetAsInteger("NPC_MAXHP");
		lpNpcData[*lpiCount].NpcHp = gQueryManager.GetAsInteger("NPC_HP");
		lpNpcData[*lpiCount].NpcX = gQueryManager.GetAsInteger("NPC_X");
		lpNpcData[*lpiCount].NpcY = gQueryManager.GetAsInteger("NPC_Y");
		lpNpcData[*lpiCount].NpcDIR = gQueryManager.GetAsInteger("NPC_DIR");

		++(*lpiCount);

		sqlRet = gQueryManager.Fetch();
	}

	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryAllGuildMarkRegInfo(int iMapSvrGroup, CSP_GUILDREGINFO* lpGuildRegInfo, int* lpiCount)
{
	if (lpGuildRegInfo == nullptr || lpiCount == nullptr)
	{
		return false;
	}

	int iMaxCount = *lpiCount;

	if (iMaxCount > 100)
	{
		iMaxCount = 100;
	}

	*lpiCount = 0;

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetAllGuildMarkRegInfo %d", iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	short sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount >= iMaxCount)
		{
			break;
		}

		gQueryManager.GetAsString("REG_SIEGE_GUILD", lpGuildRegInfo[*lpiCount].GuildName, sizeof(lpGuildRegInfo[*lpiCount].GuildName));
		lpGuildRegInfo[*lpiCount].RegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");
		lpGuildRegInfo[*lpiCount].IsGiveUp = (gQueryManager.GetAsInteger("IS_GIVEUP") > 0);
		lpGuildRegInfo[*lpiCount].RegRank = gQueryManager.GetAsInteger("SEQ_NUM");

		++(*lpiCount);

		sqlRet = gQueryManager.Fetch();
	}

	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryFirstCreateNPC(int iMapSvrGroup, CSP_REQ_NPCSAVEDATA* lpNpcSaveData)
{
	if (lpNpcSaveData == nullptr)
	{
		return false;
	}

	CSP_NPCSAVEDATA* lpMsgBody = (CSP_NPCSAVEDATA*)&lpNpcSaveData[1];

	if (!gQueryManager.ExecQuery("DELETE MuCastle_NPC WHERE MAP_SVR_GROUP = %d", iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	for (int iNpcCount = 0; iNpcCount < lpNpcSaveData->Count; ++iNpcCount)
	{
		if (!gQueryManager.ExecQuery(
			"EXEC WZ_CS_ReqNpcBuy %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			iMapSvrGroup,
			lpMsgBody[iNpcCount].NpcNumber,
			lpMsgBody[iNpcCount].NpcIndex,
			lpMsgBody[iNpcCount].NpcDfLevel,
			lpMsgBody[iNpcCount].NpcRgLevel,
			lpMsgBody[iNpcCount].NpcMaxHp,
			lpMsgBody[iNpcCount].NpcHp,
			lpMsgBody[iNpcCount].NpcX,
			lpMsgBody[iNpcCount].NpcY,
			lpMsgBody[iNpcCount].NpcDIR))
		{
			gQueryManager.Close();
			return false;
		}

		gQueryManager.Close();
	}

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCalcRegGuildList(int iMapSvrGroup, CSP_CALCREGGUILDLIST* lpCalcRegGuildList, int* lpiCount)
{
	if (lpCalcRegGuildList == nullptr || lpiCount == nullptr)
	{
		return false;
	}

	int iMaxCount = *lpiCount;

	if (iMaxCount > 100)
	{
		iMaxCount = 100;
	}

	*lpiCount = 0;

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetCalcRegGuildList %d", iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	short sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount >= iMaxCount)
		{
			break;
		}

		char szGuildName[16]{};

		gQueryManager.GetAsString("REG_SIEGE_GUILD", szGuildName, sizeof(szGuildName));

		memcpy(&lpCalcRegGuildList[*lpiCount], szGuildName, 8);

		lpCalcRegGuildList[*lpiCount].RegMarkCount = gQueryManager.GetAsInteger("REG_MARKS");
		lpCalcRegGuildList[*lpiCount].GuildMemberCount = gQueryManager.GetAsInteger("GUILD_MEMBER");
		lpCalcRegGuildList[*lpiCount].GuildMasterLevel = gQueryManager.GetAsInteger("GM_LEVEL");
		lpCalcRegGuildList[*lpiCount].SeqNum = gQueryManager.GetAsInteger("SEQ_NUM");

		++(*lpiCount);

		sqlRet = gQueryManager.Fetch();
	}

	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCsGuildUnionInfo(int iMapSvrGroup, char* lpszGuildName, int iCsGuildID, CSP_CSGUILDUNIONINFO* lpCsGuildUnionInfo, int* lpiCount)
{
	if (lpszGuildName == nullptr || lpCsGuildUnionInfo == nullptr || lpiCount == nullptr)
	{
		return false;
	}

	int iRetCount = *lpiCount;

	if (iRetCount < 0)
	{
		return false;
	}

	if (!gUtil.CheckTextSyntax(lpszGuildName, strlen(lpszGuildName)))
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetCsGuildUnionInfo '%s'", lpszGuildName))
	{
		gQueryManager.Close();
		return false;
	}

	short sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (iRetCount >= 100)
		{
			break;
		}

		char szGuildName[16]{};

		gQueryManager.GetAsString("GUILD_NAME", szGuildName, sizeof(szGuildName));

		memcpy(&lpCsGuildUnionInfo[iRetCount], szGuildName, 8);

		lpCsGuildUnionInfo[iRetCount].CsGuildID = iCsGuildID;

		++iRetCount;

		sqlRet = gQueryManager.Fetch();
	}

	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	*lpiCount = iRetCount;

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCsClearTotalGuildInfo(int iMapSvrGroup)
{
	if (!gQueryManager.ExecQuery("DELETE MuCastle_SIEGE_GUILDLIST WHERE MAP_SVR_GROUP = %d", iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCsSaveTotalGuildInfo(int iMapSvrGroup, char* lpszGuildName, int iCsGuildID, int iCsGuildInvolved, int iCsGuildScore)
{
	if (lpszGuildName == nullptr)
	{
		return false;
	}

	if (!gUtil.CheckTextSyntax(lpszGuildName, strlen(lpszGuildName)))
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

BOOL CCastleDBSet::DSDB_QueryCsSaveTotalGuildOK(int iMapSvrGroup, int* lpiResult)
{
	if (lpiResult == nullptr)
	{
		return false;
	}

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_SetSiegeGuildOK %d", iMapSvrGroup))
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

	*lpiResult = gQueryManager.GetAsInteger("QueryResult");

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCsLoadTotalGuildInfo(int iMapSvrGroup, CSP_CSLOADTOTALGUILDINFO* lpLoadTotalGuildInfo, int* lpiCount)
{
	if (lpLoadTotalGuildInfo == nullptr || lpiCount == nullptr)
	{
		return false;
	}

	int iMaxCount = *lpiCount;

	if (iMaxCount > 100)
	{
		iMaxCount = 100;
	}

	*lpiCount = 0;

	if (!gQueryManager.ExecQuery("EXEC WZ_CS_GetSiegeGuildInfo %d", iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	short sqlRet = gQueryManager.Fetch();

	while (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
	{
		if (*lpiCount >= iMaxCount)
		{
			break;
		}

		char szGuildName[16]{};

		gQueryManager.GetAsString("GUILD_NAME", szGuildName, sizeof(szGuildName));

		memcpy(&lpLoadTotalGuildInfo[*lpiCount], szGuildName, 8);

		lpLoadTotalGuildInfo[*lpiCount].CsGuildID = gQueryManager.GetAsInteger("GUILD_ID");
		lpLoadTotalGuildInfo[*lpiCount].GuildInvolved = gQueryManager.GetAsInteger("GUILD_INVOLVED");
		lpLoadTotalGuildInfo[*lpiCount].GuildScore = gQueryManager.GetAsInteger("GUILD_SCORE");

		++(*lpiCount);

		sqlRet = gQueryManager.Fetch();
	}

	if (sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	return true;
}

BOOL CCastleDBSet::DSDB_QueryCastleNpcUpdate(int iMapSvrGroup, CSP_REQ_NPCUPDATEDATA* lpNpcSaveData)
{
	if (lpNpcSaveData == nullptr)
	{
		return false;
	}

	CSP_NPCSAVEDATA* lpMsgBody = (CSP_NPCSAVEDATA*)&lpNpcSaveData[1];

	if (!gQueryManager.ExecQuery("DELETE MuCastle_NPC WHERE MAP_SVR_GROUP = %d", iMapSvrGroup))
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();

	for (int iNpcCount = 0; iNpcCount < lpNpcSaveData->Count; ++iNpcCount)
	{
		if (!gQueryManager.ExecQuery(
			"EXEC WZ_CS_ReqNpcUpdate %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			iMapSvrGroup,
			lpMsgBody[iNpcCount].NpcNumber,
			lpMsgBody[iNpcCount].NpcIndex,
			lpMsgBody[iNpcCount].NpcDfLevel,
			lpMsgBody[iNpcCount].NpcRgLevel,
			lpMsgBody[iNpcCount].NpcMaxHp,
			lpMsgBody[iNpcCount].NpcHp,
			lpMsgBody[iNpcCount].NpcX,
			lpMsgBody[iNpcCount].NpcY,
			lpMsgBody[iNpcCount].NpcDIR))
		{
			gQueryManager.Close();
			return false;
		}

		gQueryManager.Close();
	}

	return true;
}
