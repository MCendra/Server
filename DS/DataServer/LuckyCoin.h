// LuckyCoin.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

#pragma pack(push,1)
struct SDHP_LUCKY_COIN_COUNT_RECV
{
	PSBMSG_HEAD Header; // C1:1A:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
};

struct SDHP_LUCKY_COIN_REGISTER_RECV
{
	PSBMSG_HEAD Header; // C1:1A:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Slot;
};

struct SDHP_LUCKY_COIN_EXCHANGE_RECV
{
	PSBMSG_HEAD Header; // C1:1A:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	DWORD TradeCoin;
};

struct SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:1A:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	DWORD AddLuckyCoin;
};

struct SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:1A:31
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	DWORD SubLuckyCoin;
};

//********** DataServer -> GameServer **********//

struct SDHP_LUCKY_COIN_COUNT_SEND
{
	PSBMSG_HEAD Header; // C1:1A:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
	DWORD LuckyCoin;
};

struct SDHP_LUCKY_COIN_REGISTER_SEND
{
	PSBMSG_HEAD Header; // C1:1A:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
	BYTE Slot;
	DWORD LuckyCoin;
};

struct SDHP_LUCKY_COIN_EXCHANGE_SEND
{
	PSBMSG_HEAD Header; // C1:1A:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
	DWORD TradeCoin;
	DWORD LuckyCoin;
};
#pragma pack(pop)

//**********************************************//

class CLuckyCoin
{
public:
	CLuckyCoin() = default;
	~CLuckyCoin() = default;
	void GDLuckyCoinCountRecv(const SDHP_LUCKY_COIN_COUNT_RECV* lpMsg,int serverIndex, int size);
	void GDLuckyCoinRegisterRecv(const SDHP_LUCKY_COIN_REGISTER_RECV* lpMsg, int serverIndex, int size);
	void GDLuckyCoinExchangeRecv(const SDHP_LUCKY_COIN_EXCHANGE_RECV* lpMsg, int serverIndex, int size);
	void GDLuckyCoinAddCountSaveRecv(const SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDLuckyCoinSubCountSaveRecv(const SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CLuckyCoin gLuckyCoin;
