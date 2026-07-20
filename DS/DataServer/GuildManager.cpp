// GuildManager.cpp
#include "Header.h"
#include "GuildManager.h"
#include "Log.h"
#include "ESProtocol.h"
#include "QueryManager.h"
#include "Util.h"

CGuildManager gGuildManager;

void CGuildManager::Init()
{
	vGuildList.clear();

	m_GuildByName.clear();
	m_GuildByNumber.clear();
	m_GuildMembers.clear();

	if (gQueryManager.ExecQuery("SELECT * FROM Guild"))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			GUILD_INFO guildInfo;

			guildInfo.Clear();

			gQueryManager.GetAsString("G_Name", guildInfo.Name, sizeof(guildInfo.Name));
			gQueryManager.GetAsBinary("G_Mark", guildInfo.Mark, sizeof(guildInfo.Mark));

			guildInfo.Score = gQueryManager.GetAsInteger("G_Score");

#if (GUILDBOSSEVENT)
			guildInfo.Score1 = gQueryManager.GetAsInteger("G_Score1");
#endif

			gQueryManager.GetAsString("G_Master", guildInfo.Master, sizeof(guildInfo.Master));
			gQueryManager.GetAsString("G_Notice", guildInfo.Notice, sizeof(guildInfo.Notice));

			guildInfo.Number = gQueryManager.GetAsInteger("Number");
			guildInfo.Type = gQueryManager.GetAsInteger("G_Type");
			guildInfo.RivalNumber = gQueryManager.GetAsInteger("G_Rival");
			guildInfo.UnionNumber = gQueryManager.GetAsInteger("G_Union");

			vGuildList.emplace_back(guildInfo);

			GUILD_INFO* guild = &vGuildList.back();

			m_GuildByName.emplace(guild->Name, guild);
			m_GuildByNumber.emplace(guild->Number, guild);
		}
	}

	gQueryManager.Close();

	if (gQueryManager.ExecQuery("SELECT * FROM GuildMember"))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			char guildName[9] = {};

			GUILD_MEMBER_INFO guildMemberInfo;
			guildMemberInfo.Clear();

			gQueryManager.GetAsString("Name", guildMemberInfo.GuildMember, sizeof(guildMemberInfo.GuildMember));
			gQueryManager.GetAsString("G_Name", guildName, sizeof(guildName));

			guildMemberInfo.Status = gQueryManager.GetAsInteger("G_Status");

			GUILD_INFO* lpGuildInfo = GetGuildInfo(guildName);

			if (lpGuildInfo == nullptr)
			{
				continue;
			}

			GUILD_MEMBER_INFO* member = nullptr;

			if (_stricmp(lpGuildInfo->Master, guildMemberInfo.GuildMember) == 0)
			{
				lpGuildInfo->GuildMember[0] = guildMemberInfo;
				member = &lpGuildInfo->GuildMember[0];
			}
			else
			{
				for (size_t n = 1; n < MAX_GUILD_MEMBER; ++n)
				{
					if (lpGuildInfo->GuildMember[n].IsEmpty())
					{
						lpGuildInfo->GuildMember[n] = guildMemberInfo;
						member = &lpGuildInfo->GuildMember[n];
						break;
					}
				}
			}

			if (member != nullptr)
			{
				m_GuildMembers.emplace(member->GuildMember, member);
			}
		}
	}

	gQueryManager.Close();
}

GUILD_INFO* CGuildManager::GetGuildInfo(const char* Name)
{
	auto it = m_GuildByName.find(Name);

	return (it != m_GuildByName.end()) ? it->second : nullptr;
}

GUILD_INFO* CGuildManager::GetGuildInfo(DWORD Number)
{
	auto it = m_GuildByNumber.find(Number);

	return (it != m_GuildByNumber.end()) ? it->second : nullptr;
}

GUILD_INFO* CGuildManager::GetMemberGuildInfo(const char* szGuildMember)
{
	GUILD_MEMBER_INFO* lpGuildMemberInfo = GetGuildMemberInfo(szGuildMember);

	if (lpGuildMemberInfo == nullptr)
	{
		return nullptr;
	}

	for (auto& guild : vGuildList)
	{
		if (lpGuildMemberInfo >= &guild.GuildMember[0] &&
			lpGuildMemberInfo <= &guild.GuildMember[MAX_GUILD_MEMBER - 1])
		{
			return &guild;
		}
	}

	return nullptr;
}

GUILD_MEMBER_INFO* CGuildManager::GetGuildMemberInfo(const char* szGuildMember)
{
	auto it = m_GuildMembers.find(szGuildMember);

	return (it != m_GuildMembers.end()) ? it->second : nullptr;
}

BOOL CGuildManager::CheckGuildOnCS(const char* szGuildName)
{
	if (!gQueryManager.ExecQuery("EXEC WZ_CS_CheckSiegeGuildList '%s'", szGuildName) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	BOOL result = gQueryManager.GetResult(0);

	gQueryManager.Close();

	return result;
}

void CGuildManager::ConnectMember(const char* szGuildMember, WORD btServer)
{
	if (GUILD_MEMBER_INFO* lpGuildMemberInfo = GetGuildMemberInfo(szGuildMember))
	{
		lpGuildMemberInfo->Server = btServer;
	}
}

void CGuildManager::DisconnectMember(const char* szGuildMember)
{
	if (GUILD_MEMBER_INFO* lpGuildMemberInfo = GetGuildMemberInfo(szGuildMember))
	{
		lpGuildMemberInfo->Server = 0xFFFF;
	}
}

BYTE CGuildManager::AddGuild(int index, const char* guildName, const char* masterName, const BYTE* lpMark, BYTE type)
{
	if (GetGuildInfo(guildName) != nullptr)
	{
		return 3;
	}

	if (gUtil.CheckTextSyntax(guildName, strlen(guildName)) == 0)
	{
		return 4;
	}

	if (!gQueryManager.ExecQuery("WZ_GuildCreate '%s','%s'", guildName, masterName) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		return 0;
	}

	if (gQueryManager.GetResult(0) != 0)
	{
		gQueryManager.Close();
		return 2;
	}

	gQueryManager.Close();

	gQueryManager.BindParameterAsBinary(1, lpMark, 32);
	gQueryManager.ExecQuery("UPDATE Guild SET G_Mark=?,G_Type=%d WHERE G_Name='%s'", type, guildName);
	gQueryManager.Close();

	gQueryManager.ExecQuery("UPDATE GuildMember SET G_Status=%d WHERE Name='%s'", 0x80, masterName);
	gQueryManager.Close();

	GUILD_INFO guildInfo;
	guildInfo.Clear();

	if (gQueryManager.ExecQuery("SELECT Number FROM Guild WHERE G_Name='%s'", guildName) && gQueryManager.Fetch() != SQL_NO_DATA)
	{
		guildInfo.Number = gQueryManager.GetAsInteger("Number");
	}

	gQueryManager.Close();

	memcpy(guildInfo.Name, guildName, sizeof(guildInfo.Name));
	memcpy(guildInfo.Master, masterName, sizeof(guildInfo.Master));

	guildInfo.Type = type;

	memcpy(guildInfo.Mark, lpMark, sizeof(guildInfo.Mark));

	memcpy(guildInfo.GuildMember[0].GuildMember, masterName, sizeof(guildInfo.GuildMember[0].GuildMember));

	guildInfo.GuildMember[0].Status = 0x80;
	guildInfo.GuildMember[0].Server = 0xFFFF;

	vGuildList.emplace_back(guildInfo);

	GUILD_INFO* guild = &vGuildList.back();

	m_GuildByName.emplace(guild->Name, guild);
	m_GuildByNumber.emplace(guild->Number, guild);
	m_GuildMembers.emplace(guild->GuildMember[0].GuildMember, &guild->GuildMember[0]);

	return 1;
}

BYTE CGuildManager::DelGuild(int index, const char* szGuildName)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(szGuildName);

	if (lpGuildInfo == nullptr)
	{
		return 3;
	}

	if (CheckGuildOnCS(szGuildName) != 0)
	{
		return 2;
	}

	DelGuildRelationship(index, lpGuildInfo->Number, 1);
	DelGuildRelationship(index, lpGuildInfo->Number, 2);

	m_GuildByName.erase(lpGuildInfo->Name);
	m_GuildByNumber.erase(lpGuildInfo->Number);

	for (const auto& member : lpGuildInfo->GuildMember)
	{
		if (!member.IsEmpty())
		{
			m_GuildMembers.erase(member.GuildMember);
		}
	}

	lpGuildInfo->Clear();

	gQueryManager.ExecQuery("DELETE FROM Guild WHERE G_Name='%s'", szGuildName);
	gQueryManager.Close();

	gQueryManager.ExecQuery("DELETE FROM GuildMember WHERE G_Name='%s'", szGuildName);
	gQueryManager.Close();

	gQueryManager.ExecQuery("DELETE FROM WarehouseGuild WHERE Guild='%s'", szGuildName);
	gQueryManager.Close();

	return 1;
}

BYTE CGuildManager::AddGuildMember(int index, const char* szGuildName, const char* szGuildMember, BYTE btStatus, WORD btServer)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(szGuildName);

	if (lpGuildInfo == nullptr)
	{
		return 0;
	}

	if (GetGuildMemberInfo(szGuildMember) != nullptr)
	{
		return 3;
	}

	if (!gQueryManager.ExecQuery(
		"INSERT INTO GuildMember (Name,G_Name,G_Level,G_Status) VALUES ('%s','%s',%d,%d)",
		szGuildMember, szGuildName, 1, btStatus))
	{
		gQueryManager.Close();
		return 4;
	}

	gQueryManager.Close();

	for (size_t n = 1; n < MAX_GUILD_MEMBER; ++n)
	{
		if (lpGuildInfo->GuildMember[n].IsEmpty())
		{
			GUILD_MEMBER_INFO& member = lpGuildInfo->GuildMember[n];

			memcpy(member.GuildMember, szGuildMember, sizeof(member.GuildMember));

			member.Status = btStatus;
			member.Server = btServer;

			m_GuildMembers.emplace(member.GuildMember, &member);

			return 1;
		}
	}

	return 4;
}

BYTE CGuildManager::DelGuildMember(int index, const char* szGuildMember)
{
	GUILD_MEMBER_INFO* lpGuildMemberInfo = GetGuildMemberInfo(szGuildMember);

	if (lpGuildMemberInfo == nullptr)
	{
		return 3;
	}

	gQueryManager.ExecQuery("DELETE FROM GuildMember WHERE Name='%s'", szGuildMember);
	gQueryManager.Close();

	m_GuildMembers.erase(szGuildMember);

	lpGuildMemberInfo->Clear();

	return 1;
}

BYTE CGuildManager::AddGuildRelationship(int index,DWORD dwSourceGuild,DWORD dwTargetGuild,BYTE btRelationType)
{
	GUILD_INFO* lpSourceGuild = GetGuildInfo(dwSourceGuild);

	GUILD_INFO* lpTargetGuild = GetGuildInfo(dwTargetGuild);

	if(lpSourceGuild == nullptr)
	{
		return 0;
	}

	if(lpTargetGuild == nullptr)
	{
		return 0;
	}

	if(CheckGuildOnCS(lpSourceGuild->Name) != 0)
	{
		return 16;
	}

	if(CheckGuildOnCS(lpTargetGuild->Name) != 0)
	{
		return 16;
	}

	DWORD UnionGuildNumber[MAX_GUILD_UNION];

	DWORD RivalGuildNumber[MAX_GUILD_RIVAL];

	DWORD unionGuildAmount = 0;
	DWORD rivalGuildAmount = 0;

	switch(btRelationType)
	{
		case 1:
			if(lpSourceGuild->UnionNumber > 0)
			{
				return 0;
			}

			if(lpSourceGuild->RivalNumber > 0)
			{
				return 0;
			}

			unionGuildAmount = GetUnionList(lpTargetGuild->Number, UnionGuildNumber);

			if (unionGuildAmount >= MAX_GUILD_UNION)
			{
				return 0;
			}

			if(!gQueryManager.ExecQuery("UPDATE Guild SET G_Union=%d WHERE Number=%d",lpTargetGuild->Number,lpTargetGuild->Number))
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpTargetGuild->UnionNumber = lpTargetGuild->Number;
			}

			if(!gQueryManager.ExecQuery("UPDATE Guild SET G_Union=%d WHERE Number=%d",lpTargetGuild->Number,lpSourceGuild->Number))
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpSourceGuild->UnionNumber = lpTargetGuild->Number;
			}

			DGRelationShipListRecv(index,lpTargetGuild->Number,1);

			DGRelationShipNotificationRecv(index,0,unionGuildAmount,(int*)UnionGuildNumber);

			return 1;
		case 2:
			if(lpSourceGuild->RivalNumber > 0)
			{
				return 0;
			}

			if(lpTargetGuild->RivalNumber > 0)
			{
				return 0;
			}

			if(lpSourceGuild->UnionNumber > 0 && lpSourceGuild->UnionNumber != lpSourceGuild->Number)
			{
				return 0;
			}

			if(lpTargetGuild->UnionNumber > 0 && lpTargetGuild->UnionNumber != lpTargetGuild->Number)
			{
				return 0;
			}

			if(!gQueryManager.ExecQuery("UPDATE Guild SET G_Rival=%d WHERE Number=%d",lpSourceGuild->Number,lpTargetGuild->Number))
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpTargetGuild->RivalNumber = lpSourceGuild->Number;
			}

			if(!gQueryManager.ExecQuery("UPDATE Guild SET G_Rival=%d WHERE Number=%d",lpTargetGuild->Number,lpSourceGuild->Number))
			{
				gQueryManager.Close();
				return 0;
			}
			else
			{
				gQueryManager.Close();
				lpSourceGuild->RivalNumber = lpTargetGuild->Number;
			}

			DGRelationShipListRecv(index,lpSourceGuild->Number,2);

			DGRelationShipListRecv(index,lpTargetGuild->Number,2);

			RivalGuildNumber[rivalGuildAmount++] = lpSourceGuild->Number;

			RivalGuildNumber[rivalGuildAmount++] = lpTargetGuild->Number;

			DGRelationShipNotificationRecv(index,0,rivalGuildAmount,(int*)RivalGuildNumber);

			return 1;
	}

	return 0;
}

BYTE CGuildManager::DelGuildRelationship(int index, DWORD dwSourceGuild, BYTE btRelationType)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(dwSourceGuild);

	if (lpGuildInfo == nullptr)
	{
		return 0;
	}

	GUILD_INFO* lpUnionInfo = nullptr;
	GUILD_INFO* lpRivalInfo = nullptr;

	DWORD unionGuildNumber[MAX_GUILD_UNION];
	DWORD rivalGuildNumber[MAX_GUILD_RIVAL];

	DWORD unionGuildAmount = 0;
	DWORD rivalGuildAmount = 0;

	switch (btRelationType)
	{
	case 1:

		if (CheckGuildOnCS(lpGuildInfo->Name) != 0)
		{
			return 16;
		}

		if (lpGuildInfo->Number == lpGuildInfo->UnionNumber)
		{
			return 0;
		}

		lpUnionInfo = GetGuildInfo(lpGuildInfo->UnionNumber);

		if (lpUnionInfo == nullptr)
		{
			return 0;
		}

		unionGuildAmount = GetUnionList(lpUnionInfo->Number, unionGuildNumber);

		if (unionGuildAmount == 0)
		{
			return 0;
		}

		if (unionGuildAmount == 2)
		{
			if (!gQueryManager.ExecQuery(
				"UPDATE Guild SET G_Union=0 WHERE Number IN (%d,%d)",
				unionGuildNumber[0], unionGuildNumber[1]))
			{
				gQueryManager.Close();
				return 0;
			}

			gQueryManager.Close();

			lpGuildInfo->UnionNumber = 0;
			lpUnionInfo->UnionNumber = 0;

			DGRelationShipListRecv(index, lpUnionInfo->Number, 1);
			DGRelationShipNotificationRecv(index, 16, unionGuildAmount, (int*)unionGuildNumber);

			return 1;
		}

		if (!gQueryManager.ExecQuery(
			"UPDATE Guild SET G_Union=0 WHERE Number=%d",
			lpGuildInfo->Number))
		{
			gQueryManager.Close();
			return 0;
		}

		gQueryManager.Close();

		lpGuildInfo->UnionNumber = 0;

		DGRelationShipListRecv(index, lpUnionInfo->Number, 1);
		DGRelationShipNotificationRecv(index, 0, unionGuildAmount, (int*)unionGuildNumber);

		return 1;

	case 2:

		if (CheckGuildOnCS(lpGuildInfo->Name) != 0)
		{
			return 16;
		}

		lpRivalInfo = GetGuildInfo(lpGuildInfo->RivalNumber);

		if (lpRivalInfo == nullptr)
		{
			return 0;
		}

		if (!gQueryManager.ExecQuery(
			"UPDATE Guild SET G_Rival=0 WHERE Number IN (%d,%d)",
			lpGuildInfo->Number, lpRivalInfo->Number))
		{
			gQueryManager.Close();
			return 0;
		}

		gQueryManager.Close();

		lpGuildInfo->RivalNumber = 0;
		lpRivalInfo->RivalNumber = 0;

		DGRelationShipListRecv(index, lpGuildInfo->Number, 2);
		DGRelationShipListRecv(index, lpRivalInfo->Number, 2);

		rivalGuildNumber[rivalGuildAmount++] = lpGuildInfo->Number;
		rivalGuildNumber[rivalGuildAmount++] = lpRivalInfo->Number;

		DGRelationShipNotificationRecv(index, 0, rivalGuildAmount, (int*)rivalGuildNumber);

		return 1;
	}

	return 0;
}

BYTE CGuildManager::SetGuildRelationship(int index, const char* szGuildMember, const char* szGuildMaster)
{
	GUILD_INFO* lpSourceGuild = GetGuildInfo(szGuildMember);
	GUILD_INFO* lpTargetGuild = GetGuildInfo(szGuildMaster);

	if (lpSourceGuild == nullptr || lpTargetGuild == nullptr)
	{
		return 0;
	}

	if (lpSourceGuild->UnionNumber != lpTargetGuild->Number)
	{
		return 0;
	}

	if (lpTargetGuild->UnionNumber != lpTargetGuild->Number)
	{
		return 0;
	}

	if (CheckGuildOnCS(lpSourceGuild->Name) != 0)
	{
		return 16;
	}

	if (CheckGuildOnCS(lpTargetGuild->Name) != 0)
	{
		return 16;
	}

	DWORD unionGuildNumber[MAX_GUILD_UNION];

	DWORD unionGuildAmount = GetUnionList(lpTargetGuild->Number, unionGuildNumber);

	if (unionGuildAmount == 2)
	{
		if (!gQueryManager.ExecQuery(
			"UPDATE Guild SET G_Union=0 WHERE Number IN (%d,%d)",
			lpSourceGuild->Number,
			lpTargetGuild->Number))
		{
			gQueryManager.Close();
			return 0;
		}

		gQueryManager.Close();

		lpSourceGuild->UnionNumber = 0;
		lpTargetGuild->UnionNumber = 0;

		DGRelationShipListRecv(index, lpTargetGuild->Number, 1);
		DGRelationShipNotificationRecv(index, 16, unionGuildAmount, (int*)unionGuildNumber);

		return 1;
	}

	if (!gQueryManager.ExecQuery(
		"UPDATE Guild SET G_Union=0 WHERE Number=%d",
		lpSourceGuild->Number))
	{
		gQueryManager.Close();
		return 0;
	}

	gQueryManager.Close();

	lpSourceGuild->UnionNumber = 0;

	DGRelationShipListRecv(index, lpTargetGuild->Number, 1);
	DGRelationShipNotificationRecv(index, 0, unionGuildAmount, (int*)unionGuildNumber);

	return 1;
}

BYTE CGuildManager::SetGuildScore(const char* szGuildName, DWORD Score)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(szGuildName);

	if (lpGuildInfo == nullptr)
	{
		return 0;
	}

	if (!gQueryManager.ExecQuery(
		"UPDATE Guild SET G_Score=%d WHERE G_Name='%s'",
		Score,
		szGuildName))
	{
		gQueryManager.Close();
		return 0;
	}

	gQueryManager.Close();

	lpGuildInfo->Score = Score;

	return 1;
}

#if (GUILDBOSSEVENT  == 1)
BYTE CGuildManager::SetGuildScore1(const char* szGuildName, DWORD Score1)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(szGuildName);

	if (lpGuildInfo == nullptr)
	{
		return 0;
	}

	if (!gQueryManager.ExecQuery(
		"UPDATE Guild SET G_Score1=%d WHERE G_Name='%s'",
		Score1,
		szGuildName))
	{
		gQueryManager.Close();
		return 0;
	}

	gQueryManager.Close();

	lpGuildInfo->Score1 = Score1;

	return 1;
}
#endif

BYTE CGuildManager::SetGuildNotice(const char* szGuildName, const char* szNotice)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(szGuildName);

	if (lpGuildInfo == nullptr)
	{
		return 0;
	}

	gQueryManager.BindParameterAsString(1, (void*)szNotice, sizeof(lpGuildInfo->Notice));

	if (!gQueryManager.ExecQuery(
		"UPDATE Guild SET G_Notice=? WHERE G_Name='%s'",
		szGuildName))
	{
		gQueryManager.Close();
		return 0;
	}

	gQueryManager.Close();

	strncpy_s(
		lpGuildInfo->Notice,
		sizeof(lpGuildInfo->Notice),
		szNotice,
		_TRUNCATE);

	return 1;
}

BYTE CGuildManager::SetGuildType(const char* szGuildName, BYTE btType)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(szGuildName);

	if (lpGuildInfo == nullptr)
	{
		return 0;
	}

	if (!gQueryManager.ExecQuery(
		"UPDATE Guild SET G_Type=%d WHERE G_Name='%s'",
		btType,
		szGuildName))
	{
		gQueryManager.Close();
		return 0;
	}

	gQueryManager.Close();

	lpGuildInfo->Type = btType;

	return 1;
}

BYTE CGuildManager::SetGuildMemberStatus(const char* szGuildMember, BYTE btStatus)
{
	GUILD_MEMBER_INFO* lpGuildMemberInfo = GetGuildMemberInfo(szGuildMember);

	if (lpGuildMemberInfo == nullptr)
	{
		return 0;
	}

	if (!gQueryManager.ExecQuery(
		"UPDATE GuildMember SET G_Status=%d WHERE Name='%s'",
		btStatus,
		szGuildMember))
	{
		gQueryManager.Close();
		return 0;
	}

	gQueryManager.Close();

	lpGuildMemberInfo->Status = btStatus;

	return 1;
}

long CGuildManager::GetUnionList(DWORD UnionNumber, DWORD* lpUnionList)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(UnionNumber);

	if (lpGuildInfo == nullptr || lpGuildInfo->UnionNumber == 0)
	{
		return 0;
	}

	long count = 0;

	lpUnionList[count++] = lpGuildInfo->Number;

	for (const auto& guild : vGuildList)
	{
		if (guild.Number != lpGuildInfo->Number &&
			guild.UnionNumber == lpGuildInfo->Number)
		{
			lpUnionList[count++] = guild.Number;

			if (count >= MAX_GUILD_UNION)
			{
				break;
			}
		}
	}

	return count;
}

long CGuildManager::GetRivalList(DWORD RivalNumber, DWORD* lpRivalList)
{
	GUILD_INFO* lpGuildInfo = GetGuildInfo(RivalNumber);

	if (lpGuildInfo == nullptr || lpGuildInfo->RivalNumber == 0)
	{
		return 0;
	}

	lpRivalList[0] = lpGuildInfo->RivalNumber;

	return 1;
}
