// GuildMatching.cpp
#include "Header.h"
#include "GuildMatching.h"
#include "Log.h"
#include "CharacterManager.h"
#include "SocketManager.h"
#include "Util.h"

CGuildMatching gGuildMatching;

void CGuildMatching::GDGuildMatchingListRecv(const SDHP_GUILD_MATCHING_LIST_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_LIST_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_GUILD_MATCHING_LIST_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_LIST, 0);

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.CurPage = lpMsg->Page;
	pMsg.MaxPage = 1;

	int sendSize = sizeof(pMsg);

	pMsg.count = GenerateGuildMatchingList(
		&pMsg.CurPage,
		&pMsg.MaxPage,
		send,
		&sendSize);

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);

#endif
}

void CGuildMatching::GDGuildMatchingListSearchRecv(const SDHP_GUILD_MATCHING_LIST_SEARCH_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_LIST_SEARCH_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_GUILD_MATCHING_LIST_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_LIST_SEARCH, 0);

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.CurPage = lpMsg->Page;
	pMsg.MaxPage = 1;

	int sendSize = sizeof(pMsg);

	pMsg.count = GenerateGuildMatchingList(
		&pMsg.CurPage,
		&pMsg.MaxPage,
		lpMsg->SearchWord,
		send,
		&sendSize);

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);

#endif
}

void CGuildMatching::GDGuildMatchingInsertRecv(const SDHP_GUILD_MATCHING_INSERT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_INSERT_RECV);

	SDHP_GUILD_MATCHING_INSERT_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_INSERT, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	GUILD_MATCHING_INFO guildMatchingInfo{};

	if (GetGuildMatchingInfo(
		&guildMatchingInfo,
		lpMsg->GuildName))
	{
		pMsg.Result = 0xFFFFFFFF;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	std::memcpy(
		guildMatchingInfo.CharacterName,
		lpMsg->CharacterName,
		sizeof(guildMatchingInfo.CharacterName));

	std::memcpy(
		guildMatchingInfo.Text,
		lpMsg->Text,
		sizeof(guildMatchingInfo.Text));

	std::memcpy(
		guildMatchingInfo.GuildName,
		lpMsg->GuildName,
		sizeof(guildMatchingInfo.GuildName));

	guildMatchingInfo.GuildMemberCount = lpMsg->GuildMemberCount;
	guildMatchingInfo.GuildMasterClass = lpMsg->GuildMasterClass;
	guildMatchingInfo.InterestType = lpMsg->InterestType;
	guildMatchingInfo.LevelRange = lpMsg->LevelRange;
	guildMatchingInfo.ClassType = lpMsg->ClassType;
	guildMatchingInfo.GuildMasterLevel = lpMsg->GuildMasterLevel;
	guildMatchingInfo.BoardNumber = 0;
	guildMatchingInfo.GuildNumber = lpMsg->GuildNumber;
	guildMatchingInfo.GensType = lpMsg->GensType;

	InsertGuildMatchingInfo(guildMatchingInfo);

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CGuildMatching::GDGuildMatchingCancelRecv(const SDHP_GUILD_MATCHING_CANCEL_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_CANCEL_RECV);

	SDHP_GUILD_MATCHING_CANCEL_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_CANCEL, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.Flag = lpMsg->Flag;

	GUILD_MATCHING_INFO guildMatchingInfo{};

	if (!GetGuildMatchingInfo(
		&guildMatchingInfo,
		lpMsg->GuildName))
	{
		pMsg.Result = 0xFFFFFFFF;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	RemoveGuildMatchingInfo(guildMatchingInfo);

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

	RemoveGuildMatchingJoinInfoNotifyAll(guildMatchingInfo);

#endif
}

void CGuildMatching::ClearGuildMatchingInfo(WORD serverCode)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(m_critical);

	for (auto it = m_GuildMatchingInfo.begin(); it != m_GuildMatchingInfo.end();)
	{
		CHARACTER_INFO characterInfo{};

		if (gCharacterManager.GetCharacterInfo(&characterInfo, it->second.CharacterName) != 0)
		{
			++it;
			continue;
		}

		RemoveGuildMatchingJoinInfoNotifyAll(it->second);

		it = m_GuildMatchingInfo.erase(it);
	}

#endif
}

void CGuildMatching::GDGuildMatchingJoinInsertRecv(const SDHP_GUILD_MATCHING_JOIN_INSERT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_JOIN_INSERT_RECV);

	SDHP_GUILD_MATCHING_JOIN_INSERT_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_JOIN_INSERT, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	GUILD_MATCHING_INFO guildMatchingInfo{};
	GUILD_MATCHING_JOIN_INFO guildMatchingJoinInfo{};

	if (!GetGuildMatchingInfo(
		&guildMatchingInfo,
		lpMsg->GuildName) ||
		GetGuildMatchingJoinInfo(
			&guildMatchingJoinInfo,
			lpMsg->CharacterName))
	{
		pMsg.Result = 0xFFFFFFFF;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	std::memcpy(
		guildMatchingJoinInfo.CharacterName,
		lpMsg->CharacterName,
		sizeof(guildMatchingJoinInfo.CharacterName));

	std::memcpy(
		guildMatchingJoinInfo.GuildName,
		guildMatchingInfo.GuildName,
		sizeof(guildMatchingJoinInfo.GuildName));

	std::memcpy(
		guildMatchingJoinInfo.GuildMasterName,
		guildMatchingInfo.CharacterName,
		sizeof(guildMatchingJoinInfo.GuildMasterName));

	guildMatchingJoinInfo.Class = lpMsg->Class;
	guildMatchingJoinInfo.Level = lpMsg->Level;

	InsertGuildMatchingJoinInfo(guildMatchingJoinInfo);

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

	DGGuildMatchingNotifyMasterSend(
		guildMatchingInfo.CharacterName,
		0);

#endif
}

void CGuildMatching::GDGuildMatchingJoinCancelRecv(const SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV);

	SDHP_GUILD_MATCHING_JOIN_CANCEL_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_JOIN_CANCEL, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.Flag = lpMsg->Flag;

	GUILD_MATCHING_JOIN_INFO guildMatchingJoinInfo{};

	if (!GetGuildMatchingJoinInfo(
		&guildMatchingJoinInfo,
		lpMsg->CharacterName))
	{
		pMsg.Result = 0xFFFFFFFF;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	RemoveGuildMatchingJoinInfo(guildMatchingJoinInfo);

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CGuildMatching::GDGuildMatchingJoinAcceptRecv(const SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV);

	SDHP_GUILD_MATCHING_JOIN_ACCEPT_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_JOIN_ACCEPT, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.Type = lpMsg->Type;

	std::memcpy(
		pMsg.GuildName,
		lpMsg->GuildName,
		sizeof(pMsg.GuildName));

	std::memcpy(
		pMsg.MemberName,
		lpMsg->MemberName,
		sizeof(pMsg.MemberName));

	GUILD_MATCHING_INFO guildMatchingInfo{};
	GUILD_MATCHING_JOIN_INFO guildMatchingJoinInfo{};

	if (!GetGuildMatchingInfo(
		&guildMatchingInfo,
		lpMsg->GuildName) ||
		!GetGuildMatchingJoinInfo(
			&guildMatchingJoinInfo,
			lpMsg->MemberName))
	{
		pMsg.Result = 0xFFFFFFFF;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	CHARACTER_INFO characterInfo{};

	if (!gCharacterManager.GetCharacterInfo(
		&characterInfo,
		lpMsg->MemberName))
	{
		pMsg.Result = 1;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		RemoveGuildMatchingJoinInfo(guildMatchingJoinInfo);

		return;
	}

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

	DGGuildMatchingNotifySend(
		lpMsg->MemberName,
		lpMsg->GuildName,
		(lpMsg->Type == 0) ? 2 : 1);

#endif
}

void CGuildMatching::GDGuildMatchingJoinListRecv(const SDHP_GUILD_MATCHING_JOIN_LIST_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_JOIN_LIST_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_GUILD_MATCHING_JOIN_LIST_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_JOIN_LIST, 0);

	int sendSize = sizeof(pMsg);

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	pMsg.Count = GenerateGuildMatchingJoinList(
		lpMsg->GuildName,
		send,
		&sendSize);

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(
		serverIndex,
		send,
		sendSize);

#endif
}

void CGuildMatching::GDGuildMatchingJoinInfoRecv(const SDHP_GUILD_MATCHING_JOIN_INFO_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_JOIN_INFO_RECV);

	SDHP_GUILD_MATCHING_JOIN_INFO_SEND pMsg{};

	pMsg.Header.set(HEAD_GUILD_MATCHING, SUB_GUILD_MATCHING_JOIN_INFO, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	GUILD_MATCHING_JOIN_INFO guildMatchingJoinInfo{};

	if (!GetGuildMatchingJoinInfo(
		&guildMatchingJoinInfo,
		lpMsg->CharacterName))
	{
		pMsg.Result = 0xFFFFFFFE;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	std::memcpy(
		pMsg.GuildName,
		guildMatchingJoinInfo.GuildName,
		sizeof(pMsg.GuildName));

	std::memcpy(
		pMsg.GuildMasterName,
		guildMatchingJoinInfo.GuildMasterName,
		sizeof(pMsg.GuildMasterName));

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CGuildMatching::GDGuildMatchingInsertSaveRecv(const SDHP_GUILD_MATCHING_INSERT_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_GUILD_MATCHING_INSERT_SAVE_RECV);

	GUILD_MATCHING_INFO guildMatchingInfo{};

	if (!GetGuildMatchingInfo(
		&guildMatchingInfo,
		lpMsg->GuildName))
	{
		return;
	}

	guildMatchingInfo.GuildMemberCount = lpMsg->GuildMemberCount;
	guildMatchingInfo.GuildMasterClass = lpMsg->GuildMasterClass;
	guildMatchingInfo.GuildMasterLevel = lpMsg->GuildMasterLevel;
	guildMatchingInfo.GensType = lpMsg->GensType;

	InsertGuildMatchingInfo(guildMatchingInfo);

#endif
}

void CGuildMatching::ClearGuildMatchingJoinInfo(WORD serverCode)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(m_critical);

	for (auto it = m_GuildMatchingJoinInfo.begin();
		it != m_GuildMatchingJoinInfo.end();)
	{
		CHARACTER_INFO characterInfo{};

		if (gCharacterManager.GetCharacterInfo(
			&characterInfo,
			it->second.CharacterName))
		{
			++it;
			continue;
		}

		it = m_GuildMatchingJoinInfo.erase(it);
	}

#endif
}

bool CGuildMatching::GetGuildMatchingInfo(GUILD_MATCHING_INFO* lpGuildMatchingInfo,	const char* guildName)
{
#if (DATASERVER_UPDATE >= 801)

	if (lpGuildMatchingInfo == nullptr ||
		guildName == nullptr)
	{
		return false;
	}

	CCriticalSection::CLock lock(m_critical);

	const auto it = m_GuildMatchingInfo.find(
		NormalizeToLower(guildName));

	if (it == m_GuildMatchingInfo.end())
	{
		return false;
	}

	*lpGuildMatchingInfo = it->second;

	return true;

#else

	return false;

#endif
}

void CGuildMatching::InsertGuildMatchingInfo(const GUILD_MATCHING_INFO& guildMatchingInfo)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(m_critical);

	m_GuildMatchingInfo[
		NormalizeToLower(guildMatchingInfo.GuildName)] =
		guildMatchingInfo;

#endif
}

void CGuildMatching::RemoveGuildMatchingInfo(const GUILD_MATCHING_INFO& guildMatchingInfo)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(m_critical);

	m_GuildMatchingInfo.erase(
		NormalizeToLower(guildMatchingInfo.GuildName));

#endif
}

bool CGuildMatching::GetGuildMatchingJoinInfo(GUILD_MATCHING_JOIN_INFO* lpGuildMatchingJoinInfo, const char* characterName)
{
#if (DATASERVER_UPDATE >= 801)

	if (lpGuildMatchingJoinInfo == nullptr ||
		characterName == nullptr)
	{
		return false;
	}

	CCriticalSection::CLock lock(m_critical);

	const auto it = m_GuildMatchingJoinInfo.find(
		NormalizeToLower(characterName));

	if (it == m_GuildMatchingJoinInfo.end())
	{
		return false;
	}

	*lpGuildMatchingJoinInfo = it->second;

	return true;

#else

	return false;

#endif
}

void CGuildMatching::InsertGuildMatchingJoinInfo(const GUILD_MATCHING_JOIN_INFO& guildMatchingJoinInfo)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(m_critical);

	m_GuildMatchingJoinInfo[
		NormalizeToLower(guildMatchingJoinInfo.CharacterName)] =
		guildMatchingJoinInfo;

#endif
}

void CGuildMatching::RemoveGuildMatchingJoinInfo(const GUILD_MATCHING_JOIN_INFO& guildMatchingJoinInfo)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(m_critical);

	m_GuildMatchingJoinInfo.erase(
		NormalizeToLower(guildMatchingJoinInfo.CharacterName));

#endif
}

void CGuildMatching::RemoveGuildMatchingJoinInfoNotifyAll(const GUILD_MATCHING_INFO& guildMatchingInfo)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(m_critical);

	for (const auto& entry : m_GuildMatchingJoinInfo)
	{
		if (_stricmp(
			entry.second.GuildName,
			guildMatchingInfo.GuildName) != 0)
		{
			continue;
		}

		DGGuildMatchingNotifySend(entry.second.CharacterName, entry.second.GuildName, 2);
	}

#endif
}

DWORD CGuildMatching::GenerateGuildMatchingList(DWORD* curPage,	DWORD* maxPage,	BYTE* lpMsg,
	int* size)
{
#if (DATASERVER_UPDATE >= 801)

	if (curPage == nullptr ||
		maxPage == nullptr ||
		lpMsg == nullptr ||
		size == nullptr)
	{
		return 0;
	}

	if (*size < 0 || *size > MAX_SEND_PACKET_SIZE)
	{
		return 0;
	}

	if (*curPage == 0)
	{
		*curPage = 1;
	}

	CCriticalSection::CLock lock(m_critical);

	DWORD count = 0;
	DWORD pageCount = 0;

	const DWORD firstEntry = (*curPage - 1) * 9;
	const DWORD lastEntry = *curPage * 9;

	for (const auto& entry : m_GuildMatchingInfo)
	{
		if (pageCount >= firstEntry &&
			pageCount < lastEntry)
		{
			if ((*size + sizeof(SDHP_GUILD_MATCHING_LIST)) >
				MAX_SEND_PACKET_SIZE)
			{
				break;
			}

			const GUILD_MATCHING_INFO& guildInfo = entry.second;

			SDHP_GUILD_MATCHING_LIST info{};

			std::memcpy(
				info.Text,
				guildInfo.Text,
				sizeof(info.Text));

			std::memcpy(
				info.CharacterName,
				guildInfo.CharacterName,
				sizeof(info.CharacterName));

			std::memcpy(
				info.GuildName,
				guildInfo.GuildName,
				sizeof(info.GuildName));

			info.GuildMemberCount = guildInfo.GuildMemberCount;
			info.GuildMasterClass = guildInfo.GuildMasterClass;
			info.InterestType = guildInfo.InterestType;
			info.LevelRange = guildInfo.LevelRange;
			info.ClassType = guildInfo.ClassType;
			info.GuildMasterLevel = guildInfo.GuildMasterLevel;
			info.BoardNumber = guildInfo.BoardNumber;
			info.GuildNumber = guildInfo.GuildNumber;
			info.GensType = guildInfo.GensType;

			std::memcpy(
				lpMsg + *size,
				&info,
				sizeof(info));

			*size += sizeof(info);

			++count;
		}

		++pageCount;
	}

	*maxPage =
		(pageCount == 0)
		? 1
		: ((pageCount - 1) / 9) + 1;

	return count;

#else

	return 0;

#endif
}

DWORD CGuildMatching::GenerateGuildMatchingList(DWORD* curPage,	DWORD* maxPage,	const char* searchWord,	BYTE* lpMsg, int* size)
{
#if (DATASERVER_UPDATE >= 801)

	if (curPage == nullptr ||
		maxPage == nullptr ||
		searchWord == nullptr ||
		lpMsg == nullptr ||
		size == nullptr)
	{
		return 0;
	}

	if (*size < 0 || *size > MAX_SEND_PACKET_SIZE)
	{
		return 0;
	}

	if (*curPage == 0)
	{
		*curPage = 1;
	}

	CCriticalSection::CLock lock(m_critical);

	DWORD count = 0;
	DWORD pageCount = 0;

	const DWORD firstEntry = (*curPage - 1) * 9;
	const DWORD lastEntry = *curPage * 9;

	for (const auto& entry : m_GuildMatchingInfo)
	{
		const GUILD_MATCHING_INFO& guildInfo = entry.second;

		if (std::strstr(guildInfo.Text, searchWord) == nullptr)
		{
			continue;
		}

		if (pageCount >= firstEntry &&
			pageCount < lastEntry)
		{
			if ((*size + sizeof(SDHP_GUILD_MATCHING_LIST)) >
				MAX_SEND_PACKET_SIZE)
			{
				break;
			}

			SDHP_GUILD_MATCHING_LIST info{};

			std::memcpy(
				info.Text,
				guildInfo.Text,
				sizeof(info.Text));

			std::memcpy(
				info.CharacterName,
				guildInfo.CharacterName,
				sizeof(info.CharacterName));

			std::memcpy(
				info.GuildName,
				guildInfo.GuildName,
				sizeof(info.GuildName));

			info.GuildMemberCount = guildInfo.GuildMemberCount;
			info.GuildMasterClass = guildInfo.GuildMasterClass;
			info.InterestType = guildInfo.InterestType;
			info.LevelRange = guildInfo.LevelRange;
			info.ClassType = guildInfo.ClassType;
			info.GuildMasterLevel = guildInfo.GuildMasterLevel;
			info.BoardNumber = guildInfo.BoardNumber;
			info.GuildNumber = guildInfo.GuildNumber;
			info.GensType = guildInfo.GensType;

			std::memcpy(
				lpMsg + *size,
				&info,
				sizeof(info));

			*size += sizeof(info);

			++count;
		}

		++pageCount;
	}

	*maxPage =
		(pageCount == 0)
		? 1
		: ((pageCount - 1) / 9) + 1;

	return count;

#else

	return 0;

#endif
}

DWORD CGuildMatching::GenerateGuildMatchingJoinList(const char* guildName, BYTE* lpMsg, int* size)
{
#if (DATASERVER_UPDATE >= 801)

	if (guildName == nullptr ||
		lpMsg == nullptr ||
		size == nullptr)
	{
		return 0;
	}

	if (*size < 0 || *size > MAX_SEND_PACKET_SIZE)
	{
		return 0;
	}

	CCriticalSection::CLock lock(m_critical);

	DWORD count = 0;

	for (const auto& entry : m_GuildMatchingJoinInfo)
	{
		const GUILD_MATCHING_JOIN_INFO& joinInfo = entry.second;

		if (_stricmp(joinInfo.GuildName, guildName) != 0)
		{
			continue;
		}

		if ((*size + sizeof(SDHP_GUILD_MATCHING_JOIN_LIST)) >
			MAX_SEND_PACKET_SIZE)
		{
			break;
		}

		SDHP_GUILD_MATCHING_JOIN_LIST info{};

		std::memcpy(
			info.CharacterName,
			joinInfo.CharacterName,
			sizeof(info.CharacterName));

		info.Class = joinInfo.Class;
		info.Level = joinInfo.Level;

		std::memcpy(
			lpMsg + *size,
			&info,
			sizeof(info));

		*size += sizeof(info);

		++count;
	}

	return count;

#else

	return 0;

#endif
}

void CGuildMatching::DGGuildMatchingNotifySend(const char* characterName, const char* guildName, DWORD result)
{
#if (DATASERVER_UPDATE >= 801)

	if (characterName == nullptr || guildName == nullptr)
	{
		return;
	}

	CHARACTER_INFO characterInfo{};

	if (!gCharacterManager.GetCharacterInfo(
		&characterInfo,
		characterName))
	{
		return;
	}

	CServerManager* const lpServerManager =
		FindServerByCode(characterInfo.GameServerCode);

	if (lpServerManager == nullptr)
	{
		return;
	}

	SDHP_GUILD_MATCHING_NOTIFY_SEND pMsg{};

	pMsg.Header.set(
		HEAD_GUILD_MATCHING,
		SUB_GUILD_MATCHING_NOTIFY,
		sizeof(pMsg));

	pMsg.Index = characterInfo.UserIndex;
	pMsg.Result = result;

	std::memcpy(
		pMsg.Account,
		characterInfo.Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		characterInfo.CharacterName,
		sizeof(pMsg.CharacterName));

	std::memcpy(
		pMsg.GuildName,
		guildName,
		sizeof(pMsg.GuildName));

	gSocketManager.DataSend(
		lpServerManager->m_index,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CGuildMatching::DGGuildMatchingNotifyMasterSend(
	const char* characterName,
	DWORD result)
{
#if (DATASERVER_UPDATE >= 801)

	if (characterName == nullptr)
	{
		return;
	}

	CHARACTER_INFO characterInfo{};

	if (!gCharacterManager.GetCharacterInfo(
		&characterInfo,
		characterName))
	{
		return;
	}

	CServerManager* const lpServerManager =
		FindServerByCode(characterInfo.GameServerCode);

	if (lpServerManager == nullptr)
	{
		return;
	}

	SDHP_GUILD_MATCHING_NOTIFY_MASTER_SEND pMsg{};

	pMsg.Header.set(
		HEAD_GUILD_MATCHING,
		SUB_GUILD_MATCHING_NOTIFY_MASTER,
		sizeof(pMsg));

	pMsg.Index = characterInfo.UserIndex;
	pMsg.Result = result;

	std::memcpy(
		pMsg.Account,
		characterInfo.Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		characterInfo.CharacterName,
		sizeof(pMsg.CharacterName));

	gSocketManager.DataSend(
		lpServerManager->m_index,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}
