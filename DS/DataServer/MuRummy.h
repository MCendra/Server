// MuRummy.h
#pragma once
#include "DataServerProtocol.h"

#define MURUMMY_MAX_CARD 24

struct _tagMuRummyCardInfoDS
{
	_tagMuRummyCardInfoDS()
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

struct _tagMuRummyCardUpdateDS
{
	_tagMuRummyCardUpdateDS()
	{
		this->SlotNum = -1;
		this->Seq = -1;
		this->Status = -1;
	};

	BYTE SlotNum;
	BYTE Seq;
	BYTE Status;
};

struct _tagPMSG_REQ_MURUMMY_SELECT_DS
{
	PSBMSG_HEAD Header; // C1:12:00
	char AccountID[11];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Index;
};

struct _tagPMSG_ANS_MURUMMY_SELECT_DS
{
	PSBMSG_HEAD Header; // C1:12:00
	WORD Index;
	WORD Score;
	BYTE Result;
	_tagMuRummyCardInfoDS stMuRummyCardInfoDS[MURUMMY_MAX_CARD];
};

struct _tagPMSG_REQ_MURUMMY_INSERT_DS
{
	PSBMSG_HEAD Header; // C1:12:30
	char AccountID[11];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Index;
	_tagMuRummyCardInfoDS stMuRummyCardInfoDS[MURUMMY_MAX_CARD];
};

struct _tagPMSG_REQ_MURUMMY_UPDATE_DS 
{
	PSBMSG_HEAD Header; // C1:12:31
	char AccountID[11];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE SlotNum;
	BYTE Status;
	BYTE Sequence;
};

struct _tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS
{
	PSBMSG_HEAD Header; // C1:12:32
	char AccountID[11];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Score;
	_tagMuRummyCardUpdateDS stCardUpdateDS[3];
};

struct _tagPMSG_REQ_MURUMMY_DELETE_DS
{
	PSBMSG_HEAD Header; // C1:12:33
	char AccountID[11];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct _tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS
{
	PSBMSG_HEAD Header; // C1:12:34
	char AccountID[11];
	char CharacterName[MAX_CHARACTER_NAME];
	_tagMuRummyCardUpdateDS stCardUpdateDS;
};

struct _tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS
{
	PSBMSG_HEAD Header; // C1:12:35
	char AccountID[11];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Score;
	_tagMuRummyCardUpdateDS stMuRummyCardUpdateDS[MURUMMY_MAX_CARD];
};

class CMuRummy
{
public:
	CMuRummy() = default;
	~CMuRummy() = default;
	void GDReqCardInfo(_tagPMSG_REQ_MURUMMY_SELECT_DS* lpMsg,int index);
	void GDReqCardInfoInsert(_tagPMSG_REQ_MURUMMY_INSERT_DS* lpMsg);
	void GDReqCardInfoUpdate(_tagPMSG_REQ_MURUMMY_UPDATE_DS* lpMsg);
	void GDReqScoreUpdate(_tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS* lpMsg);
	void GDReqScoreDelete(_tagPMSG_REQ_MURUMMY_DELETE_DS* lpMsg);
	void GDReqSlotInfoUpdate(_tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS* lpMsg);
	void GDReqMuRummyInfoUpdate(_tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS* lpMsg);
};

extern CMuRummy gMuRummy;
