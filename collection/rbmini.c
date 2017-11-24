#include "rbmini.h"


/*====================================================================*/
/* Binary Search Tree                                                 */
/*====================================================================*/

/* LEFT is 0: walk towards left, and 1 for right */
static inline struct rb_node *_rb_node_walk(struct rb_node *node, int LEFT)
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
			struct rb_node *last = node;
			node = node->parent;
			if (node == NULL) break;
			if (node->child[RIGHT] == last) break;
		}
	}

	return node;
}

/* LEFT is 0: left head, and 1 for right head */
static inline struct rb_node *_rb_node_head(struct ib_root *root, int LEFT)
{
	struct rb_node *node = root->node;
	if (node == NULL) return NULL;
	while (node->child[LEFT]) 
		node = node->child[LEFT];
	return node;
}

struct rb_node *rb_node_first(struct ib_root *root)
{
	return _rb_node_head(root, 0);
}

struct rb_node *rb_node_last(struct ib_root *root)
{
	return _rb_node_head(root, 1);
}

struct rb_node *rb_node_next(struct rb_node *node)
{
	return _rb_node_walk(node, 1);
}

struct rb_node *rb_node_prev(struct rb_node *node)
{
	return _rb_node_walk(node, 0);
}

static inline void 
_ib_child_replace(struct rb_node *oldnode, struct rb_node *newnode, 
		struct rb_node *parent, struct ib_root *root) 
{
	if (parent)
		parent->child[(parent->child[1] == oldnode)? 1 : 0] = newnode;
	else 
		root->node = newnode;
}

#if 1
int rb_node_rotate_times = 0;
#endif

static inline struct rb_node *
_rb_node_rotate(struct rb_node *node, struct ib_root *root, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct rb_node *right = node->child[RIGHT];
	struct rb_node *parent = node->parent;
	node->child[RIGHT] = right->child[LEFT];
#if 0
	rb_node_rotate_times++;
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


void rb_node_replace(struct rb_node *victim, struct rb_node *newnode,
		struct ib_root *root)
{
	struct rb_node *parent = victim->parent;
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

static inline struct rb_node*
_rb_node_insert_update(struct ib_root *root,
		struct rb_node *node, struct rb_node *parent, 
		struct rb_node *gparent, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct rb_node *uncle = gparent->child[RIGHT];
	if (uncle) {
		if (uncle->color == IB_RED) {
			uncle->color = IB_BLACK;
			parent->color = IB_BLACK;
			gparent->color = IB_RED;
			return gparent;
		}
	}
	if (parent->child[RIGHT] == node) {
		struct rb_node *tmp;
		_rb_node_rotate(parent, root, LEFT);
		tmp = parent;
		parent = node;
		node = tmp;
	}
	parent->color = IB_BLACK;
	gparent->color = IB_RED;
	_rb_node_rotate(gparent, root, RIGHT);
	return node;
}

void rb_node_post_insert(struct rb_node *node, struct ib_root *root)
{
	node->color = IB_RED;
	while (1) {
		struct rb_node *parent, *gparent;
		parent = node->parent;
		if (parent == NULL) break;
		if (parent->color != IB_RED) break;
		gparent = parent->parent;
		if (parent == gparent->child[0]) {
			node = _rb_node_insert_update(root, node, parent, gparent, 0);
		}
		else {
			node = _rb_node_insert_update(root, node, parent, gparent, 1);
		}
	}
	root->node->color = IB_BLACK;
}

static inline struct rb_node*
_rb_node_erase_update(struct rb_node **child, struct rb_node *parent, 
		struct ib_root *root, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct rb_node *node = child[0];
	struct rb_node *sibling = parent->child[RIGHT];
	ASSERTION(sibling);
	if (sibling->color == IB_RED) {
		sibling->color = IB_BLACK;
		parent->color = IB_RED;
		_rb_node_rotate(parent, root, LEFT);
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
		struct rb_node *sl = sibling->child[LEFT];
		if (sl) sl->color = IB_BLACK;
		sibling->color = IB_RED;
		_rb_node_rotate(sibling, root, RIGHT);
		sibling = parent->child[RIGHT];
	}
	sibling->color = parent->color;
	parent->color = IB_BLACK;
	if (sibling->child[RIGHT])
		sibling->child[RIGHT]->color = IB_BLACK;
	_rb_node_rotate(parent, root, LEFT);
	child[0] = node;
	return NULL;
}

static inline void 
_rb_node_rebalance(struct rb_node *parent, struct ib_root *root)
{
	struct rb_node *node = NULL;
	while (parent) {
		if (node != NULL && node->color == IB_RED) break;
		if (parent->child[0] == node) {
			parent = _rb_node_erase_update(&node, parent, root, 0);
		}
		else {
			parent = _rb_node_erase_update(&node, parent, root, 1);
		}
	}
	if (node) {
		node->color = IB_BLACK;
	}
}

void rb_node_erase(struct rb_node *node, struct ib_root *root)
{
	struct rb_node *child, *parent;
	unsigned int color;
	ASSERTION(node);
	if (node->child[0] && node->child[1]) {
		struct rb_node *old = node;
		struct rb_node *left;
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
		_rb_node_rebalance(parent, root);
	}
}



