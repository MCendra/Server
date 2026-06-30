// GuildManager.h
#pragma once
#include <vector>
#include <unordered_map>

#define MAX_GUILD_UNION 5
#define MAX_GUILD_RIVAL 2
#define MAX_GUILD_MEMBER 80

struct GUILD_MEMBER_INFO
{
	void Clear()
	{
		szGuildMember[0] = '\0';
		btStatus = 0;
		btServer = 0xFFFF;
	}

	bool IsEmpty() const
	{
		return (szGuildMember[0] == '\0');
	}

	char szGuildMember[11];
	BYTE btStatus;
	WORD btServer;
};

struct GUILD_INFO
{
	void Clear()
	{
		dwNumber = 0;
		dwUnionNumber = 0;
		dwRivalNumber = 0;
		dwScore = 0;

#if (NEWBOSSGUILD == 1)
		dwScore1 = 0;
#endif

		btType = 0;

		szName[0] = '\0';
		szMaster[0] = '\0';
		szNotice[0] = '\0';

		memset(arMark, 0, sizeof(arMark));

		for (auto& member : arGuildMember)
		{
			member.Clear();
		}
	}

	BYTE GetMemberCount() const
	{
		BYTE count = 0;

		for (const auto& member : arGuildMember)
		{
			if (!member.IsEmpty())
			{
				++count;
			}
		}

		return count;
	}

	DWORD dwNumber;
	DWORD dwUnionNumber;
	DWORD dwRivalNumber;
	DWORD dwScore;

#if (NEWBOSSGUILD == 1)
	DWORD dwScore1;
#endif

	BYTE btType;
	char szName[9];
	char szMaster[11];
	char szNotice[60];
	BYTE arMark[32];
	GUILD_MEMBER_INFO arGuildMember[MAX_GUILD_MEMBER];
};

class CGuildManager
{
public:
	CGuildManager() = default;
	~CGuildManager() = default;

	void Init();

	GUILD_INFO* GetGuildInfo(const char* szName);
	GUILD_INFO* GetGuildInfo(DWORD dwNumber);
	GUILD_INFO* GetMemberGuildInfo(const char* szGuildMember);
	GUILD_MEMBER_INFO* GetGuildMemberInfo(const char* szGuildMember);

	BOOL CheckGuildOnCS(const char* szGuildName);

	void ConnectMember(const char* szGuildMember, WORD btServer);
	void DisconnectMember(const char* szGuildMember);

	BYTE AddGuild(int index, const char* szGuildName, const char* szMasterName, BYTE* lpMark, BYTE btType);
	BYTE DelGuild(int index, const char* szGuildName);

	BYTE AddGuildMember(int index, const char* szGuildName, const char* szGuildMember, BYTE btStatus, WORD btServer);
	BYTE DelGuildMember(int index, const char* szGuildMember);

	BYTE AddGuildRelationship(int index, DWORD dwSourceGuild, DWORD dwTargetGuild, BYTE btRelationType);
	BYTE DelGuildRelationship(int index, DWORD dwSourceGuild, BYTE btRelationType);

	BYTE SetGuildRelationship(int index, const char* szGuildMember, const char* szGuildMaster);

	BYTE SetGuildScore(const char* szGuildName, DWORD dwScore);

#if (NEWBOSSGUILD == 1)
	BYTE SetGuildScore1(const char* szGuildName, DWORD dwScore1);
#endif

	BYTE SetGuildNotice(const char* szGuildName, const char* szNotice);
	BYTE SetGuildType(const char* szGuildName, BYTE btType);
	BYTE SetGuildMemberStatus(const char* szGuildMember, BYTE btStatus);

	long GetUnionList(DWORD dwUnionNumber, DWORD* lpUnionList);
	long GetRivalList(DWORD dwRivalNumber, DWORD* lpRivalList);

private:
	std::deque<GUILD_INFO> vGuildList;

	std::unordered_map<std::string, GUILD_INFO*> m_GuildByName;
	std::unordered_map<DWORD, GUILD_INFO*> m_GuildByNumber;
	std::unordered_map<std::string, GUILD_MEMBER_INFO*> m_GuildMembers;
};

extern CGuildManager gGuildManager;