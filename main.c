#include <u.h>
#include <libc.h>
#include <ctype.h>
#include "dat.h"
#include "hash.h"
#include "pstring.h"

htable *ht;

void 
errorreply(int fd, char* s)
{
	if (s) {
		fprint(fd, "-%s\r\n", s);
	}
}

void
statusreply(int fd, char* s)
{
	if (s) {
		fprint(fd, "+%s\r\n", s);
	}
}

void
bulkreply(int fd, pstring *s)
{
	if (s) {
		fprint(fd, "$%d\r\n", s->length);
		write(fd, s->data, s->length);
		fprint(fd, "\r\n");
	}
}

void
nilreply(int fd)
{
	fprint(fd, "$-1\r\n");
}

int
gethandler(int fd, pstring** args)
{
	char *s;
	pstring *r;

	r = get(ht, args[0]);
	if (r == nil) {
		nilreply(fd);
		return 0;
	}
	s = pstring2cstring(r);
	print("gethandler read: %s\n", s);
	bulkreply(fd, r);
	free(s);
	return 0;
}

int
sethandler(int fd, pstring** args)
{
	set(ht, args[0], args[1]);
	statusreply(fd, "OK");
	return 0;
}

Command commands[] = {
	{"get", gethandler, 1},
	{"set", sethandler, 2},
};

int 
readnum(int sockd, void *vptr, int maxlen) 
{
	int n, rc;
	char	c, *buffer;

	buffer = vptr;

	for ( n = 1; n < maxlen; n++ ) {
		if ( (rc = read(sockd, &c, 1)) == 1 ) {
			*buffer++ = c;
			if ( c == '\n' && *(buffer-2) == '\r') {
				*(buffer-1) = 0;
				break;
			}
		} else if ( rc == 0 ) {
			if ( n == 1 )
				return 0;
			else
				break;
		} else {
			exits("Error in Readline()");
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

	result = (pstring*)mallocz(sizeof(pstring), 1);

	result->length = len;
	result->data = (uchar*)mallocz(sizeof(uchar)*len, 1);

	n = read(fd, result->data, len);

	if (n != len) 
		goto readerr;

	n = read(fd, &c, 1);
	if (n != 1 || c != '\r')
		goto readerr;

	n = read(fd, &c, 1);
	if (n != 1 || c != '\n')
		goto readerr;

	return result;

readerr:
	free(result->data);
	free(result);
	return nil;
}

void
handler(int fd)
{
	int n, nargs, arglen, i;
	pstring **args;
	pstring *tmp;
	char buf[128];
	int foundcommand = 0;

	for (;;) {
		/* All commands start with '*' followed by the # of arguments */
		n = read(fd, buf, 1);
		if (n != 1 || buf[0] != '*')
			goto error;

		nargs = readnum(fd, buf, 128);
	
		args = (pstring**)mallocz(sizeof(pstring*)*nargs, 1);
	
		for (i = 0; i < nargs; i++) {
			n = read(fd, buf, 1);
			if ((buf[0] != '$') || (n != 1)) {
				goto error;
			}
	
			arglen = readnum(fd, buf, 128);
	
			tmp = readpstring(fd, arglen);
	
			if (tmp == nil)
				goto error;
	
			args[i] = tmp;
		}
	
		// we do all the commands as lower case for simplicity
		pstringtolower(args[0]);
	
		for (i = 0; i < (sizeof(commands)/sizeof(commands[0])); i++) {
			if (!memcmp(args[0]->data, commands[i].name, (args[0]->length < strlen(commands[i].name) ? args[0]->length : strlen(commands[i].name)))) {
				print("the command was %s\n", commands[i].name);
				foundcommand = 1;
				(commands[i].proc)(fd, args+1);
				break;
			}
		}
		if (!foundcommand)
			goto error;
	}
error:
	print("error reading command\n");
	errorreply(fd, "ERROR");
	close(fd);
	free(args);
	exits(0);
}

void
main()
{
	int dfd, acfd, lcfd;
	char adir[40], ldir[40];

	ht = inittable(3001);

	acfd = announce("tcp!*!3000", adir);
	if (acfd < 0)
		exits("announce failed");

	for (;;) {
		lcfd = listen(adir, ldir);
		if (lcfd < 0)
			exits("listen failed");

		switch (rfork(RFPROC|RFMEM|RFNOWAIT)) {
		case -1:
			perror("fork failed");
			close(lcfd);
			break;
		case 0:
			dfd = accept(lcfd, ldir);
			if (dfd < 0)
				exits("accept failed");

			handler(dfd);
			exits(0); // should never get here
		default:
			close(lcfd);
			break;
		}
	}
}
