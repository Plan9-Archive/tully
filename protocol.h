void errorreply(Client *c, char* s);
void statusreply(Client *c, char* s);
void bulkreply(Client *c, pstring *s);
void nilreply(Client *c);
int gethandler(Client *c);
int sethandler(Client *c);
int quithandler(Client *c);
void cleanclient(Client *c, int done);
int readnum(int sockd, void *vptr, int maxlen);
pstring* readpstring(int fd, int len);
void handler(int fd);