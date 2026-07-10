#pragma once
#include "Header.h"

#define MAX_MAIN_PACKET_SIZE 2048
#define MAX_UDP_PACKET_SIZE 8192

// Encabezados de paquetes
#define PACKET_HEADER_C1 0xC1
#define PACKET_HEADER_C2 0xC2
#define PACKET_HEADER_C3 0xC3
#define PACKET_HEADER_C4 0xC4

#define DEFAULT_TIME_WAIT 5000
#define DEFAULT_BACKLOG 5

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
	void set(BYTE packetHead, WORD packetSize)
	{
		this->type = PACKET_HEADER_C2;
		this->size[0] = SET_NUMBERHB(packetSize);
		this->size[1] = SET_NUMBERLB(packetSize);
		this->head = packetHead;
	}

	void setE(BYTE packetHead, WORD packetSize)
	{
		this->type = PACKET_HEADER_C4;
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
	BYTE size[2];
	BYTE head;
	BYTE subh;
};

// GameServer -> DataServer

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
	char GuildName[9];
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
	BYTE type;
	BYTE count;
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
	BYTE type;
	char CharacterName[MAX_CHARACTER_NAME];
	char message[60];
};

struct SDHP_POST_ITEM_RECV
{
	PBMSG_HEAD Header;	//C1:78
	WORD MapServerGroup;
	BYTE type;
	char chatid[10];
	char chatmsg[60];
	char item_data[107];
};

struct SDHP_GLOBAL_ITEM_POST_RECV
{
	PBMSG_HEAD Header; // C1:78
	WORD MapServerGroup;
	char CharacterName[MAX_CHARACTER_NAME];
	char message[60];
	char item_data[107];
};

struct SDHP_GLOBAL_NOTICE_RECV
{
	PBMSG_HEAD Header; // C1:21
	WORD MapServerGroup;
	BYTE type;
	BYTE count;
	BYTE opacity;
	WORD delay;
	DWORD color;
	BYTE speed;
	char message[128];
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

#if(FLAG_SKIN)
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

#if(MOCNAP == 1)
	int MOCNAPCOIN;
#endif

	DWORD mUserSkinPick;
#if(CHONPHEDOILAP)
	BYTE ChonPheHanhTau;
#endif

#if(B_HON_HOAN)
	WORD CapDoHonHoan;
#endif
	int PointUsePhe;
#if(EVENT_END_LESS)
	WORD mLuotDiEndLess;
#endif

};


struct SDHP_INVENTORY_ITEM_SAVE_RECV
{
	PWMSG_HEAD Header; // C2:31
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Inventory[INVENTORY_SIZE][16];
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
	char TargetName[11];
	char message[60];
};

struct SDHP_SERVER_INFO_RECV
{
	PBMSG_HEAD Header; // C1:00
	BYTE type;
	WORD ServerPort;
	char ServerName[50];
	WORD ServerCode;
};

struct SDHP_MARRY_INFO_SAVE_RECV
{
    PBMSG_HEAD Header; // C1:F0
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
    char marryname[11];
    char mode[11];
};

#if(HIDE_VT)
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
	DWORD quest;
};
#endif

struct SDHP_SETCOIN_RECV
{
    PBMSG_HEAD Header; // C1:F3
    WORD Index;
    char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD value1;
	DWORD value2;
	DWORD value3;
};

#if(RANKINGGOC == 1)
struct SDHP_CUSTOM_RANKING_RECV
{
	PBMSG_HEAD Header; // C1:F4
	WORD Index;
	WORD type;
};
#endif

//---------------------------------------------------
#if(RANKING_NEW == 1)
#define MAXTOP 150

struct GDTop
{
	PSWMSG_HEAD h;
	BYTE Result;
};

struct CharTop
{
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Class;
	int level;
	int Reset;
	int Relifes;
	int Time;
	int Map;
	char Guild[9];
};

struct DGCharTop
{
	PSWMSG_HEAD h;
	CharTop	tp[MAXTOP];
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
	WORD active;
	WORD skill;
	WORD map;
	WORD posx;
	WORD posy;
	WORD autobuff;
	WORD offpvp;
	WORD autoreset;
	DWORD autoaddstr;
	DWORD autoaddagi;
	DWORD autoaddvit;
	DWORD autoaddene;
	DWORD autoaddcmd;
};

struct SDHP_RANKING_TVT_EVENT_SAVE_RECV
{
	PBMSG_HEAD Header; // C1:55
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	DWORD killcount;
	DWORD deathcount;
};

struct SDHP_CUSTOMNPCQUEST_RECV
{
    PSBMSG_HEAD Header; // C1:F1
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
    WORD quest;
	WORD indexnpc;
};

struct SDHP_CUSTOMNPCQUEST_SAVE_RECV
{
    PSBMSG_HEAD Header; // C1:F2
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
	WORD quest;
};

struct SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV
{
    PSBMSG_HEAD Header; // C1:F1
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
    WORD quest;
	DWORD monsterqtd;
};

// DataServer -> GameServer

struct SDHP_SERVER_INFO_SEND
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
	BYTE slot;
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

#if(FLAG_SKIN)
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

#if(MOCNAP == 1)
	int MOCNAPCOIN;
#endif

	DWORD mUserSkinPick;
#if(CHONPHEDOILAP)
	BYTE ChonPheHanhTau;
#endif
#if(B_HON_HOAN)
	WORD CapDoHonHoan;
#endif
	int PointUsePhe;
#if(EVENT_END_LESS)
	WORD mLuotDiEndLess;
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
	BYTE type;
	BYTE count;
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
	char OldName[11];
	char NewName[11];
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
	BYTE type;
	char CharacterName[MAX_CHARACTER_NAME];
	char message[60];
};

struct SDHP_POST_ITEM_SEND
{
	PBMSG_HEAD Header; 
	WORD MapServerGroup;
	BYTE type;
	char chatid[10];
	char chatmsg[60];
	char item_data[107];
};

struct SDHP_GLOBAL_ITEM_POST_SEND
{
	PBMSG_HEAD Header; // C1:78
	WORD MapServerGroup;
	char CharacterName[MAX_CHARACTER_NAME];
	char message[60];
	char item_data[107];
};

struct SDHP_GLOBAL_NOTICE_SEND
{
	PBMSG_HEAD Header; // C1:21
	WORD MapServerGroup;
	BYTE type;
	BYTE count;
	BYTE opacity;
	WORD delay;
	DWORD color;
	BYTE speed;
	char message[128];
};

struct SDHP_SNS_DATA_SEND
{
	PWMSG_HEAD Header; // C1:24
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	BYTE data[256];
};

struct SDHP_GLOBAL_WHISPER_SEND
{
	PBMSG_HEAD Header; // C1:72
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	BYTE Result;
	char TargetName[11];
	char message[60];
};

struct SDHP_GLOBAL_WHISPER_ECHO_SEND
{
	PBMSG_HEAD Header; // C1:73
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	char CharacterName[MAX_CHARACTER_NAME];
	char SourceName[11];
	char message[60];
};
//====================================================
#if (GHRS_TOP1_NEW)
struct SDHP_CUSTOM_GHRS_RECV
{
	PBMSG_HEAD Header; // C1:F0:09
	int time;
};
#endif
//====================================================
struct SDHP_CUSTOMQUEST_SEND
{
    PBMSG_HEAD Header; // C1:F1
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
	DWORD quest;
};

struct PMSG_CUSTOM_RANKING_SEND
{
	PWMSG_HEAD Header; 
	int Index;
	int type;
	int count;
};

struct SDHP_CARESUME_SEND
{
    PBMSG_HEAD Header; // C1:F5
    WORD Index;
    char CharacterName[MAX_CHARACTER_NAME];
	WORD active;
	WORD skill;
	WORD map;
	WORD posx;
	WORD posy;
	WORD autobuff;
	WORD offpvp;
	WORD autoreset;
	DWORD autoaddstr;
	DWORD autoaddagi;
	DWORD autoaddvit;
	DWORD autoaddene;
	DWORD autoaddcmd;
};

struct SDHP_CUSTOMNPCQUEST_SEND
{
    PSBMSG_HEAD Header; // C1:F1
    WORD Index;
    WORD quest;
	WORD indexnpc;
	DWORD questcount;
	DWORD monstercount;
};



struct THEGIFT_GD_SAVE_DATA
{
	PSBMSG_HEAD Header;
	WORD	index;
	char	Name[11];
	BYTE	TheGift;
};

#if (GHRS_TOP1_NEW)
struct PMSG_CUSTOM_GHRS_SEND
{
	PWMSG_HEAD Header;
	int time;
	int resets;
	int Grand;
};
#endif

#if (MOCNAP == 1)
struct MOCNAP_GD_REQ_DATA
{
	PSBMSG_HEAD	header;
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
	WORD	index;
	char	Name[11];
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

#if(BOT_STATUE == 1)
struct SDHP_BOT_INFO_GET
{
	PBMSG_HEAD Header; // C1:04
	WORD Index;
	WORD Rank;
	WORD TypeTop;
};
//=========================




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
	char GuildName[9];
	int GuildNumber;
	int GuildStatus;
};
#endif



struct SDHP_CUSTOM_JEWELBANK_RECV
{
	PSBMSG_HEAD Header;
	WORD Index;
	char Account[MAX_ACCOUNT_NAME];
	WORD type;
	DWORD count;
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

void GDSaveTheGiftRecv(THEGIFT_GD_SAVE_DATA* lpMsg);
void DataServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size);
void GDServerInfoRecv(SDHP_SERVER_INFO_RECV* lpMsg,int index);
void GDCharacterListRecv(SDHP_CHARACTER_LIST_RECV* lpMsg,int index);
void GDCharacterCreateRecv(SDHP_CHARACTER_CREATE_RECV* lpMsg,int index);
void GDCharacterDeleteRecv(SDHP_CHARACTER_DELETE_RECV* lpMsg,int index);
void GDCharacterInfoRecv(SDHP_CHARACTER_INFO_RECV* lpMsg,int index);
void GDCreateItemRecv(SDHP_CREATE_ITEM_RECV* lpMsg,int index);
void GDOptionDataRecv(SDHP_OPTION_DATA_RECV* lpMsg,int index);
void GDPetItemInfoRecv(SDHP_PET_ITEM_INFO_RECV* lpMsg,int index);
void GDCharacterNameCheckRecv(SDHP_CHARACTER_NAME_CHECK_RECV* lpMsg,int index);
void GDCharacterNameChangeRecv(SDHP_CHARACTER_NAME_CHANGE_RECV* lpMsg,int index);
void GDCrywolfSyncRecv(SDHP_CRYWOLF_SYNC_RECV* lpMsg,int index);
void GDCrywolfInfoRecv(SDHP_CRYWOLF_INFO_RECV* lpMsg,int index);
void GDGlobalPostRecv(SDHP_GLOBAL_POST_RECV* lpMsg,int index);
void GDGlobalItemPostRecv(SDHP_GLOBAL_ITEM_POST_RECV* lpMsg, int index);
void GDPostItemRecv(SDHP_POST_ITEM_RECV* lpMsg,int index);
void GDGlobalNoticeRecv(SDHP_GLOBAL_NOTICE_RECV* lpMsg,int index);
void GDSNSDataRecv(SDHP_SNS_DATA_RECV* lpMsg,int index);
void GDCharacterInfoSaveRecv(SDHP_CHARACTER_INFO_SAVE_RECV* lpMsg);
void GDInventoryItemSaveRecv(SDHP_INVENTORY_ITEM_SAVE_RECV* lpMsg);
void GDOptionDataSaveRecv(SDHP_OPTION_DATA_SAVE_RECV* lpMsg);
void GDPetItemInfoSaveRecv(SDHP_PET_ITEM_INFO_SAVE_RECV* lpMsg);
void GDResetInfoSaveRecv(SDHP_RESET_INFO_SAVE_RECV* lpMsg);
void GDMasterResetInfoSaveRecv(SDHP_MASTER_RESET_INFO_SAVE_RECV* lpMsg);
void GDRankingDuelSaveRecv(SDHP_RANKING_DUEL_SAVE_RECV* lpMsg);
void GDRankingBloodCastleSaveRecv(SDHP_RANKING_BLOOD_CASTLE_SAVE_RECV* lpMsg);
void GDRankingChaosCastleSaveRecv(SDHP_RANKING_CHAOS_CASTLE_SAVE_RECV* lpMsg);
void GDRankingDevilSquareSaveRecv(SDHP_RANKING_DEVIL_SQUARE_SAVE_RECV* lpMsg);
void GDRankingIllusionTempleSaveRecv(SDHP_RANKING_ILLUSION_TEMPLE_SAVE_RECV* lpMsg);
void GDCreationCardSaveRecv(SDHP_CREATION_CARD_SAVE_RECV* lpMsg);
void GDCrywolfInfoSaveRecv(SDHP_CRYWOLF_INFO_SAVE_RECV* lpMsg);
void GDSNSDataSaveRecv(SDHP_SNS_DATA_SAVE_RECV* lpMsg);
void GDCustomMonsterRewardSaveRecv(SDHP_CUSTOM_MONSTER_REWARD_SAVE_RECV* lpMsg);
void GDRankingCustomArenaSaveRecv(SDHP_RANKING_CUSTOM_ARENA_SAVE_RECV* lpMsg);
void GDRankingTvTEventSaveRecv(SDHP_RANKING_TVT_EVENT_SAVE_RECV* lpMsg);
void GDConnectCharacterRecv(SDHP_CONNECT_CHARACTER_RECV* lpMsg,int index);
void GDDisconnectCharacterRecv(SDHP_DISCONNECT_CHARACTER_RECV* lpMsg,int index);
void GDGlobalWhisperRecv(SDHP_GLOBAL_WHISPER_RECV* lpMsg,int index);
void DGGlobalWhisperEchoSend(WORD ServerCode,WORD index,char* account,char* name,char* SourceName,char* message);
void GDMarryInfoSaveRecv(SDHP_MARRY_INFO_SAVE_RECV* lpMsg);
#if(HIDE_VT)
void GDCustomQuestSaveRecv(SDHP_CUSTOMQUEST_SAVE_RECV* lpMsg);
void GDCustomQuestRecv(SDHP_CUSTOMQUEST_RECV* lpMsg, int index);
#endif
void GDSetCoinRecv(SDHP_SETCOIN_RECV* lpMsg);
#if(RANKINGGOC == 1)
void GDCustomRankingRecv(SDHP_CUSTOM_RANKING_RECV* lpMsg, int index);
#endif
void GDCustomAttackResumeRecv(SDHP_CARESUME_RECV* lpMsg,int index);
void GDCustomAttackSaveRecv(SDHP_CARESUME_SAVE_RECV* lpMsg);
void GDCustomNpcQuestSaveRecv(SDHP_CUSTOMNPCQUEST_SAVE_RECV* lpMsg);
void GDCustomNpcQuestRecv(SDHP_CUSTOMNPCQUEST_RECV* lpMsg,int index);
void GDCustomNpcQuestMonsterCountSaveRecv(SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV* lpMsg);

//===================================================================================
#if (GHRS_TOP1_NEW)
void GDCustomGHRSRecv(SDHP_CUSTOM_GHRS_RECV* lpMsg, int index);
#endif
//===================================================================================
#if (MOCNAP == 1)
void GDCharacterMocNapRecv(MOCNAP_GD_REQ_DATA* lpMsg, int index);
void GDCharacterMocNapSaveRecv(MOCNAP_GD_SAVE_DATA* lpMsg);
#endif

void GDCustomJewelBankRecv(SDHP_CUSTOM_JEWELBANK_RECV* lpMsg);
void GDCustomJewelBankInfoRecv(SDHP_CUSTOM_JEWELBANK_INFO_RECV* lpMsg, int index);

// RAW FUNCTIONS

#if (RANKING_NEW == 1)
void CharacterRanking(GDTop* lpMsg, int pIndex);
#endif

#if (BOT_STATUE == 1)
void GDBotInfoRecv(SDHP_BOT_INFO_GET* lpMsg, int index);
#endif

void DS_GDReqCastleTotalInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqOwnerGuildMaster(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleNpcBuy(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleNpcRepair(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleNpcUpgrade(BYTE *lpRecv, int aIndex);
void DS_GDReqTaxInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqTaxRateChange(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleMoneyChange(BYTE *lpRecv, int aIndex);
void DS_GDReqSiegeDateChange(BYTE *lpRecv, int aIndex);
void DS_GDReqGuildMarkRegInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqSiegeEndedChange(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleOwnerChange(BYTE *lpRecv, int aIndex);
void DS_GDReqRegAttackGuild(BYTE *lpRecv, int aIndex);
void DS_GDReqRestartCastleState(BYTE *lpRecv, int aIndex);
void DS_GDReqMapSvrMsgMultiCast(BYTE *lpRecv, int aIndex);
void DS_GDReqRegGuildMark(BYTE *lpRecv, int aIndex);
void DS_GDReqGuildMarkReset(BYTE *lpRecv, int aIndex);
void DS_GDReqGuildSetGiveUp(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleNpcRemove(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleStateSync(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleTributeMoney(BYTE *lpRecv, int aIndex);
void DS_GDReqResetCastleTaxInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqResetSiegeGuildInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqResetRegSiegeInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleInitData(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleNpcInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqAllGuildMarkRegInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqFirstCreateNPC(BYTE *lpRecv, int aIndex);
void DS_GDReqCalcRegGuildList(BYTE *lpRecv, int aIndex);
void DS_GDReqCsGuildUnionInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqCsSaveTotalGuildInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqCsLoadTotalGuildInfo(BYTE *lpRecv, int aIndex);
void DS_GDReqCastleNpcUpdate(BYTE *lpRecv, int aIndex);

//==Change Pass
#pragma pack(push, 1)
struct SDHP_CHANGE_PASSWORD_RECV
{
	PSBMSG_HEAD Header;
	int  Index;
	char Account[MAX_ACCOUNT_NAME];
	char OldPassword[11];
	char NewPassword[11];

};
#pragma pack(pop)
void GDChangePasswordRecv(SDHP_CHANGE_PASSWORD_RECV* lpMsg, int aIndex);
struct SDHP_CHANGE_PASSWORD_SEND
{
	PSBMSG_HEAD Header;
	int  Index;
	BYTE Result;
};

struct GSSENDDS_GETLISTISBUYSKIN
{
	PSBMSG_HEAD Header;
	char AccountID[11];
	int aIndex;
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

	int aIndex;
	int count;
};

void GDGetSkinIsBuy(GSSENDDS_GETLISTISBUYSKIN* lpMsg, int index);
void GDSaveSkinBuy(GSSENDDS_GETLISTISBUYSKIN* lpMsg, int index);

//====================================================
#if (SACHTHUOCTINH_NEW)
struct SACHTHUOCTINH_GD_REQ_DATA
{
	PSBMSG_HEAD	header;
	WORD Index;
	char CharacterName[MAX_CHARACTER_NAME];
};
struct SACHTHUOCTINH_DG_GET_DATA
{
	PSBMSG_HEAD Header;
	WORD Index;
	int SACHTHUOCTINH_01;
	int SACHTHUOCTINH_02;
	int SACHTHUOCTINH_03;
	int SACHTHUOCTINH_04;
	int SACHTHUOCTINH_05;
	int SACHTHUOCTINH_06;
	int SACHTHUOCTINH_07;
};
struct SACHTHUOCTINH_GD_SAVE_DATA
{
	PSBMSG_HEAD Header;
	WORD	index;
	char	Name[11];
	int SACHTHUOCTINH_01;
	int SACHTHUOCTINH_02;
	int SACHTHUOCTINH_03;
	int SACHTHUOCTINH_04;
	int SACHTHUOCTINH_05;
	int SACHTHUOCTINH_06;
	int SACHTHUOCTINH_07;
};
#endif
//====================================================
#if (SACHTHUOCTINH_NEW)
void GDCharacterSachThuocTinhRecv(SACHTHUOCTINH_GD_REQ_DATA* lpMsg, int index);
void GDCharacterSachThuocTinhSaveRecv(SACHTHUOCTINH_GD_SAVE_DATA* lpMsg);
#endif
//===================================================================================
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

void GetDBBuffPhe(BUFFPHE_REQUESTDS* lpMsg, int index);
void GetInfoCharTopBuffPhe(BUFFPHE_REQUESTDS* lpMsg, int index);

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
	char GuildName[9];
	int GuildNumber;
	int GuildStatus;
};