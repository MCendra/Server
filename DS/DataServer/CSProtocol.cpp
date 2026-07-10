// CSProtocol.cpp
#include "Header.h"
#include "CSProtocol.h"
#include "CharacterManager.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "Util.h"

void ChatServerProtocolCore(int index, BYTE head, BYTE* lpMsg, int /*size*/)
{
	switch (head)
	{
		case CS_HEAD_FRIEND_LIST:
			FriendListRequest(reinterpret_cast<FHP_FRIENDLIST_REQ*>(lpMsg), index);
			break;

		case CS_HEAD_FRIEND_STATE:
			FriendStateClientRecv(reinterpret_cast<FHP_FRIEND_STATE_C*>(lpMsg), index);
			break;

		case CS_HEAD_FRIEND_ADD:
			FriendAddRequest(reinterpret_cast<FHP_FRIEND_ADD_REQ*>(lpMsg), index);
			break;

		case CS_HEAD_WAIT_FRIEND_ADD:
			WaitFriendAddRequest(reinterpret_cast<FHP_WAITFRIEND_ADD_REQ*>(lpMsg), index);
			break;

		case CS_HEAD_FRIEND_DELETE:
			FriendDelRequest(reinterpret_cast<FHP_FRIEND_ADD_REQ*>(lpMsg), index);
			break;

		case CS_HEAD_MEMO_SEND:
			FriendMemoSend(reinterpret_cast<FHP_FRIEND_MEMO_SEND*>(lpMsg), index);
			break;

		case CS_HEAD_MEMO_LIST:
			FriendMemoListReq(reinterpret_cast<FHP_FRIEND_MEMO_LIST_REQ*>(lpMsg), index);
			break;

		case CS_HEAD_MEMO_READ:
			FriendMemoReadReq(reinterpret_cast<FHP_FRIEND_MEMO_RECV_REQ*>(lpMsg), index);
			break;

		case CS_HEAD_MEMO_DELETE:
			FriendMemoDelReq(reinterpret_cast<FHP_FRIEND_MEMO_DEL_REQ*>(lpMsg), index);
			break;
		
		default:
			break;
	}
}

void CSDataSend(int index, BYTE* lpMsg, int size)
{
	switch (lpMsg[PACKET_TYPE_OFFSET])
	{
		case PACKET_C1:
			{
				BYTE send[8192]{};

				PSBMSG_HEAD Header;

				size += 1;

				Header.set(DS_HEAD_CONNECT_SERVER, lpMsg[C1_PACKET_HEAD_OFFSET], size);

				std::memcpy(send, &Header, sizeof(Header));

				std::memcpy(send + sizeof(Header), lpMsg + C1_PACKET_DATA_OFFSET, size - sizeof(Header));

				gSocketManager.DataSend(index, send, size);
			}
			break;
		case PACKET_C2:
			{
				BYTE send[8192]{};

				PSWMSG_HEAD Header;

				size += 1;

				Header.set(DS_HEAD_CONNECT_SERVER, lpMsg[C2_PACKET_HEAD_OFFSET], size);

				std::memcpy(send, &Header, sizeof(Header));

				std::memcpy(send + sizeof(Header), lpMsg + C2_PACKET_DATA_OFFSET, size - sizeof(Header));

				gSocketManager.DataSend(index, send, size);
			}
			break;
		default:
			break;
	}
}

void CSDataRecv(int index, BYTE head, BYTE* lpMsg, int size)
{

	UNREFERENCED_PARAMETER(head);

	switch (lpMsg[PACKET_TYPE_OFFSET])
	{
	case PACKET_C1:
	{
		BYTE recv[8192]{};

		PBMSG_HEAD Header;

		size -= 1;

		Header.set(lpMsg[C1_PACKET_DATA_OFFSET], size);

		std::memcpy(recv, &Header, sizeof(Header));

		std::memcpy(recv + sizeof(Header), lpMsg + C1_PACKET_DATA_OFFSET + 1, size - sizeof(Header));

		ChatServerProtocolCore(index, Header.head, recv, size);
	}
	break;

	case PACKET_C2:
	{
		BYTE recv[8192]{};

		PWMSG_HEAD Header;

		size -= 1;

		Header.set(lpMsg[C2_PACKET_DATA_OFFSET], size);

		std::memcpy(recv, &Header, sizeof(Header));

		std::memcpy(recv + sizeof(Header), lpMsg + C2_PACKET_DATA_OFFSET + 1, size - sizeof(Header));

		ChatServerProtocolCore(index, Header.head, recv, size);
	}
	break;
	}
}

void FriendListRequest(const FHP_FRIENDLIST_REQ* lpMsg, int index)
{
	BYTE send[2048]{};

	FHP_FRIENDLIST_COUNT pMsg{};

	pMsg.Header.set(CS_HEAD_FRIEND_LIST, 0);

	int size = sizeof(pMsg);

	pMsg.Number = lpMsg->Number;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Count = 0;
	pMsg.MailCount = 0;

	gQueryManager.ExecQuery("WZ_UserGuidCreate '%s'", lpMsg->CharacterName);

	gQueryManager.Fetch();

	gQueryManager.Close();

	gQueryManager.ExecQuery("SELECT GUID,MemoTotal FROM T_FriendMain WHERE Name='%s'", lpMsg->CharacterName);

	gQueryManager.Fetch();

	DWORD guid = gQueryManager.GetAsInteger("GUID");

	pMsg.MailCount = gQueryManager.GetAsInteger("MemoTotal");

	gQueryManager.Close();

	if (gQueryManager.ExecQuery("SELECT FriendName,Del FROM T_FriendList WHERE GUID=%d", guid))
	{
		FHP_FRIENDLIST friendList{};

		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			gQueryManager.GetAsString("FriendName", friendList.CharacterName, sizeof(friendList.CharacterName));

			friendList.Server = (gQueryManager.GetAsInteger("Del") == 0)
				? static_cast<BYTE>(GetServerCodeByName(friendList.CharacterName))
				: 0xFF;

			if ((size + sizeof(friendList)) > sizeof(send))
			{
				break;
			}

			std::memcpy(send + size, &friendList, sizeof(friendList));

			size += sizeof(friendList);

			++pMsg.Count;
		}
	}

	gQueryManager.Close();

	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	CSDataSend(index, send, size);

	WaitFriendListResult(index, guid, lpMsg->Number, lpMsg->CharacterName);

	FriendMemoList(index, guid, lpMsg->Number, lpMsg->CharacterName);
}

void FriendStateClientRecv(const FHP_FRIEND_STATE_C* lpMsg, int index)
{
	UNREFERENCED_PARAMETER(lpMsg);
	UNREFERENCED_PARAMETER(index);
}

void FriendAddRequest(const FHP_FRIEND_ADD_REQ* lpMsg, int index)
{
	gQueryManager.ExecQuery(
		"WZ_WaitFriendAdd '%s','%s'",
		lpMsg->CharacterName,
		lpMsg->FriendName);

	gQueryManager.Fetch();

	BYTE result = gQueryManager.GetResult(0);

	gQueryManager.Close();

	if (result == 8)
	{
		FHP_FRIEND_ADD_RESULT pMsg{};

		pMsg.Header.set(CS_HEAD_FRIEND_ADD, sizeof(pMsg));

		pMsg.Number = lpMsg->Number;
		pMsg.Result = 0;

		std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
		std::memcpy(pMsg.FriendName, lpMsg->FriendName, sizeof(pMsg.FriendName));

		pMsg.Server = static_cast<BYTE>(GetServerCodeByName(lpMsg->FriendName));

		gQueryManager.ExecQuery(
			"WZ_FriendAdd '%s','%s'",
			lpMsg->CharacterName,
			lpMsg->FriendName);

		gQueryManager.Fetch();

		pMsg.Result = gQueryManager.GetResult(0);

		gQueryManager.Close();

		CSDataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		if (pMsg.Result == 1)
		{
			SendFriendState(
				lpMsg->FriendName,
				lpMsg->CharacterName,
				static_cast<BYTE>(GetServerCodeByName(lpMsg->FriendName)));
		}
	}
	else
	{
		FHP_FRIEND_ADD_RESULT pMsg{};

		pMsg.Header.set(CS_HEAD_FRIEND_ADD, sizeof(pMsg));

		pMsg.Number = lpMsg->Number;
		pMsg.Result = result;

		std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
		std::memcpy(pMsg.FriendName, lpMsg->FriendName, sizeof(pMsg.FriendName));

		pMsg.Server = 0xFF;

		CSDataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		if (pMsg.Result == 1)
		{
			CHARACTER_INFO characterInfo{};

			if (gCharacterManager.GetCharacterInfo(&characterInfo, lpMsg->FriendName))
			{
				FHP_WAITFRIENDLIST_COUNT notify{};

				notify.Header.set(CS_HEAD_WAIT_FRIEND_LIST, sizeof(notify));

				notify.Number = characterInfo.UserIndex;

				std::memcpy(notify.CharacterName, lpMsg->FriendName, sizeof(notify.CharacterName));
				std::memcpy(notify.FriendName, lpMsg->CharacterName, sizeof(notify.FriendName));

				CServerManager* lpServerManager = FindServerByCode(characterInfo.GameServerCode);

				if (lpServerManager != nullptr)
				{
					CSDataSend(
						lpServerManager->m_index,
						reinterpret_cast<BYTE*>(&notify),
						notify.Header.size);
				}
			}
		}
	}
}

void WaitFriendAddRequest(const FHP_WAITFRIEND_ADD_REQ* lpMsg, int index)
{
	FHP_WAITFRIEND_ADD_RESULT pMsg{};

	pMsg.Header.set(CS_HEAD_WAIT_FRIEND_ADD, sizeof(pMsg));

	pMsg.Number = lpMsg->Number;
	pMsg.Result = lpMsg->Result;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.FriendName, lpMsg->FriendName, sizeof(pMsg.FriendName));

	pMsg.Server = static_cast<BYTE>(GetServerCodeByName(lpMsg->FriendName));

	if (lpMsg->Result == 0)
	{
		gQueryManager.ExecQuery(
			"WZ_WaitFriendDel '%s','%s'",
			lpMsg->CharacterName,
			lpMsg->FriendName);

		gQueryManager.Fetch();

		pMsg.Result = gQueryManager.GetResult(0);

		gQueryManager.Close();

		CSDataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);
	}
	else
	{
		gQueryManager.ExecQuery(
			"WZ_FriendAdd '%s','%s'",
			lpMsg->CharacterName,
			lpMsg->FriendName);

		gQueryManager.Fetch();

		pMsg.Result = gQueryManager.GetResult(0);

		gQueryManager.Close();

		CSDataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		if (pMsg.Result == 1)
		{
			SendFriendState(
				lpMsg->FriendName,
				lpMsg->CharacterName,
				static_cast<BYTE>(GetServerCodeByName(lpMsg->FriendName)));
		}
	}
}

void FriendDelRequest(const FHP_FRIEND_ADD_REQ* lpMsg, int index)
{
	FHP_FRIEND_DEL_RESULT pMsg{};

	pMsg.Header.set(CS_HEAD_FRIEND_DELETE, sizeof(pMsg));

	pMsg.Number = lpMsg->Number;
	pMsg.Result = 0;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.FriendName, lpMsg->FriendName, sizeof(pMsg.FriendName));

	gQueryManager.ExecQuery(
		"WZ_FriendDel '%s','%s'",
		lpMsg->CharacterName,
		lpMsg->FriendName);

	gQueryManager.Fetch();

	pMsg.Result = gQueryManager.GetResult(0);

	gQueryManager.Close();

	CSDataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

	if (pMsg.Result == 1)
	{
		SendFriendState(
			lpMsg->FriendName,
			lpMsg->CharacterName,
			0xFF);
	}
}

void FriendMemoSend(const FHP_FRIEND_MEMO_SEND* lpMsg, int index)
{
	FHP_FRIEND_MEMO_SEND_RESULT pMsg{};

	pMsg.Header.set(CS_HEAD_MEMO_SEND, sizeof(pMsg));

	pMsg.Number = lpMsg->Number;
	pMsg.Result = 1;
	pMsg.WindowGuid = lpMsg->WindowGuid;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	gQueryManager.BindParameterAsString(1, lpMsg->ToName, sizeof(lpMsg->ToName));
	gQueryManager.BindParameterAsString(2, lpMsg->Subject, sizeof(lpMsg->Subject));

	gQueryManager.ExecQuery(
		"WZ_WriteMail '%s',?,?,'%d','%d'",
		lpMsg->CharacterName,
		lpMsg->Dir,
		lpMsg->Action);

	gQueryManager.Fetch();

	const DWORD memo = gQueryManager.GetResult(0);
	const DWORD guid = gQueryManager.GetResult(1);

	gQueryManager.Close();

	if (memo <= 10)
	{
		pMsg.Result = static_cast<BYTE>(memo);

		CSDataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

		return;
	}

	BYTE memoBuff[1000]{};

	const auto memoSize = std::min<int>(
		lpMsg->MemoSize,
		sizeof(memoBuff));

	std::memcpy(
		memoBuff,
		lpMsg->Memo,
		memoSize);

	gQueryManager.BindParameterAsBinary(1, memoBuff, sizeof(memoBuff));
	gQueryManager.BindParameterAsBinary(2, lpMsg->Photo, sizeof(lpMsg->Photo));

	gQueryManager.ExecQuery(
		"UPDATE T_FriendMail SET Memo=?,Photo=? WHERE MemoIndex=%d AND GUID=%d",
		memo,
		guid);

	gQueryManager.Close();

	CSDataSend(index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

	if (pMsg.Result != 1)
	{
		return;
	}

	CHARACTER_INFO characterInfo{};

	if (!gCharacterManager.GetCharacterInfo(&characterInfo, lpMsg->ToName))
	{
		return;
	}

	gQueryManager.ExecQuery(
		"SELECT MemoIndex,FriendName,wDate,Subject,bRead "
		"FROM T_FriendMail WHERE MemoIndex=%d AND GUID=%d",
		memo,
		guid);

	gQueryManager.Fetch();

	FHP_FRIEND_MEMO_LIST notify{};

	notify.Header.set(CS_HEAD_MEMO_LIST, sizeof(notify));

	notify.Number = characterInfo.UserIndex;
	notify.MemoIndex = gQueryManager.GetAsInteger("MemoIndex");

	gQueryManager.GetAsString("FriendName", notify.SendName, sizeof(notify.SendName));

	std::memcpy(notify.RecvName, lpMsg->ToName, sizeof(notify.RecvName));

	gQueryManager.GetAsString("wDate", notify.Date, sizeof(notify.Date));
	gQueryManager.GetAsString("Subject", notify.Subject, sizeof(notify.Subject));

	notify.Read = gQueryManager.GetAsInteger("bRead");

	gQueryManager.Close();

	CServerManager* const lpServerManager = FindServerByCode(characterInfo.GameServerCode);

	if (lpServerManager != nullptr)
	{
		CSDataSend(
			lpServerManager->m_index,
			reinterpret_cast<BYTE*>(&notify),
			sizeof(notify));
	}
}

void FriendMemoListReq(const FHP_FRIEND_MEMO_LIST_REQ* lpMsg, int index)
{
	gQueryManager.ExecQuery(
		"SELECT GUID FROM T_FriendMain WHERE Name='%s'",
		lpMsg->CharacterName);

	gQueryManager.Fetch();

	const DWORD guid = gQueryManager.GetAsInteger("GUID");

	gQueryManager.Close();

	FriendMemoList(
		index,
		guid,
		lpMsg->Number,
		lpMsg->CharacterName);
}

void FriendMemoReadReq(const FHP_FRIEND_MEMO_RECV_REQ* lpMsg, int index)
{
	gQueryManager.ExecQuery(
		"SELECT GUID FROM T_FriendMain WHERE Name='%s'",
		lpMsg->CharacterName);

	gQueryManager.Fetch();

	const DWORD guid = gQueryManager.GetAsInteger("GUID");

	gQueryManager.Close();

	FHP_FRIEND_MEMO_RECV pMsg{};

	pMsg.Header.set(CS_HEAD_MEMO_READ, sizeof(pMsg));

	pMsg.Number = lpMsg->Number;
	pMsg.MemoIndex = lpMsg->MemoIndex;

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	gQueryManager.ExecQuery(
		"SELECT Memo,Photo,Dir,Act FROM T_FriendMail WHERE MemoIndex=%d AND GUID=%d",
		lpMsg->MemoIndex,
		guid);

	gQueryManager.Fetch();

	gQueryManager.GetAsBinary(
		"Photo",
		pMsg.Photo,
		sizeof(pMsg.Photo));

	pMsg.Dir = gQueryManager.GetAsInteger("Dir");
	pMsg.Action = gQueryManager.GetAsInteger("Act");

	std::memset(pMsg.Memo, 0, sizeof(pMsg.Memo));

	gQueryManager.GetAsBinary(
		"Memo",
		reinterpret_cast<BYTE*>(pMsg.Memo),
		sizeof(pMsg.Memo) - 1);

	pMsg.MemoSize = static_cast<short>(std::strlen(pMsg.Memo));

	gQueryManager.Close();

	gQueryManager.ExecQuery(
		"UPDATE T_FriendMail SET bRead=1 WHERE MemoIndex=%d AND GUID=%d",
		lpMsg->MemoIndex,
		guid);

	gQueryManager.Close();

	CSDataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));
}

void FriendMemoDelReq(const FHP_FRIEND_MEMO_DEL_REQ* lpMsg, int index)
{
	FHP_FRIEND_MEMO_DEL_RESULT pMsg{};

	pMsg.Header.set(CS_HEAD_MEMO_DELETE, sizeof(pMsg));

	pMsg.Result = 0;
	pMsg.MemoIndex = lpMsg->MemoIndex;
	pMsg.Number = lpMsg->Number;

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	gQueryManager.ExecQuery(
		"WZ_DelMail '%s',%d",
		lpMsg->CharacterName,
		lpMsg->MemoIndex);

	gQueryManager.Fetch();

	pMsg.Result = gQueryManager.GetResult(0);

	gQueryManager.Close();

	CSDataSend(
		index,
		reinterpret_cast<BYTE*>(&pMsg),
		pMsg.Header.size);
}

void WaitFriendListResult(int index, DWORD guid, WORD aIndex, const char* name)
{
	if (gQueryManager.ExecQuery(
		"SELECT FriendName FROM T_WaitFriend WHERE GUID=%d",
		guid))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			FHP_WAITFRIENDLIST_COUNT pMsg{};

			pMsg.Header.set(CS_HEAD_WAIT_FRIEND_LIST, sizeof(pMsg));

			pMsg.Number = aIndex;

			std::memcpy(
				pMsg.CharacterName,
				name,
				sizeof(pMsg.CharacterName));

			gQueryManager.GetAsString(
				"FriendName",
				pMsg.FriendName,
				sizeof(pMsg.FriendName));

			CSDataSend(
				index,
				reinterpret_cast<BYTE*>(&pMsg),
				pMsg.Header.size);
		}
	}

	gQueryManager.Close();
}

void FriendStateRecv(const char* name, BYTE state)
{
	DWORD guid = 0;

	if (!gQueryManager.ExecQuery("SELECT GUID FROM T_FriendMain WHERE Name='%s'", name) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		return;
	}

	guid = gQueryManager.GetAsInteger("GUID");

	gQueryManager.Close();

	if (gQueryManager.ExecQuery("SELECT FriendName,Del FROM T_FriendList WHERE GUID=%d", guid))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			FHP_FRIEND_STATE pMsg{};

			pMsg.Header.set(CS_HEAD_FRIEND_STATE, sizeof(pMsg));

			gQueryManager.GetAsString("FriendName", pMsg.CharacterName, sizeof(pMsg.CharacterName));

			CHARACTER_INFO characterInfo{};

			if (!gCharacterManager.GetCharacterInfo(&characterInfo, pMsg.CharacterName))
			{
				continue;
			}

			pMsg.Number = characterInfo.UserIndex;

			std::memcpy(pMsg.FriendName, name, sizeof(pMsg.FriendName));

			const bool deleted = (gQueryManager.GetAsInteger("Del") != 0);

			pMsg.Offline = (state == 0 && !deleted)	? static_cast<BYTE>(GetServerCodeByName(pMsg.FriendName)) : 0xFF;

			CServerManager* const lpServerManager = FindServerByCode(characterInfo.GameServerCode);

			if (lpServerManager != nullptr)
			{
				CSDataSend(lpServerManager->m_index, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);
			}
		}
	}

	gQueryManager.Close();
}

void FriendMemoList(int index, DWORD guid, WORD aIndex, const char* name)
{
	if (gQueryManager.ExecQuery(
		"SELECT MemoIndex,FriendName,wDate,Subject,bRead FROM T_FriendMail WHERE GUID=%d",
		guid))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			FHP_FRIEND_MEMO_LIST pMsg{};

			pMsg.Header.set(CS_HEAD_MEMO_LIST, sizeof(pMsg));

			pMsg.Number = aIndex;

			pMsg.MemoIndex = gQueryManager.GetAsInteger("MemoIndex");

			gQueryManager.GetAsString(
				"FriendName",
				pMsg.SendName,
				sizeof(pMsg.SendName));

			std::memcpy(
				pMsg.RecvName,
				name,
				sizeof(pMsg.RecvName));

			gQueryManager.GetAsString(
				"wDate",
				pMsg.Date,
				sizeof(pMsg.Date));

			gQueryManager.GetAsString(
				"Subject",
				pMsg.Subject,
				sizeof(pMsg.Subject));

			pMsg.Read = gQueryManager.GetAsInteger("bRead");

			CSDataSend(
				index,
				reinterpret_cast<BYTE*>(&pMsg),
				sizeof(pMsg));
		}
	}

	gQueryManager.Close();
}

// Fix:
static void SendFriendState(const char* characterName, const char* friendName, BYTE state)
{
	CHARACTER_INFO characterInfo{};

	if (!gCharacterManager.GetCharacterInfo(&characterInfo, characterName))
	{
		return;
	}

	FHP_FRIEND_STATE pMsg{};

	pMsg.Header.set(CS_HEAD_FRIEND_STATE, sizeof(pMsg));

	pMsg.Number = characterInfo.UserIndex;

	std::memcpy(pMsg.CharacterName, characterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.FriendName, friendName, sizeof(pMsg.FriendName));

	pMsg.Offline = state;

	CServerManager* lpServerManager = FindServerByCode(characterInfo.GameServerCode);

	if (lpServerManager != nullptr)
	{
		CSDataSend(
			lpServerManager->m_index,
			reinterpret_cast<BYTE*>(&pMsg),
			pMsg.Header.size);
	}
}