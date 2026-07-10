// Warehouse.cpp
#include "Header.h"
#include "Warehouse.h"
#include "QueryManager.h"
#include "SocketManager.h"

CWarehouse gWarehouse;

void CWarehouse::GDWarehouseItemRecv(SDHP_WAREHOUSE_ITEM_RECV* lpMsg, int index)
{
	SDHP_WAREHOUSE_ITEM_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_WAREHOUSE, DS_SUB_WAREHOUSE_PERSONAL_LOAD, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	const int warehouseItemSize = sizeof(pMsg.WarehouseItem);

	if (lpMsg->WarehouseNumber == 0)
	{
		if (!gQueryManager.ExecQuery("SELECT AccountID FROM warehouse WHERE AccountID='%s'", lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery("INSERT INTO warehouse (AccountID,Money,EndUseDate,DbVersion) VALUES ('%s',0,getdate(),3)", lpMsg->Account);

			gQueryManager.Close();

			gQueryManager.ExecQuery("UPDATE warehouse SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) WHERE AccountID='%s'", warehouseItemSize, warehouseItemSize, lpMsg->Account);

			gQueryManager.Close();

			this->DGWarehouseFreeSend(index, lpMsg->Index, lpMsg->Account);

			return;
		}
		else
		{
			gQueryManager.Close();

			if (!gQueryManager.ExecQuery("SELECT Items,Money,pw FROM warehouse WHERE AccountID='%s'", lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
			{
				gQueryManager.Close();

				memset(pMsg.WarehouseItem, 0xFF, warehouseItemSize);

				pMsg.WarehouseMoney = 0;
				pMsg.WarehousePassword = 0;
			}
			else
			{
				gQueryManager.GetAsBinary("Items", pMsg.WarehouseItem[0], warehouseItemSize);

				pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");
				pMsg.WarehousePassword = gQueryManager.GetAsInteger("pw");

				gQueryManager.Close();
			}
		}
	}
	else
	{
		if (!gQueryManager.ExecQuery("SELECT AccountID FROM ExtWarehouse WHERE AccountID='%s' AND Number=%d", lpMsg->Account, lpMsg->WarehouseNumber) || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			gQueryManager.ExecQuery("INSERT INTO ExtWarehouse (AccountID,Money,Number) VALUES ('%s',0,%d)", lpMsg->Account, lpMsg->WarehouseNumber);

			gQueryManager.Close();

			gQueryManager.ExecQuery("UPDATE ExtWarehouse SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) WHERE AccountID='%s' AND Number=%d", warehouseItemSize, warehouseItemSize, lpMsg->Account, lpMsg->WarehouseNumber);

			gQueryManager.Close();

			this->DGWarehouseFreeSend(index, lpMsg->Index, lpMsg->Account);

			return;
		}
		else
		{
			gQueryManager.Close();

			if (!gQueryManager.ExecQuery("SELECT Items,Money FROM ExtWarehouse WHERE AccountID='%s' AND Number=%d", lpMsg->Account, lpMsg->WarehouseNumber) || gQueryManager.Fetch() == SQL_NO_DATA)
			{
				gQueryManager.Close();

				memset(pMsg.WarehouseItem, 0xFF, warehouseItemSize);

				pMsg.WarehouseMoney = 0;
			}
			else
			{
				gQueryManager.GetAsBinary("Items", pMsg.WarehouseItem[0], warehouseItemSize);

				pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");

				gQueryManager.Close();

				if (!gQueryManager.ExecQuery("SELECT pw FROM warehouse WHERE AccountID='%s'", lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
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
		}
	}

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CWarehouse::GDWarehouseItemSaveRecv(SDHP_WAREHOUSE_ITEM_SAVE_RECV* lpMsg)
{
	const int warehouseItemSize = sizeof(lpMsg->WarehouseItem);

	if (lpMsg->WarehouseNumber == 0)
	{
		gQueryManager.BindParameterAsBinary(1, lpMsg->WarehouseItem[0], warehouseItemSize);

		gQueryManager.ExecQuery("UPDATE warehouse SET Items=?,Money=%d WHERE AccountID='%s'", lpMsg->WarehouseMoney, lpMsg->Account);

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE warehouse SET pw=%d WHERE AccountID='%s'", lpMsg->WarehousePassword, lpMsg->Account);

		gQueryManager.Close();
	}
	else
	{
		gQueryManager.BindParameterAsBinary(1, lpMsg->WarehouseItem[0], warehouseItemSize);

		gQueryManager.ExecQuery("UPDATE ExtWarehouse SET Items=?,Money=%d WHERE AccountID='%s' AND Number=%d", lpMsg->WarehouseMoney, lpMsg->Account, lpMsg->WarehouseNumber);

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE warehouse SET pw=%d WHERE AccountID='%s'", lpMsg->WarehousePassword, lpMsg->Account);

		gQueryManager.Close();
	}
}

void CWarehouse::DGWarehouseFreeSend(int ServerIndex, WORD index, char* account)
{
	SDHP_WAREHOUSE_FREE_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_WAREHOUSE, DS_SUB_WAREHOUSE_PERSONAL_FREE, sizeof(pMsg));

	pMsg.Index = index;

	memcpy(pMsg.Account, account, sizeof(pMsg.Account));

	gSocketManager.DataSend(ServerIndex, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);
}

void CWarehouse::GDWarehouseGuildUpdate(SDHP_WAREHOUSE_GUILD_STATUS_UPDATE_RECV* lpMsg)
{
	gQueryManager.ExecQuery("UPDATE WarehouseGuild SET StatusRender=%d WHERE Guild='%s'", lpMsg->Status, lpMsg->Name);

	gQueryManager.Close();
}

void CWarehouse::GDWarehouseGuildConsult(SDHP_WAREHOUSE_GUILD_STATUS_RECV* lpMsg, int index)
{
	SDHP_WAREHOUSE_GUILD_STATUS_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_WAREHOUSE, DS_SUB_WAREHOUSE_GUILD_STATUS, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	if (!gQueryManager.ExecQuery("SELECT StatusRender FROM WarehouseGuild WHERE Guild='%s'", lpMsg->Name) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		pMsg.Status = 0;


	}
	else
	{
		pMsg.Status = gQueryManager.GetAsInteger("StatusRender");

	}

	gQueryManager.Close();

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CWarehouse::GDWarehouseGuildItemRecv(SDHP_WAREHOUSE_ITEM_RECV* lpMsg, int index)
{
	SDHP_WAREHOUSE_ITEM_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_WAREHOUSE, DS_SUB_WAREHOUSE_GUILD_LOAD, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	const int warehouseItemSize = sizeof(pMsg.WarehouseItem);

	if (!gQueryManager.ExecQuery("SELECT Guild FROM WarehouseGuild WHERE Guild='%s'", lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO WarehouseGuild (Guild,Money,EndUseDate,DbVersion) VALUES ('%s',0,getdate(),3)", lpMsg->Account);

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE WarehouseGuild SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) WHERE Guild='%s'", warehouseItemSize, warehouseItemSize, lpMsg->Account);

		gQueryManager.Close();

		this->DGWarehouseGuildFreeSend(index, lpMsg->Index, lpMsg->Account);

		return;
	}

	gQueryManager.Close();

	if (!gQueryManager.ExecQuery("SELECT Items,Money,pw FROM WarehouseGuild WHERE Guild='%s'", lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		memset(pMsg.WarehouseItem, 0xFF, warehouseItemSize);

		pMsg.WarehouseMoney = 0;

		pMsg.WarehousePassword = 0;
	}
	else
	{
		gQueryManager.GetAsBinary("Items", pMsg.WarehouseItem[0], warehouseItemSize);

		pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");

		pMsg.WarehousePassword = gQueryManager.GetAsInteger("pw");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));
}

void CWarehouse::GDWarehouseGuildItemSaveRecv(SDHP_WAREHOUSE_ITEM_SAVE_RECV* lpMsg)
{
	const int warehouseItemSize = sizeof(lpMsg->WarehouseItem);

	gQueryManager.BindParameterAsBinary(1, lpMsg->WarehouseItem[0], warehouseItemSize);

	gQueryManager.ExecQuery("UPDATE WarehouseGuild SET Items=?,Money=%d WHERE Guild='%s'", lpMsg->WarehouseMoney, lpMsg->Account);

	gQueryManager.Close();

	gQueryManager.ExecQuery("UPDATE WarehouseGuild SET pw=%d WHERE Guild='%s'", lpMsg->WarehousePassword, lpMsg->Account);

	gQueryManager.Close();
}

void CWarehouse::DGWarehouseGuildFreeSend(int ServerIndex, WORD index, char* account)
{
	SDHP_WAREHOUSE_FREE_SEND pMsg{};

	pMsg.Header.set(DS_HEAD_WAREHOUSE, DS_SUB_WAREHOUSE_GUILD_FREE, sizeof(pMsg));

	pMsg.Index = index;

	memcpy(pMsg.Account, account, sizeof(pMsg.Account));

	gSocketManager.DataSend(ServerIndex, reinterpret_cast<BYTE*>(&pMsg), pMsg.Header.size);
}
