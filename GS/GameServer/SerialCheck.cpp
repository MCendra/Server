#include "Header.h"
#include "SerialCheck.h"

CSerialCheck gSerialCheck[MAX_OBJECT];

CSerialCheck::CSerialCheck()
{
	Init();
}

void CSerialCheck::Init()
{
	m_RecvSerial = 0xFF;
	m_SendSerial = 0;
}

bool CSerialCheck::CheckSerial(BYTE serial)
{
	if (static_cast<BYTE>(m_RecvSerial + 1) == serial)
	{
		++m_RecvSerial;
		return true;
	}

	return false;
}

BYTE CSerialCheck::GetRecvSerial()
{
	return m_RecvSerial;
}

BYTE CSerialCheck::GetSendSerial()
{
	return m_SendSerial++;
}
