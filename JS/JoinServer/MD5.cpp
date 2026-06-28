// MD5.cpp
#include "MD5.h"
#include "MD5_KEYVAL.h"
#include "Log.h"

bool MD5::MD5_EncodeKeyVal(const char* lpszInputStr, char* lpszOutputKeyVal, int iKeyIndex)
{
	if (lpszInputStr == nullptr || lpszOutputKeyVal == nullptr)
	{
		return false;
	}

	if (iKeyIndex < 0 || iKeyIndex >= MAX_KEY_INDEX)
	{
		return false;
	}

	const unsigned int inputlen = static_cast<unsigned int>(std::strlen(lpszInputStr));

	setmagicnum(iKeyIndex);
	update(reinterpret_cast<const unsigned char*>(lpszInputStr), inputlen);
	finalize();

	std::memcpy(lpszOutputKeyVal, digest, sizeof(digest));

	init();

	return true;
}

bool MD5::MD5_EncodeString(const char* lpszInputStr, char* lpszOutputStr, int iKeyIndex)
{
	if (lpszInputStr == nullptr || lpszOutputStr == nullptr)
	{
		return false;
	}

	if (iKeyIndex < 0 || iKeyIndex >= MAX_KEY_INDEX)
	{
		return false;
	}

	const unsigned int inputlen = static_cast<unsigned int>(std::strlen(lpszInputStr));

	setmagicnum(iKeyIndex);
	update(reinterpret_cast<const unsigned char*>(lpszInputStr), inputlen);
	finalize();

	std::memcpy(lpszOutputStr, hex_digest(), 33);

	init();

	return true;
}

bool MD5::MD5_CheckValue(const char* lpszInputStr, const char* szKeyVal, int iKeyIndex)
{
	if (lpszInputStr == nullptr || szKeyVal == nullptr)
	{
		return false;
	}

	char szBUF[16]{};

	if (!MD5_EncodeKeyVal(lpszInputStr, szBUF, iKeyIndex))
	{
		return false;
	}

	return (std::memcmp(szBUF, szKeyVal, sizeof(szBUF)) == 0);
}

void MD5::setmagicnum(int keyindex)
{
	if (keyindex < 0 || keyindex >= MAX_KEY_INDEX)
	{
		return;
	}

	const int index = keyindex * 4;

	state[0] = MD5_KEYVAL[index + 0];
	state[1] = MD5_KEYVAL[index + 1];
	state[2] = MD5_KEYVAL[index + 2];
	state[3] = MD5_KEYVAL[index + 3];
}

MD5::MD5()
{
  init();
}

void MD5::update(const uint1* input, uint4 input_length)
{
	uint4 input_index;
	uint4 buffer_index;
	uint4 buffer_space;

	if (finalized)
	{
		Log.ToFile(LogType::GENERAL, "[MD5 - update] No se puede actualizar un resumen finalizado.");
		return;
	}

	buffer_index = (count[0] >> 3) & 0x3F;

	if ((count[0] += (input_length << 3)) < (input_length << 3))
	{
		++count[1];
	}

	count[1] += (input_length >> 29);

	buffer_space = 64 - buffer_index;

	if (input_length >= buffer_space)
	{
		std::memcpy(buffer + buffer_index, input, buffer_space);

		transform(buffer);

		for (input_index = buffer_space; (input_index + 63) < input_length; input_index += 64)
		{
			transform(input + input_index);
		}

		buffer_index = 0;
	}
	else
	{
		input_index = 0;
	}

	std::memcpy(buffer + buffer_index, input + input_index, input_length - input_index);
}

void MD5::update(FILE* file)
{
	uint1 filebuffer[1024]{};

	while (true)
	{
		const size_t len = std::fread(filebuffer, 1, sizeof(filebuffer), file);

		if (len == 0)
		{
			break;
		}

		update(filebuffer, static_cast<uint4>(len));
	}

	std::fclose(file);
}

void MD5::update(std::istream& stream)
{
	uint1 istreambuffer[1024] {};

	while (stream)
	{
		stream.read(reinterpret_cast<char*>(istreambuffer), sizeof(istreambuffer));

		const std::streamsize len = stream.gcount();

		if (len > 0)
		{
			update(istreambuffer, static_cast<uint4>(len));
		}
	}
}

void MD5::update(std::ifstream& stream)
{
	uint1 ifstreambuffer[1024] {};

	while (stream)
	{
		stream.read(reinterpret_cast<char*>(ifstreambuffer), sizeof(ifstreambuffer));

		const std::streamsize len = stream.gcount();

		if (len > 0)
		{
			update(ifstreambuffer, static_cast<uint4>(len));
		}
	}
}

void MD5::finalize()
{
	static const uint1 PADDING[64] =
	{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	if (finalized)
	{
		Log.ToFile(LogType::GENERAL, "[MD5 - finalize] Resumen ya finalizado.");
		return;
	}

	uint1 bits[8];

	encode(bits, count, 8);

	const uint4 index = (count[0] >> 3) & 0x3F;
	const uint4 padLen = (index < 56) ? (56 - index) : (120 - index);

	update(PADDING, padLen);
	update(bits, 8);

	encode(digest, state, 16);

	std::memset(buffer, 0, sizeof(buffer));

	finalized = true;
}

MD5::MD5(FILE* file)
{
	init();
	update(file);
	finalize();
}

MD5::MD5(std::istream& stream)
{
	init();
	update(stream);
	finalize();
}

MD5::MD5(std::ifstream& stream)
	: MD5(static_cast<std::istream&>(stream))
{}

unsigned char* MD5::raw_digest() const
{
	if (!finalized)
	{
		Log.ToFile(LogType::GENERAL, "[MD5 - raw_digest] El resumen no se ha finalizado.");
		return nullptr;
	}

	std::memcpy(m_cRaw_digest, digest, sizeof(digest));

	return m_cRaw_digest;
}

char* MD5::hex_digest() const
{
	if (!finalized)
	{
		Log.ToFile(LogType::GENERAL, "[MD5 - hex_digest] El resumen no se ha finalizado.");
		return nullptr;
	}

	for (size_t i = 0; i < sizeof(digest); ++i)
	{
		sprintf_s(m_cHex_digest + (i * 2), 3, "%02x", static_cast<unsigned int>(digest[i]));
	}

	m_cHex_digest[32] = '\0';

	return m_cHex_digest;
}

std::ostream& operator<<(std::ostream& stream, const MD5& context)
{
	stream << context.hex_digest();
	return stream;
}

void MD5::init()
{
	finalized = false;

	count[0] = 0;
	count[1] = 0;

	state[0] = 0x67452301;
	state[1] = 0xefcdab89;
	state[2] = 0x98badcfe;
	state[3] = 0x10325476;
}

void MD5::transform(const uint1* block)
{
	uint4 a = state[0];
	uint4 b = state[1];
	uint4 c = state[2];
	uint4 d = state[3];
	uint4 x[16];

	decode(x, block, 64);

	if (finalized)
	{
		Log.ToFile(LogType::GENERAL, "[MD5 - transform] Se llamó a Transform después de finalize.");
		return;
	}

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  std::memset(x, 0, sizeof(x));
}

void MD5::encode(uint1* output, const uint4* input, uint4 len)
{
	for (uint4 i = 0, j = 0; j < len; ++i, j += 4)
	{
		output[j + 0] = static_cast<uint1>(input[i] & 0xFF);
		output[j + 1] = static_cast<uint1>((input[i] >> 8) & 0xFF);
		output[j + 2] = static_cast<uint1>((input[i] >> 16) & 0xFF);
		output[j + 3] = static_cast<uint1>((input[i] >> 24) & 0xFF);
	}
}

void MD5::decode(uint4* output, const uint1* input, uint4 len)
{
	for (uint4 i = 0, j = 0; j < len; ++i, j += 4)
	{
		output[i] =
			static_cast<uint4>(input[j + 0]) |
			(static_cast<uint4>(input[j + 1]) << 8) |
			(static_cast<uint4>(input[j + 2]) << 16) |
			(static_cast<uint4>(input[j + 3]) << 24);
	}
}


