// GuildMatching.h
#pragma once
#include "CriticalSection.h"
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_GUILD_MATCHING_LIST_RECV
{
	PSBMSG_HEAD Header; // C1:28:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Page;
};

struct SDHP_GUILD_MATCHING_LIST_SEARCH_RECV
{
	PSBMSG_HEAD Header; // C1:28:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Page;
	char SearchWord[11];
};

struct SDHP_GUILD_MATCHING_INSERT_RECV
{
	PSBMSG_HEAD Header; // C1:28:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char Text[41];
	char GuildName[MAX_GUILD_NAME];
	DWORD GuildNumber;
	BYTE GuildMemberCount;
	WORD GuildMasterLevel;
	BYTE GuildMasterClass;
	BYTE InterestType;
	BYTE LevelRange;
	BYTE ClassType;
	BYTE GensType;
};

struct SDHP_GUILD_MATCHING_CANCEL_RECV
{
	PSBMSG_HEAD Header; // C1:28:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char GuildName[MAX_GUILD_NAME];
	BYTE Flag;
};

struct SDHP_GUILD_MATCHING_JOIN_INSERT_RECV
{
	PSBMSG_HEAD Header; // C1:28:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char GuildName[MAX_GUILD_NAME];
	BYTE Class;
	WORD Level;
};

struct SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV
{
	PSBMSG_HEAD Header; // C1:28:05
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Flag;
};

struct SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV
{
	PSBMSG_HEAD Header; // C1:28:06
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Type;
	char GuildName[MAX_GUILD_NAME];
	char MemberName[MAX_CHARACTER_NAME];
};

struct SDHP_GUILD_MATCHING_JOIN_LIST_RECV
{
	PSBMSG_HEAD Header; // C1:28:07
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char GuildName[MAX_GUILD_NAME];
};

struct SDHP_GUILD_MATCHING_JOIN_INFO_RECV
{
	PSBMSG_HEAD Header; // C1:28:08
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_GUILD_MATCHING_INSERT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:28:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacternName[MAX_CHARACTER_NAME];
	char GuildName[MAX_GUILD_NAME];
	BYTE GuildMemberCount;
	WORD GuildMasterLevel;
	BYTE GuildMasterClass;
	BYTE GensType;
};

//********** DataServer -> GameServer **********//

struct SDHP_GUILD_MATCHING_LIST_SEND
{
	PSWMSG_HEAD Header; // C2:28:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	DWORD CurPage;
	DWORD MaxPage;
	DWORD count;
};

struct SDHP_GUILD_MATCHING_LIST
{
	char Text[41];
	char CharacterName[MAX_CHARACTER_NAME];
	char GuildName[MAX_GUILD_NAME];
	BYTE GuildMemberCount;
	BYTE GuildMasterClass;
	BYTE InterestType;
	BYTE LevelRange;
	BYTE ClassType;
	DWORD GuildMasterLevel;
	DWORD BoardNumber;
	DWORD GuildNumber;
	BYTE GensType;
};

struct SDHP_GUILD_MATCHING_INSERT_SEND
{
	PSBMSG_HEAD Header; // C1:28:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
};

struct SDHP_GUILD_MATCHING_CANCEL_SEND
{
	PSBMSG_HEAD Header; // C1:28:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	BYTE Flag;
};

struct SDHP_GUILD_MATCHING_JOIN_INSERT_SEND
{
	PSBMSG_HEAD Header; // C1:28:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
};

struct SDHP_GUILD_MATCHING_JOIN_CANCEL_SEND
{
	PSBMSG_HEAD Header; // C1:28:05
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	BYTE Flag;
};

struct SDHP_GUILD_MATCHING_JOIN_ACCEPT_SEND
{
	PSBMSG_HEAD Header; // C1:28:06
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	DWORD Type;
	char GuildName[MAX_GUILD_NAME];
	char MemberName[MAX_CHARACTER_NAME];
};

struct SDHP_GUILD_MATCHING_JOIN_LIST_SEND
{
	PSWMSG_HEAD Header; // C2:28:07
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	DWORD Count;
};

struct SDHP_GUILD_MATCHING_JOIN_LIST
{
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Class;
	DWORD Level;
};

struct SDHP_GUILD_MATCHING_JOIN_INFO_SEND
{
	PSBMSG_HEAD Header; // C1:28:08
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	char GuildName[MAX_GUILD_NAME];
	char GuildMasterName[MAX_CHARACTER_NAME];
};

struct SDHP_GUILD_MATCHING_NOTIFY_SEND
{
	PSBMSG_HEAD Header; // C1:28:09
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	char GuildName[MAX_GUILD_NAME];
};

struct SDHP_GUILD_MATCHING_NOTIFY_MASTER_SEND
{
	PSBMSG_HEAD Header; // C1:28:0A
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
};

//**********************************************//

struct GUILD_MATCHING_INFO
{
	char CharacterName[MAX_CHARACTER_NAME];
	char Text[41];
	char GuildName[MAX_GUILD_NAME];
	BYTE GuildMemberCount;
	BYTE GuildMasterClass;
	BYTE InterestType;
	BYTE LevelRange;
	BYTE ClassType;
	DWORD GuildMasterLevel;
	DWORD BoardNumber;
	DWORD GuildNumber;
	BYTE GensType;
};

struct GUILD_MATCHING_JOIN_INFO
{
	char CharacterName[MAX_CHARACTER_NAME];
	char GuildName[MAX_GUILD_NAME];
	char GuildMasterName[MAX_CHARACTER_NAME];
	BYTE Class;
	DWORD Level;
};

class CGuildMatching
{
public:
	CGuildMatching() = default;
	~CGuildMatching() = default;
	void ClearGuildMatchingInfo(WORD serverCode);
	void ClearGuildMatchingJoinInfo(WORD serverCode);
	bool GetGuildMatchingInfo(GUILD_MATCHING_INFO* lpGuildMatchingInfo, const char* name);
	void InsertGuildMatchingInfo(const GUILD_MATCHING_INFO& guildMatchingInfo);
	void RemoveGuildMatchingInfo(const GUILD_MATCHING_INFO& guildMatchingInfo);
	bool GetGuildMatchingJoinInfo(GUILD_MATCHING_JOIN_INFO* lpGuildMatchingJoinInfo, const char* name);
	void InsertGuildMatchingJoinInfo(const GUILD_MATCHING_JOIN_INFO& guildMatchingJoinInfo);
	void RemoveGuildMatchingJoinInfo(const GUILD_MATCHING_JOIN_INFO& guildMatchingJoinInfo);
	void RemoveGuildMatchingJoinInfoNotifyAll(const GUILD_MATCHING_INFO& guildMatchingInfo);
	DWORD GenerateGuildMatchingList(DWORD* curPage,DWORD* maxPage,BYTE* lpMsg,int* size);
	DWORD GenerateGuildMatchingList(DWORD* curPage, DWORD* maxPage, const char* searchWord, BYTE* lpMsg, int* size);
	DWORD GenerateGuildMatchingJoinList(const char* guildName, BYTE* lpMsg, int* size);
	void GDGuildMatchingListRecv(const SDHP_GUILD_MATCHING_LIST_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingListSearchRecv(const SDHP_GUILD_MATCHING_LIST_SEARCH_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingInsertRecv(const SDHP_GUILD_MATCHING_INSERT_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingCancelRecv(const SDHP_GUILD_MATCHING_CANCEL_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingJoinInsertRecv(const SDHP_GUILD_MATCHING_JOIN_INSERT_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingJoinCancelRecv(const SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingJoinAcceptRecv(const SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingJoinListRecv(const SDHP_GUILD_MATCHING_JOIN_LIST_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingJoinInfoRecv(const SDHP_GUILD_MATCHING_JOIN_INFO_RECV* lpMsg, int serverIndex, int size);
	void GDGuildMatchingInsertSaveRecv(const SDHP_GUILD_MATCHING_INSERT_SAVE_RECV* lpMsg, int serverIndex, int size);
	void DGGuildMatchingNotifySend(const char* characterName, const char* guildName, DWORD result);
	void DGGuildMatchingNotifyMasterSend(const char* characterName, DWORD result);
private:
	CCriticalSection m_critical;
	std::map<std::string,GUILD_MATCHING_INFO> m_GuildMatchingInfo;
	std::map<std::string,GUILD_MATCHING_JOIN_INFO> m_GuildMatchingJoinInfo;
};

extern CGuildMatching gGuildMatching;
