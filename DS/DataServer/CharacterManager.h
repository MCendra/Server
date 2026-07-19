// CharacterManager.h
#pragma once
#include "CriticalSection.h"
#include <array>

// Cantidad de caracteres por cuenta
constexpr BYTE MAX_CHARACTER_SLOT = 5;
constexpr BYTE CHARACTER_LIST_EQUIPMENT_COUNT = 12;

#pragma pack(push,1)
struct CHARACTER_INFO
{
	char CharacterName[MAX_CHARACTER_NAME];
	char Account[MAX_ACCOUNT_NAME];
	WORD UserIndex;
	WORD GameServerCode;
};
#pragma pack(pop)

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
	bool GetFreeCharacterSlot(const std::array<std::array<char, MAX_CHARACTER_NAME>, MAX_CHARACTER_SLOT>& characterName, BYTE* slot);
	bool GetCharacterSlot(const std::array<std::array<char, MAX_CHARACTER_NAME>, MAX_CHARACTER_SLOT>& characterName, const char* name, BYTE* slot);
private:
	CCriticalSection m_critical;
	std::map<std::string,CHARACTER_INFO> m_CharacterInfo;
};

extern CCharacterManager gCharacterManager;
