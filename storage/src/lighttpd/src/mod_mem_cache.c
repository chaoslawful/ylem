/* 
Copyright (c) 2006, 2008 QUE Hongyu

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include "base.h"
#include "log.h"
#include "buffer.h"

#include "plugin.h"

#include "stat_cache.h"
#include "etag.h"
#include "response.h"
#include "status_counter.h"


#define LIGHTTPD_V14 1

#ifdef LIGHTTPD_V14
#include "splaytree.h"
#endif

#define CONFIG_MEM_CACHE_ENABLE "mem-cache.enable"
#define CONFIG_MEM_CACHE_MAX_MEMORY "mem-cache.max-memory"
#define CONFIG_MEM_CACHE_MAX_FILE_SIZE "mem-cache.max-file-size"
#define CONFIG_MEM_CACHE_LRU_REMOVE_COUNT "mem-cache.lru-remove-count"
#define CONFIG_MEM_CACHE_EXPIRE_TIME "mem-cache.expire-time"
#define CONFIG_MEM_CACHE_FILE_TYPES "mem-cache.filetypes"
#define CONFIG_MEM_CACHE_SLRU_THRESOLD "mem-cache.slru-thresold"

typedef struct {
	/* number of cache items removed by lru when memory is full */
	unsigned short lru_remove_count;
	unsigned short enable;
	short thresold;
	off_t maxmemory; /* maxium total used memory in MB */
	off_t maxfilesize; /* maxium file size will put into memory */
	unsigned int expires;
	array  *filetypes;
} plugin_config;

#define MEM_CACHE_NUM 524288 /* 2^19 */
#define LRUDEBUG 0

static int lruheader, lruend;
static unsigned long reqcount, reqhit, cachenumber;

#ifdef LIGHTTPD_V15
typedef struct tree_node {
    struct tree_node * left, * right;
    int key;
    int size;   /* maintained to be the number of nodes rooted here */

    void *data;
} splay_tree;

#define splaytree_size(x) (((x)==NULL) ? 0 : ((x)->size))

#endif
/* This macro returns the size of a node.  Unlike "x->size",     */
/* it works even if x=NULL.  The test could be avoided by using  */
/* a special version of NULL which was a real node with size 0.  */

/* use hash idea as danga's memcached */
struct cache_entry{
	short inuse;
	/* cache data */
	buffer *content;

	/* pointer for next when hash collided */
	struct cache_entry *scnext;

	/* lru info */
	unsigned int prev;
	unsigned int next;

	/* cache store time */
	time_t ct;
	/* file name */
	buffer *path;
	/* buffer to print at Last-Modified: header */
	buffer *mtime;
	/* content-type */
	buffer *content_type;
	/* etag */
	buffer *etag;
}; 

static struct cache_entry *memcache;

static float usedmemory = 0;

/* probation lru splaytree */
splay_tree *plru;
/* structure to store probation lru info */
struct probation {
	time_t startts;
	int count;
};

typedef struct {
	PLUGIN_DATA;
	
	plugin_config **config_storage;
	
	plugin_config conf; 
} plugin_data;

#ifdef LIGHTTPD_V15

#define compare(i,j) ((i)-(j))
/* This is the comparison.*/
/* Returns <0 if i<j, =0 if i=j, and >0 if i>j */

#define node_size splaytree_size

/* Splay using the key i (which may or may not be in the tree.)
 * The starting root is t, and the tree used is defined by rat
 * size fields are maintained */
static splay_tree * splaytree_splay (splay_tree *t, int i) {
	splay_tree N, *l, *r, *y;
	int comp, root_size, l_size, r_size;

	if (t == NULL) return t;
	N.left = N.right = NULL;
	l = r = &N;
	root_size = node_size(t);
	l_size = r_size = 0;

	for (;;) {
		comp = compare(i, t->key);
		if (comp < 0) {
			if (t->left == NULL) break;
			if (compare(i, t->left->key) < 0) {
				y = t->left;	/* rotate right */
				t->left = y->right;
				y->right = t;
				t->size = node_size(t->left) + node_size(t->right) + 1;
				t = y;
				if (t->left == NULL) break;
			}
			r->left = t;	/* link right */
			r = t;
			t = t->left;
			r_size += 1+node_size(r->right);
		} else if (comp > 0) {
			if (t->right == NULL) break;
			if (compare(i, t->right->key) > 0) {
				y = t->right;	/* rotate left */
				t->right = y->left;
				y->left = t;
				t->size = node_size(t->left) + node_size(t->right) + 1;
				t = y;
				if (t->right == NULL) break;
			}
			l->right = t;	/* link left */
			l = t;
			t = t->right;
			l_size += 1+node_size(l->left);
		} else {
			break;
		}
	}
	l_size += node_size(t->left);  /* Now l_size and r_size are the sizes of */
	r_size += node_size(t->right); /* the left and right trees we just built.*/
	t->size = l_size + r_size + 1;

	l->right = r->left = NULL;

	/* The following two loops correct the size fields of the right path
	 * from the left child of the root and the right path from the left
	 * child of the root.
	 */
	for (y = N.right; y != NULL; y = y->right) {
		y->size = l_size;
		l_size -= 1+node_size(y->left);
	}
	for (y = N.left; y != NULL; y = y->left) {
		y->size = r_size;
		r_size -= 1+node_size(y->right);
	}

	l->right = t->left;	/* assemble */
	r->left = t->right;
	t->left = N.right;
	t->right = N.left;

	return t;
}

static splay_tree * splaytree_insert(splay_tree * t, int i, void *data) {
	/* Insert key i into the tree t, if it is not already there.
	 * Return a pointer to the resulting tree.
	 */
	splay_tree * new;

	if (t != NULL) {
		t = splaytree_splay(t, i);
		if (compare(i, t->key)==0) {
			return t;  /* it's already there */
		}
	}
	new = (splay_tree *) malloc (sizeof (splay_tree));
	assert(new);
	if (t == NULL) {
		new->left = new->right = NULL;
	} else if (compare(i, t->key) < 0) {
		new->left = t->left;
		new->right = t;
		t->left = NULL;
		t->size = 1+node_size(t->right);
	} else {
		new->right = t->right;
		new->left = t;
		t->right = NULL;
		t->size = 1+node_size(t->left);
	}
	new->key = i;
	new->data = data;
	new->size = 1 + node_size(new->left) + node_size(new->right);
	return new;
}

static splay_tree * splaytree_delete(splay_tree *t, int i) {
	/* Deletes i from the tree if it's there.
	 * Return a pointer to the resulting tree.
	 */
	splay_tree * x;
	int tsize;

	if (t==NULL) return NULL;
	tsize = t->size;
	t = splaytree_splay(t, i);
	if (compare(i, t->key) == 0) {/* found it */
		if (t->left == NULL) {
			x = t->right;
		} else {
			x = splaytree_splay(t->left, i);
			x->right = t->right;
		}
		free(t);
		if (x != NULL) {
			x->size = tsize-1;
		}
		return x;
	} else {
		return t;						 /* It wasn't there */
	}
}


#endif

/* init cache_entry table */
static struct cache_entry *cache_entry_init(void) {
	struct cache_entry *c;
	c = (struct cache_entry *) malloc(sizeof(struct cache_entry)*(MEM_CACHE_NUM+1));
	assert(c);
	memset(c, 0, sizeof(struct cache_entry)*(MEM_CACHE_NUM+1));
	return c;
}

/* free cache_entry */
static void cache_entry_free(struct cache_entry *cache) {
	if (cache == NULL) return;
	cachenumber --;
	if (cache->content) usedmemory -= cache->content->size;
	buffer_free(cache->content);
	buffer_free(cache->content_type);
	buffer_free(cache->etag);
	buffer_free(cache->path);
	buffer_free(cache->mtime);
	memset(cache, 0, sizeof(struct cache_entry));
}

/* reset cache_entry to initial state */
static void cache_entry_reset(struct cache_entry *cache) {
	if (cache == NULL) return;
	if (cache->content == NULL) cache->content = buffer_init();
	if (cache->content_type == NULL) cache->content_type = buffer_init();
	if (cache->etag == NULL) cache->etag = buffer_init();
	if (cache->path == NULL) cache->path = buffer_init();
	if (cache->mtime == NULL) cache->mtime = buffer_init();
}

/* init the plugin data */
INIT_FUNC(mod_mem_cache_init) {
	plugin_data *p;
	
#ifdef LIGHTTPD_V15
	UNUSED(srv);
#endif
	p = calloc(1, sizeof(*p));
	memcache = cache_entry_init();
	lruheader = lruend = cachenumber = 0;
	reqcount = reqhit = 1;
	usedmemory = 0;
	plru = NULL;
	
	return p;
}

void free_cache_entry_chain(struct cache_entry *p) {
	struct cache_entry *c1, *c2;

	c1 = p;
	while(c1) {
		c2 = c1->scnext;
		cache_entry_free(c1);
		if (c1 != p) free(c1);
		c1 = c2;
	}

}

/* detroy the plugin data */
FREE_FUNC(mod_mem_cache_free) {
	plugin_data *p = p_d;
	size_t i;
	
	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;
	
	if (p->config_storage) {
		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];
			
			if (!s) continue;
			array_free(s->filetypes);
			free(s);
		}
		free(p->config_storage);
	}
	
	free(p);
	for (i = 0; i<= MEM_CACHE_NUM; i++) {
		free_cache_entry_chain(memcache+i);
	}
	free(memcache);

	return HANDLER_GO_ON;
}

/* handle plugin config and check values */

SETDEFAULTS_FUNC(mod_mem_cache_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;
	
	config_values_t cv[] = { 
		{ CONFIG_MEM_CACHE_MAX_MEMORY, NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ CONFIG_MEM_CACHE_MAX_FILE_SIZE, NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },       /* 1 */
		{ CONFIG_MEM_CACHE_LRU_REMOVE_COUNT, NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },       /* 2 */
		{ CONFIG_MEM_CACHE_ENABLE, NULL, T_CONFIG_BOOLEAN, T_CONFIG_SCOPE_CONNECTION },       /* 3 */
		{ CONFIG_MEM_CACHE_EXPIRE_TIME, NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },       /* 4 */
		{ CONFIG_MEM_CACHE_FILE_TYPES, NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       /* 5 */
		{ CONFIG_MEM_CACHE_SLRU_THRESOLD, NULL, T_CONFIG_SHORT, T_CONFIG_SCOPE_CONNECTION },       /* 6 */
		{ NULL,                         NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};
	
	if (!p) return HANDLER_ERROR;
	
	p->config_storage = calloc(1, srv->config_context->used * sizeof(specific_config *));
	
	for (i = 0; i < srv->config_context->used; i++) {
		plugin_config *s;
		
		s = calloc(1, sizeof(plugin_config));
		s->maxmemory = 256; /* 256M default */
		s->maxfilesize = 512; /* maxium 512k */
		s->lru_remove_count = 200; /* default 200 */
		s->enable = 1; /* default to cache content into memory */
		s->expires = 0; /* default to check stat at every request */
		s->filetypes = array_init();
		s->thresold = 0; /* 0 just like normal LRU algorithm */
		
		cv[0].destination = &(s->maxmemory);
		cv[1].destination = &(s->maxfilesize);
		cv[2].destination = &(s->lru_remove_count);
		cv[3].destination = &(s->enable);
		cv[4].destination = &(s->expires);
		cv[5].destination = s->filetypes;
		cv[6].destination = &(s->thresold);
		
		p->config_storage[i] = s;
	
		if (0 != config_insert_values_global(srv, ((data_config *)srv->config_context->data[i])->value, cv)) {
			return HANDLER_ERROR;
		}
		s->expires *= 60;

		if (s->thresold < 0) s->thresold = 0;
		if (s->thresold > 0)
#ifdef LIGHTTPD_V14
			status_counter_set(srv, CONST_STR_LEN("mem-cache.slru-thresold"), s->thresold);
#else
			status_counter_set(CONST_STR_LEN("mem-cache.slru-thresold"), s->thresold);
#endif
	}
	
	return HANDLER_GO_ON;
}

/* the famous DJB hash function for strings from stat_cache.c*/
static uint32_t hashme(buffer *str) {
	uint32_t hash = 5381;
	const char *s;
	for (s = str->ptr; *s; s++) {
		hash = ((hash << 5) + hash) + *s;
	}

	hash &= ~(1 << 31); /* strip the highest bit */

	return hash;
}

#define PATCH_OPTION(x) \
	p->conf.x = s->x

static int mod_mem_cache_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];
	
	PATCH_OPTION(maxmemory);
	PATCH_OPTION(maxfilesize);
	PATCH_OPTION(lru_remove_count);
	PATCH_OPTION(enable);
	PATCH_OPTION(expires);
	PATCH_OPTION(filetypes);
	PATCH_OPTION(thresold);
	
	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];
		
		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;
		
		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];
			
			if (buffer_is_equal_string(du->key, CONST_STR_LEN(CONFIG_MEM_CACHE_ENABLE))) {
				PATCH_OPTION(enable);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN(CONFIG_MEM_CACHE_MAX_FILE_SIZE))) {
				PATCH_OPTION(maxfilesize);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN(CONFIG_MEM_CACHE_MAX_MEMORY))) {
				PATCH_OPTION(maxmemory);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN(CONFIG_MEM_CACHE_FILE_TYPES))) {
				PATCH_OPTION(filetypes);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN(CONFIG_MEM_CACHE_EXPIRE_TIME))) {
				PATCH_OPTION(expires);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN(CONFIG_MEM_CACHE_LRU_REMOVE_COUNT))) {
				PATCH_OPTION(lru_remove_count);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN(CONFIG_MEM_CACHE_SLRU_THRESOLD))) {
				PATCH_OPTION(thresold);
			}
		}
	}
	
	return 0;
}

#if LRUDEBUG
static void print_static_lru(server *srv) {
	int d1;
	struct cache_entry *g, *g2;

	if (lruheader == 0 || lruend == 0) return;
	d1 = lruheader;
	TRACE("total lru number = %d, total memory used = %ld", cachenumber, (long) usedmemory);
	while(d1) {
		g = memcache + d1;
		if(g->content) 
			TRACE("list %d: data_length %d, block size %d, path %s", d1, g->content->used, g->content->size, g->path->ptr);
		else
			TRACE("list %d: path %s", d1, g->path->ptr);
		g2 = g->scnext;
		while(g2) {
			if(g2->content) 
				TRACE("chain %d: data_length %d, block size %d, path %s", d1, g2->content->used, g2->content->size, g2->path->ptr);
			else
				TRACE("chain %d: path %s", d1, g2->path->ptr);
			g2 = g2->scnext;
		}
		d1 = memcache[d1].next;
	}
}
#endif

/* free all cache-entry and init cache_entry */
static void free_all_cache_entry(server *srv) {
	int j;

	UNUSED(srv);
	for (j = 0; j <= MEM_CACHE_NUM; j++) {
		free_cache_entry_chain(memcache+j);
	}

	memset(memcache, 0, sizeof(struct cache_entry)*(MEM_CACHE_NUM+1));
	lruheader = lruend = cachenumber = 0;
	usedmemory = 0;
#ifdef LIGHTTPD_V14
	log_error_write(srv, __FILE__, __LINE__, "s", "free all state_cache data due to data inconsistence");
	status_counter_set(srv, CONST_STR_LEN("mem-cache.usedmemory"), usedmemory);
	status_counter_set(srv, CONST_STR_LEN("mem-cache.cachenumber"), cachenumber);
#else
	TRACE("%s", "free all state_cache data due to data inconsistence");
	status_counter_set(CONST_STR_LEN("mem-cache.memory-inuse(MB)"), ((long)usedmemory)>>20);
	status_counter_set(CONST_STR_LEN("mem-cache.cached-items"), cachenumber);
#endif
}

static void free_cache_entry_by_lru(server *srv, const int num) {
	int i, d1;

	if (lruheader == 0 || lruend == 0) return;
	d1 = lruheader;
#if LRUDEBUG
	log_error_write(srv, __FILE__, __LINE__, "sdsd",
			"memory size before lru remove:", usedmemory, "cachenumber", cachenumber);
#endif
	for(i = 0; i < num; i++, d1=lruheader) {
		lruheader = memcache[d1].next;
		if (memcache[d1].inuse) {
			memcache[d1].next = memcache[d1].prev = 0;
			free_cache_entry_chain(memcache+d1);
			memcache[d1].inuse = 0;
			memset(memcache+d1, 0, sizeof(struct cache_entry));
		} else { 
			/* wrong lru data, free them all! */
			free_all_cache_entry(srv);
			break;
		}
		if (lruheader == 0) { lruheader = lruend = cachenumber = usedmemory = 0; break; }
	}
#ifdef LIGHTTPD_V14
	status_counter_set(srv, CONST_STR_LEN("mem-cache.usedmemory"), usedmemory);
	status_counter_set(srv, CONST_STR_LEN("mem-cache.cachenumber"), cachenumber);
#else
	status_counter_set(CONST_STR_LEN("mem-cache.memory-inuse(MB)"), ((long)usedmemory)>>20);
	status_counter_set(CONST_STR_LEN("mem-cache.cached-items"), cachenumber);
#endif
#if LRUDEBUG
	log_error_write(srv, __FILE__, __LINE__, "sdsdsds",
			"memory size:", usedmemory, "after remove:", i, "items", cachenumber, "remained");
#endif
}

/* update LRU lists */
static void update_lru(server *srv, int i) {
	int d1, d2;

	if (i == 0 || memcache[i].inuse == 0) return;
	if (lruheader == 0 || lruend == 0) { 
		/* first item */
		memcache[i].prev = memcache[i].next = 0;
		lruheader = lruend = i;
	} else if (i != lruend && i != lruheader){ 
		/* re-order lru list */
		d1 = memcache[i].prev;
		d2 = memcache[i].next;
		if (d1 == 0 && d2 == 0) { 
			/* new item */
			memcache[i].prev = lruend;
			memcache[i].next = 0;
			memcache[lruend].next = i;
			lruend = i;
		} else if (d1 == 0 || d2 == 0) {
			/* wrong lru , free all cached data and reset lru */
			free_all_cache_entry(srv);
		} else {
			memcache[d1].next = d2;
			memcache[d2].prev = d1;
			/* append to end of list */
			memcache[lruend].next = i;
			memcache[i].next = 0;
			memcache[i].prev = lruend;
			lruend = i;
		}
	} else if (i == lruend) { 
		/* end of lru, no change */
	} else if (i == lruheader) { 
		/* move header to the end*/
		lruheader = memcache[i].next;
		memcache[lruheader].prev = 0;
		memcache[i].prev = lruend;
		memcache[i].next = 0;
		memcache[lruend].next = i;
		lruend = i;
	}
}

/* read file content into buffer dst 
 * return 1 if failed
 */
static int readfile_into_buffer(server *srv, connection *con, int filesize, buffer *dst) {
	int ifd;

	UNUSED(srv);

	if (dst == NULL) return 1;
	if (dst->size <= (size_t) filesize) return 1;
	if (-1 == (ifd = open(con->physical.path->ptr, O_RDONLY | O_BINARY))) {
#ifdef LIGHTTPD_V14
		log_error_write(srv, __FILE__, __LINE__, "sbss", "opening plain-file", 
				con->physical.path, "failed", strerror(errno));
#else
		TRACE("fail to open %s: %s", con->physical.path->ptr, strerror(errno));
#endif
		return 1;
	}

	if (filesize == read(ifd, dst->ptr, filesize)) { 
		dst->ptr[filesize] = '\0';
		dst->used = filesize + 1;
		close(ifd); 
		return 0; 
	} else { 
#ifdef LIGHTTPD_V14
		log_error_write(srv, __FILE__, __LINE__, "sds", "fail to read all of ", 
				filesize, "bytes into memory");
#else
		TRACE("fail to read %d bytes of %s into memory", filesize, con->physical.path->ptr);
#endif
		close(ifd); 
		return 1; 
	}
}

/* if HIT + not expire, set status = 1 and return ptr
 * else if HIT but expired, set status = 0 and return ptr
 * else if not HIT, set status = 0 and return NULL
 */
static struct cache_entry *check_mem_cache(server *srv, connection *con, plugin_data *p, int *status, const uint32_t i) {
	struct cache_entry *c;
	int success = 0;

	c = memcache+i;
	*status = 0;
	
	while (c) {
		if (c->path && buffer_is_equal(c->path, con->physical.path)) {
			success = 1;
			break;
		}
		c = c->scnext;
	}

	if (success) {
		if (c->inuse && p->conf.expires 
			&& (srv->cur_ts - c->ct)  <= (time_t )p->conf.expires)
			*status = 1;
		return c;
	}

	return NULL;
}

static struct cache_entry *get_mem_cache_entry(const uint32_t hash) {
	uint32_t i;
	struct cache_entry *c1, *c2;

	i = (hash & (MEM_CACHE_NUM-1))+1;
	c1 = c2 = memcache+i;
	
	/* try to find unused item first */
	while(c1 && c1->inuse) {
		c2 = c1;
		c1 = c1->scnext;
	}
	if (c1) return c1; /* use the first unused item */
	/* we need allocate new cache_entry */
	c1 = (struct cache_entry *)malloc(sizeof(struct cache_entry));
	if (c1 == NULL) return NULL;
	memset(c1, 0, sizeof(struct cache_entry));
	/* put new cache_entry into hash table */
	c2->scnext = c1;
	return c1;
}

/* return 0 when probation->count > p->conf.thresold in 24 hours or p->conf.thresold == 0
 * otherwise return 1
 */
static int check_probation_lru(server *srv, plugin_data *p, int hash) {
	struct probation *pr;
	int status = 1;

	if (p->conf.thresold == 0) return 0;
	plru = splaytree_splay(plru, hash);
	if (plru == NULL || plru->key != hash) { /* first splaytree node or new node*/
		pr = (struct probation *) malloc(sizeof(struct probation));
		if (pr == NULL) { /* out of memory */
			return 1;
		}
		pr->count = 1;
		pr->startts = srv->cur_ts;
		plru = splaytree_insert(plru, hash, (void *) pr);
	} else { /* matched */
		pr = (struct probation *) plru->data;
		if ((srv->cur_ts - pr->startts) > 86400) {
			/* keep track of last 24 hours only */
			pr->count = 0;
			pr->startts = srv->cur_ts;
		}
		pr->count ++;
		if (pr->count > p->conf.thresold) {
			free(pr);
			plru = splaytree_delete(plru, hash);
			status = 0;
		}
	}
	return status;
}

handler_t mod_mem_cache_uri_handler(server *srv, connection *con, void *p_d) {
	plugin_data *p = p_d;
	uint32_t hash;
	int i = 0, success = 0;
	size_t m;
	stat_cache_entry *sce = NULL;
	buffer *mtime;
	data_string *ds;
	struct cache_entry *cache;
	
	/* someone else has done a decision for us */
	if (con->http_status != 0) return HANDLER_GO_ON;
	if (con->uri.path->used == 0) return HANDLER_GO_ON;
	if (con->physical.path->used == 0) return HANDLER_GO_ON;
	
	/* someone else has handled this request */
	if (con->mode != DIRECT) return HANDLER_GO_ON;
#ifdef LIGHTTPD_V14
	if (con->file_finished) return HANDLER_GO_ON;
#else
	if (con->send->is_closed) return HANDLER_GO_ON;
#endif

	/* we only handle GET, POST and HEAD */
	switch(con->request.http_method) {
	case HTTP_METHOD_GET:
	case HTTP_METHOD_POST:
	case HTTP_METHOD_HEAD:
		break;
	default:
		return HANDLER_GO_ON;
	}
	
	if (con->conf.range_requests && NULL != array_get_element(con->request.headers, "Range"))
		return HANDLER_GO_ON;

	mod_mem_cache_patch_connection(srv, con, p);
	
	if (p->conf.enable == 0|| p->conf.maxfilesize == 0) return HANDLER_GO_ON;

	if (con->conf.log_request_handling) {
 		log_error_write(srv, __FILE__, __LINE__, "s", "-- mod_mem_cache_uri_handler called");
	}

	hash = hashme(con->physical.path);
	i = (hash & (MEM_CACHE_NUM-1))+1;
	cache = check_mem_cache(srv, con, p, &success, i);
	reqcount ++;

	if (success == 0 || cache == NULL) {
		/* going to put content into cache */
		if (HANDLER_ERROR == stat_cache_get_entry(srv, con, con->physical.path, &sce)) {
			return HANDLER_GO_ON;
		}
		/* we only handline regular files */
#ifdef HAVE_LSTAT
		if ((sce->is_symlink == 1) && !con->conf.follow_symlink) {
			con->http_status = 403;
			buffer_reset(con->physical.path);
			return HANDLER_FINISHED;
		}
#endif

		if (!S_ISREG(sce->st.st_mode)) {
			return HANDLER_GO_ON;
		}
		/* check filetypes */
		for (m = 0; m < p->conf.filetypes->used; m++) {
			ds = (data_string *)p->conf.filetypes->data[m];
			if (!ds) return HANDLER_GO_ON;
			if (sce->content_type->used &&
			    strncmp(ds->value->ptr, sce->content_type->ptr, ds->value->used-1)==0)
				break;
		}
		if (m && m == p->conf.filetypes->used)
			return HANDLER_GO_ON;
		if (sce->st.st_size == 0 || ((sce->st.st_size >> 10) > p->conf.maxfilesize)) 
			return HANDLER_GO_ON;

		if (cache == NULL) {
			/* check probation lru now */
			if (check_probation_lru(srv, p, hash))
				return HANDLER_GO_ON;
			cache = get_mem_cache_entry(hash);
			if (cache == NULL) {
				/* may be out of memory, just return GO_ON */
				return HANDLER_GO_ON;
			}
		}
		etag_mutate(con->physical.etag, sce->etag);

		if (cache->inuse == 0 || buffer_is_equal(con->physical.etag, cache->etag) == 0) {
			/* initialze cache's buffer if needed */
			cache_entry_reset(cache);
			if (cache->content->size <= sce->st.st_size) {
				usedmemory -= cache->content->size;
				buffer_prepare_copy(cache->content, sce->st.st_size);
				usedmemory += cache->content->size;
			}
			if (readfile_into_buffer(srv, con, sce->st.st_size, cache->content)) {
				return HANDLER_GO_ON;
			}
			/* increase cachenumber if needed */
			if (cache->inuse == 0) cachenumber ++;
			cache->inuse = 1;


			if (sce->content_type->used == 0) {
				buffer_copy_string_len(cache->content_type, CONST_STR_LEN("application/octet-stream"));
			} else {
				buffer_copy_string_buffer(cache->content_type, sce->content_type);
			}
			buffer_copy_string_buffer(cache->etag, con->physical.etag);
			buffer_copy_string_buffer(cache->path, con->physical.path);
			mtime = strftime_cache_get(srv, sce->st.st_mtime);
			buffer_copy_string_buffer(cache->mtime, mtime);
			cache->ct = srv->cur_ts;
#ifdef LIGHTTPD_V14
			status_counter_set(srv, CONST_STR_LEN("mem-cache.usedmemory"), usedmemory);
			status_counter_set(srv, CONST_STR_LEN("mem-cache.cachenumber"), cachenumber);
#else
			status_counter_set(CONST_STR_LEN("mem-cache.memory-inuse(MB)"), ((long)usedmemory)>>20);
			status_counter_set(CONST_STR_LEN("mem-cache.cached-items"), cachenumber);
#endif
		} else  {
			cache->ct = srv->cur_ts;
			reqhit ++;
//			response_header_overwrite(srv, con, CONST_STR_LEN("X-Mem-Cache"), CONST_STR_LEN("by memcache"));
		}
	} else {
		reqhit ++;
//		response_header_overwrite(srv, con, CONST_STR_LEN("X-Mem-Cache"), CONST_STR_LEN("by memcache"));
	}

	if (NULL == array_get_element(con->response.headers, "Content-Type")) {
		response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(cache->content_type));
	}
	
	if (NULL == array_get_element(con->response.headers, "ETag")) {
		response_header_overwrite(srv, con, CONST_STR_LEN("ETag"), CONST_BUF_LEN(cache->etag));
	}

	/* prepare header */
	if (NULL == (ds = (data_string *)array_get_element(con->response.headers, "Last-Modified"))) {
		mtime = cache->mtime;
		response_header_overwrite(srv, con, CONST_STR_LEN("Last-Modified"), CONST_BUF_LEN(mtime));
	} else mtime = ds->value;

#ifdef LIGHTTPD_V14
	status_counter_set(srv, CONST_STR_LEN("mem-cache.hitpercent"), reqhit*100/reqcount);
#else
	status_counter_set(CONST_STR_LEN("mem-cache.hitrate(%)"), (int) (((float)reqhit/(float)reqcount)*100));
#endif
	if (HANDLER_FINISHED == http_response_handle_cachable(srv, con, mtime, cache->etag))
		return HANDLER_FINISHED;

#ifdef LIGHTTPD_V14
	chunkqueue_append_buffer(con->write_queue, cache->content);
#else
	chunkqueue_append_buffer(con->send, cache->content);
#endif
	buffer_reset(con->physical.path);
	update_lru(srv, i);
	if ((((long)usedmemory) >> 20) > p->conf.maxmemory) {
		/* free least used items */
		free_cache_entry_by_lru(srv, p->conf.lru_remove_count); 
	}
#ifdef LIGHTTPD_V14
	con->file_finished = 1;
#else
	con->send->is_closed = 1;
#endif
	
	return HANDLER_FINISHED;
}

/* this function is called at dlopen() time and inits the callbacks */

int mod_mem_cache_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("mem_cache");
	
	p->init        = mod_mem_cache_init;
	/*p->handle_physical = mod_mem_cache_uri_handler; */
	p->handle_subrequest_start = mod_mem_cache_uri_handler;
	p->set_defaults  = mod_mem_cache_set_defaults;
	p->cleanup     = mod_mem_cache_free;
	
	p->data        = NULL;
	
	return 0;
}
