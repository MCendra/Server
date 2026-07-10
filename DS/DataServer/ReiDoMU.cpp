// ReiDoMU.cpp
#include "Header.h"
#include "ReiDoMU.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"

CReiDoMU gReiDoMU;

void CReiDoMU::GDRankingKingGuildSaveRecv(SDHP_RANKING_KING_GUILD_SAVE_RECV* lpMsg)
{
	if (!gQueryManager.ExecQuery("SELECT Name FROM RankingKingGuild WHERE Name='%s'", lpMsg->GuildName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO RankingKingGuild (Name,Score,Score_semanal) VALUES ('%s',%d,%d)", lpMsg->GuildName, lpMsg->Score, lpMsg->Score);

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE RankingKingGuild SET Score=Score+%d,Score_semanal=Score_semanal+%d WHERE Name='%s'", lpMsg->Score, lpMsg->Score, lpMsg->GuildName);

		gQueryManager.Close();
	}
}

void CReiDoMU::GDRankingKingPlayerSaveRecv(SDHP_RANKING_KING_PLAYER_SAVE_RECV* lpMsg)
{
	if (!gQueryManager.ExecQuery("SELECT Name FROM RankingKingPlayer WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO RankingKingPlayer (Name,Score,Score_semanal) VALUES ('%s',%d,%d)", lpMsg->CharacterName, lpMsg->Score, lpMsg->Score);

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE RankingKingPlayer SET Score=Score+%d,Score_semanal=Score_semanal+%d WHERE Name='%s'", lpMsg->Score, lpMsg->Score, lpMsg->CharacterName);

		gQueryManager.Close();
	}
}

