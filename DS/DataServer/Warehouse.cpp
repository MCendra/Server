// Warehouse.cpp
#include "Header.h"
#include "Warehouse.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CWarehouse gWarehouse;

void CWarehouse::GDWarehouseItemRecv(const SDHP_WAREHOUSE_ITEM_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_WAREHOUSE_ITEM_RECV);

	SDHP_WAREHOUSE_ITEM_SEND pMsg{};
	pMsg.Header.set(HEAD_WAREHOUSE, SUB_WAREHOUSE_PERSONAL_LOAD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	const int warehouseItemSize = sizeof(pMsg.WarehouseItem);

	if (lpMsg->WarehouseNumber == 0)
	{
		if (!gQueryManager.ExecQuery(
			"SELECT Items,Money,pw FROM warehouse WHERE AccountID='%s'",
			lpMsg->Account) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"INSERT INTO warehouse (AccountID,Money,EndUseDate,DbVersion) "
				"VALUES ('%s',0,getdate(),3)",
				lpMsg->Account);

			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"UPDATE warehouse SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) "
				"WHERE AccountID='%s'",
				warehouseItemSize,
				warehouseItemSize,
				lpMsg->Account);

			gQueryManager.Close();

			DGWarehouseFreeSend(lpMsg->Index, serverIndex, lpMsg->Account);
			return;
		}

		gQueryManager.GetAsBinary("Items", pMsg.WarehouseItem[0], warehouseItemSize);

		pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");
		pMsg.WarehousePassword = gQueryManager.GetAsInteger("pw");

		gQueryManager.Close();
	}
	else
	{
		if (!gQueryManager.ExecQuery(
			"SELECT Items,Money FROM ExtWarehouse WHERE AccountID='%s' AND Number=%d",
			lpMsg->Account,
			lpMsg->WarehouseNumber) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"INSERT INTO ExtWarehouse (AccountID,Money,Number) "
				"VALUES ('%s',0,%d)",
				lpMsg->Account,
				lpMsg->WarehouseNumber);

			gQueryManager.Close();

			gQueryManager.ExecQuery(
				"UPDATE ExtWarehouse SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) "
				"WHERE AccountID='%s' AND Number=%d",
				warehouseItemSize,
				warehouseItemSize,
				lpMsg->Account,
				lpMsg->WarehouseNumber);

			gQueryManager.Close();

			DGWarehouseFreeSend(lpMsg->Index, serverIndex, lpMsg->Account);
			return;
		}

		gQueryManager.GetAsBinary("Items", pMsg.WarehouseItem[0], warehouseItemSize);

		pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");

		gQueryManager.Close();

		if (!gQueryManager.ExecQuery(
			"SELECT pw FROM warehouse WHERE AccountID='%s'",
			lpMsg->Account) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.WarehousePassword = 0;
		}
		else
		{
			pMsg.WarehousePassword = gQueryManager.GetAsInteger("pw");

			gQueryManager.Close();
		}
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CWarehouse::GDWarehouseItemSaveRecv(const SDHP_WAREHOUSE_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_WAREHOUSE_ITEM_SAVE_RECV);

	const int warehouseItemSize = sizeof(lpMsg->WarehouseItem);

	gQueryManager.BindParameterAsBinary(1, lpMsg->WarehouseItem[0], warehouseItemSize);

	if (lpMsg->WarehouseNumber == 0)
	{
		gQueryManager.ExecQuery(
			"UPDATE warehouse SET Items=?,Money=%d,pw=%d WHERE AccountID='%s'",
			lpMsg->WarehouseMoney,
			lpMsg->WarehousePassword,
			lpMsg->Account);

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.ExecQuery(
			"UPDATE ExtWarehouse SET Items=?,Money=%d WHERE AccountID='%s' AND Number=%d",
			lpMsg->WarehouseMoney,
			lpMsg->Account,
			lpMsg->WarehouseNumber);

		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"UPDATE warehouse SET pw=%d WHERE AccountID='%s'",
			lpMsg->WarehousePassword,
			lpMsg->Account);

		gQueryManager.Close();
	}
}

void CWarehouse::GDWarehouseGuildItemRecv(const SDHP_WAREHOUSE_ITEM_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_WAREHOUSE_ITEM_RECV);

	SDHP_WAREHOUSE_ITEM_SEND pMsg{};
	pMsg.Header.set(HEAD_WAREHOUSE, SUB_WAREHOUSE_GUILD_LOAD, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	const int warehouseItemSize = sizeof(pMsg.WarehouseItem);

	if (!gQueryManager.ExecQuery(
		"SELECT Items,Money,pw FROM WarehouseGuild WHERE Guild='%s'",
		lpMsg->Account) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO WarehouseGuild (Guild,Money,EndUseDate,DbVersion) "
			"VALUES ('%s',0,getdate(),3)",
			lpMsg->Account);

		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"UPDATE WarehouseGuild "
			"SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) "
			"WHERE Guild='%s'",
			warehouseItemSize,
			warehouseItemSize,
			lpMsg->Account);

		gQueryManager.Close();

		DGWarehouseGuildFreeSend(lpMsg->Index, serverIndex, lpMsg->Account);
		return;
	}

	gQueryManager.GetAsBinary("Items", pMsg.WarehouseItem[0], warehouseItemSize);

	pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");
	pMsg.WarehousePassword = gQueryManager.GetAsInteger("pw");

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CWarehouse::GDWarehouseGuildItemSaveRecv(const SDHP_WAREHOUSE_ITEM_SAVE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_WAREHOUSE_ITEM_SAVE_RECV);

	const int warehouseItemSize = sizeof(lpMsg->WarehouseItem);

	gQueryManager.BindParameterAsBinary(1, lpMsg->WarehouseItem[0], warehouseItemSize);

	gQueryManager.ExecQuery(
		"UPDATE WarehouseGuild SET Items=?,Money=%d,pw=%d WHERE Guild='%s'",
		lpMsg->WarehouseMoney,
		lpMsg->WarehousePassword,
		lpMsg->Account);

	gQueryManager.Close();
}

void CWarehouse::GDWarehouseGuildConsult(const SDHP_WAREHOUSE_GUILD_STATUS_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_WAREHOUSE_GUILD_STATUS_RECV);

	SDHP_WAREHOUSE_GUILD_STATUS_SEND pMsg{};
	pMsg.Header.set(HEAD_WAREHOUSE, SUB_WAREHOUSE_GUILD_STATUS, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	if (gQueryManager.ExecQuery(
		"SELECT StatusRender FROM WarehouseGuild WHERE Guild='%s'",
		lpMsg->GuildName) &&
		gQueryManager.Fetch() != SQL_NO_DATA)
	{
		pMsg.Status = gQueryManager.GetAsInteger("StatusRender");
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CWarehouse::GDWarehouseGuildUpdate(const SDHP_WAREHOUSE_GUILD_STATUS_UPDATE_RECV* lpMsg, int serverIndex, int size)
{
	VALIDATE_PACKET_SIZE(SDHP_WAREHOUSE_GUILD_STATUS_UPDATE_RECV);

	gQueryManager.ExecQuery(
		"UPDATE WarehouseGuild SET StatusRender=%d WHERE Guild='%s'",
		lpMsg->Status,
		lpMsg->GuildName);

	gQueryManager.Close();
}

void CWarehouse::DGWarehouseFreeSend(WORD index, int serverIndex, const char* account)
{
	SDHP_WAREHOUSE_FREE_SEND pMsg{};
	pMsg.Header.set(HEAD_WAREHOUSE, SUB_WAREHOUSE_PERSONAL_FREE, sizeof(pMsg));
	pMsg.Index = index;

	std::memcpy(pMsg.Account, account, sizeof(pMsg.Account));

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CWarehouse::DGWarehouseGuildFreeSend(WORD index, int serverIndex, const char* account)
{
	SDHP_WAREHOUSE_FREE_SEND pMsg{};
	pMsg.Header.set(HEAD_WAREHOUSE, SUB_WAREHOUSE_GUILD_FREE, sizeof(pMsg));
	pMsg.Index = index;

	std::memcpy(pMsg.Account, account, sizeof(pMsg.Account));

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}
