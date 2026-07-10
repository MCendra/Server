// PersonalShop.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_PSHOP_ITEM_VALUE_RECV
{
	PSBMSG_HEAD Header; // C1:25:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_PSHOP_ITEM_VALUE_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:25:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Count;
};

struct SDHP_PSHOP_ITEM_VALUE_SAVE
{
	DWORD Slot;
	DWORD Serial;
	DWORD Value;
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
};

struct SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:25:31
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Slot;
	DWORD Serial;
	DWORD Value;
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
};

struct SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:25:32
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Slot;
};

// DataServer -> GameServer

struct SDHP_PSHOP_ITEM_VALUE_SEND
{
	PSWMSG_HEAD Header; // C2:25:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Count;
};

struct SDHP_PSHOP_ITEM_VALUE
{
	DWORD Slot;
	DWORD Serial;
	DWORD Value;
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
};

//**********************************************//

class CPersonalShop
{
public:
	CPersonalShop() = default;
	~CPersonalShop() = default;
	void GDPShopItemValueRecv(SDHP_PSHOP_ITEM_VALUE_RECV* lpMsg,int index);
	void GDPShopItemValueSaveRecv(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV* lpMsg);
	void GDPShopItemValueInsertSaveRecv(SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV* lpMsg);
	void GDPShopItemValueDeleteSaveRecv(SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV* lpMsg);
};

extern CPersonalShop gPersonalShop;
