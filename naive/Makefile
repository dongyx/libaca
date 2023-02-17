.PHONY: clean test
run.tmp: test.c aca.c aca.h
	cc -o $@ test.c aca.c
test: run.tmp
	./runtest
clean:
	rm -rf *.tmp
