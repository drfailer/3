.PHONY all: libprint.so libprint.a

libprint.so: print.c
	gcc -o libprint.so print.c -shared -fPIC

libprint.a: print.c
	gcc -c print.c
	ar rcs libprint.a print.o
