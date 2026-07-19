// GensSystem.cpp
#include "Header.h"
#include "GensSystem.h"
#include "Log.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"

CGensSystem gGensSystem;

void CGensSystem::GDGensSystemInsertRecv(
	const SDHP_GENS_SYSTEM_INSERT_RECV* lpMsg,
	int serverIndex,
	int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_GENS_SYSTEM_INSERT_RECV);

	SDHP_GENS_SYSTEM_INSERT_SEND pMsg{};

	pMsg.Header.set(HEAD_GENS_SYSTEM, SUB_GENS_SYSTEM_INSERT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"INSERT INTO Gens_Rank (Name,Family,Contribution) VALUES ('%s',%d,0)",
		lpMsg->CharacterName,
		lpMsg->GensFamily))
	{
		pMsg.GensFamily = lpMsg->GensFamily;

		gQueryManager.Close();

		if (gQueryManager.ExecQuery(
			"EXEC WZ_GetCharacterGensInfo '%s',%d",
			lpMsg->CharacterName,
			lpMsg->GensFamily))
		{
			const auto sqlRet = gQueryManager.Fetch();

			if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
			{
				pMsg.GensRank = gQueryManager.GetAsInteger("Rank");
				pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");
			}
			else
			{
				pMsg.Result = 1;
			}
		}
		else
		{
			pMsg.Result = 1;
		}
	}
	else
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CGensSystem::GDGensSystemDeleteRecv(const SDHP_GENS_SYSTEM_DELETE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_GENS_SYSTEM_DELETE_RECV);

	SDHP_GENS_SYSTEM_DELETE_SEND pMsg{};

	pMsg.Header.set(HEAD_GENS_SYSTEM, SUB_GENS_SYSTEM_DELETE, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (!gQueryManager.ExecQuery(
		"DELETE FROM Gens_Rank WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

	DGGensSystemCreateSend(
		lpMsg->GensFamily,
		lpMsg->GensRank + 1,
		0xFFFFFFFF);

#endif
}

void CGensSystem::GDGensSystemMemberRecv(const SDHP_GENS_SYSTEM_MEMBER_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_GENS_SYSTEM_MEMBER_RECV);

	SDHP_GENS_SYSTEM_MEMBER_SEND pMsg{};

	pMsg.Header.set(HEAD_GENS_SYSTEM, SUB_GENS_SYSTEM_MEMBER, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"SELECT Family FROM Gens_Rank WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.GensFamily = static_cast<BYTE>(
				gQueryManager.GetAsInteger("Family"));

			gQueryManager.Close();

			if (gQueryManager.ExecQuery(
				"EXEC WZ_GetCharacterGensInfo '%s',%d",
				lpMsg->CharacterName,
				pMsg.GensFamily))
			{
				const auto sqlRet = gQueryManager.Fetch();

				if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
				{
					pMsg.GensRank = gQueryManager.GetAsInteger("Rank");
					pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");
				}
				else
				{
					pMsg.Result = 1;
				}
			}
			else
			{
				pMsg.Result = 1;
			}
		}
		else
		{
			pMsg.Result = 1;
		}
	}
	else
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CGensSystem::GDGensSystemUpdateRecv(const SDHP_GENS_SYSTEM_UPDATE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_GENS_SYSTEM_UPDATE_RECV);

	SDHP_GENS_SYSTEM_UPDATE_SEND pMsg{};

	pMsg.Header.set(HEAD_GENS_SYSTEM, SUB_GENS_SYSTEM_UPDATE, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (!gQueryManager.ExecQuery(
		"UPDATE Gens_Rank SET Contribution=%d WHERE Name='%s'",
		lpMsg->GensContribution,
		lpMsg->CharacterName))
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

		return;
	}

	pMsg.GensFamily = lpMsg->GensFamily;

	gQueryManager.Close();

	if (gQueryManager.ExecQuery(
		"EXEC WZ_GetCharacterGensInfo '%s',%d",
		lpMsg->CharacterName,
		lpMsg->GensFamily))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.GensRank = gQueryManager.GetAsInteger("Rank");
			pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");
		}
		else
		{
			pMsg.Result = 1;
		}
	}
	else
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	if (pMsg.Result != 0 || pMsg.GensRank == lpMsg->GensRank)
	{
		gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

		return;
	}

	if (pMsg.GensRank < lpMsg->GensRank)
	{
		DGGensSystemCreateSend(
			lpMsg->GensFamily,
			pMsg.GensRank,
			lpMsg->GensRank);

		return;
	}

	DGGensSystemCreateSend(
		lpMsg->GensFamily,
		lpMsg->GensRank,
		pMsg.GensRank);

#endif
}

void CGensSystem::GDGensSystemRewardRecv(const SDHP_GENS_SYSTEM_REWARD_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_GENS_SYSTEM_REWARD_RECV);

	SDHP_GENS_SYSTEM_REWARD_SEND pMsg{};

	pMsg.Header.set(HEAD_GENS_SYSTEM, SUB_GENS_SYSTEM_REWARD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"SELECT Rank,Status FROM Gens_Reward WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.GensFamily = lpMsg->GensFamily;
			pMsg.GensRank = gQueryManager.GetAsInteger("Rank");
			pMsg.GensRewardStatus = gQueryManager.GetAsInteger("Status");
		}
		else
		{
			pMsg.Result = 1;
		}
	}
	else
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CGensSystem::GDGensSystemRewardSaveRecv(const SDHP_GENS_SYSTEM_REWARD_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_GENS_SYSTEM_REWARD_SAVE_RECV);

	if (!gQueryManager.ExecQuery(
		"UPDATE Gens_Reward SET Status=%d WHERE Name='%s'",
		lpMsg->GensRewardStatus,
		lpMsg->CharacterName))
	{
		gQueryManager.Close();
		return;
	}

	const SQLLEN affectedRows = gQueryManager.GetAffectedRows();

	gQueryManager.Close();

	if (affectedRows == 0)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO Gens_Reward (Name,Rank,Status) VALUES ('%s',%d,%d)",
			lpMsg->CharacterName,
			lpMsg->GensRank,
			lpMsg->GensRewardStatus);

		gQueryManager.Close();
	}

#endif
}

void CGensSystem::DGGensSystemCreateSend(DWORD GensFamily, DWORD GensRankStart, DWORD GensRankFinal)
{
#if (DATASERVER_UPDATE >= 501)

	SDHP_GENS_SYSTEM_CREATE_SEND pMsg{};

	pMsg.Header.set(HEAD_GENS_SYSTEM, SUB_GENS_SYSTEM_CREATE, sizeof(pMsg));
	pMsg.GensFamily = GensFamily;
	pMsg.GensRankStart = GensRankStart;
	pMsg.GensRankFinal = GensRankFinal;

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (!gServerManager[n].IsOnline())
		{
			continue;
		}

		gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
	}

#endif
}
