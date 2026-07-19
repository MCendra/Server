// PartyMatching.h
#pragma once
#include "CriticalSection.h"
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_PARTY_MATCHING_INSERT_RECV
{
	PSBMSG_HEAD Header; // C1:29:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char Text[41];
	char Password[5];
	WORD MinLevel;
	WORD MaxLevel;
	WORD HuntingGround;
	WORD LeaderLevel;
	BYTE LeaderClass;
	BYTE WantedClass;
	BYTE WantedClassDetailInfo[7];
	BYTE PartyMemberCount;
	BYTE ApprovalType;
	BYTE UsePassword;
	BYTE GensType;
};

struct SDHP_PARTY_MATCHING_LIST_RECV
{
	PSBMSG_HEAD Header; // C1:29:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Page;
	WORD Level;
	WORD Class;
	BYTE GensType;
	BYTE UseSearchWord;
	char SearchWord[11];
};

struct SDHP_PARTY_MATCHING_JOIN_INSERT_RECV
{
	PSBMSG_HEAD Header; // C1:29:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char LeaderName[11];
	char Password[5];
	WORD Level;
	BYTE Class;
	BYTE UsePassword;
	BYTE UseRandomParty;
	BYTE GensType;
};

struct SDHP_PARTY_MATCHING_JOIN_INFO_RECV
{
	PSBMSG_HEAD Header; // C1:29:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_PARTY_MATCHING_JOIN_LIST_RECV
{
	PSBMSG_HEAD Header; // C1:29:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV
{
	PSBMSG_HEAD Header; // C1:29:05
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char MemberName[11];
	BYTE Type;
};

struct SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV
{
	PSBMSG_HEAD Header; // C1:29:06
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Type;
	BYTE Flag;
};

struct SDHP_PARTY_MATCHING_INSERT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:29:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD LeaderLevel;
	BYTE LeaderClass;
	BYTE PartyMemberCount;
	BYTE GensType;
};

// DataServer -> GameServer

struct SDHP_PARTY_MATCHING_INSERT_SEND
{
	PSBMSG_HEAD Header; // C1:29:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
};

struct SDHP_PARTY_MATCHING_LIST_SEND
{
	PSWMSG_HEAD Header; // C2:29:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	DWORD CurPage;
	DWORD MaxPage;
	DWORD Count;
};

struct SDHP_PARTY_MATCHING_LIST
{
	char CharacterName[MAX_CHARACTER_NAME];
	char Text[41];
	WORD MinLevel;
	WORD MaxLevel;
	WORD HuntingGround;
	DWORD LeaderLevel;
	BYTE LeaderClass;
	BYTE WantedClass;
	BYTE PartyMemberCount;
	BYTE UsePassword;
	BYTE WantedClassDetailInfo[7];
	DWORD ServerCode;
	BYTE GensType;
};

struct SDHP_PARTY_MATCHING_JOIN_INSERT_SEND
{
	PSBMSG_HEAD Header; // C1:29:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
};

struct SDHP_PARTY_MATCHING_JOIN_INFO_SEND
{
	PSBMSG_HEAD Header; // C1:29:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	DWORD LeaderServerCode;
	char LeaderName[11];
};

struct SDHP_PARTY_MATCHING_JOIN_LIST_SEND
{
	PSWMSG_HEAD Header; // C2:29:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	DWORD count;
};

struct SDHP_PARTY_MATCHING_JOIN_LIST
{
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Class;
	DWORD Level;
};

struct SDHP_PARTY_MATCHING_JOIN_ACCEPT_SEND
{
	PSBMSG_HEAD Header; // C1:29:05
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	char MemberName[11];
	BYTE Type;
	BYTE Flag;
};

struct SDHP_PARTY_MATCHING_JOIN_CANCEL_SEND
{
	PSBMSG_HEAD Header; // C1:29:06
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	BYTE Type;
	BYTE Flag;
};

struct SDHP_PARTY_MATCHING_NOTIFY_SEND
{
	PSBMSG_HEAD Header; // C1:29:07
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
	char LeaderName[11];
};

struct SDHP_PARTY_MATCHING_NOTIFY_LEADER_SEND
{
	PSBMSG_HEAD Header; // C1:29:08
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Result;
};

//**********************************************//

struct PARTY_MATCHING_INFO
{
	char CharacterName[MAX_CHARACTER_NAME];
	char Text[41];
	char Password[5];
	WORD MinLevel;
	WORD MaxLevel;
	WORD HuntingGround;
	WORD LeaderLevel;
	BYTE LeaderClass;
	BYTE WantedClass;
	BYTE WantedClassDetailInfo[7];
	BYTE PartyMemberCount;
	BYTE ApprovalType;
	BYTE UsePassword;
	BYTE GensType;
	WORD ServerCode;
};

struct PARTY_MATCHING_JOIN_INFO
{
	char CharacterName[MAX_CHARACTER_NAME];
	char LeaderName[11];
	WORD LeaderServerCode;
	BYTE Class;
	DWORD Level;
};

class CPartyMatching
{
public:
	CPartyMatching() = default;
	~CPartyMatching() = default;
	void ClearPartyMatchingInfo(WORD ServerCode);
	void ClearPartyMatchingJoinInfo(WORD ServerCode);
	bool GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo, const char* characterName);
	bool GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo, WORD ServerCode, WORD Level, BYTE Class, BYTE GensType);
	void InsertPartyMatchingInfo(PARTY_MATCHING_INFO PartyMatchingInfo);
	void RemovePartyMatchingInfo(PARTY_MATCHING_INFO PartyMatchingInfo);
	bool GetPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO* lpPartyMatchingJoinInfo, const char* characterName);
	void InsertPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo);
	void RemovePartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo);
	void RemovePartyMatchingJoinInfoNotifyAll(PARTY_MATCHING_INFO PartyMatchingInfo);
	DWORD GeneratePartyMatchingList(DWORD* CurPage, DWORD* MaxPage, BYTE UseSearchWord, const char* SearchWord, BYTE* lpMsg, int* size);
	DWORD GeneratePartyMatchingJoinList(const char* LeaderName, BYTE* lpMsg, int* size);
	void GDPartyMatchingInsertRecv(const SDHP_PARTY_MATCHING_INSERT_RECV* lpMsg, int serverIndex, int size);
	void GDPartyMatchingListRecv(const SDHP_PARTY_MATCHING_LIST_RECV* lpMsg, int serverIndex, int size);
	void GDPartyMatchingJoinInsertRecv(const SDHP_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg, int serverIndex, int size);
	void GDPartyMatchingJoinInfoRecv(const SDHP_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg, int serverIndex, int size);
	void GDPartyMatchingJoinListRecv(const SDHP_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg, int serverIndex, int size);
	void GDPartyMatchingJoinAcceptRecv(const SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg, int serverIndex, int size);
	void GDPartyMatchingJoinCancelRecv(const SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg, int serverIndex, int size);
	void GDPartyMatchingInsertSaveRecv(const SDHP_PARTY_MATCHING_INSERT_SAVE_RECV* lpMsg, int serverIndex, int size);
	void DGPartyMatchingNotifySend(const char* name, const char* LeaderName, DWORD result);
	void DGPartyMatchingNotifyLeaderSend(const char* name, DWORD result);
private:
	CCriticalSection m_critical;
	std::map<std::string,PARTY_MATCHING_INFO> m_PartyMatchingInfo;
	std::map<std::string,PARTY_MATCHING_JOIN_INFO> m_PartyMatchingJoinInfo;
};

extern CPartyMatching gPartyMatching;
