// MD5.h
#pragma once
#include <windows.h>   // para wsprintf (en ANSI es wsprintfA, opera sobre char*)
#include <iostream>
#include <fstream>
#include <cassert>

class MD5 {

public:

	//---------------------------------------------
	//	Métodos agregados a MD5
	bool MD5_EncodeKeyVal(					// Genera una clave MD5 de 128 bits (16 bytes)
		// utilizando la cadena de entrada y un índice de clave (0~255)
		char* lpszInputStr,					// Cadena de entrada
		char* lpszOutputKeyVal,				// Buffer de salida
		int iKeyIndex							// Índice de clave (0~255)
	);

	bool MD5_EncodeString(					// Genera una cadena MD5 de 128 x 2 bits (32 bytes)
		// utilizando la cadena de entrada y un índice de clave (0~255)
		char* lpszInputStr,					// Cadena de entrada
		char* lpszOutputStr,					// Buffer de salida
		int iKeyIndex							// Índice de clave (0~255)
	);

	bool MD5_CheckValue(						// Verifica una clave MD5 utilizando una cadena,
		// un valor MD5 y un índice de clave (0~255)
		// (true: válido / false: inválido)
		// P.S.> El valor debe ser una clave MD5 de 128 bits,
		// no una cadena MD5 de 256 bits.
		char* lpszInputStr,					// Cadena de entrada
		char* szKeyVal,						// Valor MD5 de entrada
		int iKeyIndex							// Índice de clave (0~255)
	);


	//---------------------------------------------
	//	Métodos originales de MD5

	// Métodos para operación controlada:
	MD5();  // Inicializador simple
	void  update(unsigned char* input, unsigned int input_length);
	void  update(std::istream& stream);
	void  update(FILE* file);
	void  update(std::ifstream& stream);
	void  finalize();

	// Constructores para casos especiales.
	// Todos estos constructores finalizan el contexto MD5.
	MD5(unsigned char* string); // Calcula digest de una cadena y finaliza
	MD5(std::istream& stream);  // Calcula digest de un stream y finaliza
	MD5(FILE* file);            // Calcula digest de un archivo, lo cierra y finaliza
	MD5(std::ifstream& stream); // Calcula digest de un stream, lo cierra y finaliza

	// Métodos para obtener el resultado finalizado
	unsigned char* raw_digest();  // Digest como arreglo binario de 16 bytes
	char* hex_digest();  // Digest como cadena ASCII hexadecimal de 33 bytes
	friend std::ostream& operator<< (std::ostream&, MD5 context);



private:

	//---------------------------------------------
	//	Métodos agregados a MD5
	void setmagicnum(int keyindex);


	//---------------------------------------------
	//	Variables miembro agregadas a MD5
	unsigned char	m_cRaw_digest[16];
	char			m_cHex_digest[33];



	//---------------------------------------------
	//	Variables miembro originales de MD5

	// Primero, algunos tipos:
	typedef unsigned       int uint4; // Se asume que int tiene 4 bytes
	typedef unsigned short int uint2; // Se asume que short tiene 2 bytes
	typedef unsigned      char uint1; // Se asume que char tiene 1 byte

	// Datos privados:
	uint4 state[4];
	uint4 count[2];     // Cantidad de bits, módulo 2^64
	uint1 buffer[64];   // Buffer de entrada
	uint1 digest[16];
	uint1 finalized;

	// Métodos privados (la mayoría estáticos):
	void init();               // Llamado por todos los constructores
	void transform(uint1* buffer);  // Realiza el trabajo principal de actualización.
	// La longitud se asume implícitamente en 64 bytes.

	static void encode(uint1* dest, uint4* src, uint4 length);
	static void decode(uint4* dest, uint1* src, uint4 length);
	static void memcpy(uint1* dest, uint1* src, uint4 length);
	static void memset(uint1* start, uint1 val, uint4 length);

	static inline uint4  rotate_left(uint4 x, uint4 n);
	static inline uint4  F(uint4 x, uint4 y, uint4 z);
	static inline uint4  G(uint4 x, uint4 y, uint4 z);
	static inline uint4  H(uint4 x, uint4 y, uint4 z);
	static inline uint4  I(uint4 x, uint4 y, uint4 z);
	static inline void   FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void   GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void   HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void   II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);

};


// Constantes para la rutina MD5Transform.
// Aunque podrían usarse constantes estilo C++,
// los defines son preferibles porque evitan fácilmente conflictos de ámbito.
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

