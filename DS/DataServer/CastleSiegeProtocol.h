// CastleSiegeProtocol.h
#pragma once
#include "DataServerProtocol.h"
#include "CastleDBSet.h"

// GameServer -> DataServer 


//**********************************************//


// Dispatcher principal para HEAD_CASTLE_SIEGE (0x80) y sus heads
// complementarios (0x81-0x89).
void CastleSiegeProtocolCore(BYTE head, BYTE* lpMsg, int index);

void CastleSiegeDataSend(int serverIndex, const BYTE* lpMsg, int size);

// GameServer -> DataServer 
void GDReqCastleTotalInfo(BYTE* lpRecv, int index);