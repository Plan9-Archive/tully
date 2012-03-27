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

	ht = (htable*)mallocz(sizeof(htable*), 1);

	ht->tab = (hentry**)mallocz(size*sizeof(hentry*), 1);
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

	entry = (hentry*)mallocz(sizeof(hentry*), 1);

	// TODO: put in real linked-list stuff later
	h = hash(key->data, key->length);
	if (ht->tab[h%ht->size] == nil) {
		print("first element at this bucket\n");
		entry->key = key;
		entry->value = value;
		entry->next = entry->prev = nil;
		ht->tab[h%ht->size] = entry;
		return h;
	} else  {
		// traverse the list
		p = ht->tab[h%ht->size];
		for (;;) {
			cmp = pstringcmp(key, p->key);
			print("compare returned %d\n", cmp);
			if (!cmp) {
				// the key already exists, replace it
				print("replacing existing value\n");
				freepstring(p->key);
				freepstring(p->value);
				p->key = key;
				p->value = value;
				free(entry);
				return h;
			}
			if (p->next) {
				print("stepping to the next p\n");
				p = p->next;
			} else {
				print("linking value to the end of the list\n");
				entry->key = key;
				entry->value = value;
				entry->next = entry->prev = nil;
				p->next = entry;
				entry->prev = p;
				return h;
			}
		}
	}
	return -1;
}

pstring*
get(htable *ht, pstring *key)
{
	uint h;
	hentry *p;
	int cmp;

	h = hash(key->data, key->length);
	if (ht->tab[h%ht->size] == nil) return nil;

	p = ht->tab[h%ht->size];
	for (; p != nil;) {
		cmp = pstringcmp(key, p->key);
		if (!cmp) {
			return p->value;
		}
		p = p->next;
	}
	return nil;
}
