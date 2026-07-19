// CommandManager.cpp
#include "Header.h"
#include "CommandManager.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "Util.h"

CCommandManager gCommandManager;

void CCommandManager::GDCommandResetRecv(const SDHP_COMMAND_RESET_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_COMMAND_RESET_RECV);

	SDHP_COMMAND_RESET_SEND pMsg{};
	pMsg.Header.set(HEAD_COMMAND, SUB_COMMAND_RESET, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	const bool dataFound =
		gQueryManager.ExecQuery(
			"EXEC WZ_GetResetInfo '%s','%s'",
			lpMsg->Account,
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.ResetDay = gQueryManager.GetAsInteger("ResetDay");
		pMsg.ResetWek = gQueryManager.GetAsInteger("ResetWek");
		pMsg.ResetMon = gQueryManager.GetAsInteger("ResetMon");
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCommandManager::GDCommandMasterResetRecv(const SDHP_COMMAND_MASTER_RESET_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_COMMAND_MASTER_RESET_RECV);

	SDHP_COMMAND_MASTER_RESET_SEND pMsg{};
	pMsg.Header.set(HEAD_COMMAND, SUB_COMMAND_MASTER_RESET, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	const bool dataFound =
		gQueryManager.ExecQuery(
			"EXEC WZ_GetMasterResetInfo '%s','%s'",
			lpMsg->Account,
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.MasterResetDay = gQueryManager.GetAsInteger("MasterResetDay");
		pMsg.MasterResetWek = gQueryManager.GetAsInteger("MasterResetWek");
		pMsg.MasterResetMon = gQueryManager.GetAsInteger("MasterResetMon");
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

// Marry System
void CCommandManager::GDCommandMarryRecv(const SDHP_COMMAND_MARRY_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_COMMAND_MARRY_RECV);

	SDHP_COMMAND_MARRY_SEND pMsg{};
	pMsg.Header.set(HEAD_COMMAND, SUB_COMMAND_MARRY, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.MarryName, lpMsg->MarryName, sizeof(pMsg.MarryName));
	std::memcpy(pMsg.Mode, lpMsg->Mode, sizeof(pMsg.Mode));

	const bool dataFound =
		gQueryManager.ExecQuery(
			"EXEC WZ_GetMarryInfo '%s','%s'",
			lpMsg->CharacterName,
			lpMsg->MarryName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.CountYou = gQueryManager.GetAsInteger("CountYou");
		pMsg.CountHim = gQueryManager.GetAsInteger("CountHim");
		pMsg.MarriedOn = gQueryManager.GetAsInteger("MarriedOn");

		gQueryManager.GetAsString("NameGet1", pMsg.NameGet1, sizeof(pMsg.NameGet1));
		gQueryManager.GetAsString("NameGet2", pMsg.NameGet2, sizeof(pMsg.NameGet2));
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCommandManager::GDCommandRewardRecv(const SDHP_COMMAND_REWARD_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_COMMAND_REWARD_RECV);

	gQueryManager.ExecQuery(
		"EXEC WZ_SetReward '%s','%s','%s','%s',%d,%d",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->AccountGM,
		lpMsg->NameGM,
		lpMsg->Type,
		lpMsg->Value);

	gQueryManager.Close();
}

void CCommandManager::GDCommandRewardAllRecv(const SDHP_COMMAND_REWARDALL_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_COMMAND_REWARDALL_RECV);

	gQueryManager.ExecQuery(
		"EXEC WZ_SetRewardAll '%s','%s',%d,%d",
		lpMsg->AccountGM,
		lpMsg->NameGM,
		lpMsg->Type,
		lpMsg->Value);

	gQueryManager.Close();
}

void CCommandManager::GDCommandRenameRecv(const SDHP_COMMAND_RENAME_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_COMMAND_RENAME_RECV);

	SDHP_COMMAND_RENAME_SEND pMsg{};

	pMsg.Header.set(HEAD_COMMAND, SUB_COMMAND_RENAME, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.NewName, lpMsg->NewName, sizeof(pMsg.NewName));

	pMsg.Result = 2;

	if (gQueryManager.ExecQuery(
		"EXEC WZ_RenameCharacter '%s','%s','%s'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->NewName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.Result = static_cast<BYTE>(gQueryManager.GetResult(0));
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCommandManager::GDCommandBlocAccRecv(const SDHP_COMMAND_BLOC_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_COMMAND_BLOC_RECV);

	SDHP_COMMAND_BLOC_SEND pMsg{};

	pMsg.Header.set(HEAD_COMMAND, SUB_COMMAND_BLOCK_ACCOUNT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;
	pMsg.Result = 0;

	if (gQueryManager.ExecQuery(
		"UPDATE MEMB_INFO "
		"SET BLOC_CODE=1,BLOC_EXPIRE=DATEADD(DAY,%d,GETDATE()) "
		"WHERE MEMB___ID=(SELECT TOP 1 AccountID FROM Character WHERE Name='%s')",
		lpMsg->Days,
		lpMsg->CharacterNameBloc))
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CCommandManager::GDCommandBlocCharRecv(const SDHP_COMMAND_BLOC_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_COMMAND_BLOC_RECV);

	SDHP_COMMAND_BLOC_SEND pMsg{};

	pMsg.Header.set(HEAD_COMMAND, SUB_COMMAND_BLOCK_CHARACTER, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;
	pMsg.Result = 0;

	if (gQueryManager.ExecQuery(
		"UPDATE Character "
		"SET CTLCODE=1,BLOC_EXPIRE=DATEADD(DAY,%d,GETDATE()) "
		"WHERE Name='%s'",
		lpMsg->Days,
		lpMsg->CharacterNameBloc))
	{
		pMsg.Result = (gQueryManager.GetAffectedRows() == 1);
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));
}

void CCommandManager::GDCommandGiftRecv(const SDHP_GIFT_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_GIFT_RECV);

	SDHP_GIFT_SEND pMsg{};

	pMsg.Header.set(HEAD_COMMAND, SUB_COMMAND_GIFT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;
	pMsg.Result = 0;

	if (gQueryManager.ExecQuery(
		"UPDATE CustomGift "
		"SET Quantity=Quantity+1 "
		"OUTPUT INSERTED.Quantity "
		"WHERE AccountID='%s'; "
		"IF @@ROWCOUNT=0 "
		"BEGIN "
		"INSERT INTO CustomGift (AccountID,Quantity) "
		"OUTPUT INSERTED.Quantity "
		"VALUES ('%s',1); "
		"END",
		lpMsg->Account,
		lpMsg->Account))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.Result = gQueryManager.GetResult(0);
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));
}

void CCommandManager::GDCommandTopRecv(const SDHP_TOP_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_TOP_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_TOP_SEND pMsg{};

	pMsg.Header.set(HEAD_COMMAND, SUB_COMMAND_TOP, 0);
	pMsg.Index = lpMsg->Index;
	pMsg.Type = lpMsg->Type;
	pMsg.Class = lpMsg->Class;
	pMsg.Count = 0;

	int sendSize = sizeof(pMsg);

	const int wzClass = (lpMsg->Class == 999) ? -1 : lpMsg->Class;

	constexpr int maxCount =
		(sizeof(send) - sizeof(SDHP_TOP_SEND)) / sizeof(SDHP_TOP_INFO);

	if (gQueryManager.ExecQuery(
		"EXEC WZ_CustomTop %d,%d",
		lpMsg->Type,
		wzClass))
	{
		for (short sqlRet = gQueryManager.Fetch();
			sqlRet != SQL_NO_DATA &&
			sqlRet != SQL_NULL_DATA &&
			pMsg.Count < maxCount;
			sqlRet = gQueryManager.Fetch())
		{
			SDHP_TOP_INFO info{};

			gQueryManager.GetAsString(
				"VALUE1",
				info.CharacterName,
				sizeof(info.CharacterName));

			info.Value = gQueryManager.GetAsInteger("VALUE2");

			std::memcpy(send + sendSize, &info, sizeof(info));

			sendSize += sizeof(info);
			++pMsg.Count;
		}
	}

	gQueryManager.Close();

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);
}

