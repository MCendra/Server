// QuestWorld.cpp
#include "Header.h"
#include "QuestWorld.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CQuestWorld gQuestWorld;

void CQuestWorld::GDQuestWorldRecv(const SDHP_QUEST_WORLD_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_QUEST_WORLD_RECV);

	SDHP_QUEST_WORLD_SEND pMsg{};

	pMsg.Header.set(HEAD_QUEST_WORLD, SUB_QUEST_WORLD_LOAD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	std::memset(pMsg.QuestWorldList, 0xFF, sizeof(pMsg.QuestWorldList));

	if (gQueryManager.ExecQuery(
		"SELECT QuestWorldList FROM QuestWorld WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			gQueryManager.GetAsBinary(
				"QuestWorldList",
				pMsg.QuestWorldList[0],
				sizeof(pMsg.QuestWorldList));
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CQuestWorld::GDQuestWorldSaveRecv(const SDHP_QUEST_WORLD_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_QUEST_WORLD_SAVE_RECV);

	gQueryManager.BindParameterAsBinary(
		1,
		lpMsg->QuestWorldList[0],
		sizeof(lpMsg->QuestWorldList));

	if (gQueryManager.ExecQuery(
		"UPDATE QuestWorld SET QuestWorldList=? WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const SQLLEN affectedRows = gQueryManager.GetAffectedRows();

		gQueryManager.Close();

		if (affectedRows == 0)
		{
			gQueryManager.BindParameterAsBinary(
				1,
				lpMsg->QuestWorldList[0],
				sizeof(lpMsg->QuestWorldList));

			gQueryManager.ExecQuery(
				"INSERT INTO QuestWorld (Name,QuestWorldList) VALUES ('%s',?)",
				lpMsg->CharacterName);

			gQueryManager.Close();
		}
	}
	else
	{
		gQueryManager.Close();
	}

#endif
}