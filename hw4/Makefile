
CC = g++-4.7
CFLAGS = -O3

all: randtrack randtrack_global_lock

randtrack: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) randtrack.cc -o randtrack -lpthread

randtrack_global_lock: list.h hash.h defs.h randtrack_global_lock.cc
	$(CC) $(CFLAGS) randtrack_global_lock.cc -o randtrack_global_lock -lpthread


clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock
