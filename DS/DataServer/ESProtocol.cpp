// ESProtocol.cpp
#include "Header.h"
#include "ESProtocol.h"
#include "Log.h"
#include "CharacterManager.h"
#include "DataServer.h"
#include "GuildManager.h"
#include "ServerManager.h"
#include "SocketManager.h"
#include "Util.h"

void ExDBServerProtocolCore(int index, BYTE head, const BYTE* lpMsg, int size)
{

	if (lpMsg == nullptr || size <= 0)
	{
		return;
	}

	if (AdvancedLog != 0)
	{
		Log.ToDisp(LOG_BLACK, "ESPROTOCOL: Head: %x, 1: %x, 2: %x, 3: %x, 4: %x", head, lpMsg[1], lpMsg[2], lpMsg[3], lpMsg[4]);
	}

	switch (head)
	{
	case EXDB_HEAD_CHAR_CLOSE:				GDCharClose((SDHP_USERCLOSE*)lpMsg, index);								break;
	case EXDB_HEAD_GUILD_CREATE:			GDGuildCreateSend((SDHP_GUILDCREATE*)lpMsg, index);						break;
	case EXDB_HEAD_GUILD_DESTROY:			GDGuildDestroySend((SDHP_GUILDDESTROY*)lpMsg, index);					break;
	case EXDB_HEAD_GUILD_MEMBER_ADD:		GDGuildMemberAdd((SDHP_GUILDMEMBERADD*)lpMsg, index);					break;
	case EXDB_HEAD_GUILD_MEMBER_DEL:		GDGuildMemberDel((SDHP_GUILDMEMBERDEL*)lpMsg, index);					break;
	case EXDB_HEAD_GUILD_MEMBER_INFO:		DGGuildMemberInfoRequest((SDHP_GUILDMEMBER_INFO_REQUEST*)lpMsg, index);	break;
	case EXDB_HEAD_GUILD_SCORE_UPDATE:		DGGuildScoreUpdate((SDHP_GUILDSCOREUPDATE*)lpMsg, index);				break;
	case EXDB_HEAD_GUILD_NOTICE:			GDGuildNoticeSave((SDHP_GUILDNOTICE*)lpMsg, index);						break;
#if(GUILDBOSSEVENT  == 1)
	case EXDB_HEAD_GUILD_SCORE_UPDATE_EX:	DGGuildScoreUpdate1((SDHP_GUILDSCOREUPDATE1*)lpMsg, index);				break;
#endif
	case EXDB_HEAD_GUILD_CHAT:				GDGuildServerGroupChattingSend((EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND*)lpMsg, index);	break;
	case EXDB_HEAD_UNION_CHAT:				GDUnionServerGroupChattingSend((EXSDHP_SERVERGROUP_UNION_CHATTING_SEND*)lpMsg, index);	break;
	case EXDB_HEAD_ASSIGN_STATUS:			GDGuildReqAssignStatus((EXSDHP_GUILD_ASSIGN_STATUS_REQ*)lpMsg, index);	break;
	case EXDB_HEAD_ASSIGN_TYPE:				GDGuildReqAssignType((EXSDHP_GUILD_ASSIGN_TYPE_REQ*)lpMsg, index);		break;
	case EXDB_HEAD_RELATIONSHIP_JOIN:		GDRelationShipReqJoin((EXSDHP_RELATIONSHIP_JOIN_REQ*)lpMsg, index);		break;
	case EXDB_HEAD_RELATIONSHIP_BREAK:		GDUnionBreakOff((EXSDHP_RELATIONSHIP_BREAKOFF_REQ*)lpMsg, index);		break;
	case EXDB_HEAD_UNION_LIST:				GDUnionListSend((EXSDHP_UNION_LIST_REQ*)lpMsg, index);					break;
	case EXDB_HEAD_KICKOUT_UNION_MEMBER:
		switch (lpMsg[3])
		{
		case EXDB_SUB_HEAD_KICKOUT_UNION_MEMBER:	GDRelationShipReqKickOutUnionMember((EXSDHP_KICKOUT_UNIONMEMBER_REQ*)lpMsg, index);	break;
		}
		break;
	}

}

void ESDataSend(int index, const BYTE* lpMsg, int size)
{
	if (lpMsg == nullptr || size <= 0)
	{
		return;
	}

	BYTE send[8192];

	switch (lpMsg[PACKET_TYPE_OFFSET])
	{
	case PACKET_C1:
	{
		const int packetSize = size + 1;

		if (packetSize > static_cast<int>(sizeof(send)))
		{
			return;
		}

		PSBMSG_HEAD Header;

		Header.set(
			HEAD_EXDB_PROTOCOL,
			lpMsg[C1_PACKET_HEAD_OFFSET],
			packetSize);

		memcpy(send, &Header, sizeof(Header));

		memcpy(
			send + sizeof(Header),
			lpMsg + C1_PACKET_DATA_OFFSET,
			packetSize - static_cast<int>(sizeof(Header)));

		gSocketManager.DataSend(index, send, packetSize);
	}
	break;

	case PACKET_C2:
	{
		const int packetSize = size + 1;

		if (packetSize > static_cast<int>(sizeof(send)))
		{
			return;
		}

		PSWMSG_HEAD Header;

		Header.set(
			HEAD_EXDB_PROTOCOL,
			lpMsg[C2_PACKET_HEAD_OFFSET],
			packetSize);

		memcpy(send, &Header, sizeof(Header));

		memcpy(
			send + sizeof(Header),
			lpMsg + C2_PACKET_DATA_OFFSET,
			packetSize - static_cast<int>(sizeof(Header)));

		gSocketManager.DataSend(index, send, packetSize);
	}
	break;
	}
}

void ESDataRecv(int index, BYTE head, const BYTE* lpMsg, int size)
{
	UNREFERENCED_PARAMETER(head);

	if (lpMsg == nullptr || size <= 0)
	{
		return;
	}

	BYTE recv[8192];

	switch (lpMsg[PACKET_TYPE_OFFSET])
	{
	case PACKET_C1:
	{
		const int packetSize = size - 1;

		if (packetSize <= 0 || packetSize > static_cast<int>(sizeof(recv)))
		{
			return;
		}

		PBMSG_HEAD Header;

		Header.set(
			lpMsg[C1_PACKET_DATA_OFFSET],
			packetSize);

		memcpy(recv, &Header, sizeof(Header));

		memcpy(
			recv + sizeof(Header),
			lpMsg + C1_PACKET_DATA_OFFSET + 1,
			packetSize - static_cast<int>(sizeof(Header)));

		ExDBServerProtocolCore(index, Header.head, recv, packetSize);
	}
	break;

	case PACKET_C2:
	{
		const int packetSize = size - 1;

		if (packetSize <= 0 || packetSize > static_cast<int>(sizeof(recv)))
		{
			return;
		}

		PWMSG_HEAD Header;

		Header.set(
			lpMsg[C2_PACKET_DATA_OFFSET],
			packetSize);

		memcpy(recv, &Header, sizeof(Header));

		memcpy(
			recv + sizeof(Header),
			lpMsg + C2_PACKET_DATA_OFFSET + 1,
			packetSize - static_cast<int>(sizeof(Header)));

		ExDBServerProtocolCore(index, Header.head, recv, packetSize);
	}
	break;
	}
}

void GDCharClose(const SDHP_USERCLOSE* lpMsg,int index)
{
	UNREFERENCED_PARAMETER(index);

	SDHP_USERCLOSE pMsg{};

	pMsg.Header.set(EXDB_HEAD_CHAR_CLOSE, sizeof(pMsg));

	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));

	pMsg.Type = 0;

	for (int serverIndex = 0; serverIndex < MAX_SERVER; ++serverIndex)
	{
		if (!gServerManager[serverIndex].IsOnline())
		{
			continue;
		}

		ESDataSend(serverIndex, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void GDGuildCreateSend(const SDHP_GUILDCREATE* lpMsg, int index)
{
	SDHP_GUILDCREATE_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_CREATE, sizeof(pMsg));

	pMsg.Result = 0;
	pMsg.Flag = 1;
	pMsg.GuildNumber = 0;

	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;
	pMsg.GuildType = lpMsg->GuildType;

	memcpy(pMsg.Master, lpMsg->Master, sizeof(pMsg.Master));
	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));
	memcpy(pMsg.Mark, lpMsg->Mark, sizeof(pMsg.Mark));

	pMsg.Result = gGuildManager.AddGuild(
		index,
		lpMsg->GuildName,
		lpMsg->Master,
		lpMsg->Mark,
		lpMsg->GuildType);

	if (pMsg.Result != 1)
	{
		ESDataSend(index, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	GUILD_INFO* const lpGuildInfo = gGuildManager.GetGuildInfo(lpMsg->GuildName);
	GUILD_MEMBER_INFO* const lpGuildMemberInfo = gGuildManager.GetGuildMemberInfo(lpMsg->Master);

	if (lpGuildInfo == nullptr || lpGuildMemberInfo == nullptr)
	{
		return;
	}

	pMsg.GuildNumber = lpGuildInfo->Number;

	for (int serverIndex = 0; serverIndex < MAX_SERVER; ++serverIndex)
	{
		if (!gServerManager[serverIndex].IsOnline())
		{
			continue;
		}

		pMsg.Flag = (serverIndex == index) ? 1 : 0;

		ESDataSend(
			serverIndex,
			reinterpret_cast<const BYTE*>(&pMsg),
			sizeof(pMsg));
	}

	gGuildManager.ConnectMember(lpMsg->Master, GetServerCodeByName(lpMsg->Master));

	DGGuildMasterListRecv(index, lpGuildInfo->Number);

	DGRelationShipListRecv(index, lpGuildInfo->UnionNumber, 1);
	DGRelationShipListRecv(index, lpGuildInfo->RivalNumber, 2);

	DGGuildMemberInfo(
		index,
		lpMsg->GuildName,
		lpMsg->Master,
		lpGuildMemberInfo->Status,
		lpMsg->GuildType,
		static_cast<BYTE>(lpGuildMemberInfo->Server));
}

void GDGuildDestroySend(const SDHP_GUILDDESTROY* lpMsg, int index)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SDHP_GUILDDESTROY_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_DESTROY, sizeof(pMsg));

	pMsg.Result = 0;
	pMsg.Flag = 1;

	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;

	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));
	memcpy(pMsg.Master, lpMsg->Master, sizeof(pMsg.Master));

	pMsg.Result = gGuildManager.DelGuild(index, lpMsg->GuildName);

	if (pMsg.Result != 1)
	{
		ESDataSend(index, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	for (int serverIndex = 0; serverIndex < MAX_SERVER; ++serverIndex)
	{
		if (!gServerManager[serverIndex].IsOnline())
		{
			continue;
		}

		pMsg.Flag = (serverIndex == index) ? 1 : 0;

		ESDataSend(
			serverIndex,
			reinterpret_cast<const BYTE*>(&pMsg),
			sizeof(pMsg));
	}
}

void GDGuildMemberAdd(const SDHP_GUILDMEMBERADD* lpMsg, int index)
{
	SDHP_GUILDMEMBERADD_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_MEMBER_ADD, sizeof(pMsg));

	pMsg.Result = 0;
	pMsg.Flag = 1;

	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;

	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));
	memcpy(pMsg.MemberID, lpMsg->MemberID, sizeof(pMsg.MemberID));

	pMsg.Server = GetServerCodeByName(lpMsg->MemberID);

	pMsg.Result = gGuildManager.AddGuildMember(
		index,
		lpMsg->GuildName,
		lpMsg->MemberID,
		0x00,
		pMsg.Server);

	if (pMsg.Result != 1)
	{
		ESDataSend(index, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	for (int serverIndex = 0; serverIndex < MAX_SERVER; ++serverIndex)
	{
		if (!gServerManager[serverIndex].IsOnline())
		{
			continue;
		}

		pMsg.Flag = (serverIndex == index) ? 1 : 0;

		ESDataSend(
			serverIndex,
			reinterpret_cast<const BYTE*>(&pMsg),
			sizeof(pMsg));
	}
}

void GDGuildMemberDel(const SDHP_GUILDMEMBERDEL* lpMsg, int index)
{
	SDHP_GUILDMEMBERDEL_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_MEMBER_DEL, sizeof(pMsg));

	pMsg.Result = 0;
	pMsg.Flag = 1;

	pMsg.NumberH = lpMsg->NumberH;
	pMsg.NumberL = lpMsg->NumberL;

	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));
	memcpy(pMsg.MemberID, lpMsg->MemberID, sizeof(pMsg.MemberID));

	if ((pMsg.Result = gGuildManager.DelGuildMember(index, lpMsg->MemberID)) != 1)
	{
		ESDataSend(index, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		pMsg.Flag = (n == index);

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void DGGuildMemberInfoRequest(const SDHP_GUILDMEMBER_INFO_REQUEST* lpMsg, int index)
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetMemberGuildInfo(lpMsg->MemberID);
	GUILD_MEMBER_INFO* lpGuildMemberInfo = gGuildManager.GetGuildMemberInfo(lpMsg->MemberID);

	if (lpGuildInfo == nullptr || lpGuildMemberInfo == nullptr)
	{
		return;
	}

	gGuildManager.ConnectMember(lpMsg->MemberID, GetServerCodeByName(lpMsg->MemberID));

	DGGuildMasterListRecv(index, lpGuildInfo->Number);

	DGRelationShipListRecv(index, lpGuildInfo->UnionNumber, 1);

	DGRelationShipListRecv(index, lpGuildInfo->RivalNumber, 2);

	DGGuildMemberInfo(
		index,
		lpGuildInfo->Name,
		lpMsg->MemberID,
		lpGuildMemberInfo->Status,
		lpGuildInfo->Type,
		static_cast<BYTE>(lpGuildMemberInfo->Server));
}

void DGGuildScoreUpdate(const SDHP_GUILDSCOREUPDATE* lpMsg, int index)
{
	if (gGuildManager.SetGuildScore(lpMsg->GuildName, lpMsg->Score) == 0)
	{
		return;
	}

	SDHP_GUILDSCOREUPDATE pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_SCORE_UPDATE, sizeof(pMsg));

	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));

	pMsg.Score = lpMsg->Score;

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

#if (GUILDBOSSEVENT  == 1)

void DGGuildScoreUpdate1(const SDHP_GUILDSCOREUPDATE1* lpMsg, int index)
{
	if (gGuildManager.SetGuildScore1(lpMsg->GuildName, lpMsg->Score1) == 0)
	{
		return;
	}

	SDHP_GUILDSCOREUPDATE1 pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_SCORE_UPDATE_EX, sizeof(pMsg));

	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));

	pMsg.Score1 = lpMsg->Score1;

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

#endif

void GDGuildNoticeSave(const SDHP_GUILDNOTICE* lpMsg, int index)
{
	if (gGuildManager.SetGuildNotice(lpMsg->GuildName, lpMsg->GuildNotice) == 0)
	{
		return;
	}

	SDHP_GUILDNOTICE pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_NOTICE, sizeof(pMsg));

	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));
	memcpy(pMsg.GuildNotice, lpMsg->GuildNotice, sizeof(pMsg.GuildNotice));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void GDGuildServerGroupChattingSend(const EXSDHP_SERVERGROUP_GUILD_CHATTING_SEND* lpMsg, int index)
{
	EXSDHP_SERVERGROUP_GUILD_CHATTING_RECV pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_CHAT, sizeof(pMsg));

	pMsg.GuildNum = lpMsg->GuildNum;

	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	memcpy(pMsg.ChattingMsg, lpMsg->ChattingMsg, sizeof(pMsg.ChattingMsg));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void GDUnionServerGroupChattingSend(const EXSDHP_SERVERGROUP_UNION_CHATTING_SEND* lpMsg, int index)
{
	EXSDHP_SERVERGROUP_UNION_CHATTING_RECV pMsg{};

	pMsg.Header.set(EXDB_HEAD_UNION_CHAT, sizeof(pMsg));

	pMsg.UnionNum = lpMsg->UnionNum;

	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	memcpy(pMsg.ChattingMsg, lpMsg->ChattingMsg, sizeof(pMsg.ChattingMsg));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void GDGuildReqAssignStatus(const EXSDHP_GUILD_ASSIGN_STATUS_REQ* lpMsg, int index)
{
	EXSDHP_GUILD_ASSIGN_STATUS_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_ASSIGN_STATUS, sizeof(pMsg));

	pMsg.Flag = 1;
	pMsg.UserIndex = lpMsg->UserIndex;
	pMsg.Type = lpMsg->Type;

	pMsg.Result = gGuildManager.SetGuildMemberStatus(lpMsg->TargetName, lpMsg->GuildStatus);

	pMsg.GuildStatus = lpMsg->GuildStatus;

	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));
	memcpy(pMsg.TargetName, lpMsg->TargetName, sizeof(pMsg.TargetName));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		pMsg.Flag = (n == index);

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void GDGuildReqAssignType(const EXSDHP_GUILD_ASSIGN_TYPE_REQ* lpMsg, int index)
{
	EXSDHP_GUILD_ASSIGN_TYPE_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_ASSIGN_TYPE, sizeof(pMsg));

	pMsg.Flag = 1;
	pMsg.UserIndex = lpMsg->UserIndex;
	pMsg.GuildType = lpMsg->GuildType;

	pMsg.Result = gGuildManager.SetGuildType(lpMsg->GuildName, lpMsg->GuildType);

	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		pMsg.Flag = (n == index);

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void GDRelationShipReqJoin(const EXSDHP_RELATIONSHIP_JOIN_REQ* lpMsg, int index)
{
	EXSDHP_RELATIONSHIP_JOIN_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_RELATIONSHIP_JOIN, sizeof(pMsg));

	pMsg.Flag = 1;

	pMsg.RequestUserIndex = lpMsg->RequestUserIndex;
	pMsg.TargetUserIndex = lpMsg->TargetUserIndex;

	pMsg.Result = 0;

	pMsg.RelationShipType = lpMsg->RelationShipType;

	pMsg.RequestGuildNum = lpMsg->RequestGuildNum;
	pMsg.TargetGuildNum = lpMsg->TargetGuildNum;

	GUILD_INFO* lpSourceGuildInfo = gGuildManager.GetGuildInfo(lpMsg->RequestGuildNum);
	GUILD_INFO* lpTargetGuildInfo = gGuildManager.GetGuildInfo(lpMsg->TargetGuildNum);

	if (lpSourceGuildInfo == nullptr || lpTargetGuildInfo == nullptr)
	{
		ESDataSend(index, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	if ((pMsg.Result = gGuildManager.AddGuildRelationship(
		index,
		lpMsg->RequestGuildNum,
		lpMsg->TargetGuildNum,
		lpMsg->RelationShipType)) != 1)
	{
		ESDataSend(index, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	memcpy(pMsg.RequestGuildName, lpSourceGuildInfo->Name, sizeof(pMsg.RequestGuildName));
	memcpy(pMsg.TargetGuildName, lpTargetGuildInfo->Name, sizeof(pMsg.TargetGuildName));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		pMsg.Flag = (n == index);

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void GDUnionBreakOff(const EXSDHP_RELATIONSHIP_BREAKOFF_REQ* lpMsg, int index)
{
	EXSDHP_RELATIONSHIP_BREAKOFF_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_RELATIONSHIP_BREAK, sizeof(pMsg));

	pMsg.Flag = 1;

	pMsg.RequestUserIndex = lpMsg->RequestUserIndex;
	pMsg.TargetUserIndex = lpMsg->TargetUserIndex;

	pMsg.Result = 0;

	pMsg.RelationShipType = lpMsg->RelationShipType;

	pMsg.RequestGuildNum = lpMsg->RequestGuildNum;
	pMsg.TargetGuildNum = lpMsg->TargetGuildNum;

	if ((pMsg.Result = gGuildManager.DelGuildRelationship(
		index,
		lpMsg->RequestGuildNum,
		lpMsg->RelationShipType)) != 1)
	{
		ESDataSend(index, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		pMsg.Flag = (n == index);

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void GDUnionListSend(const EXSDHP_UNION_LIST_REQ* lpMsg, int index)
{
	BYTE send[2048]{};

	EXSDHP_UNION_LIST_COUNT pMsg{};

	int size = sizeof(pMsg);

	pMsg.RequestUserIndex = lpMsg->RequestUserIndex;

	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(lpMsg->UnionMasterGuildNumber);

	if (lpGuildInfo != nullptr)
	{
		pMsg.Result = 1;

		EXSDHP_UNION_LIST unionList{};

		std::array<DWORD, MAX_GUILD_UNION> guildUnionNumber{};
		std::array<DWORD, MAX_GUILD_RIVAL> guildRivalNumber{};

		pMsg.UnionMemberNum = static_cast<BYTE>(
			gGuildManager.GetUnionList(lpGuildInfo->UnionNumber, guildUnionNumber.data()));

		pMsg.RivalMemberNum = static_cast<BYTE>(
			gGuildManager.GetRivalList(lpGuildInfo->RivalNumber, guildRivalNumber.data()));

		constexpr int UnionListSize = sizeof(EXSDHP_UNION_LIST);

		for (int n = 0; n < pMsg.UnionMemberNum; ++n)
		{
			GUILD_INFO* lpGuildList = gGuildManager.GetGuildInfo(guildUnionNumber[n]);

			if (lpGuildList == nullptr)
			{
				continue;
			}

			if ((size + UnionListSize) > static_cast<int>(sizeof(send)))
			{
				break;
			}

			unionList.MemberNum = lpGuildList->GetMemberCount();

			memcpy(unionList.GuildName, lpGuildList->Name, sizeof(unionList.GuildName));
			memcpy(unionList.Mark, lpGuildList->Mark, sizeof(unionList.Mark));

			memcpy(send + size, &unionList, UnionListSize);

			size += UnionListSize;

			++pMsg.Count;
		}
	}

	pMsg.Header.set(EXDB_HEAD_UNION_LIST, size);

	memcpy(send, &pMsg, sizeof(pMsg));

	ESDataSend(index, send, size);
}

void GDRelationShipReqKickOutUnionMember(const EXSDHP_KICKOUT_UNIONMEMBER_REQ* lpMsg, int index)
{
	EXSDHP_KICKOUT_UNIONMEMBER_RESULT pMsg{};

	pMsg.Header.set(EXDB_HEAD_KICKOUT_UNION_MEMBER, EXDB_SUB_HEAD_KICKOUT_UNION_MEMBER, sizeof(pMsg));

	pMsg.Flag = 1;

	pMsg.RequestUserIndex = lpMsg->RequestUserIndex;
	pMsg.RelationShipType = lpMsg->RelationShipType;

	pMsg.Result = 0;

	memcpy(pMsg.UnionMasterGuildName, lpMsg->UnionMasterGuildName, sizeof(pMsg.UnionMasterGuildName));
	memcpy(pMsg.UnionMemberGuildName, lpMsg->UnionMemberGuildName, sizeof(pMsg.UnionMemberGuildName));

	if ((pMsg.Result = gGuildManager.SetGuildRelationship(
		index,
		lpMsg->UnionMemberGuildName,
		lpMsg->UnionMasterGuildName)) != 1)
	{
		ESDataSend(index, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() == 0)
		{
			continue;
		}

		pMsg.Flag = (n == index);

		ESDataSend(n, reinterpret_cast<const BYTE*>(&pMsg), sizeof(pMsg));
	}
}

void DGGuildMemberInfo(int index, const char* guildName, const char* memberID, BYTE status, BYTE type, BYTE server)
{
	SDHP_GUILDMEMBER_INFO pMsg{};

	pMsg.Header.set(EXDB_HEAD_GUILD_MEMBER_INFO, sizeof(pMsg));

	memcpy(pMsg.GuildName, guildName, sizeof(pMsg.GuildName));
	memcpy(pMsg.MemberID, memberID, sizeof(pMsg.MemberID));

	pMsg.GuildStatus = status;
	pMsg.GuildType = type;
	pMsg.Server = server;

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			ESDataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void DGGuildMasterListRecv(int index, int guildNumber)
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(guildNumber);

	if (lpGuildInfo == nullptr)
	{
		return;
	}

	BYTE send[2048]{};

	SDHP_GUILDALL_COUNT pMsg{};

	int size = sizeof(pMsg);

	pMsg.Number = lpGuildInfo->Number;

	memcpy(pMsg.GuildName, lpGuildInfo->Name, sizeof(pMsg.GuildName));
	memcpy(pMsg.Master, lpGuildInfo->Master, sizeof(pMsg.Master));
	memcpy(pMsg.Mark, lpGuildInfo->Mark, sizeof(pMsg.Mark));

	pMsg.Score = lpGuildInfo->Score;
	pMsg.GuildType = lpGuildInfo->Type;
	pMsg.GuildUnion = lpGuildInfo->UnionNumber;
	pMsg.GuildRival = lpGuildInfo->RivalNumber;

	GUILD_INFO* lpRivalInfo = gGuildManager.GetGuildInfo(lpGuildInfo->RivalNumber);

	if (lpRivalInfo != nullptr)
	{
		memcpy(pMsg.GuildRivalName, lpRivalInfo->Name, sizeof(pMsg.GuildRivalName));
	}

	SDHP_GUILDALL guildList{};

	constexpr int GuildListSize = sizeof(SDHP_GUILDALL);

	for (int n = 0; n < MAX_GUILD_MEMBER; ++n)
	{
		GUILD_MEMBER_INFO* lpGuildMemberInfo = &lpGuildInfo->GuildMember[n];

		if (lpGuildMemberInfo->IsEmpty() != 0)
		{
			continue;
		}

		if ((size + GuildListSize) > static_cast<int>(sizeof(send)))
		{
			break;
		}

		memcpy(guildList.MemberID, lpGuildMemberInfo->GuildMember, sizeof(guildList.MemberID));

		guildList.GuildStatus = lpGuildMemberInfo->Status;
		guildList.Server = lpGuildMemberInfo->Server;

		memcpy(send + size, &guildList, GuildListSize);

		size += GuildListSize;

		++pMsg.Count;
	}

	pMsg.Header.set(EXDB_HEAD_GUILD_MASTER_LIST, size);

	memcpy(send, &pMsg, sizeof(pMsg));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			ESDataSend(n, send, size);
		}
	}
}

void DGRelationShipListRecv(int index, int guildNumber, int relationshipType)
{
	GUILD_INFO* lpGuildInfo = gGuildManager.GetGuildInfo(guildNumber);

	if (lpGuildInfo == nullptr)
	{
		return;
	}

	EXSDHP_UNION_RELATIONSHIP_LIST pMsg{};

	pMsg.Header.set(EXDB_HEAD_RELATIONSHIP_LIST, sizeof(pMsg));

	pMsg.Flag = 1;
	pMsg.RelationShipType = static_cast<BYTE>(relationshipType);

	switch (relationshipType)
	{
	case 1:
		pMsg.RelationShipMemberCount = static_cast<BYTE>(
			gGuildManager.GetUnionList(
				lpGuildInfo->Number,
				reinterpret_cast<DWORD*>(pMsg.RelationShipMember)));
		break;

	case 2:
		pMsg.RelationShipMemberCount = static_cast<BYTE>(
			gGuildManager.GetRivalList(
				lpGuildInfo->Number,
				reinterpret_cast<DWORD*>(pMsg.RelationShipMember)));
		break;
	}

	memcpy(
		pMsg.UnionMasterGuildName,
		lpGuildInfo->Name,
		sizeof(pMsg.UnionMasterGuildName));

	pMsg.UnionMasterGuildNumber = lpGuildInfo->Number;

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			pMsg.Flag = (n == index) ? 1 : 0;

			ESDataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void DGRelationShipNotificationRecv(int index, int updateFlag, int guildListCount, const int* guildList)
{
	EXSDHP_NOTIFICATION_RELATIONSHIP pMsg{};

	pMsg.Header.set(EXDB_HEAD_RELATIONSHIP_NOTIFICATION, sizeof(pMsg));

	pMsg.Flag = 1;
	pMsg.UpdateFlag = static_cast<BYTE>(updateFlag);

	const int count = (guildListCount > 100) ? 100 : guildListCount;

	pMsg.GuildListCount = static_cast<BYTE>(count);

	memcpy(
		pMsg.GuildList,
		guildList,
		count * sizeof(int));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			pMsg.Flag = (n == index) ? 1 : 0;

			ESDataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}
