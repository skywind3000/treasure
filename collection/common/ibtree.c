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


