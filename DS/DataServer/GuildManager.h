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
		GuildMember[0] = '\0';
		Status = 0;
		Server = 0xFFFF;
	}

	bool IsEmpty() const
	{
		return (GuildMember[0] == '\0');
	}

	char GuildMember[11];
	BYTE Status;
	WORD Server;
};

struct GUILD_INFO
{
	void Clear()
	{
		Number = 0;
		UnionNumber = 0;
		RivalNumber = 0;
		Score = 0;

#if (NEWBOSSGUILD == 1)
		Score1 = 0;
#endif

		Type = 0;

		Name[0] = '\0';
		Master[0] = '\0';
		Notice[0] = '\0';

		memset(Mark, 0, sizeof(Mark));

		for (auto& member : GuildMember)
		{
			member.Clear();
		}
	}

	BYTE GetMemberCount() const
	{
		BYTE count = 0;

		for (const auto& member : GuildMember)
		{
			if (!member.IsEmpty())
			{
				++count;
			}
		}

		return count;
	}

	DWORD Number;
	DWORD UnionNumber;
	DWORD RivalNumber;
	DWORD Score;

#if (NEWBOSSGUILD == 1)
	DWORD Score1;
#endif

	BYTE Type;
	char Name[9];
	char Master[11];
	char Notice[60];
	BYTE Mark[32];
	GUILD_MEMBER_INFO GuildMember[MAX_GUILD_MEMBER];
};

class CGuildManager
{
public:
	CGuildManager() = default;
	~CGuildManager() = default;

	void Init();

	GUILD_INFO* GetGuildInfo(const char* name);
	GUILD_INFO* GetGuildInfo(DWORD number);
	GUILD_INFO* GetMemberGuildInfo(const char* guildMember);
	GUILD_MEMBER_INFO* GetGuildMemberInfo(const char* guildMember);

	BOOL CheckGuildOnCS(const char* guildName);

	void ConnectMember(const char* guildMember, WORD server);
	void DisconnectMember(const char* guildMember);

	BYTE AddGuild(int index, const char* guildName, const char* masterName, const BYTE* lpMark, BYTE type);
	BYTE DelGuild(int index, const char* guildName);

	BYTE AddGuildMember(int index, const char* guildName, const char* guildMember, BYTE status, WORD server);
	BYTE DelGuildMember(int index, const char* guildMember);

	BYTE AddGuildRelationship(int index, DWORD sourceGuild, DWORD targetGuild, BYTE relationType);
	BYTE DelGuildRelationship(int index, DWORD sourceGuild, BYTE relationType);

	BYTE SetGuildRelationship(int index, const char* guildMember, const char* guildMaster);

	BYTE SetGuildScore(const char* guildName, DWORD score);

#if (NEWBOSSGUILD == 1)
	BYTE SetGuildScore1(const char* guildName, DWORD score1);
#endif

	BYTE SetGuildNotice(const char* guildName, const char* notice);
	BYTE SetGuildType(const char* guildName, BYTE btType);
	BYTE SetGuildMemberStatus(const char* guildMember, BYTE status);

	long GetUnionList(DWORD unionNumber, DWORD* lpunionList);
	long GetRivalList(DWORD rivalNumber, DWORD* lprivalList);

private:
	std::deque<GUILD_INFO> vGuildList;

	std::unordered_map<std::string, GUILD_INFO*> m_GuildByName;
	std::unordered_map<DWORD, GUILD_INFO*> m_GuildByNumber;
	std::unordered_map<std::string, GUILD_MEMBER_INFO*> m_GuildMembers;
};

extern CGuildManager gGuildManager;