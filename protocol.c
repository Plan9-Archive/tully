#ifdef PLAN9
#include <u.h>
#include <libc.h>
#include <ctype.h>
#else
#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <unistd.h>
#include "linux.h"
#endif

#include "dat.h"
#include "hash.h"
#include "pstring.h"

/* This comes directly from the Plan 9 libc source */
long
readn(int f, void *av, long n)
{
	char *a;
	long m, t;

	a = av;
	t = 0;
	while(t < n){
		m = read(f, a+t, n-t);
		if(m <= 0){
			if(t == 0)
				return m;
			break;
		}
		t += m;
	}
	return t;
}


void 
errorreply(Client *c, char* s)
{
	if (s) {
		fprint(c->fd, "-%s\r\n", s);
	}
}

void
statusreply(Client *c, char* s)
{
	if (s) {
		fprint(c->fd, "+%s\r\n", s);
	}
}

void
bulkreply(Client *c, pstring *s)
{
	if (s) {
		fprint(c->fd, "$%d\r\n", s->length);
		write(c->fd, s->data, s->length);
		fprint(c->fd, "\r\n");
	}
}

void
nilreply(Client *c)
{
	fprint(c->fd, "$-1\r\n");
}

int
gethandler(Client *c)
{
	pstring *r;

//	print("get: fd %d\n", fd);
	r = get(ht, c->args[1]);
	if (r == nil) {
		nilreply(c);
		return 0;
	}
	bulkreply(c, r);
	freepstring(r);
	return 0;
}

int
sethandler(Client *c)
{
//	print("set: fd %d\n", fd);
	set(ht, c->args[1], c->args[2]);
	statusreply(c, "OK");
	return 0;
}

int
quithandler(Client *c)
{
	close(c->fd);
	exits(0);
	return 0; // this is stupid, but it keeps the compiler quiet
}

Command commands[] = {
	{"get", gethandler, 1},
	{"set", sethandler, 2},
	{"quit", quithandler, 0},
};

/*
 Clean up the client, deallocating stuff if needed.
 If done is set, will close the fd and exit the current thread
*/
void
cleanclient(Client *c, int done)
{
	int i;

	if (c->args != nil)
		for (i = 0; i < c->nargs; i++) {
			freepstring(c->args[i]);
//			if (c->args[i] != nil) {
//				if (c->args[i]->data != nil)
//					free(c->args[i]->data);
//				free(c->args[i]);
//			}
		}
	free(c->args); // according to malloc(2), this is safe even if args is nil
	c->args = nil;
	if (done) {
		//print("quitting\n");
		close(c->fd);
		exits(0);
	}
}

int 
readnum(int sockd, void *vptr, int maxlen) 
{
	int n, rc;
	char	c, *buffer;

	buffer = vptr;

	for ( n = 1; n < maxlen; n++ ) {
		if ( (rc = readn(sockd, &c, 1)) == 1 ) {
			*buffer++ = c;
			if ( c == '\n' && *(buffer-2) == '\r') {
				*(buffer-1) = 0;
				break;
			}
		} else if ( rc <= 0 ) {
			close(sockd);
			exits(0);
		}
	}

	*buffer = 0;

	n = atoi(vptr);
	return n;
}

// Read a string (possibly binary) off the line. It is of the form:
// <len bytes>\r\n
// Toss out the CRLF and create a p-string from the data.
pstring*
readpstring(int fd, int len)
{
	int n;
	char c;
	pstring *result;

	result = mallocz(sizeof(pstring), 1);

	result->length = len;
	result->data = mallocz(sizeof(unsigned char)*len, 1);

	n = readn(fd, result->data, len);

	if (n != len) 
		goto readerr;

	n = readn(fd, &c, 1);
	if (n != 1 || c != '\r')
		goto readerr;

	n = readn(fd, &c, 1);
	if (n != 1 || c != '\n')
		goto readerr;

	return result;

readerr:
	free(result->data);
	free(result);
	if (n <= 0) {
		close(fd);
		exits(0);
	}
	return nil;
}

void
handler(int fd)
{
	Client c;
	int n, arglen, i;
	pstring *tmp;
	char buf[128];
	int foundcommand = 0;
	char *s;

	//print("new client fd = %d\n", fd);

	c.fd = fd;
	//c.args = mallocz(sizeof(pstring*)*3, 1); // allocate 3 slots, this should actually be big enough for anything for now

	for (;;) {
		/* All commands start with '*' followed by the # of arguments */
		n = readn(fd, buf, 1);
		if (n != 1 || buf[0] != '*') {
			if (n <= 0)
				cleanclient(&c, 1);
			errorreply(&c, "ERROR");
			cleanclient(&c, 0);
			continue;
		}

		c.nargs = readnum(fd, buf, 128);
		if (c.nargs <= 0) {
			errorreply(&c, "ERROR");
			cleanclient(&c, 0);
			continue;
		}

		c.args = mallocz(sizeof(pstring*)*c.nargs, 1);

		// Read in the arguments
		for (i = 0; i < c.nargs; i++) {
			n = readn(fd, buf, 1);
			if ((buf[0] != '$') || (n != 1)) {
				if (n <= 0) { cleanclient(&c, 1); }
				errorreply(&c, "ERROR");
				cleanclient(&c, 0);
				break;
			}
	
			arglen = readnum(fd, buf, 128);
			if (arglen <= 0) {
				errorreply(&c, "ERROR");
				cleanclient(&c, 0);
				break;
			}
	
			tmp = readpstring(fd, arglen);
			if (tmp == nil) {
				errorreply(&c, "ERROR");
				cleanclient(&c, 0);
				break;
			}
	
			c.args[i] = tmp;
		}

		// Make sure we successfully finished the loop
		// c.args will be nil if we had an error
		if (!c.args)
			continue;

		// we do all the commands as lower case for simplicity
		pstringtolower(c.args[0]);

		for (i = 0; i < (sizeof(commands)/sizeof(commands[0])); i++) {
			if (!memcmp(c.args[0]->data, commands[i].name, (c.args[0]->length < strlen(commands[i].name) ? c.args[0]->length : strlen(commands[i].name))) && c.nargs - 1 == commands[i].nargs) {
				foundcommand = 1;
				(commands[i].proc)(&c);
				break;
			}
		}
		if (!foundcommand) {
			s = pstring2cstring(c.args[0]);
			print("unknown command %s\n", s);
			free(s);
		}
		cleanclient(&c, 0);
	}
	//close(fd);
	//exits(0);
}
