#ifndef MD5_H
#define MD5_H

/* Parameters of MD5. */
#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21

/**
* @Basic MD5 functions.
*
* @param there bit32.
*
* @return one bit32.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/**
* @Rotate Left.
*
* @param {num} the raw number.
*
* @param {n} rotate left n.
*
* @return the number after rotated left.
*/
#define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))

/**
* @Transformations for rounds 1, 2, 3, and 4.
*/
#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
  (a) += G ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
  (a) += I ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}

#include <string>
#include <cstring>
#include "def.h"
using std::string;

typedef unsigned char byte;
typedef unsigned int bit32;

class EXPORT_FUNCTION MD5
{
public:
	MD5(const string& message);
	const byte* getDigest();
	string toStr();

private:
	void init(const byte* input, size_t len);
	void transform(const byte block[64]);
	void encode(const bit32* input, byte* output, size_t length);
	void decode(const byte* input, bit32* output, size_t length);
private:
	bool finished;
	bit32 state[4];
	bit32 count[2];
	byte buffer[64];
	byte digest[16];
	static const byte PADDING[64];
	static const char HEX_NUMBERS[16];
};

#endif // MD5_H