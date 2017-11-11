/*********************************************************************
 *
 * ibtree.h - rbtree / avl in one file
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
/* NODE MACROS                                                        */
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

#define ib_node_init(node) do { ((node)->parent) = (node); } while (0)
#define ib_node_empty(node) ((node)->parent == (node))

#define IB_AVL_HEIGHT(node)      (((int*)(&((node)->color)))[0])
#define IB_AVL_CHILD_HEIGHT(node, c)    \
	(((node)->child[c])? IB_AVL_HEIGHT((node)->child[c]) : 0)


#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------*/
/* binary search tree - node manipulation                             */
/*--------------------------------------------------------------------*/
struct ib_node *ib_node_first(struct ib_root *root);
struct ib_node *ib_node_last(struct ib_root *root);
struct ib_node *ib_node_next(struct ib_node *node);
struct ib_node *ib_node_prev(struct ib_node *node);

void ib_node_replace(struct ib_node *victim, struct ib_node *newnode,
		struct ib_root *root);

static inline void ib_node_link(struct ib_node *node, struct ib_node *parent,
		struct ib_node **ib_link) {
	node->parent = parent;
	node->color = IB_RED;
	node->child[0] = node->child[1] = NULL;
	ib_link[0] = node;
}

/* rbtree insert rebalance and erase */
void ib_node_post_insert(struct ib_node *node, struct ib_root *root);
void ib_node_erase(struct ib_node *node, struct ib_root *root);

/* avl insert rebalance and erase */
void ib_avl_node_post_insert(struct ib_node *node, struct ib_root *root);
void ib_avl_node_erase(struct ib_node *node, struct ib_root *root);


/*--------------------------------------------------------------------*/
/* rbtree / avl - node templates                                      */
/*--------------------------------------------------------------------*/
#define ib_node_find(root, what, compare_fn, result_node, result_index) do {\
		struct ib_node *__n = (root)->node; \
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


#define ib_node_add(root, newnode, compare_fn, duplicate_node) do { \
		struct ib_node **__link = &((root)->node); \
		struct ib_node *__parent = NULL; \
		struct ib_node *__duplicate = NULL; \
		int __hr = 1; \
		while (__link[0]) { \
			__parent = __link[0]; \
			__hr = (compare_fn)(newnode, __parent); \
			if (__hr == 0) { __duplicate = __parent; break; } \
			else { __link = &(__parent->child[(__hr < 0)? 0 : 1]); } \
		} \
		(duplicate_node) = __duplicate; \
		if (__duplicate == NULL) { \
			ib_node_link(newnode, __parent, __link); \
			ib_node_post_insert(newnode, root); \
		} \
	}   while (0)

#define ib_avl_node_add(root, newnode, compare_fn, duplicate_node) do { \
		struct ib_node **__link = &((root)->node); \
		struct ib_node *__parent = NULL; \
		struct ib_node *__duplicate = NULL; \
		int __hr = 1; \
		while (__link[0]) { \
			__parent = __link[0]; \
			__hr = (compare_fn)(newnode, __parent); \
			if (__hr == 0) { __duplicate = __parent; break; } \
			else { __link = &(__parent->child[(__hr < 0)? 0 : 1]); } \
		} \
		(duplicate_node) = __duplicate; \
		if (__duplicate == NULL) { \
			ib_node_link(newnode, __parent, __link); \
			ib_avl_node_post_insert(newnode, root); \
		} \
	}   while (0)




/*--------------------------------------------------------------------*/
/* rbtree - friendly interface                                        */
/*--------------------------------------------------------------------*/
struct ib_tree
{
	struct ib_root root;		/* rbtree root */
	size_t offset;				/* node offset in user data structure */
	size_t size;                /* size of user data structure */
	size_t count;				/* node count */
	/* returns 0 for equal, -1 for n1 < n2, 1 for n1 > n2 */
	int (*compare)(const void *n1, const void *n2);
};


/* initialize rbtree, use IB_OFFSET(type, member) for "offset"
 * eg:
 *     ib_tree_init(&mytree, mystruct_compare,
 *          sizeof(struct mystruct_t), 
 *          IB_OFFSET(struct mystruct_t, node));
 */
void ib_tree_init(struct ib_tree *tree,
		int (*compare)(const void*, const void*), size_t size, size_t offset);

void *ib_tree_first(struct ib_tree *tree);
void *ib_tree_last(struct ib_tree *tree);
void *ib_tree_next(struct ib_tree *tree, void *data);
void *ib_tree_prev(struct ib_tree *tree, void *data);

/* require a temporary user structure (data) which contains the key */
void *ib_tree_find(struct ib_tree *tree, const void *data);
void *ib_tree_nearest(struct ib_tree *tree, const void *data);

/* returns NULL for success, otherwise returns conflict node with same key */
void *ib_tree_add(struct ib_tree *tree, void *data);

void ib_tree_remove(struct ib_tree *tree, void *data);
void ib_tree_replace(struct ib_tree *tree, void *victim, void *newdata);

void ib_tree_clear(struct ib_tree *tree, void (*destroy)(void *data));


#ifdef __cplusplus
}
#endif

#endif



