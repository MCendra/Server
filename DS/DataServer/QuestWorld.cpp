// QuestWorld.cpp
#include "Header.h"
#include "QuestWorld.h"
#include "QueryManager.h"
#include "SocketManager.h"

CQuestWorld gQuestWorld;

void CQuestWorld::GDQuestWorldRecv(SDHP_QUEST_WORLD_RECV* lpMsg, int index)
{
#if(DATASERVER_UPDATE>=501)

	SDHP_QUEST_WORLD_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_QUEST_WORLD, DS_SUB_QUEST_WORLD_LOAD, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (!gQueryManager.ExecQuery("SELECT QuestWorldList FROM QuestWorld WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		memset(pMsg.QuestWorldList, 0xFF, sizeof(pMsg.QuestWorldList));
	}
	else
	{
		gQueryManager.GetAsBinary("QuestWorldList", pMsg.QuestWorldList[0], sizeof(pMsg.QuestWorldList));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CQuestWorld::GDQuestWorldSaveRecv(SDHP_QUEST_WORLD_SAVE_RECV* lpMsg)
{
#if(DATASERVER_UPDATE>=501)

	if (!gQueryManager.ExecQuery("SELECT Name FROM QuestWorld WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.BindParameterAsBinary(1, lpMsg->QuestWorldList[0], sizeof(lpMsg->QuestWorldList));

		gQueryManager.ExecQuery("INSERT INTO QuestWorld (Name,QuestWorldList) VALUES ('%s',?)", lpMsg->CharacterName);

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();

		gQueryManager.BindParameterAsBinary(1, lpMsg->QuestWorldList[0], sizeof(lpMsg->QuestWorldList));

		gQueryManager.ExecQuery("UPDATE QuestWorld SET QuestWorldList=? WHERE Name='%s'", lpMsg->CharacterName);

		gQueryManager.Close();
	}

#endif
}