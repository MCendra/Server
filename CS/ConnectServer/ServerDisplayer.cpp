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
    m_font(nullptr),                            // Inicializa el puntero a la fuente a nullptr.
	m_smallfont(nullptr),                       // Inicializa el puntero a la fuente pequeña a nullptr.
	m_serverlistbottom(100),
	m_count(0),                                 // Inicializa el contador de logs a 0.
    //m_servercode(0),                            // Inicializa el codigo del servidor a 0.
    m_rect{ 0, 0, 0, 0 },                       // Inicializa RECT con valores predeterminados (0, 0, 0, 0).
	m_logRect{ 0, 100, 0, 0 }					// Inicializa LOGRECT con valores predeterminados (0, 100, 0, 0).

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

    // Inicializa el log con cadenas vacias y el color predeterminado.
    for (auto& log : m_log)
    {
        log.text.clear();       // Limpia la cadena de texto del log.
        log.color = LOG_BLACK;  // Establece el color predeterminado del log.
    }
}

CServerDisplayer::~CServerDisplayer()
{
    // Liberar recursos de fuentes y pinceles
    DeleteObject(m_font);
    for (auto& brush : m_brush)
    {
        if (brush != nullptr)
        {
            DeleteObject(brush);
            brush = nullptr;  // Opcional: para evitar futuros accesos
        }
    }

	if (m_smallfont != nullptr)
	{
		DeleteObject(m_smallfont);
		m_smallfont	 = nullptr;
	}
}

// Inicializa la clase con el HWND de la ventana principal
void CServerDisplayer::Init(HWND hWnd)
{
    m_hwnd = hWnd;

    // Inicializa RECT con el tamaño de la ventana
    GetClientRect(m_hwnd, &m_rect);

	// FIX: area del log: todo el ancho, desde y=100 hasta el final de la ventana.
	m_logRect = m_rect;

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
    // Limitar el tamaño del texto para evitar desbordamientos
    std::string trimmedText = text.substr(0, MAX_LOG_TEXT_SIZE - 1);

	{
		CCriticalSection::CLock lock(m_logLock);

		// Copiar el texto recortado al campo m_log
		m_log[m_count].text = trimmedText;

		// Establecer el color del texto
		m_log[m_count].color = color;

		// Actualizar el indice de m_count, asegurando que no se exceda MAX_LOG_TEXT_LINE
		m_count = ((++m_count) >= MAX_LOG_TEXT_LINE) ? 0 : m_count;

	}

	// FIX: No dibuja nada aca: solo pide repintado al hilo de UI.
	RedrawWindow(m_hwnd, &m_logRect, NULL, RDW_INVALIDATE);
}

// Pintar los textos del log en la ventana
void CServerDisplayer::PaintLogText(HDC hdc)
{
	m_logRect.left = 0;
	m_logRect.right = m_rect.right;
	m_logRect.top = m_serverlistbottom;
	m_logRect.bottom = m_rect.bottom;

	FillRect(hdc, &m_logRect, m_brush[4]);

	int oldBkMode = SetBkMode(hdc, TRANSPARENT);
	HFONT oldFont = (HFONT)SelectObject(hdc, m_smallfont);

	CCriticalSection::CLock lock(m_logLock);

	const int lineHeight = 18;
	const int maxVisibleLines = (m_logRect.bottom - m_logRect.top) / lineHeight;

	// m_count apunta a la próxima posición de escritura.
	// Por lo tanto, desde ahí comienza el log más antiguo.
	int start = m_count;

	int y = m_logRect.top;
	int visibleCount = 0;

	for (int n = 0; n < MAX_LOG_TEXT_LINE && visibleCount < maxVisibleLines; n++)
	{
		int index = (start + n) % MAX_LOG_TEXT_LINE;

		if (m_log[index].text.empty())
		{
			continue;
		}

		switch (m_log[index].color)
		{
		case LOG_RED:
			SetTextColor(hdc, RGB(239, 0, 0));
			break;

		case LOG_GREEN:
			SetTextColor(hdc, RGB(31, 87, 31));
			break;

		case LOG_BLUE:
			SetTextColor(hdc, RGB(29, 29, 143));
			break;

		default:
			SetTextColor(hdc, RGB(0, 0, 0));
			break;
		}

		TextOutA(hdc, 5, y, m_log[index].text.c_str(), (int)m_log[index].text.length());

		y += lineHeight;
		visibleCount++;
	}

	SelectObject(hdc, oldFont);
	SetBkMode(hdc, oldBkMode);
}

void CServerDisplayer::Refresh()
{
	// Invalida la ventana para forzar un repintado
	InvalidateRect(m_hwnd, NULL, true);
}

void CServerDisplayer::PaintGameServers(HDC hdc) 
{
	RECT back;

	back.left = 0;
	back.top = 100;
	back.right = m_rect.right;
	back.bottom = m_rect.bottom;

	FillRect(hdc, &back, m_brush[4]);

	RECT rect;

	rect.left = 10;
	rect.right = m_rect.right - 10;
	rect.top = 105;

	int oldBkMode = SetBkMode(hdc, TRANSPARENT);
	HFONT oldFont = (HFONT)SelectObject(hdc, m_smallfont);

	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);

	const int lineHeight = tm.tmHeight + 4;

	for (const auto& it : gServerList.GetGameServerList())
	{
		const SERVER_LIST_INFO& info = it.second;

		char text[128];

		if (info.ServerState)
		{
			SetTextColor(hdc, RGB(0, 180, 0));

			sprintf_s(
				text,
				"[%d] %s - ONLINE (%d/%d)",
				info.ServerCode,
				info.ServerName,
				info.UserCount,
				info.MaxUserCount);
		}
		else
		{
			SetTextColor(hdc, RGB(180, 0, 0));

			sprintf_s(
				text,
				"[%d] %s - OFFLINE",
				info.ServerCode,
				info.ServerName);
		}

		TextOutA(
			hdc,
			rect.left,
			rect.top,
			text,
			(int)strlen(text));

		rect.top += lineHeight;
	}

	// Guardar dónde terminó la lista
	m_serverlistbottom = rect.top + 5;

	SelectObject(hdc, oldFont);
	SetBkMode(hdc, oldBkMode);
}
