/*********************************************************************
 *
 * ibtree.h - 
 *
 * NOTE:
 * for more information, please see the readme file
 *
 *********************************************************************/

#ifndef __IBTREE_H__
#define __IBTREE_H__

#include <stddef.h>


/*====================================================================*/
/* INLINE                                                             */
/*====================================================================*/
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


/*====================================================================*/
/* ib_node - binary search tree (can be used in rbtree & avl)         */
/* use array for left/right child pointers to reduce cpu branches     */
/* color won't be packed into pointers (can work without alignment)   */
/*====================================================================*/
struct ib_node
{
	struct ib_node *child[2];   /* 0 for left, 1 for right, reduce branch */
	struct ib_node *parent;     /* pointing to node itself for empty node */
	unsigned int color;         /* can also be used as balance / height */
};

struct ib_root
{
	struct ib_node *node;		/* root node */
};


/*--------------------------------------------------------------------*/
/* macros                                                             */
/*--------------------------------------------------------------------*/
#define IB_LEFT    0        /* left child index */
#define IB_RIGHT   1        /* right child index */

#define IB_RED     0
#define IB_BLACK   1

#define IB_OFFSET(TYPE, MEMBER)    ((size_t) &((TYPE *)0)->MEMBER)

#define IB_NODE2DATA(n, o)    ((void *)((size_t)(n) - (o)))
#define IB_DATA2NODE(d, o)    ((struct ib_node*)((size_t)(d) + (o)))

#define IB_ENTRY(ptr, type, member) \
	IB_NODE2DATA(ptr, IB_OFFSET(type, member))

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------*/
/* rbtree - node manipulation                                         */
/*--------------------------------------------------------------------*/
struct ib_node *ib_node_first(struct ib_root *root);
struct ib_node *ib_node_last(struct ib_root *root);
struct ib_node *ib_node_next(struct ib_node *node);
struct ib_node *ib_node_prev(struct ib_node *node);


void ib_node_insert_color(struct ib_node *node, struct ib_root *root);
void ib_node_erase(struct ib_node *node, struct ib_root *root);

void ib_node_replace(struct ib_node *victim, struct ib_node *newnode,
		struct ib_root *root);

static inline void ib_link_node(struct ib_node *node, struct ib_node *parent,
		struct ib_node **ib_link) {
	node->parent = parent;
	node->color = IB_RED;
	node->child[0] = node->child[1] = NULL;
	ib_link[0] = node;
}


#ifdef __cplusplus
}
#endif

#endif



