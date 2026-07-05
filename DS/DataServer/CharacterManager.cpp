// CharacterManager.cpp
#include "Header.h"
#include "CharacterManager.h"
#include "ServerManager.h"
#include "Util.h"

CCharacterManager gCharacterManager;

void CCharacterManager::ClearServerCharacterInfo(WORD ServerCode)
{
	CCriticalSection::CLock lock(m_critical);

	for (auto it = m_CharacterInfo.begin(); it != m_CharacterInfo.end();)
	{
		if (it->second.GameServerCode != ServerCode)
		{
			++it;
			continue;
		}

		it = m_CharacterInfo.erase(it);
	}
}

bool CCharacterManager::GetCharacterInfo(CHARACTER_INFO* lpCharacterInfo, const char* name)
{
	if (lpCharacterInfo == nullptr || name == nullptr)
	{
		return false;
	}

	CCriticalSection::CLock lock(m_critical);

	const auto it = m_CharacterInfo.find(NormalizeToLower(name));

	if (it == m_CharacterInfo.end())
	{
		return false;
	}

	*lpCharacterInfo = it->second;

	return true;
}

void CCharacterManager::InsertCharacterInfo(const CHARACTER_INFO& characterInfo)
{
	CCriticalSection::CLock lock(m_critical);

	m_CharacterInfo[NormalizeToLower(characterInfo.CharacterName)] = characterInfo;
}

void CCharacterManager::RemoveCharacterInfo(const CHARACTER_INFO& characterInfo)
{
	CCriticalSection::CLock lock(m_critical);

	const auto key = NormalizeToLower(characterInfo.CharacterName);

	auto it = m_CharacterInfo.find(key);

	if (it != m_CharacterInfo.end())
	{
		m_CharacterInfo.erase(it);
	}
}

long CCharacterManager::GetCharacterCount()
{
	CCriticalSection::CLock lock(m_critical);

	return static_cast<long>(m_CharacterInfo.size());
}
