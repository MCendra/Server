// Quest.cpp
#include "Header.h"
#include "Quest.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CQuest gQuest;

void CQuest::GDQuestKillCountRecv(const SDHP_QUEST_KILL_COUNT_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_QUEST_KILL_COUNT_RECV);

	SDHP_QUEST_KILL_COUNT_SEND pMsg{};
	pMsg.Header.set(HEAD_QUEST, SUB_QUEST_KILLCOUNT_LOAD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	const bool dataFound =
		gQueryManager.ExecQuery(
			"SELECT QuestIndex,MonsterClass1,MonsterClass2,MonsterClass3,MonsterClass4,MonsterClass5,"
			"KillCount1,KillCount2,KillCount3,KillCount4,KillCount5 "
			"FROM QuestKillCount WHERE Name='%s'",
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.QuestIndex = gQueryManager.GetAsInteger("QuestIndex");

		for (int n = 0; n < 5; ++n)
		{
			char column[16];

			std::snprintf(column, sizeof(column), "MonsterClass%d", n + 1);
			pMsg.MonsterClass[n] = gQueryManager.GetAsInteger(column);

			std::snprintf(column, sizeof(column), "KillCount%d", n + 1);
			pMsg.KillCount[n] = gQueryManager.GetAsInteger(column);
		}
	}
	else
	{
		pMsg.QuestIndex = 0xFFFFFFFF;

		std::memset(pMsg.MonsterClass, 0xFF, sizeof(pMsg.MonsterClass));
		std::memset(pMsg.KillCount, 0, sizeof(pMsg.KillCount));
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));
}

void CQuest::GDQuestKillCountSaveRecv(const SDHP_QUEST_KILL_COUNT_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_QUEST_KILL_COUNT_SAVE_RECV);

	const bool dataFound =
		gQueryManager.ExecQuery(
			"SELECT Name FROM QuestKillCount WHERE Name='%s'",
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	gQueryManager.Close();

	if (!dataFound)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO QuestKillCount "
			"(Name,QuestIndex,MonsterClass1,KillCount1,MonsterClass2,KillCount2,"
			"MonsterClass3,KillCount3,MonsterClass4,KillCount4,MonsterClass5,KillCount5) "
			"VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
			lpMsg->CharacterName,
			lpMsg->QuestIndex,
			lpMsg->MonsterClass[0],
			lpMsg->KillCount[0],
			lpMsg->MonsterClass[1],
			lpMsg->KillCount[1],
			lpMsg->MonsterClass[2],
			lpMsg->KillCount[2],
			lpMsg->MonsterClass[3],
			lpMsg->KillCount[3],
			lpMsg->MonsterClass[4],
			lpMsg->KillCount[4]);
	}
	else
	{
		gQueryManager.ExecQuery(
			"UPDATE QuestKillCount SET "
			"QuestIndex=%d,"
			"MonsterClass1=%d,KillCount1=%d,"
			"MonsterClass2=%d,KillCount2=%d,"
			"MonsterClass3=%d,KillCount3=%d,"
			"MonsterClass4=%d,KillCount4=%d,"
			"MonsterClass5=%d,KillCount5=%d "
			"WHERE Name='%s'",
			lpMsg->QuestIndex,
			lpMsg->MonsterClass[0],
			lpMsg->KillCount[0],
			lpMsg->MonsterClass[1],
			lpMsg->KillCount[1],
			lpMsg->MonsterClass[2],
			lpMsg->KillCount[2],
			lpMsg->MonsterClass[3],
			lpMsg->KillCount[3],
			lpMsg->MonsterClass[4],
			lpMsg->KillCount[4],
			lpMsg->CharacterName);
	}

	gQueryManager.Close();
}
