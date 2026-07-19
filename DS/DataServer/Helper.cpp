// Helper.cpp
#include "Header.h"
#include "Helper.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CHelper gHelper;

// Construction/Destruction

void CHelper::GDHelperDataRecv(const SDHP_HELPER_DATA_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 603)

	VALIDATE_PACKET_SIZE(SDHP_HELPER_DATA_RECV);

	SDHP_HELPER_DATA_SEND pMsg{};

	pMsg.Header.set(0x17, 0x00, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	pMsg.Result = 1;
	std::memset(pMsg.Data, 0xFF, sizeof(pMsg.Data));

	if (gQueryManager.ExecQuery(
		"SELECT Data FROM HelperData WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.Result = 0;

			gQueryManager.GetAsBinary(
				"Data",
				pMsg.Data,
				sizeof(pMsg.Data));
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CHelper::GDHelperDataSaveRecv(const SDHP_HELPER_DATA_SAVE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 603)

	VALIDATE_PACKET_SIZE(SDHP_HELPER_DATA_SAVE_RECV);

	if (!gQueryManager.ExecQuery(
		"SELECT 1 FROM HelperData WHERE Name='%s'",
		lpMsg->CharacterName))
	{
		gQueryManager.Close();
		return;
	}

	const auto sqlRet = gQueryManager.Fetch();

	gQueryManager.Close();

	gQueryManager.BindParameterAsBinary(
		1,
		lpMsg->Data,
		sizeof(lpMsg->Data));

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.ExecQuery(
			"INSERT INTO HelperData (Name,Data) VALUES ('%s',?)",
			lpMsg->CharacterName);
	}
	else
	{
		gQueryManager.ExecQuery(
			"UPDATE HelperData SET Data=? WHERE Name='%s'",
			lpMsg->CharacterName);
	}

	gQueryManager.Close();

#endif
}
