// PcPoint.cpp
#include "Header.h"
#include "PcPoint.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CPcPoint gPcPoint;

void CPcPoint::GDPcPointPointRecv(const SDHP_PC_POINT_POINT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE <= 402)

	VALIDATE_PACKET_SIZE(SDHP_PC_POINT_POINT_RECV);

	SDHP_PC_POINT_POINT_SEND pMsg{};

	pMsg.Header.set(HEAD_PCPOINT, SUB_PCPOINT_POINT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	if (gQueryManager.ExecQuery(
		"SELECT PcPoint FROM PcPointData WHERE AccountID='%s'",
		lpMsg->Account))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.PcPoint = gQueryManager.GetAsInteger("PcPoint");

			gQueryManager.Close();

			gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

			return;
		}
	}

	gQueryManager.Close();

	if (!gQueryManager.ExecQuery(
		"INSERT INTO PcPointData (AccountID,PcPoint) VALUES ('%s',0)",
		lpMsg->Account))
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CPcPoint::GDPcPointItemBuyRecv(const SDHP_PC_POINT_ITEM_BUY_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE <= 402)

	VALIDATE_PACKET_SIZE(SDHP_PC_POINT_ITEM_BUY_RECV);

	SDHP_PC_POINT_ITEM_BUY_SEND pMsg{};

	pMsg.Header.set(HEAD_PCPOINT, SUB_PCPOINT_ITEM_BUY, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.slot = lpMsg->slot;

	if (gQueryManager.ExecQuery(
		"SELECT PcPoint FROM PcPointData WHERE AccountID='%s'",
		lpMsg->Account))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.PcPoint = gQueryManager.GetAsInteger("PcPoint");
		}
		else
		{
			pMsg.Result = 1;
		}
	}
	else
	{
		pMsg.Result = 1;
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CPcPoint::GDPcPointRecievePointRecv(const SDHP_PC_POINT_RECIEVE_POINT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE <= 402)

	VALIDATE_PACKET_SIZE(SDHP_PC_POINT_RECIEVE_POINT_RECV);

	SDHP_PC_POINT_RECIEVE_POINT_SEND pMsg{};

	pMsg.Header.set(HEAD_PCPOINT, SUB_PCPOINT_RECIEVE_POINT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.CallbackFunc = lpMsg->CallbackFunc;
	pMsg.CallbackArg1 = lpMsg->CallbackArg1;
	pMsg.CallbackArg2 = lpMsg->CallbackArg2;

	if (gQueryManager.ExecQuery(
		"SELECT PcPoint FROM PcPointData WHERE AccountID='%s'",
		lpMsg->Account))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.PcPoint = gQueryManager.GetAsInteger("PcPoint");
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CPcPoint::GDPcPointAddPointSaveRecv(const SDHP_PC_POINT_ADD_POINT_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE <= 402)

	VALIDATE_PACKET_SIZE(SDHP_PC_POINT_ADD_POINT_SAVE_RECV);

	if (!gQueryManager.ExecQuery(
		"SELECT PcPoint FROM PcPointData WHERE AccountID='%s'",
		lpMsg->Account))
	{
		gQueryManager.Close();
		return;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		const DWORD newPcPoint =
			(lpMsg->AddPcPoint > lpMsg->MaxPcPoint)
			? lpMsg->MaxPcPoint
			: lpMsg->AddPcPoint;

		gQueryManager.ExecQuery(
			"INSERT INTO PcPointData (AccountID,PcPoint) VALUES ('%s',%u)",
			lpMsg->Account,
			newPcPoint);

		gQueryManager.Close();
		return;
	}

	const DWORD pcPoint = static_cast<DWORD>(
		gQueryManager.GetAsInteger("PcPoint"));

	gQueryManager.Close();

	const DWORD newPcPoint =
		(pcPoint >= lpMsg->MaxPcPoint ||
			lpMsg->AddPcPoint > (lpMsg->MaxPcPoint - pcPoint))
		? lpMsg->MaxPcPoint
		: (pcPoint + lpMsg->AddPcPoint);

	gQueryManager.ExecQuery(
		"UPDATE PcPointData SET PcPoint=%u WHERE AccountID='%s'",
		newPcPoint,
		lpMsg->Account);

	gQueryManager.Close();

#endif
}

void CPcPoint::GDPcPointSubPointSaveRecv(const SDHP_PC_POINT_SUB_POINT_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE <= 402)

	VALIDATE_PACKET_SIZE(SDHP_PC_POINT_SUB_POINT_SAVE_RECV);

	if (!gQueryManager.ExecQuery(
		"SELECT PcPoint FROM PcPointData WHERE AccountID='%s'",
		lpMsg->Account))
	{
		gQueryManager.Close();
		return;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO PcPointData (AccountID,PcPoint) VALUES ('%s',0)",
			lpMsg->Account);

		gQueryManager.Close();
		return;
	}

	const DWORD pcPoint = static_cast<DWORD>(
		gQueryManager.GetAsInteger("PcPoint"));

	gQueryManager.Close();

	const DWORD newPcPoint =
		(lpMsg->SubPcPoint > pcPoint)
		? 0
		: (pcPoint - lpMsg->SubPcPoint);

	gQueryManager.ExecQuery("UPDATE PcPointData SET PcPoint=%u WHERE AccountID='%s'", newPcPoint, lpMsg->Account);

	gQueryManager.Close();

#endif
}