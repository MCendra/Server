// CastleSiege.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer 
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

struct CSP_REQ_GUILDREGINFO
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
};

struct CSP_REQ_SIEGEENDCHANGE
{
	PSBMSG_HEAD Header;
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

struct CSP_REQ_REGATTACKGUILD
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char EnemyGuildName[MAX_GUILD_NAME];
};

struct CSP_REQ_RESTARTCASTLESTATE
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_REQ_MAPSVRMULTICAST
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	char MsgText[128];
};

struct CSP_REQ_GUILDREGMARK
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
	int ItemPos;
};

struct CSP_REQ_GUILDRESETMARK
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
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

struct CSP_REQ_CASTLETRIBUTEMONEY
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	int CastleTributeMoney;
};

struct CSP_REQ_RESETCASTLETAXINFO
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_REQ_RESETSIEGEGUILDINFO
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_REQ_CSINITDATA
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
	int CastleEventCycle;
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

struct CSP_REQ_CALCREGGUILDLIST
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
};

struct CSP_REQ_CSGUILDUNIONINFO
{
	PBMSG_HEAD Header;
	WORD MapSvrNum;
	int Count;
};

struct CSP_ANS_CALCREGGUILDLIST
{
	PWMSG_HEAD Header;
	int Result;
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

struct CSP_REQ_NPCUPDATEDATA
{
	PWMSG_HEAD Header;
	WORD MapSvrNum;
	int Count;
};

// DataServer -> GameServer

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

struct CSP_ANS_OWNERGUILDMASTER
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char CastleOwnGuild[MAX_GUILD_NAME];
	char CastleOwnGuildMaster[MAX_CHARACTER_NAME];
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

struct CSP_ANS_MONEYCHANGE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int MoneyChanged;
	__int64 CastleMoney;
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

struct CSP_ANS_SIEGEENDCHANGE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int IsSiegeEnded;
};

struct CSP_ANS_CASTLEOWNERCHANGE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	BOOL IsCastleOccupied;
	char OwnerGuildName[MAX_GUILD_NAME];
};

struct CSP_ANS_REGATTACKGUILD
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char EnemyGuildName[MAX_GUILD_NAME];
};

struct CSP_ANS_CASTLESIEGEEND
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_ANS_MAPSVRMULTICAST
{
	PSBMSG_HEAD Header;
	WORD MapSvrNum;
	char MsgText[128];
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

struct CSP_ANS_GUILDRESETMARK
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	char GuildName[MAX_GUILD_NAME];
	int RegMarkCount;
};

struct CSP_ANS_NPCREMOVE
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int NpcNumber;
	int NpcIndex;
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

struct CSP_ANS_CASTLETRIBUTEMONEY
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_ANS_RESETCASTLETAXINFO
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
};

struct CSP_ANS_RESETSIEGEGUILDINFO
{
	PSBMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
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

struct CSP_ANS_ALLGUILDREGINFO
{
	PWMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Index;
	int Count;
};

struct CSP_GUILDREGINFO
{
	char GuildName[MAX_GUILD_NAME];
	int RegMarkCount;
	bool IsGiveUp;
	BYTE RegRank;
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

struct CSP_CSSAVETOTALGUILDINFO
{
	char GuildName[MAX_GUILD_NAME];
	int CsGuildID;
	int GuildInvolved;
	int GuildScore;
};

struct CSP_ANS_CSLOADTOTALGUILDINFO
{
	PWMSG_HEAD Header;
	int Result;
	WORD MapSvrNum;
	int Count;
};

struct CSP_CSLOADTOTALGUILDINFO
{
	char GuildName[MAX_GUILD_NAME];
	int CsGuildID;
	int GuildInvolved;
	int GuildScore;
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

// DB
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

//**********************************************//

class CCastleSiege
{
public:
	CCastleSiege() = default;
	~CCastleSiege() = default;
	void DS_GDReqCastleTotalInfo(const CSP_REQ_CASTLEDATA* lpMsg, int serverIndex, int size);
	void DS_GDReqOwnerGuildMaster(const CSP_REQ_OWNERGUILDMASTER* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleNpcBuy(const CSP_REQ_NPCBUY* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleNpcRepair(const CSP_REQ_NPCREPAIR* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleNpcUpgrade(const CSP_REQ_NPCUPGRADE* lpMsg, int serverIndex, int size);
	void DS_GDReqTaxInfo(const CSP_REQ_TAXINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqTaxRateChange(const CSP_REQ_TAXRATECHANGE* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleMoneyChange(const CSP_REQ_MONEYCHANGE* lpMsg, int serverIndex, int size);
	void DS_GDReqSiegeDateChange(const CSP_REQ_SDEDCHANGE* lpMsg, int serverIndex, int size);
	void DS_GDReqGuildMarkRegInfo(const CSP_REQ_GUILDREGINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqSiegeEndedChange(const CSP_REQ_SIEGEENDCHANGE* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleOwnerChange(const CSP_REQ_CASTLEOWNERCHANGE* lpMsg, int serverIndex, int size);
	void DS_GDReqRegAttackGuild(const CSP_REQ_REGATTACKGUILD* lpMsg, int serverIndex, int size);
	void DS_GDReqRestartCastleState(const CSP_REQ_RESTARTCASTLESTATE* lpMsg, int serverIndex, int size);
	void DS_GDReqMapSvrMsgMultiCast(const CSP_REQ_MAPSVRMULTICAST* lpMsg, int serverIndex, int size);
	void DS_GDReqRegGuildMark(const CSP_REQ_GUILDREGMARK* lpMsg, int serverIndex, int size);
	void DS_GDReqGuildMarkReset(const CSP_REQ_GUILDRESETMARK* lpMsg, int serverIndex, int size);
	void DS_GDReqGuildSetGiveUp(const CSP_REQ_GUILDSETGIVEUP* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleNpcRemove(const CSP_REQ_NPCREMOVE* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleStateSync(const CSP_REQ_CASTLESTATESYNC* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleTributeMoney(const CSP_REQ_CASTLETRIBUTEMONEY* lpMsg, int serverIndex, int size);
	void DS_GDReqResetCastleTaxInfo(const CSP_REQ_RESETCASTLETAXINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqResetSiegeGuildInfo(const CSP_REQ_RESETSIEGEGUILDINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqResetRegSiegeInfo(const CSP_REQ_RESETSIEGEGUILDINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleInitData(const CSP_REQ_CSINITDATA* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleNpcInfo(const CSP_REQ_NPCDATA* lpMsg, int serverIndex, int size);
	void DS_GDReqAllGuildMarkRegInfo(const CSP_REQ_ALLGUILDREGINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqFirstCreateNPC(const CSP_REQ_NPCSAVEDATA* lpMsg, int serverIndex, int size);
	void DS_GDReqCalcRegGuildList(const CSP_REQ_CALCREGGUILDLIST* lpMsg, int serverIndex, int size);
	void DS_GDReqCsGuildUnionInfo(const CSP_REQ_CSGUILDUNIONINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqCsSaveTotalGuildInfo(const CSP_REQ_CSSAVETOTALGUILDINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqCsLoadTotalGuildInfo(const CSP_REQ_CSLOADTOTALGUILDINFO* lpMsg, int serverIndex, int size);
	void DS_GDReqCastleNpcUpdate(const CSP_REQ_NPCUPDATEDATA* lpMsg, int serverIndex, int size);
private:
	bool DB_QueryCastleTotalInfo(int iMapSvrGroup, int iCastleEventCycle, CASTLE_DATA* lpCastleData);
	bool DB_QueryOwnerGuildMaster(int iMapSvrGroup, CSP_ANS_OWNERGUILDMASTER* lpOwnerGuildMaster);
	bool DB_QueryCastleNpcBuy(int iMapSvrGroup, const CSP_REQ_NPCBUY* lpNpcBuy, int* lpiResult);
	bool DB_QueryCastleNpcRepair(int iMapSvrGroup, const CSP_REQ_NPCREPAIR* lpNpcRepair, CSP_ANS_NPCREPAIR* lpNpcRepairResult, int* lpiResult);
	bool DB_QueryCastleNpcUpgrade(int iMapSvrGroup, const CSP_REQ_NPCUPGRADE* lpNpcUpgrade);
	bool DB_QueryTaxInfo(int iMapSvrGroup, CSP_ANS_TAXINFO* lpTaxInfo);
	bool DB_QueryTaxRateChange(int iMapSvrGroup, int iTaxType, int iTaxRate, CSP_ANS_TAXRATECHANGE* lpTaxRateChange, int* lpiResult);
	bool DB_QueryCastleMoneyChange(int iMapSvrGroup, int iMoneyChange, std::int64_t* resultMoney,	int* result);
	bool DB_QuerySiegeDateChange(int iMapSvrGroup, const CSP_REQ_SDEDCHANGE* lpSdEdChange, int* result);
	bool DB_QueryGuildMarkRegInfo(int iMapSvrGroup, const char* lpszGuildName, CSP_ANS_GUILDREGINFO* lpGuildRegInfo, int* lpiResult);
	bool DB_QuerySiegeEndedChange(int iMapSvrGroup, int bIsCastleSiegeEnded, int* lpiResult);
	bool DB_QueryCastleOwnerChange(int iMapSvrGroup, const CSP_REQ_CASTLEOWNERCHANGE* lpCastleOwnerInfo, CSP_ANS_CASTLEOWNERCHANGE* lpCastleOwnerInfoResult, int* lpiResult);
	bool DB_QueryRegAttackGuild(int iMapSvrGroup, const CSP_REQ_REGATTACKGUILD* lpRegAttackGuild, CSP_ANS_REGATTACKGUILD* lpRegAttackGuildResult, int* lpiResult);
	bool DB_QueryRestartCastleState(int iMapSvrGroup, int* lpiResult);
	bool DB_QueryGuildMarkRegMark(int iMapSvrGroup,	const char* lpszGuildName, CSP_ANS_GUILDREGMARK* lpGuildRegMark,int* lpiResult);
	bool DB_QueryGuildMarkReset(int iMapSvrGroup, const char* lpszGuildName, CSP_ANS_GUILDRESETMARK* lpGuildResetMark);
	bool DB_QueryGuildSetGiveUp(int iMapSvrGroup, const char* lpszGuildName, int bIsGiveUp, CSP_ANS_GUILDSETGIVEUP* lpGuildSetGiveUp);
	bool DB_QueryCastleNpcRemove(int iMapSvrGroup, const CSP_REQ_NPCREMOVE* lpNpcRemove, int* lpiResult);
	bool DB_QueryResetCastleTaxInfo(int iMapSvrGroup, int* lpiResult);
	bool DB_QueryResetSiegeGuildInfo(int iMapSvrGroup, int* lpiResult);
	bool DB_QueryResetRegSiegeInfo(int iMapSvrGroup, int* lpiResult);
	bool DB_QueryCastleNpcInfo(int iMapSvrGroup, CSP_NPCDATA* lpNpcData, int* lpiCount);
	bool DB_QueryAllGuildMarkRegInfo(int iMapSvrGroup, CSP_GUILDREGINFO* lpGuildRegInfo, int* lpiCount);
	bool DB_QueryFirstCreateNPC(int iMapSvrGroup, const CSP_NPCSAVEDATA* npcData, int count);
	bool DB_QueryCalcRegGuildList(int iMapSvrGroup, CSP_CALCREGGUILDLIST* lpCalcRegGuildList, int* lpiCount);
	bool DB_QueryCsGuildUnionInfo(int iMapSvrGroup, const char* guildName, int csGuildID, CSP_CSGUILDUNIONINFO* guildUnionInfo, int* count);
	bool DB_QueryCsClearTotalGuildInfo(int iMapSvrGroup);
	bool DB_QueryCsSaveTotalGuildInfo(int iMapSvrGroup, const char* lpszGuildName, int iCsGuildID, int iCsGuildInvolved, int iCsGuildScore);
	bool DB_QueryCsSaveTotalGuildOK(int iMapSvrGroup, int* lpiResult);
	bool DB_QueryCsLoadTotalGuildInfo(int iMapSvrGroup, CSP_CSLOADTOTALGUILDINFO* lpLoadTotalGuildInfo, int* lpiCount);
	bool DB_QueryCastleNpcUpdate(int iMapSvrGroup, const CSP_NPCUPDATEDATA* npcData, int count);
};

extern CCastleSiege gCastleSiege;