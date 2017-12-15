//=====================================================================
//
// crtzero.h - Embedded system libc free library
//
// NOTE:
//
// The only thing required is libgcc which is a part of gcc itself.
// See: http://wiki.osdev.org/Libgcc
//
// Created by skywind on 2017/11/01
// Last change: 2017/11/01 00:31:10
//
//=====================================================================

#ifndef _CRTZERO_H_
#define _CRTZERO_H_

// stddef.h and limits.h is required
#include <stddef.h>
#include <limits.h>

// optional config
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


//=====================================================================
// 32 BITS UINT/INT DEFINITION 
//=====================================================================
#ifndef __INTEGER_32_BITS__
#define __INTEGER_32_BITS__
#if defined(__UINT32_TYPE__) && defined(__UINT32_TYPE__)
	typedef __UINT32_TYPE__ ISTDUINT32;
	typedef __INT32_TYPE__ ISTDINT32;
#elif defined(__UINT_FAST32_TYPE__) && defined(__INT_FAST32_TYPE__)
	typedef __UINT_FAST32_TYPE__ ISTDUINT32;
	typedef __INT_FAST32_TYPE__ ISTDINT32;
#elif defined(_WIN64) || defined(WIN64) || defined(__amd64__) || \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_IA64) || \
	defined(_M_AMD64)
	typedef unsigned int ISTDUINT32;
	typedef int ISTDINT32;
#elif defined(_WIN32) || defined(WIN32) || defined(__i386__) || \
	defined(__i386) || defined(_M_X86)
	typedef unsigned long ISTDUINT32;
	typedef long ISTDINT32;
#elif defined(__MACOS__)
	typedef UInt32 ISTDUINT32;
	typedef SInt32 ISTDINT32;
#elif defined(__APPLE__) && defined(__MACH__)
	#include <sys/types.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif defined(__BEOS__)
	#include <sys/inttypes.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif (defined(_MSC_VER) || defined(__BORLANDC__)) && (!defined(__MSDOS__))
	typedef unsigned __int32 ISTDUINT32;
	typedef __int32 ISTDINT32;
#elif defined(__GNUC__) && (__GNUC__ > 3)
	#include <stdint.h>
	typedef uint32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#else 
#include <limits.h>
#if UINT_MAX == 0xFFFFU
	typedef unsigned long ISTDUINT32; 
	typedef long ISTDINT32;
#else
	typedef unsigned int ISTDUINT32;
	typedef int ISTDINT32;
#endif
#endif
#endif


//---------------------------------------------------------------------
// UINT/INT DEFINITION
//---------------------------------------------------------------------
#ifndef __IINT8_DEFINED
#define __IINT8_DEFINED
typedef char IINT8;
#endif

#ifndef __IUINT8_DEFINED
#define __IUINT8_DEFINED
typedef unsigned char IUINT8;
#endif

#ifndef __IUINT16_DEFINED
#define __IUINT16_DEFINED
typedef unsigned short IUINT16;
#endif

#ifndef __IINT16_DEFINED
#define __IINT16_DEFINED
typedef short IINT16;
#endif

#ifndef __IINT32_DEFINED
#define __IINT32_DEFINED
typedef ISTDINT32 IINT32;
#endif

#ifndef __IUINT32_DEFINED
#define __IUINT32_DEFINED
typedef ISTDUINT32 IUINT32;
#endif

#ifndef __IINT64_DEFINED
#define __IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 IINT64;
#else
typedef long long IINT64;
#endif
#endif

#ifndef __IUINT64_DEFINED
#define __IUINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 IUINT64;
#else
typedef unsigned long long IUINT64;
#endif
#endif


//---------------------------------------------------------------------
// INLINE
//---------------------------------------------------------------------
#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif



//---------------------------------------------------------------------
// ARCH DETECT
//---------------------------------------------------------------------
#ifndef CRTZERO_ARCH_DEFINED
#define CRTZERO_ARCH_DEFINED

#if defined(__amd64) || defined(__amd64__) || defined(_M_X86_64)
	#define CRTZERO_ARCH_X86_64		1
#elif defined(__x86_64) || defined(__x86_64__) || defined(_M_AMD64)
	#define CRTZERO_ARCH_X86_64		1
#elif defined(__i386) || defined(__i386__) || defined(_M_X86)
	#define CRTZERO_ARCH_X86		1
#elif defined(__i486__) || defined(__i586__) || defined(__i686__)
	#define CRTZERO_ARCH_X86		1
#elif defined(_M_IX86) || defined(__X86__) || defined(_X86_)
	#define CRTZERO_ARCH_X86		1
#elif defined(__arm__) || defined(_ARM) || defined(_M_ARM)
	#define CRTZERO_ARCH_ARM		1
#elif defined(__arm) || defined(__TARGET_ARCH_ARM)
	#define CRTZERO_ARCH_ARM		1
#elif defined(__thumb) || defined(_M_ARMT) || defined(__TARGET_ARCH_THUMB)
	#define CRTZERO_ARCH_ARMT		1
#elif defined(__aarch64__)
	#define CRTZERO_ARCH_ARM64		1
#elif defined(__m68k__) || defined(__MC68K__)
	#define CRTZERO_ARCH_M68K		1
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__)
	#define CRTZERO_ARCH_MIPS		1
#elif defined(_R3000) || defined(_R4000) || defined(_R5900)
	#define CRTZERO_ARCH_MIPS		1
#else
	#define CRTZERO_ARCH_UNKNOW		1
#endif

#endif


//---------------------------------------------------------------------
// BITS
//---------------------------------------------------------------------
#ifndef CRTZERO_CPU_BITS
	#if CRTZERO_ARCH_X86
		#define CRTZERO_CPU_BITS	32
	#elif CRTZERO_ARCH_X86_64
		#define CRTZERO_CPU_BITS	64
	#elif CRTZERO_ARCH_ARM64
		#define CRTZERO_CPU_BITS	64
	#elif CRTZERO_ARCH_ARM
		#define CRTZERO_CPU_BITS	32
	#elif CRTZERO_ARCH_ARMT
		#define CRTZERO_CPU_BITS	16
	#else
		#define CRTZERO_CPU_BITS	32
	#endif
#endif


//---------------------------------------------------------------------
// LSB / MSB
//---------------------------------------------------------------------
#ifndef CRTZERO_CPU_MSB
    #ifdef _BIG_ENDIAN_
        #if _BIG_ENDIAN_
			#define CRTZERO_CPU_MSB		1
		#endif
	#endif
    #ifndef CRTZERO_CPU_MSB
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MISPEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
            #define CRTZERO_CPU_MSB		1
        #endif
    #endif
    #ifndef CRTZERO_CPU_MSB
        #define CRTZERO_CPU_MSB		0
    #endif
#endif

#if CRTZERO_CPU_MSB
	#define CRTZERO_CPU_LSB		0
#else
	#define CRTZERO_CPU_LSB		1
#endif


//---------------------------------------------------------------------
// Compatible
//---------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

//=====================================================================
// MEMORY STD
//=====================================================================

void* _cz_memcpy(void *dst, const void *src, size_t size);
void* _cz_memmove(void *dst, const void *src, size_t size);
void* _cz_memset(void *dst, int ch, size_t size);
int _cz_memcmp(const void *lhs, const void *rhs, size_t size);

extern void* (*cz_memcpy)(void *dst, const void *src, size_t size);
extern void* (*cz_memmove)(void *dst, const void *src, size_t size);
extern void* (*cz_memset)(void *dst, int ch, size_t size);
extern int (*cz_memcmp)(const void *lhs, const void *rhs, size_t size);



//=====================================================================
// CHAR STD
//=====================================================================
extern IUINT32 cz_char_type[];

#define CZ_UPPER	0x0001
#define CZ_LOWER	0x0002
#define CZ_DIGIT	0x0004
#define CZ_SPACE	0x0008		// HT LF VT FF CR SP
#define CZ_PUNCT	0x0010
#define CZ_CONTROL	0x0020
#define CZ_BLANK	0x0040
#define CZ_HEX		0x0080


#ifdef __cplusplus
}
#endif

#endif



