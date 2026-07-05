// ESProtocol.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer 

struct SDHP_USERCLOSE
{
	PBMSG_HEAD Header; // C1:02 [RECV/SEND]
	char CharacterName[MAX_CHARACTER_NAME];
	char GuildName[MAX_GUILD_NAME];
	BYTE Type;
};

struct SDHP_GUILDCREATE
{
	PBMSG_HEAD Header; // C1:30
	char GuildName[MAX_GUILD_NAME];
	char Master[MAX_CHARACTER_NAME];
	BYTE Mark[GUILD_MARK_SIZE];
	BYTE NumberH;
	BYTE NumberL;
	BYTE GuildType;
};

struct SDHP_GUILDDESTROY
{
	PBMSG_HEAD Header; // C1:31
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILD_NAME];
	char Master[MAX_CHARACTER_NAME];
};

struct SDHP_GUILDMEMBERADD
{
	PBMSG_HEAD Header; // C1:32
	char GuildName[MAX_GUILD_NAME];
	char MemberID[11];
	BYTE NumberH;
	BYTE NumberL;
};

struct SDHP_GUILDMEMBERDEL
{
	PBMSG_HEAD Header; // C1:33
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILD_NAME];
	char MemberID[11];
};

struct SDHP_GUILDMEMBER_INFO_REQUEST
{
	PBMSG_HEAD Header; // C1:35
	BYTE NumberH;
	BYTE NumberL;
	char MemberID[11];
};

struct SDHP_GUILDSCOREUPDATE
{
	PBMSG_HEAD Header; // C1:37 [RECV/SEND]
	char GuildName[MAX_GUILD_NAME];
	int Score;
};

#if(NEWBOSSGUILD == 1)
struct SDHP_GUILDSCOREUPDATE1
{
	PBMSG_HEAD Header; // C1:37 [RECV/SEND]
	char GuildName[MAX_GUILD_NAME];
	int Score1;
};
#endif


struct SDHP_GUILDNOTICE
{
	PBMSG_HEAD Header; // C1:38 [RECV/SEND]
	char GuildName[MAX_GUILD_NAME];
	char GuildNotice[GUILD_NOTICE_SIZE];
};

struct EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND
{
	PBMSG_HEAD Header; // C1:50
	int GuildNum;
	char CharacterName[MAX_CHARACTER_NAME];
	char ChattingMsg[MAX_MSG_CHAT_SIZE];
};

struct EXSDHP_SERVERGROUP_UNION_CHATTING_SEND
{
	PBMSG_HEAD Header; // C1:51
	int UnionNum;
	char CharacterName[MAX_CHARACTER_NAME];
	char ChattingMsg[MAX_MSG_CHAT_SIZE];
};

struct EXSDHP_GUILD_ASSIGN_STATUS_REQ
{
	PBMSG_HEAD Header; // C1:E1
	WORD UserIndex;
	BYTE Type;
	BYTE GuildStatus;
	char GuildName[MAX_GUILD_NAME];
	char TargetName[MAX_CHARACTER_NAME];
};

struct EXSDHP_GUILD_ASSIGN_TYPE_REQ
{
	PBMSG_HEAD Header; // C1:E2
	WORD UserIndex;
	BYTE GuildType;
	char GuildName[MAX_GUILD_NAME];
};

struct EXSDHP_RELATIONSHIP_JOIN_REQ
{
	PBMSG_HEAD Header; // C1:E5
	WORD RequestUserIndex;
	WORD TargetUserIndex;
	BYTE RelationShipType;
	int RequestGuildNum;
	int TargetGuildNum;
};

struct EXSDHP_RELATIONSHIP_BREAKOFF_REQ
{
	PBMSG_HEAD Header; // C1:E6
	WORD RequestUserIndex;
	WORD TargetUserIndex;
	BYTE RelationShipType;
	int RequestGuildNum;
	int TargetGuildNum;
};

struct EXSDHP_UNION_LIST_REQ
{
	PBMSG_HEAD Header;
	WORD RequestUserIndex;
	int UnionMasterGuildNumber;
};

struct EXSDHP_KICKOUT_UNIONMEMBER_REQ
{
	PSBMSG_HEAD Header; // C1:EB:01
	WORD RequestUserIndex;
	BYTE RelationShipType;
	char UnionMasterGuildName[9];
	char UnionMemberGuildName[9];
};

// DataServer -> GameServer

struct SDHP_GUILDCREATE_RESULT
{
	PBMSG_HEAD Header; // C1:30
	BYTE Result;
	BYTE Flag;
	DWORD GuildNumber;
	BYTE NumberH;
	BYTE NumberL;
	char Master[MAX_CHARACTER_NAME];
	char GuildName[MAX_GUILD_NAME];
	BYTE Mark[GUILD_MARK_SIZE];
	BYTE GuildType;
};

struct SDHP_GUILDDESTROY_RESULT
{
	PBMSG_HEAD Header; // C1:31
	BYTE Result;
	BYTE Flag;
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILD_NAME];
	char Master[MAX_CHARACTER_NAME];
};

struct SDHP_GUILDMEMBERADD_RESULT
{
	PBMSG_HEAD Header; // C1:32
	BYTE Result;
	BYTE Flag;
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILD_NAME];
	char MemberID[11];
	short Server;
};

struct SDHP_GUILDMEMBERDEL_RESULT
{
	PBMSG_HEAD Header; // C1:33
	BYTE Result;
	BYTE Flag;
	BYTE NumberH;
	BYTE NumberL;
	char GuildName[MAX_GUILD_NAME];
	char MemberID[11];
};

struct SDHP_GUILDMEMBER_INFO
{
	PBMSG_HEAD Header; // C1:35
	char GuildName[MAX_GUILD_NAME];
	char MemberID[11];
	BYTE GuildStatus;
	BYTE GuildType;
	short Server;
};

struct SDHP_GUILDALL_COUNT
{
	PWMSG_HEAD Header; // C2:36
	int Number;
	char GuildName[MAX_GUILD_NAME];
	char Master[MAX_CHARACTER_NAME];
	BYTE Mark[GUILD_MARK_SIZE];
	int Score;
	BYTE GuildType;
	int GuildUnion;
	int GuildRival;
	char GuildRivalName[9];
	BYTE Count;
};

struct SDHP_GUILDALL
{
	char MemberID[11];
	BYTE GuildStatus;
	short Server;
};

struct EXSDHP_SERVERGROUP_GUILD_CHATTING_RECV
{
	PBMSG_HEAD Header; // C1:50
	int GuildNum;
	char CharacterName[MAX_CHARACTER_NAME];
	char ChattingMsg[MAX_MSG_CHAT_SIZE];
};

struct EXSDHP_SERVERGROUP_UNION_CHATTING_RECV
{
	PBMSG_HEAD Header; // C1:51
	int UnionNum;
	char CharacterName[MAX_CHARACTER_NAME];
	char ChattingMsg[MAX_MSG_CHAT_SIZE];
};

struct EXSDHP_GUILD_ASSIGN_STATUS_RESULT
{
	PBMSG_HEAD Header; // C1:E1
	BYTE Flag;
	WORD UserIndex;
	BYTE Type;
	BYTE Result;
	BYTE GuildStatus;
	char GuildName[MAX_GUILD_NAME];
	char TargetName[MAX_CHARACTER_NAME];
};

struct EXSDHP_GUILD_ASSIGN_TYPE_RESULT
{
	PBMSG_HEAD Header; // C1:E2
	BYTE Flag;
	WORD UserIndex;
	BYTE GuildType;
	BYTE Result;
	char GuildName[MAX_GUILD_NAME];
};

struct EXSDHP_RELATIONSHIP_JOIN_RESULT
{
	PBMSG_HEAD Header; // C1:E5
	BYTE Flag;
	WORD RequestUserIndex;
	WORD TargetUserIndex;
	BYTE Result;
	BYTE RelationShipType;
	int RequestGuildNum;
	int TargetGuildNum;
	char RequestGuildName[MAX_GUILD_NAME];
	char TargetGuildName[MAX_GUILD_NAME];
};

struct EXSDHP_RELATIONSHIP_BREAKOFF_RESULT
{
	PBMSG_HEAD Header; // C1:E6
	BYTE Flag;
	WORD RequestUserIndex;
	WORD TargetUserIndex;
	BYTE Result;
	BYTE RelationShipType;
	int RequestGuildNum;
	int TargetGuildNum;
};

struct EXSDHP_UNION_RELATIONSHIP_LIST
{
	PWMSG_HEAD Header; // C1:E7
	BYTE Flag;
	BYTE RelationShipType;
	BYTE RelationShipMemberCount;
	char UnionMasterGuildName[MAX_GUILD_NAME];
	int UnionMasterGuildNumber;
	int RelationShipMember[100];
};

struct EXSDHP_NOTIFICATION_RELATIONSHIP
{
	PWMSG_HEAD Header; // C1:E8
	BYTE Flag;
	BYTE UpdateFlag;
	BYTE GuildListCount;
	int GuildList[100];
};

struct EXSDHP_UNION_LIST_COUNT
{
	PWMSG_HEAD Header; // C1:E9
	BYTE Count;
	BYTE Result;
	WORD RequestUserIndex;
	int TimeStamp;
	BYTE RivalMemberNum;
	BYTE UnionMemberNum;
};

struct EXSDHP_UNION_LIST
{
	BYTE MemberNum;
	BYTE Mark[GUILD_MARK_SIZE];
	char GuildName[MAX_GUILD_NAME];
};

struct EXSDHP_KICKOUT_UNIONMEMBER_RESULT
{
	PSBMSG_HEAD Header; // C1:EB:01
	BYTE Flag;
	WORD RequestUserIndex;
	BYTE RelationShipType;
	BYTE Result;
	char UnionMasterGuildName[MAX_GUILD_NAME];
	char UnionMemberGuildName[MAX_GUILD_NAME];
};

//**********************************************//

//public:
	void ESDataSend(int index, const BYTE* lpMsg, int size);
	void ESDataRecv(int index, BYTE head, const BYTE* lpMsg, int size);
//private:
	void ExDBServerProtocolCore(int index, BYTE head, const BYTE* lpMsg, int size);
	void GDCharClose(const SDHP_USERCLOSE* lpMsg,int index);
	void GDGuildCreateSend(const SDHP_GUILDCREATE* lpMsg, int index);
	void GDGuildDestroySend(const SDHP_GUILDDESTROY* lpMsg, int index);
	void GDGuildMemberAdd(const SDHP_GUILDMEMBERADD* lpMsg, int index);
	void GDGuildMemberDel(const SDHP_GUILDMEMBERDEL* lpMsg, int index);
	void DGGuildMemberInfoRequest(SDHP_GUILDMEMBER_INFO_REQUEST* lpMsg, int index);
	void DGGuildScoreUpdate(SDHP_GUILDSCOREUPDATE* lpMsg, int index);
	#if(NEWBOSSGUILD == 1)
	void DGGuildScoreUpdate1(SDHP_GUILDSCOREUPDATE1* lpMsg, int index);
	#endif
	void GDGuildNoticeSave(SDHP_GUILDNOTICE* lpMsg, int index);
	void GDGuildServerGroupChattingSend(EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND* lpMsg, int index);
	void GDUnionServerGroupChattingSend(EXSDHP_SERVERGROUP_UNION_CHATTING_SEND* lpMsg, int index);
	void GDGuildReqAssignStatus(EXSDHP_GUILD_ASSIGN_STATUS_REQ* lpMsg, int index);
	void GDGuildReqAssignType(EXSDHP_GUILD_ASSIGN_TYPE_REQ* lpMsg, int index);
	void GDRelationShipReqJoin(EXSDHP_RELATIONSHIP_JOIN_REQ* lpMsg, int index);
	void GDUnionBreakOff(EXSDHP_RELATIONSHIP_BREAKOFF_REQ* lpMsg, int index);
	void GDUnionListSend(EXSDHP_UNION_LIST_REQ* lpMsg, int index);
	void GDRelationShipReqKickOutUnionMember(EXSDHP_KICKOUT_UNIONMEMBER_REQ* lpMsg, int index);
	void DGGuildMemberInfo(int index, const char* guildName, const char* memberID, BYTE status, BYTE type, BYTE server);
	void DGGuildMasterListRecv(int index, int guildNumber);
	void DGRelationShipListRecv(int index, int guildNumber, int relationshipType);
	void DGRelationShipNotificationRecv(int index, int updateFlag, int guildListCount, int* guildList);
