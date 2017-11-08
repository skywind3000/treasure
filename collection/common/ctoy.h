#ifndef __CTOY_H__
#define __CTOY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ibtree.h"

typedef struct {
	void *key;
	void *val;
	struct ib_node node;
};

typedef struct {
	struct ib_tree tree;
}	ic_dict_rb;

#endif



