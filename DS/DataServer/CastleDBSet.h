// CastleDBSet.h
#pragma once
#include "DataServerProtocol.h"

struct CASTLE_DATA
{
	WORD StartYear;
	BYTE StartMonth;
	BYTE StartDay;
	WORD EndYear;
	BYTE EndMonth;
	BYTE EndDay;
	BYTE IsSiegeGuildList;
	BYTE IsSiegeEnded;
	BYTE IsCastleOccupied;
	char CastleOwnGuild[9];
	__int64 CastleMoney;
	int TaxRateChaos;
	int TaxRateStore;
	int TaxHuntZone;
	int FirstCreate;
};

struct CSP_ANS_CASTLEDATA 
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	WORD StartYear;
	BYTE StartMonth;
	BYTE StartDay;
	WORD EndYear;
	BYTE EndMonth;
	BYTE EndDay;
	BYTE IsSiegeGuildList;
	BYTE IsSiegeEnded;
	BYTE IsCastleOccupied;
	char CastleOwnGuild[MAX_GUILD_NAME];
	__int64 CastleMoney;
	int TaxRateChaos;
	int TaxRateStore;
	int TaxHuntZone;
	int FirstCreate;
};

struct CSP_REQ_CASTLEDATA
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int CastleEventCycle;
};

struct CSP_REQ_OWNERGUILDMASTER
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
};

struct CSP_ANS_OWNERGUILDMASTER
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char CastleOwnGuild[MAX_GUILD_NAME];
	char CastleOwnGuildMaster[MAX_CHARACTER_NAME];
};

struct CSP_REQ_NPCBUY
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	int NpcNumber;
	int NpcIndex;
	int NpcDfLevel;
	int NpcRgLevel;
	int NpcMaxHp;
	int NpcHp;
	BYTE NpcX;
	BYTE NpcY;
	BYTE NpcDIR;
	int BuyCost;
};

struct CSP_REQ_NPCREPAIR
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	int NpcNumber;
	int NpcIndex;
	int RepairCost;
};

struct CSP_ANS_NPCREPAIR
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int NpcNumber;
	int NpcIndex;
	int NpcMaxHp;
	int NpcHp;
	int RepairCost;
};

struct CSP_REQ_NPCUPGRADE
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	int NpcNumber;
	int NpcIndex;
	int NpcUpType;
	int NpcUpValue;
	int NpcUpIndex;
};

struct CSP_ANS_TAXINFO
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	__int64 CastleMoney;
	int TaxRateChaos;
	int TaxRateStore;
	int TaxHuntZone;
};

struct CSP_ANS_TAXRATECHANGE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int TaxKind;
	int TaxRate;
};

struct CSP_REQ_SDEDCHANGE
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	WORD StartYear;
	BYTE StartMonth;
	BYTE StartDay;
	WORD EndYear;
	BYTE EndMonth;
	BYTE EndDay;
};

struct CSP_ANS_SDEDCHANGE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	WORD StartYear;
	BYTE StartMonth;
	BYTE StartDay;
	WORD EndYear;
	BYTE EndMonth;
	BYTE EndDay;
};

struct CSP_REQ_GUILDREGINFO
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
};

struct CSP_ANS_GUILDREGINFO
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
	int RegMarkCount;
	bool IsGiveUp;
	BYTE RegRank;
};

struct CSP_ANS_NPCBUY
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int NpcNumber;
	int NpcIndex;
	int BuyCost;
};

struct CSP_ANS_NPCUPGRADE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int NpcNumber;
	int NpcIndex;
	int NpcUpType;
	int NpcUpValue;
	int NpcUpIndex;
};

struct CSP_REQ_TAXINFO
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
};

struct CSP_REQ_TAXRATECHANGE
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	int TaxKind;
	int TaxRate;
};

struct CSP_REQ_MONEYCHANGE
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	int MoneyChanged;
};

struct CSP_ANS_MONEYCHANGE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int MoneyChanged;
	__int64 CastleMoney;
};

struct CSP_REQ_SIEGEENDCHANGE
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int IsSiegeEnded;
};

struct CSP_ANS_SIEGEENDCHANGE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int IsSiegeEnded;
};

struct CSP_REQ_CASTLEOWNERCHANGE
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	BOOL IsCastleOccupied;
	char OwnerGuildName[MAX_GUILD_NAME];
};

struct CSP_ANS_CASTLEOWNERCHANGE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	BOOL IsCastleOccupied;
	char OwnerGuildName[MAX_GUILD_NAME];
};

struct CSP_REQ_REGATTACKGUILD
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char EnemyGuildName[MAX_GUILD_NAME];
};

struct CSP_ANS_REGATTACKGUILD
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char EnemyGuildName[MAX_GUILD_NAME];
};

struct CSP_REQ_MAPSVRMULTICAST
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	char MsgText[128];
};

struct CSP_ANS_MAPSVRMULTICAST
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	char MsgText[128];
};

struct CSP_REQ_CASTLESIEGEEND
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_ANS_CASTLESIEGEEND
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_ANS_GUILDREGMARK
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
	int ItemPos;
	int RegMarkCount;
};

struct CSP_REQ_GUILDREGMARK
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
	int ItemPos;
};

struct CSP_ANS_GUILDRESETMARK
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
	int RegMarkCount;
};

struct CSP_REQ_GUILDRESETMARK
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
};

struct CSP_ANS_GUILDSETGIVEUP
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
	BOOL IsGiveUp;
	int RegMarkCount;
};

struct CSP_REQ_GUILDSETGIVEUP
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
	BOOL IsGiveUp;
};

struct CSP_REQ_NPCREMOVE
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int NpcNumber;
	int NpcIndex;
};

struct CSP_ANS_NPCREMOVE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int NpcNumber;
	int NpcIndex;
};

struct CSP_REQ_CASTLESTATESYNC
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int CastleState;
	int TaxRateChaos;
	int TaxRateStore;
	int TaxHuntZone;
	char OwnerGuildName[MAX_GUILD_NAME];
};

struct CSP_ANS_CASTLESTATESYNC
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int CastleState;
	int TaxRateChaos;
	int TaxRateStore;
	int TaxHuntZone;
	char OwnerGuildName[MAX_GUILD_NAME];
};

struct CSP_NPCDATA
{
	int NpcNumber;
	int NpcIndex;
	int NpcDfLevel;
	int NpcRgLevel;
	int NpcMaxHp;
	int NpcHp;
	BYTE NpcX;
	BYTE NpcY;
	BYTE NpcDIR;
};

struct CSP_GUILDREGINFO
{
	char GuildName[MAX_GUILD_NAME];
	int RegMarkCount;
	bool IsGiveUp;
	BYTE RegRank;
};

struct CSP_REQ_NPCSAVEDATA
{
	PWMSG_HEAD Header;
	WORD MapSvrNum;
	int Count;
};

struct CSP_ANS_NPCSAVEDATA
{
	PBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_CALCREGGUILDLIST
{
	char GuildName[MAX_GUILD_NAME];
	int RegMarkCount;
	int GuildMemberCount;
	int GuildMasterLevel;
	int SeqNum;
};

struct CSP_CSGUILDUNIONINFO
{
	char GuildName[MAX_GUILD_NAME];
	int CsGuildID;
};

struct CSP_CSLOADTOTALGUILDINFO
{
	char GuildName[MAX_GUILD_NAME];
	int CsGuildID;
	int GuildInvolved;
	int GuildScore;
};

struct CSP_REQ_NPCUPDATEDATA
{
	PWMSG_HEAD Header;
	WORD MapSvrNum;
	int Count;
};

struct CSP_NPCSAVEDATA
{
	int NpcNumber;
	int NpcIndex;
	int NpcDfLevel;
	int NpcRgLevel;
	int NpcMaxHp;
	int NpcHp;
	BYTE NpcX;
	BYTE NpcY;
	BYTE NpcDIR;
};

struct CSP_REQ_CASTLETRIBUTEMONEY
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int CastleTributeMoney;
};

struct CSP_ANS_CASTLETRIBUTEMONEY
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_REQ_RESETCASTLETAXINFO
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_ANS_RESETCASTLETAXINFO
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_REQ_RESETSIEGEGUILDINFO
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_ANS_RESETSIEGEGUILDINFO
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_REQ_CSINITDATA
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
	int CastleEventCycle;
};

struct CSP_ANS_CSINITDATA
{
	PWMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	WORD StartYear;
	BYTE StartMonth;
	BYTE StartDay;
	WORD EndYear;
	BYTE EndMonth;
	BYTE EndDay;
	BYTE IsSiegeGuildList;
	BYTE IsSiegeEnded;
	BYTE IsCastleOccupied;
	char CastleOwnGuild[8];
	__int64 CastleMoney;
	int TaxRateChaos;
	int TaxRateStore;
	int TaxHuntZone;
	int FirstCreate;
	int Count;
};

struct CSP_REQ_NPCDATA
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
};

struct CSP_ANS_NPCDATA
{
	PWMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int Count;
};

struct CSP_REQ_ALLGUILDREGINFO
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
};

struct CSP_ANS_ALLGUILDREGINFO
{
	PWMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int Count;
};

struct CSP_REQ_CALCREGGUILDLIST
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_ANS_CALCREGGUILDLIST
{
	PWMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Count;
};

struct CSP_REQ_CSGUILDUNIONINFO
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
	int Count;
};

struct CSP_ANS_CSGUILDUNIONINFO
{
	PWMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Count;
};

struct CSP_REQ_CSSAVETOTALGUILDINFO
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
	int Count;
};

struct CSP_CSSAVETOTALGUILDINFO
{
	char GuildName[MAX_GUILD_NAME];
	int CsGuildID;
	int GuildInvolved;
	int GuildScore;
};

struct CSP_ANS_CSSAVETOTALGUILDINFO
{
	PBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_REQ_CSLOADTOTALGUILDINFO
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_ANS_CSLOADTOTALGUILDINFO
{
	PWMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Count;
};

struct CSP_NPCUPDATEDATA
{
	int NpcNumber;
	int NpcIndex;
	int NpcDfLevel;
	int NpcRgLevel;
	int NpcMaxHp;
	int NpcHp;
	BYTE NpcX;
	BYTE NpcY;
	BYTE NpcDIR;
};

class CCastleDBSet
{
public:
	CCastleDBSet() = default;
	~CCastleDBSet() = default;
	BOOL DB_QueryCastleTotalInfo(int iMapSvrGroup, int iCastleEventCycle, CASTLE_DATA* lpCastleData);
	BOOL DSDB_QueryOwnerGuildMaster(int iMapSvrGroup, CSP_ANS_OWNERGUILDMASTER* lpOwnerGuildMaster);
	BOOL DSDB_QueryCastleNpcBuy(int iMapSvrGroup, CSP_REQ_NPCBUY* lpNpcBuy, int* lpiResult);
	BOOL DSDB_QueryCastleNpcRepair(int iMapSvrGroup, CSP_REQ_NPCREPAIR* lpNpcRepair, CSP_ANS_NPCREPAIR* lpNpcRepairResult,  int* lpiResult);
	BOOL DSDB_QueryCastleNpcUpgrade(int iMapSvrGroup, CSP_REQ_NPCUPGRADE* lpNpcUpgrade);
	BOOL DSDB_QueryTaxInfo(int iMapSvrGroup, CSP_ANS_TAXINFO* lpTaxInfo);
	BOOL DSDB_QueryTaxRateChange(int iMapSvrGroup, int iTaxType, int iTaxRate,  CSP_ANS_TAXRATECHANGE* lpTaxRateChange, int* lpiResult);
	BOOL DSDB_QueryCastleMoneyChange(int iMapSvrGroup, int iMoneyChange, __int64* i64ResultMoney,  int* lpiResult);
	BOOL DSDB_QuerySiegeDateChange(int iMapSvrGroup, CSP_REQ_SDEDCHANGE* lpSdEdChange, int* lpiResult);
	BOOL DSDB_QueryGuildMarkRegInfo(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDREGINFO* lpGuildRegInfo,  int* lpiResult);
	BOOL DSDB_QuerySiegeEndedChange(int iMapSvrGroup, int bIsCastleSiegeEnded, int* lpiResult);
	BOOL DSDB_QueryCastleOwnerChange(int iMapSvrGroup, CSP_REQ_CASTLEOWNERCHANGE* lpCastleOwnerInfo, CSP_ANS_CASTLEOWNERCHANGE* lpCastleOwnerInfoResult,  int* lpiResult);
	BOOL DSDB_QueryRegAttackGuild(int iMapSvrGroup, CSP_REQ_REGATTACKGUILD* lpRegAttackGuild, CSP_ANS_REGATTACKGUILD* lpRegAttackGuildResult,  int* lpiResult);
	BOOL DSDB_QueryRestartCastleState(int iMapSvrGroup, CSP_REQ_CASTLESIEGEEND* lpCastleSiegeEnd, int* lpiResult);
	BOOL DSDB_QueryGuildMarkRegMark(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDREGMARK* lpGuildRegMark,  int* lpiResult);
	BOOL DSDB_QueryGuildMarkReset(int iMapSvrGroup, char* lpszGuildName, CSP_ANS_GUILDRESETMARK* lpGuildResetMark);
	BOOL DSDB_QueryGuildSetGiveUp(int iMapSvrGroup, char* lpszGuildName, int bIsGiveUp,  CSP_ANS_GUILDSETGIVEUP* lpGuildSetGiveUp);
	BOOL DSDB_QueryCastleNpcRemove(int iMapSvrGroup, CSP_REQ_NPCREMOVE* lpNpcRemove, int* lpiResult);
	BOOL DSDB_QueryResetCastleTaxInfo(int iMapSvrGroup, int* lpiResult);
	BOOL DSDB_QueryResetSiegeGuildInfo(int iMapSvrGroup, int* lpiResult);
	BOOL DSDB_QueryResetRegSiegeInfo(int iMapSvrGroup, int* lpiResult);
	BOOL DSDB_QueryCastleNpcInfo(int iMapSvrGroup, CSP_NPCDATA* lpNpcData, int* lpiCount);
	BOOL DSDB_QueryAllGuildMarkRegInfo(int iMapSvrGroup, CSP_GUILDREGINFO* lpGuildRegInfo, int* lpiCount);
	BOOL DSDB_QueryFirstCreateNPC(int iMapSvrGroup, CSP_REQ_NPCSAVEDATA* lpNpcSaveData);
	BOOL DSDB_QueryCalcRegGuildList(int iMapSvrGroup, CSP_CALCREGGUILDLIST* lpCalcRegGuildList, int* lpiCount);
	BOOL DSDB_QueryCsGuildUnionInfo(int iMapSvrGroup, char* lpszGuildName, int iCsGuildID,  CSP_CSGUILDUNIONINFO* lpCsGuildUnionInfo, int* lpiCount);
	BOOL DSDB_QueryCsClearTotalGuildInfo(int iMapSvrGroup);
	BOOL DSDB_QueryCsSaveTotalGuildInfo(int iMapSvrGroup, char* lpszGuildName, int iCsGuildID,  int iCsGuildInvolved, int iCsGuildScore);
	BOOL DSDB_QueryCsSaveTotalGuildOK(int iMapSvrGroup, int* lpiResult);
	BOOL DSDB_QueryCsLoadTotalGuildInfo(int iMapSvrGroup, CSP_CSLOADTOTALGUILDINFO* lpLoadTotalGuildInfo, int* lpiCount);
	BOOL DSDB_QueryCastleNpcUpdate(int iMapSvrGroup, CSP_REQ_NPCUPDATEDATA* lpNpcSaveData);
};

extern CCastleDBSet gCastleDBSet;
