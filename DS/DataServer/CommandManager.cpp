// CommandManager.cpp
#include "Header.h"
#include "CommandManager.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "Util.h"

CCommandManager gCommandManager;

void CCommandManager::GDCommandResetRecv(SDHP_COMMAND_RESET_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_COMMAND_RESET_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_COMMAND, DS_SUB_COMMAND_RESET, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"EXEC WZ_GetResetInfo '%s','%s'",
		lpMsg->Account,
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.ResetDay = gQueryManager.GetAsInteger("ResetDay");
			pMsg.ResetWek = gQueryManager.GetAsInteger("ResetWek");
			pMsg.ResetMon = gQueryManager.GetAsInteger("ResetMon");
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);
}

void CCommandManager::GDCommandMasterResetRecv(SDHP_COMMAND_MASTER_RESET_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_COMMAND_MASTER_RESET_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_COMMAND, DS_SUB_COMMAND_MASTER_RESET, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"EXEC WZ_GetMasterResetInfo '%s','%s'",
		lpMsg->Account,
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.MasterResetDay = gQueryManager.GetAsInteger("MasterResetDay");
			pMsg.MasterResetWek = gQueryManager.GetAsInteger("MasterResetWek");
			pMsg.MasterResetMon = gQueryManager.GetAsInteger("MasterResetMon");
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);
}

// Marry System
void CCommandManager::GDCommandMarryRecv(SDHP_COMMAND_MARRY_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_COMMAND_MARRY_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_COMMAND, DS_SUB_COMMAND_MARRY, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.MarryName, lpMsg->MarryName, sizeof(pMsg.MarryName));
	std::memcpy(pMsg.Mode, lpMsg->Mode, sizeof(pMsg.Mode));

	if (gQueryManager.ExecQuery(
		"EXEC WZ_GetMarryInfo '%s','%s'",
		lpMsg->CharacterName,
		lpMsg->MarryName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.CountYou = gQueryManager.GetAsInteger("CountYou");
			pMsg.CountHim = gQueryManager.GetAsInteger("CountHim");
			pMsg.MarriedOn = gQueryManager.GetAsInteger("MarriedOn");

			gQueryManager.GetAsString("NameGet1", pMsg.NameGet1, sizeof(pMsg.NameGet1));
			gQueryManager.GetAsString("NameGet2", pMsg.NameGet2, sizeof(pMsg.NameGet2));
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);
}

void CCommandManager::GDCommandRewardRecv(SDHP_COMMAND_REWARD_RECV* lpMsg, int index)
{
	UNREFERENCED_PARAMETER(index);

	if (lpMsg == nullptr)
	{
		return;
	}

	gQueryManager.ExecQuery(
		"EXEC WZ_SetReward '%s','%s','%s','%s','%d','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->AccountGM,
		lpMsg->NameGM,
		lpMsg->Type,
		lpMsg->Value);

	gQueryManager.Close();
}

void CCommandManager::GDCommandRewardAllRecv(SDHP_COMMAND_REWARDALL_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	gQueryManager.ExecQuery(
		"EXEC WZ_SetRewardAll '%s','%s','%d','%d'",
		lpMsg->AccountGM,
		lpMsg->NameGM,
		lpMsg->Type,
		lpMsg->Value);

	gQueryManager.Close();
}

void CCommandManager::GDCommandRenameRecv(SDHP_COMMAND_RENAME_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_COMMAND_RENAME_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_COMMAND, DS_SUB_COMMAND_RENAME, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.NewName, lpMsg->NewName, sizeof(pMsg.NewName));

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
		else
		{
			pMsg.Result = 2;
		}
	}
	else
	{
		pMsg.Result = 2;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);
}

void CCommandManager::GDCommandBlocAccRecv(SDHP_COMMAND_BLOC_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_COMMAND_BLOC_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_COMMAND, DS_SUB_COMMAND_BLOCK_ACCOUNT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;
	pMsg.Result = 0;

	if (gQueryManager.ExecQuery(
		"SELECT TOP 1 1 FROM Character WHERE Name='%s'",
		lpMsg->CharacterNameBloc))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			gQueryManager.Close();

			if (gQueryManager.ExecQuery(
				"UPDATE MEMB_INFO "
				"SET BLOC_CODE=1, BLOC_EXPIRE=(GETDATE()+%d) "
				"WHERE MEMB___ID=(SELECT TOP 1 AccountID FROM Character WHERE Name='%s')",
				lpMsg->Days,
				lpMsg->CharacterNameBloc))
			{
				pMsg.Result = 1;
			}
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);
}

void CCommandManager::GDCommandBlocCharRecv(SDHP_COMMAND_BLOC_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_COMMAND_BLOC_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_COMMAND, DS_SUB_COMMAND_BLOCK_CHARACTER, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;
	pMsg.Result = 0;

	if (gQueryManager.ExecQuery(
		"SELECT 1 FROM Character WHERE Name='%s'",
		lpMsg->CharacterNameBloc))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			gQueryManager.Close();

			if (gQueryManager.ExecQuery(
				"UPDATE Character "
				"SET CTLCODE=1, BLOC_EXPIRE=(GETDATE()+%d) "
				"WHERE Name='%s'",
				lpMsg->Days,
				lpMsg->CharacterNameBloc))
			{
				pMsg.Result = 1;
			}
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);
}

void CCommandManager::GDCommandGiftRecv(SDHP_GIFT_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_GIFT_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_COMMAND, DS_SUB_COMMAND_GIFT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;
	pMsg.Result = 0;

	if (gQueryManager.ExecQuery(
		"SELECT Quantity FROM CustomGift WHERE AccountID='%s'",
		lpMsg->Account))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet == SQL_NO_DATA)
		{
			gQueryManager.Close();

			if (gQueryManager.ExecQuery(
				"INSERT INTO CustomGift (AccountID,Quantity) VALUES ('%s',1)",
				lpMsg->Account))
			{
				pMsg.Result = 1;
			}
		}
		else if (sqlRet != SQL_NULL_DATA)
		{
			pMsg.Result = gQueryManager.GetAsInteger("Quantity") + 1;

			gQueryManager.Close();

			if (gQueryManager.ExecQuery(
				"UPDATE CustomGift SET Quantity=Quantity+1 WHERE AccountID='%s'",
				lpMsg->Account) == FALSE)
			{
				pMsg.Result = 0;
			}
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);
}

void CCommandManager::GDCommandTopRecv(SDHP_TOP_RECV* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	BYTE send[4096]{};

	SDHP_TOP_SEND pMsg{};
	SDHP_TOP_INFO info{};

	pMsg.Header.set(DS_HEAD_COMMAND, DS_SUB_COMMAND_TOP, 0);

	int size = sizeof(pMsg);

	pMsg.Index = lpMsg->Index;
	pMsg.Type = lpMsg->Type;
	pMsg.Class = lpMsg->Class;
	pMsg.Count = 0;

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
			gQueryManager.GetAsString(
				"VALUE1",
				info.CharacterName,
				sizeof(info.CharacterName));

			info.Value = gQueryManager.GetAsInteger("VALUE2");

			std::memcpy(&send[size], &info, sizeof(info));

			size += sizeof(SDHP_TOP_INFO);
			++pMsg.Count;
		}
	}

	gQueryManager.Close();

	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);
}

