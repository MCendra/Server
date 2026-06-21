// IpManager.h
#pragma once
#include <map>
#include <string>
#include "CriticalSection.h"

class CIpManager
{
public:
    CIpManager();
    virtual ~CIpManager();
    bool CheckIpAddress(const std::string& IpAddress);
    void InsertIpAddress(const std::string& IpAddress);
    void RemoveIpAddress(const std::string& IpAddress);

private:
    bool IsValidIpAddress(const std::string& IpAddress);
	// Cambiado a int para el contador
    std::map<std::string, int> m_ipaddressinfo;
	// Protege acceso a m_ipaddressinfo
    CCriticalSection m_lock;
};

extern CIpManager gIpManager;
