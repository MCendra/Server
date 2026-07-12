// NpcTalk.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_NPC_LEO_THE_HELPER_RECV
{
	PSBMSG_HEAD Header; // C1:0E:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_NPC_SANTA_CLAUS_RECV
{
	PSBMSG_HEAD Header; // C1:0E:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_NPC_LEO_THE_HELPER_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:0E:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Status;
};

struct SDHP_NPC_SANTA_CLAUS_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:0E:31
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Status;
};

// DataServer -> GameServer

struct SDHP_NPC_LEO_THE_HELPER_SEND
{
	PSBMSG_HEAD Header; // C1:0E:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Status;
};

struct SDHP_NPC_SANTA_CLAUS_SEND
{
	PSBMSG_HEAD Header; // C1:0E:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Status;
};

//**********************************************//

class CNpcTalk
{
public:
	CNpcTalk() = default;
	~CNpcTalk() = default;
	void GDNpcLeoTheHelperRecv(const SDHP_NPC_LEO_THE_HELPER_RECV* lpMsg, int serverIndex, int size);
	void GDNpcSantaClausRecv(const SDHP_NPC_SANTA_CLAUS_RECV* lpMsg, int serverIndex, int size);
	void GDNpcLeoTheHelperSaveRecv(const SDHP_NPC_LEO_THE_HELPER_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDNpcSantaClausSaveRecv(const SDHP_NPC_SANTA_CLAUS_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CNpcTalk gNpcTalk;
