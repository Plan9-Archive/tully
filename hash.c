#include <u.h>
#include <libc.h>
#include "dat.h"
#include "pstring.h"
#include "hash.h"

/* Bernstein's hash */
uint 
hash(uchar *buf, int len)
{
	unsigned int hash = 5381;

	while (len--)
		hash = ((hash << 5) + hash) + (*buf++); /* hash * 33 + c */
	return hash;
}

htable*
inittable(uint size)
{
	htable *ht;

	ht = mallocz(sizeof(htable), 1);

	ht->tab = mallocz(size*sizeof(hentry*), 1);
	ht->size = size;

	return ht;
}

/* Returns the hash */
uint
set(htable *ht, pstring *key, pstring *value)
{
	uint h;
	int cmp;
	hentry *entry;
	hentry *p;
	pstring *k, *v;
	uint ret = 0;
	pstring *old;

	// by doing this, the caller can safely free the arguments that were passed to us
	k = clonepstring(key);
	v = clonepstring(value);

	entry = mallocz(sizeof(hentry), 1);

	h = hash(key->data, key->length);

	// Lock the hash table for writing
	wlock(&ht->l);
	if (ht->tab[h%ht->size] == nil) {
		entry->key = k;
		entry->value = v;
		entry->next = entry->prev = nil;
		ht->tab[h%ht->size] = entry;
		ret = h;
	} else  {
		// traverse the list
		p = ht->tab[h%ht->size];
		for (;;) {
			cmp = pstringcmp(k, p->key);
			if (!cmp) {
				// the key already exists, replace it
				old = p->value;
				p->value = v;
				freepstring(old);
				free(entry);
				freepstring(k);
				ret = h;
				break;
			}
			if (p->next) {
				p = p->next;
			} else {
				entry->key = k;
				entry->value = v;
				entry->next = entry->prev = nil;
				p->next = entry;
				entry->prev = p;
				ret = h;
				break;
			}
		}
	}
	// unlock the table
	wunlock(&ht->l);
	return ret;
}

pstring*
get(htable *ht, pstring *key)
{
	uint h;
	hentry *p;
	int cmp;
	pstring *ret = nil;

	h = hash(key->data, key->length);

	// Lock for reading
	rlock(&ht->l);
	p = ht->tab[h%ht->size];
	for (; p != nil;) {
		cmp = pstringcmp(key, p->key);
		if (!cmp) {
			ret = clonepstring(p->value); // return a *copy* to avoid nasty frees
		}
		p = p->next;
	}
	// unlock
	runlock(&ht->l);
	return ret;
}
