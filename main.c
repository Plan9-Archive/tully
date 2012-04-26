#include <u.h>
#include <libc.h>
#include <ctype.h>
#include "dat.h"
#include "hash.h"
#include "pstring.h"

void
main()
{
	int dfd, acfd, lcfd;
	char adir[40], ldir[40];

	ht = inittable(1);

	acfd = announce("tcp!*!6379", adir);
	if (acfd < 0)
		exits("announce failed");

	for (;;) {
		lcfd = listen(adir, ldir);
		if (lcfd < 0)
			exits("listen failed");

		switch (rfork(RFPROC|RFFDG|RFMEM|RFNOWAIT)) {
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
