// Quest.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

#pragma pack(push,1)
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
#pragma pack(pop)

//**********************************************//

class CQuest
{
public:
	CQuest() = default;
	~CQuest() = default;
	void GDQuestKillCountRecv(const SDHP_QUEST_KILL_COUNT_RECV* lpMsg, int serverIndex, int size);
	void GDQuestKillCountSaveRecv(const SDHP_QUEST_KILL_COUNT_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CQuest gQuest;
