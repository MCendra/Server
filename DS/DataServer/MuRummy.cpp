// MuRummy.cpp
#include "Header.h"
#include "MuRummy.h"
#include "QueryManager.h"
#include "SocketManager.h"

CMuRummy gMuRummy;

void CMuRummy::GDReqCardInfo(_tagPMSG_REQ_MURUMMY_SELECT_DS* lpMsg, int index)
{
#if (DATASERVER_UPDATE >= 802)

	if (lpMsg == nullptr)
	{
		return;
	}

	_tagPMSG_ANS_MURUMMY_SELECT_DS pMsg{};

	pMsg.Header.set(DS_HEAD_MURUMMY, DS_SUB_MURUMMY_LOAD, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;
	pMsg.Result = 1;

	if (gQueryManager.ExecQuery(
		"SELECT TotalScore FROM MuRummyData WHERE Name='%s'",
		lpMsg->CharacterName))
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
	else
	{
		pMsg.Result = 0;
	}

	gQueryManager.Close();

	if (pMsg.Result != 0)
	{
		if (gQueryManager.ExecQuery(
			"SELECT Color,Number,Slot,Status,Sequence FROM MuRummyCard WHERE Name='%s'",
			lpMsg->CharacterName))
		{
			for (short sqlRet = gQueryManager.Fetch();
				sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA;
				sqlRet = gQueryManager.Fetch())
			{
				const int seq = gQueryManager.GetAsInteger("Sequence");

				if (seq >= 0 && seq < _countof(pMsg.stMuRummyCardInfoDS))
				{
					pMsg.stMuRummyCardInfoDS[seq].Color = gQueryManager.GetAsInteger("Color");
					pMsg.stMuRummyCardInfoDS[seq].Number = gQueryManager.GetAsInteger("Number");
					pMsg.stMuRummyCardInfoDS[seq].SlotNum = gQueryManager.GetAsInteger("Slot");
					pMsg.stMuRummyCardInfoDS[seq].Seq = seq;
					pMsg.stMuRummyCardInfoDS[seq].Status = gQueryManager.GetAsInteger("Status");
				}
			}
		}
		else
		{
			pMsg.Result = 0;
		}

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

#endif
}

void CMuRummy::GDReqCardInfoInsert(_tagPMSG_REQ_MURUMMY_INSERT_DS* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	if (lpMsg == nullptr)
	{
		return;
	}

	for (int n = 0; n < _countof(lpMsg->stMuRummyCardInfoDS); n++)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO MuRummyCard (Name,Color,Number,Slot,Status,Sequence) VALUES ('%s',%d,%d,%d,%d,%d)",
			lpMsg->CharacterName,
			lpMsg->stMuRummyCardInfoDS[n].Color,
			lpMsg->stMuRummyCardInfoDS[n].Number,
			lpMsg->stMuRummyCardInfoDS[n].SlotNum,
			lpMsg->stMuRummyCardInfoDS[n].Status,
			lpMsg->stMuRummyCardInfoDS[n].Seq);

		gQueryManager.Close();
	}

#endif
}

void CMuRummy::GDReqCardInfoUpdate(_tagPMSG_REQ_MURUMMY_UPDATE_DS* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	if (lpMsg == nullptr)
	{
		return;
	}

	gQueryManager.ExecQuery(
		"UPDATE MuRummyCard SET Slot=%d,Status=%d WHERE Name='%s' AND Sequence=%d",
		lpMsg->SlotNum,
		lpMsg->Status,
		lpMsg->CharacterName,
		lpMsg->Sequence);

	gQueryManager.Close();

#endif
}

void CMuRummy::GDReqScoreUpdate(_tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	if (lpMsg == nullptr)
	{
		return;
	}

	if (gQueryManager.ExecQuery(
		"SELECT Name FROM MuRummyData WHERE Name='%s'",
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

	for (int n = 0; n < _countof(lpMsg->stCardUpdateDS); n++)
	{
		gQueryManager.ExecQuery(
			"UPDATE MuRummyCard SET Slot=%d,Status=%d WHERE Name='%s' AND Sequence=%d",
			lpMsg->stCardUpdateDS[n].SlotNum,
			lpMsg->stCardUpdateDS[n].Status,
			lpMsg->CharacterName,
			lpMsg->stCardUpdateDS[n].Seq);

		gQueryManager.Close();
	}

#endif
}

void CMuRummy::GDReqScoreDelete(_tagPMSG_REQ_MURUMMY_DELETE_DS* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	if (lpMsg == nullptr)
	{
		return;
	}

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

void CMuRummy::GDReqSlotInfoUpdate(_tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	if (lpMsg == nullptr)
	{
		return;
	}

	gQueryManager.ExecQuery(
		"UPDATE MuRummyCard SET Slot=%d,Status=%d WHERE Name='%s' AND Sequence=%d",
		lpMsg->stCardUpdateDS.SlotNum,
		lpMsg->stCardUpdateDS.Status,
		lpMsg->CharacterName,
		lpMsg->stCardUpdateDS.Seq);

	gQueryManager.Close();

#endif
}

void CMuRummy::GDReqMuRummyInfoUpdate(_tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS* lpMsg)
{
#if (DATASERVER_UPDATE >= 802)

	if (lpMsg == nullptr)
	{
		return;
	}

	if (gQueryManager.ExecQuery(
		"SELECT Name FROM MuRummyData WHERE Name='%s'",
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

	for (int n = 0; n < _countof(lpMsg->stMuRummyCardUpdateDS); n++)
	{
		gQueryManager.ExecQuery(
			"UPDATE MuRummyCard SET Slot=%d,Status=%d WHERE Name='%s' AND Sequence=%d",
			lpMsg->stMuRummyCardUpdateDS[n].SlotNum,
			lpMsg->stMuRummyCardUpdateDS[n].Status,
			lpMsg->CharacterName,
			lpMsg->stMuRummyCardUpdateDS[n].Seq);

		gQueryManager.Close();
	}

#endif
}
