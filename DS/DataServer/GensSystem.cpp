// GensSystem.cpp
#include "Header.h"
#include "GensSystem.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"

CGensSystem gGensSystem;

void CGensSystem::GDGensSystemInsertRecv(SDHP_GENS_SYSTEM_INSERT_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 501)

	SDHP_GENS_SYSTEM_INSERT_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_GENS_SYSTEM, DS_SUB_GENS_SYSTEM_INSERT, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.GensFamily = 0;
	pMsg.GensRank = 0;
	pMsg.GensContribution = 0;

	if (!gQueryManager.ExecQuery(
		"INSERT INTO Gens_Rank (Name,Family,Contribution) VALUES ('%s',%d,0)",
		lpMsg->CharacterName,
		lpMsg->GensFamily))
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		return;
	}

	pMsg.GensFamily = lpMsg->GensFamily;

	gQueryManager.Close();

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_GetCharacterGensInfo '%s',%d",
		lpMsg->CharacterName,
		lpMsg->GensFamily) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		return;
	}

	pMsg.GensRank = gQueryManager.GetAsInteger("Rank");
	pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");

	gQueryManager.Close();

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

#endif
}

void CGensSystem::GDGensSystemDeleteRecv(SDHP_GENS_SYSTEM_DELETE_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 501)

	SDHP_GENS_SYSTEM_DELETE_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_GENS_SYSTEM, DS_SUB_GENS_SYSTEM_DELETE, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	if (!gQueryManager.ExecQuery(
		"DELETE FROM Gens_Rank WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

	DGGensSystemCreateSend(
		lpMsg->GensFamily,
		lpMsg->GensRank + 1,
		0xFFFFFFFF);

#endif
}

void CGensSystem::GDGensSystemMemberRecv(SDHP_GENS_SYSTEM_MEMBER_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 501)

	SDHP_GENS_SYSTEM_MEMBER_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_GENS_SYSTEM, DS_SUB_GENS_SYSTEM_MEMBER, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.GensFamily = 0;
	pMsg.GensRank = 0;
	pMsg.GensContribution = 0;

	if (!gQueryManager.ExecQuery(
		"SELECT Family FROM Gens_Rank WHERE Name='%s'",
		lpMsg->CharacterName) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		return;
	}

	pMsg.GensFamily = static_cast<BYTE>(gQueryManager.GetAsInteger("Family"));

	gQueryManager.Close();

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_GetCharacterGensInfo '%s',%d",
		lpMsg->CharacterName,
		pMsg.GensFamily) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		return;
	}

	pMsg.GensRank = gQueryManager.GetAsInteger("Rank");
	pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");

	gQueryManager.Close();

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

#endif
}

void CGensSystem::GDGensSystemUpdateRecv(SDHP_GENS_SYSTEM_UPDATE_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 501)

	SDHP_GENS_SYSTEM_UPDATE_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_GENS_SYSTEM, DS_SUB_GENS_SYSTEM_UPDATE, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.GensFamily = 0;
	pMsg.GensRank = 0;
	pMsg.GensContribution = 0;

	if (!gQueryManager.ExecQuery(
		"UPDATE Gens_Rank SET Contribution=%d WHERE Name='%s'",
		lpMsg->GensContribution,
		lpMsg->CharacterName))
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		return;
	}

	pMsg.GensFamily = lpMsg->GensFamily;

	gQueryManager.Close();

	if (!gQueryManager.ExecQuery(
		"EXEC WZ_GetCharacterGensInfo '%s',%d",
		lpMsg->CharacterName,
		lpMsg->GensFamily) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		return;
	}

	pMsg.GensRank = gQueryManager.GetAsInteger("Rank");
	pMsg.GensContribution = gQueryManager.GetAsInteger("Contribution");

	gQueryManager.Close();

	if (pMsg.GensRank == lpMsg->GensRank)
	{
		gSocketManager.DataSend(
			index,
			reinterpret_cast<BYTE*>(&pMsg),
			pMsg.Header.size);

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

void CGensSystem::GDGensSystemRewardRecv(SDHP_GENS_SYSTEM_REWARD_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 501)

	SDHP_GENS_SYSTEM_REWARD_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_GENS_SYSTEM, DS_SUB_GENS_SYSTEM_REWARD, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.GensFamily = 0;
	pMsg.GensRank = 0;
	pMsg.GensRewardStatus = 0;

	if (!gQueryManager.ExecQuery(
		"SELECT Rank,Status FROM Gens_Reward WHERE Name='%s'",
		lpMsg->CharacterName) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(
			index,
			reinterpret_cast<BYTE*>(&pMsg),
			pMsg.Header.size);

		return;
	}

	pMsg.GensFamily = lpMsg->GensFamily;
	pMsg.GensRank = gQueryManager.GetAsInteger("Rank");
	pMsg.GensRewardStatus = gQueryManager.GetAsInteger("Status");

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);

#endif
}

void CGensSystem::GDGensSystemRewardSaveRecv(SDHP_GENS_SYSTEM_REWARD_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 501)

	if (!gQueryManager.ExecQuery(
		"SELECT Name FROM Gens_Reward WHERE Name='%s'",
		lpMsg->CharacterName) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO Gens_Reward (Name,Rank,Status) VALUES ('%s',%d,%d)",
			lpMsg->CharacterName,
			lpMsg->GensRank,
			lpMsg->GensRewardStatus);

		gQueryManager.Close();

		return;
	}

	gQueryManager.Close();

	gQueryManager.ExecQuery(
		"UPDATE Gens_Reward SET Status=%d WHERE Name='%s'",
		lpMsg->GensRewardStatus,
		lpMsg->CharacterName);

	gQueryManager.Close();

#endif
}

void CGensSystem::DGGensSystemCreateSend(
	DWORD GensFamily,
	DWORD GensRankStart,
	DWORD GensRankFinal)
{
#if (DATASERVER_UPDATE >= 501)

	SDHP_GENS_SYSTEM_CREATE_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_GENS_SYSTEM, DS_SUB_GENS_SYSTEM_CREATE, sizeof(pMsg));

	pMsg.GensFamily = GensFamily;
	pMsg.GensRankStart = GensRankStart;
	pMsg.GensRankFinal = GensRankFinal;

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (!gServerManager[n].IsOnline())
		{
			continue;
		}

		gSocketManager.DataSend(
			n,
			reinterpret_cast<BYTE*>(&pMsg),
			pMsg.Header.size);
	}

#endif
}
