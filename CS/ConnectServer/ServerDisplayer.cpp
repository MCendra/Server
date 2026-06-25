// ServerDisplayer.cpp
#include "Header.h"
#include "ServerDisplayer.h"
#include "ServerList.h"

// Instancia global del visualizador de servidor
CServerDisplayer gServerDisplayer;

// Definicion de textos de estado
constexpr char TEXT_WINDOWS_TITLE[] = "[CS] ConnectServer %s - Cliente %s (QueueSize : %d)";
constexpr char JOINSERVER_WAIT[] = "ESPERANDO";
constexpr char JOINSERVER_ACTIVE[] = "ACTIVO";

// Construction/Destruction
CServerDisplayer::CServerDisplayer()
    : m_hwnd(nullptr),                          // Inicializa el puntero al manejador de la ventana a nullptr.
	m_hrichedit(nullptr),
    m_font(nullptr),                            // Inicializa el puntero a la fuente a nullptr.
	m_smallfont(nullptr),                       // Inicializa el puntero a la fuente pequeña a nullptr.
	m_serverlistbottom(100),
    m_rect{ 0, 0, 0, 0 }	                    // Inicializa RECT con valores predeterminados (0, 0, 0, 0).

{
    // Inicializa la fuente con parametros predeterminados para el texto.
    m_font = CreateFont(50, 0, 0, 0, FW_THIN, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Times"));
	m_smallfont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Segoe UI"));

    // Crear pinceles para diferentes estados de visualizacion.
    #if(GAMESERVER_TYPE2 == 0)
        m_brush[0] = CreateSolidBrush(RGB(105, 105, 105));  // Cuando esta activo.
        m_brush[1] = CreateSolidBrush(RGB(105, 105, 105));  // Cuando esta desactivado.
        m_brush[2] = CreateSolidBrush(RGB(0, 152, 239));    // Encabezado.
        m_brush[3] = CreateSolidBrush(RGB(41, 37, 44));     // Fondo Principal.
        m_brush[4] = CreateSolidBrush(RGB(0, 0, 0));        // Fondo de eventos e informacion.
    #else
        m_brush[0] = CreateSolidBrush(RGB(60, 255, 51));    // Verde cuando activo.
        m_brush[1] = CreateSolidBrush(RGB(120, 120, 120));  // Gris cuando desactivado.
        m_brush[2] = CreateSolidBrush(RGB(39, 79, 121));    // Azul para encabezado.
        m_brush[3] = CreateSolidBrush(RGB(255, 255, 255));  // Blanco para fondo principal.
        m_brush[4] = CreateSolidBrush(RGB(210, 210, 210));  // Gris claro para eventos e informacion.
    #endif

    // Inicializa los textos para mostrar en la ventana.
    strncpy_s(m_displayertext[0], sizeof(m_displayertext[0]), JOINSERVER_WAIT, _TRUNCATE);
    strncpy_s(m_displayertext[1], sizeof(m_displayertext[1]), JOINSERVER_ACTIVE, _TRUNCATE);

}

CServerDisplayer::~CServerDisplayer()
{
	if (m_font) { DeleteObject(m_font);      m_font = nullptr; }
	if (m_smallfont) { DeleteObject(m_smallfont); m_smallfont = nullptr; }

	for (auto& brush : m_brush)
	{
		if (brush) { DeleteObject(brush); brush = nullptr; }
	}
	// m_hRichEdit es un child window: se destruye automaticamente
	// cuando la ventana padre recibe WM_DESTROY. No llamar DestroyWindow aqui.
}

// Inicializa la clase con el HWND de la ventana principal
void CServerDisplayer::Init(HWND hWnd)
{
    m_hwnd = hWnd;

    // Inicializa RECT con el tamaño de la ventana
    GetClientRect(m_hwnd, &m_rect);

	// Cargar Msftedit.dll (RichEdit 4.1). Ya esta en memoria en cualquier
	// Windows moderno — LoadLibrary solo incrementa el refcount, costo cero.
	// Sin esta llamada, la clase MSFTEDIT_CLASS no esta registrada y
	// CreateWindowExA falla silenciosamente devolviendo nullptr.
	LoadLibraryA("Msftedit.dll");

	// MSFTEDIT_CLASS = L"RICHDIT50W" (wide) → incompatible con CreateWindowExA
	// Usar el nombre de clase directamente como string ANSI literal
	m_hrichedit = CreateWindowExA(
		0,
		"RICHEDIT50W",      // Nombre ANSI del MSFTEDIT_CLASS — mismo resultado
		"",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_NOHIDESEL,
		0, m_serverlistbottom,
		m_rect.right,
		m_rect.bottom - m_serverlistbottom,
		m_hwnd,
		nullptr, nullptr, nullptr
	);

	// FIX: area del log: todo el ancho, desde y=100 hasta el final de la ventana.
	SendMessage(m_hrichedit, WM_SETFONT, (WPARAM)m_smallfont, FALSE);
	SendMessage(m_hrichedit, EM_SETLIMITTEXT, 200 * MAX_LOG_TEXT_SIZE, 0);

	COLORREF bkColor = GetSysColor(COLOR_WINDOW);
	SendMessage(m_hrichedit, EM_SETBKGNDCOLOR, 0, bkColor);

    // Inicializa titulo de la ventana
    UpdateWindowTitle(0);
}

// Actualiza el titulo de la ventana
void CServerDisplayer::UpdateWindowTitle(int queueSize) const
{
    char buff[MAX_LOADSTRING];
    StringCchPrintfA(buff, ARRAYSIZE(buff), TEXT_WINDOWS_TITLE, CONNECTSERVER_VERSION, CONNECTSERVER_CLIENT, queueSize);
    SetWindowText(m_hwnd, buff);
}

// Pintar el nombre del cliente en la ventana
void CServerDisplayer::PaintName(HDC hdc) const
{
    RECT rect = m_rect;
    rect.bottom = 50;

    int OldBkMode = SetBkMode(hdc, TRANSPARENT);
    HFONT OldFont = (HFONT)SelectObject(hdc, m_font);

    // Establece el color del texto y rellena el fondo
    SetTextColor(hdc, RGB(255, 255, 255));
    FillRect(hdc, &rect, m_brush[2]);

    // Dibuja el nombre del cliente en la ventana centrado
    DrawText(hdc, CONNECTSERVER_CLIENT, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Restaura los objetos GDI previos
    SelectObject(hdc, OldFont);
    SetBkMode(hdc, OldBkMode);

}

void CServerDisplayer::PaintServerState(HDC hdc) const
{
    RECT rect = m_rect;
    rect.top = 50;
    rect.bottom = 100;

    int OldBkMode = SetBkMode(hdc, TRANSPARENT);
    HFONT OldFont = (HFONT)SelectObject(hdc, m_font);

    // Actualiza el texto y el fondo basado en el estado del servidor
	if (gServerList.IsJoinServerOnline() == false)
    {
        SetTextColor(hdc, RGB(200, 200, 200));
        FillRect(hdc, &rect, m_brush[1]);
        DrawText(hdc, m_displayertext[0], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    else
    {
        SetTextColor(hdc, RGB(250, 250, 250));
        FillRect(hdc, &rect, m_brush[0]);
        DrawText(hdc, m_displayertext[1], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Restaura los objetos GDI previos
    SelectObject(hdc, OldFont);
    SetBkMode(hdc, OldBkMode);

}

// Clase para mostrar informacion del servidor en una ventana
void CServerDisplayer::LogAddText(LogColor color, const std::string& text) {
	if (!m_hrichedit) return;

	// FIX: No dibuja nada aca: solo pide repintado al hilo de UI.
	// RedrawWindow(m_hwnd, &m_logrect, NULL, RDW_INVALIDATE);
	// Truncar si es necesario
	std::string line = text.substr(0, MAX_LOG_TEXT_SIZE - 1) + "\r\n";

	// Configurar el color del texto para esta linea via CHARFORMAT2.
	// EM_SETCHARFORMAT con SCF_SELECTION aplica el formato solo al texto
	// que se inserte a continuacion — no repintea el historico existente.
	CHARFORMAT2A cf = {};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_EFFECTS;  // ambos necesarios
	cf.dwEffects = 0;                     // sin CFE_AUTOCOLOR

	switch (color)
	{
	case LOG_RED:   cf.crTextColor = RGB(220, 50, 50);  break;
	case LOG_GREEN: cf.crTextColor = RGB(50, 200, 50);  break;
	case LOG_BLUE:  cf.crTextColor = RGB(100, 180, 255); break;
	default:        cf.crTextColor = RGB(0, 0, 0); break;
	}

	// Estas tres llamadas son seguras desde hilos worker: SendMessage
	// al HWND de un control hijo es serializada por la cola de mensajes
	// de Win32 — el hilo de UI la procesa en orden, sin condiciones de carrera.
	SendMessage(m_hrichedit, EM_SETSEL, -1, -1);
	SendMessage(m_hrichedit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	SendMessage(m_hrichedit, EM_REPLACESEL, FALSE, (LPARAM)line.c_str());
}

void CServerDisplayer::Refresh()
{
	// Invalida la ventana para forzar un repintado
	InvalidateRect(m_hwnd, NULL, true);
}

void CServerDisplayer::PaintGameServers(HDC hdc)
{
	// Calculamos primero cuántas líneas hay para saber el alto real del área
	int count = (int)gServerList.GetGameServerList().size();

	int oldBkMode = SetBkMode(hdc, TRANSPARENT);
	HFONT oldFont = (HFONT)SelectObject(hdc, m_smallfont);

	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	const int lineHeight = tm.tmHeight + 4;

	int newBottom = 105 + count * lineHeight + 5;

	// Pintar fondo SOLO del área de la lista de servidores.
	// Usamos el color de fondo del sistema (gris claro por defecto)
	// para que el texto negro sea legible. El RichEdit debajo
	// tiene su propio fondo y Win32 lo repinta de forma independiente.
	RECT serverArea = { 0, 100, m_rect.right, newBottom };
	FillRect(hdc, &serverArea, (HBRUSH)(COLOR_BTNFACE + 1));

	int y = 105;
	for (const auto& it : gServerList.GetGameServerList())
	{
		const SERVER_LIST_INFO& info = it.second;
		char text[128];

		if (info.ServerState)
		{
			SetTextColor(hdc, RGB(0, 150, 0));
			sprintf_s(text, "[%d] %s - ONLINE (%d/%d)",
				info.ServerCode, info.ServerName,
				info.UserCount, info.MaxUserCount);
		}
		else
		{
			SetTextColor(hdc, RGB(180, 0, 0));
			sprintf_s(text, "[%d] %s - OFFLINE",
				info.ServerCode, info.ServerName);
		}

		TextOutA(hdc, 10, y, text, (int)strlen(text));
		y += lineHeight;
	}

	// Reposicionar el RichEdit solo si el borde cambió.
	// PostMessage en vez de MoveWindow directo: no llamar MoveWindow
	// desde dentro de WM_PAINT para evitar repintados anidados.
	if (newBottom != m_serverlistbottom)
	{
		m_serverlistbottom = newBottom;
		PostMessage(m_hwnd, WM_REPOSITION_RICHEDIT, 0, 0);
	}

	SelectObject(hdc, oldFont);
	SetBkMode(hdc, oldBkMode);
}

void CServerDisplayer::RepositionRichEdit()
{
	if (m_hrichedit)
	{
		MoveWindow(
			m_hrichedit,
			0, m_serverlistbottom,
			m_rect.right,
			m_rect.bottom - m_serverlistbottom,
			TRUE
		);
	}
}
void CServerDisplayer::InvalidateServerList()
{
	RECT serverArea = { 0, 100, m_rect.right, m_serverlistbottom };
	InvalidateRect(m_hwnd, &serverArea, false);
}
