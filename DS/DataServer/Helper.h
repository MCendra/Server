// Helper.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_HELPER_DATA_RECV
{
	PSBMSG_HEAD header; // C1:17:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_HELPER_DATA_SAVE_RECV
{
	PSWMSG_HEAD header; // C1:17:30
	WORD index;
	char account[11];
	char name[11];
	BYTE data[256];
};

// DataServer -> GameServer

struct SDHP_HELPER_DATA_SEND
{
	PSWMSG_HEAD header; // C1:17:00
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	BYTE data[256];
};

class CHelper
{
public:
	CHelper() = default;
	~CHelper() = default;

	void GDHelperDataRecv(const SDHP_HELPER_DATA_RECV* lpMsg, int index);
	void GDHelperDataSaveRecv(const SDHP_HELPER_DATA_SAVE_RECV* lpMsg);
};

extern CHelper gHelper;
