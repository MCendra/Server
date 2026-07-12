// MasterSkillTree.cpp
#include "Header.h"
#include "MasterSkillTree.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CMasterSkillTree gMasterSkillTree;

void CMasterSkillTree::GDMasterSkillTreeRecv(const SDHP_MASTER_SKILL_TREE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 401)

	VALIDATE_PACKET_SIZE(SDHP_MASTER_SKILL_TREE_RECV);

	SDHP_MASTER_SKILL_TREE_SEND pMsg{};
	pMsg.Header.set(HEAD_MASTER_SKILL, SUB_MASTER_SKILL_LOAD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	const bool dataFound =
		gQueryManager.ExecQuery(
#if (DATASERVER_UPDATE >= 602)
			"SELECT MasterLevel,MasterPoint,MasterExperience,MasterSkill "
#else
			"SELECT MasterLevel,MasterPoint,MasterExperience "
#endif
			"FROM MasterSkillTree WHERE Name='%s'",
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.MasterLevel = gQueryManager.GetAsInteger("MasterLevel");
		pMsg.MasterPoint = gQueryManager.GetAsInteger("MasterPoint");
		pMsg.MasterExperience = gQueryManager.GetAsInteger64("MasterExperience");

#if (DATASERVER_UPDATE >= 602)
		gQueryManager.GetAsBinary(
			"MasterSkill",
			pMsg.MasterSkill[0],
			sizeof(pMsg.MasterSkill));
#endif
	}
	else
	{
		pMsg.MasterLevel = 0;
		pMsg.MasterPoint = 0;
		pMsg.MasterExperience = 0;

#if (DATASERVER_UPDATE >= 602)
		std::memset(pMsg.MasterSkill, 0xFF, sizeof(pMsg.MasterSkill));
#endif
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CMasterSkillTree::GDMasterSkillTreeSaveRecv(const SDHP_MASTER_SKILL_TREE_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 401)

	VALIDATE_PACKET_SIZE(SDHP_MASTER_SKILL_TREE_SAVE_RECV);

#if (DATASERVER_UPDATE >= 602)

	gQueryManager.BindParameterAsBinary(
		1,
		lpMsg->MasterSkill[0],
		sizeof(lpMsg->MasterSkill));

	gQueryManager.ExecQuery(
		"UPDATE MasterSkillTree SET "
		"MasterLevel=%d,MasterPoint=%d,MasterExperience=%I64d,MasterSkill=? "
		"WHERE Name='%s'",
		lpMsg->MasterLevel,
		lpMsg->MasterPoint,
		lpMsg->MasterExperience,
		lpMsg->CharacterName);

#else

	gQueryManager.ExecQuery(
		"UPDATE MasterSkillTree SET "
		"MasterLevel=%d,MasterPoint=%d,MasterExperience=%I64d "
		"WHERE Name='%s'",
		lpMsg->MasterLevel,
		lpMsg->MasterPoint,
		lpMsg->MasterExperience,
		lpMsg->CharacterName);

#endif

	const SQLLEN affectedRows = gQueryManager.GetAffectedRows();

	gQueryManager.Close();

	if (affectedRows == 0)
	{
#if (DATASERVER_UPDATE >= 602)

		gQueryManager.BindParameterAsBinary(
			1,
			lpMsg->MasterSkill[0],
			sizeof(lpMsg->MasterSkill));

		gQueryManager.ExecQuery(
			"INSERT INTO MasterSkillTree "
			"(Name,MasterLevel,MasterPoint,MasterExperience,MasterSkill) "
			"VALUES ('%s',%d,%d,%I64d,?)",
			lpMsg->CharacterName,
			lpMsg->MasterLevel,
			lpMsg->MasterPoint,
			lpMsg->MasterExperience);

#else

		gQueryManager.ExecQuery(
			"INSERT INTO MasterSkillTree "
			"(Name,MasterLevel,MasterPoint,MasterExperience) "
			"VALUES ('%s',%d,%d,%I64d)",
			lpMsg->CharacterName,
			lpMsg->MasterLevel,
			lpMsg->MasterPoint,
			lpMsg->MasterExperience);

#endif

		gQueryManager.Close();
	}

#endif
}