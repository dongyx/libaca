#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "aca.h"

struct hit {
	int pat, pos;
};

char **pattab;
struct hit *hittab;
size_t htcap;
int nhit, txtpos;

void append(void *bufpp, size_t *capp, size_t width, size_t n, const void *vp)
{
	char **bpp;

	bpp = (char **)bufpp;
	if (*bpp == NULL) {
		*capp = 8;
		*bpp = malloc(*capp * width);
	}
	if (n == *capp) {
		*capp *= 2;
		*bpp = realloc(*bpp, *capp * width);
	}
	assert(*bpp);
	memcpy((*bpp)+n*width, vp, width);
}

int hit(int pat)
{
	struct hit h;

	h.pos = txtpos;
	h.pat = pat;
	append(&hittab, &htcap, sizeof *hittab, nhit++, &h);
	return 0;
}

int main(int argc, char **argv)
{
	char *line, *txt;
	size_t linecap, ptcap, plcap, txtcap;
	int *patlen, npat, patsum, txtlen, n, i;
	struct rusage ru;
	clock_t ticks;
	FILE *fp;
	aca ps;
	aca_iter it;

	fp = fopen(argv[1], "r");
	patsum = 0;
	npat = 0;
	pattab = NULL;
	patlen = NULL;
	for (line=NULL; (n=getline(&line,&linecap,fp))!=-1; line=NULL) {
		if (n > 0 && line[n - 1])
			line[--n] = '\0';
		append(&pattab, &ptcap, sizeof *pattab, npat, &line);
		append(&patlen, &plcap, sizeof *patlen, npat, &n);
		npat++;
		patsum += n;
	}
	fclose(fp);

	ticks = clock();
	if (aca_init(&ps, patsum + 1) == -1) {
		perror("aca_init");
		exit(-1);
	}
	for (i = 0; i < npat; i++)
		if (aca_add(&ps, pattab[i], patlen[i]) == -1) {
			perror("aca_add");
			exit(-1);
		}
	if (aca_build(&ps) == -1) {
		perror("aca_build");
		exit(-1);
	}
	ticks = clock() - ticks;
	printf("%f\n", (double)ticks/CLOCKS_PER_SEC);

	fp = fopen(argv[2], "r");
	txtlen = 0;
	txt = NULL;
	for (line=NULL; (n=getline(&line,&linecap,fp))!=-1; line=NULL) {
		char *lstart;

		lstart = line;
		while (*line)
			append(&txt, &txtcap, sizeof *txt, txtlen++, line++);
		free(lstart);
	}
	fclose(fp);

	ticks = clock();
	it = aca_root(&ps);
	while (*txt) {
		it = aca_next(it, *txt++, hit);
		txtpos++;
	}
	ticks = clock() - ticks;
	printf("%f\n", (double)ticks/CLOCKS_PER_SEC);

	aca_destroy(&ps);
	fp = fopen(argv[3], "w");
	for (i = 0; i < nhit; i++)
		fprintf(fp, "%d:%s\n",
			(int)hittab[i].pos, pattab[hittab[i].pat]);
	fclose(fp);
	getrusage(RUSAGE_SELF, &ru);
	printf("%ld\n", ru.ru_maxrss);
	return 0;
}
