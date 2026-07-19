// Header.h
#pragma once

// Incluir archivos de inclusion estandar del sistema y archivos especificos del proyecto
#include "TargetVer.h"

// Excluir material rara vez utilizado de encabezados de Windows
#define WIN32_LEAN_AND_MEAN             

// Constantes del proyecto
constexpr size_t MAX_LOADSTRING = 100;

// Define la version del servidor de conexion
constexpr auto DATASERVER_VERSION = "1.0.0";

constexpr auto SERVER_PART = "DATA SERVER";

constexpr size_t MAX_CHARACTER_NAME = 11;
constexpr size_t MAX_ACCOUNT_NAME = 11;
constexpr size_t MAX_GUILD_NAME = 9;
constexpr size_t GUILD_MARK_SIZE = 32;
constexpr size_t GUILD_NOTICE_SIZE = 60;
constexpr size_t GUILD_CHAT_SIZE = 60;
constexpr size_t MAX_MSG_CHAT_SIZE = 60;
constexpr size_t MAX_UNION_GUILDS = 100;

// Define el tipo de servidor de juegos
#define GAMESERVER_TYPE2 3

// Define el nombre del cliente basado en el tipo de servidor de juegos
#if (GAMESERVER_TYPE2 == 0)
#define DATASERVER_CLIENT "VIERJA&HIGHLEVEL"
#elif (GAMESERVER_TYPE2 == 1)
#define DATASERVER_CLIENT "HAI DAI SU"
#elif (GAMESERVER_TYPE2 == 2)
#define DATASERVER_CLIENT "REAL GAMING"
#else
#define DATASERVER_CLIENT "ARKANIA"
#endif

// Define la version de actualizacion del servidor de conexion si no se ha definido previamente
#ifndef DATASERVER_UPDATE
#define DATASERVER_UPDATE 803
#endif

#define RECHARGEREWARD			true
#define DANHHIEU_NEW		1
#define TULUYEN_NEW			1
#define CHIEN_TRUONG_CO		1
#define NEWBOSSGUILD		1
#define ENABLECUSTOMRANKING		false
#define NEWRANKING				true
#define TOP1RESETLIMIT			true
#define ENABLECUSTOMQUEST				0 
#define ANCIENTBATTLEFIELDEVENT	true
#define CUSTOMELEMENTALBOOK		true
#define CHONPHEDOILAP		1
#define B_HON_HOAN			1
#define ENDLESSTOWEREVENT		true
#define FLAG_SKIN			1
#ifndef TOP1BOTSTATUE
	#define TOP1BOTSTATUE		true
#endif

// Necesario por el uso de WSASocketA 
// Necesario por el uso de WSASocketA 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>		// Necesario para inet_ntop

// Archivos de encabezado de Windows
#include <Windows.h>    // Archivo de encabezado de la API de Windows

#include <Rpc.h>
#include <Psapi.h>

// Archivos de encabezado en tiempo de ejecución de C
#include <stdlib.h>     // Utilidades generales: memoria dinámica, random, exit, etc.
#include <malloc.h>     // Asignación de memoria: malloc, free, etc. (específico de algunas plataformas)
#include <memory.h>     // Manipulación de memoria (alias de <string.h> en algunas plataformas)
#include <tchar.h>      // Soporte para caracteres Unicode/ANSI (útil en aplicaciones que usan TCHAR)

// Archivos de encabezado en tiempo de ejecución de C++
#include <string>       // Manejo de cadenas en C++
#include <cstdint>		// Proporciona definiciones de tipos enteros con tamaños fijos

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <process.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
#include <assert.h>
#include <fstream>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"Psapi.lib")

// Define un tipo de dato específico (QWORD es un entero sin signo de 64 bits)
// using BYTE = uint8_t;
// using WORD = uint16_t;
// No redefinir DWORD aquí, usar el de windows.h
// using DWORD = uint32_t;
using QWORD = uint64_t;

// Definición del path de trabajo
extern char* WorkingPath;

// Definicion de archivos de configuracion
extern char ConfigFilePath[MAX_PATH];
extern char AllowableIpListFilePath[MAX_PATH];
extern char BadSyntaxFilePath[MAX_PATH];

// Declaraciones de variables de configuracion
extern char CustomerName[32];
extern char CustomerHardwareId[36];
extern WORD DataServerPort;
extern char DataServerODBC[32];
extern char DataServerUSER[32];
extern char DataServerPASS[32];
extern BOOL AdvancedLog;
extern WORD RSTimeCTC;

//==============================================================
// Common Packet Offsets
//==============================================================

constexpr std::size_t PACKET_TYPE_OFFSET = 0;

//==============================================================
// C1 / C3 Header
//==============================================================

constexpr std::size_t C1_PACKET_SIZE_OFFSET = 1;
constexpr std::size_t C1_PACKET_HEAD_OFFSET = 2;
constexpr std::size_t C1_PACKET_DATA_OFFSET = 3;

//==============================================================
// C2 / C4 Header
//==============================================================

constexpr std::size_t C2_PACKET_SIZEH_OFFSET = 1;
constexpr std::size_t C2_PACKET_SIZEL_OFFSET = 2;
constexpr std::size_t C2_PACKET_HEAD_OFFSET = 3;
constexpr std::size_t C2_PACKET_DATA_OFFSET = 4;

//==============================================================
// Packet Types
//==============================================================

constexpr BYTE PACKET_C1 = 0xC1;
constexpr BYTE PACKET_C2 = 0xC2;
constexpr BYTE PACKET_C3 = 0xC3;
constexpr BYTE PACKET_C4 = 0xC4;

//==============================================================
// Main Heads
//==============================================================

constexpr BYTE HEAD_GLOBAL_ITEM_COUNT = 0x00;
constexpr BYTE HEAD_CHARACTER_LIST = 0x01;
constexpr BYTE HEAD_CHARACTER_CREATE = 0x02;
constexpr BYTE HEAD_CHARACTER_DELETE = 0x03;
constexpr BYTE HEAD_CHARACTER_INFO = 0x04;
constexpr BYTE HEAD_WAREHOUSE = 0x05;
constexpr BYTE HEAD_CREATE_ITEM = 0x07;
constexpr BYTE HEAD_OPTION_DATA = 0x08;
constexpr BYTE HEAD_PET_ITEM = 0x09;
constexpr BYTE HEAD_CHARACTER_NAME_CHECK = 0x0A;
constexpr BYTE HEAD_CHARACTER_RENAME = 0x0B;
constexpr BYTE HEAD_QUEST = 0x0C;
constexpr BYTE HEAD_MASTER_SKILL = 0x0D;
constexpr BYTE HEAD_NPC = 0x0E;
constexpr BYTE HEAD_COMMAND = 0x0F;
constexpr BYTE HEAD_QUEST_WORLD = 0x10;
constexpr BYTE HEAD_GENS_SYSTEM = 0x11;
constexpr BYTE HEAD_MURUMMY = 0x12;
constexpr BYTE HEAD_HELPER = 0x17;
constexpr BYTE HEAD_CASHSHOP = 0x18;
constexpr BYTE HEAD_PCPOINT = 0x19;
constexpr BYTE HEAD_LUCKY_COIN = 0x1A;
constexpr BYTE HEAD_RESERVED_1B = 0x1B; // Reserved
constexpr BYTE HEAD_CRYWOLF_SYNC = 0x1E;
constexpr BYTE HEAD_CRYWOLF_INFO = 0x1F;
constexpr BYTE HEAD_GLOBAL_POST = 0x20;
constexpr BYTE HEAD_GLOBAL_NOTICE = 0x21;
constexpr BYTE HEAD_LUCKY_ITEM = 0x22;
constexpr BYTE HEAD_PENTAGRAM_SYSTEM = 0x23;
constexpr BYTE HEAD_SNS_DATA = 0x24;
constexpr BYTE HEAD_PERSONAL_SHOP = 0x25;
constexpr BYTE HEAD_EVENT_INVENTORY = 0x26;
constexpr BYTE HEAD_MUUN_SYSTEM = 0x27;
constexpr BYTE HEAD_GUILD_MATCHING = 0x28;
constexpr BYTE HEAD_PARTY_MATCHING = 0x29;
constexpr BYTE HEAD_CHARACTER_INFO_SAVE = 0x30;
constexpr BYTE HEAD_INVENTORY_ITEM_SAVE = 0x31;
constexpr BYTE HEAD_OPTION_DATA_SAVE = 0x33;
constexpr BYTE HEAD_PET_ITEM_INFO_SAVE = 0x34;
constexpr BYTE HEAD_RESET_INFO_SAVE = 0x39;
constexpr BYTE HEAD_MASTER_RESET_INFO_SAVE = 0x3A;
constexpr BYTE HEAD_RANKING_DUEL_SAVE = 0x3C;
constexpr BYTE HEAD_RANKING_BLOOD_CASTLE_SAVE = 0x3D;
constexpr BYTE HEAD_RANKING_CHAOS_CASTLE_SAVE = 0x3E;
constexpr BYTE HEAD_RANKING_DEVIL_SQUARE_SAVE = 0x3F;
constexpr BYTE HEAD_RANKING_ILLUSION_TEMPLE_SAVE = 0x40;
constexpr BYTE HEAD_CREATION_CARD_SAVE = 0x42;
constexpr BYTE HEAD_CRYWOLF_INFO_SAVE = 0x49;
constexpr BYTE HEAD_SNS_DATA_SAVE = 0x4E;
constexpr BYTE HEAD_CUSTOM_MONSTER_REWARD_SAVE = 0x52;
constexpr BYTE HEAD_RANKING_CUSTOM_ARENA_SAVE = 0x55;
constexpr BYTE HEAD_RANKING_TVT_EVENT_SAVE = 0x56;
constexpr BYTE HEAD_CONNECT_CHARACTER = 0x70;
constexpr BYTE HEAD_DISCONNECT_CHARACTER = 0x71;
constexpr BYTE HEAD_GLOBAL_WHISPER = 0x72;
constexpr BYTE HEAD_GLOBAL_WHISPER_ECHO = 0x73;
constexpr BYTE HEAD_RANKING_KING_GUILD_SAVE = 0x74;
constexpr BYTE HEAD_RANKING_KING_PLAYER_SAVE = 0x75;
constexpr BYTE HEAD_GLOBAL_ITEM_POST = 0x78;
constexpr BYTE HEAD_CASTLE_SIEGE = 0x80;
constexpr BYTE HEAD_CASTLE_INIT_DATA = 0x81;
constexpr BYTE HEAD_CASTLE_NPC_INFO = 0x82;
constexpr BYTE HEAD_CASTLE_ALL_GUILD_MARK = 0x83;
constexpr BYTE HEAD_CASTLE_FIRST_CREATE_NPC = 0x84;
constexpr BYTE HEAD_CASTLE_CALC_REG_GUILD = 0x85;
constexpr BYTE HEAD_CASTLE_GUILD_UNION_INFO = 0x86;
constexpr BYTE HEAD_CASTLE_SAVE_GUILD_INFO = 0x87;
constexpr BYTE HEAD_CASTLE_LOAD_GUILD_INFO = 0x88;
constexpr BYTE HEAD_CASTLE_NPC_UPDATE = 0x89;
constexpr BYTE HEAD_CUSTOM_PROTOCOL = 0xB0; // Custom
constexpr BYTE HEAD_EXDB_PROTOCOL = 0xE0;
constexpr BYTE HEAD_CONNECT_SERVER = 0xE1;
constexpr BYTE HEAD_MARRY_INFO_SAVE = 0xF0;
constexpr BYTE HEAD_CUSTOM_QUEST = 0xF1;
constexpr BYTE HEAD_CUSTOM_QUEST_SAVE = 0xF2;
constexpr BYTE HEAD_SET_COIN = 0xF3;
constexpr BYTE HEAD_CUSTOM_RANKING = 0xF4;
constexpr BYTE HEAD_ATTACK_RESUME = 0xF5;
constexpr BYTE HEAD_ATTACK_RESUME_SAVE = 0xF6;
constexpr BYTE HEAD_CUSTOM_NPC_QUEST = 0xF7;
constexpr BYTE HEAD_CUSTOM_GHRS = 0xF8;
constexpr BYTE HEAD_BOT_INFO = 0xF9;
constexpr BYTE HEAD_CUSTOM_EXTENSIONS = 0xD9;

//==============================================================
// ExDB Protocol Heads 0xE1 ESProtocol.cpp
//==============================================================
constexpr BYTE EXDB_HEAD_CHAR_CLOSE = 0x02;
constexpr BYTE EXDB_HEAD_GUILD_CREATE = 0x30;
constexpr BYTE EXDB_HEAD_GUILD_DESTROY = 0x31;
constexpr BYTE EXDB_HEAD_GUILD_MEMBER_ADD = 0x32;
constexpr BYTE EXDB_HEAD_GUILD_MEMBER_DEL = 0x33;
constexpr BYTE EXDB_HEAD_GUILD_MEMBER_INFO = 0x35;
constexpr BYTE EXDB_HEAD_GUILD_MASTER_LIST = 0x36;
constexpr BYTE EXDB_HEAD_GUILD_SCORE_UPDATE = 0x37;
constexpr BYTE EXDB_HEAD_GUILD_NOTICE = 0x38;
#if (NEWBOSSGUILD == 1)
constexpr BYTE EXDB_HEAD_GUILD_SCORE_UPDATE_EX = 0x39;
#endif
constexpr BYTE EXDB_HEAD_GUILD_CHAT = 0x50;
constexpr BYTE EXDB_HEAD_UNION_CHAT = 0x51;
constexpr BYTE EXDB_HEAD_ASSIGN_STATUS = 0xE1;
constexpr BYTE EXDB_HEAD_ASSIGN_TYPE = 0xE2;
constexpr BYTE EXDB_HEAD_RELATIONSHIP_JOIN = 0xE5;
constexpr BYTE EXDB_HEAD_RELATIONSHIP_BREAK = 0xE6;
constexpr BYTE EXDB_HEAD_RELATIONSHIP_LIST = 0xE7;
constexpr BYTE EXDB_HEAD_RELATIONSHIP_NOTIFICATION = 0xE8;
constexpr BYTE EXDB_HEAD_UNION_LIST = 0xE9;
constexpr BYTE EXDB_HEAD_KICKOUT_UNION_MEMBER = 0xEB;
//==============================================================
// ExDB SubHeads
//==============================================================
constexpr BYTE EXDB_SUB_HEAD_KICKOUT_UNION_MEMBER = 0x01;

//==============================================================
// ConnectServer Protocol Heads (0xE1)
//==============================================================
constexpr BYTE CS_HEAD_FRIEND_LIST			= 0x60;
constexpr BYTE CS_HEAD_WAIT_FRIEND_LIST		= 0x61;
constexpr BYTE CS_HEAD_FRIEND_STATE			= 0x62;
constexpr BYTE CS_HEAD_FRIEND_ADD			= 0x63;
constexpr BYTE CS_HEAD_WAIT_FRIEND_ADD		= 0x64;
constexpr BYTE CS_HEAD_FRIEND_DELETE		= 0x65;
constexpr BYTE CS_HEAD_CHATROOM_CREATE		= 0x66;
constexpr BYTE CS_HEAD_MEMO_SEND			= 0x70;
constexpr BYTE CS_HEAD_MEMO_LIST			= 0x71;
constexpr BYTE CS_HEAD_MEMO_READ			= 0x72;
constexpr BYTE CS_HEAD_MEMO_DELETE			= 0x73;
constexpr BYTE CS_HEAD_FRIEND_INVITATION	= 0x74;
//**********************************************
// Warehouse System SubHeads (0x05)
//**********************************************
constexpr BYTE SUB_WAREHOUSE_PERSONAL_LOAD = 0x00;
constexpr BYTE SUB_WAREHOUSE_PERSONAL_FREE = 0x01;
constexpr BYTE SUB_WAREHOUSE_PERSONAL_SAVE = 0x30;
constexpr BYTE SUB_WAREHOUSE_GUILD_LOAD = 0x70;
constexpr BYTE SUB_WAREHOUSE_GUILD_FREE = 0x71;
constexpr BYTE SUB_WAREHOUSE_GUILD_SAVE = 0x75;
constexpr BYTE SUB_WAREHOUSE_GUILD_STATUS = 0x76;
constexpr BYTE SUB_WAREHOUSE_GUILD_UPDATE = 0x77;
//**********************************************
// Gens System SubHeads (0x11)
//**********************************************
constexpr BYTE SUB_GENS_SYSTEM_INSERT		= 0x00;
constexpr BYTE SUB_GENS_SYSTEM_DELETE		= 0x01;
constexpr BYTE SUB_GENS_SYSTEM_MEMBER		= 0x02;
constexpr BYTE SUB_GENS_SYSTEM_UPDATE		= 0x03;
constexpr BYTE SUB_GENS_SYSTEM_REWARD		= 0x04;
constexpr BYTE SUB_GENS_SYSTEM_REWARD_SAVE	= 0x30;
constexpr BYTE SUB_GENS_SYSTEM_CREATE		= 0x70;
//**********************************************
// Quest SubHeads (0x0C)
//**********************************************
constexpr BYTE SUB_QUEST_KILLCOUNT_LOAD = 0x00;
constexpr BYTE SUB_QUEST_KILLCOUNT_SAVE = 0x30;
//**********************************************
// Master Skill Tree SubHeads (0x0D)
//**********************************************
constexpr BYTE SUB_MASTER_SKILL_LOAD = 0x00;
constexpr BYTE SUB_MASTER_SKILL_SAVE = 0x30;
//**********************************************
// NPC SubHeads (0x0E)
//**********************************************
constexpr BYTE SUB_NPC_LEO_LOAD = 0x00;
constexpr BYTE SUB_NPC_SANTA_LOAD = 0x01;
constexpr BYTE SUB_NPC_LEO_SAVE = 0x30;
constexpr BYTE SUB_NPC_SANTA_SAVE = 0x31;
//**********************************************
// Command SubHeads (0x0F)
//**********************************************
constexpr BYTE SUB_COMMAND_RESET = 0x00;
constexpr BYTE SUB_COMMAND_MASTER_RESET = 0x01;
constexpr BYTE SUB_COMMAND_MARRY = 0x02;
constexpr BYTE SUB_COMMAND_REWARD = 0x03;
constexpr BYTE SUB_COMMAND_REWARD_ALL = 0x04;
constexpr BYTE SUB_COMMAND_RENAME = 0x05;
constexpr BYTE SUB_COMMAND_BLOCK_ACCOUNT = 0x06;
constexpr BYTE SUB_COMMAND_BLOCK_CHARACTER = 0x07;
constexpr BYTE SUB_COMMAND_GIFT = 0x08;
constexpr BYTE SUB_COMMAND_TOP = 0x09;
//**********************************************
// Quest World SubHeads (0x10)
//**********************************************
constexpr BYTE SUB_QUEST_WORLD_LOAD = 0x00;
constexpr BYTE SUB_QUEST_WORLD_SAVE = 0x30;
//**********************************************
// MuRummy SubHeads (0x12)
//**********************************************
constexpr BYTE SUB_MURUMMY_LOAD = 0x00;
constexpr BYTE SUB_MURUMMY_INSERT = 0x30;
constexpr BYTE SUB_MURUMMY_UPDATE = 0x31;
constexpr BYTE SUB_MURUMMY_SCORE_UPDATE = 0x32;
constexpr BYTE SUB_MURUMMY_DELETE = 0x33;
constexpr BYTE SUB_MURUMMY_SLOT_UPDATE = 0x34;
constexpr BYTE SUB_MURUMMY_INFO_UPDATE = 0x35;
//**********************************************
// Helper SubHeads (0x17)
//**********************************************
constexpr BYTE SUB_HELPER_LOAD = 0x00;
constexpr BYTE SUB_HELPER_SAVE = 0x30;
//**********************************************
// CashShop SubHeads (0x18)
//**********************************************
constexpr BYTE SUB_CASHSHOP_POINT = 0x00;
constexpr BYTE SUB_CASHSHOP_ITEM_BUY = 0x01;
constexpr BYTE SUB_CASHSHOP_ITEM_GIFT = 0x02;
constexpr BYTE SUB_CASHSHOP_ITEM_LIST = 0x03;
constexpr BYTE SUB_CASHSHOP_ITEM_USE = 0x04;
constexpr BYTE SUB_CASHSHOP_PERIODIC_ITEM = 0x05;
constexpr BYTE SUB_CASHSHOP_RECIEVE_POINT = 0x06;
constexpr BYTE SUB_CASHSHOP_ADD_POINT_SAVE = 0x30;
constexpr BYTE SUB_CASHSHOP_SUB_POINT_SAVE = 0x31;
constexpr BYTE SUB_CASHSHOP_INSERT_ITEM_SAVE = 0x32;
constexpr BYTE SUB_CASHSHOP_DELETE_ITEM_SAVE = 0x33;
constexpr BYTE SUB_CASHSHOP_PERIODIC_ITEM_SAVE = 0x34;
//**********************************************
// PC Point SubHeads (0x19)
//**********************************************
constexpr BYTE SUB_PCPOINT_POINT = 0x00;
constexpr BYTE SUB_PCPOINT_ITEM_BUY = 0x01;
constexpr BYTE SUB_PCPOINT_RECIEVE_POINT = 0x02;
constexpr BYTE SUB_PCPOINT_ADD_POINT_SAVE = 0x30;
constexpr BYTE SUB_PCPOINT_SUB_POINT_SAVE = 0x31;
//**********************************************
// Lucky Coin SubHeads (0x1A)
//**********************************************
constexpr BYTE SUB_LUCKY_COIN_COUNT = 0x00;
constexpr BYTE SUB_LUCKY_COIN_REGISTER = 0x01;
constexpr BYTE SUB_LUCKY_COIN_EXCHANGE = 0x02;
constexpr BYTE SUB_LUCKY_COIN_ADD_COUNT_SAVE = 0x30;
constexpr BYTE SUB_LUCKY_COIN_SUB_COUNT_SAVE = 0x31;
//**********************************************
// Reserved SubHeads (0x1B)
//**********************************************
constexpr BYTE SUB_RESERVED_1B = 0x00;
//**********************************************
// Lucky Item SubHeads (0x22)
//**********************************************
constexpr BYTE SUB_LUCKY_ITEM_LOAD = 0x00;
constexpr BYTE SUB_LUCKY_ITEM_SAVE = 0x4A;
//**********************************************
// Pentagram System SubHeads (0x23)
//**********************************************
constexpr BYTE SUB_PENTAGRAM_JEWEL_INFO = 0x00;
constexpr BYTE SUB_PENTAGRAM_JEWEL_INFO_SAVE = 0x30;
constexpr BYTE SUB_PENTAGRAM_JEWEL_INSERT_SAVE = 0x31;
constexpr BYTE SUB_PENTAGRAM_JEWEL_DELETE_SAVE = 0x32;
//**********************************************
// Personal Shop SubHeads (0x25)
//**********************************************
constexpr BYTE SUB_PSHOP_ITEM_VALUE = 0x00;
constexpr BYTE SUB_PSHOP_ITEM_VALUE_SAVE = 0x30;
constexpr BYTE SUB_PSHOP_ITEM_VALUE_INSERT = 0x31;
constexpr BYTE SUB_PSHOP_ITEM_VALUE_DELETE = 0x32;
//**********************************************
// Event Inventory SubHeads (0x26)
//**********************************************
constexpr BYTE SUB_EVENT_INVENTORY = 0x00;
constexpr BYTE SUB_EVENT_INVENTORY_SAVE = 0x30;
//**********************************************
// Muun System SubHeads (0x27)
//**********************************************
constexpr BYTE SUB_MUUN_INVENTORY = 0x00;
constexpr BYTE SUB_MUUN_INVENTORY_SAVE = 0x30;
//**********************************************
// Guild Matching SubHeads (0x28)
//**********************************************
constexpr BYTE SUB_GUILD_MATCHING_LIST = 0x00;
constexpr BYTE SUB_GUILD_MATCHING_LIST_SEARCH = 0x01;
constexpr BYTE SUB_GUILD_MATCHING_INSERT = 0x02;
constexpr BYTE SUB_GUILD_MATCHING_CANCEL = 0x03;
constexpr BYTE SUB_GUILD_MATCHING_JOIN_INSERT = 0x04;
constexpr BYTE SUB_GUILD_MATCHING_JOIN_CANCEL = 0x05;
constexpr BYTE SUB_GUILD_MATCHING_JOIN_ACCEPT = 0x06;
constexpr BYTE SUB_GUILD_MATCHING_JOIN_LIST = 0x07;
constexpr BYTE SUB_GUILD_MATCHING_JOIN_INFO = 0x08;
constexpr BYTE SUB_GUILD_MATCHING_NOTIFY = 0x09;
constexpr BYTE SUB_GUILD_MATCHING_NOTIFY_MASTER = 0x0A;
constexpr BYTE SUB_GUILD_MATCHING_INSERT_SAVE = 0x30;
//**********************************************
// Party Matching SubHeads (0x29)
//**********************************************
constexpr BYTE SUB_PARTY_MATCHING_INSERT = 0x00;
constexpr BYTE SUB_PARTY_MATCHING_LIST = 0x01;
constexpr BYTE SUB_PARTY_MATCHING_JOIN_INSERT = 0x02;
constexpr BYTE SUB_PARTY_MATCHING_JOIN_INFO = 0x03;
constexpr BYTE SUB_PARTY_MATCHING_JOIN_LIST = 0x04;
constexpr BYTE SUB_PARTY_MATCHING_JOIN_ACCEPT = 0x05;
constexpr BYTE SUB_PARTY_MATCHING_JOIN_CANCEL = 0x06;
constexpr BYTE SUB_PARTY_MATCHING_INSERT_SAVE = 0x30;
//**********************************************
// Castle Siege SubHeads (0x80)
//**********************************************
constexpr BYTE SUB_CASTLE_TOTAL_INFO = 0x00;
constexpr BYTE SUB_CASTLE_OWNER_GUILD_MASTER = 0x01;
constexpr BYTE SUB_CASTLE_NPC_BUY = 0x03;
constexpr BYTE SUB_CASTLE_NPC_REPAIR = 0x04;
constexpr BYTE SUB_CASTLE_NPC_UPGRADE = 0x05;
constexpr BYTE SUB_CASTLE_TAX_INFO = 0x06;
constexpr BYTE SUB_CASTLE_TAX_RATE_CHANGE = 0x07;
constexpr BYTE SUB_CASTLE_MONEY_CHANGE = 0x08;
constexpr BYTE SUB_CASTLE_SIEGE_DATE_CHANGE = 0x09;
constexpr BYTE SUB_CASTLE_GUILD_MARK_INFO = 0x0A;
constexpr BYTE SUB_CASTLE_SIEGE_ENDED_CHANGE = 0x0B;
constexpr BYTE SUB_CASTLE_OWNER_CHANGE = 0x0C;
constexpr BYTE SUB_CASTLE_REG_ATTACK_GUILD = 0x0D;
constexpr BYTE SUB_CASTLE_RESTART_STATE = 0x0E;
constexpr BYTE SUB_CASTLE_MAPSVR_MULTICAST = 0x0F;
constexpr BYTE SUB_CASTLE_REG_GUILD_MARK = 0x10;
constexpr BYTE SUB_CASTLE_GUILD_MARK_RESET = 0x11;
constexpr BYTE SUB_CASTLE_GUILD_GIVEUP = 0x12;
constexpr BYTE SUB_CASTLE_NPC_REMOVE = 0x16;
constexpr BYTE SUB_CASTLE_STATE_SYNC = 0x17;
constexpr BYTE SUB_CASTLE_TRIBUTE_MONEY = 0x18;
constexpr BYTE SUB_CASTLE_RESET_TAX_INFO = 0x19;
constexpr BYTE SUB_CASTLE_RESET_SIEGE_GUILD = 0x1A;
constexpr BYTE SUB_CASTLE_RESET_REG_INFO = 0x1B;
//**********************************************
// Custom SubHeads (0xB0)
//**********************************************
#if (CUSTOMELEMENTALBOOK)
constexpr BYTE SUB_CUSTOM_ELEMENTAL_BOOK = 0x03;
constexpr BYTE SUB_CUSTOM_ELEMENTAL_BOOK_SAVE = 0x04;
#endif
#if (RECHARGEREWARD)
constexpr BYTE SUB_MOCNAP_RESULT = 0x09;
constexpr BYTE SUB_MOCNAP_LOAD = 0x13;
constexpr BYTE SUB_MOCNAP_SAVE = 0x14;
#endif
//**********************************************
// Custom NPC Quest SubHeads (0xF7)
//**********************************************
constexpr BYTE SUB_CUSTOM_NPC_QUEST_INFO = 0x00;
constexpr BYTE SUB_CUSTOM_NPC_QUEST_SAVE = 0x01;
constexpr BYTE SUB_CUSTOM_NPC_QUEST_MONSTER_COUNT_SAVE = 0x02;
constexpr BYTE SUB_CUSTOM_JEWEL_BANK = 0x04;
constexpr BYTE SUB_CUSTOM_JEWEL_BANK_INFO = 0x05;
//**********************************************
// Custom Extensions (0xD9)
//**********************************************
constexpr BYTE SUB_SKIN_GET_LIST = 0x01;
constexpr BYTE SUB_SKIN_SAVE = 0x02;
constexpr BYTE SUB_BUFFPHE_REQUESTDS = 0x03;
constexpr BYTE SUB_INFOCHAR_BUFFPHE = 0x04;
constexpr BYTE SUB_THE_GIFT_SAVE = 0x07;
constexpr BYTE SUB_CHANGE_PASSWORD_SAVE = 0x09;
constexpr BYTE SUB_CHARACTER_RANKING = 0x17;
constexpr BYTE SUB_BUFFPHE_LOAD = 0x18;
constexpr BYTE SUB_BUFFPHE_TOP_INFO = 0x19;
//==============================================================
// GameServer Main Heads
//==============================================================
constexpr BYTE GS_HEAD_CUSTOM_EXTENSIONS = 0xFB;
//**********************************************
//**********************************************
// Custom Extensions (0xFB)
//**********************************************
constexpr BYTE GS_SUB_CHANGE_PASSWORD_RESULT = 0x02;
//**********************************************
// Custom GHRS (0xF8)
//**********************************************
constexpr BYTE SUB_CUSTOM_GHRS_SEND = 0xF6;