// Helper.cpp
#include "Header.h"
#include "Helper.h"
#include "QueryManager.h"
#include "SocketManager.h"

CHelper gHelper;

// Construction/Destruction

void CHelper::GDHelperDataRecv(const SDHP_HELPER_DATA_RECV* lpMsg, int index)
{
	#if(DATASERVER_UPDATE>=603)

	SDHP_HELPER_DATA_SEND pMsg;

	pMsg.Header.set(0x17,0x00,sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));
	std::memcpy(pMsg.CharacterName, lpMsg->CharacterName, sizeof(pMsg.CharacterName));

	if (!gQueryManager.ExecQuery("SELECT Data FROM HelperData WHERE Name='%s'", lpMsg->CharacterName) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;
		std::memset(pMsg.data, 0xFF, sizeof(pMsg.data));
	}
	else
	{
		pMsg.Result = 0;

		gQueryManager.GetAsBinary("Data", pMsg.data, sizeof(pMsg.data));
		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CHelper::GDHelperDataSaveRecv(const SDHP_HELPER_DATA_SAVE_RECV* lpMsg)
{
	#if(DATASERVER_UPDATE>=603)

	if(gQueryManager.ExecQuery("SELECT Name FROM HelperData WHERE Name='%s'",lpMsg->CharacterName) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->data,sizeof(lpMsg->data));
		gQueryManager.ExecQuery("INSERT INTO HelperData (Name,Data) VALUES ('%s',?)",lpMsg->CharacterName);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->data,sizeof(lpMsg->data));
		gQueryManager.ExecQuery("UPDATE HelperData SET Data=? WHERE Name='%s'",lpMsg->CharacterName);
		gQueryManager.Close();
	}

	#endif
}
