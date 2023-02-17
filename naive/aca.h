#ifndef LIBACA
#define LIBACA
#define ACA_ALPHSZ 256

typedef struct {
	int ns;
	int np;
	int maxns;
	int (*tran)[ACA_ALPHSZ];
	int *fail;
	int *pat;
	int *prev;
} aca;

typedef struct {
	aca *aca;
	int st;
} aca_iter;

int aca_init(aca *aca, int n);

void aca_destroy(aca *aca);

int aca_add(aca *aca, char *pat, int n);

int aca_build(aca *aca);

aca_iter aca_root(aca *aca);

aca_iter aca_next(aca_iter it, char sym, int (*hit)(int));

#endif
