#include "imembase.h"


struct ib_hash_node
{
	struct ib_node avlnode;
	void *key;
	iulong hash;
};

struct ib_hash_index
{
	struct ILISTHEAD node;
	struct ib_root avlroot;
};

#define IB_HASH_INIT_SIZE    8

struct ib_hash_table
{
	size_t count;
	size_t index_size;
	size_t index_mask;
	iulong (*hash)(const void *key);
	int (*compare)(const void *key1, const void *key2);
	struct ILISTHEAD head;
	struct ib_hash_index *index;
	struct ib_hash_index init[IB_HASH_INIT_SIZE];
};

void ib_hash_init(struct ib_hash_table *ht, 
		iulong (*hash)(const void *key),
		int (*compare)(const void *key1, const void *key2));

struct ib_hash_node* ib_hash_node_first(struct ib_hash_table *ht);
struct ib_hash_node* ib_hash_node_last(struct ib_hash_table *ht);

struct ib_hash_node* ib_hash_node_next(struct ib_hash_table *ht, 
		struct ib_hash_node *node);

struct ib_hash_node* ib_hash_node_prev(struct ib_hash_table *ht, 
		struct ib_hash_node *node);

static inline void ib_hash_node_key(struct ib_hash_table *ht, 
		struct ib_hash_node *node, void *key) {
	node->key = key;
	node->hash = ht->hash(key);
}

struct ib_hash_node* ib_hash_find(struct ib_hash_table *ht,
		const struct ib_hash_node *node);

struct ib_node** ib_hash_track(struct ib_hash_table *ht,
		const struct ib_hash_node *node, struct ib_node **parent);

struct ib_hash_node* ib_hash_add(struct ib_hash_table *ht,
		struct ib_hash_node *node);

void ib_hash_remove(struct ib_hash_table *ht, struct ib_hash_node *node);

void ib_hash_replace(struct ib_hash_table *ht, 
		struct ib_hash_node *victim, struct ib_hash_node *newnode);

void ib_hash_clear(struct ib_hash_table *ht,
		void (*destroy)(struct ib_hash_node *node));

void* ib_hash_swap(struct ib_hash_table *ht, void *index, size_t nbytes);



//---------------------------------------------------------------------
// 
//---------------------------------------------------------------------
void ib_hash_init(struct ib_hash_table *ht, 
		iulong (*hash)(const void *key),
		int (*compare)(const void *key1, const void *key2))
{
	size_t i;
	ht->count = 0;
	ht->index_size = IB_HASH_INIT_SIZE;
	ht->index_mask = ht->index_size - 1;
	ht->hash = hash;
	ht->compare = compare;
	ilist_init(&ht->head);
	ht->index = ht->init;
	for (i = 0; i < IB_HASH_INIT_SIZE; i++) {
		ht->index[i].avlroot.node = NULL;
		ilist_init(&(ht->index[i].node));
	}
}

struct ib_hash_node* ib_hash_node_first(struct ib_hash_table *ht)
{
	struct ILISTHEAD *head = ht->head.next;
	if (head != &ht->head) {
		struct ib_hash_index *index = 
			ilist_entry(head, struct ib_hash_index, node);
		struct ib_node *avlnode = ib_node_first(&index->avlroot);
		if (avlnode == NULL) return NULL;
		return IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
	}
	return NULL;
}

struct ib_hash_node* ib_hash_node_last(struct ib_hash_table *ht)
{
	struct ILISTHEAD *head = ht->head.prev;
	if (head != &ht->head) {
		struct ib_hash_index *index = 
			ilist_entry(head, struct ib_hash_index, node);
		struct ib_node *avlnode = ib_node_last(&index->avlroot);
		if (avlnode == NULL) return NULL;
		return IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
	}
	return NULL;
}

struct ib_hash_node* ib_hash_node_next(struct ib_hash_table *ht, 
		struct ib_hash_node *node)
{
	struct ib_node *avlnode;
	struct ib_hash_index *index;
	struct ILISTHEAD *listnode;
	if (node == NULL) return NULL;
	avlnode = ib_node_next(&node->avlnode);
	if (avlnode) {
		return IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
	}
	index = &(ht->index[node->hash & ht->index_mask]);
	listnode = index->node.next;
	if (listnode == &(ht->head)) {
		return NULL;
	}
	index = ilist_entry(listnode, struct ib_hash_index, node);
	avlnode = ib_node_first(&index->avlroot);
	if (avlnode == NULL) return NULL;
	return IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
}

struct ib_hash_node* ib_hash_node_prev(struct ib_hash_table *ht, 
		struct ib_hash_node *node)
{
	struct ib_node *avlnode;
	struct ib_hash_index *index;
	struct ILISTHEAD *listnode;
	if (node == NULL) return NULL;
	avlnode = ib_node_prev(&node->avlnode);
	if (avlnode) {
		return IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
	}
	index = &(ht->index[node->hash & ht->index_mask]);
	listnode = index->node.prev;
	if (listnode == &(ht->head)) {
		return NULL;
	}
	index = ilist_entry(listnode, struct ib_hash_index, node);
	avlnode = ib_node_last(&index->avlroot);
	if (avlnode == NULL) return NULL;
	return IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
}

struct ib_hash_node* ib_hash_find(struct ib_hash_table *ht,
		const struct ib_hash_node *node)
{
	iulong hash = node->hash;
	const void *key = node->key;
	struct ib_hash_index *index = &(ht->index[hash & ht->index_mask]);
	struct ib_node *avlnode = index->avlroot.node;
	int (*compare)(const void *, const void *) = ht->compare;
	while (avlnode) {
		struct ib_hash_node *snode = 
			IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
	#if 1
		iulong shash = snode->hash;
		if (hash == shash) {
			int hc = compare(key, snode->key);
			if (hc == 0) return snode;
			avlnode = (hc < 0)? avlnode->left : avlnode->right;
		}
		else {
			avlnode = (hash < shash)? avlnode->left : avlnode->right;
		}
	#else
		#if 0
		int hc = ht->compare(key, snode->key);
		#else
		int hc = (int)((size_t)key - (size_t)snode->key);
		#endif
		if (hc == 0) return snode;
		#if 0
		avlnode = (hc < 0)? avlnode->left : avlnode->right;
		#else
		else if (hc < 0) avlnode = avlnode->left;
		else avlnode = avlnode->right;
		#endif
	#endif
	}
	return NULL;
}

void ib_hash_remove(struct ib_hash_table *ht, struct ib_hash_node *node)
{
	struct ib_hash_index *index;
	ASSERTION(node && ht);
	ASSERTION(ib_node_empty(&node->avlnode));
	index = &ht->index[node->hash & ht->index_mask];
	if (index->avlroot.node == &node->avlnode && node->avlnode.height == 1) {
		index->avlroot.node = NULL;
		ilist_del_init(&index->node);
	}
	else {
		ib_node_erase(&node->avlnode, &index->avlroot);
	}
	ib_node_init(&node->avlnode);
	ht->count--;
}

struct ib_node** ib_hash_track(struct ib_hash_table *ht,
		const struct ib_hash_node *node, struct ib_node **parent)
{
	iulong hash = node->hash;
	const void *key = node->key;
	struct ib_hash_index *index = &(ht->index[hash & ht->index_mask]);
	struct ib_node **link = &index->avlroot.node;
	struct ib_node *p = NULL;
	int (*compare)(const void *key1, const void *key2) = ht->compare;
	parent[0] = NULL;
	while (link[0]) {
		struct ib_hash_node *snode;
		iulong shash;
		p = link[0];
		snode = IB_ENTRY(p, struct ib_hash_node, avlnode);
		shash = snode->hash;
		if (hash == shash) {
			int hc = compare(key, snode->key);
			if (hc == 0) {
				parent[0] = p;
				return NULL;
			}
			link = (hc < 0)? (&p->left) : (&p->right);
		}
		else {
			link = (hash < shash)? (&p->left) : (&p->right);
		}
	}
	parent[0] = p;
	return link;
}


struct ib_hash_node* ib_hash_add(struct ib_hash_table *ht,
		struct ib_hash_node *node)
{
	struct ib_hash_index *index = &(ht->index[node->hash & ht->index_mask]);
	if (index->avlroot.node == NULL) {
		index->avlroot.node = &node->avlnode;
		node->avlnode.parent = NULL;
		node->avlnode.left = NULL;
		node->avlnode.right = NULL;
		node->avlnode.height = 1;
		ilist_add_tail(&index->node, &ht->head);
	}
	else {
		struct ib_node **link, *parent;
		link = ib_hash_track(ht, node, &parent);
		if (link == NULL) {
			ASSERTION(parent);
			return IB_ENTRY(parent, struct ib_hash_node, avlnode);
		}
		ib_node_link(&node->avlnode, parent, link);
		ib_node_post_insert(&node->avlnode, &index->avlroot);
	}
	ht->count++;
	return NULL;
}


void ib_hash_replace(struct ib_hash_table *ht, 
		struct ib_hash_node *victim, struct ib_hash_node *newnode)
{
	struct ib_hash_index *index = &ht->index[victim->hash & ht->index_mask];
	ib_node_replace(&victim->avlnode, &newnode->avlnode, &index->avlroot);
}

void ib_hash_clear(struct ib_hash_table *ht,
		void (*destroy)(struct ib_hash_node *node))
{
	while (!ilist_is_empty(&ht->head)) {
		struct ib_hash_index *index = ilist_entry(ht->head.next, 
				struct ib_hash_index, node);
		while (index->avlroot.node != NULL) {
			struct ib_node *avlnode = index->avlroot.node;
			ib_node_erase(avlnode, &index->avlroot);
			if (destroy) {
				struct ib_hash_node *node = 
					IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
				destroy(node);
			}
		}
		ilist_del_init(&index->node);
	}
	ht->count = 0;
}


void* ib_hash_swap(struct ib_hash_table *ht, void *ptr, size_t nbytes)
{
	struct ib_hash_index *old_index = ht->index;
	struct ib_hash_index *new_index = (struct ib_hash_index*)ptr;
	size_t test_size = sizeof(struct ib_hash_index);
	size_t index_size = 1;
	struct ILISTHEAD head;
	size_t i;
	ASSERTION(nbytes >= sizeof(struct ib_hash_index));
	if (new_index == NULL) {
		if (ht->index == ht->init) {
			return NULL;
		}
		new_index = ht->init;
		index_size = IB_HASH_INIT_SIZE;
	}
	else if (new_index == old_index) {
		return old_index;
	}
	if (new_index != ht->init) {
		while (test_size < nbytes) {
			size_t next_size = test_size * 2;
			if (next_size > nbytes) break;
			test_size = next_size;
			index_size = index_size * 2;
		}
	}
	ht->index = new_index;
	ht->index_size = index_size;
	ht->index_mask = index_size - 1;
	ht->count = 0;
	for (i = 0; i < index_size; i++) {
		ht->index[i].avlroot.node = NULL;
		ilist_init(&ht->index[i].node);
	}
	ilist_replace(&ht->head, &head);
	ilist_init(&ht->head);
	while (!ilist_is_empty(&head)) {
		struct ib_hash_index *index = ilist_entry(head.next, 
				struct ib_hash_index, node);
		while (index->avlroot.node) {
			struct ib_node *avlnode = index->avlroot.node;
			struct ib_hash_node *snode;
			ib_node_erase(avlnode, &index->avlroot);
			snode = IB_ENTRY(avlnode, struct ib_hash_node, avlnode);
			snode = ib_hash_add(ht, snode);
			ASSERTION(snode == NULL);
		}
		ilist_del_init(&index->node);
	}
	return (old_index == ht->init)? NULL : old_index;
}




