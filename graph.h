/* ************************************************************************
 ** graph.h               HB package version 0.1 (experimental)
 ** 
 ** Copyright (C) 1997 Dmitrii V. Pasechnik
 **                    SSOR/TWI, TU Delft, The Netherlands
 ** 
 ** Handling operations with a non-oriented graph
 ** 1) GRAPH *graph_init(int (*compare)(VERT *,VERT *))
 ** 2) VERT *graph_find_vertex(GRAPH *,VERT *)
 ** 3) graph_adjacency
 ** 4) graph_add_vertex
* *********************************************************************** */
#ifndef _GRAPH_HB
#define _GRAPH_HB
#include "tree.h"

typedef tree SET;
typedef void VERT_KEY;

typedef struct {int nvert;    /* # of vertices scanned */
		int vert_ctr; /* # of vertices present */
		int (*compare)(void *, void *); /* pointer to the 
						comparison function */
		SET *vertices;   /* pointer to the set of vertices */
	      } GRAPH;  

typedef struct {VERT_KEY *key; /* a unique key to identify the vertex */
		int degree; /* degree of the vertex */
		void *data; 
		int number; /* serves as a name */
		SET *adjacency; /* adjacent vertices */
	      } VERT;


GRAPH *graph_init(int (*)());
VERT *graph_find_vertex(GRAPH *,VERT *);
int graph_add_vertex(GRAPH *, VERT *);
void graph_add_edge(GRAPH *, VERT *, VERT *);

#ifdef DEBUG
void graph_print(GRAPH *);
#endif

void graph_remove_edge(GRAPH *, VERT *, VERT *);
VERT *graph_split_edge(GRAPH *, VERT *, VERT *, VERT *(*)(VERT *, VERT *), 
		       void *,
		       int (),
		       void (*)(VERT *));

void graph_remove_vertex(GRAPH *, VERT *, void (*)(VERT *));

int tree_trav_pars(tree **, int (*)(), void *);

tree_t tree_min(tree **); 
#endif
