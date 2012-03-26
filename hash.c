#include <u.h>
#include <libc.h>
#include "dat.h"
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
	hentry *entry;

	entry = (hentry*)mallocz(sizeof(hentry*), 1);
	entry->key = key;
	entry->value = value;

	// TODO: put in real linked-list stuff later
	h = hash(key->data, key->length);
	ht->tab[h%ht->size] = entry;
	return 0;
}

pstring*
get(htable *ht, pstring *key)
{
	uint h;

	h = hash(key->data, key->length);
	if (ht->tab[h%ht->size] == nil) return nil;
	return ht->tab[h%ht->size]->value;
}
