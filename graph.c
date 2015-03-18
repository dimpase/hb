/* ************************************************************************
 ** graph.c               HB package version 0.1 (experimental)
 ** 
 ** Copyright (C) 1997 Dmitrii V. Pasechnik
 **                    SSOR/TWI, TU Delft, The Netherlands
 ** 
 ** Handling operations with a non-oriented graph
 ** 1) GRAPH *graph_init(int (*compare)(VERT *,VERT *))
 ** 2) VERT *graph_find_vertex(GRAPH *,VERT *)
 ** 3) graph_adjacency
 ** 4) graph_add_vertex (see graph.h for the time being)
* *********************************************************************** */
#include "defs.h" /* standard common definitions */
#include "graph.h" /* typedefs etc */


GRAPH *graph_init(int (*compare)())
{
  GRAPH *g;
  if ((g=(GRAPH *)malloc(sizeof(GRAPH)))) 
    {
      g->nvert=0;
      g->vert_ctr=0;
      g->compare=compare;
      tree_init(&(g->vertices));
    }


#ifdef DEBUG_graph
  printf("\n initailizing the graph at location %p\n",g);
#endif

  return g;
}
	    
VERT *graph_find_vertex(GRAPH *g,VERT *key)
{
  VERT *v;
  v=tree_srch(&(g->vertices), g->compare, key);
  return v;
}


/* a function called from tree_trav in graph_add_vertex */
struct gr_edgeadd_pars{ VERT *v; GRAPH *g;};

int gr_edgeadd(VERT *u, struct gr_edgeadd_pars *p) 
{  
  void graph_add_arc(GRAPH *, VERT *, VERT *);
  graph_add_arc(p->g,u,p->v);
  return 1;
}

int graph_add_vertex(GRAPH *g, VERT *v)
{
  struct gr_edgeadd_pars pars;
  if (graph_find_vertex(g, v)) /* vertex is already there ?? */
    return 0;
  
  g->nvert++;
  g->vert_ctr++;
  v->number=g->nvert;

#ifdef DEBUG_graph
  printf("\n adding vertex # %4d of degree %4d\n",v->number,v->degree);
#endif

  tree_add(&(g->vertices), g->compare, v, NULL);
  pars.g=g;
  pars.v=v;

  /* add arcs (u,v)  for u in adj v */
  return tree_trav_pars(&(v->adjacency),gr_edgeadd, &pars); 
}

/* making an arc from u to v */
void graph_add_arc(GRAPH *g, VERT *u, VERT *v)
{
  tree_add(&(u->adjacency), g->compare, v, NULL);
  u->degree++;
}

/* making an edge between u and v */
void graph_add_edge(GRAPH *g, VERT *u, VERT *v)
{
  graph_add_arc(g, u, v);
  graph_add_arc(g, v, u);
}


#ifdef DEBUG_compile
int g_vnum_print(VERT *u)
{
  printf("%4d",u->number);
  return 1;
}

int g_vertex_print(VERT *v)
{
  printf("\n %4d. (val: %3d) ",v->number,v->degree);
  tree_trav(&(v->adjacency),g_vnum_print);
  vertex_print(v);
}

void graph_print(GRAPH *g)
{
  printf("\n vertex counter (incl. deleted) %d\n",g->nvert);
  printf("\n # of vertices in the graph %d\n",g->vert_ctr);
  tree_trav(&(g->vertices),g_vertex_print);
}

#endif

/* finding the ** filtered **
   common neighbourhood of u and v in g; *size is its size */

struct add_ext_res_pars { int (*filter)(); int ext_cntr;
  SET **ext_b; GRAPH *g; VERT *a; SET *res; void *base; };

int add_ext_res(VERT *u, struct add_ext_res_pars *z)
{
  VERT *p;
  
  if ((p=tree_srch(z->ext_b, (z->g)->compare, u)) &&
      (*(z->filter))(z->base, u, z->a))
    {
      (z->ext_cntr)++;
      tree_add(&(z->res), (z->g)->compare, p, NULL);
    }
  return 1;
}

SET *graph_common_neighbours(GRAPH *g, VERT *u, VERT *v, int *size,
			     void *base, 
			     int (*filter)(void *, VERT *, VERT *))
{
  struct add_ext_res_pars p;

  p.filter=filter;
  p.g=g;
  p.base=base;
  if (u->degree < v->degree) { p.a=u; p.ext_b=&(v->adjacency); }
  else { p.a=v; p.ext_b=&(u->adjacency); }
  tree_init(&(p.res));
  p.ext_cntr=0;
  tree_trav_pars(&(p.a->adjacency), add_ext_res, &p);
  *size=p.ext_cntr;
  return p.res;
}

void graph_remove_arc(GRAPH *g, VERT *u, VERT *v)
{
#ifdef DEBUG_graph
  printf("\n graph_remove_arc: removing the arc (%d,%d)\n",
	 u->number,v->number);
  graph_print(g);
#endif
  tree_delete(&(u->adjacency), g->compare, v, NULL);
  u->degree--;
}

void graph_remove_edge(GRAPH *g, VERT *u, VERT *v)
{
#ifdef DEBUG_graph
  printf("\n graph_remove_edge: removing the edge (%d,%d)\n",
	 u->number,v->number);
#endif
  graph_remove_arc(g,u,v);
  graph_remove_arc(g,v,u);
}

VERT *graph_split_edge(GRAPH *g, VERT *u, VERT *v, 
		      VERT *(*cr_vert)(VERT *, VERT *),
		       void *base,
		       int (*filter)(void *,VERT *, VERT *),
		       void (*rem_vert_data)(VERT *))  
{
  VERT *w;
  int goodvert;

#ifdef DEBUG_graph
  printf("\n splitting the edge (%d,%d)\n",u->number,v->number);
#endif

  w=(*cr_vert)(u,v);
  w->adjacency=graph_common_neighbours(g,u,v,&(w->degree),base,filter);
  goodvert=w->degree;

  graph_add_vertex(g,w);
#define ADD_edge_if_OK(x,y) if ((*filter)(base,x,y)) { goodvert++;\
						    graph_add_edge(g,x,y);}
  ADD_edge_if_OK(w,u)
  ADD_edge_if_OK(w,v)
  graph_remove_edge(g,u,v);
  if (goodvert)  return w;
  graph_remove_vertex(g,w,rem_vert_data);
  return NULL;
}

struct ar_rem_pars { VERT *u; GRAPH *g;};
int ar_rem(VERT *v, struct ar_rem_pars *p)
{
  graph_remove_arc(p->g,v,p->u);
  return 1;
}

void graph_remove_vertex(GRAPH *g, VERT *u, void (*rm_vert_data)(VERT *))
{
  struct ar_rem_pars pars;
#ifdef DEBUG_graph
  printf("\n removing the vertex %d\n",u->number);
#endif
  pars.g=g;
  pars.u=u;

  tree_trav_pars(&(u->adjacency),ar_rem,&pars); /* removing all 
						   arcs coming to u */
  tree_mung(&(u->adjacency),NULL);   /* and their opposite too */
  tree_delete(&(g->vertices),g->compare,u,NULL); /* removing u from the
						    list of vertices of g */
  (*rm_vert_data)(u);               /* removing all the user-defined
				       stuff in u */
  free(u->key);                     /* removing the key */
  free(u);
  g->vert_ctr--;

#ifdef DEBUG_graph
  printf("\n graph_remove_vertex: after the removal");
  graph_print(g);
#endif
}
  
