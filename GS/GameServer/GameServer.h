#pragma once
#include "Resource.h"
#include "MiniDump.h"
#include "Log.h" // Incluye el archivo de encabezado ServerLog y ServerDisplayer
#include "BloodCastle.h"
#include "CastleDeep.h"
#include "CastleSiege.h"
#include "ChaosCastle.h"
#include "Crywolf.h"
#include "CustomArena.h"
#include "CustomEventDrop.h"
#include "CustomOnlineLottery.h"
#include "CustomQuiz.h"
#include "DevilSquare.h"
#include "EventTvT.h"
#include "GameMain.h"
#include "IllusionTemple.h"
#include "InvasionManager.h"
#include "JSProtocol.h"
#include "Message.h"
#include "Notice.h"
//#include "Protect.h"
#include "QueueTimer.h"
#include "ServerDisplayer.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
//#include "ThemidaSDK.h"
#include "Util.h"
#include "ReiDoMU.h"
#include "IpManager.h"
#include "CustomAttack.h"
#include "CustomStore.h"
#include "OfflineMode.h"
#include "License.h"
#include "RamFix.h"
#include "FakeOnline.h"
#include "SkyEvent.h"
#include "BossGuild.h"
#include "CTCMini.h"
#include "BotStore.h"
#include "Path.h"
#include "BotOnline.h"
#include "BEventThanMa.h"

//#define MAX_LOADSTRING 100
//
//#define WM_TIMER_1000 100
//#define WM_TIMER_2000 101
//#define WM_TIMER_10000 102
//
//#define WM_JOIN_SERVER_MSG_PROC (WM_USER+1)
//#define WM_DATA_SERVER_MSG_PROC (WM_USER+2)
//
//ATOM MyRegisterClass(HINSTANCE hInstance);
//BOOL InitInstance(HINSTANCE hInstance,int nCmdShow);
//LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
//LRESULT CALLBACK About(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
//LRESULT CALLBACK UserOnline(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
//antiflood
LRESULT CALLBACK IPBanned(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam); // OK
HWND hWndComboBox;
HWND hWndComboBox1;