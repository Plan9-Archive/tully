</$objtype/mkfile

CFLAGS=-FTVw -DPLAN9
TARG=tully
OFILES=\
	hash.$O\
	plan9.$O\
	protocol.$O\
	pstring.$O\

HFILES=dat.h\
	hash.h\
	protocol.h\
	pstring.h\

BIN=/$objtype/bin
</sys/src/cmd/mkone