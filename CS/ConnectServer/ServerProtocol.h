// ServerProtocol.h
#pragma once
#include "Header.h"
#include <array>

#define MAX_MAIN_PACKET_SIZE 2048
#define MAX_UDP_PACKET_SIZE 4096

// Encabezados de paquetes
#define PACKET_HEADER_C1 0xC1
#define PACKET_HEADER_C2 0xC2
#define PACKET_HEADER_C3 0xC3
#define PACKET_HEADER_C4 0xC4

// Tamaños maximos permitidos de paquetes
#define PACKET_TYPE_C1_MAX_SIZE 255
#define PACKET_TYPE_C2_MAX_SIZE MAX_MAIN_PACKET_SIZE

#define DEFAULT_TIME_WAIT 5000
#define DEFAULT_BACKLOG 5

constexpr BYTE SET_NUMBERHB(DWORD x) {
	return static_cast<BYTE>(x >> 8);
}

constexpr BYTE SET_NUMBERLB(DWORD x) {
	return static_cast<BYTE>(x & 0xFF);
}

constexpr WORD SET_NUMBERHW(DWORD x) {
	return static_cast<WORD>(x >> 16);
}

constexpr WORD SET_NUMBERLW(DWORD x) {
	return static_cast<WORD>(x & 0xFFFF);
}

constexpr DWORD SET_NUMBERHDW(QWORD x) {
	return static_cast<DWORD>(x >> 32);
}

constexpr DWORD SET_NUMBERLDW(QWORD x) {
	return static_cast<DWORD>(x & 0xFFFFFFFF);
}

constexpr WORD MAKE_NUMBERW(BYTE x, BYTE y) {
	return static_cast<WORD>((static_cast<WORD>(y) & 0xFF) | (static_cast<WORD>(x) << 8));
}

constexpr DWORD MAKE_NUMBERDW(WORD x, WORD y) {
	return static_cast<DWORD>((static_cast<DWORD>(y) & 0xFFFF) | (static_cast<DWORD>(x) << 16));
}

constexpr QWORD MAKE_NUMBERQW(DWORD x, DWORD y) {
	return (static_cast<QWORD>(y) & 0xFFFFFFFF) | (static_cast<QWORD>(x) << 32);
}

// Definiciones para las cabeceras de los mensajes
constexpr BYTE MSG_HEADER_TYPE_F4 = 0xF4; // Agregamos esta definicion
constexpr BYTE MSG_HEADER_SERVER_INFO_RECV = 0x03;
constexpr BYTE MSG_HEADER_SERVER_LIST_RECV = 0x06;
constexpr BYTE MSG_HEADER_SERVER_INIT_SEND = 0x00;

// Packet Base

struct PBMSG_HEAD
{
	void set(BYTE packetHead, BYTE packetSize)
	{
		this->type = PACKET_HEADER_C1;
		this->size = packetSize;
		this->head = packetHead;
	}

	void setE(BYTE packetHead, BYTE packetSize)
	{
		this->type = PACKET_HEADER_C3;
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
		this->type = PACKET_HEADER_C1;
		this->size = packetSize;
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	void setE(BYTE packetHead, BYTE packetSubHead, BYTE packetSize)
	{
		this->type = PACKET_HEADER_C3;
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
		this->type = PACKET_HEADER_C2;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHeadh;
	}

	void setE(BYTE packetHead, WORD packetSize) {
		this->type = PACKET_HEADER_C4;
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
		this->type = PACKET_HEADER_C2;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	void setE(BYTE packetHead, BYTE packetSubHead, WORD packetSize)
	{
		this->type = PACKET_HEADER_C4;
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

void ConnectServerProtocolCore(int index, BYTE head, BYTE* lpMsg, int size);
void CCServerInfoRecv(PMSG_SERVER_INFO_RECV* lpMsg, int index);
void CCServerListRecv(PMSG_SERVER_LIST_RECV* lpMsg, int index);
void CCServerInitSend(int index, int result);
