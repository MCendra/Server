// ReiDoMU.h
#pragma once
#include "DataServerProtocol.h"

// DataServer -> GameServer

struct PMSG_OWNERGUILD_SEND
{
	PBMSG_HEAD Header; // C1:74
	char GuildName[MAX_GUILD_NAME];
};

// GameServer -> DataServer

struct SDHP_RANKING_KING_GUILD_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:74
	char GuildName[MAX_GUILD_NAME];
	DWORD Score;
};

struct SDHP_RANKING_KING_PLAYER_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:75
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Score;
};

//**********************************************//

class CReiDoMU
{
public:
	CReiDoMU() = default;
	~CReiDoMU() = default;
	void GDRankingKingGuildSaveRecv(SDHP_RANKING_KING_GUILD_SAVE_RECV* lpMsg);
	void GDRankingKingPlayerSaveRecv(SDHP_RANKING_KING_PLAYER_SAVE_RECV* lpMsg);
};

extern CReiDoMU gReiDoMU;
