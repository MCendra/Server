// ReiDoMU.cpp
#include "Header.h"
#include "ReiDoMU.h"
#include "Log.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"

CReiDoMU gReiDoMU;

void CReiDoMU::GDRankingKingGuildSaveRecv(const SDHP_RANKING_KING_GUILD_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_KING_GUILD_SAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM RankingKingGuild WHERE Name='%s'",
			lpMsg->GuildName) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE RankingKingGuild "
			"SET Score=Score+%d,Score_semanal=Score_semanal+%d "
			"WHERE Name='%s'",
			lpMsg->Score,
			lpMsg->Score,
			lpMsg->GuildName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO RankingKingGuild "
			"(Name,Score,Score_semanal) "
			"VALUES ('%s',%d,%d)",
			lpMsg->GuildName,
			lpMsg->Score,
			lpMsg->Score);
	}

	gQueryManager.Close();
}

void CReiDoMU::GDRankingKingPlayerSaveRecv(const SDHP_RANKING_KING_PLAYER_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_KING_PLAYER_SAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM RankingKingPlayer WHERE Name='%s'",
			lpMsg->CharacterName) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE RankingKingPlayer "
			"SET Score=Score+%d,Score_semanal=Score_semanal+%d "
			"WHERE Name='%s'",
			lpMsg->Score,
			lpMsg->Score,
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO RankingKingPlayer "
			"(Name,Score,Score_semanal) "
			"VALUES ('%s',%d,%d)",
			lpMsg->CharacterName,
			lpMsg->Score,
			lpMsg->Score);
	}

	gQueryManager.Close();
}