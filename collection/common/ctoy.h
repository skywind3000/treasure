#ifndef __CTOY_H__
#define __CTOY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>




//---------------------------------------------------------------------
// inline
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
/* #define inline INLINE */
#endif


//---------------------------------------------------------------------
// binary search tree
//---------------------------------------------------------------------
#define IB_OFFSET(TYPE, MEMBER)    ((size_t) &((TYPE *)0)->MEMBER)

#define IB_NODE2DATA(n, o)    ((void *)((size_t)(n) - (o)))
#define IB_DATA2NODE(d, o)    ((struct ib_node*)((size_t)(d) + (o)))

#define IB_ENTRY(ptr, type, member) \
	IB_NODE2DATA(ptr, IB_OFFSET(type, member))


//---------------------------------------------------------------------
// ib_node - intergrated binary search tree
//---------------------------------------------------------------------
struct ib_node
{
	struct ib_node *child[2];	// 0 for left, 1 for right, reduce branch
	struct ib_node *parent;
	unsigned int color;			// can also be used as balance / height
};

struct ib_root
{
	struct ib_node *node;		// root node
};

#endif



