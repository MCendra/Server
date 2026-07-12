// ChatServerProtocol.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct FHP_FRIENDLIST_REQ
{
	PBMSG_HEAD Header; // C1:60
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Server;
};

struct FHP_FRIEND_STATE_C
{
	PBMSG_HEAD Header; // C1:62
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE State;
};

struct FHP_FRIEND_ADD_REQ
{
	PBMSG_HEAD Header; // C1:[63/65]
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
};

struct FHP_WAITFRIEND_ADD_REQ
{
	PBMSG_HEAD Header; // C1:64
	BYTE Result;
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
};

struct FHP_FRIEND_CHATROOM_CREATE_REQ
{
	PBMSG_HEAD Header; // C1:66
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
};

struct FHP_FRIEND_MEMO_SEND
{
	PWMSG_HEAD Header; // C1:70
	WORD  Number;
	DWORD WindowGuid;
	char CharacterName[MAX_CHARACTER_NAME];
	char ToName[MAX_CHARACTER_NAME];
	char Subject[32];
	BYTE Dir;
	BYTE Action;
	WORD  MemoSize;
	BYTE Photo[18];
	char Memo[1000];
};

struct FHP_FRIEND_MEMO_LIST_REQ
{
	PBMSG_HEAD Header; // C1:71
	WORD Number;
	char CharacterName[MAX_CHARACTER_NAME];
};

struct FHP_FRIEND_MEMO_RECV_REQ
{
	PBMSG_HEAD Header; // C1:72
	WORD  Number;
	WORD MemoIndex;
	char CharacterName[MAX_CHARACTER_NAME];
};

struct FHP_FRIEND_MEMO_DEL_REQ
{
	PBMSG_HEAD Header; // C1:73
	WORD  Number;
	WORD MemoIndex;
	char CharacterName[MAX_CHARACTER_NAME];
};

struct FHP_FRIEND_INVITATION_REQ
{
	PBMSG_HEAD Header; // C1:74
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
	WORD RoomNumber;
	DWORD WindowGuid;
};

// DataServer -> GameServer

struct FHP_FRIENDLIST_COUNT
{
	PWMSG_HEAD Header; // C1:60
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Count;
	BYTE MailCount;
};

struct FHP_FRIENDLIST
{
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Server;
};

struct FHP_WAITFRIENDLIST_COUNT
{
	PBMSG_HEAD Header; // C1:61
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
};

struct FHP_FRIEND_STATE
{
	PBMSG_HEAD Header; // C1:62
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
	BYTE Offline;
};

struct FHP_FRIEND_ADD_RESULT
{
	PBMSG_HEAD Header; // C1:63
	WORD  Number;
	BYTE Result;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
	BYTE Server;
};

struct FHP_WAITFRIEND_ADD_RESULT
{
	PBMSG_HEAD Header; // C1:64
	WORD  Number;
	BYTE Result;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
	BYTE Server;
};

struct FHP_FRIEND_DEL_RESULT
{
	PBMSG_HEAD Header; // C1:65
	WORD  Number;
	BYTE Result;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
};

struct FHP_FRIEND_CHATROOM_CREATE_RESULT
{
	PBMSG_HEAD Header; // C1:66
	BYTE Result;
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	char FriendName[MAX_CHARACTER_NAME];
	char ServerIp[16];
	WORD RoomNumber;
	DWORD Ticket;
	BYTE Type;
};

struct FHP_FRIEND_MEMO_SEND_RESULT
{
	PBMSG_HEAD Header; // C1:70
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	DWORD WindowGuid;
};

struct FHP_FRIEND_MEMO_LIST
{
	PWMSG_HEAD Header; // C1:71
	WORD Number;
	WORD MemoIndex;
	char SendName[MAX_CHARACTER_NAME];
	char RecvName[MAX_CHARACTER_NAME];
	char Date[30];
	char Subject[32];
	BYTE Read;
};

struct FHP_FRIEND_MEMO_RECV
{
	PWMSG_HEAD Header; // C1:72
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	WORD MemoIndex;
	WORD  MemoSize;
	BYTE Photo[18];
	BYTE Dir;
	BYTE Action;
	char Memo[1000];
};

struct FHP_FRIEND_MEMO_DEL_RESULT
{
	PBMSG_HEAD Header; // C1:73
	BYTE Result;
	WORD MemoIndex;
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
};

struct FHP_FRIEND_INVITATION_RET
{
	PBMSG_HEAD Header; // C1:74
	BYTE Result;
	WORD  Number;
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD WindowGuid;
};

//**********************************************//

void CSDataSend(int index, BYTE* lpMsg, int size);
void CSDataRecv(int index, BYTE head, BYTE* lpMsg, int size);
void ChatServerProtocolCore(int index, BYTE head, BYTE* lpMsg, int size);
void FriendListRequest(const FHP_FRIENDLIST_REQ* lpMsg, int index);
void FriendStateClientRecv(const FHP_FRIEND_STATE_C* lpMsg, int index);
void FriendAddRequest(const FHP_FRIEND_ADD_REQ* lpMsg, int index);
void WaitFriendAddRequest(const FHP_WAITFRIEND_ADD_REQ* lpMsg, int index);
void FriendDelRequest(const FHP_FRIEND_ADD_REQ* lpMsg, int index);
void FriendMemoSend(const FHP_FRIEND_MEMO_SEND* lpMsg, int index);
void FriendMemoListReq(const FHP_FRIEND_MEMO_LIST_REQ* lpMsg, int index);
void FriendMemoReadReq(const FHP_FRIEND_MEMO_RECV_REQ* lpMsg, int index);
void FriendMemoDelReq(const FHP_FRIEND_MEMO_DEL_REQ* lpMsg, int index);
void WaitFriendListResult(int index, DWORD guid, WORD aIndex, const char* characterName);
void FriendStateRecv(const char* characterName, BYTE state);
void FriendMemoList(int index, DWORD guid, WORD aIndex, const char* characterName);
static void SendFriendState(const char* characterName, const char* friendName, BYTE state);