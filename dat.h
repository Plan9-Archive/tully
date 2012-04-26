// Data structures

// A Pascal-style string
typedef struct pstring {
	uint	length;
	unsigned char	*data;
} pstring;

// An entry in the hash table
typedef struct hentry hentry;
struct hentry {
	hentry *prev;
	hentry *next;
	pstring *key;
	pstring *value;
};

// A hash table
typedef struct htable {
	RWLock l;
	uint	size;
	hentry **tab;
} htable;

// The current client connection
typedef struct Client {
	pstring **args; // the current command with arguments
	int nargs; // the number of args for the current command
	int fd;	// the network connection
} Client;

// A command
typedef struct Command {
	char*	name;
	int		(*proc)(Client*);
	int		nargs; // number of arguments (size of the pstring array)
} Command;


// Our hash table
htable *ht;