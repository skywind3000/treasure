//=====================================================================
//
// crtzero.c - Embedded system libc free library
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
#include "crtzero.h"


//=====================================================================
// MEMORY STD
//=====================================================================

void* _cz_memcpy(void *dst, const void *src, size_t size)
{
	const char *ss = (const char*)src;
	char *dd = (char*)dst;
	// no need to unroll, modern CPUs is capable to predict small loops
	for (; size >= 4; dd += 4, ss += 4, size -= 4) {
		*((IUINT32*)dd) = *((const IUINT32*)ss);
	}
	switch (size) {
	case 3: dd[2] = ss[2];
	case 2: *((IUINT16*)dd) = *((const IUINT16*)ss); break;
	case 1: dd[0] = ss[0]; break;
	}
	return dst;
}


void* _cz_memmove(void *dst, const void *src, size_t size)
{
	const char *ss = (const char*)src;
	char *dd = (char*)dst;
	if (dd == ss) { return dst; }
	if (dd < ss || dd >= ss + size) {
		return _cz_memcpy(dst, src, size);
	}	else {
		dd += size;
		ss += size;
		for (; size >= 4; dd -= 4, ss -= 4, size -= 4) {
			*((IUINT32*)(dd - 4)) = *((const IUINT32*)(ss - 4));
		}
		switch (size) {
		case 3: 
			*((IUINT16*)(dd - 2)) = *((const IUINT16*)(ss - 2)); 
			dd[-3] = ss[-3];
			break;
		case 2: *((IUINT16*)(dd - 2)) = *((const IUINT16*)(ss - 2)); break;
		case 1: dd[-1] = ss[-1]; break;
		}
	}
	return dst;
}


void* _cz_memset(void *dst, int ch, size_t size)
{
	IUINT32 cc = (IUINT32)(ch & 0xff);
	unsigned char *dd = (unsigned char*)dst;
	cc = (cc << 24) | (cc << 16) | (cc << 8) | cc;
	for (; size >= 4; dd += 4, size -= 4) {
		*((IUINT32*)dd) = cc;
	}
	switch (size) {
	case 3: dd[2] = (IUINT8)(cc & 0xff);
	case 2: *((IUINT16*)dd) = (IUINT16)(cc & 0xffff); break;
	case 1: dd[0] = (IUINT8)(cc & 0xff);
	}
	return dst;
}


int _cz_memcmp(const void *lhs, const void *rhs, size_t size)
{
	const unsigned char *ll = (const unsigned char*)lhs;
	const unsigned char *rr = (const unsigned char*)rhs;
	if (ll == rr) return 0;
	for (; size >= 4; ll += 4, rr += 4, size -= 4) {
		if (*((const IUINT32*)ll) != *((const IUINT32*)rr)) break;
	}
	for (; size > 0; ll++, rr++, size--) {
		if (ll[0] != rr[0]) break;
	}
	if (size > 0) {
		return (ll[0] <= rr[0])? -1 : 1;
	}
	return 0;
}



//---------------------------------------------------------------------
// default functions
//---------------------------------------------------------------------
void* (*cz_memcpy)(void *dst, const void *src, size_t size) = _cz_memcpy;
void* (*cz_memmove)(void *dst, const void *src, size_t size) = _cz_memmove;
void* (*cz_memset)(void *dst, int ch, size_t size) = _cz_memset;
int (*cz_memcmp)(const void *lhs, const void *rhs, size_t size) = _cz_memcmp;




//=====================================================================
// CHAR STD
//=====================================================================
IUINT32 cz_char_type[256] = {
};


