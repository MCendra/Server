// MD5.h
#pragma once
#include <cstdio>
#include <cstring>
#include <iosfwd>
#include <fstream>

class MD5
{
public:
	// Extensiones
	bool MD5_EncodeKeyVal(
		const char* lpszInputStr,
		char* lpszOutputKeyVal,
		int iKeyIndex);

	bool MD5_EncodeString(
		const char* lpszInputStr,
		char* lpszOutputStr,
		int iKeyIndex);

	bool MD5_CheckValue(
		const char* lpszInputStr,
		const char* szKeyVal,
		int iKeyIndex);

	// API MD5
	MD5();

	void update(const unsigned char* input, unsigned int input_length);
	void update(std::istream& stream);
	void update(FILE* file);
	void update(std::ifstream& stream);

	void finalize();

	MD5(unsigned char* string);
	MD5(std::istream& stream);
	MD5(FILE* file);
	MD5(std::ifstream& stream);

	unsigned char* raw_digest() const;
	char* hex_digest() const;

	friend std::ostream& operator<<(std::ostream&, const MD5&);
private:

	void setmagicnum(int keyindex);

	mutable unsigned char m_cRaw_digest[16];
	mutable char m_cHex_digest[33];

	typedef unsigned int   uint4;
	typedef unsigned short uint2;
	typedef unsigned char  uint1;

	uint4 state[4];
	uint4 count[2];
	uint1 buffer[64];
	uint1 digest[16];
	bool finalized;

	void init();
	void transform(const uint1* block);

	static void encode(uint1* output, const uint4* input, uint4 len);
	static void decode(uint4* output, const uint1* input, uint4 len);

	static inline uint4 rotate_left(uint4 x, uint4 n);

	static inline uint4 F(uint4 x, uint4 y, uint4 z);
	static inline uint4 G(uint4 x, uint4 y, uint4 z);
	static inline uint4 H(uint4 x, uint4 y, uint4 z);
	static inline uint4 I(uint4 x, uint4 y, uint4 z);

	static inline void FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
};

// Constantes MD5
#define S11 7
#define S12 12
#define S13 17
#define S14 22

#define S21 5
#define S22 9
#define S23 14
#define S24 20

#define S31 4
#define S32 11
#define S33 16
#define S34 23

#define S41 6
#define S42 10
#define S43 15
#define S44 21

inline MD5::uint4 MD5::rotate_left(uint4 x, uint4 n)
{
	return (x << n) | (x >> (32 - n));
}

// Basic MD5 functions

inline MD5::uint4 MD5::F(uint4 x, uint4 y, uint4 z)
{
	return (x & y) | (~x & z);
}

inline MD5::uint4 MD5::G(uint4 x, uint4 y, uint4 z)
{
	return (x & z) | (y & ~z);
}

inline MD5::uint4 MD5::H(uint4 x, uint4 y, uint4 z)
{
	return x ^ y ^ z;
}

inline MD5::uint4 MD5::I(uint4 x, uint4 y, uint4 z)
{
	return y ^ (x | ~z);
}

// Round transformations

inline void MD5::FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
{
	a += F(b, c, d) + x + ac;
	a = rotate_left(a, s) + b;
}

inline void MD5::GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
{
	a += G(b, c, d) + x + ac;
	a = rotate_left(a, s) + b;
}

inline void MD5::HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
{
	a += H(b, c, d) + x + ac;
	a = rotate_left(a, s) + b;
}

inline void MD5::II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
{
	a += I(b, c, d) + x + ac;
	a = rotate_left(a, s) + b;
}
