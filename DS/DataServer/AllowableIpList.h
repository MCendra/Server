// AllowableIpList.h:
#pragma once

#pragma pack(push,1)
struct ALLOWABLE_IP_INFO
{
	char IpAddr[16];
};
#pragma pack(pop)

class CAllowableIpList
{
public:
	CAllowableIpList() = default;
	~CAllowableIpList() = default;
	void Load(const char* path);
	bool CheckAllowableIp(const char* ip) const;
private:
	std::map<std::string, ALLOWABLE_IP_INFO> m_AllowableIpInfo;
};

extern CAllowableIpList gAllowableIpList;
