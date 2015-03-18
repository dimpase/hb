/* ************************************************************************
 ** h_base.h               HB package version 0.2 (experimental)
 ** 
 ** Copyright (C) 1997 Dmitrii V. Pasechnik
 **                    SSOR/TWI, TU Delft, The Netherlands
 ** Headers for
 ** handling the accumulation of HB: reducibility test, insertion
* *********************************************************************** */

#include <limits.h>
typedef unsigned int MASK;

#define MAX_MASK (sizeof(MASK) * CHAR_BIT) /* how many elements are allowed
					      in a vector */

typedef struct vi_entry { /* a node in the basis representation */
  vint vi;
  struct vi_entry *next;         /* bigger on the same level */  
  struct vi_entry *down;         /* smallest one level down */
} V_I;

typedef struct _V_masks { 
  V_I *vectors;
  struct _V_masks *next; /* bigger mask */
  MASK mask;
} V_masks;

typedef struct _deg_base { /* a node in the list of lists 
			      (one for each given degree) ordered by degree */
  V_masks *vectors;
  struct _deg_base *next;
  vint degree;
} DEG_BASE;

typedef struct {
  /*  vint deg_min; */   /* degrees: minimal */
  /*  vint deg_max; */   /*      and maximal */
  DEG_BASE *start; /* beginning of the list */
  int n;           /* length of the vectors */
  int size;        /* number of the elements */
  FILE *outfile;
} BASE;

/* ***exported functions*** */

void init_BASE(BASE **, int , FILE *);
int reducible_vector(BASE *, vint, int, vint *);
void insert_base_vector(BASE *b, vint deg, vint *v);
