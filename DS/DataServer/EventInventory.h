// EventInventory.h
#pragma once
#include "DataServerProtocol.h"

#define EVENT_INVENTORY_SIZE 32

// GameServer -> DataServer

struct SDHP_EVENT_INVENTORY_RECV
{
	PSBMSG_HEAD Header; // C1:26:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_EVENT_INVENTORY_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:26:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE EventInventory[EVENT_INVENTORY_SIZE][16];
};

// DataServer -> GameServer

struct SDHP_EVENT_INVENTORY_SEND
{
	PSWMSG_HEAD Header; // C2:26:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE EventInventory[EVENT_INVENTORY_SIZE][16];
};

//**********************************************//

class CEventInventory
{
public:
	CEventInventory() = default;
	~CEventInventory() = default;
	void GDEventInventoryRecv(SDHP_EVENT_INVENTORY_RECV* lpMsg,int index);
	void GDEventInventorySaveRecv(SDHP_EVENT_INVENTORY_SAVE_RECV* lpMsg);
};

extern CEventInventory gEventInventory;
