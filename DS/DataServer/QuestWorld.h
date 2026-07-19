// QuestWorld.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_QUEST_WORLD_RECV
{
	PSBMSG_HEAD Header; // C1:10:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_QUEST_WORLD_SAVE_RECV
{
	PSWMSG_HEAD Header; // C1:10:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE QuestWorldList[MAX_QUEST_WORLD_LIST][20];
};

// DataServer -> GameServer

struct SDHP_QUEST_WORLD_SEND
{
	PSWMSG_HEAD Header; // C1:10:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE QuestWorldList[MAX_QUEST_WORLD_LIST][20];
};

//**********************************************//

class CQuestWorld
{
public:
	CQuestWorld() = default;
	~CQuestWorld() = default;
	void GDQuestWorldRecv(const SDHP_QUEST_WORLD_RECV* lpMsg, int serverIndex, int size);
	void GDQuestWorldSaveRecv(const SDHP_QUEST_WORLD_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CQuestWorld gQuestWorld;
