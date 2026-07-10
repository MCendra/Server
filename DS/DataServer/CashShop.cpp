// CashShop.cpp
#include "Header.h"
#include "CashShop.h"
#include "QueryManager.h"
#include "SocketManager.h"

CCashShop gCashShop;

void CCashShop::GDCashShopPointRecv(SDHP_CASH_SHOP_POINT_RECV* lpMsg, int index) // OK
{
	#if (DATASERVER_UPDATE >= 501)

		SDHP_CASH_SHOP_POINT_SEND pMsg{};

		pMsg.Header.set(0x18, 0x00, sizeof(pMsg));

		pMsg.Index = lpMsg->Index;

		memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

		pMsg.Result = 0;

		if (LoadCashInfo(lpMsg->Account, pMsg.WCoinC, pMsg.WCoinP, pMsg.GoblinPoint, pMsg.Ruud) == false)
		{
			if (CreateCashInfo(lpMsg->Account) == false)
			{
				pMsg.Result = 1;
			}
		}

		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

	#else

		SDHP_CASH_SHOP_POINT_SEND pMsg;

		pMsg.Header.set(0x18,0x00,sizeof(pMsg));

		pMsg.Index = lpMsg->Index;

		memcpy(pMsg.Account,lpMsg->Account,sizeof(pMsg.Account));

		pMsg.Result = 1;

		if(!gQueryManager.ExecQuery("EXEC WZ_GetCoin '%s'",lpMsg->Account) || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			pMsg.WCoinC = 0;
			pMsg.WCoinP = 0;
			pMsg.GoblinPoint = 0;
			gQueryManager.Close();
		}
		else
		{
			pMsg.WCoinC = gQueryManager.GetResult(0);
			pMsg.WCoinP = gQueryManager.GetResult(1);
			pMsg.GoblinPoint = gQueryManager.GetResult(2);
			gQueryManager.Close();
		}
		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CCashShop::GDCashShopItemBuyRecv(SDHP_CASH_SHOP_ITEM_BUY_RECV* lpMsg, int index)
{
	#if (DATASERVER_UPDATE >= 501)

		SDHP_CASH_SHOP_ITEM_BUY_SEND pMsg{};

		pMsg.Header.set(0x18, 0x01, sizeof(pMsg));

		pMsg.Index = lpMsg->Index;

		memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

		pMsg.Result = 0;

		pMsg.PackageMainIndex = lpMsg->PackageMainIndex;
		pMsg.Category = lpMsg->Category;
		pMsg.ProductMainIndex = lpMsg->ProductMainIndex;
		pMsg.ItemIndex = lpMsg->ItemIndex;
		pMsg.CoinIndex = lpMsg->CoinIndex;
		pMsg.MileageFlag = lpMsg->MileageFlag;

		if (!LoadCashInfo(lpMsg->Account, pMsg.WCoinC, pMsg.WCoinP, pMsg.GoblinPoint))
		{
			pMsg.Result = 1;
		}
		else if (!gQueryManager.ExecQuery("SELECT count(*) FROM CashShopInventory WHERE AccountID='%s' AND InventoryType=%d", lpMsg->Account, 83) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.Result = 1;
		}
		else
		{
			pMsg.ItemCount = gQueryManager.GetResult(0);

			gQueryManager.Close();
		}

		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

	#endif
}

void CCashShop::GDCashShopItemGifRecv(SDHP_CASH_SHOP_ITEM_GIF_RECV* lpMsg, int index)
{
	#if (DATASERVER_UPDATE >= 501)

		SDHP_CASH_SHOP_ITEM_GIF_SEND pMsg{};

		pMsg.Header.set(0x18, 0x02, sizeof(pMsg));

		pMsg.Index = lpMsg->Index;

		memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

		pMsg.Result = 0;

		pMsg.PackageMainIndex = lpMsg->PackageMainIndex;
		pMsg.Category = lpMsg->Category;
		pMsg.ProductMainIndex = lpMsg->ProductMainIndex;
		pMsg.SaleZone = lpMsg->SaleZone;
		pMsg.ItemIndex = lpMsg->ItemIndex;
		pMsg.CoinIndex = lpMsg->CoinIndex;
		pMsg.MileageFlag = lpMsg->MileageFlag;

		memcpy(pMsg.GiftName, lpMsg->GiftName, sizeof(pMsg.GiftName));
		memcpy(pMsg.GiftText, lpMsg->GiftText, sizeof(pMsg.GiftText));

		gQueryManager.BindParameterAsString(1, lpMsg->GiftName, sizeof(lpMsg->GiftName));

		if (!gQueryManager.ExecQuery("SELECT AccountID FROM Character WHERE Name=?") ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.Result = 1;
		}
		else
		{
			gQueryManager.GetAsString("AccountID", pMsg.GiftAccount, sizeof(pMsg.GiftAccount));

			gQueryManager.Close();

			if (!LoadCashInfo(lpMsg->Account, pMsg.WCoinC, pMsg.WCoinP, pMsg.GoblinPoint))
			{
				pMsg.Result = 1;
			}
			else if (!gQueryManager.ExecQuery("SELECT count(*) FROM CashShopInventory WHERE AccountID='%s' AND InventoryType=%d", pMsg.GiftAccount, 71) ||
				gQueryManager.Fetch() == SQL_NO_DATA)
			{
				gQueryManager.Close();

				pMsg.Result = 1;
			}
			else
			{
				pMsg.ItemCount = gQueryManager.GetResult(0);

				gQueryManager.Close();
			}
		}

		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

	#endif
}

void CCashShop::GDCashShopItemNumRecv(SDHP_CASH_SHOP_ITEM_NUM_RECV* lpMsg, int index)
{
	#if (DATASERVER_UPDATE >= 501)

		SDHP_CASH_SHOP_ITEM_NUM_SEND pMsg{};

		pMsg.Header.set(0x18, 0x03, sizeof(pMsg));

		pMsg.Index = lpMsg->Index;

		memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

		pMsg.Result = 0;

		pMsg.InventoryPage = lpMsg->InventoryPage;
		pMsg.InventoryType = lpMsg->InventoryType;

		if (!gQueryManager.ExecQuery("SELECT * FROM CashShopInventory WHERE AccountID='%s' AND InventoryType=%d", lpMsg->Account, lpMsg->InventoryType))
		{
			gQueryManager.Close();

			pMsg.Result = 1;
		}
		else
		{
			while (gQueryManager.Fetch() != SQL_NO_DATA)
			{
				if ((((pMsg.ItemCount++) / MAX_CASH_SHOP_PAGE_ITEM) + 1) == lpMsg->InventoryPage)
				{
					auto& Item = pMsg.ProductInfo[pMsg.PageCount];

					Item.BaseItemCode = gQueryManager.GetAsInteger("BaseItemCode");
					Item.MainItemCode = gQueryManager.GetAsInteger("MainItemCode");
					Item.PackageMainIndex = gQueryManager.GetAsInteger("PackageMainIndex");
					Item.ProductBaseIndex = gQueryManager.GetAsInteger("ProductBaseIndex");
					Item.ProductMainIndex = gQueryManager.GetAsInteger("ProductMainIndex");
					Item.CoinValue = gQueryManager.GetAsFloat("CoinValue");
					Item.ProductType = gQueryManager.GetAsInteger("ProductType");

					gQueryManager.GetAsString("GiftName", Item.GiftName, sizeof(Item.GiftName));
					gQueryManager.GetAsString("GiftText", Item.GiftText, sizeof(Item.GiftText));

					pMsg.PageCount++;
				}
			}

			gQueryManager.Close();
		}

		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

	#endif
}

void CCashShop::GDCashShopItemUseRecv(SDHP_CASH_SHOP_ITEM_USE_RECV* lpMsg, int index)
{
	#if (DATASERVER_UPDATE >= 501)

		SDHP_CASH_SHOP_ITEM_USE_SEND pMsg{};

		pMsg.Header.set(0x18, 0x04, sizeof(pMsg));

		pMsg.Index = lpMsg->Index;

		memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

		pMsg.Result = 0;

		pMsg.BaseItemCode = lpMsg->BaseItemCode;
		pMsg.MainItemCode = lpMsg->MainItemCode;
		pMsg.ItemIndex = lpMsg->ItemIndex;
		pMsg.ProductType = lpMsg->ProductType;

		if (!gQueryManager.ExecQuery("SELECT * FROM CashShopInventory WHERE BaseItemCode=%d", lpMsg->BaseItemCode) ||
			gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.Result = 1;
		}
		else
		{
			auto& Product = pMsg.ProductInfo;

			Product.BaseItemCode = gQueryManager.GetAsInteger("BaseItemCode");
			Product.MainItemCode = gQueryManager.GetAsInteger("MainItemCode");
			Product.PackageMainIndex = gQueryManager.GetAsInteger("PackageMainIndex");
			Product.ProductBaseIndex = gQueryManager.GetAsInteger("ProductBaseIndex");
			Product.ProductMainIndex = gQueryManager.GetAsInteger("ProductMainIndex");
			Product.CoinValue = gQueryManager.GetAsFloat("CoinValue");
			Product.ProductType = gQueryManager.GetAsInteger("ProductType");

			gQueryManager.GetAsString("GiftName", Product.GiftName, sizeof(Product.GiftName));
			gQueryManager.GetAsString("GiftText", Product.GiftText, sizeof(Product.GiftText));

			gQueryManager.Close();
		}

		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

	#endif
}

void CCashShop::GDCashShopPeriodicItemRecv(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV* lpMsg, int index)
{
	#if (DATASERVER_UPDATE >= 501)

		BYTE send[4096];

		SDHP_CASH_SHOP_PERIODIC_ITEM_SEND pMsg{};

		pMsg.Header.set(0x18, 0x05, 0);

		int size = sizeof(pMsg);

		pMsg.Index = lpMsg->Index;

		memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

		pMsg.Count = 0;

		for (int n = 0; n < lpMsg->Count; n++)
		{
			if ((size + sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM2)) > sizeof(send))
			{
				break;
			}

			auto* lpInfo = reinterpret_cast<SDHP_CASH_SHOP_PERIODIC_ITEM1*>(
				((BYTE*)lpMsg) + sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV) + (sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM1) * n));

			SDHP_CASH_SHOP_PERIODIC_ITEM2 info{};

			if (!gQueryManager.ExecQuery("SELECT Time FROM CashShopPeriodicItem WHERE ItemSerial=%d", lpInfo->Serial) ||
				gQueryManager.Fetch() == SQL_NO_DATA)
			{
				gQueryManager.Close();

				gQueryManager.ExecQuery(
					"INSERT INTO CashShopPeriodicItem (ItemSerial,Time) VALUES (%d,%d)",
					lpInfo->Serial,
					0);

				gQueryManager.Close();

				info.Time = 0;
			}
			else
			{
				info.Time = gQueryManager.GetAsInteger("Time");

				gQueryManager.Close();
			}

			info.Slot = lpInfo->Slot;
			info.Serial = lpInfo->Serial;

			memcpy(&send[size], &info, sizeof(info));

			size += sizeof(info);

			pMsg.Count++;
		}

		pMsg.Header.size[0] = SET_NUMBERHB(size);
		pMsg.Header.size[1] = SET_NUMBERLB(size);

		memcpy(send, &pMsg, sizeof(pMsg));

		gSocketManager.DataSend(index, send, size);

	#endif
}

void CCashShop::GDCashShopRecievePointRecv(SDHP_CASH_SHOP_RECIEVE_POINT_RECV* lpMsg, int index)
{
	#if (DATASERVER_UPDATE >= 501)

		SDHP_CASH_SHOP_RECIEVE_POINT_SEND pMsg{};

		pMsg.Header.set(0x18, 0x06, sizeof(pMsg));

		pMsg.Index = lpMsg->Index;

		memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

		pMsg.CallbackFunc = lpMsg->CallbackFunc;
		pMsg.CallbackArg1 = lpMsg->CallbackArg1;
		pMsg.CallbackArg2 = lpMsg->CallbackArg2;

		LoadCashInfo(lpMsg->Account, pMsg.WCoinC, pMsg.WCoinP, pMsg.GoblinPoint);

		gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));

	#endif
}

void CCashShop::GDCashShopAddPointSaveRecv(SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV* lpMsg)
{
	#if (DATASERVER_UPDATE >= 501)

		char TargetAccount[11];

		memcpy(TargetAccount, (lpMsg->GiftAccount[0] == 0) ? lpMsg->Account : lpMsg->GiftAccount, sizeof(TargetAccount));

		DWORD WCoinC = 0;
		DWORD WCoinP = 0;
		DWORD GoblinPoint = 0;
		DWORD Ruud = 0;

		if (!LoadCashInfo(TargetAccount, WCoinC, WCoinP, GoblinPoint, Ruud))
		{
			if (!CreateCashInfo(TargetAccount))
			{
				return;
			}

			WCoinC = 0;
			WCoinP = 0;
			GoblinPoint = 0;
			Ruud = 0;
		}

		gQueryManager.ExecQuery(
			"UPDATE CashShopData SET WCoinC=%d,WCoinP=%d,GoblinPoint=%d,Ruud=%d WHERE AccountID='%s'",
			(((WCoinC + lpMsg->AddWCoinC) > 0x7FFFFFFF) ? 0x7FFFFFFF : (WCoinC + lpMsg->AddWCoinC)),
			(((WCoinP + lpMsg->AddWCoinP) > 0x7FFFFFFF) ? 0x7FFFFFFF : (WCoinP + lpMsg->AddWCoinP)),
			(((GoblinPoint + lpMsg->AddGoblinPoint) > 0x7FFFFFFF) ? 0x7FFFFFFF : (GoblinPoint + lpMsg->AddGoblinPoint)),
			(((Ruud + lpMsg->AddRuud) > 0x7FFFFFFF) ? 0x7FFFFFFF : (Ruud + lpMsg->AddRuud)),
			TargetAccount);

		gQueryManager.Close();

	#endif
}

void CCashShop::GDCashShopSubPointSaveRecv(SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV* lpMsg)
{
	#if (DATASERVER_UPDATE >= 501)

		char TargetAccount[11];

		memcpy(TargetAccount, (lpMsg->GiftAccount[0] == 0) ? lpMsg->Account : lpMsg->GiftAccount, sizeof(TargetAccount));

		DWORD WCoinC = 0;
		DWORD WCoinP = 0;
		DWORD GoblinPoint = 0;
		DWORD Ruud = 0;

		if (!LoadCashInfo(TargetAccount, WCoinC, WCoinP, GoblinPoint, Ruud))
		{
			if (!CreateCashInfo(TargetAccount))
			{
				return;
			}

			WCoinC = 0;
			WCoinP = 0;
			GoblinPoint = 0;
			Ruud = 0;
		}

		gQueryManager.ExecQuery(
			"UPDATE CashShopData SET WCoinC=%d,WCoinP=%d,GoblinPoint=%d,Ruud=%d WHERE AccountID='%s'",
			((lpMsg->SubWCoinC > WCoinC) ? 0 : (WCoinC - lpMsg->SubWCoinC)),
			((lpMsg->SubWCoinP > WCoinP) ? 0 : (WCoinP - lpMsg->SubWCoinP)),
			((lpMsg->SubGoblinPoint > GoblinPoint) ? 0 : (GoblinPoint - lpMsg->SubGoblinPoint)),
			((lpMsg->SubRuud > Ruud) ? 0 : (Ruud - lpMsg->SubRuud)),
			TargetAccount);

		gQueryManager.Close();

		if (lpMsg->SubWCoinC > 0)
		{
			gQueryManager.ExecQuery(
				"INSERT INTO LOG_CREDITOS (login,valor,tipo) VALUES ('%s',%d,%d)",
				TargetAccount,
				((lpMsg->SubWCoinC > WCoinC) ? 0 : (-lpMsg->SubWCoinC)),
				4);

			gQueryManager.Close();
		}

	#endif
}

void CCashShop::GDCashShopInsertItemSaveRecv(SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 501)

	char TargetAccount[11];

	memcpy(TargetAccount, (lpMsg->GiftAccount[0] == 0) ? lpMsg->Account : lpMsg->GiftAccount, sizeof(TargetAccount));

	gQueryManager.BindParameterAsString(1, lpMsg->GiftName, sizeof(lpMsg->GiftName));
	gQueryManager.BindParameterAsString(2, lpMsg->GiftText, sizeof(lpMsg->GiftText));

	gQueryManager.ExecQuery(
		"INSERT INTO CashShopInventory "
		"(MainItemCode,AccountID,InventoryType,PackageMainIndex,ProductBaseIndex,ProductMainIndex,CoinValue,ProductType,GiftName,GiftText) "
		"VALUES (%d,'%s',%d,%d,%d,%d,%f,%d,?,?)",
		0,
		TargetAccount,
		lpMsg->InventoryType,
		lpMsg->PackageMainIndex,
		lpMsg->ProductBaseIndex,
		lpMsg->ProductMainIndex,
		lpMsg->CoinValue,
		lpMsg->ProductType);

	gQueryManager.Close();

#endif
}

void CCashShop::GDCashShopDeleteItemSaveRecv(SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV* lpMsg)
{
	#if (DATASERVER_UPDATE >= 501)

		gQueryManager.ExecQuery(
			"DELETE FROM CashShopInventory WHERE BaseItemCode=%d",
			lpMsg->BaseItemCode);

		gQueryManager.Close();

	#endif
}

void CCashShop::GDCashShopPeriodicItemSaveRecv(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV* lpMsg)
{
#if (DATASERVER_UPDATE >= 501)

	for (int n = 0; n < lpMsg->Count; n++)
	{
		auto* lpInfo = reinterpret_cast<SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE*>(
			((BYTE*)lpMsg) +
			sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV) +
			(sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE) * n));

		gQueryManager.ExecQuery(
			"IF EXISTS (SELECT 1 FROM CashShopPeriodicItem WHERE ItemSerial=%d) "
			"UPDATE CashShopPeriodicItem SET Time=%d WHERE ItemSerial=%d "
			"ELSE "
			"INSERT INTO CashShopPeriodicItem (ItemSerial,Time) VALUES (%d,%d)",
			lpInfo->Serial,
			lpInfo->Time,
			lpInfo->Serial,
			lpInfo->Serial,
			lpInfo->Time);

		gQueryManager.Close();
	}

#endif
}

// Fix: Helpers
bool CCashShop::LoadCashInfo(const char* account, DWORD& WCoinC, DWORD& WCoinP,	DWORD& GoblinPoint)
{
	DWORD dummyRuud = 0;

	return LoadCashInfo(account, WCoinC, WCoinP, GoblinPoint, dummyRuud);
}

bool CCashShop::LoadCashInfo(const char* account, DWORD& WCoinC, DWORD& WCoinP, DWORD& GoblinPoint, DWORD& Ruud)
{
	WCoinC = 0;
	WCoinP = 0;
	GoblinPoint = 0;
	Ruud = 0;

	if (!gQueryManager.ExecQuery("SELECT WCoinC,WCoinP,GoblinPoint,Ruud FROM CashShopData WHERE AccountID='%s'", account))
	{
		gQueryManager.Close();
		return false;
	}

	if (gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		return false;
	}

	WCoinC = static_cast<DWORD>(gQueryManager.GetAsInteger("WCoinC"));
	WCoinP = static_cast<DWORD>(gQueryManager.GetAsInteger("WCoinP"));
	GoblinPoint = static_cast<DWORD>(gQueryManager.GetAsInteger("GoblinPoint"));
	Ruud = static_cast<DWORD>(gQueryManager.GetAsInteger("Ruud"));

	gQueryManager.Close();

	return true;
}

bool CCashShop::CreateCashInfo(const char* account)
{
	if (!gQueryManager.ExecQuery("INSERT INTO CashShopData (AccountID,WCoinC,WCoinP,GoblinPoint,Ruud) VALUES ('%s',0,0,0,0)", account))
	{
		gQueryManager.Close();
		return false;
	}

	gQueryManager.Close();
	return true;
}
