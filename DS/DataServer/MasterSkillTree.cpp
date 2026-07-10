// MasterSkillTree.cpp
#include "Header.h"
#include "MasterSkillTree.h"
#include "QueryManager.h"
#include "SocketManager.h"

CMasterSkillTree gMasterSkillTree;

void CMasterSkillTree::GDMasterSkillTreeRecv(SDHP_MASTER_SKILL_TREE_RECV* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 401)

	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_MASTER_SKILL_TREE_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_MASTER_SKILL, DS_SUB_MASTER_SKILL_LOAD, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"SELECT * FROM MasterSkillTree WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
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
			memset(pMsg.MasterSkill, 0xFF, sizeof(pMsg.MasterSkill));
#endif
		}
	}
	else
	{
		pMsg.MasterLevel = 0;
		pMsg.MasterPoint = 0;
		pMsg.MasterExperience = 0;

#if (DATASERVER_UPDATE >= 602)
		memset(pMsg.MasterSkill, 0xFF, sizeof(pMsg.MasterSkill));
#endif
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CMasterSkillTree::GDMasterSkillTreeSaveRecv(SDHP_MASTER_SKILL_TREE_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 401)

	if (lpMsg == nullptr)
	{
		return;
	}

	bool exists = false;

	if (gQueryManager.ExecQuery(
		"SELECT 1 FROM MasterSkillTree WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		exists = (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA);
	}

	gQueryManager.Close();

#if (DATASERVER_UPDATE >= 602)

	gQueryManager.BindParameterAsBinary(
		1,
		lpMsg->MasterSkill[0],
		sizeof(lpMsg->MasterSkill));

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE MasterSkillTree "
			"SET MasterLevel=%d,MasterPoint=%d,MasterExperience=%I64d,MasterSkill=? "
			"WHERE Name='%s'",
			lpMsg->MasterLevel,
			lpMsg->MasterPoint,
			lpMsg->MasterExperience,
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO MasterSkillTree "
			"(Name,MasterLevel,MasterPoint,MasterExperience,MasterSkill) "
			"VALUES ('%s',%d,%d,%I64d,?)",
			lpMsg->CharacterName,
			lpMsg->MasterLevel,
			lpMsg->MasterPoint,
			lpMsg->MasterExperience);
	}

#else

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE MasterSkillTree "
			"SET MasterLevel=%d,MasterPoint=%d,MasterExperience=%I64d "
			"WHERE Name='%s'",
			lpMsg->MasterLevel,
			lpMsg->MasterPoint,
			lpMsg->MasterExperience,
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO MasterSkillTree "
			"(Name,MasterLevel,MasterPoint,MasterExperience) "
			"VALUES ('%s',%d,%d,%I64d)",
			lpMsg->CharacterName,
			lpMsg->MasterLevel,
			lpMsg->MasterPoint,
			lpMsg->MasterExperience);
	}

#endif

	gQueryManager.Close();

#endif
}