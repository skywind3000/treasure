#ifndef __CTOY_H__
#define __CTOY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imembase.h"



typedef struct {
	union {
		struct ILISTHEAD list;
		struct ib_node rb;
	}	node;
	void *key;
	void *val;
	iulong hash;
	ilong pos;
}	ib_hash_entry;

typedef struct {
	struct ILISTHEAD list;
	struct ib_root root;
	size_t size;
	unsigned int treeify;
}	ib_hash_index;


#endif



