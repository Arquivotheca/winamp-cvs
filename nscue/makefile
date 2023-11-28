CFLAGS=-I../nu -I../Wasabi

cueparse: main.o nscue.o PtrList.o
	g++ main.o nscue.o PtrList.o -o cueparse

main.o: main.cpp nscue.h
	g++ $(CFLAGS) -c main.cpp -o main.o

nscue.o: nscue.cpp nscue.h
	g++ $(CFLAGS) -c nscue.cpp -o nscue.o

PtrList.o: ../nu/PtrList.cpp ../nu/PtrList.h
	g++ $(CFLAGS) -c ../nu/PtrList.cpp -o PtrList.o
