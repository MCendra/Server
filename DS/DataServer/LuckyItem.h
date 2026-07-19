// LuckyItem.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

#pragma pack(push,1)
struct SDHP_LUCKY_ITEM_RECV
{
	PSWMSG_HEAD Header; // C2:22:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Count;
};

struct SDHP_LUCKY_ITEM1
{
	BYTE Slot;
	DWORD Serial;
};

struct SDHP_LUCKY_ITEM_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:22:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Count;
};

struct SDHP_LUCKY_ITEM_SAVE
{
	DWORD Serial;
	DWORD DurabilitySmall;
};

// DataServer -> GameServer

struct SDHP_LUCKY_ITEM_SEND
{
	PSWMSG_HEAD Header; // C2:22:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Count;
};

struct SDHP_LUCKY_ITEM2
{
	BYTE Slot;
	DWORD Serial;
	DWORD DurabilitySmall;
};
#pragma pack(pop)

//**********************************************//

class CLuckyItem
{
public:
	CLuckyItem() = default;
	~CLuckyItem() = default;
	void GDLuckyItemRecv(const SDHP_LUCKY_ITEM_RECV* lpMsg, int serverIndex, int size);
	void GDLuckyItemSaveRecv(const SDHP_LUCKY_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CLuckyItem gLuckyItem;
