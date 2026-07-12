// MasterSkillTree.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_MASTER_SKILL_TREE_RECV
{
	PSBMSG_HEAD Header; // C1:0D:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_MASTER_SKILL_TREE_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:0D:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD MasterLevel;
	DWORD MasterPoint;
	QWORD MasterExperience;
	#if(DATASERVER_UPDATE>=602)
	BYTE MasterSkill[MAX_MASTER_SKILL_LIST][MASTER_SKILL_DATA_SIZE];
	#endif
};

// DataServer -> GameServer

struct SDHP_MASTER_SKILL_TREE_SEND
{
	PSWMSG_HEAD Header; // C2:0D:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD MasterLevel;
	DWORD MasterPoint;
	QWORD MasterExperience;
	#if(DATASERVER_UPDATE>=602)
	BYTE MasterSkill[MAX_MASTER_SKILL_LIST][MASTER_SKILL_DATA_SIZE];
	#endif
};

//**********************************************//

class CMasterSkillTree
{
public:
	CMasterSkillTree() = default;
	~CMasterSkillTree() = default;
	void GDMasterSkillTreeRecv(const SDHP_MASTER_SKILL_TREE_RECV* lpMsg, int serverIndex, int size);
	void GDMasterSkillTreeSaveRecv(const SDHP_MASTER_SKILL_TREE_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CMasterSkillTree gMasterSkillTree;
