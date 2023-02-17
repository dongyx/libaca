#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "aca.h"
#define ALPHSZ 256

static struct aca_edge *getedge(aca *aca, int from, char sym, int creat);

int aca_init(aca *aca, int n)
{
	if (n < 1) {
		errno = EINVAL;
		goto ERR;
	}
	if (INT_MAX - (n-1) < (n-1) / 3) {
		errno = ERANGE;
		goto ERR;
	}
	aca->maxns = n;
	aca->ecap = (n-1) + (n-1)/3;
	aca->ns = 1;
	aca->np = 0;
	if (!(aca->etab = calloc(aca->ecap, sizeof aca->etab[0])))
		goto ERR;
	if (!(aca->fail = calloc(n, sizeof aca->fail[0])))
		goto FREE_ETAB;
	if (!(aca->pat = calloc(n, sizeof aca->pat[0])))
		goto FREE_FAIL;
	if (!(aca->prev = calloc(n, sizeof aca->prev[0])))
		goto FREE_PAT;
	/* if calloc() calls succeeded, n*size won't overflow size_t */
	memset(aca->pat, 0xff, n*sizeof aca->pat[0]);
	memset(aca->prev, 0xff, n*sizeof aca->prev[0]);
	aca->fail[0] = -1;
	return 0;
FREE_PAT:
	free(aca->pat);
FREE_FAIL:
	free(aca->fail);
FREE_ETAB:
	free(aca->etab);
ERR:
	return -1;
}

void aca_destroy(aca *aca)
{
	struct aca_edge *p, *q;
	int i;

	for (i = 0; i < aca->ecap; i++) {
		p = aca->etab[i];
		while (p) {
			q = p->next;
			free(p);
			p = q;
		}
	}
	free(aca->etab);
	free(aca->fail);
	free(aca->pat);
	free(aca->prev);
}

int aca_add(aca *aca, char *pat, int n)
{
	struct aca_edge *e;
	int i;

	i = 0;
	while (n-- > 0) {
		if (!(e = getedge(aca, i, *pat++, 1)))
			return -1;
		i = e->to;
	}
	if (aca->pat[i] >= 0)
		return aca->pat[i];
	/* np <= ns <= maxns, thus np++ won't overflow int */
	return aca->pat[i] = aca->np++;
}

int aca_build(aca *aca)
{
	struct aca_edge *e;
	int *q, front, rear;
	int i, j, c;

	if (!(q = calloc(aca->ns, sizeof q[0])))
		return -1;
	front = 0, rear = 1;
	while (rear > front) {
		for (c = 0; c < ALPHSZ; c++) {
			i = q[front];
			if (!(e = getedge(aca, i, c, 0)))
				continue;
			j = e->to;
			e = NULL;
			do {
				i = aca->fail[i];
			} while (i >= 0 && !(e = getedge(aca, i, c, 0)));
			i = e ? e->to : 0;
			aca->fail[j] = i;
			aca->prev[j] = aca->pat[i] >= 0 ? i : aca->prev[i];
			q[rear++] = j;
		}
		front++;
	}
	free(q);
	return 0;
}

aca_iter aca_root(aca *aca)
{
	aca_iter it;

	it.aca = aca;
	it.st = 0;
	return it;
}

aca_iter aca_next(aca_iter it, char sym, int (*hit)(int))
{
	aca *aca;
	aca_iter next;
	struct aca_edge *e;
	int i;

	aca = next.aca = it.aca;
	i = it.st;
	while (i >= 0 && !(e = getedge(aca, i, sym, 0)))
		i = aca->fail[i];
	next.st = i = e ? e->to : 0;
	if (aca->pat[i] < 0)
		i = aca->prev[i];
	while (i >= 0 && !hit(aca->pat[i]))
		i = aca->prev[i];
	return next;
}

static struct aca_edge *getedge(aca *aca, int from, char sym, int creat)
{
	struct aca_edge *e;
	unsigned long h;

	h = (unsigned long)from*ALPHSZ + (unsigned char)sym;
	h %= aca->ecap;
	for (e = aca->etab[h]; e; e = e->next)
		if (e->from == from && e->sym == sym)
			return e;
	if (creat) {
		if (aca->ns == aca->maxns) {
			errno = ENOMEM;
			return NULL;
		}
		if (!(e = malloc(sizeof *e)))
			return NULL;
		e->next = aca->etab[h];
		aca->etab[h] = e;
		e->from = from;
		e->sym = sym;
		e->to = aca->ns++;
		return e;
	}
	return NULL;
}
