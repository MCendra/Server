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
    std::map<std::string, int> m_ipaddressinfo;  // Cambiado a int para el contador
    CCriticalSection m_lock; // Protege acceso a m_ipaddressinfo
};

extern CIpManager gIpManager;
