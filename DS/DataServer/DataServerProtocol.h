#pragma once
#include "Header.h"

// Tamaño máximo de paquetes de recepción y envío
constexpr std::size_t MAX_RECV_PACKET_SIZE = 8192;		// Límite de seguridad de entrada: 8192 bytes
constexpr std::size_t MAX_SEND_PACKET_SIZE = 8192;		// Límite de seguridad de salida: 8192 bytes en un unico paquete
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

#if(DATASERVER_UPDATE>=701)
#define INVENTORY_SIZE 237
#elif(DATASERVER_UPDATE>=602)
#define INVENTORY_SIZE 236
#else
#define INVENTORY_SIZE 108
#endif
#if(DATASERVER_UPDATE>=701)
#define MAX_EFFECT_LIST 32
#else
#define MAX_EFFECT_LIST 16
#endif
#if(DATASERVER_UPDATE>=701)
#define MAX_SKILL_LIST 60
#else
#define MAX_SKILL_LIST 60
#endif

// Warehouse.h
#if(DATASERVER_UPDATE>=602)
#define WAREHOUSE_SIZE 240
#else
#define WAREHOUSE_SIZE 120
#endif
#define MAX_ITEM_BYTE 16
// MasterSkillTree.h
#define MAX_MASTER_SKILL_LIST 120
#define MASTER_SKILL_DATA_SIZE 3
// QuestWorld.h
#define MAX_QUEST_WORLD_LIST 20
// MuRummy.h
#define MURUMMY_MAX_CARD 24
// CashShop.h
#define MAX_CASH_SHOP_PAGE 5
#define MAX_CASH_SHOP_PAGE_ITEM 9
// EventInventory.h
#define EVENT_INVENTORY_SIZE 32
// PartyMatching.h
#define MAX_PARTY_USER 5
// MuunSystem.h
#define MUUN_INVENTORY_SIZE 62

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
	void set(BYTE packetHead, WORD packetSize)
	{
		this->type = PACKET_C2;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
	}

	void setE(BYTE packetHead, WORD packetSize)
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
	BYTE size[2];
	BYTE head;
	BYTE subh;
};

// GameServer -> DataServer

struct SDHP_ITEM_COUNT_RECV
{
	PBMSG_HEAD Header; // C1:00
	BYTE Type;
	WORD ServerPort;
	char ServerName[50];
	WORD ServerCode;
};

struct SDHP_CHARACTER_LIST_RECV
{
	PBMSG_HEAD Header; // C1:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
};

struct SDHP_CHARACTER_CREATE_RECV
{
	PBMSG_HEAD Header; // C1:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Class;
};

struct SDHP_CHARACTER_DELETE_RECV
{
	PBMSG_HEAD Header; // C1:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE guild;
	char GuildName[MAX_GUILD_NAME];
};

struct SDHP_CHARACTER_INFO_RECV
{
	PBMSG_HEAD Header; // C1:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_CREATE_ITEM_RECV
{
	PBMSG_HEAD Header; // C1:07
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE X;
	BYTE Y;
	BYTE Map;
	WORD ItemIndex;
	BYTE Level;
	BYTE Dur;
	BYTE Option1;
	BYTE Option2;
	BYTE Option3;
	BYTE NewOption;
	WORD LootIndex;
	BYTE SetOption;
	BYTE JewelOfHarmonyOption;
	BYTE ItemOptionEx;
	BYTE SocketOption[5];
	BYTE SocketOptionBonus;
	DWORD Duration;
};

struct SDHP_OPTION_DATA_RECV
{
	PBMSG_HEAD Header; // C1:08
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_PET_ITEM_INFO_RECV
{
	PWMSG_HEAD Header; // C2:09
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Type;
	BYTE Count;
};

struct SDHP_PET_ITEM_INFO1
{
	BYTE Slot;
	DWORD Serial;
};

struct SDHP_CHARACTER_NAME_CHECK_RECV
{
	PBMSG_HEAD Header; // C1:0A
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_CHARACTER_NAME_CHANGE_RECV
{
	PBMSG_HEAD Header; // C1:0B
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char OldName[11];
	char NewName[11];
};

struct SDHP_CRYWOLF_SYNC_RECV
{
	PBMSG_HEAD Header; // C1:1E
	WORD MapServerGroup;
	DWORD CrywolfState;
	DWORD OccupationState;
};

struct SDHP_CRYWOLF_INFO_RECV
{
	PBMSG_HEAD Header; // C1:1F
	WORD MapServerGroup;
};

struct SDHP_GLOBAL_POST_RECV
{
	PBMSG_HEAD Header; // C1:20
	WORD MapServerGroup;
	BYTE Type;
	char CharacterName[MAX_CHARACTER_NAME];
	char Message[60];
};

struct SDHP_POST_ITEM_RECV
{
	PBMSG_HEAD Header;	//C1:78
	WORD MapServerGroup;
	BYTE Type;
	char Chatid[10];
	char Chatmsg[60];
	char Item_data[107];
};

struct SDHP_GLOBAL_ITEM_POST_RECV
{
	PBMSG_HEAD Header; // C1:78
	WORD MapServerGroup;
	char CharacterName[MAX_CHARACTER_NAME];
	char Message[60];
	char Item_Data[107];
};

struct SDHP_GLOBAL_NOTICE_RECV
{
	PBMSG_HEAD Header; // C1:21
	WORD MapServerGroup;
	BYTE Type;
	BYTE Count;
	BYTE Opacity;
	WORD Delay;
	DWORD Color;
	BYTE Speed;
	char Message[128];
};

struct SDHP_SNS_DATA_RECV
{
	PBMSG_HEAD Header; // C1:24
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_CHARACTER_INFO_SAVE_RECV
{
	PWMSG_HEAD Header; // C2:30
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Level;
	BYTE Class;
	DWORD LevelUpPoint;
	DWORD Experience;
	DWORD Money;
	DWORD Strength;
	DWORD Dexterity;
	DWORD Vitality;
	DWORD Energy;
	DWORD Leadership;
	DWORD Life;
	DWORD MaxLife;
	DWORD Mana;
	DWORD MaxMana;
	DWORD BP;
	DWORD MaxBP;
	DWORD Shield;
	DWORD MaxShield;
	BYTE Inventory[INVENTORY_SIZE][16];
	BYTE Skill[MAX_SKILL_LIST][3];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	DWORD PKCount;
	DWORD PKLevel;
	DWORD PKTime;
	BYTE Quest[50];
	WORD FruitAddPoint;
	WORD FruitSubPoint;
	BYTE Effect[MAX_EFFECT_LIST][13];
#if(DATASERVER_UPDATE>=602)
	BYTE ExtInventory;
	BYTE ExtWarehouse;
#endif
	DWORD Kills;
	DWORD Deads;
#if(CUSTOMFLAGSKIN)
	int Flag;
#endif
#if(DANHHIEU_NEW)
	int rDanhHieu;
	int DHSatThuong;
	int DHSatThuongX2;
	int DHPhongThu;
	int DHHP;
	int DHSD;
	int DHGST;
#endif
#if(TULUYEN_NEW)
	int rTuLuyen;
#endif
	//==Custom CTC
	int CTCTime;
	int CTCRegDay;
#if(RECHARGEREWARD == 1)
	int MOCNAPCOIN;
#endif
	DWORD mUserSkinPick;
#if(CUSTOMFACTIONSYSTEM)
	BYTE ChonPheHanhTau;
#endif
#if(CUSTOMSOULRINGEFFECT)
	WORD CapDoHonHoan;
#endif
	int PointUsePhe;
#if(ENDLESSTOWEREVENT)
	WORD mLuotDiEndLess;
#endif
};

struct SDHP_INVENTORY_ITEM_SAVE_RECV
{
	PWMSG_HEAD Header; // C2:31
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Inventory[INVENTORY_SIZE][MAX_ITEM_BYTE];
};

struct SDHP_OPTION_DATA_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:33
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE SkillKey[20];
	BYTE GameOption;
	BYTE QKey;
	BYTE WKey;
	BYTE EKey;
	BYTE ChatWindow;
	BYTE RKey;
	DWORD QWERLevel;
	#if(DATASERVER_UPDATE>=701)
	BYTE ChangeSkin;
	#endif
};

struct SDHP_PET_ITEM_INFO_SAVE_RECV
{
	PWMSG_HEAD Header; // C2:34
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE count;
};

struct SDHP_PET_ITEM_INFO_SAVE
{
	DWORD serial;
	BYTE level;
	DWORD experience;
};

struct SDHP_RESET_INFO_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:39
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	UINT Reset;
	UINT ResetDay;
	UINT ResetWek;
	UINT ResetMon;
};

struct SDHP_MASTER_RESET_INFO_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:3A
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	UINT Reset;
	UINT MasterReset;
	UINT MasterResetDay;
	UINT MasterResetWek;
	UINT MasterResetMon;
};

struct SDHP_RANKING_DUEL_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:3C
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD WinScore;
	DWORD LoseScore;
};

struct SDHP_RANKING_BLOOD_CASTLE_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:3D
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD score;
};

struct SDHP_RANKING_CHAOS_CASTLE_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:3E
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD score;
};

struct SDHP_RANKING_DEVIL_SQUARE_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:3F
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD score;
};

struct SDHP_RANKING_ILLUSION_TEMPLE_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:40
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD score;
};

struct SDHP_CREATION_CARD_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:42
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE ExtClass;
};

struct SDHP_CRYWOLF_INFO_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:49
	WORD MapServerGroup;
	DWORD CrywolfState;
	DWORD OccupationState;
};

struct SDHP_SNS_DATA_SAVE_RECV
{
	PWMSG_HEAD Header; // C1:4E
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE data[256];
};

struct SDHP_CUSTOM_MONSTER_REWARD_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:52
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD MonsterClass;
	DWORD MapNumber;
	DWORD RewardValue1;
	DWORD RewardValue2;
	DWORD RewardValue3;
	DWORD RewardValue4;
};

struct SDHP_RANKING_CUSTOM_ARENA_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:55
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD number;
	DWORD score;
	DWORD rank;
};

struct SDHP_CONNECT_CHARACTER_RECV
{
	PBMSG_HEAD Header; // C1:70
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_DISCONNECT_CHARACTER_RECV
{
	PBMSG_HEAD Header; // C1:71
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_GLOBAL_WHISPER_RECV
{
	PBMSG_HEAD Header; // C1:72
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char TargetCharacterName[MAX_CHARACTER_NAME];
	char Message[60];
};



struct SDHP_MARRY_INFO_SAVE_RECV
{
    PBMSG_HEAD Header; // C1:F0
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
    char MarryName[MAX_CHARACTER_NAME];
    char Mode[11];
};

#if(ENABLECUSTOMQUEST)
struct SDHP_CUSTOMQUEST_RECV
{
	PBMSG_HEAD Header; // C1:F1
	WORD Index;
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_CUSTOMQUEST_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:F2
	WORD Index;
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Quest;
};
#endif

struct SDHP_SETCOIN_RECV
{
    PBMSG_HEAD Header; // C1:F3
    WORD Index;
    char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD Value1;
	DWORD Value2;
	DWORD Value3;
};

#if(ENABLECUSTOMRANKING)
struct SDHP_CUSTOM_RANKING_RECV
{
	PBMSG_HEAD Header; // C1:F4
	WORD Index;
	WORD Type;
};
#endif

//---------------------------------------------------
#if(NEWRANKING == 1)
#define MAXTOP 150

struct GDTop
{
	PSWMSG_HEAD Header;
	BYTE Result;
};

struct CharTop
{
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Class;
	int Level;
	int Reset;
	int Relifes;
	int Time;
	int Map;
	char GuildName[MAX_GUILD_NAME];
};

struct DGCharTop
{
	PSWMSG_HEAD Header;
	CharTop	Tp[MAXTOP];
};
#endif

//----------------------------------------------------

struct SDHP_CARESUME_RECV
{
    PBMSG_HEAD Header; // C1:F5
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
};

struct SDHP_CARESUME_SAVE_RECV
{
    PBMSG_HEAD Header; // C1:F6
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
	WORD Active;
	WORD Skill;
	WORD Map;
	WORD Posx;
	WORD Posy;
	WORD AutoBuff;
	WORD OffPVP;
	WORD AutoReset;
	DWORD AutoAddStr;
	DWORD AutoAddAgi;
	DWORD AutoAddVit;
	DWORD AutoAddEne;
	DWORD AutoAddCmd;
};

struct SDHP_RANKING_TVT_EVENT_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:55
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD KillCount;
	DWORD DeathCount;
};

struct SDHP_CUSTOMNPCQUEST_RECV
{
    PSBMSG_HEAD Header; // C1:F1
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
    WORD Quest;
	WORD Indexnpc;
};

struct SDHP_CUSTOMNPCQUEST_SAVE_RECV
{
    PSBMSG_HEAD Header; // C1:F2
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
	WORD Quest;
};

struct SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV
{
    PSBMSG_HEAD Header; // C1:F1
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
    WORD Quest;
	DWORD MonsterQtd;
};

// DataServer -> GameServer

struct SDHP_ITEM_COUNT_SEND
{
	PBMSG_HEAD Header; // C1:00
	BYTE Result;
	DWORD ItemCount;
};

struct SDHP_CHARACTER_LIST_SEND
{
	PWMSG_HEAD Header; // C2:01
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE MoveCnt;
	BYTE ExtClass;
	#if(DATASERVER_UPDATE>=602)
	BYTE ExtWarehouse;
	#endif
	BYTE Count;
};

struct SDHP_CHARACTER_LIST
{
	BYTE Slot;
	char CharacterName[MAX_CHARACTER_NAME];
	WORD Level;
	BYTE Class;
	BYTE CtlCode;
	BYTE Inventory[60];
	BYTE GuildStatus;
	DWORD Reset;
	DWORD MasterReset;
};

struct SDHP_CHARACTER_CREATE_SEND
{
	PBMSG_HEAD Header; // C1:02
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE Slot;
	BYTE Class;
	BYTE equipment[24];
	WORD level;
};

struct SDHP_CHARACTER_DELETE_SEND
{
	PBMSG_HEAD Header; // C1:03
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Result;
};

struct SDHP_CHARACTER_INFO_SEND
{
	PWMSG_HEAD Header; // C2:04
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE Class;
	WORD Level;
	DWORD LevelUpPoint;
	DWORD Experience;
	DWORD Money;
	DWORD Strength;
	DWORD Dexterity;
	DWORD Vitality;
	DWORD Energy;
	DWORD Leadership;
	DWORD Life;
	DWORD MaxLife;
	DWORD Mana;
	DWORD MaxMana;
	DWORD BP;
	DWORD MaxBP;
	DWORD Shield;
	DWORD MaxShield;
	BYTE Inventory[INVENTORY_SIZE][16];
	BYTE Skill[MAX_SKILL_LIST][3];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	DWORD PKCount;
	DWORD PKLevel;
	DWORD PKTime;
	BYTE CtlCode;
	BYTE Quest[50];
	WORD FruitAddPoint;
	WORD FruitSubPoint;
	BYTE Effect[MAX_EFFECT_LIST][13];
	#if(DATASERVER_UPDATE>=602)
	BYTE ExtInventory;
	BYTE ExtWarehouse;
	#endif
	DWORD Reset;
	DWORD MasterReset;
	#if(DATASERVER_UPDATE>=801)
	DWORD UseGuildMatching;
	DWORD UseGuildMatchingJoin;
	#endif
	DWORD Kills;
	DWORD Deads;
#if(CUSTOMFLAGSKIN)
	int Flag;
#endif
	BYTE TheGift; // only add in is struct
#if(DANHHIEU_NEW)
	int rDanhHieu;
	int DHSatThuong;
	int DHSatThuongX2;
	int DHPhongThu;
	int DHHP;
	int DHSD;
	int DHGST;
#endif
#if(TULUYEN_NEW)
	int rTuLuyen;
#endif
	//==Custom CTC
	int CTCTime;
	int CTCRegDay;
#if(RECHARGEREWARD == 1)
	int MOCNAPCOIN;
#endif
	DWORD mUserSkinPick;
#if(CUSTOMFACTIONSYSTEM)
	BYTE ChonPheHanhTau;
#endif
#if(CUSTOMSOULRINGEFFECT)
	WORD CapDoHonHoan;
#endif
	int PointUsePhe;
#if(ENDLESSTOWEREVENT)
	WORD LuotDiEndLess;
#endif
};

struct SDHP_CREATE_ITEM_SEND
{
	PBMSG_HEAD Header; // C1:07
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE X;
	BYTE Y;
	BYTE Map;
	DWORD Serial;
	WORD ItemIndex;
	BYTE Level;
	BYTE Dur;
	BYTE Option1;
	BYTE Option2;
	BYTE Option3;
	BYTE NewOption;
	WORD LootIndex;
	BYTE SetOption;
	BYTE JewelOfHarmonyOption;
	BYTE ItemOptionEx;
	BYTE SocketOption[5];
	BYTE SocketOptionBonus;
	DWORD Duration;


};

struct SDHP_OPTION_DATA_SEND
{
	PBMSG_HEAD Header; // C1:08
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE SkillKey[20];
	BYTE GameOption;
	BYTE QKey;
	BYTE WKey;
	BYTE EKey;
	BYTE ChatWindow;
	BYTE RKey;
	DWORD QWERLevel;
	#if(DATASERVER_UPDATE>=701)
	BYTE ChangeSkin;
	#endif
};

struct SDHP_PET_ITEM_INFO_SEND
{
	PWMSG_HEAD Header; // C2:09
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	BYTE Type;
	BYTE Count;
};

struct SDHP_PET_ITEM_INFO2
{
	BYTE Slot;
	DWORD Serial;
	BYTE Level;
	DWORD Experience;
};

struct SDHP_CHARACTER_NAME_CHECK_SEND
{
	PBMSG_HEAD Header; // C1:0A
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
};

struct SDHP_CHARACTER_NAME_CHANGE_SEND
{
	PBMSG_HEAD Header; // C1:0B
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char OldName[MAX_CHARACTER_NAME];
	char NewName[MAX_CHARACTER_NAME];
	BYTE Result;
};

struct SDHP_CRYWOLF_SYNC_SEND
{
	PBMSG_HEAD Header; // C1:1E
	WORD MapServerGroup;
	DWORD CrywolfState;
	DWORD OccupationState;
};

struct SDHP_CRYWOLF_INFO_SEND
{
	PBMSG_HEAD Header; // C1:1F
	WORD MapServerGroup;
	DWORD CrywolfState;
	DWORD OccupationState;
};

struct SDHP_GLOBAL_POST_SEND
{
	PBMSG_HEAD Header; // C1:20
	WORD MapServerGroup;
	BYTE Type;
	char CharacterName[MAX_CHARACTER_NAME];
	char Message[60];
};

struct SDHP_POST_ITEM_SEND
{
	PBMSG_HEAD Header; 
	WORD MapServerGroup;
	BYTE Type;
	char Chatid[10];
	char Chatmsg[60];
	char Item_Data[107];
};

struct SDHP_GLOBAL_ITEM_POST_SEND
{
	PBMSG_HEAD Header; // C1:78
	WORD MapServerGroup;
	char CharacterName[MAX_CHARACTER_NAME];
	char Message[60];
	char Item_Data[107];
};

struct SDHP_GLOBAL_NOTICE_SEND
{
	PBMSG_HEAD Header; // C1:21
	WORD MapServerGroup;
	BYTE Type;
	BYTE Count;
	BYTE Opacity;
	WORD Delay;
	DWORD Color;
	BYTE Speed;
	char Message[128];
};

struct SDHP_SNS_DATA_SEND
{
	PWMSG_HEAD Header; // C1:24
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE Data[256];
};

struct SDHP_GLOBAL_WHISPER_SEND
{
	PBMSG_HEAD Header; // C1:72
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	char TargetName[MAX_CHARACTER_NAME];
	char Message[60];
};

struct SDHP_GLOBAL_WHISPER_ECHO_SEND
{
	PBMSG_HEAD Header; // C1:73
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char SourceName[MAX_CHARACTER_NAME];
	char Message[60];
};

#if (TOP1RESETLIMIT)
struct SDHP_CUSTOM_GHRS_RECV
{
	PBMSG_HEAD Header; // C1:F0:09
	int Time;
};
#endif

struct SDHP_CUSTOMQUEST_SEND
{
    PBMSG_HEAD Header; // C1:F1
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
	DWORD Quest;
};

struct PMSG_CUSTOM_RANKING_SEND
{
	PWMSG_HEAD Header; 
	int Index;
	int Type;
	int Count;
};

struct SDHP_CARESUME_SEND
{
    PBMSG_HEAD Header; // C1:F5
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
	WORD Active;
	WORD Skill;
	WORD Map;
	WORD Posx;
	WORD Posy;
	WORD AutoBuff;
	WORD OffPVP;
	WORD AutoReset;
	DWORD AutoAddStr;
	DWORD AutoAddAgi;
	DWORD AutoAddVit;
	DWORD AutoAddEne;
	DWORD AutoAddCmd;
};

struct SDHP_CUSTOMNPCQUEST_SEND
{
    PSBMSG_HEAD Header; // C1:F1
    WORD Index;
    WORD Quest;
	WORD Indexnpc;
	DWORD QuestCount;
	DWORD MonsterCount;
};

struct THEGIFT_GD_SAVE_DATA
{
	PSBMSG_HEAD Header;
	WORD	Index;
	char	CharacterName[MAX_CHARACTER_NAME];
	BYTE	TheGift;
};

#if (TOP1RESETLIMIT)
struct PMSG_CUSTOM_GHRS_SEND
{
	PWMSG_HEAD Header;
	int Time;
	int Resets;
	int Grand;
};
#endif

#if (RECHARGEREWARD == 1)
struct MOCNAP_GD_REQ_DATA
{
	PSBMSG_HEAD	Header;
	WORD Index;
	char CharacterName[MAX_CHARACTER_NAME];
};
//
struct MOCNAP_DG_GET_DATA
{
	PSBMSG_HEAD Header;
	WORD	Index;
	int		MOCNAP1;
	int		MOCNAP2;
	int		MOCNAP3;
	int		MOCNAP4;
	int		MOCNAP5;
	int		MOCNAP6;
	int		MOCNAP7;
	int		MOCNAP8;
	int		MOCNAP9;
	int		MOCNAP10;
	int		MOCNAP11;
	int		MOCNAP12;
	int		MOCNAP13;
};
//
struct MOCNAP_GD_SAVE_DATA
{
	PSBMSG_HEAD Header;
	// ----
	WORD	Index;
	char	CharacterName[MAX_CHARACTER_NAME];
	int		MOCNAP1;
	int		MOCNAP2;
	int		MOCNAP3;
	int		MOCNAP4;
	int		MOCNAP5;
	int		MOCNAP6;
	int		MOCNAP7;
	int		MOCNAP8;
	int		MOCNAP9;
	int		MOCNAP10;
	int		MOCNAP11;
	int		MOCNAP12;
	int		MOCNAP13;
};
#endif

#if(TOP1BOTSTATUE == 1)
struct SDHP_BOT_INFO_GET
{
	PBMSG_HEAD Header; // C1:04
	WORD Index;
	WORD Rank;
	WORD TypeTop;
};

struct SDHP_BOT_INFO_SEND
{
	PWMSG_HEAD Header; // C2:04
	WORD Index;
	BYTE Result;
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Class;
	WORD Level;
	DWORD LevelUpPoint;
	DWORD Experience;
	DWORD Money;
	DWORD Strength;
	DWORD Dexterity;
	DWORD Vitality;
	DWORD Energy;
	DWORD Leadership;
	DWORD Life;
	DWORD MaxLife;
	DWORD Mana;
	DWORD MaxMana;
	DWORD BP;
	DWORD MaxBP;
	DWORD Shield;
	DWORD MaxShield;
	BYTE Inventory[INVENTORY_SIZE][16];
	BYTE Skill[MAX_SKILL_LIST][3];
	BYTE Dir;
	DWORD PKCount;
	DWORD PKLevel;
	DWORD PKTime;
	BYTE CtlCode;
	WORD FruitAddPoint;
	WORD FruitSubPoint;
	BYTE Effect[MAX_EFFECT_LIST][13];
	DWORD Kills;
	DWORD Deads;
	int rDanhHieu;
	// GUild
	char GuildName[MAX_GUILD_NAME];
	int GuildNumber;
	int GuildStatus;
};
#endif

struct SDHP_CUSTOM_JEWELBANK_RECV
{
	PSBMSG_HEAD Header;
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	WORD Type;
	DWORD Count;
};

struct SDHP_CUSTOM_JEWELBANK_INFO_RECV
{
	PSBMSG_HEAD Header; // C1:F5
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
};

struct SDHP_CUSTOM_JEWELBANK_INFO_SEND
{
	PSBMSG_HEAD Header; // C1:F5
	WORD Index;
	int Bless;
	int Soul;
	int Life;
	int Creation;
	int Guardian;
	int GemStone;
	int Harmony;
	int Chaos;
	int LowStone;
	int HighStone;
};

struct SDHP_CHANGE_PASSWORD_RECV
{
	PSBMSG_HEAD Header;
	int  Index;
	char Account[MAX_ACCOUNT_NAME];
	char OldPassword[11];
	char NewPassword[11];

};

struct SDHP_CHANGE_PASSWORD_SEND
{
	PSBMSG_HEAD Header;
	int  Index;
	BYTE Result;
};

struct GSSENDDS_GETLISTISBUYSKIN
{
	PSBMSG_HEAD Header;
	char AccountID[MAX_ACCOUNT_NAME];
	int Index;
};
struct BCUSTOM_SKINMODEL_DATA
{
	int SkinIndex;
	int StatusBuy;
};

//=======Xai CHung All Count
struct CBCUSTOM_LOAD_COUNT
{
	PSWMSG_HEAD Header;

	int Index;
	int Count;
};

#if (CUSTOMELEMENTALBOOK)
struct ELEMENTALBOOK_GD_REQ_DATA
{
	PSBMSG_HEAD	header;
	WORD Index;
	char CharacterName[MAX_CHARACTER_NAME];
};

struct SACHTHUOCTINH_DG_GET_DATA
{
	PSBMSG_HEAD Header;
	WORD Index;
	int BookFire;
	int BookWater;
	int BookWind;
	int BookEarth;
	int BookDark;
	int BookLight;
	int BookPoison;
};

struct ELEMENTALBOOK_GD_SAVE_DATA
{
	PSBMSG_HEAD Header;
	WORD	index;
	char	Name[11];
	int BookFire;
	int BookWater;
	int BookWind;
	int BookEarth;
	int BookDark;
	int BookLight;
	int BookPoison;
};
#endif

struct BUFFPHE_REQUESTDS
{
	PSBMSG_HEAD Header;
	int aIndex;
	char CharacterName[MAX_CHARACTER_NAME];
};
struct BUFFPHE_REQUESTDS_SETINFO
{
	PSWMSG_HEAD Header; // C2:04
	int  mDT_TongPoint;
	char mDT_Top1Name[11];
	int  mDT_Top1Point;
	int  mBT_TongPoint;
	char mBT_Top1Name[11];
	int  mBT_Top1Point;
};

struct INFOCHAR_BUFFPHE
{
	PSWMSG_HEAD Header; // C2:04
	WORD Index;
	BYTE Result;
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Class;
	WORD Level;
	BYTE Inventory[INVENTORY_SIZE][16];
	BYTE Skill[MAX_SKILL_LIST][3];
	BYTE Effect[MAX_EFFECT_LIST][13];
	BYTE ChonPheHanhTau;
	int PointUsePhe;
	// GUild
	char GuildName[MAX_GUILD_NAME];
	int GuildNumber;
	int GuildStatus;
};
#pragma pack(pop)

// Enrutador de paquetes
void DataServerProtocolCore(int serverIndex, const BYTE protocolHead, const BYTE* lpMsg, int size);

void GDGlobalItemCountRecv(const SDHP_ITEM_COUNT_RECV* lpMsg, int serverIndex, int size);
void GDCharacterListRecv(const SDHP_CHARACTER_LIST_RECV* lpMsg, int serverIndex, int size);
void GDCharacterCreateRecv(const SDHP_CHARACTER_CREATE_RECV* lpMsg, int serverIndex, int size);
void GDCharacterDeleteRecv(const SDHP_CHARACTER_DELETE_RECV* lpMsg, int serverIndex, int size);
void GDCharacterInfoRecv(const SDHP_CHARACTER_INFO_RECV* lpMsg, int serverIndex, int size);
void GDCreateItemRecv(const SDHP_CREATE_ITEM_RECV* lpMsg, int serverIndex, int size);
void GDOptionDataRecv(const SDHP_OPTION_DATA_RECV* lpMsg, int serverIndex, int size);
void GDPetItemInfoRecv(const SDHP_PET_ITEM_INFO_RECV* lpMsg, int serverIndex, int size);
void GDCharacterNameCheckRecv(const SDHP_CHARACTER_NAME_CHECK_RECV* lpMsg, int serverIndex, int size);
void GDCharacterNameChangeRecv(const SDHP_CHARACTER_NAME_CHANGE_RECV* lpMsg, int serverIndex, int size);

void GDSaveTheGiftRecv(const THEGIFT_GD_SAVE_DATA* lpMsg, int serverIndex, int size);

void GDCrywolfSyncRecv(const SDHP_CRYWOLF_SYNC_RECV* lpMsg, int serverIndex, int size);
void GDCrywolfInfoRecv(const SDHP_CRYWOLF_INFO_RECV* lpMsg, int serverIndex, int size);
void GDGlobalPostRecv(const SDHP_GLOBAL_POST_RECV* lpMsg, int serverIndex, int size);
void GDGlobalItemPostRecv(const SDHP_GLOBAL_ITEM_POST_RECV* lpMsg, int serverIndex, int size);
void GDPostItemRecv(SDHP_POST_ITEM_RECV* lpMsg,int index); // Revisar
void GDGlobalNoticeRecv(const SDHP_GLOBAL_NOTICE_RECV* lpMsg, int serverIndex, int size);
void GDSNSDataRecv(const SDHP_SNS_DATA_RECV* lpMsg, int serverIndex, int size);
void GDCharacterInfoSaveRecv(const SDHP_CHARACTER_INFO_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDInventoryItemSaveRecv(const SDHP_INVENTORY_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDOptionDataSaveRecv(const SDHP_OPTION_DATA_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDPetItemInfoSaveRecv(const SDHP_PET_ITEM_INFO_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDResetInfoSaveRecv(const SDHP_RESET_INFO_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDMasterResetInfoSaveRecv(const SDHP_MASTER_RESET_INFO_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDRankingDuelSaveRecv(const SDHP_RANKING_DUEL_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDRankingBloodCastleSaveRecv(const SDHP_RANKING_BLOOD_CASTLE_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDRankingChaosCastleSaveRecv(const SDHP_RANKING_CHAOS_CASTLE_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDRankingDevilSquareSaveRecv(const SDHP_RANKING_DEVIL_SQUARE_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDRankingIllusionTempleSaveRecv(const SDHP_RANKING_ILLUSION_TEMPLE_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDCreationCardSaveRecv(const SDHP_CREATION_CARD_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDCrywolfInfoSaveRecv(const SDHP_CRYWOLF_INFO_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDSNSDataSaveRecv(const SDHP_SNS_DATA_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDCustomMonsterRewardSaveRecv(const SDHP_CUSTOM_MONSTER_REWARD_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDRankingCustomArenaSaveRecv(const SDHP_RANKING_CUSTOM_ARENA_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDRankingTvTEventSaveRecv(const SDHP_RANKING_TVT_EVENT_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDConnectCharacterRecv(const SDHP_CONNECT_CHARACTER_RECV* lpMsg, int serverIndex, int size);
void GDDisconnectCharacterRecv(const SDHP_DISCONNECT_CHARACTER_RECV* lpMsg, int serverIndex, int size);
void GDGlobalWhisperRecv(const SDHP_GLOBAL_WHISPER_RECV* lpMsg, int serverIndex, int size);
void DGGlobalWhisperEchoSend(WORD serverCode, WORD userIndex, const char* account, const char* targetCharacterName, const char* sourceCharacterName, const char* message);
void GDMarryInfoSaveRecv(const SDHP_MARRY_INFO_SAVE_RECV* lpMsg, int serverIndex, int size);
#if(ENABLECUSTOMQUEST)
void GDCustomQuestSaveRecv(const SDHP_CUSTOMQUEST_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDCustomQuestRecv(const SDHP_CUSTOMQUEST_RECV* lpMsg, int serverIndex, int size);
#endif
void GDSetCoinRecv(const SDHP_SETCOIN_RECV* lpMsg, int serverIndex, int size);
#if(ENABLECUSTOMRANKING == 1)
void GDCustomRankingRecv(const SDHP_CUSTOM_RANKING_RECV* lpMsg, int serverIndex, int size);
#endif
void GDCustomAttackResumeRecv(const SDHP_CARESUME_RECV* lpMsg, int serverIndex, int size);
void GDCustomAttackSaveRecv(const SDHP_CARESUME_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDCustomNpcQuestSaveRecv(const SDHP_CUSTOMNPCQUEST_SAVE_RECV* lpMsg, int serverIndex, int size);
void GDCustomNpcQuestRecv(const SDHP_CUSTOMNPCQUEST_RECV* lpMsg, int serverIndex, int size);
void GDCustomNpcQuestMonsterCountSaveRecv(const SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV* lpMsg, int serverIndex, int size);

#if (TOP1RESETLIMIT)
void GDCustomGHRSRecv(const SDHP_CUSTOM_GHRS_RECV* lpMsg, int serverIndex, int size);
#endif

#if (RECHARGEREWARD)
void GDCharacterMocNapRecv(const MOCNAP_GD_REQ_DATA* lpMsg, int serverIndex, int size);
void GDCharacterMocNapSaveRecv(const MOCNAP_GD_SAVE_DATA* lpMsg, int serverIndex, int size);
#endif

void GDCustomJewelBankRecv(const SDHP_CUSTOM_JEWELBANK_RECV* lpMsg, int serverIndex, int size);
void GDCustomJewelBankInfoRecv(const SDHP_CUSTOM_JEWELBANK_INFO_RECV* lpMsg, int serverIndex, int size);

#if (NEWRANKING)
void CharacterRanking(const GDTop* lpMsg, int serverIndex, int size);
#endif

#if (TOP1BOTSTATUE)
void GDBotInfoRecv(const SDHP_BOT_INFO_GET* lpMsg, int serverIndex, int size);
#endif

void GDChangePasswordRecv(const SDHP_CHANGE_PASSWORD_RECV* lpMsg, int serverIndex, int size);

void GDGetSkinIsBuy(const GSSENDDS_GETLISTISBUYSKIN* lpMsg, int serverIndex, int size);
void GDSaveSkinBuy(const GSSENDDS_GETLISTISBUYSKIN* lpMsg, int serverIndex, int size);

#if (CUSTOMELEMENTALBOOK)
void GDCharacterElementalBookRecv(const ELEMENTALBOOK_GD_REQ_DATA* lpMsg, int serverIndex, int size);
void GDCharacterElementalBookSaveRecv(const ELEMENTALBOOK_GD_SAVE_DATA* lpMsg, int serverIndex, int size);
#endif

void GetDBBuffPhe(const BUFFPHE_REQUESTDS* lpMsg, int serverIndex, int size);
void GetInfoCharTopBuffPhe(const BUFFPHE_REQUESTDS* lpMsg, int serverIndex, int size);

inline BYTE GetProtocolSubHead(const BYTE* lpMsg);

#define VALIDATE_PACKET_SIZE(PacketType)                                      \
{                                                                             \
    const int structSize = sizeof(PacketType);                                \
    if (size != structSize)                                                   \
    {                                                                         \
        Log.ToDisp(LOG_RED,                                                   \
            "[%s] Tamaño de paquete invalido (ServerIndex: %d, Size: %d, Expected: %d)", \
            __FUNCTION__, serverIndex, size, structSize);                     \
        return;                                                               \
    }                                                                         \
}