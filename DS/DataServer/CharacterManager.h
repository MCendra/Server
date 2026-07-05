// CharacterManager.h
#pragma once
#include "CriticalSection.h"

struct CHARACTER_INFO
{
	char CharacterName[MAX_CHARACTER_NAME];
	char Account[MAX_ACCOUNT_NAME];
	WORD UserIndex;
	WORD GameServerCode;
};

class CCharacterManager
{
public:
	CCharacterManager() = default;
	~CCharacterManager() = default;
	void ClearServerCharacterInfo(WORD ServerCode);
	bool GetCharacterInfo(CHARACTER_INFO* lpCharacterInfo, const char* name);
	void InsertCharacterInfo(const CHARACTER_INFO& characterInfo);
	void RemoveCharacterInfo(const CHARACTER_INFO& characterInfo);
	long GetCharacterCount();
private:
	CCriticalSection m_critical;
	std::map<std::string,CHARACTER_INFO> m_CharacterInfo;
};

extern CCharacterManager gCharacterManager;
