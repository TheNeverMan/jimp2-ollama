all:
	gcc -ggdb -Wall -pedantic -o lc *.c

clean:
	rm -f lc
