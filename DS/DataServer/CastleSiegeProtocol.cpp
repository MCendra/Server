// CastleSiegeProtocol.cpp
#include "Header.h"
#include "CastleSiegeProtocol.h"
#include "SocketManager.h"
#include "Log.h"

// Dispatcher: centraliza el routing de HEAD_CASTLE_SIEGE y sus heads
// complementarios, sacándolos de DataServerProtocolCore para dar orden.
void CastleSiegeProtocolCore(BYTE head, BYTE* lpMsg, int index)
{
    const BYTE packetHeader = lpMsg[PACKET_TYPE_OFFSET];
    const BYTE subHead =
        (packetHeader == PACKET_C1 || packetHeader == PACKET_C3)
        ? lpMsg[C1_PACKET_DATA_OFFSET]
        : lpMsg[C2_PACKET_DATA_OFFSET];

    switch (head)
    {
    case HEAD_CASTLE_SIEGE:
        switch (subHead)
        {
        case SUB_CASTLE_TOTAL_INFO:          GDReqCastleTotalInfo(lpMsg, index);         break;
        case SUB_CASTLE_OWNER_GUILD_MASTER:  DS_GDReqOwnerGuildMaster(lpMsg, index);     break;
        case SUB_CASTLE_NPC_BUY:             DS_GDReqCastleNpcBuy(lpMsg, index);         break;
        case SUB_CASTLE_NPC_REPAIR:          DS_GDReqCastleNpcRepair(lpMsg, index);      break;
        case SUB_CASTLE_NPC_UPGRADE:         DS_GDReqCastleNpcUpgrade(lpMsg, index);     break;
        case SUB_CASTLE_TAX_INFO:            DS_GDReqTaxInfo(lpMsg, index);              break;
        case SUB_CASTLE_TAX_RATE_CHANGE:     DS_GDReqTaxRateChange(lpMsg, index);        break;
        case SUB_CASTLE_MONEY_CHANGE:        DS_GDReqCastleMoneyChange(lpMsg, index);    break;
        case SUB_CASTLE_SIEGE_DATE_CHANGE:   DS_GDReqSiegeDateChange(lpMsg, index);      break;
        case SUB_CASTLE_GUILD_MARK_INFO:     DS_GDReqGuildMarkRegInfo(lpMsg, index);     break;
        case SUB_CASTLE_SIEGE_ENDED_CHANGE:  DS_GDReqSiegeEndedChange(lpMsg, index);     break;
        case SUB_CASTLE_OWNER_CHANGE:        DS_GDReqCastleOwnerChange(lpMsg, index);    break;
        case SUB_CASTLE_REG_ATTACK_GUILD:    DS_GDReqRegAttackGuild(lpMsg, index);       break;
        case SUB_CASTLE_RESTART_STATE:       DS_GDReqRestartCastleState(lpMsg, index);   break;
        case SUB_CASTLE_MAPSVR_MULTICAST:    DS_GDReqMapSvrMsgMultiCast(lpMsg, index);   break;
        case SUB_CASTLE_REG_GUILD_MARK:      DS_GDReqRegGuildMark(lpMsg, index);         break;
        case SUB_CASTLE_GUILD_MARK_RESET: DS_GDReqGuildMarkReset(lpMsg, index);       break;
        case SUB_CASTLE_GUILD_GIVEUP:     DS_GDReqGuildSetGiveUp(lpMsg, index);       break;
        case SUB_CASTLE_NPC_REMOVE:       DS_GDReqCastleNpcRemove(lpMsg, index);      break;
        case SUB_CASTLE_STATE_SYNC:       DS_GDReqCastleStateSync(lpMsg, index);      break;
        case SUB_CASTLE_TRIBUTE_MONEY:    DS_GDReqCastleTributeMoney(lpMsg, index);   break;
        case SUB_CASTLE_RESET_TAX_INFO:   DS_GDReqResetCastleTaxInfo(lpMsg, index);   break;
        case SUB_CASTLE_RESET_SIEGE_GUILD:DS_GDReqResetSiegeGuildInfo(lpMsg, index);  break;
        case SUB_CASTLE_RESET_REG_INFO:   DS_GDReqResetRegSiegeInfo(lpMsg, index);    break;
        }
        break;

        // Heads complementarios de Castle que también se enrutan desde acá
    case DS_HEAD_CASTLE_INIT_DATA:       DS_GDReqCastleInitData(lpMsg, index);       break;
    case DS_HEAD_CASTLE_NPC_INFO:        DS_GDReqCastleNpcInfo(lpMsg, index);        break;
    case DS_HEAD_CASTLE_ALL_GUILD_MARK:  DS_GDReqAllGuildMarkRegInfo(lpMsg, index);  break;
    case DS_HEAD_CASTLE_FIRST_CREATE_NPC:DS_GDReqFirstCreateNPC(lpMsg, index);       break;
    case DS_HEAD_CASTLE_CALC_REG_GUILD:  DS_GDReqCalcRegGuildList(lpMsg, index);     break;
    case DS_HEAD_CASTLE_GUILD_UNION_INFO:DS_GDReqCsGuildUnionInfo(lpMsg, index);     break;
    case DS_HEAD_CASTLE_SAVE_GUILD_INFO: DS_GDReqCsSaveTotalGuildInfo(lpMsg, index); break;
    case DS_HEAD_CASTLE_LOAD_GUILD_INFO: DS_GDReqCsLoadTotalGuildInfo(lpMsg, index); break;
    case DS_HEAD_CASTLE_NPC_UPDATE:      DS_GDReqCastleNpcUpdate(lpMsg, index);      break;
    }
}


void GDReqCastleTotalInfo(BYTE* lpRecv, int index)
{
    auto* lpMsg = reinterpret_cast<CSP_REQ_CASTLEDATA*>(lpRecv);

    CASTLE_DATA CastleData{};

    CSP_ANS_CASTLEDATA pMsg{};
    pMsg.Header.set(HEAD_CASTLE_SIEGE, SUB_CASTLE_TOTAL_INFO, sizeof(pMsg));
    pMsg.MapSvrNum = lpMsg->MapSvrNum;

    if (!gCastleDBSet.DB_QueryCastleTotalInfo(lpMsg->MapSvrNum, lpMsg->CastleEventCycle, &CastleData))
    {
        pMsg.Result = 0;
    }
    else
    {
        pMsg.Result = 1;
        pMsg.StartYear = CastleData.StartYear;
        pMsg.StartMonth = CastleData.StartMonth;
        pMsg.StartDay = CastleData.StartDay;
        pMsg.EndYear = CastleData.EndYear;
        pMsg.EndMonth = CastleData.EndMonth;
        pMsg.EndDay = CastleData.EndDay;
        pMsg.IsSiegeGuildList = CastleData.IsSiegeGuildList;
        pMsg.IsSiegeEnded = CastleData.IsSiegeEnded;
        pMsg.IsCastleOccupied = CastleData.IsCastleOccupied;
        pMsg.CastleMoney = CastleData.CastleMoney;
        pMsg.TaxRateChaos = CastleData.TaxRateChaos;
        pMsg.TaxRateStore = CastleData.TaxRateStore;
        pMsg.TaxHuntZone = CastleData.TaxHuntZone;
        pMsg.FirstCreate = CastleData.FirstCreate;

        memcpy(pMsg.CastleOwnGuild, CastleData.CastleOwnGuild, sizeof(pMsg.CastleOwnGuild));
    }

    gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}