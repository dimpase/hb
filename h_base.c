/* ************************************************************************
 ** h_base.c               HB package version 0.1 (experimental)
 ** 
 ** Copyright (C) 1997 Dmitrii V. Pasechnik
 **                    SSOR/TWI, TU Delft, The Netherlands
 ** 
 ** Handling the accumulation of HB: reducibility test, insertion
* *********************************************************************** */
#include "defs.h" /* standard common definitions */
#include "h_base.h"
#undef DEBUG_h_base
#undef DEBUG_masks

#ifdef DEBUG_h_base
#define DEBUG_masks
#endif

#ifdef DEBUG_masks
static vint *tm;
#endif

#define get_vi_entry(p)  if (NULL==((p=(struct vi_entry *)malloc(sizeof(struct vi_entry))))) {perror("out of memory in insert_base_v"); exit(7);}

/* ************************************************************************
   computing the mask of a vector
   ************************************************************************ */
static MASK get_mask(int n, vint *v)
{
  MASK mask, one_bit;
  int i;

  for (i = 0, one_bit = 1, mask = 0; i < n; i++) {
    if (*(v++)) mask = mask | one_bit;
    one_bit = one_bit << 1;
  }
  return mask;
}
    
/* ************************************************************************
   functions dealing with V_Is
   *********************************************************************** */
#ifdef DEBUG_h_base
static void dump_subtree(V_I *b, int lev)
{
  V_I *p;
  if ((p=b)) printf("\n %3d. ",lev);
  else printf("\n empty (sub)tree ");
  while (p)
    { printf("%4d",p->vi);
      p=p->next;
    }
  p=b;
  while (p)
    { dump_subtree(p->down,lev+1);
      p=p->next;
    }
}
static void dump_base(V_I *b) /* raw dump of the basis */
{
  printf("\n *** dumping the basis ***\n");
  dump_subtree(b,0);
}
#endif

/* checking if v is reducible */
static int is_reducible_v(vint *v,     /* vector */
		 V_I *h  /* basis  */
		 )
{
  register V_I *p;


  if (!(p=h)) return 0;   /* irreducible */

  if (!p->down)           /* the last level */
      return (p->vi <= *v);
  else
    {
      while (p && p->vi <= *v)
	{ if (is_reducible_v(v+1,p->down)) return 1;
	  p=p->next;
	}
      return 0;
    }
}


/* inserting *new* v in the basis 
   (inserting the same elt again would be a problem ) ;
   called recursively */

static void init_base_v(vint *v      /* vector */, 
		 int n        /* length of v */, 
		 V_I **h /* ptr to the beginning of the basis */)
{
  int i;

  V_I *p;
  
#ifdef DEBUG_h_base
  printf("\n init_base_v: n=%d;\n vector:",n);
  for (i=0; i<n; i++) printf("%3d",v[i]);
#endif

  get_vi_entry(p);
  *h=p;
  for(i=0; i<n-1; i++)
    { p->vi=*(v++);
      p->next=NULL;
      get_vi_entry(p->down);
      p=p->down;
    }
  p->vi=*v;
  p->next=NULL;
  p->down=NULL; 
}

static void insert_base_v(vint *v      /* vector      */, 
		 int n,         /* length of v */
		 V_I **h /* ptr to the beginning of the basis */)
{
  V_I *p, *p0;
  int lev;

#ifdef DEBUG_h_base
  int i;
  if (n<0) 
    {
      perror("panic!! too deep in insert_v_rq (negative n)");
      exit(10);
    }
  printf("\n insert_base_v: n=%d;\n vector:",n);
  for (i=0; i<n; i++) printf("%3d",v[i]);
#endif

  p0=p=*h; /* beginning of the list */
  while (p && (p->vi < *v)) 
    {
      p0=p;
      p=p->next;
    }

  if (p && p->vi == *v) /* so far sharing the data */
    insert_base_v(v+1, n-1, &(p->down));
  else
    { /* insert between p0 and p */
      V_I *pt;
      get_vi_entry(pt);
      pt->vi=*v;
      pt->next=p;
 
      if (p0 != p) p0->next=pt; /* beginning of the base ? */
      else *h=pt;

      if (n>0) init_base_v(v+1, n-1, &pt->down);
      else pt->down=NULL;
    }
}
/* *********************************************************************
   functions dealins with V_masks
   ********************************************************************* */

static void insert_base_mask(vint *v, /* vector */
		      int n, /* the length of v */ 
		      MASK mask, /* the mask of v */
		      V_masks **h /* pointer to the beginning of the
				     list of masks */ )
{
  V_masks *p, *last;
  
  p = last = *h;
  while (p && p->mask < mask) {/* skip the beginning of the list */
    last = p; 
    p = p->next;
  }

  if (p && mask == p->mask) insert_base_v(v, n, &(p->vectors));
  else { 
    V_masks *x;
    if (!(x = (V_masks *)malloc(sizeof(V_masks))))
      {perror("out of memory in insert_base_mask"); exit(7);}
    x->mask = mask;
    x->next = p;
    init_base_v(v, n, &(x->vectors));
    if (last == p) *h = x;
    else last->next = x;
  }
}

#ifdef DEBUG_masks
int testv(vint *a, int n, int m, vint *v)
{
  int i, j, res, *p;
  for (i=0; i<n; i++, a+=m) {
    for (res=0, j=0, p=a; !res && j<m;) res = *(p++) > v[j++];
    if (!res) { 
      printf("\n testv failed for the basis elt %d\n", i);
      for (j=0; j<m; j++) printf("%2d",*(a++));
      return 0;
    }
  }
  return 1;
}
#endif

static int is_reducible_mask(vint *v, /* vector */
		      V_masks *h, /* data for the given mask */
		      MASK mask)	  
{
#ifdef DEBUG_masks
  V_masks blah;
  blah=*h;
#endif
  while (h && (h->mask <= mask)) {
#ifdef DEBUG_masks
    printf("\n processing mask %u; cond=%d", 
	   h->mask, (mask & h->mask) == h->mask);
#endif
    if ((mask & h->mask) == h->mask)
      if (is_reducible_v(v, h->vectors)) return 1;
    h = h->next;
  }
#ifdef DEBUG_masks
  if (!testv(tm, 14, 8, v)) {
    printf("\n *** REDUCIBLE VECTOR!!! ***\n");
  }
#endif
  return 0;
}
/* ************************************************************************
   functions dealing with BASE and DEG_BASE
   *********************************************************************** */
/* ***exported functions*** */


void init_BASE(BASE **b, int n, FILE *outfile)
{
#ifdef DEBUG_masks
  tm = getmat(fopen("b","r"), 14, 8);
#endif

  if (!(*b=(BASE *)malloc(sizeof(BASE))))
    {perror("out of memory in init_BASE"); exit(7);}    
  if (n > MAX_MASK)
    {perror("init_BASE: cannot handle that long vectors"); exit(7);}    
  /*  (*b)->deg_min=0;
      (*b)->deg_max=0; */
  (*b)->start=NULL;
  (*b)->n=n;
  (*b)->size = 0;
  (*b)->outfile=outfile;
}

static void clean_vi(V_I *x)
{
  if (x) {
      clean_vi(x->next);
      clean_vi(x->down);
      free(x);
  }
}

void clean_BASE(BASE *b)
{
    DEG_BASE *p, *pn;
    V_masks *v, *vn;
    p = b->start;
    while (p) {
#ifdef DEBUG_h_base
        printf("\n cleaning degree %d", p->degree);
#endif
        v = p->vectors;
	while (v) {
	    clean_vi(v->vectors);
            vn = v->next;
	    free(v);
	    v = vn;
	}
	pn = p->next;
	free(p);
	p = pn;
    }
}
int reducible_vector(BASE *b, vint deg, int inker, vint *v)
{
  DEG_BASE *p=b->start;
  MASK mask = get_mask(b->n, v);

#ifdef DEBUG_masks
  int i;
  printf("\n reducible_vector: vector of deg=%d with mask %u:",deg, mask);
  for (i=0; i<b->n; i++) printf("%2d",v[i]);
#endif

  if (!inker) deg--; /* Av<>0; no need to check same degree vectors */
  p=b->start;
  while (p && p->degree <= deg)
    {
#ifdef DEBUG_masks
      printf("\n checking degree %d ",p->degree);
#endif
      if (is_reducible_mask(v, p->vectors, mask)) {
#ifdef DEBUG_masks
	printf("\n rejected");
#endif
	return 1;
      }
      p=p->next;
    }
#ifdef DEBUG_masks
  printf("\n OK");
#endif
  return 0;
}
      

void insert_base_vector(BASE *b, vint deg, vint *v)
/* it is assumed that the degree of vectors can only increase
   as the algorithm works */
{
  DEG_BASE *p, *last;
  int i; 
  MASK mask;

  (b->size)++;
#ifdef DEBUG_masks
  printf("\n INSERT BASE %d:\n vector of deg=%d with mask %u:",
	 b->size,deg, mask);
  for (i=0; i<b->n; i++) printf("%2d",v[i]);
#endif

  last=p=b->start;
  mask = get_mask(b->n, v);
  if (b->outfile)
    {
      fprintf(b->outfile,"\n");
      for (i=0; i<b->n; i++) fprintf(b->outfile,"%3d",v[i]);
#ifdef DEBUG_masks
      fflush(b->outfile);
#endif
    }
  while (p && p->degree < deg) /* skip smaller degrees */
    { last=p; p=p->next; }

  if (p) /* already have this degree present */
    insert_base_mask(v, b->n, mask, &(p->vectors));
  
  else    /* have to initialize */
    {
      if (!(p=(DEG_BASE *)malloc(sizeof(DEG_BASE))))
	{perror("out of memory in insert_base_vector"); exit(7);}
      p->degree=deg;
      p->vectors = NULL;
      p->next = NULL;
      insert_base_mask(v, b->n, mask, &(p->vectors));
      if (last) last->next=p;
      else b->start=p;
    }
}
