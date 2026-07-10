// PartyMatching.cpp
#include "Header.h"
#include "PartyMatching.h"
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

bool CPartyMatching::GetPartyMatchingInfo(PARTY_MATCHING_INFO* lpPartyMatchingInfo, char* name)
{
#if (DATASERVER_UPDATE >= 801)

	CCriticalSection::CLock lock(this->m_critical);

	auto it = this->m_PartyMatchingInfo.find(NormalizeToLower(name));

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

bool CPartyMatching::GetPartyMatchingJoinInfo(PARTY_MATCHING_JOIN_INFO* lpPartyMatchingJoinInfo, char* name)
{
#if(DATASERVER_UPDATE>=801)

	CCriticalSection::CLock lock(this->m_critical);

	auto it = this->m_PartyMatchingJoinInfo.find(NormalizeToLower(name));

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

DWORD CPartyMatching::GeneratePartyMatchingList(DWORD* CurPage, DWORD* MaxPage, BYTE UseSearchWord, char* SearchWord, BYTE* lpMsg, int* size)
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

DWORD CPartyMatching::GeneratePartyMatchingJoinList(char* LeaderName, BYTE* lpMsg, int* size)
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

void CPartyMatching::GDPartyMatchingInsertRecv(SDHP_PARTY_MATCHING_INSERT_RECV* lpMsg, int index)
{
#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_INSERT_SEND pMsg;

	pMsg.Header.set(0x29, 0x00, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	PARTY_MATCHING_INFO PartyMatchingInfo;

	if (this->GetPartyMatchingInfo(&PartyMatchingInfo, lpMsg->CharacterName) != 0)
	{
		pMsg.Result = 0xFFFFFFFE;
		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	memcpy(PartyMatchingInfo.CharacterName, lpMsg->CharacterName, sizeof(PartyMatchingInfo.CharacterName));
	memcpy(PartyMatchingInfo.Text, lpMsg->Text, sizeof(PartyMatchingInfo.Text));
	memcpy(PartyMatchingInfo.Password, lpMsg->Password, sizeof(PartyMatchingInfo.Password));

	PartyMatchingInfo.MinLevel = lpMsg->MinLevel;
	PartyMatchingInfo.MaxLevel = lpMsg->MaxLevel;
	PartyMatchingInfo.HuntingGround = lpMsg->HuntingGround;
	PartyMatchingInfo.LeaderLevel = lpMsg->LeaderLevel;
	PartyMatchingInfo.LeaderClass = lpMsg->LeaderClass;
	PartyMatchingInfo.WantedClass = lpMsg->WantedClass;

	memcpy(
		PartyMatchingInfo.WantedClassDetailInfo,
		lpMsg->WantedClassDetailInfo,
		sizeof(PartyMatchingInfo.WantedClassDetailInfo));

	PartyMatchingInfo.PartyMemberCount = lpMsg->PartyMemberCount;
	PartyMatchingInfo.ApprovalType = lpMsg->ApprovalType;
	PartyMatchingInfo.UsePassword = lpMsg->UsePassword;
	PartyMatchingInfo.GensType = lpMsg->GensType;
	PartyMatchingInfo.ServerCode = gServerManager[index].m_ServerCode;

	this->InsertPartyMatchingInfo(PartyMatchingInfo);

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

#endif
}

void CPartyMatching::GDPartyMatchingListRecv(SDHP_PARTY_MATCHING_LIST_RECV* lpMsg, int index)
{
#if(DATASERVER_UPDATE>=801)

	BYTE send[1024];

	SDHP_PARTY_MATCHING_LIST_SEND pMsg;

	pMsg.Header.set(0x29, 0x01, 0);

	int size = sizeof(pMsg);

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.CurPage = lpMsg->Page;
	pMsg.MaxPage = 1;

	pMsg.Count = this->GeneratePartyMatchingList(
		&pMsg.CurPage,
		&pMsg.MaxPage,
		lpMsg->UseSearchWord,
		lpMsg->SearchWord,
		send,
		&size);

	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);

#endif
}

void CPartyMatching::GDPartyMatchingJoinInsertRecv(SDHP_PARTY_MATCHING_JOIN_INSERT_RECV* lpMsg, int index)
{
#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_JOIN_INSERT_SEND pMsg;

	pMsg.Header.set(0x29, 0x02, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	PARTY_MATCHING_INFO PartyMatchingInfo;
	PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo;

	if (lpMsg->UseRandomParty == 0 &&
		this->GetPartyMatchingInfo(&PartyMatchingInfo, lpMsg->LeaderName) == 0)
	{
		pMsg.Result = 0xFFFFFFFE;
		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	if (lpMsg->UseRandomParty != 0 &&
		this->GetPartyMatchingInfo(
			&PartyMatchingInfo,
			gServerManager[index].m_ServerCode,
			lpMsg->Level,
			lpMsg->Class,
			lpMsg->GensType) == 0)
	{
		pMsg.Result = 0xFFFFFFFD;
		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	if (PartyMatchingInfo.UsePassword != 0 &&
		strcmp(PartyMatchingInfo.Password, lpMsg->Password) != 0)
	{
		pMsg.Result = 0xFFFFFFFF;
		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	if (this->GetPartyMatchingJoinInfo(&PartyMatchingJoinInfo, lpMsg->CharacterName) != 0)
	{
		pMsg.Result = 0xFFFFFFFC;
		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	if (_stricmp(PartyMatchingInfo.CharacterName, lpMsg->CharacterName) == 0)
	{
		pMsg.Result = 0xFFFFFFFB;
		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	memcpy(
		PartyMatchingJoinInfo.CharacterName,
		lpMsg->CharacterName,
		sizeof(PartyMatchingJoinInfo.CharacterName));

	memcpy(
		PartyMatchingJoinInfo.LeaderName,
		PartyMatchingInfo.CharacterName,
		sizeof(PartyMatchingJoinInfo.LeaderName));

	PartyMatchingJoinInfo.LeaderServerCode = PartyMatchingInfo.ServerCode;
	PartyMatchingJoinInfo.Class = lpMsg->Class;
	PartyMatchingJoinInfo.Level = lpMsg->Level;

	this->InsertPartyMatchingJoinInfo(PartyMatchingJoinInfo);

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

	if (PartyMatchingInfo.ApprovalType == 0)
	{
		this->DGPartyMatchingNotifyLeaderSend(PartyMatchingInfo.CharacterName, 0);
	}
	else
	{
		this->DGPartyMatchingNotifySend(
			PartyMatchingJoinInfo.CharacterName,
			PartyMatchingInfo.CharacterName,
			1);
	}

#endif
}

void CPartyMatching::GDPartyMatchingJoinInfoRecv(SDHP_PARTY_MATCHING_JOIN_INFO_RECV* lpMsg, int index)
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
		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	pMsg.LeaderServerCode = PartyMatchingJoinInfo.LeaderServerCode;

	memcpy(
		pMsg.LeaderName,
		PartyMatchingJoinInfo.LeaderName,
		sizeof(pMsg.LeaderName));

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

#endif
}

void CPartyMatching::GDPartyMatchingJoinListRecv(SDHP_PARTY_MATCHING_JOIN_LIST_RECV* lpMsg, int index)
{
#if(DATASERVER_UPDATE>=801)

	BYTE send[1024];

	SDHP_PARTY_MATCHING_JOIN_LIST_SEND pMsg;

	pMsg.Header.set(0x29, 0x04, 0);

	int size = sizeof(pMsg);

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	pMsg.count = this->GeneratePartyMatchingJoinList(
		lpMsg->CharacterName,
		send,
		&size);

	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);

#endif
}

void CPartyMatching::GDPartyMatchingJoinAcceptRecv(SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV* lpMsg, int index)
{
#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_JOIN_ACCEPT_SEND pMsg;

	pMsg.Header.set(0x29, 0x05, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;

	memcpy(pMsg.MemberName, lpMsg->MemberName, sizeof(pMsg.MemberName));

	pMsg.Type = lpMsg->Type;
	pMsg.Flag = 0;

	PARTY_MATCHING_INFO PartyMatchingInfo;
	PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo;

	if (this->GetPartyMatchingInfo(&PartyMatchingInfo, lpMsg->CharacterName) == 0 ||
		this->GetPartyMatchingJoinInfo(&PartyMatchingJoinInfo, lpMsg->MemberName) == 0)
	{
		pMsg.Result = 0xFFFFFFFF;
		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
		return;
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

	this->DGPartyMatchingNotifySend(
		lpMsg->MemberName,
		lpMsg->CharacterName,
		(lpMsg->Type == 0) ? 2 : 1);

#endif
}

void CPartyMatching::GDPartyMatchingJoinCancelRecv(SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV* lpMsg, int index)
{
#if(DATASERVER_UPDATE>=801)

	SDHP_PARTY_MATCHING_JOIN_CANCEL_SEND pMsg;

	pMsg.Header.set(0x29, 0x06, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 0;
	pMsg.Type = lpMsg->Type;
	pMsg.Flag = lpMsg->Flag;

	if (lpMsg->Type == 0)
	{
		PARTY_MATCHING_INFO PartyMatchingInfo;

		if (this->GetPartyMatchingInfo(&PartyMatchingInfo, lpMsg->CharacterName) == 0)
		{
			pMsg.Result = 0xFFFFFFFF;
			gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
			return;
		}

		this->RemovePartyMatchingInfo(PartyMatchingInfo);

		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

		this->RemovePartyMatchingJoinInfoNotifyAll(PartyMatchingInfo);
	}
	else
	{
		PARTY_MATCHING_JOIN_INFO PartyMatchingJoinInfo;

		if (this->GetPartyMatchingJoinInfo(&PartyMatchingJoinInfo, lpMsg->CharacterName) == 0)
		{
			pMsg.Result = 0xFFFFFFFF;
			gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
			return;
		}

		this->RemovePartyMatchingJoinInfo(PartyMatchingJoinInfo);

		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
	}

#endif
}

void CPartyMatching::GDPartyMatchingInsertSaveRecv(SDHP_PARTY_MATCHING_INSERT_SAVE_RECV* lpMsg)
{
#if(DATASERVER_UPDATE>=801)

	PARTY_MATCHING_INFO PartyMatchingInfo;

	if (this->GetPartyMatchingInfo(&PartyMatchingInfo, lpMsg->CharacterName) == 0)
	{
		return;
	}

	PartyMatchingInfo.LeaderLevel = lpMsg->LeaderLevel;
	PartyMatchingInfo.LeaderClass = lpMsg->LeaderClass;
	PartyMatchingInfo.PartyMemberCount = lpMsg->PartyMemberCount;
	PartyMatchingInfo.GensType = lpMsg->GensType;

	this->InsertPartyMatchingInfo(PartyMatchingInfo);

#endif
}

void CPartyMatching::DGPartyMatchingNotifySend(const char* name, const char* LeaderName, DWORD result)
{
#if(DATASERVER_UPDATE>=801)

	CHARACTER_INFO CharacterInfo;

	if (gCharacterManager.GetCharacterInfo(&CharacterInfo, const_cast<char*>(name)) == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_NOTIFY_SEND pMsg;

	pMsg.Header.set(0x29, 0x07, sizeof(pMsg));

	pMsg.Index = CharacterInfo.UserIndex;

	memcpy(pMsg.Account, CharacterInfo.Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, CharacterInfo.CharacterName, sizeof(pMsg.CharacterName));
	memcpy(pMsg.LeaderName, LeaderName, sizeof(pMsg.LeaderName));

	pMsg.Result = result;

	CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

	if (lpServerManager != nullptr)
	{
		gSocketManager.DataSend(lpServerManager->m_index, (BYTE*)&pMsg, pMsg.Header.size);
	}

#endif
}

void CPartyMatching::DGPartyMatchingNotifyLeaderSend(const char* name, DWORD result)
{
#if(DATASERVER_UPDATE>=801)

	CHARACTER_INFO CharacterInfo;

	if (gCharacterManager.GetCharacterInfo(&CharacterInfo, const_cast<char*>(name)) == 0)
	{
		return;
	}

	SDHP_PARTY_MATCHING_NOTIFY_LEADER_SEND pMsg;

	pMsg.Header.set(0x29, 0x08, sizeof(pMsg));

	pMsg.Index = CharacterInfo.UserIndex;

	memcpy(pMsg.Account, CharacterInfo.Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, CharacterInfo.CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = result;

	CServerManager* lpServerManager = FindServerByCode(CharacterInfo.GameServerCode);

	if (lpServerManager != nullptr)
	{
		gSocketManager.DataSend(lpServerManager->m_index, (BYTE*)&pMsg, pMsg.Header.size);
	}

#endif
}