#include "linux.h"
#include "dat.h"
#include "protocol.h"

#define MAXCLIENTS 1024

pthread_t threads[MAXCLIENTS];

int sock;

void*
handler_wrapper(void *arg)
{
	int *fd;

	fd = (int*)arg;
	handler(*fd);
}

void*
reaper(void *arg)
{
	int i;
	void *ret;
	for (;;) {
		for (i = 0; i < MAXCLIENTS; i++) {
			if (threads[i] != NULL) {
				pthread_tryjoin_np(threads[i], &ret);
			}
		}
	}
}

void
main()
{
        int fd, i;
        int on = 1;
        struct sockaddr_in servaddr;

	ht = inittable(1);

	pthread_rwlock_init(&ht->l, NULL);

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
                printf("something wicked happened\n");
                exit(-1);
        }
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(6379);

        bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));

        listen(sock, 100);

	for (;;) {
		fd = accept(sock, NULL, NULL);
		if (fd < 0)
			exits("listen failed");

		for (i = 0; i < MAXCLIENTS; i++) {
			if (threads[i] == NULL) { 
				pthread_create(&threads[i], NULL, handler_wrapper, &fd);
				break;
			}
		}
	
	}
}
