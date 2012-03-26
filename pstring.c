#include <u.h>
#include <libc.h>
#include "dat.h"

/* 
 This is a very bad idea unless you're sure of what you're converting.
 Returns a malloc'd string which needs to be freed.
*/
char *
pstring2cstring(pstring *s)
{
	char *res;

	res = (char*)mallocz(sizeof(char)*s->length+1, 1);

	memcpy(res, s->data, s->length);
	res[s->length] = 0;

	return res;
}


/* This is just a convenient way to make a pstring for testing, NOT binary safe 
  Don't forget to free the string!
*/
pstring*
cstring2pstring(char *s)
{
	pstring *p;

	p = (pstring*)mallocz(sizeof(pstring*), 1);

	p->length = strlen(s);
	p->data = (uchar*)strdup(s);
	return p;
}

void
pstringtolower(pstring *s)
{
	int i;
	for (i = 0; i < s->length; i++)
		s->data[i] = tolower(s->data[i]);;
}