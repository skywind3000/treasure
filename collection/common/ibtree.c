#include "ibtree.h"


/*====================================================================*/
/* Binary Search Tree                                                 */
/*====================================================================*/

/* LEFT is 0: walk towards left, and 1 for right */
static inline struct ib_node *_ib_node_walk(struct ib_node *node, int LEFT)
{
	int RIGHT = 1 - LEFT;
	if (node == NULL) return NULL;
	
	if (node->child[LEFT]) {
		node = node->child[LEFT];
		while (node->child[RIGHT]) 
			node = node->child[RIGHT];
	}
	else {
		while (1) {
			struct ib_node *last = node;
			node = node->parent;
			if (node == NULL) break;
			if (node->child[RIGHT] == last) break;
		}
	}

	return node;
}

/* LEFT is 0: left head, and 1 for right head */
static inline struct ib_node *_ib_node_head(struct ib_root *root, int LEFT)
{
	struct ib_node *node = root->node;
	if (node == NULL) return NULL;
	while (node->child[LEFT]) 
		node = node->child[LEFT];
	return node;
}

struct ib_node *ib_node_first(struct ib_root *root)
{
	return _ib_node_head(root, 0);
}

struct ib_node *ib_node_last(struct ib_root *root)
{
	return _ib_node_head(root, 1);
}

struct ib_node *ib_node_next(struct ib_node *node)
{
	return _ib_node_walk(node, 1);
}

struct ib_node *ib_node_prev(struct ib_node *node)
{
	return _ib_node_walk(node, 0);
}

static inline void 
_ib_child_replace(struct ib_node *oldnode, struct ib_node *newnode, 
		struct ib_node *parent, struct ib_root *root) 
{
	if (parent)
		parent->child[(parent->child[1] == oldnode)? 1 : 0] = newnode;
	else 
		root->node = newnode;
}

#if 1
int ib_node_rotate_times = 0;
#endif

static inline struct ib_node *
_ib_node_rotate(struct ib_node *node, struct ib_root *root, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct ib_node *right = node->child[RIGHT];
	struct ib_node *parent = node->parent;
	node->child[RIGHT] = right->child[LEFT];
#if 0
	ib_node_rotate_times++;
#endif
	ASSERTION(node && right);
	if (right->child[LEFT]) 
		right->child[LEFT]->parent = node;
	right->child[LEFT] = node;
	right->parent = parent;
	_ib_child_replace(node, right, parent, root);
	node->parent = right;
	return right;
}


void ib_node_replace(struct ib_node *victim, struct ib_node *newnode,
		struct ib_root *root)
{
	struct ib_node *parent = victim->parent;
	_ib_child_replace(victim, newnode, parent, root);
	if (victim->child[0]) victim->child[0]->parent = newnode;
	if (victim->child[1]) victim->child[1]->parent = newnode;
	newnode->child[0] = victim->child[0];
	newnode->child[1] = victim->child[1];
	newnode->parent = victim->parent;
	newnode->color = victim->color;
}


/*--------------------------------------------------------------------*/
/* rbree - node manipulation                                          */
/*--------------------------------------------------------------------*/

static inline struct ib_node*
_ib_node_insert_update(struct ib_root *root,
		struct ib_node *node, struct ib_node *parent, 
		struct ib_node *gparent, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct ib_node *uncle = gparent->child[RIGHT];
	if (uncle) {
		if (uncle->color == IB_RED) {
			uncle->color = IB_BLACK;
			parent->color = IB_BLACK;
			gparent->color = IB_RED;
			return gparent;
		}
	}
	if (parent->child[RIGHT] == node) {
		struct ib_node *tmp;
		_ib_node_rotate(parent, root, LEFT);
		tmp = parent;
		parent = node;
		node = tmp;
	}
	parent->color = IB_BLACK;
	gparent->color = IB_RED;
	_ib_node_rotate(gparent, root, RIGHT);
	return node;
}

void ib_node_post_insert(struct ib_node *node, struct ib_root *root)
{
	node->color = IB_RED;
	while (1) {
		struct ib_node *parent, *gparent;
		parent = node->parent;
		if (parent == NULL) break;
		if (parent->color != IB_RED) break;
		gparent = parent->parent;
		if (parent == gparent->child[0]) {
			node = _ib_node_insert_update(root, node, parent, gparent, 0);
		}
		else {
			node = _ib_node_insert_update(root, node, parent, gparent, 1);
		}
	}
	root->node->color = IB_BLACK;
}

static inline struct ib_node*
_ib_node_erase_update(struct ib_node **child, struct ib_node *parent, 
		struct ib_root *root, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct ib_node *node = child[0];
	struct ib_node *sibling = parent->child[RIGHT];
	ASSERTION(sibling);
	if (sibling->color == IB_RED) {
		sibling->color = IB_BLACK;
		parent->color = IB_RED;
		_ib_node_rotate(parent, root, LEFT);
		sibling = parent->child[RIGHT];
	}
	if (((!sibling->child[0]) || sibling->child[0]->color == IB_BLACK) &&
		((!sibling->child[1]) || sibling->child[1]->color == IB_BLACK)) {
		sibling->color = IB_RED;
		node = parent;
		child[0] = node;
		return node->parent;
	}
	if ((!sibling->child[RIGHT]) || sibling->child[RIGHT]->color != IB_RED) {
		struct ib_node *sl = sibling->child[LEFT];
		if (sl) sl->color = IB_BLACK;
		sibling->color = IB_RED;
		_ib_node_rotate(sibling, root, RIGHT);
		sibling = parent->child[RIGHT];
	}
	sibling->color = parent->color;
	parent->color = IB_BLACK;
	if (sibling->child[RIGHT])
		sibling->child[RIGHT]->color = IB_BLACK;
	_ib_node_rotate(parent, root, LEFT);
	child[0] = node;
	return NULL;
}

static inline void 
_ib_node_rebalance(struct ib_node *parent, struct ib_root *root)
{
	struct ib_node *node = NULL;
	while (parent) {
		if (node != NULL && node->color == IB_RED) break;
		if (parent->child[0] == node) {
			parent = _ib_node_erase_update(&node, parent, root, 0);
		}
		else {
			parent = _ib_node_erase_update(&node, parent, root, 1);
		}
	}
	if (node) {
		node->color = IB_BLACK;
	}
}

void ib_node_erase(struct ib_node *node, struct ib_root *root)
{
	struct ib_node *child, *parent;
	unsigned int color;
	ASSERTION(node);
	if (node->child[0] && node->child[1]) {
		struct ib_node *old = node;
		struct ib_node *left;
		node = node->child[IB_RIGHT];
		while ((left = node->child[IB_LEFT]) != NULL)
			node = left;
		child = node->child[IB_RIGHT];
		parent = node->parent;
		color = node->color;
		if (child) {
			child->parent = parent;
		}
		_ib_child_replace(node, child, parent, root);
		if (node->parent == old)
			parent = node;
		node->child[0] = old->child[0];
		node->child[1] = old->child[1];
		node->parent = old->parent;
		node->color = old->color;
		_ib_child_replace(old, node, old->parent, root);
		old->child[IB_LEFT]->parent = node;
		if (old->child[IB_RIGHT]) {
			old->child[IB_RIGHT]->parent = node;
		}
	}
	else {
		child = node->child[(node->child[0] == NULL)? 1 : 0];
		parent = node->parent;
		color = node->color;
		/* printf("delete %d child=%d\n", ib_value(node), child? 1:0); */
		_ib_child_replace(node, child, parent, root);
		if (child) {
			child->parent = parent;
		}
	}
	/* if node has only one child, it must be red, and this node must 
	 * be black, therefore just replace the node with its child.
	 */
	if (child) {
		ASSERTION(child->color == IB_RED);
		child->color = IB_BLACK;
	}
	else if (color == IB_BLACK && parent) {
		_ib_node_rebalance(parent, root);
	}
}


/*--------------------------------------------------------------------*/
/* avl - node manipulation                                            */
/*--------------------------------------------------------------------*/

static inline int IB_MAX(int x, int y) 
{
	return (x < y)? y : x;
}

static inline void
_ib_avl_node_height_update(struct ib_node *node)
{
	int h0 = IB_AVL_CHILD_HEIGHT(node, 0);
	int h1 = IB_AVL_CHILD_HEIGHT(node, 1);
	IB_AVL_HEIGHT(node) = IB_MAX(h0, h1) + 1;
}

static inline struct ib_node *
_ib_avl_node_fix(struct ib_node *node, struct ib_root *root, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct ib_node *right = node->child[RIGHT];
	int rh0, rh1;
	ASSERTION(right);
	rh0 = IB_AVL_CHILD_HEIGHT(right, LEFT);
	rh1 = IB_AVL_CHILD_HEIGHT(right, RIGHT);
	if (rh0 > rh1) {
		right = _ib_node_rotate(right, root, RIGHT);
		_ib_avl_node_height_update(right->child[RIGHT]);
		_ib_avl_node_height_update(right);
		_ib_avl_node_height_update(node);
	}
	node = _ib_node_rotate(node, root, LEFT);
	_ib_avl_node_height_update(node->child[LEFT]);
	_ib_avl_node_height_update(node);
	return node;
}

static inline void 
_ib_avl_node_rebalance(struct ib_node *node, struct ib_root *root)
{
	while (node) {
		int h0 = (int)IB_AVL_CHILD_HEIGHT(node, 0);
		int h1 = (int)IB_AVL_CHILD_HEIGHT(node, 1);
		int height = IB_MAX(h0, h1) + 1;
		int diff = h0 - h1;
		if (IB_AVL_HEIGHT(node) != height) {
			IB_AVL_HEIGHT(node) = height;
		}	
		else if (diff >= -1 && diff <= 1) {
			break;
		}
		/* printf("rebalance %d\n", ib_value(node)); */
		if (diff <= -2) {
			node = _ib_avl_node_fix(node, root, 0);
		}
		else if (diff >= 2) {
			node = _ib_avl_node_fix(node, root, 1);
		}
		node = node->parent;
		/* printf("parent %d\n", (!node)? -1 : ib_value(node)); */
	}
}

void ib_avl_node_post_insert(struct ib_node *node, struct ib_root *root)
{
	IB_AVL_HEIGHT(node) = 0;
	_ib_avl_node_rebalance(node, root);
}

void ib_avl_node_erase(struct ib_node *node, struct ib_root *root)
{
	struct ib_node *child, *parent;
	ASSERTION(node);
	if (node->child[0] && node->child[1]) {
		struct ib_node *old = node;
		struct ib_node *left;
		node = node->child[IB_RIGHT];
		while ((left = node->child[IB_LEFT]) != NULL)
			node = left;
		child = node->child[IB_RIGHT];
		parent = node->parent;
		if (child) {
			child->parent = parent;
		}
		_ib_child_replace(node, child, parent, root);
		if (node->parent == old)
			parent = node;
		node->child[0] = old->child[0];
		node->child[1] = old->child[1];
		node->parent = old->parent;
		node->color = old->color;
		_ib_child_replace(old, node, old->parent, root);
		ASSERTION(old->child[IB_LEFT]);
		old->child[IB_LEFT]->parent = node;
		if (old->child[IB_RIGHT]) {
			old->child[IB_RIGHT]->parent = node;
		}
	}
	else {
		child = node->child[(node->child[0] == NULL)? 1 : 0];
		parent = node->parent;
		_ib_child_replace(node, child, parent, root);
		if (child) {
			child->parent = parent;
		}
	}
	if (parent) {
		_ib_avl_node_rebalance(parent, root);
	}
}



/*--------------------------------------------------------------------*/
/* rbtree - friendly interface                                        */
/*--------------------------------------------------------------------*/

void ib_tree_init(struct ib_tree *tree,
	int (*compare)(const void*, const void*), size_t size, size_t offset)
{
	tree->root.node = NULL;
	tree->offset = offset;
	tree->size = size;
	tree->count = 0;
	tree->compare = compare;
}


void *ib_tree_first(struct ib_tree *tree)
{
	struct ib_node *node = ib_node_first(&tree->root);
	if (!node) return NULL;
	return IB_NODE2DATA(node, tree->offset);
}

void *ib_tree_last(struct ib_tree *tree)
{
	struct ib_node *node = ib_node_last(&tree->root);
	if (!node) return NULL;
	return IB_NODE2DATA(node, tree->offset);
}

void *ib_tree_next(struct ib_tree *tree, void *data)
{
	struct ib_node *nn;
	if (!data) return NULL;
	nn = IB_DATA2NODE(data, tree->offset);
	nn = ib_node_next(nn);
	if (!nn) return NULL;
	return IB_NODE2DATA(nn, tree->offset);
}

void *ib_tree_prev(struct ib_tree *tree, void *data)
{
	struct ib_node *nn;
	if (!data) return NULL;
	nn = IB_DATA2NODE(data, tree->offset);
	nn = ib_node_prev(nn);
	if (!nn) return NULL;
	return IB_NODE2DATA(nn, tree->offset);
}


/* require a temporary user structure (data) which contains the key */
void *ib_tree_find(struct ib_tree *tree, const void *data)
{
	struct ib_node *n = tree->root.node;
	int (*compare)(const void*, const void*) = tree->compare;
	int offset = tree->offset;
	while (n) {
		void *nd = IB_NODE2DATA(n, offset);
		int hr = compare(data, nd);
		if (hr != 0) {
			n = n->child[(hr > 0)? IB_RIGHT : IB_LEFT];
		}
		else {
			return nd;
		}
	}
	return NULL;
}

void *ib_tree_nearest(struct ib_tree *tree, const void *data)
{
	struct ib_node *n = tree->root.node;
	struct ib_node *p = NULL;
	int (*compare)(const void*, const void*) = tree->compare;
	int offset = tree->offset;
	while (n) {
		void *nd = IB_NODE2DATA(n, offset);
		int hr = compare(data, nd);
		if (hr != 0) {
			p = n;
			n = n->child[(hr > 0)? IB_RIGHT : IB_LEFT];
		}
		else {
			return nd;
		}
	}
	return (p)? IB_NODE2DATA(p, offset) : NULL;
}


/* returns NULL for success, otherwise returns conflict node with same key */
void *ib_tree_add(struct ib_tree *tree, void *data)
{
	struct ib_node **link = &tree->root.node;
	struct ib_node *parent = NULL;
	struct ib_node *node = IB_DATA2NODE(data, tree->offset);
	int (*compare)(const void*, const void*) = tree->compare;
	int offset = tree->offset;
	while (link[0]) {
		void *pd;
		int hr;
		parent = link[0];
		pd = IB_NODE2DATA(parent, offset);
		hr = compare(data, pd);
		if (hr == 0) {
			return pd;
		}	
		else {
			link = &(parent->child[(hr > 0)? 1 : 0]);
		}
	}
	ib_node_link(node, parent, link);
	ib_node_post_insert(node, &tree->root);
	tree->count++;
	return NULL;
}


void ib_tree_remove(struct ib_tree *tree, void *data)
{
	struct ib_node *node = IB_DATA2NODE(data, tree->offset);
	if (!ib_node_empty(node)) {
		ib_node_erase(node, &tree->root);
		node->parent = node;
		tree->count--;
	}
}


void ib_tree_replace(struct ib_tree *tree, void *victim, void *newdata)
{
	struct ib_node *vicnode = IB_DATA2NODE(victim, tree->offset);
	struct ib_node *newnode = IB_DATA2NODE(newdata, tree->offset);
	ib_node_replace(vicnode, newnode, &tree->root);
	vicnode->parent = vicnode;
}


void ib_tree_clear(struct ib_tree *tree, void (*destroy)(void *data))
{
	while (1) {
		void *data;
		if (tree->root.node == NULL) break;
		data = IB_NODE2DATA(tree->root.node, tree->offset);
		ib_tree_remove(tree, data);
		if (destroy) destroy(data);
	}
}



