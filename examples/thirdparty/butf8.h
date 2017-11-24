/* Branchless UTF-8 decoder
 * https://github.com/skeeto/branchless-utf8
 * This is free and unencumbered software released into the public domain.
 */
#pragma once

#include <stdint.h>
#if 0
/* Decode the next character, C, from BUF, reporting errors in E.
 *
 * Since this is a branchless decoder, four bytes will be read from the
 * buffer regardless of the actual length of the next character. This
 * means the buffer _must_ have at least three bytes of zero padding
 * following the end of the data stream.
 *
 * Errors are reported in E, which will be non-zero if the parsed
 * character was somehow invalid: invalid byte sequence, non-canonical
 * encoding, or a surrogate half.
 *
 * The function returns a pointer to the next character. When an error
 * occurs, this pointer will be a guess that depends on the particular
 * error, but it will always advance at least one byte.
 */
static void *
utf8_decode(void *buf, uint32_t *c, int *e)
{
    static const char lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };
    static const int masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
    static const uint32_t mins[] = {4194304, 0, 128, 2048, 65536};
    static const int shiftc[] = {0, 18, 12, 6, 0};
    static const int shifte[] = {0, 6, 4, 2, 0};

    unsigned char *s = (unsigned char *)buf;
    int len = lengths[s[0] >> 3];

    /* Compute the pointer to the next character early so that the next
     * iteration can start working on the next character. Neither Clang
     * nor GCC figure out this reordering on their own.
     */
    unsigned char *next = s + len + !len;

    /* Assume a four-byte character and load four bytes. Unused bits are
     * shifted out.
     */
    *c  = (uint32_t)(s[0] & masks[len]) << 18;
    *c |= (uint32_t)(s[1] & 0x3f) << 12;
    *c |= (uint32_t)(s[2] & 0x3f) <<  6;
    *c |= (uint32_t)(s[3] & 0x3f) <<  0;
    *c >>= shiftc[len];

    /* Accumulate the various error conditions. */
    *e  = (*c < mins[len]) << 6; // non-canonical encoding
    *e |= ((*c >> 11) == 0x1b) << 7;  // surrogate half?
    *e |= (*c > 0x10FFFF) << 8;  // out of range?
    *e |= (s[1] & 0xc0) >> 2;
    *e |= (s[2] & 0xc0) >> 4;
    *e |= (s[3]       ) >> 6;
    *e ^= 0x2a; // top two bits of each tail byte correct?
    *e >>= shifte[len];

    return next;
}

#endif

#define IS_IN_RANGE(c, f, l)    (((c) >= (f)) && ((c) <= (l)))
uint32_t getUTF8Next(const char * & p)
{
	// TODO: since UTF-8 is a variable-length
	// encoding, you should pass in the input
	// buffer's actual byte length so that you
	// can determine if a malformed UTF-8
	// sequence would exceed the end of the buffer...

	uint8_t c1, c2, *ptr = (uint8_t *) p;
	uint32_t uc = 0;
	int seqlen;

	/*
	  int datalen = ... available length of p ...;
	  if( datalen < 1 )
	  {
	  // malformed data, do something !!!
	  return (uint32_t) -1;
	  }
	*/

	c1 = ptr[0];

	if ((c1 & 0x80) == 0) {
		uc = (uint32_t) (c1 & 0x7F);
		seqlen = 1;
	} else if ((c1 & 0xE0) == 0xC0) {
		uc = (uint32_t) (c1 & 0x1F);
		seqlen = 2;
	} else if ((c1 & 0xF0) == 0xE0) {
		uc = (uint32_t) (c1 & 0x0F);
		seqlen = 3;
	} else if ((c1 & 0xF8) == 0xF0) {
		uc = (uint32_t) (c1 & 0x07);
		seqlen = 4;
	} else {
		// malformed data, do something !!!
		return (uint32_t) -1;
	}

	/*
	  if( seqlen > datalen )
	  {
	  // malformed data, do something !!!
	  return (uint32_t) -1;
	  }
	*/

	for (int i = 1; i < seqlen; ++i) {
		c1 = ptr[i];
		if ((c1 & 0xC0) != 0x80) {
			// malformed data, do something !!!
			return (uint32_t) -1;
		}
	}

	switch (seqlen) {
	case 2:
		c1 = ptr[0];
		if (!IS_IN_RANGE(c1, 0xC2, 0xDF)) {
			// malformed data, do something !!!
			return (uint32_t) -1;
		}
		break;

	case 3:
		c1 = ptr[0];
		c2 = ptr[1];
		if (((c1 == 0xE0) && !IS_IN_RANGE(c2, 0xA0, 0xBF)) ||
			((c1 == 0xED) && !IS_IN_RANGE(c2, 0x80, 0x9F)) ||
			(!IS_IN_RANGE(c1, 0xE1, 0xEC) && !IS_IN_RANGE(c1, 0xEE, 0xEF))) {
			// malformed data, do something !!!
			return (uint32_t) -1;
		}
		break;

	case 4:
		c1 = ptr[0];
		c2 = ptr[1];
		if (((c1 == 0xF0) && !IS_IN_RANGE(c2, 0x90, 0xBF)) ||
			((c1 == 0xF4) && !IS_IN_RANGE(c2, 0x80, 0x8F)) || !IS_IN_RANGE(c1, 0xF1, 0xF3)) {
			// malformed data, do something !!!
			return (uint32_t) -1;
		}
		break;
	}

	for (int i = 1; i < seqlen; ++i) {
		uc = ((uc << 6) | (uint32_t) (ptr[i] & 0x3F));
	}

	p += seqlen;
	return uc;
}
#undef IS_IN_RANGE
