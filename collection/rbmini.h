/*********************************************************************
 *
 * rbmini.h - mini rbtree implementation
 *
 * NOTE:
 * for more information, please see the readme file
 *
 *********************************************************************/

#ifndef __IBTREE_H__
#define __IBTREE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_NOT_STDDEF_H
#include <stddef.h>
#endif


/*====================================================================*/
/* GLOBAL MACROS                                                      */
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
#define inline INLINE
#endif

/* you can change this by config.h or predefined macro */
#ifndef ASSERTION
#define ASSERTION(x) ((void)0)
#endif


/*====================================================================*/
/* rb_node - binary search tree (can be used in rbtree & avl)         */
/* use array for left/right child pointers to reduce cpu branches     */
/* color won't be packed into pointers (can work without alignment)   */
/*====================================================================*/
struct rb_node
{
	struct rb_node *child[2];   /* 0 for left, 1 for right, reduce branch */
	struct rb_node *parent;     /* pointing to node itself for empty node */
	unsigned int color;         /* can also be used as balance / height */
};

struct ib_root
{
	struct rb_node *node;		/* root node */
};


/*--------------------------------------------------------------------*/
/* NODE MACROS                                                        */
/*--------------------------------------------------------------------*/
#define IB_LEFT    0        /* left child index */
#define IB_RIGHT   1        /* right child index */

#define IB_RED     0
#define IB_BLACK   1

#define IB_OFFSET(TYPE, MEMBER)    ((size_t) &((TYPE *)0)->MEMBER)

#define rb_node2DATA(n, o)    ((void *)((size_t)(n) - (o)))
#define IB_DATA2NODE(d, o)    ((struct rb_node*)((size_t)(d) + (o)))

#define IB_ENTRY(ptr, type, member) \
	rb_node2DATA(ptr, IB_OFFSET(type, member))

#define rb_node_init(node) do { ((node)->parent) = (node); } while (0)
#define rb_node_empty(node) ((node)->parent == (node))


#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------*/
/* binary search tree - node manipulation                             */
/*--------------------------------------------------------------------*/
struct rb_node *rb_node_first(struct ib_root *root);
struct rb_node *rb_node_last(struct ib_root *root);
struct rb_node *rb_node_next(struct rb_node *node);
struct rb_node *rb_node_prev(struct rb_node *node);

void rb_node_replace(struct rb_node *victim, struct rb_node *newnode,
		struct ib_root *root);

static inline void rb_node_link(struct rb_node *node, struct rb_node *parent,
		struct rb_node **ib_link) {
	node->parent = parent;
	node->color = IB_RED;
	node->child[0] = node->child[1] = NULL;
	ib_link[0] = node;
}

/* rbtree insert rebalance and erase */
void rb_node_post_insert(struct rb_node *node, struct ib_root *root);
void rb_node_erase(struct rb_node *node, struct ib_root *root);


/*--------------------------------------------------------------------*/
/* rbtree - node templates                                            */
/*--------------------------------------------------------------------*/
#define rb_node_find(root, what, compare_fn, result_node, result_index) do {\
		struct rb_node *__n = (root)->node; \
		(result_node) = NULL; \
		(result_index) = 0; \
		while (__n) { \
			int __hr = (compare_fn)(what, __n); \
			(result_node) = __n; \
			if (__hr == 0) { (result_index) = -1; break; } \
			else if (__hr < 0) { __n = __n->child[0]; (result_index) = 0; } \
			else { __n = __n->child[1]; (result_index) = 1; } \
		} \
	}   while (0)


#define rb_node_add(root, newnode, compare_fn, duplicate_node) do { \
		struct rb_node **__link = &((root)->node); \
		struct rb_node *__parent = NULL; \
		struct rb_node *__duplicate = NULL; \
		int __hr = 1; \
		while (__link[0]) { \
			__parent = __link[0]; \
			__hr = (compare_fn)(newnode, __parent); \
			if (__hr == 0) { __duplicate = __parent; break; } \
			else { __link = &(__parent->child[(__hr < 0)? 0 : 1]); } \
		} \
		(duplicate_node) = __duplicate; \
		if (__duplicate == NULL) { \
			rb_node_link(newnode, __parent, __link); \
			rb_node_post_insert(newnode, root); \
		} \
	}   while (0)


#ifdef __cplusplus
}
#endif

#endif



