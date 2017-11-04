#ifndef __CTOY_H__
#define __CTOY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>





//---------------------------------------------------------------------
// ib_node
//---------------------------------------------------------------------
struct ib_node
{
	struct ib_node *child[2];
	struct ib_node *parent;
	unsigned int color;
};

struct ib_root
{
	struct ib_node *root;
};

#endif



