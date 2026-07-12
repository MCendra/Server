// DataServerProtocol.cpp
#include "DataServerProtocol.h"
#include "ServerManager.h"
#include "Log.h"
#include "ESProtocol.h"
#include "ChatServerProtocol.h"
#include "Helper.h"
#include "BadSyntax.h"
#include "CashShop.h"
#include "CastleDBSet.h"
#include "CharacterManager.h"
#include "CommandManager.h"
#include "EventInventory.h"
#include "GuildManager.h"
#include "GensSystem.h"
#include "GuildMatching.h"
#include "LuckyCoin.h"
#include "LuckyItem.h"
#include "MasterSkillTree.h"
#include "MuRummy.h"
#include "MuunSystem.h"
#include "NpcTalk.h"
#include "PartyMatching.h"
#include "PcPoint.h"
#include "PentagramSystem.h"
#include "PersonalShop.h"
#include "QueryManager.h"
#include "Quest.h"
#include "QuestWorld.h"
#include "SocketManager.h"
#include "Warehouse.h"
#include "ReiDoMU.h"
#include "Util.h"

void DataServerProtocolCore(int serverIndex, const BYTE protocolHead, const BYTE* lpMsg, int size)
{
	gServerManager[serverIndex].m_LastPacketTime = GetTickCount64();

	// Determina el tipo de cabecera del protocolo (C1 o C2).
	const BYTE packetType = lpMsg[PACKET_TYPE_OFFSET];

	switch (packetType)
	{
		case PACKET_HEADER_C1:
		{
			// Un paquete C1 debe contener como mínimo su cabecera completa.
			if (size < static_cast<int>(sizeof(PBMSG_HEAD)))
			{
				Log.ToDisp(LOG_RED, "[DataServerProtocolCore] Tamaño C1 invalido (ServerIndex: %d, Size: %d)", serverIndex, size);
				return;
			}

			// Verifica que el tamaño indicado en la cabecera coincida con el
			// tamaño real del paquete recibido.
			if (lpMsg[1] != size)
			{
				Log.ToDisp(LOG_RED, "[DataServerProtocolCore] Tamaño C1 inconsistente (ServerIndex: %d, HeaderSize: %d, RealSize: %d)", serverIndex, lpMsg[1], size);
				return;
			}

			// Comprueba que el ProtocolHead extraído durante la recepción sea
			// exactamente el mismo que contiene el paquete.
			if (lpMsg[2] != protocolHead)
			{
				Log.ToDisp(LOG_RED, "[DataServerProtocolCore] Header C1 inconsistente (ServerIndex: %d, QueueHead: %02X, PacketHead: %02X)", serverIndex, protocolHead, lpMsg[2]);
				return;
			}
		}
		break;
		case PACKET_HEADER_C2:
		{
			// Un paquete C2 debe contener como mínimo su cabecera completa.
			if (size < static_cast<int>(sizeof(PWMSG_HEAD)))
			{
				Log.ToDisp(LOG_RED, "[DataServerProtocolCore] Tamaño C2 invalido (ServerIndex: %d, Size: %d)", serverIndex, size);
				return;
			}

			// Obtiene el tamaño declarado en la cabecera C2.
			const WORD packetSize = (static_cast<WORD>(lpMsg[1]) << 8) | lpMsg[2];

			// Verifica que el tamaño indicado en la cabecera coincida con el
			// tamaño real del paquete recibido.
			if (packetSize != size)
			{
				Log.ToDisp(LOG_RED, "[DataServerProtocolCore] Tamaño C2 inconsistente (ServerIndex: %d, HeaderSize: %d, RealSize: %d)", serverIndex, packetSize, size);
				return;
			}

			// Comprueba que el ProtocolHead extraído durante la recepción sea
			// exactamente el mismo que contiene el paquete.
			if (lpMsg[3] != protocolHead)
			{
				Log.ToDisp(LOG_RED, "[DataServerProtocolCore] Header C2 inconsistente (ServerIndex: %d, QueueHead: %02X, PacketHead: %02X)", serverIndex, protocolHead, lpMsg[3]);
				return;
			}
		}
		break;

		default:
		{
			// El protocolo DataServer únicamente admite paquetes C1 y C2.
			Log.ToDisp(LOG_RED, "[DataServerProtocolCore] Tipo de paquete invalido (ServerIndex: %d, Type: %02X)", serverIndex, packetType);
			return;
		}
	}

	if (AdvancedLog != 0)
	{
		if ((protocolHead != HEAD_GLOBAL_ITEM_COUNT) && (protocolHead != HEAD_CREATE_ITEM))
		{
			Log.ToFile(LogType::GENERAL,"[DataServerProtocolCore] ProtocolHead: %x, 1: %x, 2: %x, 3: %x, 4: %x, 5: %x", protocolHead,lpMsg[1],lpMsg[2],lpMsg[3],lpMsg[4],lpMsg[5]);
		}
	}

	const BYTE protocolSubHead = GetProtocolSubHead(lpMsg);

	switch(protocolHead)
	{
		case HEAD_GLOBAL_ITEM_COUNT:	GDGlobalItemCountRecv(reinterpret_cast<const SDHP_ITEM_COUNT_RECV*>(lpMsg), serverIndex, size);			break;
		case HEAD_CHARACTER_LIST:		GDCharacterListRecv(reinterpret_cast<const SDHP_CHARACTER_LIST_RECV*>(lpMsg), serverIndex, size);		break;
		case HEAD_CHARACTER_CREATE:		GDCharacterCreateRecv(reinterpret_cast<const SDHP_CHARACTER_CREATE_RECV*>(lpMsg), serverIndex, size);	break;
		case HEAD_CHARACTER_DELETE:		GDCharacterDeleteRecv(reinterpret_cast<const SDHP_CHARACTER_DELETE_RECV*>(lpMsg), serverIndex, size);	break;
		case HEAD_CHARACTER_INFO:		GDCharacterInfoRecv(reinterpret_cast<const SDHP_CHARACTER_INFO_RECV*>(lpMsg), serverIndex, size);		break;
		case HEAD_WAREHOUSE:
			switch (protocolSubHead)
			{
			case SUB_WAREHOUSE_PERSONAL_LOAD:	gWarehouse.GDWarehouseItemRecv(reinterpret_cast<const SDHP_WAREHOUSE_ITEM_RECV*>(lpMsg), serverIndex, size);			break;
			case SUB_WAREHOUSE_PERSONAL_SAVE:	gWarehouse.GDWarehouseItemSaveRecv(reinterpret_cast<const SDHP_WAREHOUSE_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);	break;
			case SUB_WAREHOUSE_GUILD_LOAD:		gWarehouse.GDWarehouseGuildItemRecv(reinterpret_cast<const SDHP_WAREHOUSE_ITEM_RECV*>(lpMsg), serverIndex, size);			break;
			case SUB_WAREHOUSE_GUILD_SAVE:		gWarehouse.GDWarehouseGuildItemSaveRecv(reinterpret_cast<const SDHP_WAREHOUSE_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);	break;
			case SUB_WAREHOUSE_GUILD_STATUS:	gWarehouse.GDWarehouseGuildConsult(reinterpret_cast<const SDHP_WAREHOUSE_GUILD_STATUS_RECV*>(lpMsg), serverIndex, size);	break;
			case SUB_WAREHOUSE_GUILD_UPDATE:	gWarehouse.GDWarehouseGuildUpdate(reinterpret_cast<const SDHP_WAREHOUSE_GUILD_STATUS_UPDATE_RECV*>(lpMsg), serverIndex, size);	break;
			}
			break;
		case HEAD_CREATE_ITEM:	GDCreateItemRecv(reinterpret_cast<const SDHP_CREATE_ITEM_RECV*>(lpMsg), serverIndex, size);		break;
		case HEAD_OPTION_DATA:	GDOptionDataRecv(reinterpret_cast<const SDHP_OPTION_DATA_RECV*>(lpMsg), serverIndex, size);		break;
		case HEAD_PET_ITEM:		GDPetItemInfoRecv(reinterpret_cast<const SDHP_PET_ITEM_INFO_RECV*>(lpMsg), serverIndex, size);	break;
#if(DATASERVER_UPDATE>=401)
		case HEAD_CHARACTER_NAME_CHECK:	GDCharacterNameCheckRecv(reinterpret_cast<const SDHP_CHARACTER_NAME_CHECK_RECV*>(lpMsg), serverIndex, size);	break;
#endif
#if(DATASERVER_UPDATE>=401)
		case HEAD_CHARACTER_RENAME:		GDCharacterNameChangeRecv(reinterpret_cast<const SDHP_CHARACTER_NAME_CHANGE_RECV*>(lpMsg), serverIndex, size);	break;
#endif
#if(HIDE_VT)
		case HEAD_QUEST:
			switch (protocolSubHead)
			{
			case SUB_QUEST_KILLCOUNT_LOAD:	gQuest.GDQuestKillCountRecv(reinterpret_cast<const SDHP_QUEST_KILL_COUNT_RECV*>(lpMsg), serverIndex, size);	break;
			case SUB_QUEST_KILLCOUNT_SAVE:	gQuest.GDQuestKillCountSaveRecv(reinterpret_cast<const SDHP_QUEST_KILL_COUNT_SAVE_RECV*>(lpMsg), serverIndex, size);	break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=401)
		case HEAD_MASTER_SKILL:
			switch (protocolSubHead)
			{
			case SUB_MASTER_SKILL_LOAD:	gMasterSkillTree.GDMasterSkillTreeRecv(reinterpret_cast<const SDHP_MASTER_SKILL_TREE_RECV*>(lpMsg), serverIndex, size);	break;
			case SUB_MASTER_SKILL_SAVE:	gMasterSkillTree.GDMasterSkillTreeSaveRecv(reinterpret_cast<const SDHP_MASTER_SKILL_TREE_SAVE_RECV*>(lpMsg), serverIndex, size);
				break;
			}
			break;
#endif
		case HEAD_NPC:
			switch (protocolSubHead)
			{
#if(DATASERVER_UPDATE>=202)
			case SUB_NPC_LEO_LOAD:
				gNpcTalk.GDNpcLeoTheHelperRecv(reinterpret_cast<const SDHP_NPC_LEO_THE_HELPER_RECV*>(lpMsg), serverIndex, size);
				break;
			case SUB_NPC_LEO_SAVE:
				gNpcTalk.GDNpcLeoTheHelperSaveRecv(reinterpret_cast<const SDHP_NPC_LEO_THE_HELPER_SAVE_RECV*>(lpMsg), serverIndex, size);
				break;
#endif
#if(DATASERVER_UPDATE>=401)
			case SUB_NPC_SANTA_LOAD:
				gNpcTalk.GDNpcSantaClausRecv(reinterpret_cast<const SDHP_NPC_SANTA_CLAUS_RECV*>(lpMsg), serverIndex, size);
				break;
			case SUB_NPC_SANTA_SAVE:
				gNpcTalk.GDNpcSantaClausSaveRecv(reinterpret_cast<const SDHP_NPC_SANTA_CLAUS_SAVE_RECV*>(lpMsg), serverIndex, size);
				break;
			}
			break;
#endif
		case HEAD_COMMAND:
			switch (protocolSubHead)
			{
			case DS_SUB_COMMAND_RESET:	gCommandManager.GDCommandResetRecv((SDHP_COMMAND_RESET_RECV*)lpMsg, serverIndex);	break;
			case DS_SUB_COMMAND_MASTER_RESET:	gCommandManager.GDCommandMasterResetRecv((SDHP_COMMAND_MASTER_RESET_RECV*)lpMsg, serverIndex);	break;
			case DS_SUB_COMMAND_MARRY:	gCommandManager.GDCommandMarryRecv((SDHP_COMMAND_MARRY_RECV*)lpMsg, serverIndex);	break;
			case DS_SUB_COMMAND_REWARD:	gCommandManager.GDCommandRewardRecv((SDHP_COMMAND_REWARD_RECV*)lpMsg, serverIndex);	break;
			case DS_SUB_COMMAND_REWARD_ALL:	gCommandManager.GDCommandRewardAllRecv((SDHP_COMMAND_REWARDALL_RECV*)lpMsg, serverIndex);	break;
			case DS_SUB_COMMAND_RENAME:	gCommandManager.GDCommandRenameRecv((SDHP_COMMAND_RENAME_RECV*)lpMsg, serverIndex);	break;
			case DS_SUB_COMMAND_BLOCK_ACCOUNT:	gCommandManager.GDCommandBlocAccRecv((SDHP_COMMAND_BLOC_RECV*)lpMsg, serverIndex);		break;
			case DS_SUB_COMMAND_BLOCK_CHARACTER:gCommandManager.GDCommandBlocCharRecv((SDHP_COMMAND_BLOC_RECV*)lpMsg, serverIndex);		break;
			case DS_SUB_COMMAND_GIFT:	gCommandManager.GDCommandGiftRecv((SDHP_GIFT_RECV*)lpMsg, serverIndex);				break;
			case DS_SUB_COMMAND_TOP:	gCommandManager.GDCommandTopRecv((SDHP_TOP_RECV*)lpMsg, serverIndex);				break;
			}
			break;
		case DS_HEAD_QUEST_WORLD:
			switch (protocolSubHead)
			{
			#if(HIDE_VT)
			case DS_SUB_QUEST_WORLD_LOAD:
				#if(DATASERVER_UPDATE>=501)
				gQuestWorld.GDQuestWorldRecv((SDHP_QUEST_WORLD_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_QUEST_WORLD_SAVE:
				#if(DATASERVER_UPDATE>=501)
				gQuestWorld.GDQuestWorldSaveRecv((SDHP_QUEST_WORLD_SAVE_RECV*)lpMsg);
				#endif
				break;
			#endif
			}
			break;
		case DS_HEAD_GENS_SYSTEM:
			switch (protocolSubHead)
			{
			case DS_SUB_GENS_SYSTEM_INSERT:
				#if(DATASERVER_UPDATE>=501)
				gGensSystem.GDGensSystemInsertRecv((SDHP_GENS_SYSTEM_INSERT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GENS_SYSTEM_DELETE:
				#if(DATASERVER_UPDATE>=501)
				gGensSystem.GDGensSystemDeleteRecv((SDHP_GENS_SYSTEM_DELETE_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GENS_SYSTEM_MEMBER:
				#if(DATASERVER_UPDATE>=501)
				gGensSystem.GDGensSystemMemberRecv((SDHP_GENS_SYSTEM_MEMBER_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GENS_SYSTEM_UPDATE:
				#if(DATASERVER_UPDATE>=501)
				gGensSystem.GDGensSystemUpdateRecv((SDHP_GENS_SYSTEM_UPDATE_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GENS_SYSTEM_REWARD:
				#if(DATASERVER_UPDATE>=501)
				gGensSystem.GDGensSystemRewardRecv((SDHP_GENS_SYSTEM_REWARD_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GENS_SYSTEM_REWARD_SAVE:
				#if(DATASERVER_UPDATE>=501)
				gGensSystem.GDGensSystemRewardSaveRecv((SDHP_GENS_SYSTEM_REWARD_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_MURUMMY:
			switch (protocolSubHead)
			{
			case DS_SUB_MURUMMY_LOAD:
				#if(DATASERVER_UPDATE>=802)
				gMuRummy.GDReqCardInfo((_tagPMSG_REQ_MURUMMY_SELECT_DS*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_MURUMMY_INSERT:
				#if(DATASERVER_UPDATE>=802)
				gMuRummy.GDReqCardInfoInsert((_tagPMSG_REQ_MURUMMY_INSERT_DS*)lpMsg);
				#endif
				break;
			case DS_SUB_MURUMMY_UPDATE:
				#if(DATASERVER_UPDATE>=802)
				gMuRummy.GDReqCardInfoUpdate((_tagPMSG_REQ_MURUMMY_UPDATE_DS*)lpMsg);
				#endif
				break;
			case DS_SUB_MURUMMY_SCORE_UPDATE:
				#if(DATASERVER_UPDATE>=802)
				gMuRummy.GDReqScoreUpdate((_tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS*)lpMsg);
				#endif
				break;
			case DS_SUB_MURUMMY_DELETE:
				#if(DATASERVER_UPDATE>=802)
				gMuRummy.GDReqScoreDelete((_tagPMSG_REQ_MURUMMY_DELETE_DS*)lpMsg);
				#endif
				break;
			case DS_SUB_MURUMMY_SLOT_UPDATE:
				#if(DATASERVER_UPDATE>=802)
				gMuRummy.GDReqSlotInfoUpdate((_tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS*)lpMsg);
				#endif
				break;
			case DS_SUB_MURUMMY_INFO_UPDATE:
				#if(DATASERVER_UPDATE>=802)
				gMuRummy.GDReqMuRummyInfoUpdate((_tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_HELPER:
			switch (protocolSubHead)
			{
			case DS_SUB_HELPER_LOAD:
				#if(DATASERVER_UPDATE>=603)
				gHelper.GDHelperDataRecv((SDHP_HELPER_DATA_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_HELPER_SAVE:
				#if(DATASERVER_UPDATE>=603)
				gHelper.GDHelperDataSaveRecv((SDHP_HELPER_DATA_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_CASHSHOP:
			switch (protocolSubHead)
			{
			case DS_SUB_CASHSHOP_POINT:
				gCashShop.GDCashShopPointRecv((SDHP_CASH_SHOP_POINT_RECV*)lpMsg, serverIndex);
				break;
			case DS_SUB_CASHSHOP_ITEM_BUY:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopItemBuyRecv((SDHP_CASH_SHOP_ITEM_BUY_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_CASHSHOP_ITEM_GIFT:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopItemGifRecv((SDHP_CASH_SHOP_ITEM_GIF_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_CASHSHOP_ITEM_LIST:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopItemNumRecv((SDHP_CASH_SHOP_ITEM_NUM_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_CASHSHOP_ITEM_USE:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopItemUseRecv((SDHP_CASH_SHOP_ITEM_USE_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_CASHSHOP_PERIODIC_ITEM:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopPeriodicItemRecv((SDHP_CASH_SHOP_PERIODIC_ITEM_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_CASHSHOP_RECIEVE_POINT:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopRecievePointRecv((SDHP_CASH_SHOP_RECIEVE_POINT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_CASHSHOP_ADD_POINT_SAVE:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopAddPointSaveRecv((SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_CASHSHOP_SUB_POINT_SAVE:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopSubPointSaveRecv((SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_CASHSHOP_INSERT_ITEM_SAVE:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopInsertItemSaveRecv((SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_CASHSHOP_DELETE_ITEM_SAVE:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopDeleteItemSaveRecv((SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_CASHSHOP_PERIODIC_ITEM_SAVE:
				#if(DATASERVER_UPDATE>=501)
				gCashShop.GDCashShopPeriodicItemSaveRecv((SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_PCPOINT:
			switch (protocolSubHead)
			{
			case DS_SUB_PCPOINT_POINT:
				#if(DATASERVER_UPDATE<=402)
				gPcPoint.GDPcPointPointRecv((SDHP_PC_POINT_POINT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PCPOINT_ITEM_BUY:
				#if(DATASERVER_UPDATE<=402)
				gPcPoint.GDPcPointItemBuyRecv((SDHP_PC_POINT_ITEM_BUY_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PCPOINT_RECIEVE_POINT:
				#if(DATASERVER_UPDATE<=402)
				gPcPoint.GDPcPointRecievePointRecv((SDHP_PC_POINT_RECIEVE_POINT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PCPOINT_ADD_POINT_SAVE:
				#if(DATASERVER_UPDATE<=402)
				gPcPoint.GDPcPointAddPointSaveRecv((SDHP_PC_POINT_ADD_POINT_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_PCPOINT_SUB_POINT_SAVE:
				#if(DATASERVER_UPDATE<=402)
				gPcPoint.GDPcPointSubPointSaveRecv((SDHP_PC_POINT_SUB_POINT_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_LUCKY_COIN:
			switch (protocolSubHead)
			{
			case DS_SUB_LUCKY_COIN_COUNT:
				#if(DATASERVER_UPDATE>=402)
				gLuckyCoin.GDLuckyCoinCountRecv((SDHP_LUCKY_COIN_COUNT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_LUCKY_COIN_REGISTER:
				#if(DATASERVER_UPDATE>=402)
				gLuckyCoin.GDLuckyCoinRegisterRecv((SDHP_LUCKY_COIN_REGISTER_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_LUCKY_COIN_EXCHANGE:
				#if(DATASERVER_UPDATE>=402)
				gLuckyCoin.GDLuckyCoinExchangeRecv((SDHP_LUCKY_COIN_EXCHANGE_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_LUCKY_COIN_ADD_COUNT_SAVE:
				#if(DATASERVER_UPDATE>=402)
				gLuckyCoin.GDLuckyCoinAddCountSaveRecv((SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_LUCKY_COIN_SUB_COUNT_SAVE:
				#if(DATASERVER_UPDATE>=402)
				gLuckyCoin.GDLuckyCoinSubCountSaveRecv((SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_RESERVED_1B:
			switch (protocolSubHead)
			{
			case DS_SUB_RESERVED_1B:
				break;
			}
			break;
		case HEAD_CRYWOLF_SYNC:
			GDCrywolfSyncRecv((SDHP_CRYWOLF_SYNC_RECV*)lpMsg, serverIndex);
			break;
		case HEAD_CRYWOLF_INFO:
			GDCrywolfInfoRecv((SDHP_CRYWOLF_INFO_RECV*)lpMsg, serverIndex);
			break;
		case HEAD_GLOBAL_POST:
			GDGlobalPostRecv((SDHP_GLOBAL_POST_RECV*)lpMsg, serverIndex);
			break;
		case HEAD_GLOBAL_NOTICE:
			GDGlobalNoticeRecv((SDHP_GLOBAL_NOTICE_RECV*)lpMsg, serverIndex);
			break;
		case DS_HEAD_LUCKY_ITEM:
			switch (protocolSubHead)
			{
			case DS_SUB_LUCKY_ITEM_LOAD:
				#if(DATASERVER_UPDATE>=602)
				gLuckyItem.GDLuckyItemRecv((SDHP_LUCKY_ITEM_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_LUCKY_ITEM_SAVE:
				#if(DATASERVER_UPDATE>=602)
				gLuckyItem.GDLuckyItemSaveRecv((SDHP_LUCKY_ITEM_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_PENTAGRAM_SYSTEM:
			switch (protocolSubHead)
			{
			case DS_SUB_PENTAGRAM_JEWEL_INFO:
				#if(DATASERVER_UPDATE>=701)
				gPentagramSystem.GDPentagramJewelInfoRecv((SDHP_PENTAGRAM_JEWEL_INFO_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PENTAGRAM_JEWEL_INFO_SAVE:
				#if(DATASERVER_UPDATE>=701)
				gPentagramSystem.GDPentagramJewelInfoSaveRecv((SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_PENTAGRAM_JEWEL_INSERT_SAVE:
				#if(DATASERVER_UPDATE>=701)
				gPentagramSystem.GDPentagramJewelInsertSaveRecv((SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_PENTAGRAM_JEWEL_DELETE_SAVE:
				#if(DATASERVER_UPDATE>=701)
				gPentagramSystem.GDPentagramJewelDeleteSaveRecv((SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case HEAD_SNS_DATA:
			#if(DATASERVER_UPDATE>=801)
			GDSNSDataRecv((SDHP_SNS_DATA_RECV*)lpMsg, serverIndex);
			#endif
			break;
		case DS_HEAD_PERSONAL_SHOP:
			switch (protocolSubHead)
			{
			case DS_SUB_PSHOP_ITEM_VALUE:
				#if(DATASERVER_UPDATE>=802)
				gPersonalShop.GDPShopItemValueRecv((SDHP_PSHOP_ITEM_VALUE_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PSHOP_ITEM_VALUE_SAVE:
				#if(DATASERVER_UPDATE>=802)
				gPersonalShop.GDPShopItemValueSaveRecv((SDHP_PSHOP_ITEM_VALUE_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_PSHOP_ITEM_VALUE_INSERT:
				#if(DATASERVER_UPDATE>=802)
				gPersonalShop.GDPShopItemValueInsertSaveRecv((SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV*)lpMsg);
				#endif
				break;
			case DS_SUB_PSHOP_ITEM_VALUE_DELETE:
				#if(DATASERVER_UPDATE>=802)
				gPersonalShop.GDPShopItemValueDeleteSaveRecv((SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_EVENT_INVENTORY:
			switch (protocolSubHead)
			{
			case DS_SUB_EVENT_INVENTORY:
				#if(DATASERVER_UPDATE>=802)
				gEventInventory.GDEventInventoryRecv((SDHP_EVENT_INVENTORY_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_EVENT_INVENTORY_SAVE:
				#if(DATASERVER_UPDATE>=802)
				gEventInventory.GDEventInventorySaveRecv((SDHP_EVENT_INVENTORY_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_MUUN_SYSTEM:
			switch (protocolSubHead)
			{
			case DS_SUB_MUUN_INVENTORY:
				#if(DATASERVER_UPDATE>=803)
				gMuunSystem.GDMuunInventoryRecv((SDHP_MUUN_INVENTORY_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_MUUN_INVENTORY_SAVE:
				#if(DATASERVER_UPDATE>=803)
				gMuunSystem.GDMuunInventorySaveRecv((SDHP_MUUN_INVENTORY_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_GUILD_MATCHING:
			switch (protocolSubHead)
			{
			case DS_SUB_GUILD_MATCHING_LIST:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingListRecv((SDHP_GUILD_MATCHING_LIST_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_LIST_SEARCH:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingListSearchRecv((SDHP_GUILD_MATCHING_LIST_SEARCH_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_INSERT:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingInsertRecv((SDHP_GUILD_MATCHING_INSERT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_CANCEL:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingCancelRecv((SDHP_GUILD_MATCHING_CANCEL_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_JOIN_INSERT:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingJoinInsertRecv((SDHP_GUILD_MATCHING_JOIN_INSERT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_JOIN_CANCEL:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingJoinCancelRecv((SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_JOIN_ACCEPT:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingJoinAcceptRecv((SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_JOIN_LIST:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingJoinListRecv((SDHP_GUILD_MATCHING_JOIN_LIST_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_JOIN_INFO:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingJoinInfoRecv((SDHP_GUILD_MATCHING_JOIN_INFO_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_GUILD_MATCHING_INSERT_SAVE:
				#if(DATASERVER_UPDATE>=801)
				gGuildMatching.GDGuildMatchingInsertSaveRecv((SDHP_GUILD_MATCHING_INSERT_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_PARTY_MATCHING:
			switch (protocolSubHead)
			{
			case DS_SUB_PARTY_MATCHING_INSERT:
				#if(DATASERVER_UPDATE>=801)
				gPartyMatching.GDPartyMatchingInsertRecv((SDHP_PARTY_MATCHING_INSERT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PARTY_MATCHING_LIST:
				#if(DATASERVER_UPDATE>=801)
				gPartyMatching.GDPartyMatchingListRecv((SDHP_PARTY_MATCHING_LIST_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PARTY_MATCHING_JOIN_INSERT:
				#if(DATASERVER_UPDATE>=801)
				gPartyMatching.GDPartyMatchingJoinInsertRecv((SDHP_PARTY_MATCHING_JOIN_INSERT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PARTY_MATCHING_JOIN_INFO:
				#if(DATASERVER_UPDATE>=801)
				gPartyMatching.GDPartyMatchingJoinInfoRecv((SDHP_PARTY_MATCHING_JOIN_INFO_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PARTY_MATCHING_JOIN_LIST:
				#if(DATASERVER_UPDATE>=801)
				gPartyMatching.GDPartyMatchingJoinListRecv((SDHP_PARTY_MATCHING_JOIN_LIST_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PARTY_MATCHING_JOIN_ACCEPT:
				#if(DATASERVER_UPDATE>=801)
				gPartyMatching.GDPartyMatchingJoinAcceptRecv((SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PARTY_MATCHING_JOIN_CANCEL:
				#if(DATASERVER_UPDATE>=801)
				gPartyMatching.GDPartyMatchingJoinCancelRecv((SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV*)lpMsg, serverIndex);
				#endif
				break;
			case DS_SUB_PARTY_MATCHING_INSERT_SAVE:
				#if(DATASERVER_UPDATE>=801)
				gPartyMatching.GDPartyMatchingInsertSaveRecv((SDHP_PARTY_MATCHING_INSERT_SAVE_RECV*)lpMsg);
				#endif
				break;
			}
			break;
		case DS_HEAD_CHARACTER_INFO_SAVE:
			GDCharacterInfoSaveRecv((SDHP_CHARACTER_INFO_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_INVENTORY_ITEM_SAVE:
			GDInventoryItemSaveRecv((SDHP_INVENTORY_ITEM_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_OPTION_DATA_SAVE:
			GDOptionDataSaveRecv((SDHP_OPTION_DATA_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_PET_ITEM_INFO_SAVE:
			GDPetItemInfoSaveRecv((SDHP_PET_ITEM_INFO_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RESET_INFO_SAVE:
			GDResetInfoSaveRecv((SDHP_RESET_INFO_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_MASTER_RESET_INFO_SAVE:
			GDMasterResetInfoSaveRecv((SDHP_MASTER_RESET_INFO_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RANKING_DUEL_SAVE:
			GDRankingDuelSaveRecv((SDHP_RANKING_DUEL_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RANKING_BLOOD_CASTLE_SAVE:
			GDRankingBloodCastleSaveRecv((SDHP_RANKING_BLOOD_CASTLE_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RANKING_CHAOS_CASTLE_SAVE:
			GDRankingChaosCastleSaveRecv((SDHP_RANKING_CHAOS_CASTLE_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RANKING_DEVIL_SQUARE_SAVE:
			GDRankingDevilSquareSaveRecv((SDHP_RANKING_DEVIL_SQUARE_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RANKING_ILLUSION_TEMPLE_SAVE:
			GDRankingIllusionTempleSaveRecv((SDHP_RANKING_ILLUSION_TEMPLE_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_CREATION_CARD_SAVE:
			GDCreationCardSaveRecv((SDHP_CREATION_CARD_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_CRYWOLF_INFO_SAVE:
			GDCrywolfInfoSaveRecv((SDHP_CRYWOLF_INFO_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_SNS_DATA_SAVE:
			#if(DATASERVER_UPDATE>=801)
			GDSNSDataSaveRecv((SDHP_SNS_DATA_SAVE_RECV*)lpMsg);
			#endif
			break;
		case DS_HEAD_CUSTOM_MONSTER_REWARD_SAVE:
			GDCustomMonsterRewardSaveRecv((SDHP_CUSTOM_MONSTER_REWARD_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RANKING_CUSTOM_ARENA_SAVE:
			GDRankingCustomArenaSaveRecv((SDHP_RANKING_CUSTOM_ARENA_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RANKING_TVT_EVENT_SAVE:
			GDRankingTvTEventSaveRecv((SDHP_RANKING_TVT_EVENT_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_CONNECT_CHARACTER:
			GDConnectCharacterRecv((SDHP_CONNECT_CHARACTER_RECV*)lpMsg, serverIndex);
			break;
		case DS_HEAD_DISCONNECT_CHARACTER:
			GDDisconnectCharacterRecv((SDHP_DISCONNECT_CHARACTER_RECV*)lpMsg, serverIndex);
			break;
		case HEAD_GLOBAL_WHISPER:
			GDGlobalWhisperRecv((SDHP_GLOBAL_WHISPER_RECV*)lpMsg, serverIndex);
			break;
		case DS_HEAD_RANKING_KING_GUILD_SAVE:
			gReiDoMU.GDRankingKingGuildSaveRecv((SDHP_RANKING_KING_GUILD_SAVE_RECV*)lpMsg);
			break;
		case DS_HEAD_RANKING_KING_PLAYER_SAVE:
			gReiDoMU.GDRankingKingPlayerSaveRecv((SDHP_RANKING_KING_PLAYER_SAVE_RECV*)lpMsg);
			break;
		case HEAD_GLOBAL_ITEM_POST:
			GDGlobalItemPostRecv((SDHP_GLOBAL_ITEM_POST_RECV*)lpMsg, serverIndex);
			break;
		case HEAD_CASTLE_SIEGE:
			switch(protocolSubHead)
			{
				//case SUB_CASTLE_TOTAL_INFO:
				//	GDReqCastleTotalInfo(lpMsg,index);
				//	break;
				case SUB_CASTLE_OWNER_GUILD_MASTER:
					DS_GDReqOwnerGuildMaster(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_NPC_BUY:
					DS_GDReqCastleNpcBuy(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_NPC_REPAIR:
					DS_GDReqCastleNpcRepair(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_NPC_UPGRADE:
					DS_GDReqCastleNpcUpgrade(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_TAX_INFO:
					DS_GDReqTaxInfo(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_TAX_RATE_CHANGE:
					DS_GDReqTaxRateChange(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_MONEY_CHANGE:
					DS_GDReqCastleMoneyChange(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_SIEGE_DATE_CHANGE:
					DS_GDReqSiegeDateChange(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_GUILD_MARK_INFO:
					DS_GDReqGuildMarkRegInfo(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_SIEGE_ENDED_CHANGE:
					DS_GDReqSiegeEndedChange(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_OWNER_CHANGE:
					DS_GDReqCastleOwnerChange(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_REG_ATTACK_GUILD:
					DS_GDReqRegAttackGuild(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_RESTART_STATE:
					DS_GDReqRestartCastleState(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_MAPSVR_MULTICAST:
					DS_GDReqMapSvrMsgMultiCast(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_REG_GUILD_MARK:
					DS_GDReqRegGuildMark(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_GUILD_MARK_RESET:
					DS_GDReqGuildMarkReset(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_GUILD_GIVEUP:
					DS_GDReqGuildSetGiveUp(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_NPC_REMOVE:
					DS_GDReqCastleNpcRemove(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_STATE_SYNC:
					DS_GDReqCastleStateSync(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_TRIBUTE_MONEY:
					DS_GDReqCastleTributeMoney(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_RESET_TAX_INFO:
					DS_GDReqResetCastleTaxInfo(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_RESET_SIEGE_GUILD:
					DS_GDReqResetSiegeGuildInfo(lpMsg, serverIndex);
					break;
				case SUB_CASTLE_RESET_REG_INFO:
					DS_GDReqResetRegSiegeInfo(lpMsg, serverIndex);
					break;
			}
			break;
		case DS_HEAD_CASTLE_INIT_DATA:
			DS_GDReqCastleInitData(lpMsg, serverIndex);
			break;
		case DS_HEAD_CASTLE_NPC_INFO:
			DS_GDReqCastleNpcInfo(lpMsg, serverIndex);
			break;
		case DS_HEAD_CASTLE_ALL_GUILD_MARK:
			DS_GDReqAllGuildMarkRegInfo(lpMsg, serverIndex);
			break;
		case DS_HEAD_CASTLE_FIRST_CREATE_NPC:
			DS_GDReqFirstCreateNPC(lpMsg, serverIndex);
			break;
		case DS_HEAD_CASTLE_CALC_REG_GUILD:
			DS_GDReqCalcRegGuildList(lpMsg, serverIndex);
			break;
		case DS_HEAD_CASTLE_GUILD_UNION_INFO:
			DS_GDReqCsGuildUnionInfo(lpMsg, serverIndex);
			break;
		case DS_HEAD_CASTLE_SAVE_GUILD_INFO:
			DS_GDReqCsSaveTotalGuildInfo(lpMsg, serverIndex);
			break;
		case DS_HEAD_CASTLE_LOAD_GUILD_INFO:
			DS_GDReqCsLoadTotalGuildInfo(lpMsg, serverIndex);
			break;
		case DS_HEAD_CASTLE_NPC_UPDATE:
			DS_GDReqCastleNpcUpdate(lpMsg, serverIndex);
			break;
		case DS_HEAD_CUSTOM_PROTOCOL:
			switch (protocolSubHead)
			{
			#if (SACHTHUOCTINH_NEW)
			case DS_SUB_SACH_THUOC_TINH_LOAD:
				GDCharacterSachThuocTinhRecv((SACHTHUOCTINH_GD_REQ_DATA*)lpMsg, serverIndex);
				break;
			case DS_SUB_SACH_THUOC_TINH_SAVE:
				GDCharacterSachThuocTinhSaveRecv((SACHTHUOCTINH_GD_SAVE_DATA*)lpMsg);
				break;
			#endif
			#if (MOCNAP == 1)
			case DS_SUB_MOCNAP_LOAD:
				GDCharacterMocNapRecv((MOCNAP_GD_REQ_DATA*)lpMsg, serverIndex);
				break;
			case DS_SUB_MOCNAP_SAVE:
				GDCharacterMocNapSaveRecv((MOCNAP_GD_SAVE_DATA*)lpMsg);
				break;
			#endif	
			}
			break;
		case DS_HEAD_EXDB_PROTOCOL:
			ESDataRecv(serverIndex, protocolHead, lpMsg, size);
			break;
		case DS_HEAD_CONNECT_SERVER:
			CSDataRecv(serverIndex, protocolHead, lpMsg, size);
			break;
        case DS_HEAD_MARRY_INFO_SAVE:
            GDMarryInfoSaveRecv((SDHP_MARRY_INFO_SAVE_RECV*)lpMsg);
            break;
			#if(HIDE_VT)
		case DS_HEAD_CUSTOM_QUEST:
			GDCustomQuestRecv((SDHP_CUSTOMQUEST_RECV*)lpMsg, serverIndex);
			break;
		case DS_HEAD_CUSTOM_QUEST_SAVE:
			GDCustomQuestSaveRecv((SDHP_CUSTOMQUEST_SAVE_RECV*)lpMsg);
			break;
			#endif
        case DS_HEAD_SET_COIN:
            GDSetCoinRecv((SDHP_SETCOIN_RECV*)lpMsg);
            break;
			#if(RANKINGGOC == 1)
		case DS_HEAD_CUSTOM_RANKING:
			GDCustomRankingRecv((SDHP_CUSTOM_RANKING_RECV*)lpMsg, serverIndex);
			break;
			#endif
        case DS_HEAD_ATTACK_RESUME:
            GDCustomAttackResumeRecv((SDHP_CARESUME_RECV*)lpMsg, serverIndex);
            break;
        case DS_HEAD_ATTACK_RESUME_SAVE:
            GDCustomAttackSaveRecv((SDHP_CARESUME_SAVE_RECV*)lpMsg);
            break;
        case DS_HEAD_CUSTOM_NPC_QUEST:
			switch(protocolSubHead)
			{
			case DS_SUB_CUSTOM_NPC_QUEST_INFO:
				GDCustomNpcQuestRecv((SDHP_CUSTOMNPCQUEST_RECV*)lpMsg, serverIndex);
				break;
			case DS_SUB_CUSTOM_NPC_QUEST_SAVE:
				GDCustomNpcQuestSaveRecv((SDHP_CUSTOMNPCQUEST_SAVE_RECV*)lpMsg);
				break;
			case DS_SUB_CUSTOM_NPC_QUEST_MONSTER_COUNT_SAVE:
				GDCustomNpcQuestMonsterCountSaveRecv((SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV*)lpMsg);
				break;
			case DS_SUB_CUSTOM_JEWEL_BANK:
				GDCustomJewelBankRecv((SDHP_CUSTOM_JEWELBANK_RECV*)lpMsg);
				break;
			case DS_SUB_CUSTOM_JEWEL_BANK_INFO:
				GDCustomJewelBankInfoRecv((SDHP_CUSTOM_JEWELBANK_INFO_RECV*)lpMsg, serverIndex);
				break;
			}
            break;
		#if (GHRS_TOP1_NEW)
		case DS_HEAD_CUSTOM_GHRS:
			GDCustomGHRSRecv((SDHP_CUSTOM_GHRS_RECV*)lpMsg, serverIndex);
			break;
		#endif

		#if (BOT_STATUE == 1)
		case DS_HEAD_BOT_INFO:
			GDBotInfoRecv((SDHP_BOT_INFO_GET*)lpMsg, serverIndex);
			break;
		#endif
		case DS_HEAD_CUSTOM_EXTENSIONS:
			switch(protocolSubHead)
			{
			case DS_SUB_SKIN_GET_LIST:
				GDGetSkinIsBuy((GSSENDDS_GETLISTISBUYSKIN*)lpMsg, serverIndex);
				break;
				//==Save Skin Da MUa
			case DS_SUB_SKIN_SAVE:
				GDSaveSkinBuy((GSSENDDS_GETLISTISBUYSKIN*)lpMsg, serverIndex);
				break;
			case DS_SUB_THE_GIFT_SAVE:
				GDSaveTheGiftRecv((THEGIFT_GD_SAVE_DATA*)lpMsg);
				break;
			case DS_SUB_CHANGE_PASSWORD_SAVE:
				GDChangePasswordRecv((SDHP_CHANGE_PASSWORD_RECV*)lpMsg, serverIndex);
				break;
			#if(RANKING_NEW == 1)
			case DS_SUB_CHARACTER_RANKING:
				CharacterRanking((GDTop*)lpMsg, serverIndex);
				break;
			#endif
			case DS_SUB_BUFFPHE_LOAD:
				GetDBBuffPhe((BUFFPHE_REQUESTDS*)lpMsg, serverIndex);
				break;
			case DS_SUB_BUFFPHE_TOP_INFO:
				GetInfoCharTopBuffPhe((BUFFPHE_REQUESTDS*)lpMsg, serverIndex);
				break;
			}
			break;
	}
}

void GDGlobalItemCountRecv(const SDHP_ITEM_COUNT_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_ITEM_COUNT_RECV);

	SDHP_ITEM_COUNT_SEND pMsg{};
	pMsg.Header.set(HEAD_GLOBAL_ITEM_COUNT, sizeof(pMsg));
	pMsg.Result = 1;
	pMsg.ItemCount = 0;

	if (!gQueryManager.ExecQuery("SELECT ItemCount FROM GameServerInfo WHERE Number=0") || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO GameServerInfo (Number,ItemCount,ZenCount,AceItemCount) VALUES (0,0,0,0)");
		gQueryManager.Close();
	}
	else
	{
		pMsg.ItemCount = gQueryManager.GetAsInteger("ItemCount");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

	// Revisar
	gServerManager[serverIndex].SetServerInfo(lpMsg->ServerName, lpMsg->ServerPort, lpMsg->ServerCode);
}

void GDCharacterListRecv(const SDHP_CHARACTER_LIST_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CHARACTER_LIST_RECV);

	SDHP_CHARACTER_LIST_SEND pMsg{};
	pMsg.Header.set(HEAD_CHARACTER_LIST, 0);
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	pMsg.Count = 0;

	const size_t maxPacketSize =
		sizeof(SDHP_CHARACTER_LIST_SEND) +
		(static_cast<size_t>(MAX_CHARACTER_SLOT) * sizeof(SDHP_CHARACTER_LIST));

	std::vector<BYTE> sendBuffer(maxPacketSize);

	size_t bufferOffset = sizeof(SDHP_CHARACTER_LIST_SEND);

	bool accountExists =
		gQueryManager.ExecQuery("SELECT Id FROM AccountCharacter WHERE Id='%s'", lpMsg->Account) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	gQueryManager.Close();

	if (!accountExists)
	{
		gQueryManager.ExecQuery("INSERT INTO AccountCharacter (Id) VALUES ('%s')", lpMsg->Account);
		gQueryManager.Close();
	}

	std::array<std::array<char, MAX_CHARACTER_NAME>, MAX_CHARACTER_SLOT> characterNames{};

	if (!gQueryManager.ExecQuery("SELECT * FROM AccountCharacter WHERE Id='%s'", lpMsg->Account) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		return;
	}

	pMsg.MoveCnt = static_cast<BYTE>(gQueryManager.GetAsInteger("MoveCnt"));
	pMsg.ExtClass = static_cast<BYTE>(gQueryManager.GetAsInteger("ExtClass"));

#if (DATASERVER_UPDATE >= 602)
	pMsg.ExtWarehouse = static_cast<BYTE>(gQueryManager.GetAsInteger("ExtWarehouse"));
#endif

	gQueryManager.GetAsString("GameID1", characterNames[0].data(), characterNames[0].size());
	gQueryManager.GetAsString("GameID2", characterNames[1].data(), characterNames[1].size());
	gQueryManager.GetAsString("GameID3", characterNames[2].data(), characterNames[2].size());
	gQueryManager.GetAsString("GameID4", characterNames[3].data(), characterNames[3].size());
	gQueryManager.GetAsString("GameID5", characterNames[4].data(), characterNames[4].size());

	gQueryManager.Close();

	for (BYTE slot = 0; slot < MAX_CHARACTER_SLOT; ++slot)
	{
		if (characterNames[slot][0] == '\0')
		{
			continue;
		}

		const bool characterFound =
			gQueryManager.ExecQuery(
				"SELECT cLevel,Class,Inventory,CtlCode,ResetCount,MasterResetCount FROM Character WHERE AccountID='%s' AND Name='%s'",
				lpMsg->Account,
				characterNames[slot].data()) &&
			gQueryManager.Fetch() != SQL_NO_DATA;

		if (!characterFound)
		{
			gQueryManager.Close();
			continue;
		}

		SDHP_CHARACTER_LIST info{};
		info.Slot = slot;

		std::memcpy(info.CharacterName, characterNames[slot].data(), sizeof(info.CharacterName));

		info.Level = static_cast<WORD>(gQueryManager.GetAsInteger("cLevel"));
		info.Class = static_cast<BYTE>(gQueryManager.GetAsInteger("Class"));
		info.CtlCode = static_cast<BYTE>(gQueryManager.GetAsInteger("CtlCode"));
		info.Reset = static_cast<DWORD>(gQueryManager.GetAsInteger("ResetCount"));
		info.MasterReset = static_cast<DWORD>(gQueryManager.GetAsInteger("MasterResetCount"));

		std::fill(std::begin(info.Inventory), std::end(info.Inventory), 0xFF);

		std::array<std::array<BYTE, 16>, 12> inventory{};

		gQueryManager.GetAsBinary("Inventory", inventory[0].data(), sizeof(inventory));

		gQueryManager.Close();

		for (size_t i = 0; i < inventory.size(); ++i)
		{
			const size_t inventoryOffset = i * 5;

			if (inventory[i][0] == 0xFF &&
				(inventory[i][7] & 0x80) == 0x80 &&
				(inventory[i][9] & 0xF0) == 0xF0)
			{
				continue;
			}

			info.Inventory[inventoryOffset + 0] = inventory[i][0];
			info.Inventory[inventoryOffset + 1] = inventory[i][1];
			info.Inventory[inventoryOffset + 2] = inventory[i][7];
			info.Inventory[inventoryOffset + 3] = inventory[i][8];
			info.Inventory[inventoryOffset + 4] = inventory[i][9];
		}

		const bool guildFound =
			gQueryManager.ExecQuery(
				"SELECT G_Status FROM GuildMember WHERE Name='%s'",
				characterNames[slot].data()) &&
			gQueryManager.Fetch() != SQL_NO_DATA;

		info.GuildStatus = guildFound ?
			static_cast<BYTE>(gQueryManager.GetAsInteger("G_Status")) :
			0xFF;

		gQueryManager.Close();

		// Verifica que la siguiente estructura entre completamente
		// dentro del buffer de salida.
		if ((bufferOffset + sizeof(SDHP_CHARACTER_LIST)) > sendBuffer.size())
		{
			Log.ToDisp(LOG_RED,
				"[GDCharacterListRecv] Buffer de salida excedido (ServerIndex: %d)",
				serverIndex);
			return;
		}

		std::memcpy(sendBuffer.data() + bufferOffset, &info, sizeof(info));

		bufferOffset += sizeof(info);

		++pMsg.Count;
	}

	const WORD packetLength = static_cast<WORD>(bufferOffset);

	pMsg.Header.size[0] = SET_NUMBERHB(packetLength);
	pMsg.Header.size[1] = SET_NUMBERLB(packetLength);

	std::memcpy(sendBuffer.data(), &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, sendBuffer.data(), packetLength);
}

void GDCharacterCreateRecv(const SDHP_CHARACTER_CREATE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CHARACTER_CREATE_RECV);

	SDHP_CHARACTER_CREATE_SEND pMsg{};
	pMsg.Header.set(HEAD_CHARACTER_CREATE, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = (gUtil.CheckTextSyntax(lpMsg->CharacterName, sizeof(lpMsg->CharacterName)) != 0 && gBadSyntax.CheckSyntax(lpMsg->CharacterName) != 0);

	pMsg.slot = 0;
	pMsg.Class = lpMsg->Class;
	pMsg.level = 1;

	std::fill(std::begin(pMsg.equipment), std::end(pMsg.equipment), 0xFF);

	do
	{
		if (pMsg.Result == 0)
		{
			break;
		}

		std::array<std::array<char, MAX_CHARACTER_NAME>, MAX_CHARACTER_SLOT> characterName{};

		if (!gQueryManager.ExecQuery("SELECT * FROM AccountCharacter WHERE Id='%s'", lpMsg->Account) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			pMsg.Result = 0;
			break;
		}

		gQueryManager.GetAsString("GameID1", characterName[0].data(), characterName[0].size());
		gQueryManager.GetAsString("GameID2", characterName[1].data(), characterName[1].size());
		gQueryManager.GetAsString("GameID3", characterName[2].data(), characterName[2].size());
		gQueryManager.GetAsString("GameID4", characterName[3].data(), characterName[3].size());
		gQueryManager.GetAsString("GameID5", characterName[4].data(), characterName[4].size());

		gQueryManager.Close();

		if (!gCharacterManager.GetFreeCharacterSlot(characterName, &pMsg.slot))
		{
			pMsg.Result = 2;
			break;
		}

		if (!gQueryManager.ExecQuery(
			"EXEC WZ_CreateCharacter '%s','%s','%d'",
			lpMsg->Account,
			lpMsg->CharacterName,
			lpMsg->Class) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			pMsg.Result = 0;
			break;
		}

		pMsg.Result = gQueryManager.GetResult(0);

		gQueryManager.Close();

		if (pMsg.Result == 1)
		{
			gQueryManager.ExecQuery(
				"UPDATE AccountCharacter SET GameID%d='%s' WHERE Id='%s'",
				pMsg.slot + 1,
				lpMsg->CharacterName,
				lpMsg->Account);

			gQueryManager.Close();
		}

	} while (false);

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDCharacterDeleteRecv(const SDHP_CHARACTER_DELETE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CHARACTER_DELETE_RECV);

	SDHP_CHARACTER_DELETE_SEND pMsg{};
	pMsg.Header.set(HEAD_CHARACTER_DELETE, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.Result = (gUtil.CheckTextSyntax(lpMsg->CharacterName, sizeof(lpMsg->CharacterName)) != 0);

	do
	{
		if (pMsg.Result == 0)
		{
			break;
		}

		if (!gQueryManager.ExecQuery(
			"EXEC WZ_DeleteCharacter '%s','%s'",
			lpMsg->Account,
			lpMsg->CharacterName) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			pMsg.Result = 0;
			break;
		}

		pMsg.Result = gQueryManager.GetResult(0);

		gQueryManager.Close();

		if (pMsg.Result != 1)
		{
			break;
		}

		std::array<std::array<char, MAX_CHARACTER_NAME>, MAX_CHARACTER_SLOT> characterName{};

		if (!gQueryManager.ExecQuery("SELECT * FROM AccountCharacter WHERE Id='%s'", lpMsg->Account) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			break;
		}

		gQueryManager.GetAsString("GameID1", characterName[0].data(), characterName[0].size());
		gQueryManager.GetAsString("GameID2", characterName[1].data(), characterName[1].size());
		gQueryManager.GetAsString("GameID3", characterName[2].data(), characterName[2].size());
		gQueryManager.GetAsString("GameID4", characterName[3].data(), characterName[3].size());
		gQueryManager.GetAsString("GameID5", characterName[4].data(), characterName[4].size());

		gQueryManager.Close();

		BYTE slot = 0;

		if (gCharacterManager.GetCharacterSlot(characterName, lpMsg->CharacterName, &slot))
		{
			gQueryManager.ExecQuery(
				"UPDATE AccountCharacter SET GameID%d=NULL WHERE Id='%s'",
				slot + 1,
				lpMsg->Account);

			gQueryManager.Close();
		}

	} while (false);

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDCharacterInfoRecv(const SDHP_CHARACTER_INFO_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CHARACTER_INFO_RECV);

	SDHP_CHARACTER_INFO_SEND pMsg{};
	pMsg.Header.set(HEAD_CHARACTER_INFO, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = (gUtil.CheckTextSyntax(lpMsg->CharacterName, sizeof(lpMsg->CharacterName)) != 0);

	do
	{
		if (pMsg.Result == 0)
		{
			break;
		}

		if (!gQueryManager.ExecQuery(
			"SELECT * FROM Character WHERE AccountID='%s' AND Name='%s'",
			lpMsg->Account,
			lpMsg->CharacterName) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			pMsg.Result = 0;
			break;
		}

		pMsg.Level = static_cast<WORD>(gQueryManager.GetAsInteger("cLevel"));
		pMsg.Class = static_cast<BYTE>(gQueryManager.GetAsInteger("Class"));
		pMsg.LevelUpPoint = gQueryManager.GetAsInteger("LevelUpPoint");
		pMsg.Experience = gQueryManager.GetAsInteger("Experience");
		pMsg.Strength = gQueryManager.GetAsInteger("Strength");
		pMsg.Dexterity = gQueryManager.GetAsInteger("Dexterity");
		pMsg.Vitality = gQueryManager.GetAsInteger("Vitality");
		pMsg.Energy = gQueryManager.GetAsInteger("Energy");
		pMsg.Leadership = gQueryManager.GetAsInteger("Leadership");
		pMsg.Money = gQueryManager.GetAsInteger("Money");

		pMsg.PKCount = gQueryManager.GetAsInteger("PkCount");
		pMsg.PKLevel = gQueryManager.GetAsInteger("PkLevel");
		pMsg.PKTime = gQueryManager.GetAsInteger("PkTime");
		pMsg.CtlCode = static_cast<BYTE>(gQueryManager.GetAsInteger("CtlCode"));
		pMsg.FruitAddPoint = static_cast<WORD>(gQueryManager.GetAsInteger("FruitAddPoint"));
		pMsg.FruitSubPoint = static_cast<WORD>(gQueryManager.GetAsInteger("FruitSubPoint"));
		pMsg.Kills = static_cast<DWORD>(gQueryManager.GetAsInteger("Kills"));
		pMsg.Deads = static_cast<DWORD>(gQueryManager.GetAsInteger("Deads"));
		pMsg.TheGift = gQueryManager.GetAsInteger("TheGift");
		pMsg.CTCTime = static_cast<DWORD>(gQueryManager.GetAsInteger("CTCTime"));
		pMsg.CTCRegDay = static_cast<DWORD>(gQueryManager.GetAsInteger("CTCRegDay"));

		pMsg.Life = static_cast<DWORD>(gQueryManager.GetAsFloat("Life"));
		pMsg.MaxLife = static_cast<DWORD>(gQueryManager.GetAsFloat("MaxLife"));
		pMsg.Mana = static_cast<DWORD>(gQueryManager.GetAsFloat("Mana"));
		pMsg.MaxMana = static_cast<DWORD>(gQueryManager.GetAsFloat("MaxMana"));
		pMsg.BP = static_cast<DWORD>(gQueryManager.GetAsFloat("BP"));
		pMsg.MaxBP = static_cast<DWORD>(gQueryManager.GetAsFloat("MaxBP"));
		pMsg.Shield = static_cast<DWORD>(gQueryManager.GetAsFloat("Shield"));
		pMsg.MaxShield = static_cast<DWORD>(gQueryManager.GetAsFloat("MaxShield"));

		pMsg.Map = static_cast<BYTE>(gQueryManager.GetAsInteger("MapNumber"));
		pMsg.X = static_cast<BYTE>(gQueryManager.GetAsInteger("MapPosX"));
		pMsg.Y = static_cast<BYTE>(gQueryManager.GetAsInteger("MapPosY"));
		pMsg.Dir = static_cast<BYTE>(gQueryManager.GetAsInteger("MapDir"));

		gQueryManager.GetAsBinary("Inventory", pMsg.Inventory[0], sizeof(pMsg.Inventory));
		gQueryManager.GetAsBinary("MagicList", pMsg.Skill[0], sizeof(pMsg.Skill));
		gQueryManager.GetAsBinary("Quest", pMsg.Quest, sizeof(pMsg.Quest));
		gQueryManager.GetAsBinary("EffectList", pMsg.Effect[0], sizeof(pMsg.Effect));

#if(DATASERVER_UPDATE >= 602)
		pMsg.ExtInventory = static_cast<BYTE>(gQueryManager.GetAsInteger("ExtInventory"));
#endif

#if(FLAG_SKIN)
		pMsg.Flag = gQueryManager.GetAsInteger("Flag");
#endif

#if(DANHHIEU_NEW)
		pMsg.rDanhHieu = gQueryManager.GetAsInteger("rDanhHieu");
		pMsg.DHSatThuong = gQueryManager.GetAsInteger("DHSatThuong");
		pMsg.DHSatThuongX2 = gQueryManager.GetAsInteger("DHSatThuongX2");
		pMsg.DHPhongThu = gQueryManager.GetAsInteger("DHPhongThu");
		pMsg.DHHP = gQueryManager.GetAsInteger("DHHP");
		pMsg.DHSD = gQueryManager.GetAsInteger("DHSD");
		pMsg.DHGST = gQueryManager.GetAsInteger("DHGST");
#endif

#if(TULUYEN_NEW)
		pMsg.rTuLuyen = gQueryManager.GetAsInteger("rTuLuyen");
#endif

		pMsg.mUserSkinPick = static_cast<DWORD>(gQueryManager.GetAsInteger("UserSkinPick"));

#if(CHONPHEDOILAP)
		pMsg.ChonPheHanhTau = static_cast<BYTE>(gQueryManager.GetAsInteger("PheHanhTau"));
#endif

#if(B_HON_HOAN)
		pMsg.CapDoHonHoan = static_cast<BYTE>(gQueryManager.GetAsInteger("HonHoan"));
		pMsg.PointUsePhe = static_cast<DWORD>(gQueryManager.GetAsInteger("PointUsePhe"));
#endif

#if(EVENT_END_LESS)
		pMsg.mLuotDiEndLess = static_cast<WORD>(gQueryManager.GetAsInteger("LuotDiEndLess"));
#endif

#if(MOCNAP == 1)
		pMsg.MOCNAPCOIN = static_cast<DWORD>(gQueryManager.GetAsInteger("MOCNAPCOIN"));
#endif

		gQueryManager.Close();

#if(DATASERVER_UPDATE >= 602)
		if (gQueryManager.ExecQuery(
			"SELECT ExtWarehouse FROM AccountCharacter WHERE Id='%s'",
			lpMsg->Account) &&
			gQueryManager.Fetch() != SQL_NO_DATA)
		{
			pMsg.ExtWarehouse = static_cast<BYTE>(gQueryManager.GetAsInteger("ExtWarehouse"));
		}

		gQueryManager.Close();
#endif

		if (gQueryManager.ExecQuery(
			"EXEC WZ_GetResetInfo '%s','%s'",
			lpMsg->Account,
			lpMsg->CharacterName) &&
			gQueryManager.Fetch() != SQL_NO_DATA)
		{
			pMsg.Reset = gQueryManager.GetAsInteger("Reset");
		}

		gQueryManager.Close();

		if (gQueryManager.ExecQuery(
			"EXEC WZ_GetMasterResetInfo '%s','%s'",
			lpMsg->Account,
			lpMsg->CharacterName) &&
			gQueryManager.Fetch() != SQL_NO_DATA)
		{
			pMsg.MasterReset = gQueryManager.GetAsInteger("MasterReset");
		}

		gQueryManager.Close();

#if(DATASERVER_UPDATE >= 801)
		GUILD_MATCHING_INFO guildMatchingInfo{};
		GUILD_MATCHING_JOIN_INFO guildMatchingJoinInfo{};

		GUILD_INFO* lpGuildInfo = gGuildManager.GetMemberGuildInfo(lpMsg->CharacterName);

		pMsg.UseGuildMatching = (lpGuildInfo == nullptr)
			? 0
			: gGuildMatching.GetGuildMatchingInfo(&guildMatchingInfo, lpGuildInfo->Name);

		pMsg.UseGuildMatchingJoin = (lpGuildInfo != nullptr)
			? 0
			: gGuildMatching.GetGuildMatchingJoinInfo(&guildMatchingJoinInfo, lpMsg->CharacterName);
#endif

		gQueryManager.ExecQuery(
			"UPDATE AccountCharacter SET GameIDC='%s' WHERE Id='%s'",
			lpMsg->CharacterName,
			lpMsg->Account);

		gQueryManager.Close();

	} while (false);

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDCreateItemRecv(const SDHP_CREATE_ITEM_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CREATE_ITEM_RECV);

	SDHP_CREATE_ITEM_SEND pMsg{};
	pMsg.Header.set(HEAD_CREATE_ITEM, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.X = lpMsg->X;
	pMsg.Y = lpMsg->Y;
	pMsg.Map = lpMsg->Map;

	if (gQueryManager.ExecQuery("EXEC WZ_GetItemSerial") &&
		gQueryManager.Fetch() != SQL_NO_DATA)
	{
		pMsg.Serial = gQueryManager.GetResult(0);
	}

	gQueryManager.Close();

	if (pMsg.Serial != 0 &&
		(lpMsg->ItemIndex == 0x1A04 || lpMsg->ItemIndex == 0x1A05))
	{
		gQueryManager.ExecQuery(
			"INSERT INTO T_PetItem_Info (ItemSerial,Pet_Level,Pet_Exp) VALUES (%d,1,0)",
			pMsg.Serial);

		gQueryManager.Close();
	}

	pMsg.ItemIndex = lpMsg->ItemIndex;
	pMsg.Level = lpMsg->Level;
	pMsg.Dur = lpMsg->Dur;
	pMsg.Option1 = lpMsg->Option1;
	pMsg.Option2 = lpMsg->Option2;
	pMsg.Option3 = lpMsg->Option3;
	pMsg.NewOption = lpMsg->NewOption;
	pMsg.LootIndex = lpMsg->LootIndex;
	pMsg.SetOption = lpMsg->SetOption;
	pMsg.JewelOfHarmonyOption = lpMsg->JewelOfHarmonyOption;
	pMsg.ItemOptionEx = lpMsg->ItemOptionEx;

	std::memcpy(pMsg.SocketOption, lpMsg->SocketOption, sizeof(pMsg.SocketOption));

	pMsg.SocketOptionBonus = lpMsg->SocketOptionBonus;
	pMsg.Duration = lpMsg->Duration;

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDOptionDataRecv(const SDHP_OPTION_DATA_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_OPTION_DATA_RECV);

	SDHP_OPTION_DATA_SEND pMsg{};
	pMsg.Header.set(HEAD_OPTION_DATA, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	const bool dataFound =
		gQueryManager.ExecQuery(
			"SELECT * FROM OptionData WHERE Name='%s'",
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		gQueryManager.GetAsBinary("SkillKey", pMsg.SkillKey, sizeof(pMsg.SkillKey));

		pMsg.GameOption = static_cast<BYTE>(gQueryManager.GetAsInteger("GameOption"));
		pMsg.QKey = static_cast<BYTE>(gQueryManager.GetAsInteger("Qkey"));
		pMsg.WKey = static_cast<BYTE>(gQueryManager.GetAsInteger("Wkey"));
		pMsg.EKey = static_cast<BYTE>(gQueryManager.GetAsInteger("Ekey"));
		pMsg.ChatWindow = static_cast<BYTE>(gQueryManager.GetAsInteger("ChatWindow"));
		pMsg.RKey = static_cast<BYTE>(gQueryManager.GetAsInteger("Rkey"));
		pMsg.QWERLevel = gQueryManager.GetAsInteger("QWERLevel");

#if(DATASERVER_UPDATE >= 701)
		pMsg.ChangeSkin = static_cast<BYTE>(gQueryManager.GetAsInteger("ChangeSkin"));
#endif
	}

	gQueryManager.Close();

	if (!dataFound)
	{
		std::memset(pMsg.SkillKey, 0xFF, sizeof(pMsg.SkillKey));

		pMsg.GameOption = 0xFF;
		pMsg.QKey = 0xFF;
		pMsg.WKey = 0xFF;
		pMsg.EKey = 0xFF;
		pMsg.ChatWindow = 0xFF;
		pMsg.RKey = 0xFF;
		pMsg.QWERLevel = 0xFFFFFFFF;

#if(DATASERVER_UPDATE >= 701)
		pMsg.ChangeSkin = 0;
#endif
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDPetItemInfoRecv(const SDHP_PET_ITEM_INFO_RECV* lpMsg, int serverIndex, int size)
{
	const size_t expectedSize =
		sizeof(SDHP_PET_ITEM_INFO_RECV) +
		(static_cast<size_t>(lpMsg->Count) * sizeof(SDHP_PET_ITEM_INFO1));

	if (size < 0 || static_cast<size_t>(size) != expectedSize)
	{
		Log.ToDisp(
			LOG_RED,
			"[%s] Tamaño de paquete invalido (ServerIndex: %d, Size: %d, Expected: %zu)",
			__FUNCTION__,
			serverIndex,
			size,
			expectedSize);

		return;
	}

	SDHP_PET_ITEM_INFO_SEND pMsg{};
	pMsg.Header.set(HEAD_PET_ITEM, 0);
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.type = lpMsg->Type;
	pMsg.count = 0;

	const size_t packetSize =
		sizeof(SDHP_PET_ITEM_INFO_SEND) +
		(static_cast<size_t>(lpMsg->Count) * sizeof(SDHP_PET_ITEM_INFO2));

	std::vector<BYTE> sendBuffer(packetSize);

	const auto* itemList = reinterpret_cast<const SDHP_PET_ITEM_INFO1*>(
		reinterpret_cast<const BYTE*>(lpMsg) + sizeof(SDHP_PET_ITEM_INFO_RECV));

	size_t offset = sizeof(SDHP_PET_ITEM_INFO_SEND);

	for (BYTE n = 0; n < lpMsg->Count; ++n)
	{
		const auto& item = itemList[n];

		SDHP_PET_ITEM_INFO2 info{};
		info.Slot = item.Slot;
		info.Serial = item.Serial;
		info.Level = 1;
		info.Experience = 0;

		const bool dataFound =
			gQueryManager.ExecQuery(
				"SELECT Pet_Level,Pet_Exp FROM T_PetItem_Info WHERE ItemSerial=%d",
				item.Serial) &&
			gQueryManager.Fetch() != SQL_NO_DATA;

		if (dataFound)
		{
			info.Level = gQueryManager.GetAsInteger("Pet_Level");
			info.Experience = gQueryManager.GetAsInteger("Pet_Exp");
		}

		gQueryManager.Close();

		if (!dataFound)
		{
			gQueryManager.ExecQuery(
				"INSERT INTO T_PetItem_Info (ItemSerial,Pet_Level,Pet_Exp) VALUES (%d,1,0)",
				item.Serial);

			gQueryManager.Close();
		}

		std::memcpy(sendBuffer.data() + offset, &info, sizeof(info));

		offset += sizeof(info);
		++pMsg.count;
	}

	const WORD packetLength = static_cast<WORD>(offset);

	pMsg.Header.size[0] = SET_NUMBERHB(packetLength);
	pMsg.Header.size[1] = SET_NUMBERLB(packetLength);

	std::memcpy(sendBuffer.data(), &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, sendBuffer.data(), packetLength);
}

void GDCharacterNameCheckRecv(const SDHP_CHARACTER_NAME_CHECK_RECV* lpMsg, int serverIndex, int size)
{
#if(DATASERVER_UPDATE >= 401)

	VALIDATE_PACKET_SIZE(SDHP_CHARACTER_NAME_CHECK_RECV);

	SDHP_CHARACTER_NAME_CHECK_SEND pMsg{};

	pMsg.Header.set(HEAD_CHARACTER_NAME_CHECK, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	// Lógica simplificada: 
	// CheckTextSyntax y CheckSyntax devuelven 0 si el texto es inválido.
	// El operador '!' invierte el resultado: 0 se vuelve true(1), y no-0 se vuelve false(0).
	// Por lo tanto, si la sintaxis es inválida o contiene palabras prohibidas, Result será 1.
	pMsg.Result = static_cast<BYTE>(
		!gUtil.CheckTextSyntax(lpMsg->CharacterName, sizeof(lpMsg->CharacterName)) ||
		!gBadSyntax.CheckSyntax(lpMsg->CharacterName)
		);

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);

#endif
}

void GDCharacterNameChangeRecv(const SDHP_CHARACTER_NAME_CHANGE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 401)

	VALIDATE_PACKET_SIZE(SDHP_CHARACTER_NAME_CHANGE_RECV);

	SDHP_CHARACTER_NAME_CHANGE_SEND pMsg{};
	pMsg.Header.set(HEAD_CHARACTER_RENAME, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.OldName, lpMsg->OldName, sizeof(pMsg.OldName));
	std::memcpy(pMsg.NewName, lpMsg->NewName, sizeof(pMsg.NewName));

	pMsg.Result =
		(gUtil.CheckTextSyntax(lpMsg->NewName, sizeof(lpMsg->NewName)) == 0 ||
			gBadSyntax.CheckSyntax(lpMsg->NewName) == 0);

	if (pMsg.Result == 0)
	{
		if (gQueryManager.ExecQuery(
			"EXEC WZ_RenameCharacter '%s','%s','%s'",
			lpMsg->Account,
			lpMsg->OldName,
			lpMsg->NewName) &&
			gQueryManager.Fetch() != SQL_NO_DATA)
		{
			pMsg.Result = gQueryManager.GetResult(0);
		}
		else
		{
			pMsg.Result = 1;
		}

		gQueryManager.Close();
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void GDSaveTheGiftRecv(THEGIFT_GD_SAVE_DATA* lpMsg)
{
	gQueryManager.ExecQuery("UPDATE Character SET TheGift=%d WHERE Name='%s'", lpMsg->TheGift, lpMsg->Name);

	gQueryManager.Close();
}

void GDChangePasswordRecv(SDHP_CHANGE_PASSWORD_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CHANGE_PASSWORD_RECV);

	SDHP_CHANGE_PASSWORD_SEND pMsg {};
	pMsg.Header.set(GS_HEAD_CUSTOM_EXTENSIONS, GS_SUB_CHANGE_PASSWORD_RESULT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;
	
	if (!gQueryManager.ExecQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s' COLLATE Latin1_General_BIN", lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.Result = 1;
		gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}
	char password[11] = { 0 };

	gQueryManager.GetAsString("memb__pwd", password, sizeof(password));

	if (strcmp(lpMsg->OldPassword, password) != 0)
	{
		gQueryManager.Close();
		pMsg.Result = 2;
		gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		return;
	}

	gQueryManager.Close();

	gQueryManager.ExecQuery("UPDATE MEMB_INFO SET memb__pwd = '%s' WHERE memb___id = '%s'", lpMsg->NewPassword, lpMsg->Account);
	pMsg.Result = 3;
	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
	Log.ToDisp(LOG_RED, "[DataServerProtocol - GDChangePasswordRecv] Cambio de password correcto. (Cuenta: %s)", lpMsg->Account);
	gQueryManager.Close();
}









void GDCrywolfSyncRecv(SDHP_CRYWOLF_SYNC_RECV* lpMsg, int aIndex)
{
	SDHP_CRYWOLF_SYNC_SEND pMsg{};
	pMsg.Header.set(HEAD_CRYWOLF_SYNC, sizeof(pMsg));

	pMsg.MapServerGroup = lpMsg->MapServerGroup;
	pMsg.CrywolfState = lpMsg->CrywolfState;
	pMsg.OccupationState = lpMsg->OccupationState;

	// Iteramos sobre todos los servidores y enviamos el paquete a los que estén online
	for (int n = 0; n < MAX_SERVER; ++n)
	{
		// IsOnline() devuelve un valor distinto de 0 si está conectado (evalúa como true)
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void GDCrywolfInfoRecv(SDHP_CRYWOLF_INFO_RECV* lpMsg, int aIndex)
{
	SDHP_CRYWOLF_INFO_SEND pMsg{};
	pMsg.Header.set(HEAD_CRYWOLF_INFO, sizeof(pMsg));
	pMsg.MapServerGroup = lpMsg->MapServerGroup;

	const bool dataFound =
		gQueryManager.ExecQuery(
			"EXEC WZ_CW_InfoLoad '%d'",
			lpMsg->MapServerGroup) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.CrywolfState = static_cast<BYTE>(gQueryManager.GetAsInteger("CRYWOLF_STATE"));
		pMsg.OccupationState = static_cast<BYTE>(gQueryManager.GetAsInteger("CRYWOLF_OCCUFY"));
	}
	else
	{
		// Valores por defecto en caso de que la DB falle o no tenga datos
		pMsg.CrywolfState = 0;
		pMsg.OccupationState = 0;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDGlobalPostRecv(SDHP_GLOBAL_POST_RECV* lpMsg, int aIndex)
{
	SDHP_GLOBAL_POST_SEND pMsg{};
	pMsg.Header.set(HEAD_GLOBAL_POST, sizeof(pMsg));

	pMsg.MapServerGroup = lpMsg->MapServerGroup;
	pMsg.type = lpMsg->type;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.message, lpMsg->message, sizeof(pMsg.message));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void GDGlobalItemPostRecv(SDHP_GLOBAL_ITEM_POST_RECV* lpMsg, int aIndex)
{
	SDHP_GLOBAL_ITEM_POST_SEND pMsg{};
	pMsg.Header.set(HEAD_GLOBAL_ITEM_POST, sizeof(pMsg));
	pMsg.MapServerGroup = lpMsg->MapServerGroup;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.message, lpMsg->message, sizeof(pMsg.message));
	std::memcpy(pMsg.item_data, lpMsg->item_data, sizeof(pMsg.item_data));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void GDGlobalNoticeRecv(SDHP_GLOBAL_NOTICE_RECV* lpMsg, int aIndex)
{
	SDHP_GLOBAL_NOTICE_SEND pMsg{};
	pMsg.Header.set(HEAD_GLOBAL_NOTICE, sizeof(pMsg));

	pMsg.MapServerGroup = lpMsg->MapServerGroup;
	pMsg.type = lpMsg->type;
	pMsg.count = lpMsg->count;
	pMsg.opacity = lpMsg->opacity;
	pMsg.delay = lpMsg->delay;
	pMsg.color = lpMsg->color;
	pMsg.speed = lpMsg->speed;

	std::memcpy(pMsg.message, lpMsg->message, sizeof(pMsg.message));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void GDSNSDataRecv(SDHP_SNS_DATA_RECV* lpMsg, int aIndex)
{
#if (DATASERVER_UPDATE >= 801)

	SDHP_SNS_DATA_SEND pMsg{};
	pMsg.Header.set(HEAD_SNS_DATA, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	const bool dataFound =
		gQueryManager.ExecQuery(
			"SELECT Data FROM SNSData WHERE Name='%s'",
			lpMsg->CharacterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA;

	if (dataFound)
	{
		pMsg.Result = 0;
		gQueryManager.GetAsBinary("Data", pMsg.data, sizeof(pMsg.data));
	}
	else
	{
		pMsg.Result = 1;
		std::memset(pMsg.data, 0xFF, sizeof(pMsg.data));
	}

	gQueryManager.Close();

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void GDCharacterInfoSaveRecv(SDHP_CHARACTER_INFO_SAVE_RECV* lpMsg)
{
	// --- 1. CONSULTA PRINCIPAL CON PARÁMETROS BINARIOS ---
	gQueryManager.BindParameterAsBinary(1, lpMsg->Inventory[0], sizeof(lpMsg->Inventory));
	gQueryManager.BindParameterAsBinary(2, lpMsg->Skill[0], sizeof(lpMsg->Skill));
	gQueryManager.BindParameterAsBinary(3, lpMsg->Quest, sizeof(lpMsg->Quest));
	gQueryManager.BindParameterAsBinary(4, lpMsg->Effect[0], sizeof(lpMsg->Effect));

	////==Chien Truong Co
	//gQueryManager.ExecQuery("UPDATE Character SET CTCTime=%d, CTCRegDay=%d  WHERE AccountID='%s' WHERE Name='%s'", lpMsg->CTCTime, lpMsg->CTCRegDay, lpMsg->Account, lpMsg->CharacterName);
	//LogAdd(LOG_BLUE, "UPDATE Character SET CTCTime=%d, CTCRegDay=%d  WHERE AccountID='%s' AND Name='%s'", lpMsg->CTCTime, lpMsg->CTCRegDay, lpMsg->Account, lpMsg->CharacterName);
	//gQueryManager.Close();

#if(DATASERVER_UPDATE >= 602)
	if (!gQueryManager.ExecQuery(
		"UPDATE Character SET "
		"cLevel=%d, Class=%d, LevelUpPoint=%d, Experience=%d, "
		"Strength=%d, Dexterity=%d, Vitality=%d, Energy=%d, Leadership=%d, "
		"Inventory=?, MagicList=?, Money=%d, "
		"Life=%f, MaxLife=%f, Mana=%f, MaxMana=%f, "
		"BP=%f, MaxBP=%f, Shield=%f, MaxShield=%f, "
		"MapNumber=%d, MapPosX=%d, MapPosY=%d, MapDir=%d, "
		"PkCount=%d, PkLevel=%d, PkTime=%d, "
		"Quest=?, EffectList=?, "
		"FruitAddPoint=%d, FruitSubPoint=%d, ExtInventory=%d "
		"WHERE AccountID='%s' AND Name='%s'",
		lpMsg->Level, lpMsg->Class, lpMsg->LevelUpPoint, lpMsg->Experience,
		lpMsg->Strength, lpMsg->Dexterity, lpMsg->Vitality, lpMsg->Energy, lpMsg->Leadership,
		lpMsg->Money,
		static_cast<float>(lpMsg->Life), static_cast<float>(lpMsg->MaxLife),
		static_cast<float>(lpMsg->Mana), static_cast<float>(lpMsg->MaxMana),
		static_cast<float>(lpMsg->BP), static_cast<float>(lpMsg->MaxBP),
		static_cast<float>(lpMsg->Shield), static_cast<float>(lpMsg->MaxShield),
		lpMsg->Map, lpMsg->X, lpMsg->Y, lpMsg->Dir,
		lpMsg->PKCount, lpMsg->PKLevel, lpMsg->PKTime,
		lpMsg->FruitAddPoint, lpMsg->FruitSubPoint, lpMsg->ExtInventory,
		lpMsg->Account, lpMsg->CharacterName))
	{
		gQueryManager.Close();
		return;
	}
#else
	if (!gQueryManager.ExecQuery(
		"UPDATE Character SET "
		"cLevel=%d, Class=%d, LevelUpPoint=%d, Experience=%d, "
		"Strength=%d, Dexterity=%d, Vitality=%d, Energy=%d, Leadership=%d, "
		"Inventory=?, MagicList=?, Money=%d, "
		"Life=%f, MaxLife=%f, Mana=%f, MaxMana=%f, "
		"BP=%f, MaxBP=%f, Shield=%f, MaxShield=%f, "
		"MapNumber=%d, MapPosX=%d, MapPosY=%d, MapDir=%d, "
		"PkCount=%d, PkLevel=%d, PkTime=%d, "
		"Quest=?, EffectList=?, "
		"FruitAddPoint=%d, FruitSubPoint=%d "
		"WHERE AccountID='%s' AND Name='%s'",
		lpMsg->Level, lpMsg->Class, lpMsg->LevelUpPoint, lpMsg->Experience,
		lpMsg->Strength, lpMsg->Dexterity, lpMsg->Vitality, lpMsg->Energy, lpMsg->Leadership,
		lpMsg->Money,
		static_cast<float>(lpMsg->Life), static_cast<float>(lpMsg->MaxLife),
		static_cast<float>(lpMsg->Mana), static_cast<float>(lpMsg->MaxMana),
		static_cast<float>(lpMsg->BP), static_cast<float>(lpMsg->MaxBP),
		static_cast<float>(lpMsg->Shield), static_cast<float>(lpMsg->MaxShield),
		lpMsg->Map, lpMsg->X, lpMsg->Y, lpMsg->Dir,
		lpMsg->PKCount, lpMsg->PKLevel, lpMsg->PKTime,
		lpMsg->FruitAddPoint, lpMsg->FruitSubPoint,
		lpMsg->Account, lpMsg->CharacterName))
	{
		gQueryManager.Close();
		return;
	}
#endif
	gQueryManager.Close();

	// --- 2. CONSULTA AGRUPADA DE SISTEMAS CUSTOM (SQL FIJO) ---
	// Los #if se aplican tanto al literal de cadena como a los argumentos,
	// garantizando un alineamiento 1:1 en tiempo de compilación.
	if (!gQueryManager.ExecQuery(
		"UPDATE Character SET "
		"UserSkinPick=%d, Kills=%d, Deads=%d"
#if (CHONPHEDOILAP)
		", PheHanhTau=%d"
#endif
#if (EVENT_END_LESS)
		", LuotDiEndLess=%d"
#endif
#if (B_HON_HOAN)
		", HonHoan=%d, PointUsePhe=%d"
#endif
#if (DANHHIEU_NEW)
		", rDanhHieu=%d, DHSatThuong=%d, DHSatThuongX2=%d, DHPhongThu=%d, DHHP=%d, DHSD=%d, DHGST=%d"
#endif
#if (TULUYEN_NEW)
		", rTuLuyen=%d"
#endif
#if (MOCNAP == 1)
		", MOCNAPCOIN=%d"
#endif
		// Lógica de CTC unificada en un solo lugar y con WHERE seguro
#if (CHIENTRUONGCO == 1)
		", CTCTime=%d, CTCRegDay=%d"
#else
		", CTCTime=%d, CTCRegDay=0" // Bug original corregido: Agregado WHERE abajo, y CTCRegDay=0
#endif
#if (FLAG_SKIN)
		", Flag=%d"
#endif
		" WHERE AccountID='%s' AND Name='%s'",

		// Argumentos (mismo orden exacto que los bloques #if de arriba)
		lpMsg->mUserSkinPick, lpMsg->Kills, lpMsg->Deads,
#if (CHONPHEDOILAP)
		lpMsg->ChonPheHanhTau,
#endif
#if (EVENT_END_LESS)
		lpMsg->mLuotDiEndLess,
#endif
#if (B_HON_HOAN)
		lpMsg->CapDoHonHoan, lpMsg->PointUsePhe,
#endif
#if (DANHHIEU_NEW)
		lpMsg->rDanhHieu, lpMsg->DHSatThuong, lpMsg->DHSatThuongX2, lpMsg->DHPhongThu, lpMsg->DHHP, lpMsg->DHSD, lpMsg->DHGST,
#endif
#if (TULUYEN_NEW)
		lpMsg->rTuLuyen,
#endif
#if (MOCNAP == 1)
		lpMsg->MOCNAPCOIN,
#endif
#if (CHIENTRUONGCO == 1)
		lpMsg->CTCTime, lpMsg->CTCRegDay,
#else
		static_cast<int>(RSTimeCTC * 60), // El valor para CTCTime si el sistema está desactivado
#endif
#if (FLAG_SKIN)
		lpMsg->Flag,
#endif
		lpMsg->Account, lpMsg->CharacterName))
	{
		gQueryManager.Close();
		return;
	}
	gQueryManager.Close();

	// --- 3. ACTUALIZACIÓN DE TABLA AccountCharacter ---
#if (DATASERVER_UPDATE >= 602)
	if (!gQueryManager.ExecQuery("UPDATE AccountCharacter SET ExtWarehouse=%d WHERE Id='%s'", lpMsg->ExtWarehouse, lpMsg->Account))
	{
		gQueryManager.Close();
		return;
	}
	gQueryManager.Close();
#endif
}

void GDInventoryItemSaveRecv(SDHP_INVENTORY_ITEM_SAVE_RECV* lpMsg)
{
	gQueryManager.BindParameterAsBinary(1,lpMsg->Inventory[0],sizeof(lpMsg->Inventory));
	gQueryManager.ExecQuery("UPDATE Character SET Inventory=? WHERE AccountID='%s' AND Name='%s'",lpMsg->Account,lpMsg->CharacterName);
	gQueryManager.Close();
}

void GDOptionDataSaveRecv(SDHP_OPTION_DATA_SAVE_RECV* lpMsg)
{
	if(!gQueryManager.ExecQuery("SELECT Name FROM OptionData WHERE Name='%s'",lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		#if(DATASERVER_UPDATE>=701)
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->SkillKey,sizeof(lpMsg->SkillKey));
		gQueryManager.ExecQuery("INSERT INTO OptionData (Name,SkillKey,GameOption,Qkey,Wkey,Ekey,ChatWindow,Rkey,QWERLevel,ChangeSkin) VALUES ('%s',?,%d,%d,%d,%d,%d,%d,%d,%d)",lpMsg->CharacterName,lpMsg->GameOption,lpMsg->QKey,lpMsg->WKey,lpMsg->EKey,lpMsg->ChatWindow,lpMsg->RKey,lpMsg->QWERLevel,lpMsg->ChangeSkin);
		gQueryManager.Close();
		#else
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->SkillKey,sizeof(lpMsg->SkillKey));
		gQueryManager.ExecQuery("INSERT INTO OptionData (Name,SkillKey,GameOption,Qkey,Wkey,Ekey,ChatWindow,Rkey,QWERLevel) VALUES ('%s',?,%d,%d,%d,%d,%d,%d,%d)",lpMsg->CharacterName,lpMsg->GameOption,lpMsg->QKey,lpMsg->WKey,lpMsg->EKey,lpMsg->ChatWindow,lpMsg->RKey,lpMsg->QWERLevel);
		gQueryManager.Close();
		#endif
	}
	else
	{
		#if(DATASERVER_UPDATE>=701)
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->SkillKey,sizeof(lpMsg->SkillKey));
		gQueryManager.ExecQuery("UPDATE OptionData SET SkillKey=?,GameOption=%d,Qkey=%d,Wkey=%d,Ekey=%d,ChatWindow=%d,Rkey=%d,QWERLevel=%d,ChangeSkin=%d WHERE Name='%s'",lpMsg->GameOption,lpMsg->QKey,lpMsg->WKey,lpMsg->EKey,lpMsg->ChatWindow,lpMsg->RKey,lpMsg->QWERLevel,lpMsg->ChangeSkin,lpMsg->CharacterName);
		gQueryManager.Close();
		#else
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->SkillKey,sizeof(lpMsg->SkillKey));
		gQueryManager.ExecQuery("UPDATE OptionData SET SkillKey=?,GameOption=%d,Qkey=%d,Wkey=%d,Ekey=%d,ChatWindow=%d,Rkey=%d,QWERLevel=%d WHERE Name='%s'",lpMsg->GameOption,lpMsg->QKey,lpMsg->WKey,lpMsg->EKey,lpMsg->ChatWindow,lpMsg->RKey,lpMsg->QWERLevel,lpMsg->CharacterName);
		gQueryManager.Close();
		#endif
	}
}

void GDPetItemInfoSaveRecv(SDHP_PET_ITEM_INFO_SAVE_RECV* lpMsg)
{
	for (int n = 0; n < lpMsg->count; ++n)
	{
		auto* lpInfo = reinterpret_cast<SDHP_PET_ITEM_INFO_SAVE*>(
			reinterpret_cast<BYTE*>(lpMsg) +
			sizeof(SDHP_PET_ITEM_INFO_SAVE_RECV) +
			(sizeof(SDHP_PET_ITEM_INFO_SAVE) * n));

		const bool exists =
			gQueryManager.ExecQuery(
				"SELECT ItemSerial FROM T_PetItem_Info WHERE ItemSerial=%d",
				lpInfo->serial) &&
			(gQueryManager.Fetch() != SQL_NO_DATA);

		gQueryManager.Close();

		if (exists)
		{
			gQueryManager.ExecQuery(
				"UPDATE T_PetItem_Info SET Pet_Level=%d,Pet_Exp=%d WHERE ItemSerial=%d",
				lpInfo->level,
				lpInfo->experience,
				lpInfo->serial);
		}
		else
		{
			gQueryManager.ExecQuery(
				"INSERT INTO T_PetItem_Info (ItemSerial,Pet_Level,Pet_Exp) VALUES (%d,%d,%d)",
				lpInfo->serial,
				lpInfo->level,
				lpInfo->experience);
		}

		gQueryManager.Close();
	}
}

void GDResetInfoSaveRecv(SDHP_RESET_INFO_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_SetResetInfo '%s','%s','%d','%d','%d','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->Reset,
		lpMsg->ResetDay,
		lpMsg->ResetWek,
		lpMsg->ResetMon))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDMasterResetInfoSaveRecv(SDHP_MASTER_RESET_INFO_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_SetMasterResetInfo '%s','%s','%d','%d','%d','%d','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->Reset,
		lpMsg->MasterReset,
		lpMsg->MasterResetDay,
		lpMsg->MasterResetWek,
		lpMsg->MasterResetMon))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDRankingDuelSaveRecv(SDHP_RANKING_DUEL_SAVE_RECV* lpMsg)
{
	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM RankingDuel WHERE Name='%s'",
			lpMsg->CharacterName) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE RankingDuel SET "
			"WinScore=WinScore+%d,"
			"WinScore_semanal=WinScore_semanal+%d,"
			"LoseScore=LoseScore+%d "
			"WHERE Name='%s'",
			lpMsg->WinScore,
			lpMsg->WinScore,
			lpMsg->LoseScore,
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO RankingDuel "
			"(Name,WinScore,LoseScore,WinScore_semanal) "
			"VALUES ('%s',%d,%d,%d)",
			lpMsg->CharacterName,
			lpMsg->WinScore,
			lpMsg->LoseScore,
			lpMsg->WinScore);
	}

	gQueryManager.Close();
}

void GDRankingBloodCastleSaveRecv(SDHP_RANKING_BLOOD_CASTLE_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_RankingBloodCastle '%s','%s','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->score))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDRankingChaosCastleSaveRecv(SDHP_RANKING_CHAOS_CASTLE_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_RankingChaosCastle '%s','%s','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->score))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDRankingDevilSquareSaveRecv(SDHP_RANKING_DEVIL_SQUARE_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_RankingDevilSquare '%s','%s','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->score))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDRankingIllusionTempleSaveRecv(SDHP_RANKING_ILLUSION_TEMPLE_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_RankingIllusionTemple '%s','%s','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->score))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDCreationCardSaveRecv(SDHP_CREATION_CARD_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"UPDATE AccountCharacter SET ExtClass=%d WHERE Id='%s'",
		lpMsg->ExtClass,
		lpMsg->Account))
	{
		// Opcional:
		// const SQLLEN affectedRows = gQueryManager.GetAffectedRows();
	}

	gQueryManager.Close();
}

void GDCrywolfInfoSaveRecv(SDHP_CRYWOLF_INFO_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_CW_InfoSave '%d','%d','%d'",
		lpMsg->MapServerGroup,
		lpMsg->CrywolfState,
		lpMsg->OccupationState))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDSNSDataSaveRecv(SDHP_SNS_DATA_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 801)

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM SNSData WHERE Name='%s'",
			lpMsg->CharacterName) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	gQueryManager.BindParameterAsBinary(1, lpMsg->data, sizeof(lpMsg->data));

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE SNSData SET Data=? WHERE Name='%s'",
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO SNSData (Name,Data) VALUES ('%s',?)",
			lpMsg->CharacterName);
	}

	gQueryManager.Close();

#endif
}

void GDCustomMonsterRewardSaveRecv(SDHP_CUSTOM_MONSTER_REWARD_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_CustomMonsterReward '%s','%s','%d','%d','%d','%d','%d','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->MonsterClass,
		lpMsg->MapNumber,
		lpMsg->RewardValue1,
		lpMsg->RewardValue2,
		lpMsg->RewardValue3,
		lpMsg->RewardValue4))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDRankingCustomArenaSaveRecv(SDHP_RANKING_CUSTOM_ARENA_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_CustomArenaRanking '%s','%s','%d','%d','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->number,
		lpMsg->score,
		lpMsg->rank))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDRankingTvTEventSaveRecv(SDHP_RANKING_TVT_EVENT_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery(
		"EXEC WZ_TvTRanking '%s','%s','%d','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->killcount,
		lpMsg->deathcount))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDConnectCharacterRecv(SDHP_CONNECT_CHARACTER_RECV* lpMsg, int index)
{
	CHARACTER_INFO CharacterInfo{};

	if (gCharacterManager.GetCharacterInfo(&CharacterInfo, lpMsg->CharacterName) != 0)
	{
		return;
	}

	strcpy_s(CharacterInfo.CharacterName, lpMsg->CharacterName);
	strcpy_s(CharacterInfo.Account, lpMsg->Account);

	CharacterInfo.UserIndex = lpMsg->Index;
	CharacterInfo.GameServerCode = gServerManager[index].m_ServerCode;

	gCharacterManager.InsertCharacterInfo(CharacterInfo);

	FriendStateRecv(lpMsg->CharacterName, 0);
}

void GDDisconnectCharacterRecv(SDHP_DISCONNECT_CHARACTER_RECV* lpMsg, int index)
{
	CHARACTER_INFO CharacterInfo{};

	if (gCharacterManager.GetCharacterInfo(&CharacterInfo, lpMsg->CharacterName) == 0)
	{
		return;
	}

	if (CharacterInfo.UserIndex != lpMsg->Index)
	{
		return;
	}

	if (CharacterInfo.GameServerCode != gServerManager[index].m_ServerCode)
	{
		return;
	}

	gCharacterManager.RemoveCharacterInfo(CharacterInfo);

	FriendStateRecv(lpMsg->CharacterName, 1);
}

void GDGlobalWhisperRecv(SDHP_GLOBAL_WHISPER_RECV* lpMsg, int index)
{
	SDHP_GLOBAL_WHISPER_SEND pMsg{};
	pMsg.Header.set(HEAD_GLOBAL_WHISPER, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	CHARACTER_INFO CharacterInfo{};

	if (gCharacterManager.GetCharacterInfo(&CharacterInfo, lpMsg->TargetName) == 0)
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = 1;

		DGGlobalWhisperEchoSend(
			CharacterInfo.GameServerCode,
			CharacterInfo.UserIndex,
			CharacterInfo.Account,
			CharacterInfo.CharacterName,
			lpMsg->CharacterName,
			lpMsg->message);
	}

	memcpy(pMsg.TargetName, lpMsg->TargetName, sizeof(pMsg.TargetName));
	memcpy(pMsg.message, lpMsg->message, sizeof(pMsg.message));

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DGGlobalWhisperEchoSend(WORD ServerCode,WORD index,char* account,char* name,char* SourceName,char* message)
{
	SDHP_GLOBAL_WHISPER_ECHO_SEND pMsg {};
	pMsg.Header.set(HEAD_GLOBAL_WHISPER_ECHO,sizeof(pMsg));
	pMsg.Index = index;

	memcpy(pMsg.Account,account,sizeof(pMsg.Account));
	memcpy(pMsg.CharacterName,name,sizeof(pMsg.CharacterName));
	memcpy(pMsg.SourceName,SourceName,sizeof(pMsg.SourceName));
	memcpy(pMsg.message,message,sizeof(pMsg.message));

	CServerManager* lpServerManager = FindServerByCode(ServerCode);

	if(lpServerManager != 0)
	{
		gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
	}
}

// RAW FUNCTIONS

void DS_GDReqOwnerGuildMaster(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_OWNERGUILDMASTER*>(lpRecv);

	CSP_ANS_OWNERGUILDMASTER pMsg{};
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_OWNER_GUILD_MASTER, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	if (!gCastleDBSet.DSDB_QueryOwnerGuildMaster(lpMsg->MapSvrNum, &pMsg))
	{
		pMsg.Result = 0;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqCastleNpcBuy(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_NPCBUY*>(lpRecv);

	CSP_ANS_NPCBUY pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_NPC_BUY, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.NpcNumber = lpMsg->NpcNumber;
	pMsg.NpcIndex = lpMsg->NpcIndex;
	pMsg.BuyCost = lpMsg->BuyCost;

	int QueryResult = 0;

	if (!gCastleDBSet.DSDB_QueryCastleNpcBuy(lpMsg->MapSvrNum, lpMsg, &QueryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = QueryResult;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqCastleNpcRepair(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_NPCREPAIR*>(lpRecv);

	CSP_ANS_NPCREPAIR pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_NPC_REPAIR, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.NpcNumber = lpMsg->NpcNumber;
	pMsg.NpcIndex = lpMsg->NpcIndex;
	pMsg.RepairCost = lpMsg->RepairCost;

	int QueryResult = 0;

	if (!gCastleDBSet.DSDB_QueryCastleNpcRepair(lpMsg->MapSvrNum, lpMsg, &pMsg, &QueryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = QueryResult;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqCastleNpcUpgrade(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_NPCUPGRADE*>(lpRecv);

	CSP_ANS_NPCUPGRADE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_NPC_UPGRADE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.NpcNumber = lpMsg->NpcNumber;
	pMsg.NpcIndex = lpMsg->NpcIndex;
	pMsg.NpcUpType = lpMsg->NpcUpType;
	pMsg.NpcUpValue = lpMsg->NpcUpValue;
	pMsg.NpcUpIndex = lpMsg->NpcUpIndex;

	pMsg.Result = gCastleDBSet.DSDB_QueryCastleNpcUpgrade(lpMsg->MapSvrNum, lpMsg) ? 1 : 0;

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqTaxInfo(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_TAXINFO*>(lpRecv);

	CSP_ANS_TAXINFO pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_TAX_INFO, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	pMsg.Result = gCastleDBSet.DSDB_QueryTaxInfo(lpMsg->MapSvrNum, &pMsg) ? 1 : 0;

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqTaxRateChange(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_TAXRATECHANGE*>(lpRecv);

	CSP_ANS_TAXRATECHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_TAX_RATE_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	int QueryResult = 0;

	if (!gCastleDBSet.DSDB_QueryTaxRateChange(lpMsg->MapSvrNum, lpMsg->TaxKind, lpMsg->TaxRate, &pMsg, &QueryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = QueryResult;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqCastleMoneyChange(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_MONEYCHANGE*>(lpRecv);

	CSP_ANS_MONEYCHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_MONEY_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	pMsg.MoneyChanged = lpMsg->MoneyChanged;

	int QueryResult = 0;
	__int64 MoneyResult = 0;

	if (!gCastleDBSet.DSDB_QueryCastleMoneyChange(lpMsg->MapSvrNum, lpMsg->MoneyChanged, &MoneyResult, &QueryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = QueryResult;
		pMsg.CastleMoney = MoneyResult;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqSiegeDateChange(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_SDEDCHANGE*>(lpRecv);

	CSP_ANS_SDEDCHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_SIEGE_DATE_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	int QueryResult = 0;

	if (!gCastleDBSet.DSDB_QuerySiegeDateChange(lpMsg->MapSvrNum, lpMsg, &QueryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = QueryResult;
		pMsg.StartYear = lpMsg->StartYear;
		pMsg.StartMonth = lpMsg->StartMonth;
		pMsg.StartDay = lpMsg->StartDay;
		pMsg.EndYear = lpMsg->EndYear;
		pMsg.EndMonth = lpMsg->EndMonth;
		pMsg.EndDay = lpMsg->EndDay;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqGuildMarkRegInfo(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_GUILDREGINFO*>(lpRecv);

	CSP_ANS_GUILDREGINFO pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_GUILD_MARK_INFO, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;

	int QueryResult = 0;

	char GuildName[9]{};
	memcpy(GuildName, lpMsg->GuildName, sizeof(lpMsg->GuildName));

	if (!gCastleDBSet.DSDB_QueryGuildMarkRegInfo(lpMsg->MapSvrNum, GuildName, &pMsg, &QueryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = QueryResult;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqSiegeEndedChange(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_SIEGEENDCHANGE*>(lpRecv);

	CSP_ANS_SIEGEENDCHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_SIEGE_ENDED_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	int QueryResult = 0;

	if (!gCastleDBSet.DSDB_QuerySiegeEndedChange(lpMsg->MapSvrNum, lpMsg->IsSiegeEnded, &QueryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = QueryResult;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqCastleOwnerChange(BYTE* lpRecv, int aIndex)
{
	auto* lpMsg = reinterpret_cast<CSP_REQ_CASTLEOWNERCHANGE*>(lpRecv);

	CSP_ANS_CASTLEOWNERCHANGE pMsg{};

	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_OWNER_CHANGE, sizeof(pMsg));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;

	int QueryResult = 0;

	if (!gCastleDBSet.DSDB_QueryCastleOwnerChange(lpMsg->MapSvrNum, lpMsg, &pMsg, &QueryResult))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = QueryResult;
	}

	gSocketManager.DataSend(aIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void DS_GDReqRegAttackGuild(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_REGATTACKGUILD* lpMsg = (CSP_REQ_REGATTACKGUILD*)lpRecv;
	CSP_ANS_REGATTACKGUILD pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_REG_ATTACK_GUILD, sizeof(CSP_ANS_REGATTACKGUILD));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
    
	int iQueryResult = 0;
    
	if (!gCastleDBSet.DSDB_QueryRegAttackGuild(lpMsg->MapSvrNum, lpMsg, &pMsg, &iQueryResult))
	{
		pMsg.Result = 0;
	}
    else
	{
		pMsg.Result = iQueryResult;
	}
	
	gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_REGATTACKGUILD));		
}

void DS_GDReqRestartCastleState(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_CASTLESIEGEEND* lpMsg = (CSP_REQ_CASTLESIEGEEND*)lpRecv;
	CSP_ANS_CASTLESIEGEEND pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_RESTART_STATE, sizeof(CSP_ANS_CASTLESIEGEEND));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
    
	int iQueryResult = 0;

	if (!gCastleDBSet.DSDB_QueryRestartCastleState(lpMsg->MapSvrNum, lpMsg, &iQueryResult))
	{
		pMsg.Result = 0;
	}
    else
	{
		pMsg.Result = iQueryResult;	
	}

	gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_CASTLESIEGEEND));	
}

void DS_GDReqMapSvrMsgMultiCast(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_MAPSVRMULTICAST* lpMsg = (CSP_REQ_MAPSVRMULTICAST*)lpRecv;
	CSP_ANS_MAPSVRMULTICAST pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_MAPSVR_MULTICAST, sizeof(CSP_ANS_MAPSVRMULTICAST));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
    
	memcpy(pMsg.MsgText, lpMsg->MsgText, sizeof(lpMsg->MsgText));
    
	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, (BYTE*)&pMsg, sizeof(CSP_ANS_MAPSVRMULTICAST));
		}
    }
}

void DS_GDReqRegGuildMark(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_GUILDREGMARK* lpMsg = (CSP_REQ_GUILDREGMARK*)lpRecv;
	CSP_ANS_GUILDREGMARK pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_REG_GUILD_MARK, sizeof(CSP_ANS_GUILDREGMARK));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
    pMsg.Index = lpMsg->Index;
	pMsg.ItemPos = lpMsg->ItemPos;
	
	char szGuildName[9] = {'\0'};
	memcpy(szGuildName, lpMsg->GuildName, 8);
	
	int iQueryResult = 0;
    
	if (!gCastleDBSet.DSDB_QueryGuildMarkRegMark(lpMsg->MapSvrNum, szGuildName, &pMsg, &iQueryResult))
	{
		pMsg.Result = 0;
	}
    else
	{
		pMsg.Result = iQueryResult;	
	}

	gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_GUILDREGMARK));	
}

void DS_GDReqGuildMarkReset(BYTE *lpRecv, int aIndex)
{

    CSP_REQ_GUILDRESETMARK* lpMsg = (CSP_REQ_GUILDRESETMARK*)lpRecv;
	CSP_ANS_GUILDRESETMARK pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_GUILD_MARK_RESET, sizeof(CSP_ANS_GUILDRESETMARK));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
	
	char szGuildName[9] = {'\0'};
	memcpy(szGuildName, lpMsg->GuildName, 8);

	if (!gCastleDBSet.DSDB_QueryGuildMarkReset(lpMsg->MapSvrNum, szGuildName, &pMsg))
	{
		pMsg.Result = 0;
	}

	gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_GUILDRESETMARK));		
}

void DS_GDReqGuildSetGiveUp(BYTE *lpRecv, int aIndex)
{
	CSP_REQ_GUILDSETGIVEUP* lpMsg = (CSP_REQ_GUILDSETGIVEUP*)lpRecv;
	CSP_ANS_GUILDSETGIVEUP pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_GUILD_GIVEUP, sizeof(CSP_ANS_GUILDSETGIVEUP));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
	pMsg.Index = lpMsg->Index;
	
	char szGuildName[9] = {'\0'};
	memcpy(szGuildName, lpMsg->GuildName, 8);
    
	if (!gCastleDBSet.DSDB_QueryGuildSetGiveUp(lpMsg->MapSvrNum, szGuildName, lpMsg->IsGiveUp, &pMsg))
	{
		pMsg.Result = 0;
	}

	gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_GUILDSETGIVEUP));	
}

void DS_GDReqCastleNpcRemove(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_NPCREMOVE* lpMsg = (CSP_REQ_NPCREMOVE*)lpRecv;
	CSP_ANS_NPCREMOVE pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_NPC_REMOVE, sizeof(CSP_ANS_NPCREMOVE));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
    pMsg.NpcNumber = lpMsg->NpcNumber;
	pMsg.NpcIndex = lpMsg->NpcIndex;
	
	int iQueryResult = 0;
    
	if (!gCastleDBSet.DSDB_QueryCastleNpcRemove(lpMsg->MapSvrNum, lpMsg, &iQueryResult))
	{
		pMsg.Result = 0;
	}
    else
	{
		pMsg.Result = iQueryResult;	
	}

	gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_NPCREMOVE));		
}

void DS_GDReqCastleStateSync(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_CASTLESTATESYNC* lpMsg = (CSP_REQ_CASTLESTATESYNC*)lpRecv;
	CSP_ANS_CASTLESTATESYNC pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_STATE_SYNC, sizeof(CSP_ANS_CASTLESTATESYNC));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
    pMsg.CastleState = lpMsg->CastleState;
    pMsg.TaxRateChaos = lpMsg->TaxRateChaos;
    pMsg.TaxRateStore = lpMsg->TaxRateStore;
    pMsg.TaxHuntZone = lpMsg->TaxHuntZone;
    memcpy(pMsg.OwnerGuildName, lpMsg->OwnerGuildName, 8);
    
	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, (BYTE*)&pMsg, sizeof(CSP_ANS_CASTLESTATESYNC));
		}
	}
}

void DS_GDReqCastleTributeMoney(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_CASTLETRIBUTEMONEY* lpMsg = (CSP_REQ_CASTLETRIBUTEMONEY*)lpRecv;
	CSP_ANS_CASTLETRIBUTEMONEY pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_TRIBUTE_MONEY, sizeof(CSP_ANS_CASTLETRIBUTEMONEY));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
    
	int iQueryResult = 0;
	__int64 i64MoneyResult = 0;
	
	if (lpMsg->CastleTributeMoney < 0)
	{
		pMsg.Result = 0;
	}
	else
	{
		if(!gCastleDBSet.DSDB_QueryCastleMoneyChange(lpMsg->MapSvrNum, lpMsg->CastleTributeMoney, &i64MoneyResult, &iQueryResult))
		{
			pMsg.Result = 0;
			gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_CASTLETRIBUTEMONEY));
			return;
		}

		pMsg.Result = iQueryResult;
	}

	gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_CASTLETRIBUTEMONEY));
}

void DS_GDReqResetCastleTaxInfo(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_RESETCASTLETAXINFO* lpMsg = (CSP_REQ_RESETCASTLETAXINFO*)lpRecv;
	CSP_ANS_RESETCASTLETAXINFO pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_RESET_TAX_INFO, sizeof(CSP_ANS_RESETCASTLETAXINFO));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
	
	int iQueryResult = 0;
    
	if (!gCastleDBSet.DSDB_QueryResetCastleTaxInfo(lpMsg->MapSvrNum, &iQueryResult))
	{
		pMsg.Result = 0;
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_RESETCASTLETAXINFO));
	}
    else
	{
		pMsg.Result = iQueryResult;
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_RESETCASTLETAXINFO));
	}
}

void DS_GDReqResetSiegeGuildInfo(BYTE *lpRecv, int aIndex)
{
    CSP_REQ_RESETSIEGEGUILDINFO* lpMsg = (CSP_REQ_RESETSIEGEGUILDINFO*)lpRecv;
	CSP_ANS_RESETSIEGEGUILDINFO pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_RESET_SIEGE_GUILD, sizeof(CSP_ANS_RESETSIEGEGUILDINFO));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
	
	int iQueryResult = 0;
    
	if (!gCastleDBSet.DSDB_QueryResetSiegeGuildInfo(lpMsg->MapSvrNum, &iQueryResult))
	{
		pMsg.Result = 0;
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_RESETSIEGEGUILDINFO));
	}
    else
	{
		pMsg.Result = iQueryResult;
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_RESETSIEGEGUILDINFO));
	}
}

void DS_GDReqResetRegSiegeInfo(BYTE *lpRecv, int aIndex)
{

    CSP_REQ_RESETSIEGEGUILDINFO* lpMsg = (CSP_REQ_RESETSIEGEGUILDINFO*)lpRecv;
	CSP_ANS_RESETSIEGEGUILDINFO pMsg {};
    
	pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_RESET_REG_INFO, sizeof(CSP_ANS_RESETSIEGEGUILDINFO));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;
	
	int iQueryResult = 0;
    
	if (!gCastleDBSet.DSDB_QueryResetRegSiegeInfo(lpMsg->MapSvrNum, &iQueryResult))
	{
		pMsg.Result = 0;
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_RESETSIEGEGUILDINFO));
	}
    else
	{
		pMsg.Result = iQueryResult;
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_RESETSIEGEGUILDINFO));
	}
}

void DS_GDReqCastleInitData(BYTE* lpRecv, int aIndex)
{
	CSP_REQ_CSINITDATA* lpMsg = (CSP_REQ_CSINITDATA*)lpRecv;
	CASTLE_DATA pCastleData;

	char cBUFFER[5920];

	CSP_ANS_CSINITDATA* lpMsgSend = (CSP_ANS_CSINITDATA*)cBUFFER;
	CSP_NPCDATA* lpMsgSendBody = (CSP_NPCDATA*)&cBUFFER[64];

	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;
	lpMsgSend->Count = 0;

	int iDataCount = 200;

	if (!gCastleDBSet.DB_QueryCastleTotalInfo(lpMsg->MapSvrNum, lpMsg->CastleEventCycle, &pCastleData))
	{
		lpMsgSend->Result = 0;
		lpMsgSend->Header.set(0x81, (sizeof(CSP_NPCDATA) * lpMsgSend->Count) + sizeof(CSP_ANS_CSINITDATA));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_NPCDATA) * lpMsgSend->Count) + sizeof(CSP_ANS_CSINITDATA));
		return;
	}

	lpMsgSend->Result = 0;
	lpMsgSend->StartYear = pCastleData.StartYear;
	lpMsgSend->StartMonth = pCastleData.StartMonth;
	lpMsgSend->StartDay = pCastleData.StartDay;
	lpMsgSend->EndYear = pCastleData.EndYear;
	lpMsgSend->EndMonth = pCastleData.EndMonth;
	lpMsgSend->EndDay = pCastleData.EndDay;
	lpMsgSend->IsSiegeGuildList = pCastleData.IsSiegeGuildList;
	lpMsgSend->IsSiegeEnded = pCastleData.IsSiegeEnded;
	lpMsgSend->IsCastleOccupied = pCastleData.IsCastleOccupied;
	lpMsgSend->CastleMoney = pCastleData.CastleMoney;
	lpMsgSend->TaxRateChaos = pCastleData.TaxRateChaos;
	lpMsgSend->TaxRateStore = pCastleData.TaxRateStore;
	lpMsgSend->TaxHuntZone = pCastleData.TaxHuntZone;
	lpMsgSend->FirstCreate = pCastleData.FirstCreate;

	memset(lpMsgSend->CastleOwnGuild, 0, sizeof(lpMsgSend->CastleOwnGuild));
	memcpy(lpMsgSend->CastleOwnGuild, pCastleData.CastleOwnGuild, sizeof(lpMsgSend->CastleOwnGuild));

	if (!gCastleDBSet.DSDB_QueryCastleNpcInfo(lpMsg->MapSvrNum, lpMsgSendBody, &iDataCount))
	{
		lpMsgSend->Result = 0;
	}
	else
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = iDataCount;
	}

	lpMsgSend->Header.set(0x81, (sizeof(CSP_NPCDATA) * lpMsgSend->Count) + sizeof(CSP_ANS_CSINITDATA));
	gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_NPCDATA) * lpMsgSend->Count) + sizeof(CSP_ANS_CSINITDATA));
}

void DS_GDReqCastleNpcInfo(BYTE *lpRecv, int aIndex)
{
	if ( lpRecv == nullptr)
	{
		return;
	}

	CSP_REQ_NPCDATA* lpMsg = (CSP_REQ_NPCDATA*)lpRecv;
	
	char cBUFFER[5876];
	
	CSP_ANS_NPCDATA* lpMsgSend = (CSP_ANS_NPCDATA*)cBUFFER;
	CSP_NPCDATA* lpMsgSendBody = (CSP_NPCDATA *)&cBUFFER[20];
	
	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;	
	
	int iDataCount = 200;
	
	lpMsgSend->Count = 0;
	
	if (!gCastleDBSet.DSDB_QueryCastleNpcInfo(lpMsg->MapSvrNum, lpMsgSendBody, &iDataCount))
	{
		lpMsgSend->Result = 0;
		lpMsgSend->Header.set(0x82, (sizeof(CSP_NPCDATA) * lpMsgSend->Count) + sizeof(CSP_ANS_NPCDATA));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_NPCDATA) * lpMsgSend->Count) + sizeof(CSP_ANS_NPCDATA));
	}
	else
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = iDataCount;
		lpMsgSend->Header.set(0x82, (sizeof(CSP_NPCDATA) * lpMsgSend->Count) + sizeof(CSP_ANS_NPCDATA));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_NPCDATA) * lpMsgSend->Count) + sizeof(CSP_ANS_NPCDATA));
	}
}

void DS_GDReqAllGuildMarkRegInfo(BYTE *lpRecv, int aIndex)
{
	if ( lpRecv == nullptr)
	{
		return;
	}

	CSP_REQ_ALLGUILDREGINFO* lpMsg = (CSP_REQ_ALLGUILDREGINFO*)lpRecv;
	
	char cBUFFER[1876];
	
	CSP_ANS_ALLGUILDREGINFO* lpMsgSend = (CSP_ANS_ALLGUILDREGINFO*)cBUFFER;
	CSP_GUILDREGINFO* lpMsgSendBody = (CSP_GUILDREGINFO *)&cBUFFER[20];
	
	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;	
	lpMsgSend->Index = lpMsg->Index;
	
	int iDataCount = 100;
	
	lpMsgSend->Count = 0;
	
	if (!gCastleDBSet.DSDB_QueryAllGuildMarkRegInfo(lpMsg->MapSvrNum, lpMsgSendBody, &iDataCount))
	{
		lpMsgSend->Result = 0;
		lpMsgSend->Header.set(0x83, (sizeof(CSP_GUILDREGINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_ALLGUILDREGINFO));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_GUILDREGINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_ALLGUILDREGINFO));
	}
	else
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = iDataCount;
		lpMsgSend->Header.set(0x83, (sizeof(CSP_GUILDREGINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_ALLGUILDREGINFO));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_GUILDREGINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_ALLGUILDREGINFO));
	}	
}

void DS_GDReqFirstCreateNPC(BYTE *lpRecv, int aIndex)
{
	if ( lpRecv == nullptr)
	{
		return;
	}

	CSP_REQ_NPCSAVEDATA* lpMsg = (CSP_REQ_NPCSAVEDATA*)lpRecv;
	CSP_NPCSAVEDATA* lpMsgBody = (CSP_NPCSAVEDATA*)&lpRecv[12];
	
	CSP_ANS_NPCSAVEDATA pMsg {};
	pMsg.Header.set(0x84, sizeof(CSP_ANS_NPCSAVEDATA));
	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	
	if (!gCastleDBSet.DSDB_QueryFirstCreateNPC(lpMsg->MapSvrNum, lpMsg))
	{
		pMsg.Result = 0;
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_NPCSAVEDATA));
	}
	else
	{
		pMsg.Result = 1;
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_NPCSAVEDATA));		
	}
}

void DS_GDReqCalcRegGuildList(BYTE *lpRecv, int aIndex)
{
	if ( lpRecv == nullptr)
	{
		return;
	}

	CSP_REQ_CALCREGGUILDLIST* lpMsg = (CSP_REQ_CALCREGGUILDLIST*)lpRecv;
	
	char cBUFFER[2672];
	
	CSP_ANS_CALCREGGUILDLIST* lpMsgSend = (CSP_ANS_CALCREGGUILDLIST*)cBUFFER;
	CSP_CALCREGGUILDLIST* lpMsgSendBody = (CSP_CALCREGGUILDLIST *)&cBUFFER[16];
	
	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;	
	
	int iDataCount = 100;
	
	lpMsgSend->Count = 0;
	
	if (!gCastleDBSet.DSDB_QueryCalcRegGuildList(lpMsg->MapSvrNum, lpMsgSendBody, &iDataCount))
	{
		lpMsgSend->Result = 0;
		lpMsgSend->Header.set(0x85, (sizeof(CSP_CALCREGGUILDLIST) * lpMsgSend->Count) + sizeof(CSP_ANS_CALCREGGUILDLIST));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_CALCREGGUILDLIST) * lpMsgSend->Count) + sizeof(CSP_ANS_CALCREGGUILDLIST));
	}
	else
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = iDataCount;
		lpMsgSend->Header.set(0x85, (sizeof(CSP_CALCREGGUILDLIST) * lpMsgSend->Count) + sizeof(CSP_ANS_CALCREGGUILDLIST));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_CALCREGGUILDLIST) * lpMsgSend->Count) + sizeof(CSP_ANS_CALCREGGUILDLIST));
	}		
}

void DS_GDReqCsGuildUnionInfo(BYTE *lpRecv, int aIndex)
{
	if ( lpRecv == nullptr)
	{
		return;
	}

	CSP_REQ_CSGUILDUNIONINFO* lpMsg = (CSP_REQ_CSGUILDUNIONINFO*)lpRecv;
	CSP_CSGUILDUNIONINFO* lpMsgBody = (CSP_CSGUILDUNIONINFO*)&lpRecv[12];
	
	char cBUFFER[1472];
	
	CSP_ANS_CSGUILDUNIONINFO* lpMsgSend = (CSP_ANS_CSGUILDUNIONINFO*)cBUFFER;
	CSP_CSGUILDUNIONINFO* lpMsgSendBody = (CSP_CSGUILDUNIONINFO *)&cBUFFER[16];
	
	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;	

	if (lpMsg->Count < 0)
	{
		lpMsg->Count = 0;
	}

	int iRET_COUNT = 0;
	
	for (int iGCNT=0; iGCNT < lpMsg->Count; iGCNT++)
	{
		char szGuildName[9] = {'\0'};
		memcpy(szGuildName, lpMsgBody[iGCNT].GuildName, 8);
		
		if (!gCastleDBSet.DSDB_QueryCsGuildUnionInfo(lpMsg->MapSvrNum, szGuildName, lpMsgBody[iGCNT].CsGuildID, lpMsgSendBody, &iRET_COUNT))
		{
			lpMsgSend->Result = 0;
			lpMsgSend->Count = 0;
			lpMsgSend->Header.set(0x86, (sizeof(CSP_CSGUILDUNIONINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_CSGUILDUNIONINFO));
			gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_CSGUILDUNIONINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_CSGUILDUNIONINFO));
			return;
		}
	}

	lpMsgSend->Result = 1;
	lpMsgSend->Count = iRET_COUNT;	
	lpMsgSend->Header.set(0x86, (sizeof(CSP_CSGUILDUNIONINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_CSGUILDUNIONINFO));
	gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_CSGUILDUNIONINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_CSGUILDUNIONINFO));
}

void DS_GDReqCsSaveTotalGuildInfo(BYTE *lpRecv, int aIndex)
{
	if ( lpRecv == nullptr)
	{
		return;
	}

	CSP_REQ_CSSAVETOTALGUILDINFO* lpMsg = (CSP_REQ_CSSAVETOTALGUILDINFO*)lpRecv;
	CSP_CSSAVETOTALGUILDINFO* lpMsgBody = (CSP_CSSAVETOTALGUILDINFO*)&lpRecv[12];
	
	CSP_ANS_CSSAVETOTALGUILDINFO pMsg;
	
	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	
	if (!gCastleDBSet.DSDB_QueryCsClearTotalGuildInfo(lpMsg->MapSvrNum))
	{
		pMsg.Result = 0;
		pMsg.Header.set(0x87, sizeof(CSP_ANS_CSSAVETOTALGUILDINFO));
		gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_CSSAVETOTALGUILDINFO));
	}
	else
	{
		for (int iGCNT = 0; iGCNT < lpMsg->Count; iGCNT++)
		{
			char szGuildName[9] = {'\0'};
			memcpy(szGuildName, lpMsgBody[iGCNT].GuildName, 8);

			if (!gCastleDBSet.DSDB_QueryCsSaveTotalGuildInfo(lpMsg->MapSvrNum, szGuildName, lpMsgBody[iGCNT].CsGuildID, lpMsgBody[iGCNT].GuildInvolved, lpMsgBody[iGCNT].GuildScore))
			{
				pMsg.Result = 0;
				pMsg.Header.set(0x87, sizeof(CSP_ANS_CSSAVETOTALGUILDINFO));
				gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_CSSAVETOTALGUILDINFO));
				return;
			}
		}

		int iQueryResult = 0;
		
		if (!gCastleDBSet.DSDB_QueryCsSaveTotalGuildOK(lpMsg->MapSvrNum, &iQueryResult))
		{
			pMsg.Result = 0;
			pMsg.Header.set(0x87, sizeof(CSP_ANS_CSSAVETOTALGUILDINFO));
			gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_CSSAVETOTALGUILDINFO));
		}
		else
		{
			pMsg.Result = 1;
			pMsg.Result = 1; //??
			pMsg.Header.set(0x87, sizeof(CSP_ANS_CSSAVETOTALGUILDINFO));
			gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_CSSAVETOTALGUILDINFO));
		}
	}
}

void DS_GDReqCsLoadTotalGuildInfo(BYTE *lpRecv, int aIndex)
{
	if ( lpRecv == nullptr)
	{
		return;
	}
	
	CSP_REQ_CSLOADTOTALGUILDINFO* lpMsg = (CSP_REQ_CSLOADTOTALGUILDINFO*)lpRecv;
	
	char cBUFFER[2272];
	
	CSP_ANS_CSLOADTOTALGUILDINFO* lpMsgSend = (CSP_ANS_CSLOADTOTALGUILDINFO*)cBUFFER;
	CSP_CSLOADTOTALGUILDINFO* lpMsgSendBody = (CSP_CSLOADTOTALGUILDINFO *)&cBUFFER[16];
	
	lpMsgSend->MapSvrNum = lpMsg->MapSvrNum;

	int iDataCount = 100;
	
	lpMsgSend->Count = 0;
	
	if (!gCastleDBSet.DSDB_QueryCsLoadTotalGuildInfo(lpMsg->MapSvrNum, lpMsgSendBody, &iDataCount))
	{
		lpMsgSend->Result = 0;
		lpMsgSend->Header.set(0x88, (sizeof(CSP_CSLOADTOTALGUILDINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_CSLOADTOTALGUILDINFO));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_CSLOADTOTALGUILDINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_CSLOADTOTALGUILDINFO));
	}
	else
	{
		lpMsgSend->Result = 1;
		lpMsgSend->Count = iDataCount;
		lpMsgSend->Header.set(0x88, (sizeof(CSP_CSLOADTOTALGUILDINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_CSLOADTOTALGUILDINFO));
		gSocketManager.DataSend(aIndex, (BYTE*)lpMsgSend, (sizeof(CSP_CSLOADTOTALGUILDINFO) * lpMsgSend->Count) + sizeof(CSP_ANS_CSLOADTOTALGUILDINFO));
	}	
}

void DS_GDReqCastleNpcUpdate(BYTE *lpRecv, int aIndex)
{
	if ( lpRecv == nullptr)
	{
		return;
	}

	CSP_REQ_NPCUPDATEDATA* lpMsg = (CSP_REQ_NPCUPDATEDATA*)lpRecv;
	CSP_NPCUPDATEDATA* lpMsgBody = (CSP_NPCUPDATEDATA*)&lpRecv[12];
	
	CSP_ANS_NPCSAVEDATA pMsg {};
	pMsg.Header.set(0x89,sizeof(CSP_ANS_NPCSAVEDATA));

	pMsg.MapSvrNum = lpMsg->MapSvrNum;
	
	if (!gCastleDBSet.DSDB_QueryCastleNpcUpdate(lpMsg->MapSvrNum, lpMsg))
	{
		pMsg.Result = 0;
	}
	else
	{
		pMsg.Result = 1;
	}

	gSocketManager.DataSend(aIndex, (BYTE*)&pMsg, sizeof(CSP_ANS_NPCSAVEDATA));
}

void GDMarryInfoSaveRecv(SDHP_MARRY_INFO_SAVE_RECV* lpMsg) // OK
{
    if(strcmp(lpMsg->mode,"marry") == 0)
    {
        gQueryManager.ExecQuery("EXEC WZ_SetMarryInfo '%s','%s'",lpMsg->CharacterName,lpMsg->marryname);
        gQueryManager.Fetch();
        gQueryManager.Close();
    }
    else if(strcmp(lpMsg->mode,"divorce") == 0)
    {
        gQueryManager.ExecQuery("EXEC WZ_SetDivorceInfo '%s','%s'",lpMsg->CharacterName,lpMsg->marryname);
        gQueryManager.Fetch();
        gQueryManager.Close();
    }
}

#if(HIDE_VT)
void GDCustomQuestSaveRecv(SDHP_CUSTOMQUEST_SAVE_RECV* lpMsg) // OK
{
	if (!gQueryManager.ExecQuery("SELECT * FROM CustomQuest WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO CustomQuest (Name,quest) VALUES ('%s',%d)", lpMsg->CharacterName, lpMsg->quest);
		gQueryManager.Close();

	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("Update CustomQuest SET quest = %d WHERE Name = '%s'", lpMsg->quest, lpMsg->CharacterName);
		gQueryManager.Close();
	}

}

void GDCustomQuestRecv(SDHP_CUSTOMQUEST_RECV* lpMsg, int index) // OK
{

	SDHP_CUSTOMQUEST_SEND pMsg;

	pMsg.Header.set(0xF1, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (!gQueryManager.ExecQuery("SELECT * FROM CustomQuest WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.quest = 0;
	}
	else
	{
		pMsg.quest = gQueryManager.GetAsInteger("quest");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

}
#endif
void GDSetCoinRecv(SDHP_SETCOIN_RECV* lpMsg) // OK
{
	gQueryManager.ExecQuery("EXEC WZ_SetCoin '%s','%s','%d','%d','%d'",lpMsg->Account,lpMsg->CharacterName,lpMsg->value1,lpMsg->value2,lpMsg->value3);
	gQueryManager.Fetch();
	gQueryManager.Close();
}

#if(RANKINGGOC == 1)
void GDCustomRankingRecv(SDHP_CUSTOM_RANKING_RECV* lpMsg, int index) // OK
{
	BYTE send[4096];

	PMSG_CUSTOM_RANKING_SEND pMsg;

	pMsg.Header.set(0xF4, 0);

	int size = sizeof(pMsg);

	pMsg.Index = lpMsg->Index;

	pMsg.type = lpMsg->type;

	pMsg.count = 0;

	CUSTOM_RANKING_DATA info;

	if (gQueryManager.ExecQuery("EXEC WZ_CustomRanking %d", lpMsg->type))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			gQueryManager.GetAsString("VALUE1", info.szName, sizeof(info.szName));
			info.valor = gQueryManager.GetAsInteger("VALUE2");

			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}

	}

	gQueryManager.Close();


	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	//LogAdd(LOG_BLACK,"valor %d",pMsg.Index);

	gSocketManager.DataSend(index, send, size);

}
#endif

#if(RANKING_NEW == 1)
void CharacterRanking(GDTop* lpMsg, int pIndex)
{
	int iCharCount = 0;
	DGCharTop rSend;
	ZeroMemory(&rSend, sizeof(rSend));
	rSend.h.set(0xD9, 0x02, sizeof(DGCharTop));

	if (!gQueryManager.ExecQuery("EXEC EX_RANKING1"))
	{
		gQueryManager.Close();
	}

	short i = gQueryManager.Fetch();

	while (i != SQL_NO_DATA && i != SQL_NULL_DATA)
	{
		int iReturnCode = gQueryManager.GetResult(1);

		if (iReturnCode < 0)
		{
			break;
		}

		char NickName[11] = { 0 };
		gQueryManager.GetAsString("Name", NickName, sizeof(NickName));
		strncpy_s(rSend.tp[iCharCount].CharacterName, sizeof(rSend.tp[iCharCount].CharacterName), NickName, _TRUNCATE);
		rSend.tp[iCharCount].Class = gQueryManager.GetAsInteger("Class");
		rSend.tp[iCharCount].level = gQueryManager.GetAsInteger("cLevel");
		rSend.tp[iCharCount].Reset = gQueryManager.GetAsInteger("ResetCount");
		rSend.tp[iCharCount].Relifes = gQueryManager.GetAsInteger("rResetLife");
		rSend.tp[iCharCount].Time = gQueryManager.GetAsInteger("Resets_Time");
		rSend.tp[iCharCount].Map = gQueryManager.GetAsInteger("MapNumber");
		iCharCount++;

		if (iCharCount >= 150)
		{
			break;
		}

		i = gQueryManager.Fetch();
	}

	gQueryManager.Close();

	for (int b = 0; b<iCharCount; b++)
	{
		char cName[11] = { 0 };
		strncpy_s(cName, sizeof(cName), rSend.tp[b].CharacterName, _TRUNCATE);

		if (!gQueryManager.ExecQuery("EXEC EX_RANKING2 '%s'", cName) || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
		}
		else
		{
			char GuildName[9] = { 0 };
			gQueryManager.GetAsString("G_Name", GuildName, sizeof(GuildName));
			strncpy_s(rSend.tp[b].Guild, sizeof(rSend.tp[b].Guild), GuildName, _TRUNCATE);
			gQueryManager.Close();
		}
	}
	gSocketManager.DataSend(pIndex, (BYTE*)&rSend, sizeof(DGCharTop));
}
#endif

void GetInfoCharTopBuffPhe(BUFFPHE_REQUESTDS* lpMsg, int index)
{
	INFOCHAR_BUFFPHE pMsg {};

	pMsg.Header.set(0xD9, 0x04, sizeof(pMsg));

	pMsg.Index = lpMsg->aIndex;

	memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = ((gUtil.CheckTextSyntax(lpMsg->CharacterName, sizeof(lpMsg->CharacterName)) == 0) ? 0 : 1);

	if (pMsg.Result == 0 || !gQueryManager.ExecQuery("SELECT * FROM Character WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 0;
	}
	else
	{
		pMsg.Level = (WORD)gQueryManager.GetAsInteger("cLevel");
		pMsg.Class = (BYTE)gQueryManager.GetAsInteger("Class");
		gQueryManager.GetAsBinary("Inventory", pMsg.Inventory[0], sizeof(pMsg.Inventory));
		gQueryManager.GetAsBinary("MagicList", pMsg.Skill[0], sizeof(pMsg.Skill));
		gQueryManager.GetAsBinary("EffectList", pMsg.Effect[0], sizeof(pMsg.Effect));
		pMsg.ChonPheHanhTau = (BYTE)gQueryManager.GetAsInteger("PheHanhTau");
		pMsg.PointUsePhe = (DWORD)gQueryManager.GetAsInteger("PointUsePhe");

		gQueryManager.Close();

		//========================
		if (!gQueryManager.ExecQuery("Select G_Name,G_Status from GuildMember Where Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));
			gQueryManager.Close();

		}
		else {

			gQueryManager.GetAsString("G_Name", pMsg.GuildName, sizeof(pMsg.GuildName));
			pMsg.GuildStatus = gQueryManager.GetAsInteger("G_Status");
			gQueryManager.Close();


			gQueryManager.ExecQuery("Select Number from Guild Where G_Name='%s'", pMsg.GuildName);
			gQueryManager.Fetch();
			pMsg.GuildNumber = (BYTE)gQueryManager.GetAsInteger("Number");
			gQueryManager.Close();

			//LogAdd(LOG_RED, "Send Guild Name %s", pMsg.GuildName);
		}
		//==========================
		//LogAdd(LOG_RED, "Send Data Name %s", lpMsg->Name);
	}

	gSocketManager.DataSend(index, (BYTE*)& pMsg, sizeof(pMsg));

}
void GetDBBuffPhe(BUFFPHE_REQUESTDS* lpMsg, int index)
{
	BUFFPHE_REQUESTDS_SETINFO pMsg {};

	pMsg.Header.set(0xD9, 0x03, sizeof(pMsg));

	pMsg.mDT_TongPoint = 0;
	pMsg.mDT_Top1Point = 0;
	memset(&pMsg.mDT_Top1Name, 0, sizeof(pMsg.mDT_Top1Name));
	//===
	pMsg.mBT_TongPoint = 0;
	pMsg.mBT_Top1Point = 0;
	memset(&pMsg.mBT_Top1Name, 0, sizeof(pMsg.mBT_Top1Name));

	if (!gQueryManager.ExecQuery("SELECT SUM(PointUsePhe) As TongPoint FROM Character WHERE PheHanhTau ='1'") || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
	}
	else
	{
		pMsg.mDT_TongPoint = gQueryManager.GetAsInteger("TongPoint");
		gQueryManager.Close();
	}

	if (!gQueryManager.ExecQuery("Select top 1 name as Name, PointUsePhe as UsePoint from Character where PheHanhTau ='1' order by PointUsePhe desc") || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
	}
	else
	{
		pMsg.mDT_Top1Point = gQueryManager.GetAsInteger("UsePoint");
		gQueryManager.GetAsString("Name", pMsg.mDT_Top1Name, sizeof(pMsg.mDT_Top1Name));
		gQueryManager.Close();
	}

	//===Bao Tieu
	if (!gQueryManager.ExecQuery("SELECT SUM(PointUsePhe) As TongPoint FROM Character WHERE PheHanhTau ='2'") || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
	}
	else
	{
		pMsg.mBT_TongPoint = gQueryManager.GetAsInteger("TongPoint");
		gQueryManager.Close();
	}
	if (!gQueryManager.ExecQuery("Select top 1 name as Name, PointUsePhe as UsePoint from Character where PheHanhTau ='2' order by PointUsePhe desc") || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
	}
	else
	{
		pMsg.mBT_Top1Point = gQueryManager.GetAsInteger("UsePoint");
		gQueryManager.GetAsString("Name", pMsg.mBT_Top1Name, sizeof(pMsg.mBT_Top1Name));
		gQueryManager.Close();
	}
	//LogAdd(LOG_RED, "GetDBBuffPhe");
	gSocketManager.DataSend(index, (BYTE*)& pMsg, sizeof(pMsg));

}
void GDCustomAttackResumeRecv(SDHP_CARESUME_RECV* lpMsg,int index)
{

	SDHP_CARESUME_SEND pMsg {};

	pMsg.Header.set(0xF5,sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.CharacterName,lpMsg->CharacterName,sizeof(pMsg.CharacterName));

	if(!gQueryManager.ExecQuery("SELECT * FROM CustomAttack WHERE Name='%s'",lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
	}
	else
	{
		pMsg.active = gQueryManager.GetAsInteger("Active");

		pMsg.skill = gQueryManager.GetAsInteger("Skill");

		pMsg.map = gQueryManager.GetAsInteger("Map");

		pMsg.posx = gQueryManager.GetAsInteger("PosX");

		pMsg.posy = gQueryManager.GetAsInteger("PosY");

		pMsg.autobuff = gQueryManager.GetAsInteger("AutoBuff");

		pMsg.offpvp = gQueryManager.GetAsInteger("OffPvP");

		pMsg.autoreset = gQueryManager.GetAsInteger("AutoReset");

		pMsg.autoaddstr = gQueryManager.GetAsInteger("AutoAddStr");

		pMsg.autoaddagi = gQueryManager.GetAsInteger("AutoAddAgi");

		pMsg.autoaddvit = gQueryManager.GetAsInteger("AutoAddVit");

		pMsg.autoaddene = gQueryManager.GetAsInteger("AutoAddEne");

		pMsg.autoaddcmd = gQueryManager.GetAsInteger("AutoAddCmd");

		gQueryManager.Close();

		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
	}
}

void GDCustomAttackSaveRecv(SDHP_CARESUME_SAVE_RECV* lpMsg)
{
	if(!gQueryManager.ExecQuery("SELECT * FROM CustomAttack WHERE Name='%s'",lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO CustomAttack (Name,Active,Skill,Map,PosX,PosY,AutoBuff,OffPvP,AutoReset,AutoAddStr,AutoAddAgi,AutoAddVit,AutoAddEne,AutoAddCmd) VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",lpMsg->CharacterName,lpMsg->active,lpMsg->skill,lpMsg->map,lpMsg->posx,lpMsg->posy,lpMsg->autobuff,lpMsg->offpvp,lpMsg->autoreset,lpMsg->autoaddstr,lpMsg->autoaddagi,lpMsg->autoaddvit,lpMsg->autoaddene,lpMsg->autoaddcmd);
		gQueryManager.Close();

	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("Update CustomAttack SET Active = %d, Skill = %d, Map = %d, PosX = %d, PosY = %d, AutoBuff = %d, OffPvP = %d, AutoReset = %d, AutoAddStr = %d, AutoAddAgi = %d, AutoAddVit = %d, AutoAddEne = %d, AutoAddCmd = %d  WHERE Name = '%s'",lpMsg->active,lpMsg->skill,lpMsg->map,lpMsg->posx,lpMsg->posy,lpMsg->autobuff,lpMsg->offpvp,lpMsg->autoreset,lpMsg->autoaddstr,lpMsg->autoaddagi,lpMsg->autoaddvit,lpMsg->autoaddene,lpMsg->autoaddcmd,lpMsg->CharacterName);
		gQueryManager.Close();
	}
}

void GDCustomNpcQuestSaveRecv(SDHP_CUSTOMNPCQUEST_SAVE_RECV* lpMsg) // OK
{
	if(!gQueryManager.ExecQuery("SELECT * FROM CustomNpcQuest WHERE Name='%s' and Quest = %d",lpMsg->CharacterName,lpMsg->quest) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO CustomNpcQuest (Name,quest,count,MonsterCount) VALUES ('%s',%d,1,99999)",lpMsg->CharacterName,lpMsg->quest);
		gQueryManager.Close();

	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("Update CustomNpcQuest SET Count = Count+1,MonsterCount=99999 WHERE Name = '%s' and Quest = %d",lpMsg->CharacterName,lpMsg->quest);
		gQueryManager.Close();
	}

}

void GDCustomNpcQuestRecv(SDHP_CUSTOMNPCQUEST_RECV* lpMsg,int index) // OK
{

	SDHP_CUSTOMNPCQUEST_SEND pMsg {};

	pMsg.Header.set(0xF7,0x00,sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	if(!gQueryManager.ExecQuery("SELECT * FROM CustomNpcQuest WHERE Name='%s' and Quest = %d",lpMsg->CharacterName,lpMsg->quest) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.quest = lpMsg->quest;

		pMsg.indexnpc = lpMsg->indexnpc;

		pMsg.questcount = 0;

		pMsg.monstercount = 99999;
	}
	else
	{
		pMsg.questcount = gQueryManager.GetAsInteger("Count");

		pMsg.monstercount = gQueryManager.GetAsInteger("MonsterCount");

		gQueryManager.Close();

		pMsg.quest = lpMsg->quest;

		pMsg.indexnpc = lpMsg->indexnpc;
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

}

void GDCustomNpcQuestMonsterCountSaveRecv(SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV* lpMsg) // OK
{
	if(!gQueryManager.ExecQuery("SELECT * FROM CustomNpcQuest WHERE Name='%s' and Quest = %d",lpMsg->CharacterName,lpMsg->quest) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO CustomNpcQuest (Name,quest,count,monstercount) VALUES ('%s',%d,0,%d)",lpMsg->CharacterName,lpMsg->quest,lpMsg->monsterqtd);
		gQueryManager.Close();

	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("Update CustomNpcQuest SET MonsterCount=%d WHERE Name = '%s' and Quest = %d",lpMsg->monsterqtd,lpMsg->CharacterName,lpMsg->quest);
		gQueryManager.Close();

	}

}


void GDCustomJewelBankRecv(SDHP_CUSTOM_JEWELBANK_RECV* lpMsg) // OK
{
	int Bless = 0;
	int Soul = 0;
	int Life = 0;
	int Creation = 0;
	int Guardian = 0;
	int GemStone = 0;
	int Harmony = 0;
	int Chaos = 0;
	int LowStone = 0;
	int HighStone = 0;

	switch (lpMsg->type)
	{
	case 0:
		Bless = lpMsg->count;
		break;
	case 1:
		Soul = lpMsg->count;
		break;
	case 2:
		Life = lpMsg->count;
		break;
	case 3:
		Creation = lpMsg->count;
		break;
	case 4:
		Guardian = lpMsg->count;
		break;
	case 5:
		GemStone = lpMsg->count;
		break;
	case 6:
		Harmony = lpMsg->count;
		break;
	case 7:
		Chaos = lpMsg->count;
		break;
	case 8:
		LowStone = lpMsg->count;
		break;
	case 9:
		HighStone = lpMsg->count;
		break;
	}

	if (!gQueryManager.ExecQuery("SELECT * FROM CustomJewelBank WHERE AccountID='%s'", lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO CustomJewelBank (AccountID,Bless,Soul,Life,Creation,Guardian,GemStone,Harmony,Chaos,LowStone,HighStone) VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)", lpMsg->Account, Bless, Soul, Life, Creation, Guardian, GemStone, Harmony, Chaos, LowStone, HighStone);
		gQueryManager.Close();

	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("Update CustomJewelBank SET Bless+=%d,Soul+=%d,Life+=%d,Creation+=%d,Guardian+=%d,GemStone+=%d,Harmony+=%d,Chaos+=%d,LowStone+=%d,HighStone+=%d WHERE AccountID = '%s'", Bless, Soul, Life, Creation, Guardian, GemStone, Harmony, Chaos, LowStone, HighStone, lpMsg->Account);
		gQueryManager.Close();
	}

}

void GDCustomJewelBankInfoRecv(SDHP_CUSTOM_JEWELBANK_INFO_RECV* lpMsg, int index)
{
	SDHP_CUSTOM_JEWELBANK_INFO_SEND pMsg = { 0 };

	pMsg.Header.set(0xF7, 0x05, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	if (!gQueryManager.ExecQuery("SELECT * FROM CustomJewelBank WHERE AccountID='%s'", lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Bless = 0;
		pMsg.Soul = 0;
		pMsg.Life = 0;
		pMsg.Creation = 0;
		pMsg.Guardian = 0;
		pMsg.GemStone = 0;
		pMsg.Harmony = 0;
		pMsg.Chaos = 0;
		pMsg.LowStone = 0;
		pMsg.HighStone = 0;
	}
	else
	{
		pMsg.Bless = gQueryManager.GetAsInteger("Bless");
		pMsg.Soul = gQueryManager.GetAsInteger("Soul");
		pMsg.Life = gQueryManager.GetAsInteger("Life");
		pMsg.Creation = gQueryManager.GetAsInteger("Creation");
		pMsg.Guardian = gQueryManager.GetAsInteger("Guardian");
		pMsg.GemStone = gQueryManager.GetAsInteger("GemStone");
		pMsg.Harmony = gQueryManager.GetAsInteger("Harmony");
		pMsg.Chaos = gQueryManager.GetAsInteger("Chaos");;
		pMsg.LowStone = gQueryManager.GetAsInteger("LowStone");
		pMsg.HighStone = gQueryManager.GetAsInteger("HighStone");

		gQueryManager.Close();
	}
	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
}


//========================================================================================================
#if (GHRS_TOP1_NEW)
void GDCustomGHRSRecv(SDHP_CUSTOM_GHRS_RECV* lpMsg, int index) // OK
{
	BYTE send[4096];

	PMSG_CUSTOM_GHRS_SEND pMsg {};

	pMsg.Header.set(0xF6, 0);

	int size = sizeof(pMsg);

	pMsg.time = lpMsg->time;

	pMsg.resets = 0;

	pMsg.Grand = 0;

	if (!gQueryManager.ExecQuery("select top 1 MasterResetCount, ResetCount from GHRS_top1 where time = %d order by MasterResetCount DESC, ResetCount DESC", lpMsg->time) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		if (gQueryManager.ExecQuery("select TOP 1 MasterResetCount, ResetCount from Character where CtlCode = 0 Order by MasterResetCount DESC, ResetCount DESC, cLevel DESC, Experience DESC, Money DESC") && gQueryManager.Fetch() != SQL_NO_DATA) {
			pMsg.resets = gQueryManager.GetAsInteger("ResetCount");
			pMsg.Grand = gQueryManager.GetAsInteger("MasterResetCount");
		}
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO GHRS_top1 (time, MasterResetCount, ResetCount) VALUES (%d, %d, %d)", lpMsg->time, pMsg.Grand, pMsg.resets);
		Log.ToDisp(LOG_GREEN, "Gioi han reset Updated!");

#if(CHIENTRUONGCO == 1)
		gQueryManager.ExecQuery("Update Character SET CTCTime = 900 , CTCRegDay = 0");
		Log.ToDisp(LOG_BLUE, "ChienTruongCo Update!");
		gQueryManager.Close();
#endif
#if(EVENT_END_LESS)
		//==CB Custom New
		gQueryManager.ExecQuery("UPDATE Character SET LuotDiEndLess='0'");
		gQueryManager.Close();
#endif
	}
	else {
		pMsg.resets = gQueryManager.GetAsInteger("ResetCount");
		pMsg.Grand = gQueryManager.GetAsInteger("MasterResetCount");
	}


	gQueryManager.Close();

	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);

}

#endif
//========================================================================================================
#if(BOT_STATUE == 1)

void GDBotInfoRecv(SDHP_BOT_INFO_GET* lpMsg, int index) // OK
{
	SDHP_BOT_INFO_SEND pMsg {};

	pMsg.Header.set(0xF9, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	char CharName[11] = { 0 };

	if (!gQueryManager.ExecQuery("exec WZ_GetTopStatue %d, %d", lpMsg->TypeTop, lpMsg->Rank) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 0;
	}
	else
	{
		gQueryManager.GetAsString("Name", CharName, sizeof(CharName));
		gQueryManager.Close();

	}
	memcpy(pMsg.CharacterName, CharName, sizeof(pMsg.CharacterName));

	pMsg.Result = ((gUtil.CheckTextSyntax(CharName, sizeof(CharName)) == 0) ? 0 : 1);

	if (pMsg.Result == 0 || !gQueryManager.ExecQuery("SELECT * FROM Character WHERE Name='%s'", CharName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 0;
	}
	else
	{
		pMsg.Level = (WORD)gQueryManager.GetAsInteger("cLevel");

		pMsg.Class = (BYTE)gQueryManager.GetAsInteger("Class");

		/*pMsg.LevelUpPoint = gQueryManager.GetAsInteger("LevelUpPoint");

		pMsg.Experience = gQueryManager.GetAsInteger("Experience");

		pMsg.Strength = gQueryManager.GetAsInteger("Strength");

		pMsg.Dexterity = gQueryManager.GetAsInteger("Dexterity");

		pMsg.Vitality = gQueryManager.GetAsInteger("Vitality");

		pMsg.Energy = gQueryManager.GetAsInteger("Energy");

		pMsg.Leadership = gQueryManager.GetAsInteger("Leadership");*/

		gQueryManager.GetAsBinary("Inventory", pMsg.Inventory[0], sizeof(pMsg.Inventory));

		gQueryManager.GetAsBinary("MagicList", pMsg.Skill[0], sizeof(pMsg.Skill));

		/*pMsg.Money = gQueryManager.GetAsInteger("Money");

		pMsg.Life = (DWORD)gQueryManager.GetAsFloat("Life");

		pMsg.MaxLife = (DWORD)gQueryManager.GetAsFloat("MaxLife");

		pMsg.Mana = (DWORD)gQueryManager.GetAsFloat("Mana");

		pMsg.MaxMana = (DWORD)gQueryManager.GetAsFloat("MaxMana");

		pMsg.BP = (DWORD)gQueryManager.GetAsFloat("BP");

		pMsg.MaxBP = (DWORD)gQueryManager.GetAsFloat("MaxBP");

		pMsg.Shield = (DWORD)gQueryManager.GetAsFloat("Shield");

		pMsg.MaxShield = (DWORD)gQueryManager.GetAsFloat("MaxShield");

		pMsg.Dir = (BYTE)gQueryManager.GetAsInteger("MapDir");

		pMsg.PKCount = gQueryManager.GetAsInteger("PkCount");

		pMsg.PKLevel = gQueryManager.GetAsInteger("PkLevel");

		pMsg.PKTime = gQueryManager.GetAsInteger("PkTime");

		pMsg.CtlCode = (BYTE)gQueryManager.GetAsInteger("CtlCode");*/

		gQueryManager.GetAsBinary("EffectList", pMsg.Effect[0], sizeof(pMsg.Effect));

		/*pMsg.FruitAddPoint = (WORD)gQueryManager.GetAsInteger("FruitAddPoint");

		pMsg.FruitSubPoint = (WORD)gQueryManager.GetAsInteger("FruitSubPoint");

		pMsg.Kills = (DWORD)gQueryManager.GetAsInteger("Kills");

		pMsg.Deads = (DWORD)gQueryManager.GetAsInteger("Deads");*/

		pMsg.rDanhHieu = (int)gQueryManager.GetAsInteger("rDanhHieu");

		gQueryManager.Close();

		//========================
		if (!gQueryManager.ExecQuery("Select G_Name,G_Status from GuildMember Where Name='%s'", CharName) || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));
			gQueryManager.Close();

		}
		else {

			gQueryManager.GetAsString("G_Name", pMsg.GuildName, sizeof(pMsg.GuildName));
			pMsg.GuildStatus = gQueryManager.GetAsInteger("G_Status");
			gQueryManager.Close();


			gQueryManager.ExecQuery("Select Number from Guild Where G_Name='%s'", pMsg.GuildName);
			gQueryManager.Fetch();
			pMsg.GuildNumber = (BYTE)gQueryManager.GetAsInteger("Number");
			gQueryManager.Close();

			Log.ToDisp(LOG_RED, "Send Guild Name %s", pMsg.GuildName);
		}
		//==========================
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
}
#endif

void GDGetSkinIsBuy(GSSENDDS_GETLISTISBUYSKIN* lpMsg, int index) // OK
{
	//LogAdd(LOG_RED, "GDGetSkinIsBuy %d %s", index, lpMsg->AccountID);

	BYTE send[4096];

	CBCUSTOM_LOAD_COUNT pMsg {};

	pMsg.Header.set(0xD9, 0x01, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.aIndex = lpMsg->aIndex;

	BCUSTOM_SKINMODEL_DATA info;

	if (gQueryManager.ExecQuery("Select SkinIndex,StatusBuy from SkinModel where AccountID='%s'", lpMsg->AccountID))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			info.SkinIndex = gQueryManager.GetAsInteger("SkinIndex");
			info.StatusBuy = gQueryManager.GetAsInteger("StatusBuy");
			//gQueryManager.GetAsString("VALUE4", info.valor4, sizeof(info.valor4));
			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		return;
	}

	pMsg.Header.size[0] = SET_NUMBERHB(size);
	pMsg.Header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);

}

void GDSaveSkinBuy(GSSENDDS_GETLISTISBUYSKIN* lpMsg, int index) // OK
{
	gQueryManager.ExecQuery("INSERT INTO SkinModel (AccountID,SkinIndex,StatusBuy) VALUES ('%s',%d,1)", lpMsg->AccountID, lpMsg->aIndex);
	gQueryManager.Close();

}

//========================================================================================================
#if (SACHTHUOCTINH_NEW)
void GDCharacterSachThuocTinhRecv(SACHTHUOCTINH_GD_REQ_DATA* lpMsg, int index) // OK
{
	SACHTHUOCTINH_DG_GET_DATA pMsg {};

	pMsg.Header.set(0xB0, 0x04, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	if (!gQueryManager.ExecQuery("SELECT * FROM CustomSachThuocTinh WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		if (!gQueryManager.ExecQuery("INSERT INTO CustomSachThuocTinh (Name,SACH_01,SACH_02,SACH_03,SACH_04,SACH_05,SACH_06,SACH_07) VALUES ('%s',0,0,0,0,0,0,0)", lpMsg->CharacterName))
		{
			gQueryManager.Close();
		}
		else
		{
			gQueryManager.Close();
			//-----
			pMsg.SACHTHUOCTINH_01 = 0;
			pMsg.SACHTHUOCTINH_02 = 0;
			pMsg.SACHTHUOCTINH_03 = 0;
			pMsg.SACHTHUOCTINH_04 = 0;
			pMsg.SACHTHUOCTINH_05 = 0;
			pMsg.SACHTHUOCTINH_06 = 0;
			pMsg.SACHTHUOCTINH_07 = 0;
			//-----
		}
	}
	else
	{
		//-----
		pMsg.SACHTHUOCTINH_01 = (BYTE)gQueryManager.GetAsInteger("SACH_01");
		pMsg.SACHTHUOCTINH_02 = (BYTE)gQueryManager.GetAsInteger("SACH_02");
		pMsg.SACHTHUOCTINH_03 = (BYTE)gQueryManager.GetAsInteger("SACH_03");
		pMsg.SACHTHUOCTINH_04 = (BYTE)gQueryManager.GetAsInteger("SACH_04");
		pMsg.SACHTHUOCTINH_05 = (BYTE)gQueryManager.GetAsInteger("SACH_05");
		pMsg.SACHTHUOCTINH_06 = (BYTE)gQueryManager.GetAsInteger("SACH_06");
		pMsg.SACHTHUOCTINH_07 = (BYTE)gQueryManager.GetAsInteger("SACH_07");
		//-----
		gQueryManager.Close();
	}
	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
}
void GDCharacterSachThuocTinhSaveRecv(SACHTHUOCTINH_GD_SAVE_DATA* lpMsg) // OK
{
	gQueryManager.ExecQuery("UPDATE CustomSachThuocTinh SET SACH_01 = %d, SACH_02 = %d, SACH_03 = %d, SACH_04 = %d, SACH_05 = %d, SACH_06 = %d, SACH_07 = %d WHERE Name = '%s'",
		lpMsg->SACHTHUOCTINH_01, lpMsg->SACHTHUOCTINH_02, lpMsg->SACHTHUOCTINH_03, lpMsg->SACHTHUOCTINH_04, lpMsg->SACHTHUOCTINH_05, lpMsg->SACHTHUOCTINH_06, lpMsg->SACHTHUOCTINH_07, lpMsg->Name);
	gQueryManager.Close();
}
#endif
//========================================================================================================
#if(MOCNAP == 1)
void GDCharacterMocNapRecv(MOCNAP_GD_REQ_DATA* lpMsg, int index) // OK
{
	MOCNAP_DG_GET_DATA pMsg {};

	pMsg.Header.set(0xB0, 0x09, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	if (!gQueryManager.ExecQuery("SELECT * FROM CustomMocNap WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		if (!gQueryManager.ExecQuery("INSERT INTO CustomMocNap (Name,MOCNAP1,MOCNAP2,MOCNAP3,MOCNAP4,MOCNAP5,MOCNAP6,MOCNAP7,MOCNAP8,MOCNAP9,MOCNAP10,MOCNAP11,MOCNAP12,MOCNAP13) VALUES ('%s',0,0,0,0,0,0,0,0,0,0,0,0,0)", lpMsg->CharacterName))
		{
			gQueryManager.Close();
		}
		else
		{
			gQueryManager.Close();

			pMsg.MOCNAP1 = 0;
			pMsg.MOCNAP2 = 0;
			pMsg.MOCNAP3 = 0;
			pMsg.MOCNAP4 = 0;
			pMsg.MOCNAP5 = 0;
			pMsg.MOCNAP6 = 0;
			pMsg.MOCNAP7 = 0;
			pMsg.MOCNAP8 = 0;
			pMsg.MOCNAP9 = 0;
			pMsg.MOCNAP10 = 0;
			pMsg.MOCNAP11 = 0;
			pMsg.MOCNAP12 = 0;
			pMsg.MOCNAP13 = 0;
		}
	}
	else
	{
		pMsg.MOCNAP1 = (BYTE)gQueryManager.GetAsInteger("MOCNAP1");
		pMsg.MOCNAP2 = (BYTE)gQueryManager.GetAsInteger("MOCNAP2");
		pMsg.MOCNAP3 = (BYTE)gQueryManager.GetAsInteger("MOCNAP3");
		pMsg.MOCNAP4 = (BYTE)gQueryManager.GetAsInteger("MOCNAP4");
		pMsg.MOCNAP5 = (BYTE)gQueryManager.GetAsInteger("MOCNAP5");
		pMsg.MOCNAP6 = (BYTE)gQueryManager.GetAsInteger("MOCNAP6");
		pMsg.MOCNAP7 = (BYTE)gQueryManager.GetAsInteger("MOCNAP7");
		pMsg.MOCNAP8 = (BYTE)gQueryManager.GetAsInteger("MOCNAP8");
		pMsg.MOCNAP9 = (BYTE)gQueryManager.GetAsInteger("MOCNAP9");
		pMsg.MOCNAP10 = (BYTE)gQueryManager.GetAsInteger("MOCNAP10");
		pMsg.MOCNAP11 = (BYTE)gQueryManager.GetAsInteger("MOCNAP11");
		pMsg.MOCNAP12 = (BYTE)gQueryManager.GetAsInteger("MOCNAP12");
		pMsg.MOCNAP13 = (BYTE)gQueryManager.GetAsInteger("MOCNAP13");

		gQueryManager.Close();
	}
	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
}
void GDCharacterMocNapSaveRecv(MOCNAP_GD_SAVE_DATA* lpMsg) // OK
{
	gQueryManager.ExecQuery("UPDATE CustomMocNap SET MOCNAP1 = %d, MOCNAP2 = %d, MOCNAP3 = %d, MOCNAP4 = %d, MOCNAP5 = %d, MOCNAP6 = %d, MOCNAP7 = %d, MOCNAP8 = %d, MOCNAP9 = %d, MOCNAP10 = %d, MOCNAP11 = %d, MOCNAP12 = %d, MOCNAP13 = %d WHERE Name = '%s'", lpMsg->MOCNAP1, lpMsg->MOCNAP2, lpMsg->MOCNAP3, lpMsg->MOCNAP4, lpMsg->MOCNAP5, lpMsg->MOCNAP6, lpMsg->MOCNAP7, lpMsg->MOCNAP8, lpMsg->MOCNAP9, lpMsg->MOCNAP10, lpMsg->MOCNAP11, lpMsg->MOCNAP12, lpMsg->MOCNAP13, lpMsg->Name);
	Log.ToDisp(LOG_BLUE, "SET MOCNAP1 = %d, MOCNAP2 = %d, MOCNAP3 = %d, MOCNAP4 = %d, MOCNAP5 = %d, MOCNAP6 = %d, MOCNAP7 = %d, MOCNAP8 = %d, MOCNAP9 = %d, MOCNAP10 = %d, MOCNAP11 = %d, MOCNAP12 = %d, MOCNAP13 = %d WHERE Name = '%s'", lpMsg->MOCNAP1, lpMsg->MOCNAP2, lpMsg->MOCNAP3, lpMsg->MOCNAP4, lpMsg->MOCNAP5, lpMsg->MOCNAP6, lpMsg->MOCNAP7, lpMsg->MOCNAP8, lpMsg->MOCNAP9, lpMsg->MOCNAP10, lpMsg->MOCNAP11, lpMsg->MOCNAP12, lpMsg->MOCNAP13, lpMsg->Name);
	gQueryManager.Close();
}
#endif

// Helper
inline BYTE GetProtocolSubHead(const BYTE* lpMsg)
{
	return (lpMsg[PACKET_TYPE_OFFSET] == PACKET_HEADER_C1)
		? lpMsg[C1_PACKET_DATA_OFFSET]
		: lpMsg[C2_PACKET_DATA_OFFSET];
}