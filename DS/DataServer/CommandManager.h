// CommandManager.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

#pragma pack(push,1)
struct SDHP_COMMAND_RESET_RECV
{
	PSBMSG_HEAD Header; // C1:0F:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_COMMAND_MASTER_RESET_RECV
{
	PSBMSG_HEAD Header; // C1:0F:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_COMMAND_MARRY_RECV
{
    PSBMSG_HEAD Header; // C1:0F:02
	WORD Index;
	char CharacterName[MAX_CHARACTER_NAME];
	char MarryName[11];
    char Mode[11];
};

struct SDHP_COMMAND_REWARD_RECV
{
	PSBMSG_HEAD Header; // C1:0F:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char AccountGM[MAX_ACCOUNT_NAME];
	char NameGM[MAX_CHARACTER_NAME];
	UINT Type;
	UINT Value;
};

struct SDHP_COMMAND_REWARDALL_RECV
{
	PSBMSG_HEAD Header; // C1:0F:04
	WORD Index;
	char AccountGM[MAX_ACCOUNT_NAME];
	char NameGM[MAX_CHARACTER_NAME];
	UINT Type;
	UINT Value;
};

struct SDHP_COMMAND_RENAME_RECV
{
    PSBMSG_HEAD Header; // C1:F0:05
    WORD Index;
	char Account[MAX_ACCOUNT_NAME];
    char CharacterName[MAX_CHARACTER_NAME];
    char NewName[MAX_CHARACTER_NAME];
};

struct SDHP_COMMAND_BLOC_RECV
{
    PSBMSG_HEAD Header; // C1:F0:06
    WORD Index;
    char CharacterNameBloc[MAX_CHARACTER_NAME];
	WORD Days;
};

struct SDHP_GIFT_RECV
{
    PSBMSG_HEAD Header; // C1:F0:08
    WORD Index;
	char Account[MAX_ACCOUNT_NAME];
};

struct SDHP_TOP_RECV
{
    PSBMSG_HEAD Header; // C1:F0:09
    WORD Index;
	WORD Type;
	WORD Class;
};

// DataServer -> GameServer

struct SDHP_COMMAND_RESET_SEND
{
	PSBMSG_HEAD Header; // C1:0F:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	UINT ResetDay;
	UINT ResetWek;
	UINT ResetMon;
};

struct SDHP_COMMAND_MASTER_RESET_SEND
{
	PSBMSG_HEAD Header; // C1:0F:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	UINT MasterResetDay;
	UINT MasterResetWek;
	UINT MasterResetMon;
};

struct SDHP_COMMAND_MARRY_SEND
{
    PSBMSG_HEAD Header; // C1:0F:02
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
    char MarryName[MAX_CHARACTER_NAME];
    char Mode[11];
    char NameGet1[11];
    char NameGet2[11];
    UINT CountYou;
    UINT CountHim;
    UINT MarriedOn;
};

struct SDHP_COMMAND_RENAME_SEND
{
    PSBMSG_HEAD Header; // C1:0F:05
    WORD Index;
	char Account[MAX_ACCOUNT_NAME];
    char CharacterName[MAX_CHARACTER_NAME];
    char NewName[MAX_CHARACTER_NAME];
	BYTE Result;
};

struct SDHP_GIFT_SEND
{
    PSBMSG_HEAD Header; // C1:0F:08
    WORD Index;
	BYTE Result;
};

struct SDHP_TOP_SEND
{
	PSWMSG_HEAD Header; // C1:0F:09
	WORD Index;
	WORD Type;
	WORD Class;
	BYTE Count;
};

struct SDHP_COMMAND_BLOC_SEND
{
    PSBMSG_HEAD Header; // C1:0F:06
    WORD Index;
	BYTE Result;
};

//**********************************************//

struct SDHP_TOP_INFO
{
	char CharacterName[MAX_CHARACTER_NAME];
	UINT Value;
};

struct CUSTOM_RANKING_DATA
{
	char Name[20];
	int Value;
};
#pragma pack(pop)

class CCommandManager
{
public:
	CCommandManager() = default;
	~CCommandManager() = default;
	void GDCommandResetRecv(const SDHP_COMMAND_RESET_RECV* lpMsg,int serverIndex, int size);
	void GDCommandMasterResetRecv(const SDHP_COMMAND_MASTER_RESET_RECV* lpMsg, int serverIndex, int size);
	void GDCommandMarryRecv(const SDHP_COMMAND_MARRY_RECV* lpMsg, int serverIndex, int size);
	void GDCommandRewardRecv(const SDHP_COMMAND_REWARD_RECV* lpMsg, int serverIndex, int size);
	void GDCommandRewardAllRecv(const SDHP_COMMAND_REWARDALL_RECV* lpMsg, int serverIndex, int size);
	void GDCommandRenameRecv(const SDHP_COMMAND_RENAME_RECV* lpMsg, int serverIndex, int size);
	void GDCommandBlocAccRecv(const SDHP_COMMAND_BLOC_RECV* lpMsg, int serverIndex, int size);
	void GDCommandBlocCharRecv(const SDHP_COMMAND_BLOC_RECV* lpMsg, int serverIndex, int size);
	void GDCommandGiftRecv(const SDHP_GIFT_RECV* lpMsg, int serverIndex, int size);
	void GDCommandTopRecv(const SDHP_TOP_RECV* lpMsg, int serverIndex, int size);
};

extern CCommandManager gCommandManager;
