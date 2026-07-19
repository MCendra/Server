// MuRummy.cpp
#include "Header.h"
#include "MuRummy.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CMuRummy gMuRummy;

void CMuRummy::GDReqCardInfo(const PMSG_REQ_MURUMMY_SELECT_DS* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(PMSG_REQ_MURUMMY_SELECT_DS);

	PMSG_ANS_MURUMMY_SELECT_DS pMsg{};

	pMsg.Header.set(HEAD_MURUMMY, SUB_MURUMMY_LOAD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;
	pMsg.Result = 1;

	if (!gQueryManager.ExecQuery(
		"SELECT TotalScore FROM MuRummyData WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		pMsg.Result = 0;
	}
	else
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			pMsg.Result = 0;
		}
		else
		{
			pMsg.Score = gQueryManager.GetAsInteger("TotalScore");
		}
	}

	gQueryManager.Close();

	if (pMsg.Result != 0)
	{
		if (!gQueryManager.ExecQuery(
			"SELECT Color,Number,Slot,Status,Sequence "
			"FROM MuRummyCard WHERE Name='%s'",
			lpMsg->CharacterName))
		{
			pMsg.Result = 0;
		}
		else
		{
			for (auto sqlRet = gQueryManager.Fetch();
				sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA;
				sqlRet = gQueryManager.Fetch())
			{
				const int sequence =
					gQueryManager.GetAsInteger("Sequence");

				if (sequence < 0 ||
					sequence >= static_cast<int>(_countof(pMsg.stMuRummyCardInfoDS)))
				{
					continue;
				}

				auto& card = pMsg.stMuRummyCardInfoDS[sequence];

				card.Color = gQueryManager.GetAsInteger("Color");
				card.Number = gQueryManager.GetAsInteger("Number");
				card.SlotNum = gQueryManager.GetAsInteger("Slot");
				card.Seq = sequence;
				card.Status = gQueryManager.GetAsInteger("Status");
			}
		}

		gQueryManager.Close();
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg),	sizeof(pMsg));

#endif
}

void CMuRummy::GDReqCardInfoInsert(const PMSG_REQ_MURUMMY_INSERT_DS* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(PMSG_REQ_MURUMMY_INSERT_DS);

	for (const auto& card : lpMsg->stMuRummyCardInfoDS)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO MuRummyCard "
			"(Name,Color,Number,Slot,Status,Sequence) "
			"VALUES ('%s',%d,%d,%d,%d,%d)",
			lpMsg->CharacterName,
			card.Color,
			card.Number,
			card.SlotNum,
			card.Status,
			card.Seq);

		gQueryManager.Close();
	}

#endif
}

void CMuRummy::GDReqCardInfoUpdate(const PMSG_REQ_MURUMMY_UPDATE_DS* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(PMSG_REQ_MURUMMY_UPDATE_DS);

	gQueryManager.ExecQuery(
		"UPDATE MuRummyCard "
		"SET Slot=%d,Status=%d "
		"WHERE Name='%s' AND Sequence=%d",
		lpMsg->SlotNum,
		lpMsg->Status,
		lpMsg->CharacterName,
		lpMsg->Sequence);

	gQueryManager.Close();

#endif
}

void CMuRummy::GDReqScoreUpdate(const PMSG_REQ_MURUMMY_SCORE_UPDATE_DS* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(PMSG_REQ_MURUMMY_SCORE_UPDATE_DS);

	bool exists = false;

	if (gQueryManager.ExecQuery(
		"SELECT 1 FROM MuRummyData WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		exists = (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA);
	}

	gQueryManager.Close();

	if (exists)
	{
		gQueryManager.ExecQuery(
			"UPDATE MuRummyData SET TotalScore=%d WHERE Name='%s'",
			lpMsg->Score,
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"INSERT INTO MuRummyData (Name,TotalScore) VALUES ('%s',%d)",
			lpMsg->CharacterName,
			lpMsg->Score);
	}

	gQueryManager.Close();

	for (const auto& card : lpMsg->stCardUpdateDS)
	{
		gQueryManager.ExecQuery(
			"UPDATE MuRummyCard "
			"SET Slot=%d,Status=%d "
			"WHERE Name='%s' AND Sequence=%d",
			card.SlotNum,
			card.Status,
			lpMsg->CharacterName,
			card.Seq);

		gQueryManager.Close();
	}

#endif
}

void CMuRummy::GDReqScoreDelete(const PMSG_REQ_MURUMMY_DELETE_DS* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(PMSG_REQ_MURUMMY_DELETE_DS);

	gQueryManager.ExecQuery(
		"DELETE FROM MuRummyCard WHERE Name='%s'",
		lpMsg->CharacterName);

	gQueryManager.Close();

	gQueryManager.ExecQuery(
		"DELETE FROM MuRummyData WHERE Name='%s'",
		lpMsg->CharacterName);

	gQueryManager.Close();

#endif
}

void CMuRummy::GDReqSlotInfoUpdate(const PMSG_REQ_MURUMMY_SLOTUPDATE_DS* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(PMSG_REQ_MURUMMY_SLOTUPDATE_DS);

	gQueryManager.ExecQuery(
		"UPDATE MuRummyCard "
		"SET Slot=%d,Status=%d "
		"WHERE Name='%s' AND Sequence=%d",
		lpMsg->stCardUpdateDS.SlotNum,
		lpMsg->stCardUpdateDS.Status,
		lpMsg->CharacterName,
		lpMsg->stCardUpdateDS.Seq);

	gQueryManager.Close();

#endif
}

void CMuRummy::GDReqMuRummyInfoUpdate(const PMSG_REQ_MURUMMY_INFO_UPDATE_DS* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 802)

	VALIDATE_PACKET_SIZE(PMSG_REQ_MURUMMY_INFO_UPDATE_DS);

	if (gQueryManager.ExecQuery(
		"SELECT 1 FROM MuRummyData WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		gQueryManager.Close();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			gQueryManager.ExecQuery(
				"INSERT INTO MuRummyData (Name,TotalScore) VALUES ('%s',%d)",
				lpMsg->CharacterName,
				lpMsg->Score);
		}
		else
		{
			gQueryManager.ExecQuery(
				"UPDATE MuRummyData SET TotalScore=%d WHERE Name='%s'",
				lpMsg->Score,
				lpMsg->CharacterName);
		}

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
	}

	for (const auto& card : lpMsg->stMuRummyCardUpdateDS)
	{
		gQueryManager.ExecQuery(
			"UPDATE MuRummyCard "
			"SET Slot=%d,Status=%d "
			"WHERE Name='%s' AND Sequence=%d",
			card.SlotNum,
			card.Status,
			lpMsg->CharacterName,
			card.Seq);

		gQueryManager.Close();
	}

#endif
}
