/* handling a priority queue */
#include "defs.h"
#include "graph.h"
#include "h_base.h"
#include "tree.h"
#include <stdlib.h>

static int re_ctr; /* count the edges first put in the queue but turned out
		      to be reducible... */

static int size_pq;

/* to be used as a parameter when graph_delete_vertex is called */
void rem_vert_data(VERT *v)
{
  VERT_DATA *vvv=v->data;
  free(vvv->Ax);
  free(vvv);
}

int reducible_edge(VERT *u, VERT *v)
{
  int i,n;
  vint *pu, *pv, sp;
  pu=((VERT_DATA *)(u->data))->Ax;
  pv=((VERT_DATA *)(v->data))->Ax;
  n=((VERT_DATA *)(u->data))->nrows;
  /* old strategy - replaced
  for (i=0; i<n; i++,pu++,pv++)
      if ((*pu<0 && *pv>0) || (*pu>0 && *pv<0)) return 1;
  return 0;
  */
  /* compute the scalar product sp of Au and Av */
  for (i=0, sp=0; i<n; i++) sp+=(*pu++)*(*pv++);
  return sp<0;
}

/* typedef struct {VERT *u; VERT *v; vint degree;} PQ_ITEM; 
 */
typedef struct {int nu; int nv; VERT *u; VERT *v; vint degree;} PQ_ITEM;

int PQ_ITEM_cmp(PQ_ITEM *e1,PQ_ITEM *e2)
{ /* 1st order by degree; then order by location in memory, just to 
     avoid conflicts */
  vint res;
  long int x;

  if ((res=(e1->degree) - (e2->degree))) return (res<0) ? -1 : 1;
  /*  if ((x=(e1->u)-(e2->u))) return (x<0) ? -1 : 1;
      if ((x=(e1->v)-(e2->v))) return (x<0) ? -1 : 1; 
      */
  if ((x=(e1->nu)-(e2->nu))) return (x<0) ? -1 : 1;
  if ((x=(e1->nv)-(e2->nv))) return (x<0) ? -1 : 1;
  return 0;
}

/* *********************************************************************
   *
   *         init_pq(g)
   *
   ********************************************************************* */

/* called from init_pq to initialize activity counters */
int vert_activ_init(VERT *u)
{
  VERT_DATA *p;
  p=u->data;
  p->activ_ctr=0;
  return 1;
}

struct edge_act_pars {VERT *u; SET **pq;};

int edge_act(VERT *v, struct edge_act_pars *p)
{
  PQ_ITEM *e;
  if ((p->u)->number<v->number /* u<v  */
      && reducible_edge((p->u),v))
    {
      VERT_DATA *ud, *vd;
      if (!(e=(PQ_ITEM *)malloc(sizeof(PQ_ITEM))))
	{ perror("out of memory  in init_pq"); exit(7);}
      e->nu=(p->u)->number;
      e->nv=v->number;
      e->u=(p->u);
      e->v=v;
      ud=(p->u)->data;
      vd=v->data;
      e->degree=ud->degree + vd->degree;
      ud->activ_ctr++;
      vd->activ_ctr++;
      tree_add(p->pq,PQ_ITEM_cmp,e,NULL);
      size_pq++;
      if (!tree_srch(p->pq,PQ_ITEM_cmp,e))
	{perror("tree inconsistency in edge_act!"); exit(13);}
    }
  return 1;
}

int vert_act(VERT *u, SET **pq)
{
  struct edge_act_pars pars;
  pars.u=u;
  pars.pq=pq;
  tree_trav_pars(&(u->adjacency),edge_act,&pars);
  return 1;
}

SET *init_pq(GRAPH *g) /* browse the edges of g and store those in pq which 
			 are reducible (in sense of reducible_edge()<>0) */
{
  SET *pq;
  re_ctr = 0;
  size_pq = 0;
  tree_init(&pq);
  tree_trav(&(g->vertices),vert_activ_init);
  tree_trav_pars(&(g->vertices),vert_act,&pq);
  return pq;
}

/* *******************************************************************
   *  various functions dealing with vectors 
   ******************************************************************* */
/* determine whether A(u+v)=0 */
/* int edge_in_kernel(PQ_ITEM *e) */

int edge_in_kernel(VERT *u, VERT *v)
{
  vint rhs[MAXN], *p;
  void add_vect(int, vint *, vint *, vint *);
  int i;
  VERT_DATA *u_d, *v_d;

  /*  u=e->u;
      v=e->v; */
  u_d=u->data; 
  v_d=v->data; 
  add_vect(u_d->nrows, rhs, u_d->Ax, v_d->Ax); /* compute A(u+v) */
  for (i=0, p=rhs; i<u_d->nrows; i++)          /* is it 0 ? */
    if (*(p++)) return 0;
  return 1;
}

/* determine whether u+v is redundant, i.e. there exist an element
 in H.B. reducing it */
/* int redundant_edge(BASE *b, PQ_ITEM *e, int in_kernel, vint w[]) */
int redundant_edge(BASE *b, VERT *u, VERT *v, vint deg, 
		   int in_kernel, vint w[])
{
  void add_vect(int, vint *, vint *, vint *);
  int i;
  VERT_DATA *u_d;

  u_d=u->data; 
  add_vect(u_d->ncols, w, u->key, v->key); /* compute u+v */
  return reducible_vector(b, deg, in_kernel, w);
}

int filter_redundant(BASE *b, VERT *x, VERT *w)
{
  vint www[MAXM];
  return 
    !redundant_edge(b, w, x, 
		   (((VERT_DATA *)(w->data))->degree)+
		   (((VERT_DATA *)(x->data))->degree),
		   0, www);
}

/* *********************************************************************
   *
   *         process_pq(pq, g, base)
   *
   ********************************************************************* */

struct add_edge_pars{VERT *w; SET **pq; };

int add_edge(VERT *x, struct add_edge_pars *p) /* adding edge to the queue */
{
  SET **pq;
  VERT *w;
  PQ_ITEM *n_e;

  pq=p->pq; /* adding to the queue pq */
  w=p->w;   /* the edge (x,w)         */

#ifdef DEBUG_graph
  printf("\n add_edge (%d,%d) ",w->number,x->number);
#endif
  if (reducible_edge(w,x))
    {
#ifdef DEBUG_graph
      printf(": reducible\n");
#endif

      if (!(n_e=(PQ_ITEM *)malloc(sizeof(PQ_ITEM))))
	{ perror("out of memory  in process_pq"); exit(7);}
#define SET_n_e(a,b) { \
		       n_e->nu=a->number; n_e->nv=b->number;\
		       n_e->u=a; n_e->v=b;}

      if (w->number<x->number) SET_n_e(w,x)
      else SET_n_e(x,w)

	n_e->degree=((VERT_DATA *)(w->data))->degree+
	  ((VERT_DATA *)(x->data))->degree;
      if (tree_srch(pq,PQ_ITEM_cmp,n_e)) free(n_e);
      else 
	{ 
	  /* printf(" %p: %d %d %d\n ",n_e,n_e->nu,n_e->nv,n_e->degree); */
	  tree_add(pq,PQ_ITEM_cmp,n_e,NULL);
#define ADD_activ_ctr(z) ((VERT_DATA *)((z)->data))->activ_ctr++;
	  ADD_activ_ctr(n_e->u)
	  ADD_activ_ctr(n_e->v)
	  size_pq++;
	}
    }
  return 1;
}


int process_pq(SET **pq, GRAPH *g, BASE *b)
{
  extern int prtlev;
  extern vint old_deg;
  PQ_ITEM *e;
  VERT *w, *cr_vert(VERT *, VERT *);
  int inker, irred;
  vint www[MAXM];
  void rem_vert_data(VERT *);

#ifdef DEBUG_graph
  int ctr;
#endif

  if (!(e=tree_min(pq))) return 0; /* queue empty ? */
  size_pq--;
/* reducibility check */
  inker = edge_in_kernel(e->u, e->v); /* A(u+v)=0 ? */
  irred = !redundant_edge(b, e->u, e->v, e->degree, inker, 
			  www); /* as a by-product, we have u+v here in www */

#ifdef DEBUG
  printf("\n processing edge %p=(%d,%d) of degree %d\n",
	e,(e->u)->number, (e->v)->number, e->degree);
  printf(" in kernel ? - %d; irredundant ? - %d\n", inker, irred);
#endif
  
  if (prtlev && (old_deg != e->degree)) { 
    printf("\n degree %4d; %8d scanned, %8d present,\n    base size %8d, removed edges %8d; edges to process %d",
	   e->degree,g->nvert,g->vert_ctr,b->size, re_ctr, size_pq);
    old_deg=e->degree;
  }

#define IF_NOT_ACTIVE_DEL(z) if (!(--(((VERT_DATA *)(z)->data)->activ_ctr))) \
  graph_remove_vertex(g,z,rem_vert_data);

  if (irred)
    { if (inker)
	{
#ifdef DEBUG
	  printf("\n found a base element of degree %d\n", e->degree);
#endif
	  insert_base_vector(b, e->degree, www);
	  graph_remove_edge(g, e->u, e->v);
	}
      else
	{ if ((w=graph_split_edge(g, e->u, e->v, cr_vert, b, 
			     filter_redundant, rem_vert_data)))
	    {
	      struct add_edge_pars pars;
	      pars.pq=pq;
	      pars.w=w;
	      tree_trav_pars(&(w->adjacency), add_edge, &pars);
	      if (!((VERT_DATA *)w->data)->activ_ctr)
		graph_remove_vertex(g,w,rem_vert_data);

	    }
	}
    }
  else {
    graph_remove_edge(g,e->u,e->v);
    re_ctr++;
#ifdef DEBUG
    graph_print(g);
    printf("\n deleting edge %p \n",e);
#endif
  }
#define IF_NOT_ACTIVE_DEL(z) if (!(--(((VERT_DATA *)(z)->data)->activ_ctr))) \
  graph_remove_vertex(g,z,rem_vert_data);

  IF_NOT_ACTIVE_DEL(e->u)
  IF_NOT_ACTIVE_DEL(e->v)
  tree_delete(pq,PQ_ITEM_cmp,e,free);
  return 1;
}

