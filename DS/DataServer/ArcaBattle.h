// ArcaBattle.h
#pragma once
#include "DataServerProtocol.h"

// GameServer -> DataServer

struct SDHP_ARCA_BATTLE_GUILD_MASTER_REGISTER_RECV
{
	PSBMSG_HEAD Header; // C1:1B:00
};

struct SDHP_ARCA_BATTLE_GUILD_MEMBER_REGISTER_RECV
{
	PSBMSG_HEAD Header; // C1:1B:00
};

//********** DataServer -> GameServer **********//

class CArcaBattle
{
public:
	CArcaBattle() = default;
	~CArcaBattle() = default;
	void GDArcaBattleGuildMasterRegisterRecv(SDHP_ARCA_BATTLE_GUILD_MASTER_REGISTER_RECV* lpMsg,int index);
	void GDArcaBattleGuildMemberRegisterRecv(SDHP_ARCA_BATTLE_GUILD_MEMBER_REGISTER_RECV* lpMsg,int index);
private:
};
