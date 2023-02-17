#ifndef LIBACA
#define LIBACA

struct aca_edge {
	struct aca_edge *next;
	int from, to;
	char sym;
};

typedef struct {
	struct aca_edge **etab;
	int *fail, *prev, *pat;
	int np, ns, maxns, ecap;
} aca;

typedef struct {
	aca *aca;
	int st;
} aca_iter;


/* Initialize the ACA with the specific maximum number of states.
 * Return 0 upon success.
 * Otherwise, return -1 and set errno.
 */
int aca_init(aca *aca, int n);

/* Destroy the ACA */
void aca_destroy(aca *aca);

/* Add a pattern of length n to the ACA.
 * Return the pattern index, upon success.
 * Otherwise, return -1 and set errno.
 */
int aca_add(aca *aca, char *pat, int n);

/* Build the ACA */
int aca_build(aca *aca);

/* Return the initial iterator of the ACA */
aca_iter aca_root(aca *aca);

/* Feed a symbol to the ACA and return the next iterator.
 * If there are matched occurrences after feed the symbol,
 * hit() will be called with the matched pattern index.
 * If hit() returns 0, the matching continues.
 * Otherwise, the matching stops.
 */
aca_iter aca_next(aca_iter it, char sym, int (*hit)(int));

#endif
