// IpManager.h
#pragma once

struct IP_ADDRESS_INFO
{
	char IpAddress[16];
	WORD IpAddressCount;
	WORD IpFloodAttemps;
	WORD IpBlocked;
	ULONGLONG IpBlockedTime;
	ULONGLONG IpTick;
	ULONGLONG IpFloodLastTime;
	WORD IpRealUser;
};

struct IP_ADDRESS_BLOCK
{
	char IpAddress[16];
};

class CIpManager
{
public:
	CIpManager() = default;
	~CIpManager() = default;
	bool CheckIpAddress(char* IpAddress);
	void InsertIpAddress(char* IpAddress);
	void RemoveIpAddress(char* IpAddress);
	void InsertRealUser(char* IpAddress);
	std::map<std::string, IP_ADDRESS_INFO> m_IpAddressInfo;
private:
};

extern CIpManager gIpManager;
