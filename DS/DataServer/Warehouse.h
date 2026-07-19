// Warehouse.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

#pragma pack(push,1)
struct SDHP_WAREHOUSE_ITEM_RECV
{
	PSBMSG_HEAD Header; // C1:05:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	UINT WarehouseNumber;
};

struct SDHP_WAREHOUSE_ITEM_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:05:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE WarehouseItem[WAREHOUSE_SIZE][MAX_ITEM_BYTE];
	UINT WarehouseMoney;
	UINT WarehousePassword;
	UINT WarehouseNumber;
};

// DataServer -> GameServer

struct SDHP_WAREHOUSE_ITEM_SEND
{
	PSWMSG_HEAD Header; // C2:05:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE WarehouseItem[WAREHOUSE_SIZE][MAX_ITEM_BYTE];
	UINT WarehouseMoney;
	WORD WarehousePassword;
};

struct SDHP_WAREHOUSE_FREE_SEND
{
	PSBMSG_HEAD Header; // C1:05:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
};

struct SDHP_WAREHOUSE_GUILD_STATUS_RECV
{
	PSBMSG_HEAD Header; // C1:05:00
	WORD Index;
	char GuildName[MAX_GUILD_NAME];
};

struct SDHP_WAREHOUSE_GUILD_STATUS_SEND
{
	PSBMSG_HEAD Header; // C1:05:00
	WORD Index;
	int Status;
};

struct SDHP_WAREHOUSE_GUILD_STATUS_UPDATE_RECV
{
	PSBMSG_HEAD Header; // C1:05:00
	WORD Index;
	char GuildName[MAX_GUILD_NAME];
	int Status;
};
#pragma pack(pop)

class CWarehouse
{
public:
	CWarehouse() = default;
	~CWarehouse() = default;
	void GDWarehouseItemRecv(const SDHP_WAREHOUSE_ITEM_RECV* lpMsg, int serverIndex, int size);
	void GDWarehouseItemSaveRecv(const SDHP_WAREHOUSE_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDWarehouseGuildItemRecv(const SDHP_WAREHOUSE_ITEM_RECV* lpMsg, int serverIndex, int size);
	void GDWarehouseGuildUpdate(const SDHP_WAREHOUSE_GUILD_STATUS_UPDATE_RECV* lpMsg, int serverIndex, int size);
	void GDWarehouseGuildItemSaveRecv(const SDHP_WAREHOUSE_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDWarehouseGuildConsult(const SDHP_WAREHOUSE_GUILD_STATUS_RECV * lpMsg, int serverIndex, int size);
	void DGWarehouseFreeSend(WORD index, int serverIndex, const char* account);
	void DGWarehouseGuildFreeSend(WORD index, int serverIndex, const char* account);
};

extern CWarehouse gWarehouse;
