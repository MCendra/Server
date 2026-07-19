// CashShop.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

#pragma pack(push,1)
struct SDHP_CASH_SHOP_POINT_RECV
{
	PSBMSG_HEAD Header; // C1:18:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
};

struct SDHP_CASH_SHOP_ITEM_BUY_RECV
{
	PSBMSG_HEAD Header; // C1:18:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
};

struct SDHP_CASH_SHOP_ITEM_GIF_RECV
{
	PSWMSG_HEAD Header; // C2:18:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	DWORD SaleZone;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
	char GiftName[11];
	char GiftText[200];
};

struct SDHP_CASH_SHOP_ITEM_NUM_RECV
{
	PSBMSG_HEAD Header; // C1:18:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	UINT InventoryPage;
	BYTE InventoryType;
};

struct SDHP_CASH_SHOP_ITEM_USE_RECV
{
	PSBMSG_HEAD Header; // C1:18:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	DWORD BaseItemCode;
	DWORD MainItemCode;
	WORD ItemIndex;
	BYTE ProductType;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_RECV
{
	PSWMSG_HEAD Header; // C2:18:05
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Count;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM1
{
	BYTE Slot;
	DWORD Serial;
};

struct SDHP_CASH_SHOP_RECIEVE_POINT_RECV
{
	PSBMSG_HEAD Header; // C1:18:06
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	DWORD CallbackFunc;
	DWORD CallbackArg1;
	DWORD CallbackArg2;
};

struct SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:18:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char GiftAccount[MAX_ACCOUNT_NAME];
	DWORD AddWCoinC;
	DWORD AddWCoinP;
	DWORD AddGoblinPoint;
	DWORD AddRuud;
};

struct SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:18:31
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char GiftAccount[MAX_ACCOUNT_NAME];
	DWORD SubWCoinC;
	DWORD SubWCoinP;
	DWORD SubGoblinPoint;
	DWORD SubRuud;
};

struct SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:18:32
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char GiftAccount[11];
	BYTE InventoryType;
	DWORD PackageMainIndex;
	DWORD ProductBaseIndex;
	DWORD ProductMainIndex;
	double CoinValue;
	BYTE ProductType;
	char GiftName[11];
	char GiftText[200];
};

struct SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:18:33
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char GiftAccount[11];
	DWORD BaseItemCode;
	DWORD MainItemCode;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:18:34
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Count;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE
{
	DWORD Serial;
	DWORD Time;
};

// DataServer -> GameServer

struct SDHP_CASH_SHOP_POINT_SEND
{
	PSBMSG_HEAD Header; // C1:18:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
	DWORD Ruud;
};

struct SDHP_CASH_SHOP_ITEM_BUY_SEND
{
	PSBMSG_HEAD Header; // C1:18:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
	DWORD ItemCount;
};

struct SDHP_CASH_SHOP_ITEM_GIF_SEND
{
	PSWMSG_HEAD Header; // C2:18:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	DWORD SaleZone;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
	char GiftName[11];
	char GiftText[200];
	char GiftAccount[11];
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
	DWORD ItemCount;
};

struct SDHP_CASH_SHOP_ITEM_NUM_SEND
{
	PSWMSG_HEAD Header; // C2:18:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
	UINT InventoryPage;
	BYTE InventoryType;
	DWORD ItemCount;
	DWORD PageCount;
	struct
	{
		DWORD BaseItemCode;
		DWORD MainItemCode;
		DWORD PackageMainIndex;
		DWORD ProductBaseIndex;
		DWORD ProductMainIndex;
		double CoinValue;
		BYTE ProductType;
		char GiftName[11];
		char GiftText[200];
	}ProductInfo[MAX_CASH_SHOP_PAGE_ITEM];
};

struct SDHP_CASH_SHOP_ITEM_USE_SEND
{
	PSWMSG_HEAD Header; // C2:18:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
	DWORD BaseItemCode;
	DWORD MainItemCode;
	WORD ItemIndex;
	BYTE ProductType;
	struct
	{
		DWORD BaseItemCode;
		DWORD MainItemCode;
		DWORD PackageMainIndex;
		DWORD ProductBaseIndex;
		DWORD ProductMainIndex;
		double CoinValue;
		BYTE ProductType;
		char GiftName[11];
		char GiftText[200];
	}ProductInfo;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_SEND
{
	PSWMSG_HEAD Header; // C2:18:05
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Count;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM2
{
	BYTE Slot;
	DWORD Serial;
	DWORD Time;
};

struct SDHP_CASH_SHOP_RECIEVE_POINT_SEND
{
	PSBMSG_HEAD Header; // C1:18:06
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	DWORD CallbackFunc;
	DWORD CallbackArg1;
	DWORD CallbackArg2;
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
};
struct BANKEX_GD_SAVE_POINT
{
	PSBMSG_HEAD	Header;
	WORD	UserIndex;
	char	Account[MAX_ACCOUNT_NAME];
	long	Money;
	long	Bless;
	long	Soul;
	long	Chaos;
	long	Creation;
	long	Life;
	long	Harmony;
};

struct BANKEX_GD_REQ_POINT
{
	PSBMSG_HEAD	Header;
	WORD	UserIndex;
	char	Account[MAX_ACCOUNT_NAME];
};

struct BANKEX_DG_GET_POINT
{
	PSBMSG_HEAD	Header;
	WORD	UserIndex;
	long	Money;
	long	Bless;
	long	Soul;
	long	Chaos;
	long	Creation;
	long	Life;
	long	Harmony;
};
#pragma pack(pop)

//**********************************************//

class CCashShop
{
public:
	CCashShop() = default;
	~CCashShop() = default;
	void GDCashShopPointRecv(const SDHP_CASH_SHOP_POINT_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopItemBuyRecv(const SDHP_CASH_SHOP_ITEM_BUY_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopItemGifRecv(const SDHP_CASH_SHOP_ITEM_GIF_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopItemNumRecv(const SDHP_CASH_SHOP_ITEM_NUM_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopItemUseRecv(const SDHP_CASH_SHOP_ITEM_USE_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopPeriodicItemRecv(const SDHP_CASH_SHOP_PERIODIC_ITEM_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopRecievePointRecv(const SDHP_CASH_SHOP_RECIEVE_POINT_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopAddPointSaveRecv(const SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopSubPointSaveRecv(const SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopInsertItemSaveRecv(const SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopDeleteItemSaveRecv(const SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDCashShopPeriodicItemSaveRecv(const SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size);

private:
	// Fix:
	bool LoadCashInfo(const char* account, DWORD& WCoinC, DWORD& WCoinP, DWORD& GoblinPoint);
	bool LoadCashInfo(const char* account, DWORD& WCoinC, DWORD& WCoinP, DWORD& GoblinPoint, DWORD& Ruud);
	bool CreateCashInfo(const char* account);
};

extern CCashShop gCashShop;
