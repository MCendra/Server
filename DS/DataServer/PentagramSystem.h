// PentagramSystem.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_PENTAGRAM_JEWEL_INFO_RECV
{
	PSBMSG_HEAD Header; // C1:23:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Type;
};

struct SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV
{
	PSWMSG_HEAD Header; // C2:23:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Count;
};

struct SDHP_PENTAGRAM_JEWEL_INFO_SAVE
{
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

struct SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:23:31
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Type;
	BYTE JewelIndex;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

struct SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_RECV
{
	PSBMSG_HEAD Header; // C1:23:32
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Type;
	BYTE JewelIndex;
};

//********** DataServer -> GameServer **********//

struct SDHP_PENTAGRAM_JEWEL_INFO_SEND
{
	PSWMSG_HEAD Header; // C2:23:00
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Type;
	BYTE Count;
};

struct SDHP_PENTAGRAM_JEWEL_INFO
{
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

//**********************************************//

class CPentagramSystem
{
public:
	CPentagramSystem() = default;
	~CPentagramSystem() = default;
	void GDPentagramJewelInfoRecv(const SDHP_PENTAGRAM_JEWEL_INFO_RECV* lpMsg, int serverIndex, int size);
	void GDPentagramJewelInfoSaveRecv(const SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDPentagramJewelInsertSaveRecv(const SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_RECV* lpMsg, int serverIndex, int size);
	void GDPentagramJewelDeleteSaveRecv(const SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_RECV* lpMsg, int serverIndex, int size);
};

extern CPentagramSystem gPentagramSystem;
