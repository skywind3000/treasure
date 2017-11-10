#ifndef _PRINTT_H__
#define _PRINTT_H__

#include "ibtree.h"

#define PRINTT_MAX_STR_SIZE		1024

#define PRINTT_NODE_DIR_CHILD_LEFT		0
#define PRINTT_NODE_DIR_CHILD_RIGHT		1


void print_tree(void *node,
		void (*get_text)(const void *node, char *text),
		void* (*get_child)(const void *node, int which),
		void (*output)(const char *line));

void print_tree_console(void *node,
		void (*get_text)(const void *node, char *text),
		void* (*get_child)(const void *node, int which));

void print_tree_file(void *node,
		void (*get_text)(const void *node, char *text),
		void* (*get_child)(const void *node, int which),
		const char *filename);

#endif



