#include "ctoy.h"
#include "ibtree.c"
#include "e:/lab/work/inetbase.c"
#include <assert.h>

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


