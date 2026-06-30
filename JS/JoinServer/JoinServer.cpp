// ConnectServer.cpp : Define el punto de entrada de la aplicación.
#include "Header.h"
#include "JoinServer.h"

// Variables globales:
HINSTANCE hInst;                               // Instancia actual
CHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
CHAR szWindowClass[MAX_LOADSTRING];            // Nombre de clase de la ventana principal
HWND g_hWnd;                                   // Renombrado para evitar shadowing

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

constexpr int WINDOW_WIDTH = 700;                  // Ancho de la ventana
constexpr int WINDOW_HEIGHT = 600;                  // Alto de la ventana

constexpr UINT TIMER_MAINTENANCE = 1;
constexpr UINT MAINTENANCE_INTERVAL = 1000;
constexpr UINT TIMER_CHECKSERVER = 2;
constexpr UINT SERVER_CHECK_TIMEOUTS = 5000;

constexpr char CONFIRM_EXIT_MESSAGE[] = "\xBFTerminar JoinServer?"; // \xBF = ¿ en ANSI
constexpr char CONFIRM_EXIT_TITLE[] = "Confirmar cierre";
constexpr char ERROR_WSA_STARTUP[] = "[JS] Fallo critico: WSAStartup() error %d. El servidor no puede iniciar.";
constexpr char ERROR_DB_CONNECT[] = "[JS] Fallo critico: no se pudo conectar a la base de datos. Codigo de error: %d";
constexpr char ERROR_TCP_STARTUP[] = "[JS] Fallo critico: no se pudo iniciar el socket TCP en el puerto %d.";
constexpr char ERROR_UDP_CONNECT[] = "[JS] Fallo critico: no se pudo conectar via UDP a ConnectServer (%s:%d).";

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// Inicia la captura de minidumps en caso de fallos
	CMiniDump::Start();

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Inicializar cadenas globales en ASCII
	LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringA(hInstance, IDC_JOINSERVER, szWindowClass, MAX_LOADSTRING);

	// Registrar el tipo de ventana
	MyRegisterClass(hInstance);

	// Realizar la inicializacion de la aplicacion:
	if (!InitInstance(hInstance, nCmdShow))
	{
		// Asegurar limpieza del minidump antes de salir
		CMiniDump::Clean();
		return false;
	}

	// Obtener el path del ejecutable
	gUtil.GetExecutablePath();

	// Inicializa controlador de visualizacion de mensajes
	gServerDisplayer.Init(g_hWnd);

	// Inicializa el log al disco
	gServerLog.Init(true);

	// Inicializa el archivo ini de configuración del servidor
	gServerConfig.Init();

	// ------------------------------------------------------------------
	// Secuencia de arranque de red y base de datos.
	//
	// A diferencia de ConnectServer (que solo abre sockets), JoinServer
	// depende de tres recursos externos en cadena: Winsock, la base de
	// datos (ODBC) y el socket UDP de notificacion hacia ConnectServer.
	// Si cualquiera falla, abortamos con mensaje claro en vez de dejar
	// el proceso "vivo" pero inoperante (mismo criterio aplicado en CS).
	// ------------------------------------------------------------------

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		Log.ToFile(LogType::GENERAL, ERROR_WSA_STARTUP, WSAGetLastError());
		gUtil.ErrorMessageBox(ERROR_WSA_STARTUP, WSAGetLastError());
		CMiniDump::Clean();
		return false;  // gUtil.ErrorMessageBox ya llama ExitProcess, pero por claridad dejamos el return
	}

	bool wsaStarted = true;

	if (!gQueryManager.Connect(JoinServerODBC, JoinServerUSER, JoinServerPASS))
	{
		Log.ToFile(LogType::GENERAL, ERROR_DB_CONNECT, GetLastError());
		gUtil.ErrorMessageBox(ERROR_DB_CONNECT, GetLastError());
		return false;
	}

	if (!gSocketManager.Init(JoinServerPort))
	{
		Log.ToFile(LogType::GENERAL, ERROR_TCP_STARTUP, WSAGetLastError());
		gUtil.ErrorMessageBox(ERROR_TCP_STARTUP, JoinServerPort);
		gQueryManager.Disconnect();
		return false;
	}

	if (!gSocketManagerUDP.Connect(ConnectServerAddress, ConnectServerPortUDP))
	{
		Log.ToFile(LogType::GENERAL, ERROR_UDP_CONNECT, ConnectServerAddress, ConnectServerPortUDP);
		gUtil.ErrorMessageBox(ERROR_UDP_CONNECT, ConnectServerAddress, ConnectServerPortUDP);
		gSocketManager.Clean();
		gQueryManager.Disconnect();
		return false;
	}

	gAllowableIpList.Load(AllowableIpListFilePath);

	gServerDisplayer.SetActiveState(true);
	
	// FIX:
	// PaintName se dibuja en WM_PAINT via InvalidateRect, no directamente.
	InvalidateRect(g_hWnd, nullptr, true);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JOINSERVER));

	MSG msg;

	// Bucle principal de mensajes:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Limpieza Winsock si se inicializo
	if (wsaStarted)
	{
		// Detener primero todos los sockets e hilos, igual que en CS:
		// cerrar la red antes de WSACleanup evita que callbacks de
		// IOCP pendientes intenten usar Winsock ya finalizado.
		gSocketManagerUDP.Clean();
		gSocketManager.Clean();
		gQueryManager.Disconnect();

		WSACleanup();
	}

	// Limpia el minidump al finalizar
	CMiniDump::Clean();

	return (int)msg.wParam;
}

//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: Registra la clase de ventana.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	// Inicializa toda la estructura a cero
	WNDCLASSEXA wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_JOINSERVER));
	wcex.hCursor = LoadCursorA(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEA(IDC_JOINSERVER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIconA(wcex.hInstance, MAKEINTRESOURCEA(IDI_SMALL));

	return RegisterClassExA(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: Guarda el identificador de instancia y crea la ventana principal
//
//   COMENTARIOS:
//
//        En esta función, se guarda el identificador de instancia en una variable común y
//        se crea y muestra la ventana principal del programa.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// Almacenar identificador de instancia en una variable global
	hInst = hInstance;

	g_hWnd = CreateWindowA(szWindowClass, szTitle, WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);

	if (!g_hWnd)
	{
		Log.ToFile(LogType::GENERAL, "[JS] CreateWindowA fallo. Codigo error: %d", GetLastError());
		return false;
	}

	ShowWindow(g_hWnd, nCmdShow);

	// Timer 1: refresco de UI cada 1 segundo
	SetTimer(g_hWnd, TIMER_MAINTENANCE, MAINTENANCE_INTERVAL, nullptr);

	// Timer 2: chequeo de estado de servidores cada 5 segundos
	SetTimer(g_hWnd, TIMER_CHECKSERVER, SERVER_CHECK_TIMEOUTS, nullptr);

	UpdateWindow(g_hWnd);

	return true;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO: Procesa mensajes de la ventana principal.
//
//  WM_COMMAND  - Procesar el menú de aplicaciones
//  WM_PAINT    - Pintar la ventana principal
//  WM_DESTROY  - Publicar un mensaje de salida y volver
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Analizar las selecciones de menu:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			if (MessageBoxA(nullptr, CONFIRM_EXIT_MESSAGE, CONFIRM_EXIT_TITLE, MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				DestroyWindow(hWnd);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_TIMER:
		switch (wParam)
		{
		case TIMER_MAINTENANCE:
			JoinServerLiveProc();
			gServerDisplayer.UpdateWindowTitle(gAccountManager.GetAccountCount(), gSocketManager.GetQueueSize());
			gServerDisplayer.UpdateLayout();
			break;
		case TIMER_CHECKSERVER:
			gAccountManager.DisconnectProc();
			break;
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		gServerDisplayer.PaintName(hdc);
		gServerDisplayer.PaintJoinServerState(hdc);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_SIZE:
	{
		if (g_hWnd != nullptr)
		{
			gServerDisplayer.UpdateLayout();
		}
		break;
	}
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_MAINTENANCE);
		KillTimer(hWnd, TIMER_CHECKSERVER);
		PostQuitMessage(0);
		break;
	case WM_CLOSE: // Manejar el cierre de la ventana con el boton X
		if (MessageBoxA(hWnd, CONFIRM_EXIT_MESSAGE, CONFIRM_EXIT_TITLE, MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			DestroyWindow(hWnd);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
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
