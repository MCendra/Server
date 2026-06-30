// ServerDisplayer.cpp
#include "Header.h"
#include "ServerDisplayer.h"
#include "Log.h"

// Instancia global del visualizador de servidor
CServerDisplayer gServerDisplayer;

// Definicion de textos de estado
constexpr char TEXT_WINDOWS_TITLE[] = "[DS] DataServer %s - (QueueSize : %d)";
constexpr char DATASERVER_WAIT[] = "ESPERANDO";
constexpr char DATASERVER_ACTIVE[] = "ACTIVO";

// Construction/Destruction
CServerDisplayer::CServerDisplayer()
	: m_hwnd(nullptr),
	m_hrichedit(nullptr),
	m_font(nullptr),
	m_smallfont(nullptr),
	m_richeditmodule(nullptr),
	m_serverlistbottom(100),
	m_lineheight(22),
	m_rect{ 0, 0, 0, 0 }

{
	// Inicializa la fuente con parametros predeterminados para el texto.
	m_font = CreateFont(50, 0, 0, 0, FW_THIN, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Times"));
	m_smallfont = CreateFont(18, 0, 0, 0, FW_NORMAL, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Segoe UI"));

	// Crear pinceles para diferentes estados de visualizacion.
	m_brush[0] = CreateSolidBrush((GAMESERVER_TYPE2 == 0) ? RGB(105, 105, 105) : RGB(60, 255, 51));
	m_brush[1] = CreateSolidBrush((GAMESERVER_TYPE2 == 0) ? RGB(105, 105, 105) : RGB(120, 120, 120));
	m_brush[2] = CreateSolidBrush((GAMESERVER_TYPE2 == 0) ? RGB(0, 152, 239) : RGB(39, 79, 121));
	m_brush[3] = CreateSolidBrush((GAMESERVER_TYPE2 == 0) ? RGB(41, 37, 44) : RGB(255, 255, 255));
	m_brush[4] = CreateSolidBrush((GAMESERVER_TYPE2 == 0) ? RGB(0, 0, 0) : RGB(210, 210, 210));

	// Inicializa los textos para mostrar en la ventana.
	strncpy_s(m_displayertext[0], sizeof(m_displayertext[0]), DATASERVER_WAIT, _TRUNCATE);
	strncpy_s(m_displayertext[1], sizeof(m_displayertext[1]), DATASERVER_ACTIVE, _TRUNCATE);

}

CServerDisplayer::~CServerDisplayer()
{
	if (m_font)
	{
		DeleteObject(m_font);
		m_font = nullptr;
	}

	if (m_smallfont)
	{
		DeleteObject(m_smallfont);
		m_smallfont = nullptr;
	}

	for (auto& brush : m_brush)
	{
		if (brush)
		{
			DeleteObject(brush);
			brush = nullptr;
		}
	}

	if (m_richeditmodule != nullptr)
	{
		FreeLibrary(m_richeditmodule);
		m_richeditmodule = nullptr;
	}
}

// Inicializa la clase con el HWND de la ventana principal
void CServerDisplayer::Init(HWND hWnd)
{
	m_hwnd = hWnd;

	GetClientRect(m_hwnd, &m_rect);

	m_richeditmodule = LoadLibraryA("Msftedit.dll");

	if (m_richeditmodule == nullptr)
	{
		Log.ToDisp(LOG_RED, "[ServerDisplayer - Init] No se pudo cargar Msftedit.dll");
		return;
	}

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

	if (m_hrichedit == nullptr)
	{
		Log.ToDisp(LOG_RED, "[ServerDisplayer - Init] No se pudo crear el control RichEdit. Error: %lu", GetLastError());
		return;
	}

	UpdateLayout();

	SendMessage(m_hrichedit, WM_SETFONT, (WPARAM)m_smallfont, true);
	SendMessage(m_hrichedit, EM_SETLIMITTEXT, 200 * MAX_LOG_TEXT_SIZE, 0);

	COLORREF bkColor = GetSysColor(COLOR_WINDOW);
	SendMessage(m_hrichedit, EM_SETBKGNDCOLOR, 0, bkColor);

	UpdateWindowTitle(0);
}

// Actualiza el titulo de la ventana
void CServerDisplayer::UpdateWindowTitle(int queueSize) const
{
	char buff[MAX_LOADSTRING];
	StringCchPrintfA(buff, ARRAYSIZE(buff), TEXT_WINDOWS_TITLE, DATASERVER_VERSION, queueSize);
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
	DrawText(hdc, SERVER_PART, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// Restaura los objetos GDI previos
	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);

}

void CServerDisplayer::PaintDataServerState(HDC hdc) const
{
	RECT rect = m_rect;
	rect.top = 50;
	rect.bottom = 100;

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);
	HFONT OldFont = (HFONT)SelectObject(hdc, m_font);

	// Actualiza el texto y el fondo basado en el estado del servidor
	if (m_isactive == false)
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

	std::string line = text.substr(0, MAX_LOG_TEXT_SIZE - 1) + "\r\n";

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

	{
		CCriticalSection::CLock lock(m_logLock);

		SendMessage(m_hrichedit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
		SendMessage(m_hrichedit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		SendMessage(m_hrichedit, EM_REPLACESEL, false, (LPARAM)line.c_str());
		SendMessage(m_hrichedit, EM_SCROLLCARET, 0, 0);
	}
}

void CServerDisplayer::Refresh()
{
	if (m_hwnd != nullptr)
	{
		InvalidateRect(m_hwnd, nullptr, FALSE);
	}
}

void CServerDisplayer::InvalidateServerList()
{
	UpdateLayout();

	if (m_hwnd != nullptr)
	{
		RECT rc =
		{
			0,
			100,
			m_rect.right,
			m_rect.bottom
		};

		InvalidateRect(m_hwnd, &rc, true);
	}
}

void CServerDisplayer::UpdateLayout()
{
	if (m_hwnd == nullptr || m_hrichedit == nullptr)
	{
		return;
	}

	GetClientRect(m_hwnd, &m_rect);

	HDC hdc = GetDC(m_hwnd);

	HFONT oldFont = (HFONT)SelectObject(hdc, m_smallfont);

	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);

	m_lineheight = tm.tmHeight + 4;

	SelectObject(hdc, oldFont);
	ReleaseDC(m_hwnd, hdc);

	int count = 1; // (int)gServerList.GetGameServerList().size();

	m_serverlistbottom = 105 + (count * m_lineheight) + 5;

	if (m_hrichedit != nullptr)
	{
		MoveWindow(
			m_hrichedit,
			0,
			m_serverlistbottom,
			m_rect.right,
			m_rect.bottom - m_serverlistbottom,
			TRUE);
	}

	InvalidateRect(m_hwnd, nullptr, FALSE);
}
