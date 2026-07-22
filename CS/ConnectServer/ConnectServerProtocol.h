// ConnectServerProtocol.h
#pragma once
#include "Header.h"
#include <array>

// ================================================================
// ConnectServer — recibe de clientes y de JoinServer (UDP)
// ================================================================
// Clientes: paquetes C1 pequeños (server list, init), máx 50 bytes.
// JoinServer UDP: heartbeat de 8 bytes.
// Nunca recibe paquetes grandes.

// Tamaño máximo de paquetes de recepción y envío
constexpr std::size_t MAX_RECV_PACKET_SIZE = 512;		// Límite de seguridad de entrada: 512 bytes
constexpr std::size_t MAX_SEND_PACKET_SIZE = 2048;		// Límite de seguridad de salida: 2048 bytes en un unico paquete
constexpr std::size_t MAX_SEND_SIDE_PACKET_SIZE = 8192;	// Límite de seguridad de salida: 8192 bytes en suma de partes de un paquete
constexpr std::size_t MAX_UDP_PACKET_SIZE = 8192;		// Límite de seguridad de salida: 8192 bytes entrada y salida en un unico paquete UDP

// Common Packet Offsets
constexpr std::size_t PACKET_TYPE_OFFSET = 0;

// Cabeceras C1 / C3 
constexpr std::size_t C1_PACKET_SIZE_OFFSET = 1;
constexpr std::size_t C1_PACKET_HEAD_OFFSET = 2;
constexpr std::size_t C1_PACKET_DATA_OFFSET = 3;

// Cabeceras C2 / C4
constexpr std::size_t C2_PACKET_SIZEH_OFFSET = 1;
constexpr std::size_t C2_PACKET_SIZEL_OFFSET = 2;
constexpr std::size_t C2_PACKET_HEAD_OFFSET = 3;
constexpr std::size_t C2_PACKET_DATA_OFFSET = 4;

// Tipos de paquetes
constexpr BYTE PACKET_C1 = 0xC1;
constexpr BYTE PACKET_C2 = 0xC2;
constexpr BYTE PACKET_C3 = 0xC3;
constexpr BYTE PACKET_C4 = 0xC4;

//Tamaño de cabeceras de paquetes
constexpr std::size_t PACKET_C1_C3_HEADER_SIZE = 3;
constexpr std::size_t PACKET_C2_C4_HEADER_SIZE = 4;

// Tamaños maximos permitidos de paquetes
constexpr std::size_t  PACKET_TYPE_C1_MAX_SIZE = 255;
constexpr std::size_t  PACKET_TYPE_C2_MAX_SIZE = MAX_RECV_PACKET_SIZE;

// Definiciones para las cabeceras de los mensajes
constexpr BYTE MSG_HEADER_TYPE_F4 = 0xF4; // Agregamos esta definicion
constexpr BYTE MSG_HEADER_SERVER_INFO_RECV = 0x03;
constexpr BYTE MSG_HEADER_SERVER_LIST_RECV = 0x06;
constexpr BYTE MSG_HEADER_SERVER_INIT_SEND = 0x00;

// Packet Base

#pragma pack(push,1)
struct PBMSG_HEAD
{
	void set(BYTE packetHead, BYTE packetSize)
	{
		this->type = PACKET_C1;
		this->size = packetSize;
		this->head = packetHead;
	}

	void setE(BYTE packetHead, BYTE packetSize)
	{
		this->type = PACKET_C3;
		this->size = packetSize;
		this->head = packetHead;
	}

	BYTE type;
	BYTE size;
	BYTE head;
};

struct PSBMSG_HEAD
{
	void set(BYTE packetHead, BYTE packetSubHead, BYTE packetSize)
	{
		this->type = PACKET_C1;
		this->size = packetSize;
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	void setE(BYTE packetHead, BYTE packetSubHead, BYTE packetSize)
	{
		this->type = PACKET_C3;
		this->size = packetSize;
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	BYTE type;
	BYTE size;
	BYTE head;
	BYTE subh;
};

struct PWMSG_HEAD
{
	void set(BYTE packetHeadh, WORD packetSize)
	{
		this->type = PACKET_C2;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHeadh;
	}

	void setE(BYTE packetHead, WORD packetSize) {
		this->type = PACKET_C4;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
	}

	BYTE type;
	std::array<BYTE, 2> size;
	BYTE head;
};

struct PSWMSG_HEAD {
	void set(BYTE packetHead, BYTE packetSubHead, WORD packetSize)
	{
		this->type = PACKET_C2;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	void setE(BYTE packetHead, BYTE packetSubHead, WORD packetSize)
	{
		this->type = PACKET_C4;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	BYTE type;
	std::array<BYTE, 2> size;
	BYTE head;
	BYTE subh;
};

// Client -> ConnectServer

struct PMSG_SERVER_INFO_RECV {
	PSBMSG_HEAD header; // C1:F4:03
	BYTE ServerCode;
};

struct PMSG_SERVER_LIST_RECV {
	PSBMSG_HEAD header; // C1:F4:06
};

// ConnectServer -> Client

struct PMSG_SERVER_INIT_SEND {
	PBMSG_HEAD header; // C1:00
	BYTE result;
};

struct PMSG_SERVER_INFO_SEND {
	PSBMSG_HEAD header; // C1:F4:03
	char ServerAddress[16];
	WORD ServerPort;
};

struct PMSG_SERVER_LIST_SEND {
	PSWMSG_HEAD header; // C1:F4:06
	std::array<BYTE, 2> count;
};

struct PMSG_SERVER_LIST {
	WORD ServerCode;
	BYTE UserTotal;
	BYTE type;
};
#pragma pack(pop)

void ConnectServerProtocolCore(int index, BYTE head, BYTE* lpMsg, int size);
void CCServerInfoRecv(PMSG_SERVER_INFO_RECV* lpMsg, int index);
void CCServerListRecv(PMSG_SERVER_LIST_RECV* lpMsg, int index);
void CCServerInitSend(int index, int result);
