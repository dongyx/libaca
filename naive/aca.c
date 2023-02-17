#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "aca.h"
#define ALPHSZ ACA_ALPHSZ

int aca_init(aca *aca, int n)
{
	if (n < 1) {
		errno = EINVAL;
		goto ERR;
	}
	aca->maxns = n;
	aca->ns = 1;
	aca->np = 0;
	if (!(aca->tran = calloc(n, sizeof aca->tran[0])))
		goto ERR;
	if (!(aca->fail = calloc(n, sizeof aca->fail[0])))
		goto FREE_TRAN;
	if (!(aca->pat = calloc(n, sizeof aca->pat[0])))
		goto FREE_FAIL;
	if (!(aca->prev = calloc(n, sizeof aca->prev[0])))
		goto FREE_PAT;
	memset(aca->tran, 0xff, n*sizeof aca->tran[0]);
	memset(aca->prev, 0xff, n*sizeof aca->prev[0]);
	memset(aca->pat, 0xff, n*sizeof aca->pat[0]);
	aca->fail[0] = -1;
	return 0;
FREE_PAT:
	free(aca->pat);
FREE_FAIL:
	free(aca->fail);
FREE_TRAN:
	free(aca->tran);
ERR:
	return -1;
}

void aca_destroy(aca *aca)
{
	free(aca->tran);
	free(aca->fail);
	free(aca->pat);
	free(aca->prev);
}

int aca_add(aca *aca, char *pat, int n)
{
	int i;

	i = 0;
	while (n-- > 0) {
		if (aca->tran[i][(unsigned char)*pat] < 0) {
			if (aca->ns == aca->maxns) {
				errno = ENOMEM;
				return -1;
			}
			aca->tran[i][(unsigned char)*pat] = aca->ns++;
		}
		i = aca->tran[i][(unsigned char)*pat];
		pat++;
	}
	if (aca->pat[i] < 0)
		aca->pat[i] = aca->np++;
	return aca->pat[i];
}

int aca_build(aca *aca)
{
	int *q, front, rear;
	int i, j, c;

	if (!(q = calloc(aca->maxns, sizeof q[0])))
		return -1;
	front = 0, rear = 1;
	while (rear > front) {
		for (c = 0; c < ALPHSZ; c++) {
			i = q[front];
			if ((j = aca->tran[i][c]) < 0)
				continue;
			do {
				i = aca->fail[i];
			} while (i >= 0 && aca->tran[i][c] < 0);
			i = i >= 0 ? aca->tran[i][c] : 0;
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
	int i;

	aca = next.aca = it.aca;
	i = it.st;
	while (i >= 0 && aca->tran[i][(unsigned char)sym] < 0)
		i = aca->fail[i];
	i = i >= 0 ? aca->tran[i][(unsigned char)sym] : 0;
	next.st = i;
	if (aca->pat[i] < 0)
		i = aca->prev[i];
	while (i >= 0 && !hit(aca->pat[i]))
		i = aca->prev[i];
	return next;
}
