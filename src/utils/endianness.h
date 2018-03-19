#ifndef ENDIANNESS_H
#define ENDIANNESS_H

/* Public domain implementation for endianness detection and byte ordering on
   several platforms. In case the concept of public domain does not exist
   under your jurisdiction, you can consider it to be dual licensed
   under the MIT, Apache and WTFPL licenses.

   Grab it and drop it into your project, include it and use
   the following macros to determine endianness:

   ENDIANNESS_LE, ENDIANNESS_BE

   e.g. #if ENDIANNESS_LE ...

   or, even nicer without littering your code with #ifdefs:

   if (ENDIANNESS_BE) { big_endian_code(); } else { little_endian_code(); }

   ... since the compiler can optimize away unused branches, this makes your
   code easier to read while not loosing any of the advantage of using
   conditional compilation, plus you get a free compile-time check of the
   unused code path (rarely used conditonally compiled code paths often get
   defunct over time if nobody checks them all the time).

   To debug this header yourself, you can define ENDIANNESS_DEBUG to see
   warnings from where we take the defs for the specific target.

   If you need only the conversion functions from big to little endian
   and vice versa, you may want to #define ENDIANNESS_PORTABLE_CONVERSION
   prior to including this header. That way, when the endiannes can't be
   determined at compile time, the code will fallback to a slower,
   but portable version of those functions.
   However, if using it, it's not guaranteed that ENDIANNESS_LE/BE
   will be defined.
   Most people however need only the conversion functions in their code,
   so if you stick to them you can safely turn the portable conversion on.
*/

/* This should catch all modern GCCs and Clang */
#if (defined __BYTE_ORDER__) && (defined __ORDER_LITTLE_ENDIAN__)
# ifdef ENDIANNESS_DEBUG
#  warning "Taking endiannes from built-in __BYTE_ORDER__"
# endif
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define ENDIANNESS_LE 1
#  define ENDIANNESS_BE 0
# elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define ENDIANNESS_LE 0
#  define ENDIANNESS_BE 1
# endif
/* Try to derive from arch/compiler-specific macros */
#elif defined(_X86_) || defined(__x86_64__) || defined(__i386__) || \
      defined(__i486__) || defined(__i586__) || defined(__i686__) || \
      defined(__MIPSEL) || defined(_MIPSEL) || defined(MIPSEL) || \
      defined(__ARMEL__) || \
      (defined(__LITTLE_ENDIAN__) && __LITTLE_ENDIAN__ == 1) || \
      (defined(_LITTLE_ENDIAN) && _LITTLE_ENDIAN == 1) || \
      defined(_M_IX86) || defined(_M_AMD64) /* MSVC */
# ifdef ENDIANNESS_DEBUG
#  warning "Detected Little Endian target CPU"
# endif
# define ENDIANNESS_LE 1
# define ENDIANNESS_BE 0
#elif defined(__MIPSEB) || defined(_MIPSEB) || defined(MIPSEB) || \
      defined(__MICROBLAZEEB__) || defined(__ARMEB__) || \
      (defined(__BIG_ENDIAN__) && __BIG_ENDIAN__ == 1) || \
      (defined(_BIG_ENDIAN) && _BIG_ENDIAN == 1)
# ifdef ENDIANNESS_DEBUG
#  warning "Detected Big Endian target CPU"
# endif
# define ENDIANNESS_LE 0
# define ENDIANNESS_BE 1
/* Try to get it from a header */
#else
# if defined(__linux)
#  ifdef ENDIANNESS_DEBUG
#   warning "Taking endiannes from endian.h"
#  endif
#  include <endian.h>
# else
#  ifdef ENDIANNESS_DEBUG
#   warning "Taking endiannes from machine/endian.h"
#  endif
#  include <machine/endian.h>
# endif
#endif

#ifndef ENDIANNESS_LE
# undef ENDIANNESS_BE
# if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN)
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define ENDIANNESS_LE 1
#   define ENDIANNESS_BE 0
#  elif __BYTE_ORDER == __BIG_ENDIAN
#   define ENDIANNESS_LE 0
#   define ENDIANNESS_BE 1
#  endif
# elif defined(BYTE_ORDER) && defined(LITTLE_ENDIAN)
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define ENDIANNESS_LE 1
#   define ENDIANNESS_BE 0
#  elif BYTE_ORDER == BIG_ENDIAN
#   define ENDIANNESS_LE 0
#   define ENDIANNESS_BE 1
#  endif
# endif
#endif

/* In case the user passed one of -DENDIANNESS_LE or BE in CPPFLAS,
   set the second one too */
#if defined(ENDIANNESS_LE) && !(defined(ENDIANNESS_BE))
# if ENDIANNESS_LE == 0
#  define ENDIANNESS_BE 1
# else
#  define ENDIANNESS_BE 0
# endif
#elif defined(ENDIANNESS_BE) && !(defined(ENDIANNESS_LE))
# if ENDIANNESS_BE == 0
#  define ENDIANNESS_LE 1
# else
#  define ENDIANNESS_LE 0
# endif
#endif

#if !(defined(ENDIANNESS_LE)) && !(defined(ENDIANNESS_PORTABLE_CONVERSION))
# error "Sorry, we couldn't detect endiannes for your system! Please set -DENDIANNESS_LE=1 or 0 using your CPPFLAGS/CFLAGS and open an issue for your system on https://github.com/rofl0r/endianness.h - Thanks!"
#endif

#ifdef ENDIANNESS_DEBUG
# if ENDIANNESS_LE == 1
#  warning "Detected Little Endian target CPU"
# endif
# if ENDIANNESS_BE == 1
#  warning "Detected BIG Endian target CPU"
# endif
#endif

#include <stdint.h>
#include <limits.h>

static __inline uint16_t end_bswap16(uint16_t __x)
{
        return (__x<<8) | (__x>>8);
}

static __inline uint32_t end_bswap32(uint32_t __x)
{
        return (__x>>24) | (__x>>8&0xff00) | (__x<<8&0xff0000) | (__x<<24);
}

static __inline uint64_t end_bswap64(uint64_t __x)
{
        return ((end_bswap32(__x)+0ULL)<<32) | (end_bswap32(__x>>32));
}

static __inline uint16_t end_net2host16(uint16_t net_number)
{
	uint16_t result = 0;
	int i;
	for (i = 0; i < (int)sizeof(result); i++) {
		result <<= CHAR_BIT;
		result += (((unsigned char *)&net_number)[i] & UCHAR_MAX);
	}
	return result;
}

static __inline uint16_t end_host2net16(uint16_t native_number)
{
	uint16_t result = 0;
	int i;
	for (i = (int)sizeof(result) - 1; i >= 0; i--) {
		((unsigned char *)&result)[i] = native_number & UCHAR_MAX;
		native_number >>= CHAR_BIT;
	}
	return result;
}

static __inline uint32_t end_net2host32(uint32_t net_number)
{
	uint32_t result = 0;
	int i;
	for (i = 0; i < (int)sizeof(result); i++) {
		result <<= CHAR_BIT;
		result += (((unsigned char *)&net_number)[i] & UCHAR_MAX);
	}
	return result;
}

static __inline uint32_t end_host2net32(uint32_t native_number)
{
	uint32_t result = 0;
	int i;
	for (i = (int)sizeof(result) - 1; i >= 0; i--) {
		((unsigned char *)&result)[i] = native_number & UCHAR_MAX;
		native_number >>= CHAR_BIT;
	}
	return result;
}

static __inline uint64_t end_net2host64(uint64_t net_number)
{
	uint64_t result = 0;
	int i;
	for (i = 0; i < (int)sizeof(result); i++) {
		result <<= CHAR_BIT;
		result += (((unsigned char *)&net_number)[i] & UCHAR_MAX);
	}
	return result;
}

static __inline uint64_t end_host2net64(uint64_t native_number)
{
	uint64_t result = 0;
	int i;
	for (i = (int)sizeof(result) - 1; i >= 0; i--) {
		((unsigned char *)&result)[i] = native_number & UCHAR_MAX;
		native_number >>= CHAR_BIT;
	}
	return result;
}

#if ENDIANNESS_LE+0 == 1
# define end_htobe16(x) end_bswap16(x)
# define end_be16toh(x) end_bswap16(x)
# define end_htobe32(x) end_bswap32(x)
# define end_be32toh(x) end_bswap32(x)
# define end_htobe64(x) end_bswap64(x)
# define end_be64toh(x) end_bswap64(x)
# define end_htole16(x) (uint16_t)(x)
# define end_le16toh(x) (uint16_t)(x)
# define end_htole32(x) (uint32_t)(x)
# define end_le32toh(x) (uint32_t)(x)
# define end_htole64(x) (uint64_t)(x)
# define end_le64toh(x) (uint64_t)(x)
#elif ENDIANNESS_BE+0 == 1
# define end_htobe16(x) (uint16_t)(x)
# define end_be16toh(x) (uint16_t)(x)
# define end_htobe32(x) (uint32_t)(x)
# define end_be32toh(x) (uint32_t)(x)
# define end_htobe64(x) (uint64_t)(x)
# define end_be64toh(x) (uint64_t)(x)
# define end_htole16(x) end_bswap16(x)
# define end_le16toh(x) end_bswap16(x)
# define end_htole32(x) end_bswap32(x)
# define end_le32toh(x) end_bswap32(x)
# define end_htole64(x) end_bswap64(x)
# define end_le64toh(x) end_bswap64(x)
#else
/* Resort to slower, but neutral code */
# define end_htobe16(x)  end_host2net16(x)
# define end_be16toh(x)  end_net2host16(x)
# define end_htobe32(x)  end_host2net32(x)
# define end_be32toh(x)  end_net2host32(x)
# define end_htobe64(x)  end_host2net64(x)
# define end_be64toh(x)  end_net2host64(x)
# define end_htole16(x)  end_bswap_16(end_host2net16(x))
# define end_le16toh(x)  end_bswap_16(end_host2net16(x))
# define end_htole32(x)  end_bswap_32(end_host2net32(x))
# define end_le32toh(x)  end_bswap_32(end_host2net32(x))
# define end_htole64(x)  end_bswap_64(end_host2net64(x))
# define end_le64toh(x)  end_bswap_64(end_host2net64(x))
#endif

#define end_ntoh16(x)  end_be16toh(x)
#define end_hton16(x)  end_htobe16(x)
#define end_ntoh32(x)  end_be32toh(x)
#define end_hton32(x)  end_htobe32(x)
#define end_ntoh64(x)  end_be64toh(x)
#define end_hton64(x)  end_htobe64(x)

#endif
