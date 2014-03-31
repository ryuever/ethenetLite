cc = clang -g -lpthread

all : enltest

enltest : enltest.o enl_network.o enl_object.o enl_memory.o enl_parser.o enl_thread.o enl_api.o
	$(cc) -o $@ $^ 

enltest.o : enltest.c
enl_object.o : enl_object.c enl_object.h
enl_memory.o : enl_memory.c enl_memory.h
enl_parser.o : enl_parser.c enl_parser.h
enl_thread.o : enl_thread.c enl_thread.h
enl_api.o : enl_api.c enl_api.h
enl_network.o : enl_network.c enl_network.h

clean :
	rm -f enl_test *.o *~ core

.c.o:
	$(cc) -c $*.c	
