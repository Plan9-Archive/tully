#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define sprint sprintf
#define print printf
#define fprint dprintf
#define OREAD O_RDONLY
#define exits(S) pthread_exit((int)(S))
#define RWLock pthread_rwlock_t

#define nil NULL

#define mallocz(a, b) malloc(a)

#define rlock pthread_rwlock_rdlock
#define runlock pthread_rwlock_unlock
#define wlock pthread_rwlock_wrlock
#define wunlock pthread_rwlock_unlock
