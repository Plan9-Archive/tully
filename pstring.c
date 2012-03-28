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

	res = mallocz(sizeof(char)*s->length+1, 1);

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

	p = mallocz(sizeof(pstring), 1);

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

int
pstringcmp(pstring *s1, pstring *s2)
{
	int i, len;
	uchar c1, c2;
	uchar *d1, *d2;

	d1 = s1->data;
	d2 = s2->data;

	len = s1->length < s2->length ? s1->length : s2->length;

	for(i = 0; i < len; i++) {
		c1 = *(d1++);
		c2 = *(d2++);
		if(c1 != c2) {
			if(c1 > c2)
				return 1;
			return -1;
		}
	}

	return (s1->length == s2->length ? 0 : 1);
}

void
freepstring(pstring *s)
{
	if(s)
		free(s->data);
	free(s);
}

/*
 Create a copy of the given pstring.
*/
pstring*
clonepstring(pstring *s)
{
	pstring *r;

	r = mallocz(sizeof(pstring), 1);

	r->length = s->length;
	r->data = mallocz(sizeof(uchar)*(r->length), 1);
	memcpy(r->data, s->data, r->length);
	return r;
}
