// GensSystem.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_GENS_SYSTEM_INSERT_RECV
{
	PSBMSG_HEAD Header; // C1:11:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE GensFamily;
};

struct SDHP_GENS_SYSTEM_DELETE_RECV
{
	PSBMSG_HEAD Header; // C1:11:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE GensFamily;
	DWORD GensRank;
};

struct SDHP_GENS_SYSTEM_MEMBER_RECV
{
	PSBMSG_HEAD Header; // C1:11:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_GENS_SYSTEM_UPDATE_RECV
{
	PSBMSG_HEAD Header; // C1:11:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensContribution;
};

struct SDHP_GENS_SYSTEM_REWARD_RECV
{
	PSBMSG_HEAD Header; // C1:11:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE GensFamily;
};

struct SDHP_GENS_SYSTEM_REWARD_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:11:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensRewardStatus;
};

//********** DataServer -> GameServer **********//

struct SDHP_GENS_SYSTEM_INSERT_SEND
{
	PSBMSG_HEAD Header; // C1:11:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensContribution;
};

struct SDHP_GENS_SYSTEM_DELETE_SEND
{
	PSBMSG_HEAD Header; // C1:11:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
};

struct SDHP_GENS_SYSTEM_MEMBER_SEND
{
	PSBMSG_HEAD Header; // C1:11:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensContribution;
};

struct SDHP_GENS_SYSTEM_UPDATE_SEND
{
	PSBMSG_HEAD Header; // C1:11:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensContribution;
};

struct SDHP_GENS_SYSTEM_REWARD_SEND
{
	PSBMSG_HEAD Header; // C1:11:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensRewardStatus;
};

struct SDHP_GENS_SYSTEM_CREATE_SEND
{
	PSBMSG_HEAD Header; // C1:11:70
	DWORD GensFamily;
	DWORD GensRankStart;
	DWORD GensRankFinal;
};

//**********************************************//

class CGensSystem
{
public:
	CGensSystem() = default;
	~CGensSystem() = default;
	void GDGensSystemInsertRecv(const SDHP_GENS_SYSTEM_INSERT_RECV* lpMsg, int serverIndex, int size);
	void GDGensSystemDeleteRecv(const SDHP_GENS_SYSTEM_DELETE_RECV* lpMsg, int serverIndex, int size);
	void GDGensSystemMemberRecv(const SDHP_GENS_SYSTEM_MEMBER_RECV* lpMsg, int serverIndex, int size);
	void GDGensSystemUpdateRecv(const SDHP_GENS_SYSTEM_UPDATE_RECV* lpMsg, int serverIndex, int size);
	void GDGensSystemRewardRecv(const SDHP_GENS_SYSTEM_REWARD_RECV* lpMsg, int serverIndex, int size);
	void GDGensSystemRewardSaveRecv(const SDHP_GENS_SYSTEM_REWARD_SAVE_RECV* lpMsg, int serverIndex, int size);
	void DGGensSystemCreateSend(DWORD GensFamily,DWORD GensRankStart,DWORD GensRankFinal);
};

extern CGensSystem gGensSystem;
