# freebsd3 makefile

LDFLAGS= /usr/lib/libz.dylib
CFLAGS= -g  -Wall -I../Wasabi -DTHREAD_SAFE -D_THREAD_SAFE -D_REENTRANT
CC= g++
GFLAGS=  -g -Wall -I../Wasabi -DTHREAD_SAFE -D_THREAD_SAFE -D_REENTRANT
CPP= g++
CXX= g++

OBJS = asyncdns.o connection.o httpget.o util.o

default: jnl.a

asyncdns.o: asyncdns.cpp
	$(CC) $(CFLAGS) -c -o asyncdns.o asyncdns.cpp

connection.o: connection.cpp
	$(CC) $(CFLAGS) -c -o connection.o connection.cpp

httpget.o: httpget.cpp
	$(CC) $(CFLAGS) -c -o httpget.o httpget.cpp

util.o: util.cpp
	$(CC) $(CFLAGS) -c -o util.o util.cpp

test.o: test.cpp
	$(CC) $(CFLAGS) -c -o test.o test.cpp


jnl.a: ${OBJS}
	-rm -f jnl.a
	ar rcs jnl.a ${OBJS}

test: ${OBJS} test.o
	$(CC) $(CFLAGS) -o test test.o ${OBJS} ${LDFLAGS}

clean:
	-rm -f ${OBJS} test jnl.a test.o
