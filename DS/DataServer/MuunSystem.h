// MuunSystem.h
#pragma once
#include "DataServerProtocol.h"

#define MUUN_INVENTORY_SIZE 62

// GameServer -> DataServer

struct SDHP_MUUN_INVENTORY_RECV
{
	PSBMSG_HEAD Header; // C1:27:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_MUUN_INVENTORY_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:27:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE MuunInventory[MUUN_INVENTORY_SIZE][16];
};

// DataServer -> GameServer

struct SDHP_MUUN_INVENTORY_SEND
{
	PSWMSG_HEAD Header; // C2:27:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE MuunInventory[MUUN_INVENTORY_SIZE][16];
};

//**********************************************//

class CMuunSystem
{
public:
	CMuunSystem() = default;
	~CMuunSystem() = default;
	void GDMuunInventoryRecv(SDHP_MUUN_INVENTORY_RECV* lpMsg,int index);
	void GDMuunInventorySaveRecv(SDHP_MUUN_INVENTORY_SAVE_RECV* lpMsg);
};

extern CMuunSystem gMuunSystem;
