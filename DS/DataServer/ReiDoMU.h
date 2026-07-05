// ReiDoMU.h
#pragma once
#include "DataServerProtocol.h"

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct PMSG_OWNERGUILD_SEND
{
	PBMSG_HEAD header; // C1:74
	char GuildName[9];
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//
struct SDHP_RANKING_KING_GUILD_SAVE_RECV
{
	PBMSG_HEAD header; // C1:74
	char name[9];
	DWORD score;
};

struct SDHP_RANKING_KING_PLAYER_SAVE_RECV
{
	PBMSG_HEAD header; // C1:75
	WORD index;
	char account[11];
	char charactername[MAX_CHARACTER_NAME];
	DWORD score;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CReiDoMU
{
public:
	CReiDoMU();
	virtual ~CReiDoMU();
	void GDRankingKingGuildSaveRecv(SDHP_RANKING_KING_GUILD_SAVE_RECV* lpMsg);
	void GDRankingKingPlayerSaveRecv(SDHP_RANKING_KING_PLAYER_SAVE_RECV* lpMsg);
};

extern CReiDoMU gReiDoMU;
