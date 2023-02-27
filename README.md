LibACA: C Library of Aho-Corasick Algorithm, with Balance between Time, Space, and Simplicity
=============================================================================================

*LibACA* is a C library of the Aho-Corasick algorithm/automaton.

Given a set of strings, each called a pattern, and an input string called the text, the Aho-Corasick algorithm finds every occurrence of every pattern in the text.

LibACA aims for:

- Time-space balance

	LibACA uses an original variant of trie, called *coordinate hash trie*.
	This trie variant provides a good balance between time and space,
	without compromising simplicity. 
	See the [*Internals*](#internals) section for detail.

	**The space complexity is proportional to the number of states.**
	Because no reallocation, resizing, or rehashing will be performed, **the space consumption is stable**.

	When transition from one state to another,
	**the time complexity is constant for the average case**,
	and **proportional to the size of the alphabet for the worst case**.

- Simplicity

	**The implementation contains no more than 175 LOCs of C89**, including header files.

- Robustness

	LibACA carefully deals with errors and arithmetic overflows.

LibACA is designed for binary streams, including infinite ones.

**Table of Contents**

* [System Requirements](#system-requirements)
* [Getting Started](#getting-started)
* [Installation](#installation)
* [Types](#types)
* [Functions](#functions)
* [Benchmarks](#benchmarks)
* [Internals](#internals)
* [Following News](#following-news)

System Requirements
-------------------

LibACA requires only C89 and the following feature of the target architecture:

- The binary representation of -1 is `111...11`.

Most mainstream architectures use two's complement, thus they all meet the requirement.

Getting Started
---------------

The following example `eg.c` reads symbols from the standard input and matches in a fixed pattern set.
For every match, it prints the matched pattern.

	#include <aca.h>

	...

	char *pattab[] = {"he", "she", "his", "hers"};

	int main(void)
	{
		int hit(int pat); /* callback for matching */
		aca fgrep;	/* the automaton */
		aca_iter it;	/* iterator of the automaton */
		int i, ch;

		/* initialize an automaton with the maximum number of states to be 16 */
		aca_init(&fgrep, 16);

		/* add all patterns in pattab to the automaton */
		for (i = 0; i < sizeof pattab / sizeof pattab[0]; i++)
			aca_add(&fgrep, pattab[i], strlen(pattab[i]));

		/* build the automaton */
		aca_build(&fgrep);

		/* get the initial iterator */
		it = aca_root(&fgrep);

		/* read symbols from the stdin stream and feed to the automaton
		 * each match is passed to hit()
		 */
		while ((ch = getchar()) != EOF)
			it = aca_next(it, ch, hit);

		/* destroy the automaton */
		aca_destroy(&fgrep);
		return 0;
	}

	int hit(int pat)
	{
		/* print the matched pattern */
		puts(pattab[pat]);
		return 0;
	}

The above example should behave like the following.

	$ cc -l aca eg.c
	$ echo shers | ./a.out
	she
	he
	hers

Installation
------------

To install LibACA as header files and a static library into the system:

	make
	sudo make install

By default, LibACA is installed to `/usr/local`.
To link LibACA, use the `-l aca` option on your compiler.

It's also simple to embed LibACA into your project.
Just include `aca.h`.
Compile and link `aca.c`.

Types
-----

- `aca`: Aho-Corasick Automaton

- `aca_iter`: iterator of an `aca` instance

Functions
---------

- `int aca_init(aca *aca, int n)`

	Initialize the automaton with the specific maximum number of states.

	The safest estimation of `n` is the sum of lengths of all patterns adding 1. A `n` less than 1 is equivalent to 1.

	Upon sucess, return 0.

	Otherwise, return -1 and set `errno`.

- `void aca_destroy(aca *aca)`

	Destroy the automaton.

- `int aca_add(aca *aca, char *pat, int n)`

	Add a pattern of length `n` to the automaton.

	A negative `n` is equivalent to 0.

	Upon success, return the pattern index.
	The first pattern has index 0;
	the second has index 1;
	and so on.
	Duplicated patterns would have the same index.

	Otherwise, return -1 and set `errno`.
	The internal state of the automaton will be indeterminate.
	The only allowed next action is `aca_destroy()`.

- `int aca_build(aca *aca)`

	Build the automaton.
	This routine should not be called before all patterns are added.

	Upon success, return 0.

	Otherwise, return -1 and set `errno`.


- `aca_iter aca_root(aca *aca)`

	Return the initial iterator of the automaton.

	This routine should not be called before the automaton is built.

- `aca_iter aca_next(aca_iter it, char sym, int (*hit)(int))`

	Feed a symbol to the automaton and return the next iterator.

	If there are matched occurrences after feeding the symbol,
	`hit()` will be called with the matched pattern index.

	If `hit()` returns 0, the matching continues, until `hit()` is called on all occurrences.

	Otherwise, the matching stops.

	The `hit()` function is called in the descending order of the length of the matched pattern.

Benchmarks
----------

There are three test cases.

- `test/alice`: the text of *Alice's Adventures in Wonderland* and the pattern set of 10000 random English words

- `test/rand`: the text of 300000 random characters and the pattern set of 10000 random strings

- `test/lfail`: the text of 399999 `a`s followed by a `b`, and a single pattern containing 400000 `a`s

~~~
test/alice
Prog        Build    Match    RSS
-------     -------  -------  -------
naive       86.4ms   4.9ms    95.9MB
libaca      247.3ms  8.5ms    5.3MB
-------     -------  -------  -------
Improv      -65%     -42%     94%

test/rand
Prog       Build    Match    RSS
-------    -------  -------  -------
naive      63.4ms   16.2ms   65.9MB
libaca     207.6ms  27.4ms   4.7MB
-------    -------  -------  -------
Improv     -69%     -41%     93%

test/lfail
Prog        Build    Match    RSS
-------     -------  -------  -------
naive       404.3ms  29.3ms   398.8MB
libaca      827.2ms  20.8ms   24.5MB
-------     -------  -------  -------
Improv      -51%     40%      94%
~~~

You could run `make benchmark` to perform the benchmark in your machine.
The unit of RSS may be incorrect in some systems, e.g. Linux.

Internals
---------

LibACA uses the coordinate hash trie to reach time-space balance and simplicity.
This trie variant is documented in the following paper.

[Storing a Trie with Compact and Predictable Space](https://arxiv.org/abs/2302.03690)

Following News
--------------

- <https://www.dyx.name>
- [Twitter](https://twitter.com/dongyx2)
