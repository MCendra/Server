// DataServer.cpp : Define el punto de entrada de la aplicacion.
#include "Header.h"
#include "DataServer.h"

//#include "stdafx.h"
//#include "AllowableIpList.h"
//#include "BadSyntax.h"
//#include "GuildManager.h"
//#include "QueryManager.h"
//#include "ServerDisplayer.h"
//#include "SocketManager.h"
//#include "Util.h"

// Variables globales:
HINSTANCE hInst;                               // Instancia actual
CHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de titulo
CHAR szWindowClass[MAX_LOADSTRING];            // Nombre de clase de la ventana principal
HWND g_hWnd;                                   // Renombrado para evitar shadowing

// Declaraciones de funciones adelantadas incluidas en este modulo de codigo:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

char CustomerName[32];
char CustomerHardwareId[36];
int AdvancedLog;
int RSTimeCTC;

constexpr char ERROR_WSA_STARTUP[] = "[CS] Fallo critico: WSAStartup() error %d. El servidor no puede iniciar.";
constexpr char ERROR_WSA_TCP_STARTUP[] = "[CS] Fallo critico: no se pudo iniciar el socket TCP en el puerto %d.";
constexpr char ERROR_WSA_UDP_STARTUP[] = "[CS] Fallo critico: no se pudo iniciar el socket UDP en el puerto %d.";

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{

	CMiniDump::Start();

	LoadString(hInstance,IDS_APP_TITLE,szTitle,MAX_LOADSTRING);
	LoadString(hInstance,IDC_DATASERVER,szWindowClass,MAX_LOADSTRING);

	MyRegisterClass(hInstance);

	if(InitInstance(hInstance,nCmdShow) == 0)
	{
		return 0;
	}

	GetPrivateProfileString("DataServerInfo","CustomerName","",CustomerName,sizeof(CustomerName),".\\DataServer.ini");

	GetPrivateProfileString("DataServerInfo","CustomerHardwareId","",CustomerHardwareId,sizeof(CustomerHardwareId),".\\DataServer.ini");

	#if(PROTECT_STATE==0)

	#if(DATASERVER_UPDATE>=801)
	gProtect.StartAuth(AUTH_SERVER_TYPE_S8_DATA_SERVER);
	#elif(DATASERVER_UPDATE>=601)
	gProtect.StartAuth(AUTH_SERVER_TYPE_S6_DATA_SERVER);
	#elif(DATASERVER_UPDATE>=401)
	gProtect.StartAuth(AUTH_SERVER_TYPE_S4_DATA_SERVER);
	#else
	gProtect.StartAuth(AUTH_SERVER_TYPE_S2_DATA_SERVER);
	#endif

	#endif

	char buff[256];

	wsprintf(buff,"[%s] %s DataServer (QueueSize : %d)",DATASERVER_VERSION,DATASERVER_CLIENT,0);

	SetWindowText(hWnd,buff);

	gServerDisplayer.Init(hWnd);

	WSADATA wsa;

	if(WSAStartup(MAKEWORD(2,2),&wsa) == 0)
	{
		char DataServerODBC[32] = {0};

		char DataServerUSER[32] = {0};

		char DataServerPASS[32] = {0};

		GetPrivateProfileString("DataServerInfo","DataServerODBC","",DataServerODBC,sizeof(DataServerODBC),".\\DataServer.ini");

		GetPrivateProfileString("DataServerInfo","DataServerUSER","",DataServerUSER,sizeof(DataServerUSER),".\\DataServer.ini");

		GetPrivateProfileString("DataServerInfo","DataServerPASS","",DataServerPASS,sizeof(DataServerPASS),".\\DataServer.ini");

		WORD DataServerPort = GetPrivateProfileInt("DataServerInfo","DataServerPort",55960,".\\DataServer.ini");

		AdvancedLog = GetPrivateProfileInt("DataServerInfo","AdvancedLog",0,".\\DataServer.ini");

#if(CHIEN_TRUONG_CO)
		RSTimeCTC = GetPrivateProfileInt("DataServerInfo", "RSTimeCTC", 40, ".\\DataServer.ini");
#endif

		if(gQueryManager.Connect(DataServerODBC,DataServerUSER,DataServerPASS) == 0)
		{
			LogAdd(LOG_RED,"Could not connect to database");
		}
		else
		{
			if(gSocketManager.Start(DataServerPort) == 0)
			{
				gQueryManager.Disconnect();
			}
			else
			{
				gAllowableIpList.Load("AllowableIpList.txt");

				gBadSyntax.Load("BadSyntax.txt");

				SetTimer(hWnd,TIMER_1000,1000,0);

				gGuildManager.Init();
			}
		}
	}
	else
	{
		LogAdd(LOG_RED,"WSAStartup() failed with error: %d",WSAGetLastError());
	}

	gServerDisplayer.PaintAllInfo();

	gServerDisplayer.PaintName();

	SetTimer(hWnd,TIMER_2000,2000,0);

	HACCEL hAccelTable = LoadAccelerators(hInstance,(LPCTSTR)IDC_DATASERVER);

	MSG msg;

	while(GetMessage(&msg,0,0,0) != 0)
	{
		if(TranslateAccelerator(msg.hwnd,hAccelTable,&msg) == 0)
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}

	CMiniDump::Clean();


	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) // OK
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance,(LPCTSTR)IDI_DATASERVER);
	wcex.hCursor = LoadCursor(0,IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = (LPCSTR)IDC_DATASERVER;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance,(LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance,int nCmdShow) // OK
{
	hInst = hInstance;

	hWnd = CreateWindow(szWindowClass,szTitle,WS_OVERLAPPEDWINDOW | WS_THICKFRAME,CW_USEDEFAULT,0,600,600,0,0,hInstance,0);

	if(hWnd == 0)
	{
		return 0;
	}

	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
	return 1;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) // OK
{
	switch(message)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDM_ABOUT:
					DialogBox(hInst,(LPCTSTR)IDD_ABOUTBOX,hWnd,(DLGPROC)About);
					break;
				case IDM_EXIT:
					if(MessageBox(0,"Are you sure to terminate DataServer?","Ask terminate server",MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
						DestroyWindow(hWnd);
					}
					break;
				default:
					return DefWindowProc(hWnd,message,wParam,lParam);
			}
			break;
		case WM_TIMER:
			switch(wParam)
			{
				case TIMER_1000:
					break;
				case TIMER_2000:
					gServerDisplayer.Run();
					break;
				default:
					break;
			}
			break;
		case WM_CLOSE:
			if (MessageBox(0, "Close DataServer?", "DataServer", MB_OKCANCEL) == IDOK)
			{
				DestroyWindow(hWnd);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd,message,wParam,lParam);
	}

	return 0;
}

// Controlador de mensajes del cuadro Acerca de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)true;
		}
		break;
	}
	return (INT_PTR)false;
}

