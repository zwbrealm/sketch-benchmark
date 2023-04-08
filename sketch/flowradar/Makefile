CC = g++
CFLAGS = -O2 -std=c++14 -g
flowradarreq = bloomfilter.o flowradar.o

HEADER += hash.h murmur3.h 

1: $(flowradarreq)
	$(CC) $(CFLAGS) -o test $(flowradarreq) -lpcap

bloomfilter.o: bloomfilter.h $(HEADER)
	$(CC) $(CFLAGS) -c bloomfilter.h murmur3.h hash.h bloomfilter.cpp
	
flowradar.o: flowradar.h $(HEADER) 
	$(CC) $(CFLAGS) -c flowradar.h murmur3.h hash.h flowradar.cpp
