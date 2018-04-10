mem: mem.c
	gcc -g -Wall -o mem mem.c


libmem: mem.c mem.h memtest.c libmem.so
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o


memtest: memtest.c libmem.so
	gcc -L. -o memtest memtest.c -Wall -lmem

all: libmem mem memtest
