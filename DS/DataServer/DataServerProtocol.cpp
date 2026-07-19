// DataServerProtocol.cpp
#include "DataServerProtocol.h"
#include "ServerManager.h"
#include "Log.h"
#include "ESProtocol.h"
#include "ChatServerProtocol.h"
#include "Helper.h"
#include "BadSyntax.h"
#include "CashShop.h"
#include "CastleSiege.h"
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
		case HEAD_GLOBAL_ITEM_COUNT:GDGlobalItemCountRecv(reinterpret_cast<const SDHP_ITEM_COUNT_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CHARACTER_LIST:GDCharacterListRecv(reinterpret_cast<const SDHP_CHARACTER_LIST_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CHARACTER_CREATE:GDCharacterCreateRecv(reinterpret_cast<const SDHP_CHARACTER_CREATE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CHARACTER_DELETE:GDCharacterDeleteRecv(reinterpret_cast<const SDHP_CHARACTER_DELETE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CHARACTER_INFO:GDCharacterInfoRecv(reinterpret_cast<const SDHP_CHARACTER_INFO_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_WAREHOUSE:
			switch (protocolSubHead)
			{
			case SUB_WAREHOUSE_PERSONAL_LOAD:gWarehouse.GDWarehouseItemRecv(reinterpret_cast<const SDHP_WAREHOUSE_ITEM_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_WAREHOUSE_PERSONAL_SAVE:gWarehouse.GDWarehouseItemSaveRecv(reinterpret_cast<const SDHP_WAREHOUSE_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_WAREHOUSE_GUILD_LOAD:gWarehouse.GDWarehouseGuildItemRecv(reinterpret_cast<const SDHP_WAREHOUSE_ITEM_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_WAREHOUSE_GUILD_SAVE:gWarehouse.GDWarehouseGuildItemSaveRecv(reinterpret_cast<const SDHP_WAREHOUSE_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_WAREHOUSE_GUILD_STATUS:gWarehouse.GDWarehouseGuildConsult(reinterpret_cast<const SDHP_WAREHOUSE_GUILD_STATUS_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_WAREHOUSE_GUILD_UPDATE:gWarehouse.GDWarehouseGuildUpdate(reinterpret_cast<const SDHP_WAREHOUSE_GUILD_STATUS_UPDATE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
		case HEAD_CREATE_ITEM:GDCreateItemRecv(reinterpret_cast<const SDHP_CREATE_ITEM_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_OPTION_DATA:GDOptionDataRecv(reinterpret_cast<const SDHP_OPTION_DATA_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_PET_ITEM:GDPetItemInfoRecv(reinterpret_cast<const SDHP_PET_ITEM_INFO_RECV*>(lpMsg), serverIndex, size);break;
#if(DATASERVER_UPDATE>=401)
		case HEAD_CHARACTER_NAME_CHECK:GDCharacterNameCheckRecv(reinterpret_cast<const SDHP_CHARACTER_NAME_CHECK_RECV*>(lpMsg), serverIndex, size);break;
#endif
#if(DATASERVER_UPDATE>=401)
		case HEAD_CHARACTER_RENAME:GDCharacterNameChangeRecv(reinterpret_cast<const SDHP_CHARACTER_NAME_CHANGE_RECV*>(lpMsg), serverIndex, size);break;
#endif
#if(ENABLECUSTOMQUEST)
		case HEAD_QUEST:
			switch (protocolSubHead)
			{
			case SUB_QUEST_KILLCOUNT_LOAD:gQuest.GDQuestKillCountRecv(reinterpret_cast<const SDHP_QUEST_KILL_COUNT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_QUEST_KILLCOUNT_SAVE:gQuest.GDQuestKillCountSaveRecv(reinterpret_cast<const SDHP_QUEST_KILL_COUNT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=401)
		case HEAD_MASTER_SKILL:
			switch (protocolSubHead)
			{
			case SUB_MASTER_SKILL_LOAD:gMasterSkillTree.GDMasterSkillTreeRecv(reinterpret_cast<const SDHP_MASTER_SKILL_TREE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_MASTER_SKILL_SAVE:gMasterSkillTree.GDMasterSkillTreeSaveRecv(reinterpret_cast<const SDHP_MASTER_SKILL_TREE_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
		case HEAD_NPC:
			switch (protocolSubHead)
			{
#if(DATASERVER_UPDATE>=202)
			case SUB_NPC_LEO_LOAD:gNpcTalk.GDNpcLeoTheHelperRecv(reinterpret_cast<const SDHP_NPC_LEO_THE_HELPER_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_NPC_LEO_SAVE:gNpcTalk.GDNpcLeoTheHelperSaveRecv(reinterpret_cast<const SDHP_NPC_LEO_THE_HELPER_SAVE_RECV*>(lpMsg), serverIndex, size);break;
#endif
#if(DATASERVER_UPDATE>=401)
			case SUB_NPC_SANTA_LOAD:gNpcTalk.GDNpcSantaClausRecv(reinterpret_cast<const SDHP_NPC_SANTA_CLAUS_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_NPC_SANTA_SAVE:gNpcTalk.GDNpcSantaClausSaveRecv(reinterpret_cast<const SDHP_NPC_SANTA_CLAUS_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
		case HEAD_COMMAND:
			switch (protocolSubHead)
			{
			case SUB_COMMAND_RESET:gCommandManager.GDCommandResetRecv(reinterpret_cast<const SDHP_COMMAND_RESET_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_MASTER_RESET:gCommandManager.GDCommandMasterResetRecv(reinterpret_cast<const SDHP_COMMAND_MASTER_RESET_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_MARRY:gCommandManager.GDCommandMarryRecv(reinterpret_cast<const SDHP_COMMAND_MARRY_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_REWARD:gCommandManager.GDCommandRewardRecv(reinterpret_cast<const SDHP_COMMAND_REWARD_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_REWARD_ALL:gCommandManager.GDCommandRewardAllRecv(reinterpret_cast<const SDHP_COMMAND_REWARDALL_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_RENAME:gCommandManager.GDCommandRenameRecv(reinterpret_cast<const SDHP_COMMAND_RENAME_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_BLOCK_ACCOUNT:gCommandManager.GDCommandBlocAccRecv(reinterpret_cast<const SDHP_COMMAND_BLOC_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_BLOCK_CHARACTER:gCommandManager.GDCommandBlocCharRecv(reinterpret_cast<const SDHP_COMMAND_BLOC_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_GIFT:gCommandManager.GDCommandGiftRecv(reinterpret_cast<const SDHP_GIFT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_COMMAND_TOP:gCommandManager.GDCommandTopRecv(reinterpret_cast<const SDHP_TOP_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#if(ENABLECUSTOMQUEST)
		case HEAD_QUEST_WORLD:
#if(DATASERVER_UPDATE>=501)
			switch (protocolSubHead)
			{
			case SUB_QUEST_WORLD_LOAD:gQuestWorld.GDQuestWorldRecv(reinterpret_cast<const SDHP_QUEST_WORLD_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_QUEST_WORLD_SAVE:gQuestWorld.GDQuestWorldSaveRecv(reinterpret_cast<const SDHP_QUEST_WORLD_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
#endif
			break;
#endif
#if(DATASERVER_UPDATE>=501)
		case HEAD_GENS_SYSTEM:
			switch (protocolSubHead)
			{
			case SUB_GENS_SYSTEM_INSERT:gGensSystem.GDGensSystemInsertRecv(reinterpret_cast<const SDHP_GENS_SYSTEM_INSERT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GENS_SYSTEM_DELETE:gGensSystem.GDGensSystemDeleteRecv(reinterpret_cast<const SDHP_GENS_SYSTEM_DELETE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GENS_SYSTEM_MEMBER:gGensSystem.GDGensSystemMemberRecv(reinterpret_cast<const SDHP_GENS_SYSTEM_MEMBER_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GENS_SYSTEM_UPDATE:gGensSystem.GDGensSystemUpdateRecv(reinterpret_cast<const SDHP_GENS_SYSTEM_UPDATE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GENS_SYSTEM_REWARD:gGensSystem.GDGensSystemRewardRecv(reinterpret_cast<const SDHP_GENS_SYSTEM_REWARD_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GENS_SYSTEM_REWARD_SAVE:gGensSystem.GDGensSystemRewardSaveRecv(reinterpret_cast<const SDHP_GENS_SYSTEM_REWARD_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=802)
		case HEAD_MURUMMY:
			switch (protocolSubHead)
			{
			case SUB_MURUMMY_LOAD:gMuRummy.GDReqCardInfo(reinterpret_cast<const PMSG_REQ_MURUMMY_SELECT_DS*>(lpMsg), serverIndex, size);break;
			case SUB_MURUMMY_INSERT:gMuRummy.GDReqCardInfoInsert(reinterpret_cast<const PMSG_REQ_MURUMMY_INSERT_DS*>(lpMsg), serverIndex, size);break;
			case SUB_MURUMMY_UPDATE:gMuRummy.GDReqCardInfoUpdate(reinterpret_cast<const PMSG_REQ_MURUMMY_UPDATE_DS*>(lpMsg), serverIndex, size);break;
			case SUB_MURUMMY_SCORE_UPDATE:gMuRummy.GDReqScoreUpdate(reinterpret_cast<const PMSG_REQ_MURUMMY_SCORE_UPDATE_DS*>(lpMsg), serverIndex, size);break;
			case SUB_MURUMMY_DELETE:gMuRummy.GDReqScoreDelete(reinterpret_cast<const PMSG_REQ_MURUMMY_DELETE_DS*>(lpMsg), serverIndex, size);break;
			case SUB_MURUMMY_SLOT_UPDATE:gMuRummy.GDReqSlotInfoUpdate(reinterpret_cast<const PMSG_REQ_MURUMMY_SLOTUPDATE_DS*>(lpMsg), serverIndex, size);break;
			case SUB_MURUMMY_INFO_UPDATE:gMuRummy.GDReqMuRummyInfoUpdate(reinterpret_cast<const PMSG_REQ_MURUMMY_INFO_UPDATE_DS*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=603)
		case HEAD_HELPER:
			switch (protocolSubHead)
			{
			case SUB_HELPER_LOAD:gHelper.GDHelperDataRecv(reinterpret_cast<const SDHP_HELPER_DATA_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_HELPER_SAVE:gHelper.GDHelperDataSaveRecv(reinterpret_cast<const SDHP_HELPER_DATA_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
		case HEAD_CASHSHOP:
			switch (protocolSubHead)
			{
			case SUB_CASHSHOP_POINT:gCashShop.GDCashShopPointRecv(reinterpret_cast<const SDHP_CASH_SHOP_POINT_RECV*>(lpMsg), serverIndex, size);break;
#if(DATASERVER_UPDATE>=501)
			case SUB_CASHSHOP_ITEM_BUY:gCashShop.GDCashShopItemBuyRecv(reinterpret_cast<const SDHP_CASH_SHOP_ITEM_BUY_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_ITEM_GIFT:gCashShop.GDCashShopItemGifRecv(reinterpret_cast<const SDHP_CASH_SHOP_ITEM_GIF_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_ITEM_LIST:gCashShop.GDCashShopItemNumRecv(reinterpret_cast<const SDHP_CASH_SHOP_ITEM_NUM_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_ITEM_USE:gCashShop.GDCashShopItemUseRecv(reinterpret_cast<const SDHP_CASH_SHOP_ITEM_USE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_PERIODIC_ITEM:gCashShop.GDCashShopPeriodicItemRecv(reinterpret_cast<const SDHP_CASH_SHOP_PERIODIC_ITEM_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_RECIEVE_POINT:gCashShop.GDCashShopRecievePointRecv(reinterpret_cast<const SDHP_CASH_SHOP_RECIEVE_POINT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_ADD_POINT_SAVE:gCashShop.GDCashShopAddPointSaveRecv(reinterpret_cast<const SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_SUB_POINT_SAVE:gCashShop.GDCashShopSubPointSaveRecv(reinterpret_cast<const SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_INSERT_ITEM_SAVE:gCashShop.GDCashShopInsertItemSaveRecv(reinterpret_cast<const SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_DELETE_ITEM_SAVE:gCashShop.GDCashShopDeleteItemSaveRecv(reinterpret_cast<const SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CASHSHOP_PERIODIC_ITEM_SAVE:gCashShop.GDCashShopPeriodicItemSaveRecv(reinterpret_cast<const SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);break;
#endif
			}
			break;
#if(DATASERVER_UPDATE<=402)
		case HEAD_PCPOINT:
			switch (protocolSubHead)
			{
			case SUB_PCPOINT_POINT:gPcPoint.GDPcPointPointRecv(reinterpret_cast<const SDHP_PC_POINT_POINT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PCPOINT_ITEM_BUY:gPcPoint.GDPcPointItemBuyRecv(reinterpret_cast<const SDHP_PC_POINT_ITEM_BUY_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PCPOINT_RECIEVE_POINT:gPcPoint.GDPcPointRecievePointRecv(reinterpret_cast<const SDHP_PC_POINT_RECIEVE_POINT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PCPOINT_ADD_POINT_SAVE:gPcPoint.GDPcPointAddPointSaveRecv(reinterpret_cast<const SDHP_PC_POINT_ADD_POINT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PCPOINT_SUB_POINT_SAVE:gPcPoint.GDPcPointSubPointSaveRecv(reinterpret_cast<const SDHP_PC_POINT_SUB_POINT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=402)
		case HEAD_LUCKY_COIN:
			switch (protocolSubHead)
			{
			case SUB_LUCKY_COIN_COUNT:gLuckyCoin.GDLuckyCoinCountRecv(reinterpret_cast<const SDHP_LUCKY_COIN_COUNT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_LUCKY_COIN_REGISTER:gLuckyCoin.GDLuckyCoinRegisterRecv(reinterpret_cast<const SDHP_LUCKY_COIN_REGISTER_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_LUCKY_COIN_EXCHANGE:gLuckyCoin.GDLuckyCoinExchangeRecv(reinterpret_cast<const SDHP_LUCKY_COIN_EXCHANGE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_LUCKY_COIN_ADD_COUNT_SAVE:gLuckyCoin.GDLuckyCoinAddCountSaveRecv(reinterpret_cast<const SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_LUCKY_COIN_SUB_COUNT_SAVE:gLuckyCoin.GDLuckyCoinSubCountSaveRecv(reinterpret_cast<const SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
		case HEAD_RESERVED_1B:
			switch (protocolSubHead)
			{
			case SUB_RESERVED_1B: break;
			}
			break;
		case HEAD_CRYWOLF_SYNC:	GDCrywolfSyncRecv(reinterpret_cast<const SDHP_CRYWOLF_SYNC_RECV*>(lpMsg), serverIndex, size);	break;
		case HEAD_CRYWOLF_INFO:	GDCrywolfInfoRecv(reinterpret_cast<const SDHP_CRYWOLF_INFO_RECV*>(lpMsg), serverIndex, size);	break;
		case HEAD_GLOBAL_POST:	GDGlobalPostRecv(reinterpret_cast<const SDHP_GLOBAL_POST_RECV*>(lpMsg), serverIndex, size);		break;
		case HEAD_GLOBAL_NOTICE:GDGlobalNoticeRecv(reinterpret_cast<const SDHP_GLOBAL_NOTICE_RECV*>(lpMsg), serverIndex, size);	break;
#if(DATASERVER_UPDATE>=602)
		case HEAD_LUCKY_ITEM:
			switch (protocolSubHead)
			{
			case SUB_LUCKY_ITEM_LOAD:gLuckyItem.GDLuckyItemRecv(reinterpret_cast<const SDHP_LUCKY_ITEM_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_LUCKY_ITEM_SAVE:gLuckyItem.GDLuckyItemSaveRecv(reinterpret_cast<const SDHP_LUCKY_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=701)
		case HEAD_PENTAGRAM_SYSTEM:
			switch (protocolSubHead)
			{
			case SUB_PENTAGRAM_JEWEL_INFO:gPentagramSystem.GDPentagramJewelInfoRecv(reinterpret_cast<const SDHP_PENTAGRAM_JEWEL_INFO_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PENTAGRAM_JEWEL_INFO_SAVE:gPentagramSystem.GDPentagramJewelInfoSaveRecv(reinterpret_cast<const SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PENTAGRAM_JEWEL_INSERT_SAVE:gPentagramSystem.GDPentagramJewelInsertSaveRecv(reinterpret_cast<const SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PENTAGRAM_JEWEL_DELETE_SAVE:gPentagramSystem.GDPentagramJewelDeleteSaveRecv(reinterpret_cast<const SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=801)
		case HEAD_SNS_DATA:GDSNSDataRecv(reinterpret_cast<const SDHP_SNS_DATA_RECV*>(lpMsg), serverIndex, size);break;
#endif
#if(DATASERVER_UPDATE>=802)
		case HEAD_PERSONAL_SHOP:
			switch (protocolSubHead)
			{
			case SUB_PSHOP_ITEM_VALUE:gPersonalShop.GDPShopItemValueRecv(reinterpret_cast<const SDHP_PSHOP_ITEM_VALUE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PSHOP_ITEM_VALUE_SAVE:gPersonalShop.GDPShopItemValueSaveRecv(reinterpret_cast<const SDHP_PSHOP_ITEM_VALUE_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PSHOP_ITEM_VALUE_INSERT:gPersonalShop.GDPShopItemValueInsertSaveRecv(reinterpret_cast<const SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PSHOP_ITEM_VALUE_DELETE:gPersonalShop.GDPShopItemValueDeleteSaveRecv(reinterpret_cast<const SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=802)
		case HEAD_EVENT_INVENTORY:
			switch (protocolSubHead)
			{
			case SUB_EVENT_INVENTORY:gEventInventory.GDEventInventoryRecv(reinterpret_cast<const SDHP_EVENT_INVENTORY_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_EVENT_INVENTORY_SAVE:gEventInventory.GDEventInventorySaveRecv(reinterpret_cast<const SDHP_EVENT_INVENTORY_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=803)
		case HEAD_MUUN_SYSTEM:
			switch (protocolSubHead)
			{
			case SUB_MUUN_INVENTORY:gMuunSystem.GDMuunInventoryRecv(reinterpret_cast<const SDHP_MUUN_INVENTORY_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_MUUN_INVENTORY_SAVE:gMuunSystem.GDMuunInventorySaveRecv(reinterpret_cast<const SDHP_MUUN_INVENTORY_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=801)
		case HEAD_GUILD_MATCHING:
			switch (protocolSubHead)
			{
			case SUB_GUILD_MATCHING_LIST:gGuildMatching.GDGuildMatchingListRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_LIST_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_LIST_SEARCH:gGuildMatching.GDGuildMatchingListSearchRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_LIST_SEARCH_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_INSERT:gGuildMatching.GDGuildMatchingInsertRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_INSERT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_CANCEL:gGuildMatching.GDGuildMatchingCancelRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_CANCEL_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_JOIN_INSERT:gGuildMatching.GDGuildMatchingJoinInsertRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_JOIN_INSERT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_JOIN_CANCEL:gGuildMatching.GDGuildMatchingJoinCancelRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_JOIN_CANCEL_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_JOIN_ACCEPT:gGuildMatching.GDGuildMatchingJoinAcceptRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_JOIN_ACCEPT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_JOIN_LIST:gGuildMatching.GDGuildMatchingJoinListRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_JOIN_LIST_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_JOIN_INFO:gGuildMatching.GDGuildMatchingJoinInfoRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_JOIN_INFO_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_GUILD_MATCHING_INSERT_SAVE:gGuildMatching.GDGuildMatchingInsertSaveRecv(reinterpret_cast<const SDHP_GUILD_MATCHING_INSERT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
#if(DATASERVER_UPDATE>=801)
		case HEAD_PARTY_MATCHING:
			switch (protocolSubHead)
			{
			case SUB_PARTY_MATCHING_INSERT:gPartyMatching.GDPartyMatchingInsertRecv(reinterpret_cast<const SDHP_PARTY_MATCHING_INSERT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PARTY_MATCHING_LIST:gPartyMatching.GDPartyMatchingListRecv(reinterpret_cast<const SDHP_PARTY_MATCHING_LIST_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PARTY_MATCHING_JOIN_INSERT:gPartyMatching.GDPartyMatchingJoinInsertRecv(reinterpret_cast<const SDHP_PARTY_MATCHING_JOIN_INSERT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PARTY_MATCHING_JOIN_INFO:gPartyMatching.GDPartyMatchingJoinInfoRecv(reinterpret_cast<const SDHP_PARTY_MATCHING_JOIN_INFO_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PARTY_MATCHING_JOIN_LIST:gPartyMatching.GDPartyMatchingJoinListRecv(reinterpret_cast<const SDHP_PARTY_MATCHING_JOIN_LIST_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PARTY_MATCHING_JOIN_ACCEPT:gPartyMatching.GDPartyMatchingJoinAcceptRecv(reinterpret_cast<const SDHP_PARTY_MATCHING_JOIN_ACCEPT_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PARTY_MATCHING_JOIN_CANCEL:gPartyMatching.GDPartyMatchingJoinCancelRecv(reinterpret_cast<const SDHP_PARTY_MATCHING_JOIN_CANCEL_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_PARTY_MATCHING_INSERT_SAVE:gPartyMatching.GDPartyMatchingInsertSaveRecv(reinterpret_cast<const SDHP_PARTY_MATCHING_INSERT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			}
			break;
#endif
		case HEAD_CHARACTER_INFO_SAVE:GDCharacterInfoSaveRecv(reinterpret_cast<const SDHP_CHARACTER_INFO_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_INVENTORY_ITEM_SAVE:GDInventoryItemSaveRecv(reinterpret_cast<const SDHP_INVENTORY_ITEM_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_OPTION_DATA_SAVE:GDOptionDataSaveRecv(reinterpret_cast<const SDHP_OPTION_DATA_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_PET_ITEM_INFO_SAVE:GDPetItemInfoSaveRecv(reinterpret_cast<const SDHP_PET_ITEM_INFO_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RESET_INFO_SAVE:GDResetInfoSaveRecv(reinterpret_cast<const SDHP_RESET_INFO_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_MASTER_RESET_INFO_SAVE:GDMasterResetInfoSaveRecv(reinterpret_cast<const SDHP_MASTER_RESET_INFO_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_DUEL_SAVE:GDRankingDuelSaveRecv(reinterpret_cast<const SDHP_RANKING_DUEL_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_BLOOD_CASTLE_SAVE:GDRankingBloodCastleSaveRecv(reinterpret_cast<const SDHP_RANKING_BLOOD_CASTLE_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_CHAOS_CASTLE_SAVE:GDRankingChaosCastleSaveRecv(reinterpret_cast<const SDHP_RANKING_CHAOS_CASTLE_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_DEVIL_SQUARE_SAVE:GDRankingDevilSquareSaveRecv(reinterpret_cast<const SDHP_RANKING_DEVIL_SQUARE_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_ILLUSION_TEMPLE_SAVE:GDRankingIllusionTempleSaveRecv(reinterpret_cast<const SDHP_RANKING_ILLUSION_TEMPLE_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CREATION_CARD_SAVE:GDCreationCardSaveRecv(reinterpret_cast<const SDHP_CREATION_CARD_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CRYWOLF_INFO_SAVE:GDCrywolfInfoSaveRecv(reinterpret_cast<const SDHP_CRYWOLF_INFO_SAVE_RECV*>(lpMsg), serverIndex, size);break;
#if(DATASERVER_UPDATE>=801)
		case HEAD_SNS_DATA_SAVE:GDSNSDataSaveRecv(reinterpret_cast<const SDHP_SNS_DATA_SAVE_RECV*>(lpMsg), serverIndex, size);break;
#endif
		case HEAD_CUSTOM_MONSTER_REWARD_SAVE:GDCustomMonsterRewardSaveRecv(reinterpret_cast<const SDHP_CUSTOM_MONSTER_REWARD_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_CUSTOM_ARENA_SAVE:GDRankingCustomArenaSaveRecv(reinterpret_cast<const SDHP_RANKING_CUSTOM_ARENA_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_TVT_EVENT_SAVE:GDRankingTvTEventSaveRecv(reinterpret_cast<const SDHP_RANKING_TVT_EVENT_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CONNECT_CHARACTER:GDConnectCharacterRecv(reinterpret_cast<const SDHP_CONNECT_CHARACTER_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_DISCONNECT_CHARACTER:GDDisconnectCharacterRecv(reinterpret_cast<const SDHP_DISCONNECT_CHARACTER_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_GLOBAL_WHISPER:GDGlobalWhisperRecv(reinterpret_cast<const SDHP_GLOBAL_WHISPER_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_KING_GUILD_SAVE:gReiDoMU.GDRankingKingGuildSaveRecv(reinterpret_cast<const SDHP_RANKING_KING_GUILD_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_RANKING_KING_PLAYER_SAVE:gReiDoMU.GDRankingKingPlayerSaveRecv(reinterpret_cast<const SDHP_RANKING_KING_PLAYER_SAVE_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_GLOBAL_ITEM_POST:GDGlobalItemPostRecv(reinterpret_cast<const SDHP_GLOBAL_ITEM_POST_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_SIEGE:
			switch(protocolSubHead)
			{
				case SUB_CASTLE_TOTAL_INFO:gCastleSiege.DS_GDReqCastleTotalInfo(reinterpret_cast<const CSP_REQ_CASTLEDATA*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_OWNER_GUILD_MASTER:gCastleSiege.DS_GDReqOwnerGuildMaster(reinterpret_cast<const CSP_REQ_OWNERGUILDMASTER*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_NPC_BUY:gCastleSiege.DS_GDReqCastleNpcBuy(reinterpret_cast<const CSP_REQ_NPCBUY*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_NPC_REPAIR:gCastleSiege.DS_GDReqCastleNpcRepair(reinterpret_cast<const CSP_REQ_NPCREPAIR*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_NPC_UPGRADE:gCastleSiege.DS_GDReqCastleNpcUpgrade(reinterpret_cast<const CSP_REQ_NPCUPGRADE*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_TAX_INFO:gCastleSiege.DS_GDReqTaxInfo(reinterpret_cast<const CSP_REQ_TAXINFO*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_TAX_RATE_CHANGE:gCastleSiege.DS_GDReqTaxRateChange(reinterpret_cast<const CSP_REQ_TAXRATECHANGE*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_MONEY_CHANGE:gCastleSiege.DS_GDReqCastleMoneyChange(reinterpret_cast<const CSP_REQ_MONEYCHANGE*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_SIEGE_DATE_CHANGE:gCastleSiege.DS_GDReqSiegeDateChange(reinterpret_cast<const CSP_REQ_SDEDCHANGE*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_GUILD_MARK_INFO:gCastleSiege.DS_GDReqGuildMarkRegInfo(reinterpret_cast<const CSP_REQ_GUILDREGINFO*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_SIEGE_ENDED_CHANGE:gCastleSiege.DS_GDReqSiegeEndedChange(reinterpret_cast<const CSP_REQ_SIEGEENDCHANGE*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_OWNER_CHANGE:gCastleSiege.DS_GDReqCastleOwnerChange(reinterpret_cast<const CSP_REQ_CASTLEOWNERCHANGE*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_REG_ATTACK_GUILD:gCastleSiege.DS_GDReqRegAttackGuild(reinterpret_cast<const CSP_REQ_REGATTACKGUILD*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_RESTART_STATE:gCastleSiege.DS_GDReqRestartCastleState(reinterpret_cast<const CSP_REQ_RESTARTCASTLESTATE*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_MAPSVR_MULTICAST:gCastleSiege.DS_GDReqMapSvrMsgMultiCast(reinterpret_cast<const CSP_REQ_MAPSVRMULTICAST*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_REG_GUILD_MARK:gCastleSiege.DS_GDReqRegGuildMark(reinterpret_cast<const CSP_REQ_GUILDREGMARK*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_GUILD_MARK_RESET:gCastleSiege.DS_GDReqGuildMarkReset(reinterpret_cast<const CSP_REQ_GUILDRESETMARK*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_GUILD_GIVEUP:gCastleSiege.DS_GDReqGuildSetGiveUp(reinterpret_cast<const CSP_REQ_GUILDSETGIVEUP*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_NPC_REMOVE:gCastleSiege.DS_GDReqCastleNpcRemove(reinterpret_cast<const CSP_REQ_NPCREMOVE*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_STATE_SYNC:gCastleSiege.DS_GDReqCastleStateSync(reinterpret_cast<const CSP_REQ_CASTLESTATESYNC*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_TRIBUTE_MONEY:gCastleSiege.DS_GDReqCastleTributeMoney(reinterpret_cast<const CSP_REQ_CASTLETRIBUTEMONEY*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_RESET_TAX_INFO:gCastleSiege.DS_GDReqResetCastleTaxInfo(reinterpret_cast<const CSP_REQ_RESETCASTLETAXINFO*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_RESET_SIEGE_GUILD:gCastleSiege.DS_GDReqResetSiegeGuildInfo(reinterpret_cast<const CSP_REQ_RESETSIEGEGUILDINFO*>(lpMsg), serverIndex, size);break;
				case SUB_CASTLE_RESET_REG_INFO:gCastleSiege.DS_GDReqResetRegSiegeInfo(reinterpret_cast<const CSP_REQ_RESETSIEGEGUILDINFO*>(lpMsg), serverIndex, size);break;
			}
			break;
		case HEAD_CASTLE_INIT_DATA:gCastleSiege.DS_GDReqCastleInitData(reinterpret_cast<const CSP_REQ_CSINITDATA*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_NPC_INFO:gCastleSiege.DS_GDReqCastleNpcInfo(reinterpret_cast<const CSP_REQ_NPCDATA*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_ALL_GUILD_MARK:gCastleSiege.DS_GDReqAllGuildMarkRegInfo(reinterpret_cast<const CSP_REQ_ALLGUILDREGINFO*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_FIRST_CREATE_NPC:gCastleSiege.DS_GDReqFirstCreateNPC(reinterpret_cast<const CSP_REQ_NPCSAVEDATA*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_CALC_REG_GUILD:gCastleSiege.DS_GDReqCalcRegGuildList(reinterpret_cast<const CSP_REQ_CALCREGGUILDLIST*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_GUILD_UNION_INFO:gCastleSiege.DS_GDReqCsGuildUnionInfo(reinterpret_cast<const CSP_REQ_CSGUILDUNIONINFO*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_SAVE_GUILD_INFO:gCastleSiege.DS_GDReqCsSaveTotalGuildInfo(reinterpret_cast<const CSP_REQ_CSSAVETOTALGUILDINFO*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_LOAD_GUILD_INFO:gCastleSiege.DS_GDReqCsLoadTotalGuildInfo(reinterpret_cast<const CSP_REQ_CSLOADTOTALGUILDINFO*>(lpMsg), serverIndex, size);break;
		case HEAD_CASTLE_NPC_UPDATE:gCastleSiege.DS_GDReqCastleNpcUpdate(reinterpret_cast<const CSP_REQ_NPCUPDATEDATA*>(lpMsg), serverIndex, size);break;
		case HEAD_CUSTOM_PROTOCOL:
			switch (protocolSubHead)
			{
			#if (CUSTOMELEMENTALBOOK)
			case SUB_CUSTOM_ELEMENTAL_BOOK:GDCharacterElementalBookRecv(reinterpret_cast<const ELEMENTALBOOK_GD_REQ_DATA*>(lpMsg), serverIndex, size);break;
			case SUB_CUSTOM_ELEMENTAL_BOOK_SAVE:GDCharacterElementalBookSaveRecv(reinterpret_cast<const ELEMENTALBOOK_GD_SAVE_DATA*>(lpMsg), serverIndex, size);break;
			#endif
			#if (RECHARGEREWARD == 1)
			case SUB_MOCNAP_LOAD:GDCharacterMocNapRecv(reinterpret_cast<const MOCNAP_GD_REQ_DATA*>(lpMsg), serverIndex, size);break;
			case SUB_MOCNAP_SAVE:GDCharacterMocNapSaveRecv(reinterpret_cast<const MOCNAP_GD_SAVE_DATA*>(lpMsg), serverIndex, size);break;
			#endif	
			}
			break;
		case HEAD_EXDB_PROTOCOL:ESDataRecv(serverIndex, protocolHead, lpMsg, size);break;
		case HEAD_CONNECT_SERVER:CSDataRecv(serverIndex, protocolHead, lpMsg, size);break;
        case HEAD_MARRY_INFO_SAVE:GDMarryInfoSaveRecv(reinterpret_cast<const SDHP_MARRY_INFO_SAVE_RECV*>(lpMsg), serverIndex, size);break;
#if(ENABLECUSTOMQUEST)
		case HEAD_CUSTOM_QUEST:GDCustomQuestRecv(reinterpret_cast<const SDHP_CUSTOMQUEST_RECV*>(lpMsg), serverIndex, size);break;
		case HEAD_CUSTOM_QUEST_SAVE:GDCustomQuestSaveRecv(reinterpret_cast<const SDHP_CUSTOMQUEST_SAVE_RECV*>(lpMsg), serverIndex, size);break;
#endif
        case HEAD_SET_COIN:GDSetCoinRecv(reinterpret_cast<const SDHP_SETCOIN_RECV*>(lpMsg), serverIndex, size);break;
#if(ENABLECUSTOMRANKING)
		case HEAD_CUSTOM_RANKING:GDCustomRankingRecv(reinterpret_cast<const SDHP_CUSTOM_RANKING_RECV*>(lpMsg), serverIndex, size);break;
#endif
        case HEAD_ATTACK_RESUME:GDCustomAttackResumeRecv(reinterpret_cast<const SDHP_CARESUME_RECV*>(lpMsg), serverIndex, size);break;
        case HEAD_ATTACK_RESUME_SAVE:GDCustomAttackSaveRecv(reinterpret_cast<const SDHP_CARESUME_SAVE_RECV*>(lpMsg), serverIndex, size);break;
        case HEAD_CUSTOM_NPC_QUEST:
			switch(protocolSubHead)
			{
			case SUB_CUSTOM_NPC_QUEST_INFO:GDCustomNpcQuestRecv(reinterpret_cast<const SDHP_CUSTOMNPCQUEST_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CUSTOM_NPC_QUEST_SAVE:GDCustomNpcQuestSaveRecv(reinterpret_cast<const SDHP_CUSTOMNPCQUEST_SAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CUSTOM_NPC_QUEST_MONSTER_COUNT_SAVE:GDCustomNpcQuestMonsterCountSaveRecv(reinterpret_cast<const SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CUSTOM_JEWEL_BANK:GDCustomJewelBankRecv(reinterpret_cast<const SDHP_CUSTOM_JEWELBANK_RECV*>(lpMsg), serverIndex, size);break;
			case SUB_CUSTOM_JEWEL_BANK_INFO:GDCustomJewelBankInfoRecv(reinterpret_cast<const SDHP_CUSTOM_JEWELBANK_INFO_RECV*>(lpMsg), serverIndex, size);break;
			}
            break;
#if (TOP1RESETLIMIT)
		case HEAD_CUSTOM_GHRS:GDCustomGHRSRecv(reinterpret_cast<const SDHP_CUSTOM_GHRS_RECV*>(lpMsg), serverIndex, size);break;
#endif
#if (TOP1BOTSTATUE)
		case HEAD_BOT_INFO:GDBotInfoRecv(reinterpret_cast<const SDHP_BOT_INFO_GET*>(lpMsg), serverIndex, size);break;
#endif
		case HEAD_CUSTOM_EXTENSIONS:
			switch(protocolSubHead)
			{
			case SUB_SKIN_GET_LIST:GDGetSkinIsBuy(reinterpret_cast<const GSSENDDS_GETLISTISBUYSKIN*>(lpMsg), serverIndex, size);break;
				//==Save Skin Da MUa
			case SUB_SKIN_SAVE:GDSaveSkinBuy(reinterpret_cast<const GSSENDDS_GETLISTISBUYSKIN*>(lpMsg), serverIndex, size);break;
			case SUB_THE_GIFT_SAVE:GDSaveTheGiftRecv(reinterpret_cast<const THEGIFT_GD_SAVE_DATA*>(lpMsg), serverIndex, size);break;
			case SUB_CHANGE_PASSWORD_SAVE:GDChangePasswordRecv(reinterpret_cast<const SDHP_CHANGE_PASSWORD_RECV*>(lpMsg), serverIndex, size);break;
#if(NEWRANKING)
			case SUB_CHARACTER_RANKING:CharacterRanking(reinterpret_cast<const GDTop*>(lpMsg), serverIndex, size);break;
#endif
			case SUB_BUFFPHE_LOAD:GetDBBuffPhe(reinterpret_cast<const BUFFPHE_REQUESTDS*>(lpMsg), serverIndex, size);break;
			case SUB_BUFFPHE_TOP_INFO:GetInfoCharTopBuffPhe(reinterpret_cast<const BUFFPHE_REQUESTDS*>(lpMsg), serverIndex, size);break;
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

	pMsg.Slot = 0;
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

		if (!gCharacterManager.GetFreeCharacterSlot(characterName, &pMsg.Slot))
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
				pMsg.Slot + 1,
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

#if(ENDLESSTOWEREVENT)
		pMsg.LuotDiEndLess = static_cast<WORD>(gQueryManager.GetAsInteger("LuotDiEndLess"));
#endif

#if(RECHARGEREWARD == 1)
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

	pMsg.Type = lpMsg->Type;
	pMsg.Count = 0;

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
		++pMsg.Count;
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

void GDSaveTheGiftRecv(const THEGIFT_GD_SAVE_DATA* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(THEGIFT_GD_SAVE_DATA);

	gQueryManager.ExecQuery(
		"UPDATE Character SET TheGift=%d WHERE Name='%s'",
		lpMsg->TheGift,
		lpMsg->CharacterName);

	gQueryManager.Close();
}

void GDChangePasswordRecv(const SDHP_CHANGE_PASSWORD_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CHANGE_PASSWORD_RECV);

	SDHP_CHANGE_PASSWORD_SEND pMsg{};

	pMsg.Header.set(GS_HEAD_CUSTOM_EXTENSIONS, GS_SUB_CHANGE_PASSWORD_RESULT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	if (!gQueryManager.ExecQuery(
		"SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s' COLLATE Latin1_General_BIN",
		lpMsg->Account) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;

		gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

		return;
	}

	char password[11]{};

	gQueryManager.GetAsString("memb__pwd", password, sizeof(password));

	if (std::strcmp(lpMsg->OldPassword, password) != 0)
	{
		gQueryManager.Close();

		pMsg.Result = 2;

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	gQueryManager.Close();

	if (gQueryManager.ExecQuery(
		"UPDATE MEMB_INFO SET memb__pwd='%s' WHERE memb___id='%s'",
		lpMsg->NewPassword,
		lpMsg->Account))
	{
		// const SQLLEN affectedRows = gQueryManager.GetAffectedRows();
	}

	gQueryManager.Close();

	pMsg.Result = 3;

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

	Log.ToDisp(
		LOG_RED,
		"[GDChangePasswordRecv] Password changed successfully (Account: %s)",
		lpMsg->Account);
}

void GDCrywolfSyncRecv(const SDHP_CRYWOLF_SYNC_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CRYWOLF_SYNC_RECV);

	SDHP_CRYWOLF_SYNC_SEND pMsg{};

	pMsg.Header.set(HEAD_CRYWOLF_SYNC, sizeof(pMsg));

	pMsg.MapServerGroup = lpMsg->MapServerGroup;
	pMsg.CrywolfState = lpMsg->CrywolfState;
	pMsg.OccupationState = lpMsg->OccupationState;

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void GDCrywolfInfoRecv(const SDHP_CRYWOLF_INFO_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CRYWOLF_INFO_RECV);

	SDHP_CRYWOLF_INFO_SEND pMsg{};

	pMsg.Header.set(HEAD_CRYWOLF_INFO, sizeof(pMsg));
	pMsg.MapServerGroup = lpMsg->MapServerGroup;

	if (gQueryManager.ExecQuery(
		"EXEC WZ_CW_InfoLoad %d",
		lpMsg->MapServerGroup))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.CrywolfState =
				static_cast<BYTE>(
					gQueryManager.GetAsInteger("CRYWOLF_STATE"));

			pMsg.OccupationState =
				static_cast<BYTE>(
					gQueryManager.GetAsInteger("CRYWOLF_OCCUFY"));
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDGlobalPostRecv(const SDHP_GLOBAL_POST_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_GLOBAL_POST_RECV);

	SDHP_GLOBAL_POST_SEND pMsg{};

	pMsg.Header.set(HEAD_GLOBAL_POST, sizeof(pMsg));
	pMsg.MapServerGroup = lpMsg->MapServerGroup;
	pMsg.Type = lpMsg->Type;

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	std::memcpy(
		pMsg.Message,
		lpMsg->Message,
		sizeof(pMsg.Message));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(
				n,
				reinterpret_cast<BYTE*>(&pMsg),
				sizeof(pMsg));
		}
	}
}

void GDGlobalItemPostRecv(const SDHP_GLOBAL_ITEM_POST_RECV* lpMsg, int serverIndex,	int size)
{
	VALIDATE_PACKET_SIZE(SDHP_GLOBAL_ITEM_POST_RECV);

	SDHP_GLOBAL_ITEM_POST_SEND pMsg{};

	pMsg.Header.set(HEAD_GLOBAL_ITEM_POST, sizeof(pMsg));
	pMsg.MapServerGroup = lpMsg->MapServerGroup;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.Message, lpMsg->Message, sizeof(pMsg.Message));
	std::memcpy(pMsg.Item_Data, lpMsg->Item_Data, sizeof(pMsg.Item_Data));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void GDSNSDataRecv(const SDHP_SNS_DATA_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_SNS_DATA_RECV);

	SDHP_SNS_DATA_SEND pMsg{};

	pMsg.Header.set(HEAD_SNS_DATA, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"SELECT Data FROM SNSData WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.Result = 0;
			gQueryManager.GetAsBinary("Data", pMsg.Data, sizeof(pMsg.Data));
		}
		else
		{
			pMsg.Result = 1;
			std::memset(pMsg.Data, 0xFF, sizeof(pMsg.Data));
		}
	}
	else
	{
		pMsg.Result = 1;

		std::memset(pMsg.Data, 0xFF, sizeof(pMsg.Data));
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void GDGlobalNoticeRecv(const SDHP_GLOBAL_NOTICE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_GLOBAL_NOTICE_RECV);

	SDHP_GLOBAL_NOTICE_SEND pMsg{};

	pMsg.Header.set(HEAD_GLOBAL_NOTICE, sizeof(pMsg));

	pMsg.MapServerGroup = lpMsg->MapServerGroup;
	pMsg.Type = lpMsg->Type;
	pMsg.Count = lpMsg->Count;
	pMsg.Opacity = lpMsg->Opacity;
	pMsg.Delay = lpMsg->Delay;
	pMsg.Color = lpMsg->Color;
	pMsg.Speed = lpMsg->Speed;

	std::memcpy(
		pMsg.Message,
		lpMsg->Message,
		sizeof(pMsg.Message));

	for (int n = 0; n < MAX_SERVER; ++n)
	{
		if (gServerManager[n].IsOnline() != 0)
		{
			gSocketManager.DataSend(n, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
		}
	}
}

void GDCharacterInfoSaveRecv(const SDHP_CHARACTER_INFO_SAVE_RECV* lpMsg, int serverIndex, int size)
{
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

	if (!gQueryManager.ExecQuery(
		"UPDATE Character SET "
		"UserSkinPick=%d, Kills=%d, Deads=%d"
#if (CHONPHEDOILAP)
		", PheHanhTau=%d"
#endif
#if (ENDLESSTOWEREVENT)
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
#if (RECHARGEREWARD == 1)
		", MOCNAPCOIN=%d"
#endif
#if (ANCIENTBATTLEFIELDEVENT == 1)
		", CTCTime=%d, CTCRegDay=%d"
#else
		", CTCTime=%d, CTCRegDay=0" // Bug original corregido: Agregado WHERE abajo, y CTCRegDay=0
#endif
#if (FLAG_SKIN)
		", Flag=%d"
#endif
		" WHERE AccountID='%s' AND Name='%s'",

		lpMsg->mUserSkinPick, lpMsg->Kills, lpMsg->Deads,
#if (CHONPHEDOILAP)
		lpMsg->ChonPheHanhTau,
#endif
#if (ENDLESSTOWEREVENT)
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
#if (RECHARGEREWARD == 1)
		lpMsg->MOCNAPCOIN,
#endif
#if (ANCIENTBATTLEFIELDEVENT == 1)
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

#if (DATASERVER_UPDATE >= 602)
	if (!gQueryManager.ExecQuery("UPDATE AccountCharacter SET ExtWarehouse=%d WHERE Id='%s'", lpMsg->ExtWarehouse, lpMsg->Account))
	{
		gQueryManager.Close();
		return;
	}
	gQueryManager.Close();
#endif
}

void GDInventoryItemSaveRecv(const SDHP_INVENTORY_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_INVENTORY_ITEM_SAVE_RECV);

	gQueryManager.BindParameterAsBinary(
		1,
		lpMsg->Inventory,
		sizeof(lpMsg->Inventory));

	gQueryManager.ExecQuery(
		"UPDATE Character SET Inventory=? WHERE AccountID='%s' AND Name='%s'",
		lpMsg->Account,
		lpMsg->CharacterName);

	gQueryManager.Close();
}

void GDOptionDataSaveRecv(const SDHP_OPTION_DATA_SAVE_RECV* lpMsg, int serverIndex,	int size)
{
	VALIDATE_PACKET_SIZE(SDHP_OPTION_DATA_SAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT 1 FROM OptionData WHERE Name='%s'",
			lpMsg->CharacterName) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	gQueryManager.BindParameterAsBinary(
		1,
		lpMsg->SkillKey,
		sizeof(lpMsg->SkillKey));

	if (exists)
	{
#if (DATASERVER_UPDATE >= 701)

		gQueryManager.ExecQuery(
			"UPDATE OptionData SET "
			"SkillKey=?,GameOption=%d,Qkey=%d,Wkey=%d,Ekey=%d,"
			"ChatWindow=%d,Rkey=%d,QWERLevel=%d,ChangeSkin=%d "
			"WHERE Name='%s'",
			lpMsg->GameOption,
			lpMsg->QKey,
			lpMsg->WKey,
			lpMsg->EKey,
			lpMsg->ChatWindow,
			lpMsg->RKey,
			lpMsg->QWERLevel,
			lpMsg->ChangeSkin,
			lpMsg->CharacterName);

#else

		gQueryManager.ExecQuery(
			"UPDATE OptionData SET "
			"SkillKey=?,GameOption=%d,Qkey=%d,Wkey=%d,Ekey=%d,"
			"ChatWindow=%d,Rkey=%d,QWERLevel=%d "
			"WHERE Name='%s'",
			lpMsg->GameOption,
			lpMsg->QKey,
			lpMsg->WKey,
			lpMsg->EKey,
			lpMsg->ChatWindow,
			lpMsg->RKey,
			lpMsg->QWERLevel,
			lpMsg->CharacterName);

#endif
	}
	else
	{
#if (DATASERVER_UPDATE >= 701)

		gQueryManager.ExecQuery(
			"INSERT INTO OptionData "
			"(Name,SkillKey,GameOption,Qkey,Wkey,Ekey,ChatWindow,Rkey,QWERLevel,ChangeSkin) "
			"VALUES ('%s',?,%d,%d,%d,%d,%d,%d,%d,%d)",
			lpMsg->CharacterName,
			lpMsg->GameOption,
			lpMsg->QKey,
			lpMsg->WKey,
			lpMsg->EKey,
			lpMsg->ChatWindow,
			lpMsg->RKey,
			lpMsg->QWERLevel,
			lpMsg->ChangeSkin);

#else

		gQueryManager.ExecQuery(
			"INSERT INTO OptionData "
			"(Name,SkillKey,GameOption,Qkey,Wkey,Ekey,ChatWindow,Rkey,QWERLevel) "
			"VALUES ('%s',?,%d,%d,%d,%d,%d,%d,%d)",
			lpMsg->CharacterName,
			lpMsg->GameOption,
			lpMsg->QKey,
			lpMsg->WKey,
			lpMsg->EKey,
			lpMsg->ChatWindow,
			lpMsg->RKey,
			lpMsg->QWERLevel);

#endif
	}

	gQueryManager.Close();
}

void GDPetItemInfoSaveRecv(const SDHP_PET_ITEM_INFO_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_PET_ITEM_INFO_SAVE_RECV);

	for (int n = 0; n < lpMsg->count; ++n)
	{
		auto* lpInfo =
			reinterpret_cast<const SDHP_PET_ITEM_INFO_SAVE*>(
				reinterpret_cast<const BYTE*>(lpMsg) +
				sizeof(SDHP_PET_ITEM_INFO_SAVE_RECV) +
				(sizeof(SDHP_PET_ITEM_INFO_SAVE) * n));

		const bool exists =
			gQueryManager.ExecQuery(
				"SELECT 1 FROM T_PetItem_Info WHERE ItemSerial=%d",
				lpInfo->serial) &&
			(gQueryManager.Fetch() != SQL_NO_DATA);

		gQueryManager.Close();

		if (exists)
		{
			gQueryManager.ExecQuery(
				"UPDATE T_PetItem_Info "
				"SET Pet_Level=%d,Pet_Exp=%d "
				"WHERE ItemSerial=%d",
				lpInfo->level,
				lpInfo->experience,
				lpInfo->serial);
		}
		else
		{
			gQueryManager.ExecQuery(
				"INSERT INTO T_PetItem_Info "
				"(ItemSerial,Pet_Level,Pet_Exp) "
				"VALUES (%d,%d,%d)",
				lpInfo->serial,
				lpInfo->level,
				lpInfo->experience);
		}

		gQueryManager.Close();
	}
}

void GDResetInfoSaveRecv(const SDHP_RESET_INFO_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RESET_INFO_SAVE_RECV);

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

void GDMasterResetInfoSaveRecv(const SDHP_MASTER_RESET_INFO_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_MASTER_RESET_INFO_SAVE_RECV);

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

void GDRankingDuelSaveRecv(const SDHP_RANKING_DUEL_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_DUEL_SAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT 1 FROM RankingDuel WHERE Name='%s'",
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

void GDRankingBloodCastleSaveRecv(const SDHP_RANKING_BLOOD_CASTLE_SAVE_RECV* lpMsg,	int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_BLOOD_CASTLE_SAVE_RECV);

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

void GDRankingChaosCastleSaveRecv(const SDHP_RANKING_CHAOS_CASTLE_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_CHAOS_CASTLE_SAVE_RECV);

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

void GDRankingDevilSquareSaveRecv(const SDHP_RANKING_DEVIL_SQUARE_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_DEVIL_SQUARE_SAVE_RECV);

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

void GDRankingIllusionTempleSaveRecv(
	const SDHP_RANKING_ILLUSION_TEMPLE_SAVE_RECV* lpMsg,
	int serverIndex,
	int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_ILLUSION_TEMPLE_SAVE_RECV);

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

void GDCreationCardSaveRecv(const SDHP_CREATION_CARD_SAVE_RECV* lpMsg, int serverIndex,	int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CREATION_CARD_SAVE_RECV);

	gQueryManager.ExecQuery(
		"UPDATE AccountCharacter SET ExtClass=%d WHERE Id='%s'",
		lpMsg->ExtClass,
		lpMsg->Account);

	gQueryManager.Close();
}

void GDCrywolfInfoSaveRecv(const SDHP_CRYWOLF_INFO_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CRYWOLF_INFO_SAVE_RECV);

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

void GDSNSDataSaveRecv(const SDHP_SNS_DATA_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 801)

	VALIDATE_PACKET_SIZE(SDHP_SNS_DATA_SAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM SNSData WHERE Name='%s'",
			lpMsg->CharacterName) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	gQueryManager.BindParameterAsBinary(
		1,
		lpMsg->data,
		sizeof(lpMsg->data));

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

void GDCustomMonsterRewardSaveRecv(const SDHP_CUSTOM_MONSTER_REWARD_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOM_MONSTER_REWARD_SAVE_RECV);

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

void GDRankingCustomArenaSaveRecv(const SDHP_RANKING_CUSTOM_ARENA_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_CUSTOM_ARENA_SAVE_RECV);

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

void GDRankingTvTEventSaveRecv(const SDHP_RANKING_TVT_EVENT_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_RANKING_TVT_EVENT_SAVE_RECV);

	if (gQueryManager.ExecQuery(
		"EXEC WZ_TvTRanking '%s','%s','%d','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->KillCount,
		lpMsg->DeathCount))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

void GDConnectCharacterRecv(const SDHP_CONNECT_CHARACTER_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CONNECT_CHARACTER_RECV);

	CHARACTER_INFO characterInfo{};

	if (gCharacterManager.GetCharacterInfo(
		&characterInfo,
		lpMsg->CharacterName))
	{
		return;
	}

	std::memcpy(
		characterInfo.CharacterName,
		lpMsg->CharacterName,
		sizeof(characterInfo.CharacterName));

	std::memcpy(
		characterInfo.Account,
		lpMsg->Account,
		sizeof(characterInfo.Account));

	characterInfo.UserIndex = lpMsg->Index;
	characterInfo.GameServerCode = gServerManager[serverIndex].m_ServerCode;

	gCharacterManager.InsertCharacterInfo(characterInfo);

	FriendStateRecv(lpMsg->CharacterName, 0);
}

void GDDisconnectCharacterRecv(const SDHP_DISCONNECT_CHARACTER_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_DISCONNECT_CHARACTER_RECV);

	CHARACTER_INFO characterInfo{};

	if (!gCharacterManager.GetCharacterInfo(
		&characterInfo,
		lpMsg->CharacterName))
	{
		return;
	}

	if (characterInfo.UserIndex != lpMsg->Index)
	{
		return;
	}

	if (characterInfo.GameServerCode != gServerManager[serverIndex].m_ServerCode)
	{
		return;
	}

	gCharacterManager.RemoveCharacterInfo(characterInfo);

	FriendStateRecv(lpMsg->CharacterName, 1);
}

void GDGlobalWhisperRecv(const SDHP_GLOBAL_WHISPER_RECV* lpMsg,	int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_GLOBAL_WHISPER_RECV);

	SDHP_GLOBAL_WHISPER_SEND pMsg{};

	pMsg.Header.set(HEAD_GLOBAL_WHISPER, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.TargetName, lpMsg->TargetCharacterName, sizeof(pMsg.TargetName));
	std::memcpy(pMsg.Message, lpMsg->Message, sizeof(pMsg.Message));

	CHARACTER_INFO characterInfo{};

	if (gCharacterManager.GetCharacterInfo(&characterInfo, lpMsg->TargetCharacterName))
	{
		pMsg.Result = 1;

		DGGlobalWhisperEchoSend(
			characterInfo.GameServerCode,
			characterInfo.UserIndex,
			characterInfo.Account,
			characterInfo.CharacterName,
			lpMsg->CharacterName,
			lpMsg->Message);
	}
	else
	{
		pMsg.Result = 0;
	}

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));
}

void DGGlobalWhisperEchoSend(WORD serverCode, WORD userIndex, const char* account, const char* targetCharacterName,	const char* sourceCharacterName, const char* message)
{
	SDHP_GLOBAL_WHISPER_ECHO_SEND pMsg{};

	pMsg.Header.set(
		HEAD_GLOBAL_WHISPER_ECHO,
		sizeof(pMsg));

	pMsg.Index = userIndex;

	std::memcpy(pMsg.Account, account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, targetCharacterName, sizeof(pMsg.CharacterName));
	std::memcpy(pMsg.SourceName, sourceCharacterName, sizeof(pMsg.SourceName));
	std::memcpy(pMsg.Message, message, sizeof(pMsg.Message));

	CServerManager* const lpServerManager = FindServerByCode(serverCode);

	if (lpServerManager == nullptr)
	{
		return;
	}

	gSocketManager.DataSend(lpServerManager->m_index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

// RAW FUNCTIONS

void GDMarryInfoSaveRecv(const SDHP_MARRY_INFO_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_MARRY_INFO_SAVE_RECV);

	if (std::strcmp(lpMsg->Mode, "marry") == 0)
	{
		if (gQueryManager.ExecQuery(
			"EXEC WZ_SetMarryInfo '%s','%s'",
			lpMsg->CharacterName,
			lpMsg->MarryName))
		{
			gQueryManager.Fetch();
		}
	}
	else if (std::strcmp(lpMsg->Mode, "divorce") == 0)
	{
		if (gQueryManager.ExecQuery(
			"EXEC WZ_SetDivorceInfo '%s','%s'",
			lpMsg->CharacterName,
			lpMsg->MarryName))
		{
			gQueryManager.Fetch();
		}
	}

	gQueryManager.Close();
}

#if (ENABLECUSTOMQUEST)
void GDCustomQuestRecv(const SDHP_CUSTOMQUEST_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOMQUEST_RECV);

	SDHP_CUSTOMQUEST_SEND pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_QUEST, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(
		pMsg.CharacterName,
		lpMsg->CharacterName,
		sizeof(pMsg.CharacterName));

	pMsg.quest = 0;

	if (gQueryManager.ExecQuery(
		"SELECT quest FROM CustomQuest WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.quest = gQueryManager.GetAsInteger("quest");
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDCustomQuestSaveRecv(const SDHP_CUSTOMQUEST_SAVE_RECV* lpMsg,	int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOMQUEST_SAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM CustomQuest WHERE Name='%s'",
			lpMsg->CharacterName) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE CustomQuest SET quest=%d WHERE Name='%s'",
			lpMsg->quest,
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO CustomQuest (Name,quest) VALUES ('%s',%d)",
			lpMsg->CharacterName,
			lpMsg->quest);
	}

	gQueryManager.Close();
}
#endif

void GDSetCoinRecv(const SDHP_SETCOIN_RECV* lpMsg, int serverIndex,	int size)
{
	VALIDATE_PACKET_SIZE(SDHP_SETCOIN_RECV);

	if (gQueryManager.ExecQuery(
		"EXEC WZ_SetCoin '%s','%s','%d','%d','%d'",
		lpMsg->Account,
		lpMsg->CharacterName,
		lpMsg->Value1,
		lpMsg->Value2,
		lpMsg->Value3))
	{
		gQueryManager.Fetch();
	}

	gQueryManager.Close();
}

#if (ENABLECUSTOMRANKING)
void GDCustomRankingRecv(const SDHP_CUSTOM_RANKING_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOM_RANKING_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	PMSG_CUSTOM_RANKING_SEND pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_RANKING, 0);

	int sendSize = sizeof(pMsg);

	pMsg.Index = lpMsg->Index;
	pMsg.type = lpMsg->type;
	pMsg.count = 0;

	CUSTOM_RANKING_DATA info{};

	if (gQueryManager.ExecQuery("EXEC WZ_CustomRanking %d", lpMsg->type))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			gQueryManager.GetAsString("VALUE1", info.szName, sizeof(info.szName));
			info.valor = gQueryManager.GetAsInteger("VALUE2");

			if ((sendSize + static_cast<int>(sizeof(info))) > MAX_SEND_PACKET_SIZE)
			{
				break;
			}

			std::memcpy(send + sendSize, &info, sizeof(info));

			sendSize += sizeof(info);

			++pMsg.count;
		}
	}

	gQueryManager.Close();

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);
}
#endif

#if(NEWRANKING)
void CharacterRanking(const GDTop* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(GDTop);

	DGCharTop pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_EXTENSIONS, SUB_SKIN_SAVE, sizeof(pMsg));

	int characterCount = 0;

	if (gQueryManager.ExecQuery("EXEC EX_RANKING1"))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			if (gQueryManager.GetResult(1) < 0)
			{
				break;
			}

			char characterName[11]{};

			gQueryManager.GetAsString("Name", characterName, sizeof(characterName));

			std::strncpy(
				pMsg.Tp[characterCount].CharacterName,
				characterName,
				sizeof(pMsg.Tp[characterCount].CharacterName));

			pMsg.Tp[characterCount].CharacterName[sizeof(pMsg.Tp[characterCount].CharacterName) - 1] = 0;

			pMsg.Tp[characterCount].Class = gQueryManager.GetAsInteger("Class");
			pMsg.Tp[characterCount].Level = gQueryManager.GetAsInteger("cLevel");
			pMsg.Tp[characterCount].Reset = gQueryManager.GetAsInteger("ResetCount");
			pMsg.Tp[characterCount].Relifes = gQueryManager.GetAsInteger("rResetLife");
			pMsg.Tp[characterCount].Time = gQueryManager.GetAsInteger("Resets_Time");
			pMsg.Tp[characterCount].Map = gQueryManager.GetAsInteger("MapNumber");

			if (++characterCount >= 150)
			{
				break;
			}
		}
	}

	gQueryManager.Close();

	for (int n = 0; n < characterCount; ++n)
	{
		if (!gQueryManager.ExecQuery(
			"EXEC EX_RANKING2 '%s'",
			pMsg.Tp[n].CharacterName) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			continue;
		}

		char guildName[9]{};

		gQueryManager.GetAsString("G_Name", guildName, sizeof(guildName));

		std::strncpy(
			pMsg.Tp[n].GuildName,
			guildName,
			sizeof(pMsg.Tp[n].GuildName));

		pMsg.Tp[n].GuildName[sizeof(pMsg.Tp[n].GuildName) - 1] = 0;

		gQueryManager.Close();
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}
#endif

void GetInfoCharTopBuffPhe(const BUFFPHE_REQUESTDS* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(BUFFPHE_REQUESTDS);

	INFOCHAR_BUFFPHE pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_EXTENSIONS, SUB_INFOCHAR_BUFFPHE, sizeof(pMsg));

	pMsg.Index = lpMsg->aIndex;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = (gUtil.CheckTextSyntax(lpMsg->CharacterName, sizeof(lpMsg->CharacterName)) != 0);

	if (pMsg.Result == 0 ||
		!gQueryManager.ExecQuery(
			"SELECT cLevel,Class,Inventory,MagicList,EffectList,PheHanhTau,PointUsePhe "
			"FROM Character WHERE Name='%s'",
			lpMsg->CharacterName) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 0;

		gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

		return;
	}

	pMsg.Level = static_cast<WORD>(gQueryManager.GetAsInteger("cLevel"));
	pMsg.Class = static_cast<BYTE>(gQueryManager.GetAsInteger("Class"));

	gQueryManager.GetAsBinary("Inventory", pMsg.Inventory[0], sizeof(pMsg.Inventory));
	gQueryManager.GetAsBinary("MagicList", pMsg.Skill[0], sizeof(pMsg.Skill));
	gQueryManager.GetAsBinary("EffectList", pMsg.Effect[0], sizeof(pMsg.Effect));

	pMsg.ChonPheHanhTau = static_cast<BYTE>(gQueryManager.GetAsInteger("PheHanhTau"));
	pMsg.PointUsePhe = static_cast<DWORD>(gQueryManager.GetAsInteger("PointUsePhe"));

	gQueryManager.Close();

	if (!gQueryManager.ExecQuery(
		"SELECT G_Name,G_Status FROM GuildMember WHERE Name='%s'",
		lpMsg->CharacterName) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		std::memset(pMsg.GuildName, 0, sizeof(pMsg.GuildName));

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.GetAsString("G_Name", pMsg.GuildName, sizeof(pMsg.GuildName));
		pMsg.GuildStatus = gQueryManager.GetAsInteger("G_Status");

		gQueryManager.Close();

		if (gQueryManager.ExecQuery(
			"SELECT Number FROM Guild WHERE G_Name='%s'",
			pMsg.GuildName) &&
			gQueryManager.Fetch() != SQL_NO_DATA)
		{
			pMsg.GuildNumber = static_cast<BYTE>(gQueryManager.GetAsInteger("Number"));
		}

		gQueryManager.Close();
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GetDBBuffPhe(const BUFFPHE_REQUESTDS* lpMsg, int serverIndex, int size)
{
	BUFFPHE_REQUESTDS_SETINFO pMsg {};

	pMsg.Header.set(HEAD_CUSTOM_EXTENSIONS, SUB_BUFFPHE_REQUESTDS, sizeof(pMsg));

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
	gSocketManager.DataSend(serverIndex, (BYTE*)& pMsg, sizeof(pMsg));

}

void GDCustomAttackResumeRecv(const SDHP_CARESUME_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CARESUME_RECV);

	SDHP_CARESUME_SEND pMsg{};

	pMsg.Header.set(HEAD_ATTACK_RESUME, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (gQueryManager.ExecQuery(
		"SELECT Active,Skill,Map,PosX,PosY,AutoBuff,OffPvP,AutoReset,AutoAddStr,AutoAddAgi,AutoAddVit,AutoAddEne,AutoAddCmd "
		"FROM CustomAttack WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.Active = gQueryManager.GetAsInteger("Active");
			pMsg.Skill = gQueryManager.GetAsInteger("Skill");
			pMsg.Map = gQueryManager.GetAsInteger("Map");
			pMsg.Posx = gQueryManager.GetAsInteger("PosX");
			pMsg.Posy = gQueryManager.GetAsInteger("PosY");
			pMsg.AutoBuff = gQueryManager.GetAsInteger("AutoBuff");
			pMsg.OffPVP = gQueryManager.GetAsInteger("OffPvP");
			pMsg.AutoReset = gQueryManager.GetAsInteger("AutoReset");
			pMsg.AutoAddStr = gQueryManager.GetAsInteger("AutoAddStr");
			pMsg.AutoAddAgi = gQueryManager.GetAsInteger("AutoAddAgi");
			pMsg.AutoAddVit = gQueryManager.GetAsInteger("AutoAddVit");
			pMsg.AutoAddEne = gQueryManager.GetAsInteger("AutoAddEne");
			pMsg.AutoAddCmd = gQueryManager.GetAsInteger("AutoAddCmd");
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDCustomAttackSaveRecv(const SDHP_CARESUME_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CARESUME_SAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM CustomAttack WHERE Name='%s'",
			lpMsg->CharacterName) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE CustomAttack SET "
			"Active=%d,"
			"Skill=%d,"
			"Map=%d,"
			"PosX=%d,"
			"PosY=%d,"
			"AutoBuff=%d,"
			"OffPvP=%d,"
			"AutoReset=%d,"
			"AutoAddStr=%d,"
			"AutoAddAgi=%d,"
			"AutoAddVit=%d,"
			"AutoAddEne=%d,"
			"AutoAddCmd=%d "
			"WHERE Name='%s'",
			lpMsg->Active,
			lpMsg->Skill,
			lpMsg->Map,
			lpMsg->Posx,
			lpMsg->Posy,
			lpMsg->AutoBuff,
			lpMsg->OffPVP,
			lpMsg->AutoReset,
			lpMsg->AutoAddStr,
			lpMsg->AutoAddAgi,
			lpMsg->AutoAddVit,
			lpMsg->AutoAddEne,
			lpMsg->AutoAddCmd,
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO CustomAttack "
			"(Name,Active,Skill,Map,PosX,PosY,AutoBuff,OffPvP,AutoReset,AutoAddStr,AutoAddAgi,AutoAddVit,AutoAddEne,AutoAddCmd) "
			"VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
			lpMsg->CharacterName,
			lpMsg->Active,
			lpMsg->Skill,
			lpMsg->Map,
			lpMsg->Posx,
			lpMsg->Posy,
			lpMsg->AutoBuff,
			lpMsg->OffPVP,
			lpMsg->AutoReset,
			lpMsg->AutoAddStr,
			lpMsg->AutoAddAgi,
			lpMsg->AutoAddVit,
			lpMsg->AutoAddEne,
			lpMsg->AutoAddCmd);
	}

	gQueryManager.Close();
}


void GDCustomNpcQuestSaveRecv(const SDHP_CUSTOMNPCQUEST_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOMNPCQUEST_SAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM CustomNpcQuest WHERE Name='%s' AND Quest=%d",
			lpMsg->CharacterName,
			lpMsg->Quest) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE CustomNpcQuest "
			"SET Count=Count+1,MonsterCount=99999 "
			"WHERE Name='%s' AND Quest=%d",
			lpMsg->CharacterName,
			lpMsg->Quest);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO CustomNpcQuest "
			"(Name,Quest,Count,MonsterCount) "
			"VALUES ('%s',%d,1,99999)",
			lpMsg->CharacterName,
			lpMsg->Quest);
	}

	gQueryManager.Close();
}

void GDCustomNpcQuestRecv(const SDHP_CUSTOMNPCQUEST_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOMNPCQUEST_RECV);

	SDHP_CUSTOMNPCQUEST_SEND pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_NPC_QUEST, 0x00, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;
	pMsg.Quest = lpMsg->Quest;
	pMsg.Indexnpc = lpMsg->Indexnpc;
	pMsg.QuestCount = 0;
	pMsg.MonsterCount = 99999;

	if (gQueryManager.ExecQuery(
		"SELECT Count,MonsterCount FROM CustomNpcQuest WHERE Name='%s' AND Quest=%d",
		lpMsg->CharacterName,
		lpMsg->Quest))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.QuestCount = gQueryManager.GetAsInteger("Count");
			pMsg.MonsterCount = gQueryManager.GetAsInteger("MonsterCount");
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));
}


void GDCustomNpcQuestMonsterCountSaveRecv(const SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOMNPCQUESTMONSTERSAVE_RECV);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT Name FROM CustomNpcQuest WHERE Name='%s' AND Quest=%d",
			lpMsg->CharacterName,
			lpMsg->Quest) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE CustomNpcQuest SET MonsterCount=%d WHERE Name='%s' AND Quest=%d",
			lpMsg->MonsterQtd,
			lpMsg->CharacterName,
			lpMsg->Quest);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO CustomNpcQuest (Name,Quest,Count,MonsterCount) VALUES ('%s',%d,0,%d)",
			lpMsg->CharacterName,
			lpMsg->Quest,
			lpMsg->MonsterQtd);
	}

	gQueryManager.Close();
}

void GDCustomJewelBankRecv(const SDHP_CUSTOM_JEWELBANK_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOM_JEWELBANK_RECV);

	int bless = 0;
	int soul = 0;
	int life = 0;
	int creation = 0;
	int guardian = 0;
	int gemStone = 0;
	int harmony = 0;
	int chaos = 0;
	int lowStone = 0;
	int highStone = 0;

	switch (lpMsg->Type)
	{
	case 0: bless = lpMsg->Count; break;
	case 1: soul = lpMsg->Count; break;
	case 2: life = lpMsg->Count; break;
	case 3: creation = lpMsg->Count; break;
	case 4: guardian = lpMsg->Count; break;
	case 5: gemStone = lpMsg->Count; break;
	case 6: harmony = lpMsg->Count; break;
	case 7: chaos = lpMsg->Count; break;
	case 8: lowStone = lpMsg->Count; break;
	case 9: highStone = lpMsg->Count; break;
	default:
		return;
	}

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT AccountID FROM CustomJewelBank WHERE AccountID='%s'",
			lpMsg->Account) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE CustomJewelBank SET "
			"Bless=Bless+%d,"
			"Soul=Soul+%d,"
			"Life=Life+%d,"
			"Creation=Creation+%d,"
			"Guardian=Guardian+%d,"
			"GemStone=GemStone+%d,"
			"Harmony=Harmony+%d,"
			"Chaos=Chaos+%d,"
			"LowStone=LowStone+%d,"
			"HighStone=HighStone+%d "
			"WHERE AccountID='%s'",
			bless,
			soul,
			life,
			creation,
			guardian,
			gemStone,
			harmony,
			chaos,
			lowStone,
			highStone,
			lpMsg->Account);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO CustomJewelBank "
			"(AccountID,Bless,Soul,Life,Creation,Guardian,GemStone,Harmony,Chaos,LowStone,HighStone) "
			"VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
			lpMsg->Account,
			bless,
			soul,
			life,
			creation,
			guardian,
			gemStone,
			harmony,
			chaos,
			lowStone,
			highStone);
	}

	gQueryManager.Close();
}

void GDCustomJewelBankInfoRecv(const SDHP_CUSTOM_JEWELBANK_INFO_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOM_JEWELBANK_INFO_RECV);

	SDHP_CUSTOM_JEWELBANK_INFO_SEND pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_NPC_QUEST, SUB_CUSTOM_JEWEL_BANK_INFO, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	if (gQueryManager.ExecQuery(
		"SELECT Bless,Soul,Life,Creation,Guardian,GemStone,Harmony,Chaos,LowStone,HighStone "
		"FROM CustomJewelBank WHERE AccountID='%s'",
		lpMsg->Account) &&
		gQueryManager.Fetch() != SQL_NO_DATA)
	{
		pMsg.Bless = gQueryManager.GetAsInteger("Bless");
		pMsg.Soul = gQueryManager.GetAsInteger("Soul");
		pMsg.Life = gQueryManager.GetAsInteger("Life");
		pMsg.Creation = gQueryManager.GetAsInteger("Creation");
		pMsg.Guardian = gQueryManager.GetAsInteger("Guardian");
		pMsg.GemStone = gQueryManager.GetAsInteger("GemStone");
		pMsg.Harmony = gQueryManager.GetAsInteger("Harmony");
		pMsg.Chaos = gQueryManager.GetAsInteger("Chaos");
		pMsg.LowStone = gQueryManager.GetAsInteger("LowStone");
		pMsg.HighStone = gQueryManager.GetAsInteger("HighStone");
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

#if (TOP1RESETLIMIT)
void GDCustomGHRSRecv(const SDHP_CUSTOM_GHRS_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_CUSTOM_GHRS_RECV);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	PMSG_CUSTOM_GHRS_SEND pMsg{};

	pMsg.Header.set(SUB_CUSTOM_GHRS_SEND, 0);

	int sendSize = sizeof(pMsg);

	pMsg.Time = lpMsg->Time;
	pMsg.Resets = 0;
	pMsg.Grand = 0;

	if (gQueryManager.ExecQuery(
		"SELECT TOP 1 MasterResetCount,ResetCount FROM GHRS_top1 WHERE Time=%d ORDER BY MasterResetCount DESC,ResetCount DESC",
		lpMsg->Time) &&
		gQueryManager.Fetch() != SQL_NO_DATA)
	{
		pMsg.Grand = gQueryManager.GetAsInteger("MasterResetCount");
		pMsg.Resets = gQueryManager.GetAsInteger("ResetCount");

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();

		if (gQueryManager.ExecQuery(
			"SELECT TOP 1 MasterResetCount,ResetCount FROM Character WHERE CtlCode=0 ORDER BY MasterResetCount DESC,ResetCount DESC,cLevel DESC,Experience DESC,Money DESC") &&
			gQueryManager.Fetch() != SQL_NO_DATA)
		{
			pMsg.Grand = gQueryManager.GetAsInteger("MasterResetCount");
			pMsg.Resets = gQueryManager.GetAsInteger("ResetCount");
		}

		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO GHRS_top1 (Time,MasterResetCount,ResetCount) VALUES (%d,%d,%d)",
			lpMsg->Time,
			pMsg.Grand,
			pMsg.Resets);

		gQueryManager.Close();

		Log.ToDisp(LOG_GREEN, "Gioi han reset Updated!");

#if (ANCIENTBATTLEFIELDEVENT)
		gQueryManager.ExecQuery(
			"UPDATE Character SET CTCTime=900,CTCRegDay=0");

		gQueryManager.Close();

		Log.ToDisp(LOG_BLUE, "ChienTruongCo Update!");
#endif

#if (ENDLESSTOWEREVENT)
		gQueryManager.ExecQuery(
			"UPDATE Character SET LuotDiEndLess=0");

		gQueryManager.Close();
#endif
	}

	std::memcpy(send, &pMsg, sizeof(pMsg));

	sendSize = sizeof(pMsg);

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);
}
#endif

#if (TOP1BOTSTATUE)
void GDBotInfoRecv(const SDHP_BOT_INFO_GET* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_BOT_INFO_GET);

	SDHP_BOT_INFO_SEND pMsg{};

	pMsg.Header.set(HEAD_BOT_INFO, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;
	pMsg.Result = 0;

	char characterName[11]{};

	if (gQueryManager.ExecQuery(
		"EXEC WZ_GetTopStatue %d,%d",
		lpMsg->TypeTop,
		lpMsg->Rank) &&
		gQueryManager.Fetch() != SQL_NO_DATA)
	{
		gQueryManager.GetAsString(
			"Name",
			characterName,
			sizeof(characterName));
	}

	gQueryManager.Close();

	std::memcpy(
		pMsg.CharacterName,
		characterName,
		sizeof(pMsg.CharacterName));

	if (!gUtil.CheckTextSyntax(characterName, sizeof(characterName)))
	{
		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	if (!gQueryManager.ExecQuery(
		"SELECT * FROM Character WHERE Name='%s'",
		characterName) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gSocketManager.DataSend(
			serverIndex,
			reinterpret_cast<BYTE*>(&pMsg),
			sizeof(pMsg));

		return;
	}

	pMsg.Result = 1;

	pMsg.Level = static_cast<WORD>(gQueryManager.GetAsInteger("cLevel"));
	pMsg.Class = static_cast<BYTE>(gQueryManager.GetAsInteger("Class"));
	pMsg.LevelUpPoint = gQueryManager.GetAsInteger("LevelUpPoint");
	pMsg.Experience = gQueryManager.GetAsInteger("Experience");
	pMsg.Strength = gQueryManager.GetAsInteger("Strength");
	pMsg.Dexterity = gQueryManager.GetAsInteger("Dexterity");
	pMsg.Vitality = gQueryManager.GetAsInteger("Vitality");
	pMsg.Energy = gQueryManager.GetAsInteger("Energy");
	pMsg.Leadership = gQueryManager.GetAsInteger("Leadership");

	gQueryManager.GetAsBinary(
		"Inventory",
		pMsg.Inventory[0],
		sizeof(pMsg.Inventory));

	gQueryManager.GetAsBinary(
		"MagicList",
		pMsg.Skill[0],
		sizeof(pMsg.Skill));

	pMsg.Money = gQueryManager.GetAsInteger("Money");
	pMsg.Life = static_cast<DWORD>(gQueryManager.GetAsFloat("Life"));
	pMsg.MaxLife = static_cast<DWORD>(gQueryManager.GetAsFloat("MaxLife"));
	pMsg.Mana = static_cast<DWORD>(gQueryManager.GetAsFloat("Mana"));
	pMsg.MaxMana = static_cast<DWORD>(gQueryManager.GetAsFloat("MaxMana"));
	pMsg.BP = static_cast<DWORD>(gQueryManager.GetAsFloat("BP"));
	pMsg.MaxBP = static_cast<DWORD>(gQueryManager.GetAsFloat("MaxBP"));
	pMsg.Shield = static_cast<DWORD>(gQueryManager.GetAsFloat("Shield"));
	pMsg.MaxShield = static_cast<DWORD>(gQueryManager.GetAsFloat("MaxShield"));
	pMsg.Dir = static_cast<BYTE>(gQueryManager.GetAsInteger("MapDir"));
	pMsg.PKCount = gQueryManager.GetAsInteger("PkCount");
	pMsg.PKLevel = gQueryManager.GetAsInteger("PkLevel");
	pMsg.PKTime = gQueryManager.GetAsInteger("PkTime");
	pMsg.CtlCode = static_cast<BYTE>(gQueryManager.GetAsInteger("CtlCode"));

	gQueryManager.GetAsBinary(
		"EffectList",
		pMsg.Effect[0],
		sizeof(pMsg.Effect));

	pMsg.FruitAddPoint = static_cast<WORD>(gQueryManager.GetAsInteger("FruitAddPoint"));
	pMsg.FruitSubPoint = static_cast<WORD>(gQueryManager.GetAsInteger("FruitSubPoint"));
	pMsg.Kills = static_cast<DWORD>(gQueryManager.GetAsInteger("Kills"));
	pMsg.Deads = static_cast<DWORD>(gQueryManager.GetAsInteger("Deads"));
	pMsg.rDanhHieu = gQueryManager.GetAsInteger("rDanhHieu");

	gQueryManager.Close();

	if (gQueryManager.ExecQuery(
		"SELECT G_Name,G_Status FROM GuildMember WHERE Name='%s'",
		characterName) &&
		gQueryManager.Fetch() != SQL_NO_DATA)
	{
		gQueryManager.GetAsString(
			"G_Name",
			pMsg.GuildName,
			sizeof(pMsg.GuildName));

		pMsg.GuildStatus = gQueryManager.GetAsInteger("G_Status");

		gQueryManager.Close();

		if (gQueryManager.ExecQuery(
			"SELECT Number FROM Guild WHERE G_Name='%s'",
			pMsg.GuildName) &&
			gQueryManager.Fetch() != SQL_NO_DATA)
		{
			pMsg.GuildNumber = static_cast<BYTE>(gQueryManager.GetAsInteger("Number"));
		}

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();

		std::memset(pMsg.GuildName,	0, sizeof(pMsg.GuildName));
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}
#endif

void GDGetSkinIsBuy(const GSSENDDS_GETLISTISBUYSKIN* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(GSSENDDS_GETLISTISBUYSKIN);

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	CBCUSTOM_LOAD_COUNT pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_EXTENSIONS, SUB_SKIN_GET_LIST, 0);

	int sendSize = sizeof(pMsg);

	pMsg.count = 0;
	pMsg.aIndex = lpMsg->aIndex;

	BCUSTOM_SKINMODEL_DATA info{};

	if (gQueryManager.ExecQuery("SELECT SkinIndex,StatusBuy FROM SkinModel WHERE AccountID='%s'", lpMsg->AccountID))
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{
			info.SkinIndex = gQueryManager.GetAsInteger("SkinIndex");
			info.StatusBuy = gQueryManager.GetAsInteger("StatusBuy");

			if ((sendSize + static_cast<int>(sizeof(info))) > MAX_SEND_PACKET_SIZE)
			{
				break;
			}

			std::memcpy(&send[sendSize], &info, sizeof(info));
			sendSize += sizeof(info);

			++pMsg.count;
		}
	}

	gQueryManager.Close();

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);
}

void GDSaveSkinBuy(const GSSENDDS_GETLISTISBUYSKIN* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(GSSENDDS_GETLISTISBUYSKIN);

	const bool exists =
		gQueryManager.ExecQuery(
			"SELECT AccountID FROM SkinModel WHERE AccountID='%s' AND SkinIndex=%d",
			lpMsg->AccountID,
			lpMsg->aIndex) &&
		(gQueryManager.Fetch() != SQL_NO_DATA);

	gQueryManager.Close();

	if (!exists)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO SkinModel (AccountID,SkinIndex,StatusBuy) VALUES ('%s',%d,1)",
			lpMsg->AccountID,
			lpMsg->aIndex);

		gQueryManager.Close();
	}
}

#if (CUSTOMELEMENTALBOOK)
void GDCharacterElementalBookRecv(
	const ELEMENTALBOOK_GD_REQ_DATA* lpMsg,
	int serverIndex,
	int size)
{
	VALIDATE_PACKET_SIZE(ELEMENTALBOOK_GD_REQ_DATA);

	SACHTHUOCTINH_DG_GET_DATA pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_PROTOCOL, SUB_CUSTOM_ELEMENTAL_BOOK_SAVE, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	if (gQueryManager.ExecQuery(
		"SELECT * FROM CustomSachThuocTinh WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.BookFire = static_cast<BYTE>(gQueryManager.GetAsInteger("SACH_01"));
			pMsg.BookWater = static_cast<BYTE>(gQueryManager.GetAsInteger("SACH_02"));
			pMsg.BookWind = static_cast<BYTE>(gQueryManager.GetAsInteger("SACH_03"));
			pMsg.BookEarth = static_cast<BYTE>(gQueryManager.GetAsInteger("SACH_04"));
			pMsg.BookDark = static_cast<BYTE>(gQueryManager.GetAsInteger("SACH_05"));
			pMsg.BookLight = static_cast<BYTE>(gQueryManager.GetAsInteger("SACH_06"));
			pMsg.BookPoison = static_cast<BYTE>(gQueryManager.GetAsInteger("SACH_07"));
		}
		else
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"INSERT INTO CustomSachThuocTinh "
				"(Name,SACH_01,SACH_02,SACH_03,SACH_04,SACH_05,SACH_06,SACH_07) "
				"VALUES ('%s',0,0,0,0,0,0,0)",
				lpMsg->CharacterName);

			gQueryManager.Close();

			gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

			return;
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDCharacterElementalBookSaveRecv(const ELEMENTALBOOK_GD_SAVE_DATA* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(ELEMENTALBOOK_GD_SAVE_DATA);

	gQueryManager.ExecQuery(
		"UPDATE CustomSachThuocTinh SET "
		"SACH_01=%d,"
		"SACH_02=%d,"
		"SACH_03=%d,"
		"SACH_04=%d,"
		"SACH_05=%d,"
		"SACH_06=%d,"
		"SACH_07=%d "
		"WHERE Name='%s'",
		lpMsg->BookFire,
		lpMsg->BookWater,
		lpMsg->BookWind,
		lpMsg->BookEarth,
		lpMsg->BookDark,
		lpMsg->BookLight,
		lpMsg->BookPoison,
		lpMsg->Name);

	gQueryManager.Close();
}
#endif


#if (RECHARGEREWARD == 1)
void GDCharacterMocNapRecv(const MOCNAP_GD_REQ_DATA* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(MOCNAP_GD_REQ_DATA);

	MOCNAP_DG_GET_DATA pMsg{};

	pMsg.Header.set(HEAD_CUSTOM_PROTOCOL, SUB_MOCNAP_RESULT, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	if (gQueryManager.ExecQuery(
		"SELECT * FROM CustomMocNap WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.MOCNAP1 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP1"));
			pMsg.MOCNAP2 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP2"));
			pMsg.MOCNAP3 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP3"));
			pMsg.MOCNAP4 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP4"));
			pMsg.MOCNAP5 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP5"));
			pMsg.MOCNAP6 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP6"));
			pMsg.MOCNAP7 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP7"));
			pMsg.MOCNAP8 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP8"));
			pMsg.MOCNAP9 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP9"));
			pMsg.MOCNAP10 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP10"));
			pMsg.MOCNAP11 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP11"));
			pMsg.MOCNAP12 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP12"));
			pMsg.MOCNAP13 = static_cast<BYTE>(gQueryManager.GetAsInteger("MOCNAP13"));
		}
		else
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"INSERT INTO CustomMocNap "
				"(Name,MOCNAP1,MOCNAP2,MOCNAP3,MOCNAP4,MOCNAP5,MOCNAP6,"
				"MOCNAP7,MOCNAP8,MOCNAP9,MOCNAP10,MOCNAP11,MOCNAP12,MOCNAP13) "
				"VALUES ('%s',0,0,0,0,0,0,0,0,0,0,0,0,0)",
				lpMsg->CharacterName);

			gQueryManager.Close();

			gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

			return;
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void GDCharacterMocNapSaveRecv(const MOCNAP_GD_SAVE_DATA* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(MOCNAP_GD_SAVE_DATA);

	gQueryManager.ExecQuery(
		"UPDATE CustomMocNap SET "
		"MOCNAP1=%d,"
		"MOCNAP2=%d,"
		"MOCNAP3=%d,"
		"MOCNAP4=%d,"
		"MOCNAP5=%d,"
		"MOCNAP6=%d,"
		"MOCNAP7=%d,"
		"MOCNAP8=%d,"
		"MOCNAP9=%d,"
		"MOCNAP10=%d,"
		"MOCNAP11=%d,"
		"MOCNAP12=%d,"
		"MOCNAP13=%d "
		"WHERE Name='%s'",
		lpMsg->MOCNAP1,
		lpMsg->MOCNAP2,
		lpMsg->MOCNAP3,
		lpMsg->MOCNAP4,
		lpMsg->MOCNAP5,
		lpMsg->MOCNAP6,
		lpMsg->MOCNAP7,
		lpMsg->MOCNAP8,
		lpMsg->MOCNAP9,
		lpMsg->MOCNAP10,
		lpMsg->MOCNAP11,
		lpMsg->MOCNAP12,
		lpMsg->MOCNAP13,
		lpMsg->CharacterName);

	Log.ToDisp(
		LOG_BLUE,
		"SET MOCNAP1=%d,MOCNAP2=%d,MOCNAP3=%d,MOCNAP4=%d,MOCNAP5=%d,"
		"MOCNAP6=%d,MOCNAP7=%d,MOCNAP8=%d,MOCNAP9=%d,MOCNAP10=%d,"
		"MOCNAP11=%d,MOCNAP12=%d,MOCNAP13=%d WHERE Name='%s'",
		lpMsg->MOCNAP1,
		lpMsg->MOCNAP2,
		lpMsg->MOCNAP3,
		lpMsg->MOCNAP4,
		lpMsg->MOCNAP5,
		lpMsg->MOCNAP6,
		lpMsg->MOCNAP7,
		lpMsg->MOCNAP8,
		lpMsg->MOCNAP9,
		lpMsg->MOCNAP10,
		lpMsg->MOCNAP11,
		lpMsg->MOCNAP12,
		lpMsg->MOCNAP13,
		lpMsg->CharacterName);

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