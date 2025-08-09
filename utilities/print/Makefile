.PHONY all: libprint.so libprint.a

libprint.so: print.c
	gcc -g -o libprint.so print.c -shared -fPIC

libprint.a: print.c
	gcc -g -c print.c
	ar rcs libprint.a print.o
