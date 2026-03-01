.PHONY all: libstring.so libstring.a

libstring.so: string.c
	gcc -O0 -g -o libstring.so string.c -shared -fPIC

libstring.a: string.c
	gcc -O0 -g -c string.c
	ar rcs libstring.a string.o
