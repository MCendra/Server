// CashShop.cpp
#include "Header.h"
#include "CashShop.h"
#include "Log.h"
#include "QueryManager.h"
#include "SocketManager.h"

CCashShop gCashShop;

void CCashShop::GDCashShopPointRecv(const SDHP_CASH_SHOP_POINT_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_POINT_RECV);

	SDHP_CASH_SHOP_POINT_SEND pMsg{};

	pMsg.Header.set(HEAD_CASHSHOP, SUB_CASHSHOP_POINT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	if (!LoadCashInfo(
		lpMsg->Account,
		pMsg.WCoinC,
		pMsg.WCoinP,
		pMsg.GoblinPoint,
		pMsg.Ruud) &&
		!CreateCashInfo(lpMsg->Account))
	{
		pMsg.Result = 1;
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#else

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_POINT_RECV);

	SDHP_CASH_SHOP_POINT_SEND pMsg{};

	pMsg.Header.set(HEAD_CASHSHOP, 0x00, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.Result = 1;

	if (gQueryManager.ExecQuery(
		"EXEC WZ_GetCoin '%s'",
		lpMsg->Account))
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
		{
			pMsg.WCoinC = gQueryManager.GetResult(0);
			pMsg.WCoinP = gQueryManager.GetResult(1);
			pMsg.GoblinPoint = gQueryManager.GetResult(2);
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CCashShop::GDCashShopItemBuyRecv(const SDHP_CASH_SHOP_ITEM_BUY_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_ITEM_BUY_RECV);

	SDHP_CASH_SHOP_ITEM_BUY_SEND pMsg{};

	pMsg.Header.set(HEAD_CASHSHOP, 0x01, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.PackageMainIndex = lpMsg->PackageMainIndex;
	pMsg.Category = lpMsg->Category;
	pMsg.ProductMainIndex = lpMsg->ProductMainIndex;
	pMsg.ItemIndex = lpMsg->ItemIndex;
	pMsg.CoinIndex = lpMsg->CoinIndex;
	pMsg.MileageFlag = lpMsg->MileageFlag;

	if (!LoadCashInfo(
		lpMsg->Account,
		pMsg.WCoinC,
		pMsg.WCoinP,
		pMsg.GoblinPoint))
	{
		pMsg.Result = 1;
	}
	else if (!gQueryManager.ExecQuery(
		"SELECT COUNT(*) FROM CashShopInventory "
		"WHERE AccountID='%s' AND InventoryType=%d",
		lpMsg->Account,
		83))
	{
		pMsg.Result = 1;
	}
	else
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			pMsg.Result = 1;
		}
		else
		{
			pMsg.ItemCount = gQueryManager.GetResult(0);
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CCashShop::GDCashShopItemGifRecv(const SDHP_CASH_SHOP_ITEM_GIF_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_ITEM_GIF_RECV);

	SDHP_CASH_SHOP_ITEM_GIF_SEND pMsg{};

	pMsg.Header.set(HEAD_CASHSHOP, SUB_CASHSHOP_ITEM_GIFT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.PackageMainIndex = lpMsg->PackageMainIndex;
	pMsg.Category = lpMsg->Category;
	pMsg.ProductMainIndex = lpMsg->ProductMainIndex;
	pMsg.SaleZone = lpMsg->SaleZone;
	pMsg.ItemIndex = lpMsg->ItemIndex;
	pMsg.CoinIndex = lpMsg->CoinIndex;
	pMsg.MileageFlag = lpMsg->MileageFlag;

	std::memcpy(pMsg.GiftName, lpMsg->GiftName, sizeof(pMsg.GiftName));
	std::memcpy(pMsg.GiftText, lpMsg->GiftText, sizeof(pMsg.GiftText));

	gQueryManager.BindParameterAsString(
		1,
		lpMsg->GiftName,
		sizeof(lpMsg->GiftName));

	if (!gQueryManager.ExecQuery(
		"SELECT AccountID FROM Character WHERE Name=?"))
	{
		pMsg.Result = 1;
	}
	else
	{
		const auto sqlRet = gQueryManager.Fetch();

		if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
		{
			pMsg.Result = 1;
		}
		else
		{
			gQueryManager.GetAsString(
				"AccountID",
				pMsg.GiftAccount,
				sizeof(pMsg.GiftAccount));
		}
	}

	gQueryManager.Close();

	if (pMsg.Result == 0)
	{
		if (!LoadCashInfo(
			lpMsg->Account,
			pMsg.WCoinC,
			pMsg.WCoinP,
			pMsg.GoblinPoint))
		{
			pMsg.Result = 1;
		}
		else if (!gQueryManager.ExecQuery(
			"SELECT COUNT(*) FROM CashShopInventory "
			"WHERE AccountID='%s' AND InventoryType=%d",
			pMsg.GiftAccount,
			71))
		{
			pMsg.Result = 1;
		}
		else
		{
			const auto sqlRet = gQueryManager.Fetch();

			if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
			{
				pMsg.Result = 1;
			}
			else
			{
				pMsg.ItemCount = gQueryManager.GetResult(0);
			}
		}

		gQueryManager.Close();
	}

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CCashShop::GDCashShopItemNumRecv(const SDHP_CASH_SHOP_ITEM_NUM_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_ITEM_NUM_RECV);

	SDHP_CASH_SHOP_ITEM_NUM_SEND pMsg{};

	pMsg.Header.set(HEAD_CASHSHOP, SUB_CASHSHOP_ITEM_LIST, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.InventoryPage = lpMsg->InventoryPage;
	pMsg.InventoryType = lpMsg->InventoryType;

	if (!gQueryManager.ExecQuery(
		"SELECT * FROM CashShopInventory "
		"WHERE AccountID='%s' AND InventoryType=%d",
		lpMsg->Account,
		lpMsg->InventoryType))
	{
		pMsg.Result = 1;
	}
	else
	{
		for (auto sqlRet = gQueryManager.Fetch();
			sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA;
			sqlRet = gQueryManager.Fetch())
		{
			const int itemPage =
				(pMsg.ItemCount / MAX_CASH_SHOP_PAGE_ITEM) + 1;

			++pMsg.ItemCount;

			if (itemPage != lpMsg->InventoryPage ||
				pMsg.PageCount >= _countof(pMsg.ProductInfo))
			{
				continue;
			}

			auto& item = pMsg.ProductInfo[pMsg.PageCount];

			item.BaseItemCode = gQueryManager.GetAsInteger("BaseItemCode");
			item.MainItemCode = gQueryManager.GetAsInteger("MainItemCode");
			item.PackageMainIndex = gQueryManager.GetAsInteger("PackageMainIndex");
			item.ProductBaseIndex = gQueryManager.GetAsInteger("ProductBaseIndex");
			item.ProductMainIndex = gQueryManager.GetAsInteger("ProductMainIndex");
			item.CoinValue = gQueryManager.GetAsFloat("CoinValue");
			item.ProductType = gQueryManager.GetAsInteger("ProductType");

			gQueryManager.GetAsString(
				"GiftName",
				item.GiftName,
				sizeof(item.GiftName));

			gQueryManager.GetAsString(
				"GiftText",
				item.GiftText,
				sizeof(item.GiftText));

			++pMsg.PageCount;
		}
	}

	gQueryManager.Close();

	gSocketManager.DataSend(serverIndex, reinterpret_cast<BYTE*>(&pMsg), sizeof(pMsg));

#endif
}

void CCashShop::GDCashShopPeriodicItemRecv(const SDHP_CASH_SHOP_PERIODIC_ITEM_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV);

	constexpr size_t maxRecvCount =
		(MAX_RECV_PACKET_SIZE - sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV)) /
		sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM1);

	const size_t recvCount = static_cast<size_t>(lpMsg->Count);

	if (recvCount > maxRecvCount ||
		(sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV) +
			(sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM1) * recvCount)) > static_cast<size_t>(size))
	{
		return;
	}

	BYTE send[MAX_SEND_PACKET_SIZE]{};

	SDHP_CASH_SHOP_PERIODIC_ITEM_SEND pMsg{};

	pMsg.Header.set(HEAD_CASHSHOP, SUB_CASHSHOP_PERIODIC_ITEM, 0);
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	int sendSize = sizeof(pMsg);

	constexpr size_t maxSendCount =
		(MAX_SEND_PACKET_SIZE - sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SEND)) /
		sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM2);

	const size_t count = (recvCount < maxSendCount) ? recvCount : maxSendCount;

	const auto* lpInfo =
		reinterpret_cast<const SDHP_CASH_SHOP_PERIODIC_ITEM1*>(
			reinterpret_cast<const BYTE*>(lpMsg) +
			sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV));

	for (size_t n = 0; n < count; ++n)
	{
		SDHP_CASH_SHOP_PERIODIC_ITEM2 info{};

		info.Slot = lpInfo[n].Slot;
		info.Serial = lpInfo[n].Serial;

		if (gQueryManager.ExecQuery(
			"SELECT Time FROM CashShopPeriodicItem WHERE ItemSerial=%d",
			lpInfo[n].Serial))
		{
			const auto sqlRet = gQueryManager.Fetch();

			if (sqlRet != SQL_NO_DATA && sqlRet != SQL_NULL_DATA)
			{
				info.Time = gQueryManager.GetAsInteger("Time");

				gQueryManager.Close();

				std::memcpy(&send[sendSize], &info, sizeof(info));

				sendSize += sizeof(info);
				++pMsg.Count;

				continue;
			}
		}

		gQueryManager.Close();

		gQueryManager.ExecQuery(
			"INSERT INTO CashShopPeriodicItem (ItemSerial,Time) VALUES (%d,0)",
			lpInfo[n].Serial);

		gQueryManager.Close();

		info.Time = 0;

		std::memcpy(&send[sendSize], &info, sizeof(info));

		sendSize += sizeof(info);
		++pMsg.Count;
	}

	pMsg.Header.size[0] = SET_NUMBERHB(sendSize);
	pMsg.Header.size[1] = SET_NUMBERLB(sendSize);

	std::memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(serverIndex, send, sendSize);

#endif
}

void CCashShop::GDCashShopRecievePointRecv(const SDHP_CASH_SHOP_RECIEVE_POINT_RECV* lpMsg, int serverIndex,	int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_RECIEVE_POINT_RECV);

	SDHP_CASH_SHOP_RECIEVE_POINT_SEND pMsg{};

	pMsg.Header.set(HEAD_CASHSHOP, SUB_CASHSHOP_RECIEVE_POINT, sizeof(pMsg));
	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.CallbackFunc = lpMsg->CallbackFunc;
	pMsg.CallbackArg1 = lpMsg->CallbackArg1;
	pMsg.CallbackArg2 = lpMsg->CallbackArg2;

	LoadCashInfo(
		lpMsg->Account,
		pMsg.WCoinC,
		pMsg.WCoinP,
		pMsg.GoblinPoint);

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}

void CCashShop::GDCashShopAddPointSaveRecv(const SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV* lpMsg,	int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV);

	char targetAccount[MAX_ACCOUNT_NAME]{};

	std::memcpy(
		targetAccount,
		(lpMsg->GiftAccount[0] == '\0') ? lpMsg->Account : lpMsg->GiftAccount,
		sizeof(targetAccount));

	DWORD wCoinC = 0;
	DWORD wCoinP = 0;
	DWORD goblinPoint = 0;
	DWORD ruud = 0;

	if (!LoadCashInfo(targetAccount, wCoinC, wCoinP, goblinPoint, ruud))
	{
		if (!CreateCashInfo(targetAccount))
		{
			return;
		}
	}

	constexpr DWORD maxPoint = 0x7FFFFFFF;

	const auto addClamped = [](DWORD current, DWORD value)
		{
			constexpr DWORD maxPoint = 0x7FFFFFFF;

			return (value > (maxPoint - current))
				? maxPoint
				: (current + value);
		};

	gQueryManager.ExecQuery(
		"UPDATE CashShopData "
		"SET WCoinC=%u,WCoinP=%u,GoblinPoint=%u,Ruud=%u "
		"WHERE AccountID='%s'",
		addClamped(wCoinC, lpMsg->AddWCoinC),
		addClamped(wCoinP, lpMsg->AddWCoinP),
		addClamped(goblinPoint, lpMsg->AddGoblinPoint),
		addClamped(ruud, lpMsg->AddRuud),
		targetAccount);

	gQueryManager.Close();

#endif
}

void CCashShop::GDCashShopSubPointSaveRecv(const SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV* lpMsg,	int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV);

	char targetAccount[MAX_ACCOUNT_NAME]{};

	std::memcpy(
		targetAccount,
		(lpMsg->GiftAccount[0] == '\0') ? lpMsg->Account : lpMsg->GiftAccount,
		sizeof(targetAccount));

	DWORD wCoinC = 0;
	DWORD wCoinP = 0;
	DWORD goblinPoint = 0;
	DWORD ruud = 0;

	if (!LoadCashInfo(targetAccount, wCoinC, wCoinP, goblinPoint, ruud))
	{
		if (!CreateCashInfo(targetAccount))
		{
			return;
		}
	}

	const auto subtractClamped = [](DWORD current, DWORD value)
		{
			return (value > current) ? 0 : (current - value);
		};

	gQueryManager.ExecQuery(
		"UPDATE CashShopData "
		"SET WCoinC=%u,WCoinP=%u,GoblinPoint=%u,Ruud=%u "
		"WHERE AccountID='%s'",
		subtractClamped(wCoinC, lpMsg->SubWCoinC),
		subtractClamped(wCoinP, lpMsg->SubWCoinP),
		subtractClamped(goblinPoint, lpMsg->SubGoblinPoint),
		subtractClamped(ruud, lpMsg->SubRuud),
		targetAccount);

	gQueryManager.Close();

	if (lpMsg->SubWCoinC > 0)
	{
		const DWORD subWCoinC =
			(lpMsg->SubWCoinC > wCoinC) ? 0 : lpMsg->SubWCoinC;

		gQueryManager.ExecQuery(
			"INSERT INTO LOG_CREDITOS (login,valor,tipo) "
			"VALUES ('%s',-%u,4)",
			targetAccount,
			subWCoinC);

		gQueryManager.Close();
	}

#endif
}

void CCashShop::GDCashShopInsertItemSaveRecv(const SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV* lpMsg,	int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV);

	char targetAccount[MAX_ACCOUNT_NAME]{};

	std::memcpy(
		targetAccount,
		(lpMsg->GiftAccount[0] == '\0') ? lpMsg->Account : lpMsg->GiftAccount,
		sizeof(targetAccount));

	gQueryManager.BindParameterAsString(
		1,
		lpMsg->GiftName,
		sizeof(lpMsg->GiftName));

	gQueryManager.BindParameterAsString(
		2,
		lpMsg->GiftText,
		sizeof(lpMsg->GiftText));

	gQueryManager.ExecQuery(
		"INSERT INTO CashShopInventory "
		"(MainItemCode,AccountID,InventoryType,PackageMainIndex,ProductBaseIndex,"
		"ProductMainIndex,CoinValue,ProductType,GiftName,GiftText) "
		"VALUES (0,'%s',%d,%d,%d,%d,%f,%d,?,?)",
		targetAccount,
		lpMsg->InventoryType,
		lpMsg->PackageMainIndex,
		lpMsg->ProductBaseIndex,
		lpMsg->ProductMainIndex,
		lpMsg->CoinValue,
		lpMsg->ProductType);

	gQueryManager.Close();

#endif
}

void CCashShop::GDCashShopDeleteItemSaveRecv(
	const SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV* lpMsg,
	int serverIndex,
	int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV);

	gQueryManager.ExecQuery(
		"DELETE FROM CashShopInventory WHERE BaseItemCode=%d",
		lpMsg->BaseItemCode);

	gQueryManager.Close();

#endif
}

void CCashShop::GDCashShopPeriodicItemSaveRecv(const SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV* lpMsg,	int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	if (size < sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV))
	{
		return;
	}

	constexpr size_t maxCount =
		(MAX_RECV_PACKET_SIZE -
			sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV)) /
		sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE);

	const size_t count = lpMsg->Count;

	if (count > maxCount ||
		(sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV) +
			(sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE) * count)) > static_cast<size_t>(size))
	{
		return;
	}

	const auto* lpInfo =
		reinterpret_cast<const SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE*>(
			reinterpret_cast<const BYTE*>(lpMsg) +
			sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV));

	for (size_t n = 0; n < count; ++n)
	{
		gQueryManager.ExecQuery(
			"IF EXISTS (SELECT 1 FROM CashShopPeriodicItem WHERE ItemSerial=%d) "
			"UPDATE CashShopPeriodicItem SET Time=%d WHERE ItemSerial=%d "
			"ELSE "
			"INSERT INTO CashShopPeriodicItem (ItemSerial,Time) VALUES (%d,%d)",
			lpInfo[n].Serial,
			lpInfo[n].Time,
			lpInfo[n].Serial,
			lpInfo[n].Serial,
			lpInfo[n].Time);

		gQueryManager.Close();
	}

#endif
}

// Fix: Helpers
bool CCashShop::LoadCashInfo(const char* account, DWORD& wCoinC, DWORD& wCoinP,	DWORD& goblinPoint)
{
	DWORD ruud = 0;

	return LoadCashInfo(account, wCoinC, wCoinP, goblinPoint, ruud);
}

bool CCashShop::LoadCashInfo(const char* account, DWORD& wCoinC, DWORD& wCoinP, DWORD& goblinPoint, DWORD& ruud)
{
	wCoinC = 0;
	wCoinP = 0;
	goblinPoint = 0;
	ruud = 0;

	if (!gQueryManager.ExecQuery(
		"SELECT WCoinC,WCoinP,GoblinPoint,Ruud "
		"FROM CashShopData WHERE AccountID='%s'",
		account))
	{
		gQueryManager.Close();

		return false;
	}

	const auto sqlRet = gQueryManager.Fetch();

	if (sqlRet == SQL_NO_DATA || sqlRet == SQL_NULL_DATA)
	{
		gQueryManager.Close();

		return false;
	}

	wCoinC = static_cast<DWORD>(gQueryManager.GetAsInteger("WCoinC"));
	wCoinP = static_cast<DWORD>(gQueryManager.GetAsInteger("WCoinP"));
	goblinPoint = static_cast<DWORD>(gQueryManager.GetAsInteger("GoblinPoint"));
	ruud = static_cast<DWORD>(gQueryManager.GetAsInteger("Ruud"));

	gQueryManager.Close();

	return true;
}

bool CCashShop::CreateCashInfo(const char* account)
{
	const bool result = gQueryManager.ExecQuery(
		"INSERT INTO CashShopData "
		"(AccountID,WCoinC,WCoinP,GoblinPoint,Ruud) "
		"VALUES ('%s',0,0,0,0)",
		account);

	gQueryManager.Close();

	return result;
}

void CCashShop::GDCashShopItemUseRecv(const SDHP_CASH_SHOP_ITEM_USE_RECV* lpMsg, int serverIndex, int size)
{
#if (DATASERVER_UPDATE >= 501)

	VALIDATE_PACKET_SIZE(SDHP_CASH_SHOP_ITEM_USE_RECV);

	SDHP_CASH_SHOP_ITEM_USE_SEND pMsg{};

	pMsg.Header.set(0x18, 0x04, sizeof(pMsg));

	pMsg.Index = lpMsg->Index;

	std::memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	pMsg.Result = 0;

	pMsg.BaseItemCode = lpMsg->BaseItemCode;
	pMsg.MainItemCode = lpMsg->MainItemCode;
	pMsg.ItemIndex = lpMsg->ItemIndex;
	pMsg.ProductType = lpMsg->ProductType;

	if (!gQueryManager.ExecQuery(
		"SELECT BaseItemCode,MainItemCode,PackageMainIndex,ProductBaseIndex,"
		"ProductMainIndex,CoinValue,ProductType,GiftName,GiftText "
		"FROM CashShopInventory WHERE BaseItemCode=%d",
		lpMsg->BaseItemCode) ||
		gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.Result = 1;
	}
	else
	{
		pMsg.ProductInfo.BaseItemCode = gQueryManager.GetAsInteger("BaseItemCode");
		pMsg.ProductInfo.MainItemCode = gQueryManager.GetAsInteger("MainItemCode");
		pMsg.ProductInfo.PackageMainIndex = gQueryManager.GetAsInteger("PackageMainIndex");
		pMsg.ProductInfo.ProductBaseIndex = gQueryManager.GetAsInteger("ProductBaseIndex");
		pMsg.ProductInfo.ProductMainIndex = gQueryManager.GetAsInteger("ProductMainIndex");
		pMsg.ProductInfo.CoinValue = gQueryManager.GetAsFloat("CoinValue");
		pMsg.ProductInfo.ProductType = gQueryManager.GetAsInteger("ProductType");

		gQueryManager.GetAsString(
			"GiftName",
			pMsg.ProductInfo.GiftName,
			sizeof(pMsg.ProductInfo.GiftName));

		gQueryManager.GetAsString(
			"GiftText",
			pMsg.ProductInfo.GiftText,
			sizeof(pMsg.ProductInfo.GiftText));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(
		serverIndex,
		reinterpret_cast<BYTE*>(&pMsg),
		sizeof(pMsg));

#endif
}