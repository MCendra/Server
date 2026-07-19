// PartyMatching.cpp
#include "Header.h"
#include "PartyMatching.h"
#include "Log.h"
#include "CharacterManager.h"
#include "SocketManager.h"
#include "Util.h"

CPartyMatching gPartyMatching;

void CPartyMatching::ClearPartyMatchingInfo(WORD ServerCode)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(this->m_critical);

	for (auto it = this->m_PartyMatchingInfo.begin(); it != this->m_PartyMatchingInfo.end();)
	{
		CHARACTER_INFO CharacterInfo;

		if (gCharacterManager.GetCharacterInfo(&CharacterInfo, it->second.CharacterName) != 0)
		{
			++it;
			continue;
		}

		this->RemovePartyMatchingJoinInfoNotifyAll(it->second);

		it = this->m_PartyMatchingInfo.erase(it);
	}

#endif
}

void CPartyMatching::ClearPartyMatchingJoinInfo(WORD ServerCode)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(this->m_critical);

	for (auto it = this->m_PartyMatchingJoinInfo.begin(); it != this->m_PartyMatchingJoinInfo.end();)
	{
		CHARACTER_INFO CharacterInfo;

		if (gCharacterManager.GetCharacterInfo(&CharacterInfo, it->second.CharacterName) != 0)
		{
			++it;
			continue;
		}

		it = this->m_PartyMatchingJoinInfo.erase(it);
	}

#endif
}

bool CPartyMatching::GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo, const char* characterName)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(this->m_critical);

	auto it = this->m_PartyMatchingInfo.find(NormalizeToLower(characterName));

	if (it != this->m_PartyMatchingInfo.end())
	{
		*lpPartyMatchingInfo = it->second;
		return true;
	}

	return false;

#else

	return false;

#endif
}

bool CPartyMatching::GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo, WORD ServerCode, WORD Level, BYTE Class, BYTE GensType)
{
#if (DATASERVER_UPDATE >= 801)

	this->m_critical.lock();

	std::vector<PARTY_MATCHING_INFO> RandomPartyMatchingInfo;

	for (const auto& pair : this->m_PartyMatchingInfo)
	{
		const PARTY_MATCHING_INFO& info = pair.second;

		if (info.UsePassword != 0)
			continue;

		if (info.GensType != GensType)
			continue;

		if (info.ServerCode != ServerCode)
			continue;

		if (info.PartyMemberCount >= MAX_PARTY_USER)
			continue;

		if ((info.WantedClass & (1 << (Class & 7))) == 0)
			continue;

		if (info.MinLevel > Level || info.MaxLevel < Level)
			continue;

		RandomPartyMatchingInfo.push_back(info);
	}

	if (RandomPartyMatchingInfo.empty())
	{
		this->m_critical.unlock();
		return 0;
	}

	(*lpPartyMatchingInfo) = RandomPartyMatchingInfo[rand() % RandomPartyMatchingInfo.size()];

	this->m_critical.unlock();
	return 1;

#else

	return 0;

#endif
}

void CPartyMatching::InsertPartyMatchingInfo(PARTY_MATCHING_INFO PartyMatchingInfo)
{
#if (DATASERVER_UPDATE >= 801)

	this->m_critical.lock();

	std::string key = NormalizeToLower(PartyMatchingInfo.CharacterName);

	auto it = this->m_PartyMatchingInfo.find(key);

	if (it == this->m_PartyMatchingInfo.end())
	{
		this->m_PartyMatchingInfo.emplace(key, PartyMatchingInfo);
	}
	else
	{
		it->second = PartyMatchingInfo;
	}

	this->m_critical.unlock();

#endif
}

void CPartyMatching::RemovePartyMatchingInfo(PARTY_MATCHING_INFO PartyMatchingInfo)
{
#if (DATASERVER_UPDATE >= 801)

	this->m_critical.lock();

	std::string key = NormalizeToLower(PartyMatchingInfo.CharacterName);

	auto it = this->m_PartyMatchingInfo.find(key);

	if (it != this->m_PartyMatchingInfo.end())
	{
		this->m_PartyMatchingInfo.erase(it);
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();

#endif
}

bool CPartyMatching::GetPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO* lpPartyMatchingJoinInfo, const char* characterName)
{
#if(DATASERVER_UPDATE>=801)

	CCriticalSection::CLock lock(this->m_critical);

	auto it = this->m_PartyMatchingJoinInfo.find(NormalizeToLower(characterName));

	if (it != this->m_PartyMatchingJoinInfo.end())
	{
		*lpPartyMatchingJoinInfo = it->second;
		return true;
	}

	return false;

#else

	return false;

#endif
}

void CPartyMatching::InsertPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo)
{
#if(DATASERVER_UPDATE>=801)

	CCriticalSection::CLock lock(this->m_critical);

	auto it = this->m_PartyMatchingJoinInfo.find(NormalizeToLower(PartyMatchingJoinInfo.CharacterName));

	if (it == this->m_PartyMatchingJoinInfo.end())
	{
		this->m_PartyMatchingJoinInfo.insert(std::make_pair(NormalizeToLower(PartyMatchingJoinInfo.CharacterName), PartyMatchingJoinInfo));
	}
	else
	{
		it->second = PartyMatchingJoinInfo;
	}

#endif
}

void CPartyMatching::RemovePartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo)
{
#if(DATASERVER_UPDATE>=801)

	CCriticalSection::CLock lock(this->m_critical);

	auto it = this->m_PartyMatchingJoinInfo.find(NormalizeToLower(PartyMatchingJoinInfo.CharacterName));

	if (it != this->m_PartyMatchingJoinInfo.end())
	{
		this->m_PartyMatchingJoinInfo.erase(it);
	}

#endif
}

void CPartyMatching::RemovePartyMatchingJoinInfoNotifyAll(PARTY_MATCHING_INFO PartyMatchingInfo)
{
#if(DATASERVER_UPDATE>=801)

	CCriticalSection::CLock lock(this->m_critical);

	for (const auto& pair : this->m_PartyMatchingJoinInfo)
	{
		if (_stricmp(pair.second.LeaderName, PartyMatchingInfo.CharacterName) == 0)
		{
			this->DGPartyMatchingNotifySend(pair.second.CharacterName, pair.second.LeaderName, 2);
		}
	}

#endif
}

DWORD CPartyMatching::GeneratePartyMatchingList(DWORD* CurPage, DWORD* MaxPage, BYTE UseSearchWord, const char* SearchWord, BYTE* lpMsg, int* size)
{
#if(DATASERVER_UPDATE>=801)

	DWORD count = 0;
	DWORD PageCount = 0;

	CCriticalSection::CLock lock(this->m_critical);

	SDHP_PARTY_MATCHING_LIST info;

	for (const auto& pair : this->m_PartyMatchingInfo)
	{
		const PARTY_MATCHING_INFO& data = pair.second;

		if (UseSearchWord == 0 || strstr(data.Text, SearchWord) != nullptr)
		{
			if (PageCount >= (((*CurPage) - 1) * 9) && PageCount < ((*CurPage) * 9))
			{
				memcpy(info.Text, data.Text, sizeof(info.Text));
				memcpy(info.CharacterName, data.CharacterName, sizeof(info.CharacterName));

				info.MinLevel = data.MinLevel;
				info.MaxLevel = data.MaxLevel;
				info.HuntingGround = data.HuntingGround;
				info.LeaderLevel = data.LeaderLevel;
				info.LeaderClass = data.LeaderClass;
				info.WantedClass = data.WantedClass;
				info.PartyMemberCount = data.PartyMemberCount;
				info.UsePassword = data.UsePassword;

				memcpy(info.WantedClassDetailInfo, data.WantedClassDetailInfo, sizeof(info.WantedClassDetailInfo));

				info.ServerCode = data.ServerCode;
				info.GensType = data.GensType;

				memcpy(&lpMsg[*size], &info, sizeof(info));
				(*size) += sizeof(info);

				++count;
			}

			++PageCount;
		}
	}

	*MaxPage = ((PageCount == 0) ? 1 : (((PageCount - 1) / 9) + 1));

	return count;

#else

	return 0;

#endif
}

DWORD CPartyMatching::GeneratePartyMatchingJoinList(const char* LeaderName, BYTE* lpMsg, int* size)
{
#if(DATASERVER_UPDATE>=801)

	DWORD count = 0;

	CCriticalSection::CLock lock(this->m_critical);

	SDHP_PARTY_MATCHING_JOIN_LIST info;

	for (const auto& pair : this->m_PartyMatchingJoinInfo)
	{
		const PARTY_MATCHING_JOIN_INFO& data = pair.second;

		if (_stricmp(data.LeaderName, LeaderName) == 0)
		{
			memcpy(info.CharacterName, data.CharacterName, sizeof(info.CharacterName));

			info.Class = data.Class;
			info.Level = data.Level;

			memcpy(&lpMsg[*size], &info, sizeof(info));
			(*size) += sizeof(info);

			++count;
		}
	}

	return count;

#else

	return 0;

#endif
}

void CPartyMatching::GDPartyMatchingInsertRecv(
	const SDHP_PARTY_MATCHING_INSERT_RECV* lpMsg,
	int serverIndex,
	int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_PARTY_MATCHING_INSERT_RECV);

	SDHP_PARTY_MATCHING_INSERT_SEND pMsg{};

	pMsg.Header.set(0x29, 0x00, sizeof(pMsg));

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

	PARTY_MATCHING_INFO partyMatchingInfo{};

	if (GetPartyMatchingInfo(&partyMatchingInfo, lpMsg->CharacterName))
	{
		pMsg.Result = 0xFFFFFFFE;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	std::memcpy(
		partyMatchingInfo.CharacterName,
		lpMsg->CharacterName,
		sizeof(partyMatchingInfo.CharacterName));

	std::memcpy(
		partyMatchingInfo.Text,
		lpMsg->Text,
		sizeof(partyMatchingInfo.Text));

	std::memcpy(
		partyMatchingInfo.Password,
		lpMsg->Password,
		sizeof(partyMatchingInfo.Password));

	partyMatchingInfo.MinLevel = lpMsg->MinLevel;
	partyMatchingInfo.MaxLevel = lpMsg->MaxLevel;
	partyMatchingInfo.HuntingGround = lpMsg->HuntingGround;
	partyMatchingInfo.LeaderLevel = lpMsg->LeaderLevel;
	partyMatchingInfo.LeaderClass = lpMsg->LeaderClass;
	partyMatchingInfo.WantedClass = lpMsg->WantedClass;

	std::memcpy(
		partyMatchingInfo.WantedClassDetailInfo,
		lpMsg->WantedClassDetailInfo,
		sizeof(partyMatchingInfo.WantedClassDetailInfo));

	partyMatchingInfo.PartyMemberCount = lpMsg->PartyMemberCount;
	partyMatchingInfo.ApprovalType = lpMsg->ApprovalType;
	partyMatchingInfo.UsePassword = lpMsg->UsePassword;
	partyMatchingInfo.GensType = lpMsg->GensType;
	partyMatchingInfo.ServerCode =
		gServerManager[serverIndex].m_ServerCode;

	InsertPartyMatchingInfo(partyMatchingInfo);

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CPartyMatching::GDPartyMatchingListRecv(const SDHP_PARTY_MATCHING_LIST_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_PARTY_MATCHING_LIST_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_PARTY_MATCHING_LIST_SEND pMsg{};

	pMsg.Header.set(0x29, 0x01, 0);

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
	pMsg.CurPage = lpMsg->Page;
	pMsg.MaxPage = 1;

	pMsg.Count = GeneratePartyMatchingList(
		&pMsg.CurPage,
		&pMsg.MaxPage,
		lpMsg->UseSearchWord,
		lpMsg->SearchWord,
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


void CPartyMatching::GDPartyMatchingJoinInsertRecv(const SDHP_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_PARTY_MATCHING_JOIN_INSERT_RECV);

	SDHP_PARTY_MATCHING_JOIN_INSERT_SEND pMsg{};

	pMsg.Header.set(0x29, 0x02, sizeof(pMsg));

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

	PARTY_MATCHING_INFO partyMatchingInfo{};
	PARTY_MATCHING_JOIN_INFO partyMatchingJoinInfo{};

	if (lpMsg->UseRandomParty == 0)
	{
		if (!GetPartyMatchingInfo(
			&partyMatchingInfo,
			lpMsg->LeaderName))
		{
			pMsg.Result = 0xFFFFFFFE;

			gSocketManager.DataSend(
				serverIndex,
				reinterpret_cast<BYTE*>(&pMsg),
				sizeof(pMsg));

			return;
		}
	}
	else
	{
		if (!GetPartyMatchingInfo(
			&partyMatchingInfo,
			gServerManager[serverIndex].m_ServerCode,
			lpMsg->Level,
			lpMsg->Class,
			lpMsg->GensType))
		{
			pMsg.Result = 0xFFFFFFFD;

			gSocketManager.DataSend(
				serverIndex,
				reinterpret_cast<BYTE*>(&pMsg),
				sizeof(pMsg));

			return;
		}
	}

	if (partyMatchingInfo.UsePassword != 0 &&
		std::strcmp(
			partyMatchingInfo.Password,
			lpMsg->Password) != 0)
	{
		pMsg.Result = 0xFFFFFFFF;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	if (GetPartyMatchingJoinInfo(
		&partyMatchingJoinInfo,
		lpMsg->CharacterName))
	{
		pMsg.Result = 0xFFFFFFFC;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	if (_stricmp(
		partyMatchingInfo.CharacterName,
		lpMsg->CharacterName) == 0)
	{
		pMsg.Result = 0xFFFFFFFB;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	std::memcpy(
		partyMatchingJoinInfo.CharacterName,
		lpMsg->CharacterName,
		sizeof(partyMatchingJoinInfo.CharacterName));

	std::memcpy(
		partyMatchingJoinInfo.LeaderName,
		partyMatchingInfo.CharacterName,
		sizeof(partyMatchingJoinInfo.LeaderName));

	partyMatchingJoinInfo.LeaderServerCode =
		partyMatchingInfo.ServerCode;

	partyMatchingJoinInfo.Class = lpMsg->Class;
	partyMatchingJoinInfo.Level = lpMsg->Level;

	InsertPartyMatchingJoinInfo(partyMatchingJoinInfo);

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

	if (partyMatchingInfo.ApprovalType == 0)
	{
		DGPartyMatchingNotifyLeaderSend(
			partyMatchingInfo.CharacterName,
			0);
	}
	else
	{
		DGPartyMatchingNotifySend(
			partyMatchingJoinInfo.CharacterName,
			partyMatchingInfo.CharacterName,
			1);
	}

#endif
}

void CPartyMatching::GDPartyMatchingJoinAcceptRecv(const SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV);

	SDHP_PARTY_MATCHING_JOIN_ACCEPT_SEND pMsg{};

	pMsg.Header.set(0x29, 0x05, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.Account,
		lpMsg->Account,
		sizeof(pMsg.Account));

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	std::memcpy(
		pMsg.MemberName,
		lpMsg->MemberName,
		sizeof(pMsg.MemberName));

	pMsg.Result = 0;
	pMsg.Type = lpMsg->Type;
	pMsg.Flag = 0;

	PARTY_MATCHING_INFO partyMatchingInfo{};
	PARTY_MATCHING_JOIN_INFO partyMatchingJoinInfo{};

	if (!GetPartyMatchingInfo(
		&partyMatchingInfo,
		lpMsg->CharacterName) ||
		!GetPartyMatchingJoinInfo(
			&partyMatchingJoinInfo,
			lpMsg->MemberName))
	{
		pMsg.Result = 0xFFFFFFFF;

		gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

		return;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

	DGPartyMatchingNotifySend(
		lpMsg->MemberName,
		lpMsg->CharacterName,
		(lpMsg->Type == 0) ? 2 : 1);

#endif
}

void CPartyMatching::GDPartyMatchingJoinCancelRecv(
	const SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg,
	int serverIndex,
	int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV);

	SDHP_PARTY_MATCHING_JOIN_CANCEL_SEND pMsg{};

	pMsg.Header.set(0x29, 0x06, sizeof(pMsg));

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
	pMsg.Flag = lpMsg->Flag;

	if (lpMsg->Type == 0)
	{
		PARTY_MATCHING_INFO partyMatchingInfo{};

		if (!GetPartyMatchingInfo(
			&partyMatchingInfo,
			lpMsg->CharacterName))
		{
			pMsg.Result = 0xFFFFFFFF;

			gSocketManager.DataSend(
				serverIndex,
				reinterpret_cast<BYTE*>(&pMsg),
				sizeof(pMsg));

			return;
		}

		RemovePartyMatchingInfo(partyMatchingInfo);

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		RemovePartyMatchingJoinInfoNotifyAll(
			partyMatchingInfo);

		return;
	}

	PARTY_MATCHING_JOIN_INFO partyMatchingJoinInfo{};

	if (!GetPartyMatchingJoinInfo(
		&partyMatchingJoinInfo,
		lpMsg->CharacterName))
	{
		pMsg.Result = 0xFFFFFFFF;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	RemovePartyMatchingJoinInfo(
		partyMatchingJoinInfo);

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CPartyMatching::GDPartyMatchingInsertSaveRecv(
	const SDHP_PARTY_MATCHING_INSERT_SAVE_RECV* lpMsg,
	int serverIndex,
	int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_PARTY_MATCHING_INSERT_SAVE_RECV);

	PARTY_MATCHING_INFO partyMatchingInfo{};

	if (!GetPartyMatchingInfo(&partyMatchingInfo, lpMsg->CharacterName))
	{
		return;
	}

	partyMatchingInfo.LeaderLevel = lpMsg->LeaderLevel;
	partyMatchingInfo.LeaderClass = lpMsg->LeaderClass;
	partyMatchingInfo.PartyMemberCount = lpMsg->PartyMemberCount;
	partyMatchingInfo.GensType = lpMsg->GensType;

	InsertPartyMatchingInfo(partyMatchingInfo);

#endif
}

void CPartyMatching::DGPartyMatchingNotifySend(const char* characterName, const char* leaderName, DWORD result)
{
#if (DATASERVER_UPDATE >= 801)

	if (characterName == nullptr || leaderName == nullptr)
	{
		return;
	}

	CHARACTER_INFO characterInfo{};

	if (!gCharacterManager.GetCharacterInfo(&characterInfo, characterName))
	{
		return;
	}

	CServerManager* const lpServerManager =	FindServerByCode(characterInfo.GameServerCode);

	if (lpServerManager == nullptr)
	{
		return;
	}

	SDHP_PARTY_MATCHING_NOTIFY_SEND pMsg{};

	pMsg.Header.set(0x29, 0x07, sizeof(pMsg));

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
		pMsg.LeaderName,
		leaderName,
		sizeof(pMsg.LeaderName));

	gSocketManager.DataSend(
		lpServerManager->m_index,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CPartyMatching::DGPartyMatchingNotifyLeaderSend(const char* name, DWORD result)
{
#if (DATASERVER_UPDATE >= 801)

	if (name == nullptr)
	{
		return;
	}

	CHARACTER_INFO characterInfo{};

	if (!gCharacterManager.GetCharacterInfo(
		&characterInfo,
		name))
	{
		return;
	}

	CServerManager* const lpServerManager =
		FindServerByCode(characterInfo.GameServerCode);

	if (lpServerManager == nullptr)
	{
		return;
	}

	SDHP_PARTY_MATCHING_NOTIFY_LEADER_SEND pMsg{};

	pMsg.Header.set(0x29, 0x08, sizeof(pMsg));

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

void CPartyMatching::GDPartyMatchingJoinInfoRecv(const SDHP_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg, int serverIndex, int size)
{
#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_JOIN_INFO_SEND pMsg;

	pMsg.Header.set(0x29, 0x03, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo;

	if (this->GetPartyMatchingJoinInfo(&PartyMatchingJoinInfo, lpMsg->CharacterName) == 0)
	{
		pMsg.Result = 0xFFFFFFFF;
		gSocketManager.DataSend(serverIndex, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	pMsg.LeaderServerCode = PartyMatchingJoinInfo.LeaderServerCode;

	memcpy(pMsg.LeaderName, PartyMatchingJoinInfo.LeaderName, sizeof(pMsg.LeaderName));

	gSocketManager.DataSend(serverIndex, (BYTE*)&pMsg, sizeof(pMsg));

#endif
}

void CPartyMatching::GDPartyMatchingJoinListRecv(const SDHP_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_PARTY_MATCHING_JOIN_LIST_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_PARTY_MATCHING_JOIN_LIST_SEND pMsg{};

	pMsg.Header.set(0x29, 0x04, 0);

	int sendSize = sizeof(pMsg);

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	pMsg.count = this->GeneratePartyMatchingJoinList(
		lpMsg->CharacterName,
		send,
		&sendSize);

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);

#endif
}