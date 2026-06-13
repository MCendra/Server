// IpManager.cpp
#include "Header.h"
#include "IpManager.h"

CIpManager gIpManager;

// Construction/Destruction

CIpManager::CIpManager()
{
}

CIpManager::~CIpManager()
{
}

bool CIpManager::CheckIpAddress(const std::string& IpAddress)
{
    // Verificar que el formato de la IP sea valido
    if (!IsValidIpAddress(IpAddress)) {
        return false;
    }

    // Buscar la direccion IP en el mapa de m_IpAddressInfo
    auto it = this->m_ipaddressinfo.find(IpAddress);

    // Si la direccion IP no esta registrada en el mapa, permitimos la conexion o no
    if (it == this->m_ipaddressinfo.end())
    {
        return MaxIpConnection == 0 ? false : true;
    }
    else
    {
        // Si la IP esta en el mapa, verificamos si su contador es menor que MaxIpConnection
        return (it->second < MaxIpConnection);
    }
}

void CIpManager::InsertIpAddress(const std::string& IpAddress)
{
    if (!IsValidIpAddress(IpAddress)) {
        return;
    }

    auto it = this->m_ipaddressinfo.find(IpAddress);

    if (it == this->m_ipaddressinfo.end())
    {
        // Insertar nueva IP con un contador inicial de 1
        this->m_ipaddressinfo[IpAddress] = 1;
    }
    else
    {
        // Incrementar el contador existente
        it->second++;
    }
}

void CIpManager::RemoveIpAddress(const std::string& IpAddress)
{
    // Verificar que el formato de la IP sea valido
    if (IpAddress.empty() || !IsValidIpAddress(IpAddress)) {
        return;
    }

    auto it = this->m_ipaddressinfo.find(IpAddress);

    if (it != this->m_ipaddressinfo.end())
    {
        if ((--it->second) == 0)
        {
            this->m_ipaddressinfo.erase(it);
        }
    }
}

bool CIpManager::IsValidIpAddress(const std::string& IpAddress)
{
    struct sockaddr_in sa = {};
    // inet_pton devuelve 1 si la direccion es valida, 0 si no lo es.
    return inet_pton(AF_INET, IpAddress.c_str(), &(sa.sin_addr)) == 1;
}
