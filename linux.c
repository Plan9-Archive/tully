#include "linux.h"
#include "dat.h"
#include "protocol.h"

#define MAXCLIENTS 1024

// This is a silly thing pthreads does
typedef struct pClient pClient;
struct pClient {
	int fd;
};

pthread_rwlock_t threadslock;
pthread_t threads[MAXCLIENTS];
pClient *clients[MAXCLIENTS];

int sock;

void*
handler_wrapper(void *arg)
{
	pClient *c;
	int fd;

	c = (pClient*)arg;
	fd = c->fd;
	handler(fd);
}

// This will clean up threads which have exited.
void*
reaper(void *arg)
{
	int i, joined;
	void *ret;
	for (;;) {
		wlock(&threadslock);
		for (i = 0; i < MAXCLIENTS; i++) {
			if (threads[i] != NULL) {
				joined = pthread_tryjoin_np(threads[i], &ret);
				if (joined == 0) {
					threads[i] = NULL;
					free(clients[i]);
				}
			}
		}
		wunlock(&threadslock);
		sleep(2);
	}
}

void
main(int argc, char *argv[])
{
	int fd, i;
	int tablesize = 2001;
	int on = 1;
	struct sockaddr_in servaddr;
	pthread_t reaperthread;

	if (!(argc == 1 || argc == 2)) {
		printf("usage: %s [table size]\n", argv[0]);
		exit(0);
	}

	if (argc == 2) 
		tablesize = atoi(argv[1]);

	if (tablesize <= 0) {
		printf("Invalid table size %d, quitting\n", tablesize);
		exit(1);
	}

	ht = inittable(tablesize);

	pthread_rwlock_init(&ht->l, NULL);
	pthread_rwlock_init(&threadslock, NULL);

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

	// Start the harvester of threads
	pthread_create(&reaperthread, NULL, reaper, NULL);

	for (;;) {
		fd = accept(sock, NULL, NULL);

		if (fd < 0)
			exits("listen failed");

		wlock(&threadslock);
		for (i = 0; i < MAXCLIENTS; i++) {
			if (threads[i] == NULL) { 
				clients[i] = (pClient*)malloc(sizeof(pClient));
				clients[i]->fd = fd;
				pthread_create(&threads[i], NULL, handler_wrapper, (void*)clients[i]);
				break;
			}
		}
		wunlock(&threadslock);
	}
}
