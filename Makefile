# Name: Omar Madhani

all: PMan

PMan: PMan.o
	gcc PMan.o -o PMan

PMan.o: PMan.c PMan.h
	gcc -c PMan.c

clean:
	rm -rf *.o PMan
