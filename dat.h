// Data structures

// A Pascal-style string
typedef struct pstring {
	uint	length;
	uchar	*data;
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
	uint	size;
	hentry **tab;
} htable;

// A command from a client
typedef struct Command {
	char*	name;
	int		(*proc)(int, pstring**);
	int		nargs; // number of arguments (size of the pstring array)
} Command;
