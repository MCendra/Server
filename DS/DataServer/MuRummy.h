// MuRummy.h
#pragma once
#include "DataServerProtocol.h"

#pragma pack(push,1)
struct MuRummyCardInfoDS
{
	MuRummyCardInfoDS()
	{
		this->Color = -1;
		this->Number = -1;
		this->SlotNum = -1;
		this->Seq = -1;
		this->Status = -1;
	};

	BYTE Color;
	BYTE Number;
	BYTE SlotNum;
	BYTE Seq;
	BYTE Status;
};

struct MuRummyCardUpdateDS
{
	MuRummyCardUpdateDS()
	{
		this->SlotNum = -1;
		this->Seq = -1;
		this->Status = -1;
	};

	BYTE SlotNum;
	BYTE Seq;
	BYTE Status;
};

struct PMSG_REQ_MURUMMY_SELECT_DS
{
	PSBMSG_HEAD Header; // C1:12:00
	char AccountID[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Index;
};

struct PMSG_ANS_MURUMMY_SELECT_DS
{
	PSBMSG_HEAD Header; // C1:12:00
	WORD Index;
	WORD Score;
	BYTE Result;
	MuRummyCardInfoDS stMuRummyCardInfoDS[MURUMMY_MAX_CARD];
};

struct PMSG_REQ_MURUMMY_INSERT_DS
{
	PSBMSG_HEAD Header; // C1:12:30
	char AccountID[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Index;
	MuRummyCardInfoDS stMuRummyCardInfoDS[MURUMMY_MAX_CARD];
};

struct PMSG_REQ_MURUMMY_UPDATE_DS 
{
	PSBMSG_HEAD Header; // C1:12:31
	char AccountID[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE SlotNum;
	BYTE Status;
	BYTE Sequence;
};

struct PMSG_REQ_MURUMMY_SCORE_UPDATE_DS
{
	PSBMSG_HEAD Header; // C1:12:32
	char AccountID[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Score;
	MuRummyCardUpdateDS stCardUpdateDS[3];
};

struct PMSG_REQ_MURUMMY_DELETE_DS
{
	PSBMSG_HEAD Header; // C1:12:33
	char AccountID[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct PMSG_REQ_MURUMMY_SLOTUPDATE_DS
{
	PSBMSG_HEAD Header; // C1:12:34
	char AccountID[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	MuRummyCardUpdateDS stCardUpdateDS;
};

struct PMSG_REQ_MURUMMY_INFO_UPDATE_DS
{
	PSBMSG_HEAD Header; // C1:12:35
	char AccountID[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Score;
	MuRummyCardUpdateDS stMuRummyCardUpdateDS[MURUMMY_MAX_CARD];
};
#pragma pack(pop)

class CMuRummy
{
public:
	CMuRummy() = default;
	~CMuRummy() = default;
	void GDReqCardInfo(const PMSG_REQ_MURUMMY_SELECT_DS* lpMsg, int serverIndex, int size);
	void GDReqCardInfoInsert(const PMSG_REQ_MURUMMY_INSERT_DS* lpMsg, int serverIndex, int size);
	void GDReqCardInfoUpdate(const PMSG_REQ_MURUMMY_UPDATE_DS* lpMsg, int serverIndex, int size);
	void GDReqScoreUpdate(const PMSG_REQ_MURUMMY_SCORE_UPDATE_DS* lpMsg, int serverIndex, int size);
	void GDReqScoreDelete(const PMSG_REQ_MURUMMY_DELETE_DS* lpMsg, int serverIndex, int size);
	void GDReqSlotInfoUpdate(const PMSG_REQ_MURUMMY_SLOTUPDATE_DS* lpMsg, int serverIndex, int size);
	void GDReqMuRummyInfoUpdate(const PMSG_REQ_MURUMMY_INFO_UPDATE_DS* lpMsg, int serverIndex, int size);
};

extern CMuRummy gMuRummy;
