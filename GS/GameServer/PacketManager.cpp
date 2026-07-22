// PacketManager.cpp
#include "Header.h"
#include "PacketManager.h"

CPacketManager gPacketManager;

// Construction/Destruction default

CPacketManager::CPacketManager()
{
	this->Init();
}

void CPacketManager::Init()
{
#if (GAMESERVER_UPDATE >= 701)

	static constexpr BYTE DesXex3Key[24] =
	{
		0x0C,0xB0,0x66,0xCC,0xEF,0x92,0x8C,0x5C,
		0x65,0xF4,0xAC,0x3F,0x71,0xF2,0x7B,0xCE,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	m_Encryption.SetKey(DesXex3Key, sizeof(DesXex3Key));
	m_Decryption.SetKey(DesXex3Key, sizeof(DesXex3Key));

#else

	memset(&m_Encryption, 0, sizeof(m_Encryption));
	memset(&m_Decryption, 0, sizeof(m_Decryption));

	static constexpr DWORD SaveLoadXor[4] =
	{
		0x3F08A79B,
		0xE25CC287,
		0x93D27AB9,
		0x20DEA7BF
	};

	memcpy(m_SaveLoadXor, SaveLoadXor, sizeof(SaveLoadXor));

#endif

	memset(m_buff, 0, sizeof(m_buff));

	m_size = 0;

#if (GAMESERVER_UPDATE >= 601)

	static constexpr BYTE XorFilter[32] =
	{
		0xAB,0x11,0xCD,0xFE,0x18,0x23,0xC5,0xA3,
		0xCA,0x33,0xC1,0xCC,0x66,0x67,0x21,0xF3,
		0x32,0x12,0x15,0x35,0x29,0xFF,0xFE,0x1D,
		0x44,0xEF,0xCD,0x41,0x26,0x3C,0x4E,0x4D
	};

#else

	static constexpr BYTE XorFilter[32] =
	{
		0xE7,0x6D,0x3A,0x89,0xBC,0xB2,0x9F,0x73,
		0x23,0xA8,0xFE,0xB6,0x49,0x5D,0x39,0x5D,
		0x8A,0xCB,0x63,0x8D,0xEA,0x7D,0x2B,0x5F,
		0xC3,0xB1,0xE9,0x83,0x29,0x51,0xE8,0x56
	};

#endif

	memcpy(m_XorFilter, XorFilter, sizeof(XorFilter));
}

bool CPacketManager::LoadEncryptionKey(const char* name)
{
#if (GAMESERVER_UPDATE >= 701)

	return true;

#else

	return LoadKey(name, 4370, false);

#endif
}

bool CPacketManager::LoadDecryptionKey(const char* name)
{
#if (GAMESERVER_UPDATE >= 701)

	return true;

#else

	return LoadKey(name, 4370, true);

#endif
}

bool CPacketManager::LoadKey(const char* name, WORD header, bool type)
{
#if (GAMESERVER_UPDATE >= 701)

	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(header);
	UNREFERENCED_PARAMETER(type);

	return true;

#else

	ENCDEC_HEADER HeaderInfo;

	HANDLE file = CreateFileA(
		name,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (file == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD bytesRead = 0;

	if (ReadFile(file, &HeaderInfo, sizeof(HeaderInfo), &bytesRead, nullptr) == FALSE ||
		bytesRead != sizeof(HeaderInfo))
	{
		CloseHandle(file);
		return false;
	}

	if (HeaderInfo.Header != header ||
		HeaderInfo.Size != (sizeof(ENCDEC_HEADER) + sizeof(ENCDEC_DATA)))
	{
		CloseHandle(file);
		return false;
	}

	ENCDEC_DATA* lpData = (type == false) ? &m_Encryption : &m_Decryption;

	DWORD table[4];

	if (ReadFile(file, table, sizeof(table), &bytesRead, nullptr) == FALSE ||
		bytesRead != sizeof(table))
	{
		CloseHandle(file);
		return false;
	}

	for (int n = 0; n < 4; ++n)
	{
		lpData->Modulus[n] = m_SaveLoadXor[n] ^ table[n];
	}

	if (ReadFile(file, table, sizeof(table), &bytesRead, nullptr) == FALSE ||
		bytesRead != sizeof(table))
	{
		CloseHandle(file);
		return false;
	}

	for (int n = 0; n < 4; ++n)
	{
		lpData->Key[n] = m_SaveLoadXor[n] ^ table[n];
	}

	if (ReadFile(file, table, sizeof(table), &bytesRead, nullptr) == FALSE ||
		bytesRead != sizeof(table))
	{
		CloseHandle(file);
		return false;
	}

	for (int n = 0; n < 4; ++n)
	{
		lpData->Xor[n] = m_SaveLoadXor[n] ^ table[n];
	}

	CloseHandle(file);

	return true;

#endif
}

int CPacketManager::Encrypt(BYTE* lpTarget, BYTE* lpSource, int size)
{
#if (GAMESERVER_UPDATE >= 701)

	const int originalSize = size;

	int encryptedSize = originalSize;

	if ((encryptedSize % 8) != 0)
	{
		encryptedSize = ((encryptedSize / 8) + 1) * 8;
	}

	m_Encryption.ProcessData(lpTarget, lpSource, encryptedSize);

	lpTarget[encryptedSize] = static_cast<BYTE>(encryptedSize - originalSize);

	return (encryptedSize + 1);

#else

	const int originalSize = size;

	const int blockCount = (originalSize + 7) / 8;
	const int encryptedSize = ((blockCount + (blockCount * 4)) * 2) + blockCount;

	if (lpTarget != nullptr)
	{
		int remainingSize = originalSize;

		for (int offset = 0; remainingSize > 0; offset += 8, remainingSize -= 8, lpTarget += 11)
		{
			const int blockSize = (remainingSize >= 8) ? 8 : remainingSize;

			EncryptBlock(lpTarget, &lpSource[offset], blockSize);
		}
	}

	return encryptedSize;

#endif
}

int CPacketManager::Decrypt(BYTE* lpTarget, BYTE* lpSource, int size)
{
#if (GAMESERVER_UPDATE >= 701)

	const int originalSize = size;

	int encryptedSize = originalSize - 1;

	if ((encryptedSize % 8) != 0)
	{
		encryptedSize = ((encryptedSize / 8) + 1) * 8;
	}

	m_Decryption.ProcessData(lpTarget, lpSource, encryptedSize);

	return (originalSize - lpSource[originalSize - 1]);

#else

	if (lpTarget == nullptr || size <= 0)
	{
		return (size * 8) / 11;
	}

	int result = 0;
	int processedSize = 0;

	while (processedSize < size)
	{
		const int blockSize = DecryptBlock(lpTarget, lpSource);

		if (blockSize < 0)
		{
			return blockSize;
		}

		result += blockSize;

		processedSize += 11;
		lpSource += 11;
		lpTarget += 8;
	}

	return result;

#endif
}

int CPacketManager::EncryptBlock(BYTE* lpTarget, BYTE* lpSource, int size)
{
#if (GAMESERVER_UPDATE >= 701)

	return 0;

#else

	DWORD encBuffer[4] = {};
	DWORD encValue = 0;

	memset(lpTarget, 0, 11);

	for (int n = 0; n < 4; n++)
	{
		encBuffer[n] =
			(((m_Encryption.Xor[n] ^ reinterpret_cast<WORD*>(lpSource)[n]) ^ encValue) * m_Encryption.Key[n]) %
			m_Encryption.Modulus[n];

		encValue = static_cast<WORD>(encBuffer[n]);
	}

	for (int n = 0; n < 3; n++)
	{
		encBuffer[n] =
			(encBuffer[n] ^ m_Encryption.Xor[n]) ^
			static_cast<WORD>(encBuffer[n + 1]);
	}

	int bitPos = 0;

	for (int n = 0; n < 4; n++)
	{
		bitPos = AddBits(lpTarget, bitPos, reinterpret_cast<BYTE*>(&encBuffer[n]), 0, 16);
		bitPos = AddBits(lpTarget, bitPos, reinterpret_cast<BYTE*>(&encBuffer[n]), 22, 2);
	}

	BYTE checkSum = 0xF8;

	for (int n = 0; n < 8; n++)
	{
		checkSum ^= lpSource[n];
	}

	reinterpret_cast<BYTE*>(&encValue)[0] = (checkSum ^ size) ^ 0x3D;
	reinterpret_cast<BYTE*>(&encValue)[1] = checkSum;

	return AddBits(lpTarget, bitPos, reinterpret_cast<BYTE*>(&encValue), 0, 16);

#endif
}

int CPacketManager::DecryptBlock(BYTE* lpTarget, BYTE* lpSource)
{
#if (GAMESERVER_UPDATE >= 701)

	return 0;

#else

	DWORD decBuffer[4] = {};

	memset(lpTarget, 0, 8);

	int bitPos = 0;

	for (int n = 0; n < 4; n++)
	{
		AddBits(reinterpret_cast<BYTE*>(&decBuffer[n]), 0, lpSource, bitPos, 16);
		bitPos += 16;

		AddBits(reinterpret_cast<BYTE*>(&decBuffer[n]), 22, lpSource, bitPos, 2);
		bitPos += 2;
	}

	for (int n = 2; n >= 0; n--)
	{
		decBuffer[n] =
			(decBuffer[n] ^ m_Decryption.Xor[n]) ^
			static_cast<WORD>(decBuffer[n + 1]);
	}

	DWORD value = 0;

	for (int n = 0; n < 4; n++)
	{
		reinterpret_cast<WORD*>(lpTarget)[n] =
			static_cast<WORD>(
				(((m_Decryption.Key[n] * decBuffer[n]) % m_Decryption.Modulus[n]) ^
					m_Decryption.Xor[n]) ^
				value);

		value = static_cast<WORD>(decBuffer[n]);
	}

	decBuffer[0] = 0;

	AddBits(reinterpret_cast<BYTE*>(decBuffer), 0, lpSource, bitPos, 16);

	reinterpret_cast<BYTE*>(decBuffer)[0] =
		(reinterpret_cast<BYTE*>(decBuffer)[0] ^
			reinterpret_cast<BYTE*>(decBuffer)[1]) ^ 0x3D;

	BYTE checkSum = 0xF8;

	for (int n = 0; n < 8; n++)
	{
		checkSum ^= lpTarget[n];
	}

	if (checkSum != reinterpret_cast<BYTE*>(decBuffer)[1])
	{
		return -1;
	}

	return reinterpret_cast<BYTE*>(decBuffer)[0];

#endif
}

int CPacketManager::AddBits(BYTE* lpTarget, int targetBitPos, BYTE* lpSource, int sourceBitPos, int size)
{
#if (GAMESERVER_UPDATE >= 701)

	return 0;

#else

	const int sourceBitEnd = sourceBitPos + size;

	const int sourceByteOffset = GetByteOfBit(sourceBitPos);
	const int sourceByteCount = GetByteOfBit(sourceBitEnd - 1) + (1 - sourceByteOffset);

	std::vector<BYTE> tempBuffer(sourceByteCount + 1, 0);

	memcpy(tempBuffer.data(), &lpSource[sourceByteOffset], sourceByteCount);

	if ((sourceBitEnd % 8) != 0)
	{
		tempBuffer[sourceByteCount - 1] &= static_cast<BYTE>(0xFF << (8 - (sourceBitEnd % 8)));
	}

	const int shiftLeft = sourceBitPos % 8;
	const int shiftRight = targetBitPos % 8;

	Shift(tempBuffer.data(), sourceByteCount, -shiftLeft);
	Shift(tempBuffer.data(), sourceByteCount + 1, shiftRight);

	const int targetByteCount = sourceByteCount + ((shiftRight > shiftLeft) ? 1 : 0);

	BYTE* lpTempTarget = &lpTarget[GetByteOfBit(targetBitPos)];

	for (int n = 0; n < targetByteCount; n++)
	{
		lpTempTarget[n] |= tempBuffer[n];
	}

	return (targetBitPos + size);

#endif
}

int CPacketManager::GetByteOfBit(int value)
{
#if (GAMESERVER_UPDATE >= 701)

	return 0;

#else

	return (value >> 3);

#endif
}

void CPacketManager::Shift(BYTE* lpBuff, int size, int shiftSize)
{
#if (GAMESERVER_UPDATE >= 701)

	return;

#else

	if (shiftSize == 0)
	{
		return;
	}

	if (shiftSize > 0)
	{
		if (size > 1)
		{
			for (int n = size - 1; n > 0; n--)
			{
				lpBuff[n] =
					static_cast<BYTE>(
						(lpBuff[n - 1] << (8 - shiftSize)) |
						(lpBuff[n] >> shiftSize));
			}
		}

		lpBuff[0] >>= shiftSize;
	}
	else
	{
		shiftSize = -shiftSize;

		if (size > 1)
		{
			for (int n = 0; n < (size - 1); n++)
			{
				lpBuff[n] =
					static_cast<BYTE>(
						(lpBuff[n + 1] >> (8 - shiftSize)) |
						(lpBuff[n] << shiftSize));
			}
		}

		lpBuff[size - 1] <<= shiftSize;
	}

#endif
}

bool CPacketManager::AddData(BYTE* lpBuff, int size)
{
	if (size <= 0 || size > static_cast<int>(sizeof(m_buff)))
	{
		return false;
	}

	memcpy(m_buff, lpBuff, size);

	m_size = size;

	return true;
}

bool CPacketManager::ExtractPacket(BYTE* lpBuff)
{
	int size = 0;
	int headerSize = 0;

	switch (m_buff[0])
	{
	case 0xC1:
		size = m_buff[1];
		headerSize = 2;
		break;

	case 0xC2:
		size = MAKE_NUMBERW(m_buff[2], m_buff[1]);
		headerSize = 3;
		break;

	default:
		return false;
	}

	if (m_size < static_cast<DWORD>(size))
	{
		return false;
	}

	XorData(size - 1, headerSize);

	memcpy(lpBuff, m_buff, size);

	return true;
}

void CPacketManager::XorData(int start, int end)
{
	if (start <= end)
	{
		return;
	}

	for (int n = start; n > end; --n)
	{
		m_buff[n] ^= (m_buff[n - 1] ^ m_XorFilter[n & 0x1F]);
	}
}