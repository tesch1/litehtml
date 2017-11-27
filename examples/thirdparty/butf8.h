/* Branchless UTF-8 decoder
 * https://github.com/skeeto/branchless-utf8
 * This is free and unencumbered software released into the public domain.
 */
#pragma once

#include <stdint.h>
#if 0
uint32_t getUTF8Next(const char * & p)
{
  // Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
  // See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
  static const uint32_t UTF8_ACCEPT = 0;
  static const uint32_t UTF8_REJECT = 12;

  static const uint8_t utf8d[] = {
    // The first part of the table maps bytes to character classes that
    // to reduce the size of the transition table and create bitmasks.
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

    // The second part is a transition table that maps a combination
    // of a state of the automaton and a character class to a state.
    0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
    12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
    12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
    12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
    12,36,12,12,12,12,12,12,12,12,12,12,
  };

  auto decode = [](uint32_t* state, uint32_t* codep, uint32_t byte) -> uint32_t {
    uint32_t type = utf8d[byte];

    *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

    *state = utf8d[256 + *state + type];
    return *state;
  };

  uint32_t codepoint = 0;
  uint32_t state = 0;
  const uint8_t * s = (const uint8_t *)p;
  for (; *s; ++s) {
    switch (decode(&state, &codepoint, *s)) {
    case UTF8_ACCEPT:
      p = (const char *)s;
      return codepoint;
    case UTF8_REJECT:
      printf("The string is not well-formed\n");
      return 0;
    }
  }

  //if (state != UTF8_ACCEPT)
  //  printf("The string is not well-formed\n");
  return 0;
}

#else

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
#endif
