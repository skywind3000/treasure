#include "ctoy.h"
#include "e:/lab/work/inetbase.c"
#include <assert.h>

#define ib_value(node) ((int*)((char*)node + sizeof(struct ib_node)))[0]


//---------------------------------------------------------------------
// internal inline
//---------------------------------------------------------------------

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


//---------------------------------------------------------------------
// 
//---------------------------------------------------------------------
static inline void 
ib_set_child(struct ib_node *parent, struct ib_node *child, int LEFT)
{
	parent->child[LEFT] = child;
	child->parent = parent;
}

void rotate_left(struct ib_node *node, struct ib_root *root)
{
	_ib_node_rotate(node, root, 0);
}

void rotate_right(struct ib_node *node, struct ib_root *root)
{
	_ib_node_rotate(node, root, 1);
}

typedef struct ib_node ib_node;


//---------------------------------------------------------------------
// help
//---------------------------------------------------------------------

ib_node *node_new(int value, ib_node *parent, ib_node *left, ib_node *right)
{
	ib_node *node = (ib_node*)malloc(sizeof(ib_node) + sizeof(int));
	node->parent = parent;
	node->child[0] = left;
	node->child[1] = right;
	if (left) left->parent = node;
	if (right) right->parent = node;
	ib_value(node) = value;
	return node;
}

void test_nodes(struct ib_root *root)
{
	ib_node *n4 = node_new(4, NULL, NULL, NULL);
	ib_node *n6 = node_new(6, NULL, NULL, NULL);
	ib_node *n5 = node_new(5, NULL, n4, n6);
	ib_node *n15 = node_new(15, NULL, NULL, NULL);
	ib_node *n30 = node_new(30, NULL, NULL, NULL);
	ib_node *n20 = node_new(20, NULL, n15, n30);
	ib_node *n10 = node_new(10, NULL, n5, n20);
	root->node = n10;
}

void print_nodes(struct ib_root *root)
{
	struct ib_node *node = _ib_node_head(root, 0);
	while (node) {
		printf("%d ", ib_value(node));
		node = _ib_node_walk(node, 1);
		if (node) printf("-> ");
	}
	printf("\n");
}

int _print_t(struct ib_node *tree, int is_left, int offset, int depth, char s[32][256])
{
    char b[20];
    int width = 5;
	int i;

    if (!tree) return 0;

    sprintf(b, "(%03d)", ib_value(tree));

    int left  = _print_t(tree->child[0],  1, offset,                depth + 1, s);
    int right = _print_t(tree->child[1],  0, offset + left + width, depth + 1, s);

#ifdef COMPACT
    for (int i = 0; i < width; i++)
        s[depth][offset + left + i] = b[i];

    if (depth && is_left) {

        for (int i = 0; i < width + right; i++)
            s[depth - 1][offset + left + width/2 + i] = '-';

        s[depth - 1][offset + left + width/2] = '.';

    } else if (depth && !is_left) {

        for (int i = 0; i < left + width; i++)
            s[depth - 1][offset - width/2 + i] = '-';

        s[depth - 1][offset + left + width/2] = '.';
    }
#else
    for (i = 0; i < width; i++)
        s[2 * depth][offset + left + i] = b[i];

    if (depth && is_left) {

        for (i = 0; i < width + right; i++)
            s[2 * depth - 1][offset + left + width/2 + i] = '-';

        s[2 * depth - 1][offset + left + width/2] = '+';
        s[2 * depth - 1][offset + left + width + right + width/2] = '+';

    } else if (depth && !is_left) {

        for (i = 0; i < left + width; i++)
            s[2 * depth - 1][offset - width/2 + i] = '-';

        s[2 * depth - 1][offset + left + width/2] = '+';
        s[2 * depth - 1][offset - width/2 - 1] = '+';
    }
#endif

    return left + width + right;
}

void print_t(struct ib_node *tree)
{
    char s[32][256];
	int i, count;
    for (i = 0; i < 32; i++)
        sprintf(s[i], "%80s", " ");

    _print_t(tree, 0, 0, 0, s);

	for (i = 0; i < 32; i++) {
		char *str = s[i];
		int j = (int)strlen(str);
		for (; j > 0; j--) {
			if (str[j - 1] != ' ') break;
		}
		str[j] = 0;
	}

	for (count = 32; count > 0; count--) {
		if (s[count - 1][0] != 0) break;
	}

    for (i = 0; i < count; i++) {
        printf("%s\n", s[i]);
	}

	printf("\n");
}


//---------------------------------------------------------------------
// construct
//---------------------------------------------------------------------
struct ib_root *new_tree_from_int(const int *array, int size)
{
	struct ib_root *root = (struct ib_root*)malloc(sizeof(struct ib_root));
	root->node = NULL;
	for (; size > 0; size--) {
		int x = *array++;
		struct ib_node *newnode = node_new(x, NULL, NULL, NULL);
		struct ib_node *node = root->node;
		if (node == NULL) {
			printf("insert root: %d\n", x);
			root->node = newnode;
		}
		else {
			struct ib_node *parent = node;
			int nv;
			printf("insert leaf: %d\n", x);
			while (node) {
				parent = node;
				nv = ib_value(node);
				if (x <= nv) 
					node = node->child[0];
				else  
					node = node->child[1];
			}
			parent->child[(x <= nv)? 0 : 1] = newnode;
			newnode->parent = parent;
		}
	}
	printf("over\n");
	return root;
}



//---------------------------------------------------------------------
// test
//---------------------------------------------------------------------

void test1()
{
	struct ib_root root;
	IUINT32 t1, t2, i;
#define TIMES	400000000
	test_nodes(&root);
	print_t(root.node);
	print_nodes(&root);
	printf("\n");
	rotate_left(root.node, &root);	
	print_t(root.node);
	rotate_right(root.node, &root);	
	print_t(root.node);
	test_nodes(&root);
	printf("ready\n");
	isleep(100);
	t1 = iclock();
	for (i = TIMES; i > 0; i--) {
		if (i & 1) {
			rotate_right(root.node, &root);
		}	else {
			rotate_left(root.node, &root);
		}
	}
	t1 = iclock() - t1;
	printf("time1: %d\n", (int)t1);
	/* print_t(root.node); */
	test_nodes(&root);
	printf("ready\n");
	isleep(100);
	t2 = iclock();
	for (i = TIMES; i > 0; i--) {
		_ib_node_rotate(root.node, &root, i & 1);
	}
	t2 = iclock() - t2;
	/* print_t(root.node); */
	printf("time2: %d\n", (int)t2);
}

void test2()
{
	int array[] = { 10, 5, 20, 4, 6, 15, 30 };
	struct ib_root *root = new_tree_from_int(array, 7);
	printf("sizeof=%d\n", sizeof(array));
	print_t(root->node);
}

int main(void)
{
	test2();
	return 0;
}

/*
 *
 */


