// PacketManager.h
#pragma once

#if(GAMESERVER_UPDATE>=701)
#include "cryptopp\\cryptlib.h"
#include "cryptopp\\modes.h"
#include "cryptopp\\des.h"
using namespace CryptoPP;
#endif

#pragma pack(push,1)
struct ENCDEC_HEADER
{
	WORD Header;
	DWORD Size;
};

struct ENCDEC_DATA
{
	DWORD Modulus[4];
	DWORD Key[4];
	DWORD Xor[4];
};
#pragma pack(pop)

class CPacketManager
{
public:
	CPacketManager();
	~CPacketManager() = default;
	void Init();
	bool LoadEncryptionKey(const char* name);
	bool LoadDecryptionKey(const char* name);
	bool LoadKey(const char* name, WORD header, bool type);
	int Encrypt(BYTE* lpTarget,BYTE* lpSource,int size);
	int Decrypt(BYTE* lpTarget,BYTE* lpSource,int size);
	int EncryptBlock(BYTE* lpTarget,BYTE* lpSource,int size);
	int DecryptBlock(BYTE* lpTarget,BYTE* lpSource);
	int AddBits(BYTE* lpTarget,int targetBitPos,BYTE* lpSource,int sourceBitPos,int size);
	int GetByteOfBit(int value);
	void Shift(BYTE* lpBuff,int size,int shiftSize);
	bool AddData(BYTE* lpBuff,int size);
	bool ExtractPacket(BYTE* lpBuff);
	void XorData(int start,int end);
private:
#if(GAMESERVER_UPDATE>=701)
	ECB_Mode<DES_XEX3>::Encryption m_Encryption;
	ECB_Mode<DES_XEX3>::Decryption m_Decryption;
#else
	ENCDEC_DATA m_Encryption;
	ENCDEC_DATA m_Decryption;
	DWORD m_SaveLoadXor[4];
#endif
	BYTE m_buff[2048];
	DWORD m_size;
	BYTE m_XorFilter[32];
};

extern CPacketManager gPacketManager;
