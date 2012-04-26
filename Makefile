all:
	gcc -static -pthread -o tully hash.c pstring.c protocol.c linux.c

