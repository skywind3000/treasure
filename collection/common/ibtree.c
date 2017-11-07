#include "ibtree.h"


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

static inline struct ib_node *
_ib_node_rotate(struct ib_node *node, struct ib_root *root, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct ib_node *right = node->child[RIGHT];
	node->child[RIGHT] = right->child[LEFT];
	ASSERTION(node && right);
	if (right->child[LEFT]) 
		right->child[LEFT]->parent = node;
	right->child[LEFT] = node;
	right->parent = node->parent;
	if (right->parent) {
		if (node == node->parent->child[LEFT]) 
			node->parent->child[LEFT] = right;
		else
			node->parent->child[RIGHT] = right;
	}
	else {
		root->node = right;
	}
	node->parent = right;
	return right;
}

static inline struct ib_node*
_ib_node_update_insert(struct ib_root *root,
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

void ib_node_insert_color(struct ib_node *node, struct ib_root *root)
{
	node->color = IB_RED;
	while (1) {
		struct ib_node *parent, *gparent;
		parent = node->parent;
		if (parent == NULL) break;
		if (parent->color != IB_RED) break;
		gparent = parent->parent;
		if (parent == gparent->child[0]) {
			node = _ib_node_update_insert(root, node, parent, gparent, 0);
		}
		else {
			node = _ib_node_update_insert(root, node, parent, gparent, 1);
		}
	}
	root->node->color = IB_BLACK;
}

static inline struct ib_node*
_ib_node_update_erase(struct ib_node *node, struct ib_root *root, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct ib_node *parent = node->parent;
	struct ib_node *sibling = parent->child[RIGHT];

	if (sibling->color == IB_RED) {
		sibling->color = IB_BLACK;
		parent->color = IB_RED;
		_ib_node_rotate(parent, root, LEFT);
		sibling = parent->child[RIGHT];
	}
	if (((!sibling->child[0]) || sibling->child[0]->color == IB_BLACK) &&
		((!sibling->child[1]) || sibling->child[1]->color == IB_BLACK)) {
		sibling->color = IB_RED;
		return parent;
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
	return root->node;
}

static inline void
_ib_erase_color(struct ib_node *node, struct ib_root *root)
{
	while (1) {
		struct ib_node *parent;
		if (node == NULL) break;
		if (node->color == IB_RED) break;
		parent = node->parent;
		if (parent == NULL) break;
		if (parent->child[0] == node) {
			node = _ib_node_update_erase(node, root, 0);
		}
		else {
			node = _ib_node_update_erase(node, root, 1);
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
		struct ib_node *left = node->child[IB_RIGHT];
		while ((left = node->child[IB_LEFT]) != NULL)
			node = left;
		child = node->child[IB_RIGHT];
		parent = node->parent;
		color = node->color;
		if (child) child->parent = parent;
		if (parent) {
			parent->child[(parent->child[1] == node)? 1 : 0] = child;
		}	else {
			root->node = child;
		}
		if (node->parent == old)
			parent = node;
		node->child[0] = old->child[0];
		node->child[1] = old->child[1];
		node->parent = old->parent;
		node->color = old->color;
		if (old->parent) {
			old->parent->child[(old->parent->child[1] == old)? 1 : 0] = node;
		}	else {
			root->node = node;
		}
		old->child[IB_LEFT]->parent = node;
		if (old->child[IB_RIGHT]) {
			old->child[IB_RIGHT]->parent = node;
		}
	}
	else {
		child = node->child[(node->child[0] == NULL)? 1 : 0];
		parent = node->parent;
		color = node->color;
		if (child) 
			child->parent = parent;
		if (parent) {
			parent->child[(parent->child[1] == node)? 1 : 0] = child;
		}	else {
			root->node = child;
		}
	}
	if (color == IB_BLACK) {
		_ib_erase_color(child, root);
	}
}

void ib_node_replace(struct ib_node *victim, struct ib_node *newnode,
		struct ib_root *root)
{
	struct ib_node *parent = victim->parent;
	if (parent) {
		parent->child[(parent->child[1] == victim)? 1 : 0] = newnode;
	}	else {
		root->node = newnode;
	}
	if (victim->child[0]) victim->child[0]->parent = newnode;
	if (victim->child[1]) victim->child[1]->parent = newnode;
	newnode->child[0] = victim->child[0];
	newnode->child[1] = victim->child[1];
	newnode->parent = victim->parent;
	newnode->color = victim->color;
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
		hr = compare(node, pd);
		if (hr == 0) {
			return pd;
		}	
		else if (hr < 0) {
			link = &(parent->child[0]);
		}
		else {
			link = &(parent->child[1]);
		}
	}
	ib_node_link(node, parent, link);
	ib_node_insert_color(node, &tree->root);
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



