// JoinServerProtocol.h
#pragma once
#include "Header.h"

// ================================================================
// JoinServer — recibe de GameServer, envía a GameServer
// ================================================================
// Paquetes más grandes: move auth con 4 authcodes ULONGLONG = ~80 bytes.
// GameServer puede acumular hasta su MAX_SEND_PACKET_SIZE en el side buffer.
// Asumimos GS tiene al menos 4096 de send → JS debe poder recibir 4096.

// Tamaño máximo de paquetes de recepción y envío
constexpr std::size_t MAX_RECV_PACKET_SIZE = 4096;		// Límite de seguridad de entrada: 4096 bytes
constexpr std::size_t MAX_SEND_PACKET_SIZE = 4096;		// Límite de seguridad de salida: 4096 bytes en un unico paquete
constexpr std::size_t MAX_SEND_SIDE_PACKET_SIZE = 16384;	// Límite de seguridad de salida: 16384 bytes en suma de partes de un paquete
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

#define CREATE_ACCOUNT_FAIL_ID				0
#define CREATE_ACCOUNT_SUCCESS				1
#define CREATE_ACCOUNT_FAIL_RESIDENT		2

// Packet Base

#pragma pack(push,1)
struct PBMSG_HEAD
{
	void set(BYTE packetHead,BYTE packetSize)
	{
		this->type = PACKET_C1;
		this->size = packetSize;
		this->head = packetHead;
	}

	void setE(BYTE packetHead,BYTE packetSize)
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
	void set(BYTE packetHead,BYTE packetSubHead,BYTE packetSize)
	{
		this->type = PACKET_C1;
		this->size = packetSize;
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	void setE(BYTE packetHead,BYTE packetSubHead,BYTE packetSize)
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
	void set(BYTE packetHead,WORD packetSize)
	{
		this->type = PACKET_C2;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
	}

	void setE(BYTE packetHead,WORD packetSize)
	{
		this->type = PACKET_C4;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
	}

	BYTE type;
	BYTE size[2];
	BYTE head;
};

struct PSWMSG_HEAD
{
	void set(BYTE packetHead,BYTE packetSubHead,WORD packetSize)
	{
		this->type = PACKET_C2;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	void setE(BYTE packetHead,BYTE packetSubHead,WORD packetSize)
	{
		this->type = PACKET_C4;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
		this->subh = packetSubHead;
	}

	BYTE type;
	BYTE size[2];
	BYTE head;
	BYTE subh;
};

// GameServer -> JoinServer

struct SDHP_SERVER_INFO_RECV
{
	PBMSG_HEAD header; // C1:00
	BYTE type;
	WORD ServerPort;
	char ServerName[50];
	WORD ServerCode;
};

struct SDHP_CONNECT_ACCOUNT_RECV
{
	PBMSG_HEAD header; // C1:01
	WORD index;
	char account[11];
	char password[11];
	char IpAddress[16];
};

struct SDHP_REGISTER_ACCOUNT_SEND
{
	PBMSG_HEAD header; // C1:01
	WORD index;
	char account[11];
	char password[11];
	char personalcode[11];
	char Email[50];
};

struct SDHP_DISCONNECT_ACCOUNT_RECV
{
	PBMSG_HEAD header; // C1:02
	WORD index;
	char account[11];
	char IpAddress[16];
};

struct SDHP_MAP_SERVER_MOVE_RECV
{
	PBMSG_HEAD header; // C1:03
	WORD index;
	char account[11];
	char name[11];
	WORD GameServerCode;
	WORD NextServerCode;
	WORD map;
	BYTE x;
	BYTE y;
};

struct SDHP_MAP_SERVER_MOVE_AUTH_RECV
{
	PBMSG_HEAD header; // C1:04
	WORD index;
	char account[11];
	char name[11];
	WORD LastServerCode;
	DWORD AuthCode1;
	DWORD AuthCode2;
	DWORD AuthCode3;
	DWORD AuthCode4;
};

struct SDHP_ACCOUNT_LEVEL_RECV
{
	PBMSG_HEAD header; // C1:05
	WORD index;
	char account[11];
};

struct SDHP_MAP_SERVER_MOVE_CANCEL_RECV
{
	PBMSG_HEAD header; // C1:10
	WORD index;
	char account[11];
};

struct SDHP_ACCOUNT_LEVEL_SAVE_RECV
{
	PBMSG_HEAD header; // C1:11
	WORD index;
	char account[11];
	WORD AccountLevel;
	DWORD AccountExpireTime;
};

struct SDHP_SERVER_USER_INFO_RECV
{
	PBMSG_HEAD header; // C1:20
	WORD CurUserCount;
	WORD MaxUserCount;
};

struct SDHP_EXTERNAL_DISCONNECT_ACCOUNT_RECV
{
	PBMSG_HEAD header; // C1:30
	char account[11];
};

struct SDHP_LOCK_SAVE_RECV
{
	PBMSG_HEAD header; // C1:11
	WORD index;
	char account[11];
	DWORD Lock;
};

// JoinServer -> GameServer

struct SDHP_SERVER_INFO_SEND
{
	PBMSG_HEAD header; // C1:00
	BYTE result;
	DWORD ItemCount;
};

struct SDHP_CONNECT_ACCOUNT_SEND
{
	PBMSG_HEAD header; // C1:01
	WORD index;
	char account[11];
	char PersonalCode[14];
	BYTE result;
	BYTE BlockCode;
	int AccountLevel;
	char AccountExpireDate[20];
	DWORD Lock;
};

struct SDHP_DISCONNECT_ACCOUNT_SEND
{
	PBMSG_HEAD header; // C1:02
	WORD index;
	char account[11];
	BYTE result;
};

struct SDHP_MAP_SERVER_MOVE_SEND
{
	PBMSG_HEAD header; // C1:03
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	int GameServerCode;
	int NextServerCode;
	WORD map;
	BYTE x;
	BYTE y;
	ULONGLONG AuthCode1;
	ULONGLONG AuthCode2;
	ULONGLONG AuthCode3;
	ULONGLONG AuthCode4;
};

struct SDHP_MAP_SERVER_MOVE_AUTH_SEND
{
	PBMSG_HEAD header; // C1:04
	WORD index;
	char account[11];
	char name[11];
	char PersonalCode[14];
	BYTE result;
	BYTE BlockCode;
	int LastServerCode;
	WORD map;
	BYTE x;
	BYTE y;
	int AccountLevel;
	char AccountExpireDate[20];
	DWORD Lock;
};

struct SDHP_ACCOUNT_LEVEL_SEND
{
	PBMSG_HEAD header; // C1:05
	WORD index;
	char account[11];
	int AccountLevel;
	char AccountExpireDate[20];
};

struct SDHP_ACCOUNT_ALREADY_CONNECTED_SEND
{
	PBMSG_HEAD header; // C1:30
	WORD index;
	char account[11];
};

struct SDHP_JOIN_SERVER_LIVE_SEND
{
	PBMSG_HEAD header; // C1:02
	DWORD QueueSize;
};
#pragma pack(pop)

void GJRegisterAccountRecv(SDHP_REGISTER_ACCOUNT_SEND* lpMsg,int index);
void JoinServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size);
void GJServerInfoRecv(SDHP_SERVER_INFO_RECV* lpMsg,int index);
void GJConnectAccountRecv(SDHP_CONNECT_ACCOUNT_RECV* lpMsg,int index);
void GJDisconnectAccountRecv(SDHP_DISCONNECT_ACCOUNT_RECV* lpMsg,int index);
void GJMapServerMoveRecv(SDHP_MAP_SERVER_MOVE_RECV* lpMsg,int index);
void GJMapServerMoveAuthRecv(SDHP_MAP_SERVER_MOVE_AUTH_RECV* lpMsg,int index);
void GJAccountLevelRecv(SDHP_ACCOUNT_LEVEL_RECV* lpMsg,int index);
void GJMapServerMoveCancelRecv(SDHP_MAP_SERVER_MOVE_CANCEL_RECV* lpMsg,int index);
void GJAccountLevelSaveRecv(SDHP_ACCOUNT_LEVEL_SAVE_RECV* lpMsg,int index);
void GJServerUserInfoRecv(SDHP_SERVER_USER_INFO_RECV* lpMsg,int index);
void GJExternalDisconnectAccountRecv(SDHP_EXTERNAL_DISCONNECT_ACCOUNT_RECV* lpMsg,int index);
void JGExternalDisconnectAccountSend(int GameServerCode,WORD UserIndex, const char* account);
void JGAccountAlreadyConnectedSend(int GameServerCode,WORD UserIndex, const char* account);
void GJAccountLevelRecv2(SDHP_ACCOUNT_LEVEL_RECV* lpMsg,int index);
void GJAccountLockSaveRecv(SDHP_LOCK_SAVE_RECV* lpMsg,int index);
void JoinServerLiveProc();
