#include "defs.h"
#include "graph.h"
#include "h_base.h"

int prtlev;
vint old_deg;

/* ******************************************************************
   here go the comparison function and cr_vert 
   which we want to use now.
*/

int compa(VERT *u, VERT *v)
{
  VERT_DATA *u_d=u->data, *v_d=v->data;
  register vint r, *ku, *kv;
  register int n, i;
/* #ifdef DEBUG_graph
  printf("\n comparing vertices %d and %d",u->number,v->number);
#endif */
  if ((r=u_d->degree - v_d->degree)) return (r<0) ? -1 : 1;
  
  n=u_d->ncols;
  ku=u->key; kv=v->key;
  for (i=0; i<n; i++)
    if ((r=*(ku++) - *(kv++))) return (r<0) ? -1 : 1;
  return 0;
}

#ifdef DEBUG_compile
void vertex_print(VERT *v)
{
  VERT_DATA *vd;
  int i;
  vint *p;

  vd=v->data;
  printf("\n activ_ctr=%d; key: ",vd->activ_ctr);
  for (i=0, p=v->key; i<vd->ncols; i++) printf("%3d",*(p++));
  printf("\n Ax: ");
  for (i=0, p=vd->Ax; i<vd->nrows; i++) printf("%3d",*(p++));
}
#endif

    
void add_vect(int n, vint *res, vint *a, vint *b)
{
  int i;
  for (i=0; i<n; i++) *(res++)=*(a++) + *(b++);
}

VERT *cr_empty_vert(int n, int m)
{
  VERT *w;
  char *oomem={"*** out of memory in cr_empty_vert ***"};
  VERT_DATA *w_d;

  if (!(w=(VERT *)malloc(sizeof(VERT)))) 
    {perror(oomem); exit(7);}

  if (!(w->data=(void *)malloc(sizeof(VERT_DATA))))
    {perror(oomem); exit(7);}

  w_d=w->data;
  w_d->nrows=n;
  w_d->ncols=m;
  w_d->activ_ctr=0;

  if (!(w->key=(void *)malloc(sizeof(vint)*m)))
    {perror(oomem); exit(7);}

  if (!(w_d->Ax=(vint *)malloc(sizeof(vint)*n)))
    {perror(oomem); exit(7);}

  return w;
}

VERT *cr_vert(VERT *u, VERT *v)
{
  VERT *w;
  VERT_DATA *w_d, *u_d, *v_d;

  u_d=u->data;
  v_d=v->data;
  w=cr_empty_vert(u_d->nrows,u_d->ncols);
  w_d=w->data;
  w_d->degree=u_d->degree + v_d->degree;
  add_vect(w_d->ncols,(vint *)(w->key),
	   (vint *)(u->key),(vint *)(v->key));
  add_vect(w_d->nrows,w_d->Ax,u_d->Ax,v_d->Ax);
  return w;
}

 /* *****************************************************************
   inputting a matrix from an already opened file
*/
vint *getmat(FILE *file, int n, int m)
{
  int i, j;
  vint *a, *p;
  if (n<=0 || m<=0) 
    { perror("wrong dimesions in getmat"); exit(1);}

  /* NB: the reason for adding one extra elt to the length of a is
     to prevent fscanf writing to already allocated area when
     writing the last element of a */
  if (NULL==(a=(vint *)calloc(n*m+1,sizeof(vint)))) 
    {perror("out of memory in getmat"); exit(7);}

  for (i=0, p=a; i<n; i++)
    for (j=0; j<m; j++) 
      {
	if (!fscanf(file,"%d",p++)) 
	  { perror("input error in getmat"); exit(1); }
      }
  return a;
}
 
struct adj1_pars {GRAPH *g; VERT *v;};

int adjust_adj1(VERT *u, struct adj1_pars *p)
{
  if (u != (p->v))
    tree_add(&((p->v)->adjacency),(p->g)->compare,u,NULL);
  return 1;
}

int adjust_adj(VERT *v, GRAPH *g)
{
  struct adj1_pars pars;
  SET **vertices;
  vertices=&(g->vertices);
  
  tree_init(&(v->adjacency));
  v->degree=((VERT_DATA *)(v->data))->ncols - 1;
  
  pars.v=v;
  pars.g=g;
  tree_trav_pars(vertices,adjust_adj1,&pars); /* traversing the vertices
					     and copying all of them except
					     v itself */
  return 1;
}

GRAPH *cr_graph(int n, int m, vint *a)
{
  VERT *w;
  VERT_DATA *w_d;
  GRAPH *g;
  int i,j;
  vint *p, *q;

  g=graph_init(compa);

  for (i=0; i<m; i++) /* creating null graph on m vertices */
    {
      w=cr_empty_vert(n,m);
      w_d=w->data;

      for (j=0,p=(vint *)(w->key); j<m; j++) *(p++)=0;
      *(((vint *)(w->key)) + i)=1;

      for (j=0,p=a+i, q=w_d->Ax; j<n; j++, p+=m) *(q++)=*p;
      w_d->degree=1;
      w->degree=0;
      w->adjacency=NULL;

      graph_add_vertex(g,w);
    }

#ifdef DEBUG_graph
  graph_print(g);
#endif

  tree_trav_pars(&(g->vertices), adjust_adj, g); /* adjusting the 
						    neighbourhoods */
  return g;
}

/*
#ifdef DEBUG_graph
int ext_idx;
VERT **ext_vvvv;

int v_store(VERT *u)
{
  extern int ext_idx;
  extern VERT **ext_vvvv;
  *(ext_vvvv+(ext_idx++))=u;
  return 1;
}
#endif
*/

main()
{
  extern int prtlev;
  extern vint old_deg;
  vint *a;
  GRAPH *g;
  FILE *inp=stdin, *outbase;
  int n,m, process_pq(SET **, GRAPH *, BASE *);
  SET *pq, *init_pq(GRAPH *);
  BASE *b;

  old_deg=1;
  fscanf(inp,"%d %d %d", &prtlev, &n, &m);
  a=getmat(inp,n,m);

  outbase=fopen("base","w");
  init_BASE(&b,m,outbase);
  g=cr_graph(n,m,a);
  /*  graph_print(g); */
  pq=init_pq(g);

  while(process_pq(&pq,g,b));

  /* graph_print(g); */
  printf(" \n %d elements in the basis found\n", b->size);
  fclose(outbase);
}

/* 
#ifdef DEBUG_graph
  extern int ext_idx;
  extern VERT **ext_vvvv;
  vint *p;
  int i,j;

  graph_print(g);
  ext_idx=0;
  ext_vvvv=(VERT **)calloc(m,sizeof(VERT *));
  tree_trav(&(g->vertices), v_store);
  printf("\n keys:\n");
  for (i=0; i<m; i++)
    { printf("\n %3d. ",i+1);
      for (j=0, p=(vint *)(*(ext_vvvv+i))->key; j<m; j++) 
	printf("%3d",*(p++));
    }
  graph_split_edge(g,ext_vvvv[0],ext_vvvv[2],cr_vert);
  graph_print(g);
  
#endif
*/

