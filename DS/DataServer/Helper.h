// Helper.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

#pragma pack(push,1)
struct SDHP_HELPER_DATA_RECV
{
	PSBMSG_HEAD Header; // C1:17:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_HELPER_DATA_SAVE_RECV
{
	PSWMSG_HEAD Header; // C1:17:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Data[256];
};

// DataServer -> GameServer

struct SDHP_HELPER_DATA_SEND
{
	PSWMSG_HEAD Header; // C1:17:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE Data[256];
};
#pragma pack(pop)

class CHelper
{
public:
	CHelper() = default;
	~CHelper() = default;

	void GDHelperDataRecv(const SDHP_HELPER_DATA_RECV* lpMsg, int serverIndex, int size);
	void GDHelperDataSaveRecv(const SDHP_HELPER_DATA_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CHelper gHelper;
