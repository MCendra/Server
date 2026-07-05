// Quest.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_QUEST_KILL_COUNT_RECV
{
	PSBMSG_HEAD Header; // C1:0C:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_QUEST_KILL_COUNT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:0C:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD QuestIndex;
	DWORD MonsterClass[5];
	DWORD KillCount[5];
};

// DataServer -> GameServer

struct SDHP_QUEST_KILL_COUNT_SEND
{
	PSBMSG_HEAD Header; // C1:0C:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD QuestIndex;
	DWORD MonsterClass[5];
	DWORD KillCount[5];
};

//**********************************************//

class CQuest
{
public:
	CQuest() = default;
	~CQuest() = default;
	void GDQuestKillCountRecv(SDHP_QUEST_KILL_COUNT_RECV* lpMsg,int index);
	void GDQuestKillCountSaveRecv(SDHP_QUEST_KILL_COUNT_SAVE_RECV* lpMsg);
};

extern CQuest gQuest;
