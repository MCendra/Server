// MuunSystem.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

#pragma pack(push,1)
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
#pragma pack(pop)

//**********************************************//

class CMuunSystem
{
public:
	CMuunSystem() = default;
	~CMuunSystem() = default;
	void GDMuunInventoryRecv(const SDHP_MUUN_INVENTORY_RECV* lpMsg, int serverIndex, int size);
	void GDMuunInventorySaveRecv(const SDHP_MUUN_INVENTORY_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CMuunSystem gMuunSystem;
