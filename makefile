#CC=checkergcc
CC=gcc
CFLAGS=-g -O
EXT=.exe

all : main.o pq.o graph.o h_base.o t_trav.o tree.o
	$(CC) $(CFLAGS) -o main$(EXT) main.o pq.o graph.o h_base.o tree.o t_trav.o

main.o  : main.c defs.h graph.h h_base.h tree.h
	$(CC) $(CFLAGS) -c main.c

pq.o    : pq.c defs.h graph.h h_base.h tree.h
	$(CC) $(CFLAGS) -c pq.c

graph.o : graph.c defs.h graph.h tree.h
	$(CC) $(CFLAGS) -c graph.c

h_base.o : h_base.c defs.h h_base.h tree.h
	$(CC) $(CFLAGS) -c h_base.c

t_trav.o : t_trav.c tree.h vixie.h
	$(CC) $(CFLAGS) -c t_trav.c

tree.o : tree.c tree.h vixie.h
	$(CC) $(CFLAGS) -c tree.c

clean :
	rm *.o main$(EXT) *~ *# Tests/*~

arch :
	tar cvzf HB.tgz *.c *.h makefile Tests/* README


