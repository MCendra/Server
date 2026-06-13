// ServerDisplayer.cpp
#include "Header.h"
#include "ServerDisplayer.h"

//#include "AccountManager.h"
//#include "JoinServerProtocol.h"
//#include "Log.h"
//#include "Protect.h"
//#include "ServerManager.h"
//#include "SocketManager.h"

// Instancia global del visualizador de servidor
CServerDisplayer gServerDisplayer;

// Definición de textos de estado
constexpr char TEXT_WINDOWS_TITLE[] = "[JS] JoinServer %s - AccountCount : %d / %d (QueueSize : %d) ";
constexpr char JOINSERVER_WAIT[] = "ESPERANDO";
constexpr char JOINSERVER_ACTIVE[] = "ACTIVO";

// Constructor de CServerDisplayer
CServerDisplayer::CServerDisplayer()
	: m_hwnd(nullptr),                          // Inicializa el puntero al manejador de la ventana a nullptr.
	m_font(nullptr),                            // Inicializa el puntero a la fuente a nullptr.
	m_count(0),                                 // Inicializa el contador de logs a 0.
	m_servercode(0),                            // Inicializa el código del servidor a 0.
	m_rect{ 0, 0, 0, 0 }                        // Inicializa RECT con valores predeterminados (0, 0, 0, 0).
{
	// Inicializa la fuente con parámetros predeterminados para el texto.
	m_font = CreateFont(50, 0, 0, 0, FW_THIN, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Times"));

	// Crear pinceles para diferentes estados de visualización.
#if(GAMESERVER_TYPE2 == 0)
	m_brush[0] = CreateSolidBrush(RGB(105, 105, 105));  // Cuando está activo.
	m_brush[1] = CreateSolidBrush(RGB(105, 105, 105));  // Cuando está desactivado.
	m_brush[2] = CreateSolidBrush(RGB(0, 152, 239));    // Encabezado.
	m_brush[3] = CreateSolidBrush(RGB(41, 37, 44));     // Fondo Principal.
	m_brush[4] = CreateSolidBrush(RGB(0, 0, 0));        // Fondo de eventos e información.
#else
	m_brush[0] = CreateSolidBrush(RGB(60, 255, 51));    // Verde cuando activo.
	m_brush[1] = CreateSolidBrush(RGB(120, 120, 120));  // Gris cuando desactivado.
	m_brush[2] = CreateSolidBrush(RGB(39, 79, 121));    // Azul para encabezado.
	m_brush[3] = CreateSolidBrush(RGB(255, 255, 255));  // Blanco para fondo principal.
	m_brush[4] = CreateSolidBrush(RGB(210, 210, 210));  // Gris claro para eventos e información.
#endif

	// Inicializa los textos para mostrar en la ventana.
	strncpy_s(m_displayertext[0], sizeof(m_displayertext[0]), JOINSERVER_WAIT, _TRUNCATE);
	strncpy_s(m_displayertext[1], sizeof(m_displayertext[1]), JOINSERVER_ACTIVE, _TRUNCATE);

	// Inicializa el log con cadenas vacías y el color predeterminado.
	for (auto& log : m_log)
	{
		log.text.clear();       // Limpia la cadena de texto del log.
		log.color = LOG_BLACK;  // Establece el color predeterminado del log.
	}
}

// Destructor de CServerDisplayer
CServerDisplayer::~CServerDisplayer()
{
	// Liberar recursos de fuentes y pinceles
	DeleteObject(m_font);
	for (auto& brush : this->m_brush)
	{
		if (brush != nullptr)
		{
			DeleteObject(brush);
			brush = nullptr;  // Opcional: para evitar futuros accesos
		}
	}
}

// Inicializa la clase con el HWND de la ventana principal
void CServerDisplayer::Init(HWND hWnd)
{
	this->m_hwnd = hWnd;

	// Inicializa RECT con el tamaño de la ventana
	GetClientRect(this->m_hwnd, &m_rect);

	// Inicializa titulo de la ventana
	UpdateWindowTitle(0);

}

//void CServerDisplayer::Init(HWND hWnd) // OK
//{
//	PROTECT_START
//
//	this->m_hwnd = hWnd;
//
//	PROTECT_FINAL
//
//	gLog.AddLog(1,"LOG");
//
//	gLog.AddLog(1,"LOG_ACCOUNT");
//}

// Actualiza el título de la ventana
void CServerDisplayer::UpdateWindowTitle(int queueSize) const
{
	char buff[MAX_LOADSTRING];
	StringCchPrintfA(buff, ARRAYSIZE(buff), TEXT_WINDOWS_TITLE, JOINSERVER_VERSION, JOINSERVER_CLIENT, queueSize);
	SetWindowText(this->m_hwnd, buff);
}

//void CServerDisplayer::Run() // OK
//{
//	this->SetWindowName();
//	this->PaintAllInfo();
//	this->LogTextPaint();
//	this->PaintName();
//}
//
//void CServerDisplayer::SetWindowName() // OK
//{
//	char buff[256];
//
//    wsprintf(buff,"[%s] %s JoinServer (QueueSize : %d) (AccountCount : %d/%d)",JOINSERVER_VERSION,JOINSERVER_CLIENT,gSocketManager.GetQueueSize(),
//        gAccountManager.GetAccountCount(),
//#if    PROTECT_STATE
//        gJoinServerMaxAccount[gProtect.m_AuthInfo.PackageType][gProtect.m_AuthInfo.PlanType]
//#else
//        MAX_ACCOUNT
//#endif
//    );
//
//	SetWindowText(this->m_hwnd,buff);
//}

//void CServerDisplayer::PaintAllInfo() // OK
//{
//	RECT rect;
//
//	GetClientRect(this->m_hwnd,&rect);
//
//	rect.top = 50;
//	rect.bottom = 100;
//
//	HDC hdc = GetDC(this->m_hwnd);
//
//	int OldBkMode = SetBkMode(hdc,TRANSPARENT);
//
//	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font);
//
//	int state = 0;
//
//	for(int n=0;n < MAX_SERVER;n++)
//	{
//		if(gServerManager[n].CheckState() == 0)
//		{
//			continue;
//		}
//
//		if((GetTickCount()-gServerManager[n].m_PacketTime) <= 60000)
//		{
//			state = 1;
//			break;
//		}
//	}
//
//
//	if(state == 0)
//	{
//		SetTextColor(hdc,RGB(200,200,200));
//		FillRect(hdc,&rect,this->m_brush[1]);
//		TextOut(hdc,120,50,this->m_DisplayerText[0],strlen(this->m_DisplayerText[0]));
//	}
//	else
//	{
//		SetTextColor(hdc,RGB(250,250,250));
//		FillRect(hdc,&rect,this->m_brush[0]);
//		TextOut(hdc,150,50,this->m_DisplayerText[1],strlen(this->m_DisplayerText[1]));
//	}
//
//
//
//	SelectObject(hdc,OldFont);
//	SetBkMode(hdc,OldBkMode);
//	ReleaseDC(this->m_hwnd,hdc);
//
//}

// Pintar el nombre del cliente en la ventana
void CServerDisplayer::PaintName() const
{
	RECT rect = m_rect;
	rect.bottom = 50;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);
	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font);

	// Establece el color del texto y rellena el fondo
	SetTextColor(hdc, RGB(255, 255, 255));
	FillRect(hdc, &rect, this->m_brush[2]);

	// Dibuja el nombre del cliente en la ventana centrado
	DrawText(hdc, JOINSERVER_CLIENT, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// Restaura los objetos GDI previos
	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

// Pintar los textos del log en la ventana
void CServerDisplayer::PaintLogText()
{
	RECT rect = m_rect;
	rect.top = 100;

	HDC hdc = GetDC(this->m_hwnd);

	int line = MAX_LOG_TEXT_LINE;
	int count = m_count - 1 >= 0 ? m_count - 1 : MAX_LOG_TEXT_LINE - 1;

	for (int n = 0; n < MAX_LOG_TEXT_LINE; n++)
	{
		switch (m_log[count].color)
		{
		case LOG_BLACK:
			SetTextColor(hdc, RGB(0, 0, 0));
			break;
		case LOG_RED:
			SetTextColor(hdc, RGB(239, 0, 0));
			break;
		case LOG_GREEN:
			SetTextColor(hdc, RGB(0, 255, 0));
			break;
		case LOG_BLUE:
			SetTextColor(hdc, RGB(0, 152, 239));
			break;
		}

		int size = m_log[count].text.size();

		if (size > 1)
		{
			TextOutA(hdc, 0, 85 + (line * 15), m_log[count].text.c_str(), size);
			line--;
		}

		count = --count >= 0 ? count : MAX_LOG_TEXT_LINE - 1;
	}
}

// Clase para mostrar información del servidor en una ventana
void CServerDisplayer::LogAddText(LogColor color, const std::string& text) {
	// Limitar el tamaño del texto para evitar desbordamientos
	std::string trimmedText = text.substr(0, MAX_LOG_TEXT_SIZE - 1);

	// Copiar el texto recortado al campo m_log
	this->m_log[this->m_count].text = trimmedText;

	// Establecer el color del texto
	this->m_log[this->m_count].color = color;

	// Actualizar el índice de m_count, asegurando que no se exceda MAX_LOG_TEXT_LINE
	this->m_count = ((++this->m_count) >= MAX_LOG_TEXT_LINE) ? 0 : this->m_count;

	PaintLogText();
}

//void CServerDisplayer::LogTextPaint() // OK
//{
//	RECT rect;
//
//	GetClientRect(this->m_hwnd,&rect);
//
//	rect.top = 100;
//
//	HDC hdc = GetDC(this->m_hwnd);
//
//	int OldBkMode = SetBkMode(hdc,TRANSPARENT);
//
//	FillRect(hdc,&rect,this->m_brush[3]);
//
//	int line = MAX_LOG_TEXT_LINE;
//
//	int count = (((this->m_count-1)>=0)?(this->m_count-1):(MAX_LOG_TEXT_LINE-1));
//
//	for(int n=0;n < MAX_LOG_TEXT_LINE;n++)
//	{
//		switch(this->m_log[count].color)
//		{
//			case LOG_BLACK:
//				SetTextColor(hdc,RGB(0,0,0));
//				break;
//			case LOG_RED:
//				SetTextColor(hdc,RGB(239,0,0));
//				break;
//			case LOG_GREEN:
//				SetTextColor(hdc,RGB(0,255,0));
//				break;
//			case LOG_BLUE:
//				SetTextColor(hdc,RGB(0, 152, 239));
//				break;
//		}
//
//		int size = strlen(this->m_log[count].text);
//
//		if(size > 1)
//		{
//			TextOut(hdc,0,(85+(line*15)),this->m_log[count].text,size);
//			line--;
//		}
//
//		count = (((--count)>=0)?count:(MAX_LOG_TEXT_LINE-1));
//	}
//
//	ReleaseDC(this->m_hwnd,hdc);
//}

//void CServerDisplayer::LogAddText(eLogColor color,char* text,int size) // OK
//{
//	PROTECT_START
//
//	size = ((size>=MAX_LOG_TEXT_SIZE)?(MAX_LOG_TEXT_SIZE-1):size);
//
//	memset(&this->m_log[this->m_count].text,0,sizeof(this->m_log[this->m_count].text));
//
//	memcpy(&this->m_log[this->m_count].text,text,size);
//
//	this->m_log[this->m_count].color = color;
//
//	this->m_count = (((++this->m_count)>=MAX_LOG_TEXT_LINE)?0:this->m_count);
//
//	PROTECT_FINAL
//
//	gLog.Output(LOG_GENERAL,"%s",&text[9]);
//}

