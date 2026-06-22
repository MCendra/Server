// ConnectServer.cpp : Define el punto de entrada de la aplicacion.
#include "Header.h"
#include "ConnectServer.h"

// Variables globales:
HINSTANCE hInst;                                // Instancia actual
CHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de titulo
CHAR szWindowClass[MAX_LOADSTRING];            // Nombre de clase de la ventana principal
HWND g_hWnd;                                    // Renombrado para evitar shadowing

// Declaraciones de funciones adelantadas incluidas en este modulo de codigo:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

constexpr int WINDOW_WIDTH = 700;                   // Ancho de la ventana
constexpr int WINDOW_HEIGHT = 600;                  // Alto de la ventana

constexpr UINT TIMER_MAINTENANCE = 1;
constexpr UINT MAINTENANCE_INTERVAL = 1000;
constexpr UINT TIMER_CHECKCLIENT = 2;
constexpr UINT CLIENT_CHECK_TIMEOUTS = 5000;

constexpr char CONFIRM_EXIT_MESSAGE[] = "\xBFTerminar ConnectServer?"; // \xBF = ¿ en ASCII
constexpr char CONFIRM_EXIT_TITLE[] = "Confirmar cierre";
constexpr char ERROR_WSA_STARTUP[] = "[CS] Fallo critico: WSAStartup() error %d. El servidor no puede iniciar.";
constexpr char ERROR_WSA_TCP_STARTUP[] = "[CS] Fallo critico: no se pudo iniciar el socket TCP en el puerto %d.";
constexpr char ERROR_WSA_UDP_STARTUP[] = "[CS] Fallo critico: no se pudo iniciar el socket UDP en el puerto %d.";
constexpr char ERROR_CREATE_WINDOW[] = "[CS] CreateWindowA fallo. Codigo error: %d";

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Inicia la captura de minidumps en caso de fallos
    CMiniDump::Start();

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
        
	// Inicializar cadenas globales en ASCII
    LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringA(hInstance, IDC_CONNECTSERVER, szWindowClass, MAX_LOADSTRING);

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

    // Inicializa el archivo ini de configuracion del servidor
    gServerConfig.Init();
        
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		Log.ToFile(ERROR_WSA_STARTUP, WSAGetLastError());
		gUtil.ErrorMessageBox(ERROR_WSA_STARTUP, WSAGetLastError());
		CMiniDump::Clean();
		return false;  // gUtil.ErrorMessageBox ya llama ExitProcess, pero por claridad dejamos el return
	}

	bool wsaStarted = true;

	if (!gSocketManager.Init(ConnectServerPortTCP))
	{
		Log.ToFile(ERROR_WSA_TCP_STARTUP, WSAGetLastError());
		gUtil.ErrorMessageBox(ERROR_WSA_TCP_STARTUP, ConnectServerPortTCP);
		return false;
	}

	if (!gSocketManagerUdp.Init(ConnectServerPortUDP))
	{
		Log.ToFile(ERROR_WSA_UDP_STARTUP, WSAGetLastError());
		gUtil.ErrorMessageBox(ERROR_WSA_UDP_STARTUP, ConnectServerPortUDP);
		return false;
	}

	gServerList.Init(ServerListFilePath);

	// FIX:
	// PaintName se dibuja en WM_PAINT via InvalidateRect, no directamente.
	InvalidateRect(g_hWnd, nullptr, true);
    
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CONNECTSERVER));

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
		// Detener primero todos los sockets e hilos
		gSocketManagerUdp.Clean();
		gSocketManager.Clean();

		WSACleanup();
    }

    // Limpia el minidump al finalizar
    CMiniDump::Clean();

    return (int) msg.wParam;
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEDIUM));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEA(IDC_CONNECTSERVER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExA(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: Guarda el identificador de instancia y crea la ventana principal
//
//   COMENTARIOS:
//
//        En esta funcion, se guarda el identificador de instancia en una variable comun y
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
		Log.ToFile(ERROR_CREATE_WINDOW, GetLastError());
		return false;
	}

	ShowWindow(g_hWnd, nCmdShow);

	// Timer 1: refresco de UI cada 1 segundo
	SetTimer(g_hWnd, TIMER_MAINTENANCE, MAINTENANCE_INTERVAL, nullptr);

	// Timer 2: chequeo de estado de clientes cada 5 segundos
	SetTimer(g_hWnd, TIMER_CHECKCLIENT, CLIENT_CHECK_TIMEOUTS, nullptr);

	UpdateWindow(g_hWnd);

	return true;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO: Procesa mensajes de la ventana principal.
//
//  WM_COMMAND  - Procesar el menu de aplicaciones
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
	{
		switch (wParam)
		{
		case TIMER_MAINTENANCE:
			gServerList.CheckServerTimeouts();

			gServerDisplayer.UpdateWindowTitle(gSocketManager.GetQueueSize());
			// Invalida la ventana para forzar un repaint y actualizar la informacion visual
			InvalidateRect(hWnd, nullptr, false);
			break;

		case TIMER_CHECKCLIENT:
			CClientManager::CheckClientTimeouts();
			break;
		}
		break;
	}
    case WM_PAINT:
        {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			gServerDisplayer.PaintName(hdc);
			gServerDisplayer.PaintServerState(hdc);
			gServerDisplayer.PaintGameServers(hdc);
			gServerDisplayer.PaintLogText(hdc);

			EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		KillTimer(hWnd, TIMER_MAINTENANCE);
		KillTimer(hWnd, TIMER_CHECKCLIENT);
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
