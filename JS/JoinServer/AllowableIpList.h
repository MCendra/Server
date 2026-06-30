// AllowableIpList.h:
#pragma once

struct ALLOWABLE_IP_INFO
{
	char IpAddr[16];
};

class CAllowableIpList
{
public:
	CAllowableIpList() = default;;
	virtual ~CAllowableIpList() = default;
	void Load(const char* path);
	bool CheckAllowableIp(const char* ip) const;
private:
	std::map<std::string,ALLOWABLE_IP_INFO> m_AllowableIpInfo;
};

extern CAllowableIpList gAllowableIpList;
