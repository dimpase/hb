/* ************************************************************************
 ** defs.c               HB package version 0.1 (experimental)
 ** 
 ** Copyright (C) 1997 Dmitrii V. Pasechnik
 **                    SSOR/TWI, TU Delft, The Netherlands
 ** standard common definitions 
 ** 
* *********************************************************************** */
#ifndef _DEFS_HB
#define _DEFS_HB

#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

typedef int vint; /* vector entries */

#undef DEBUG       /* set debugging on */
#define DEBUG_compile       /* compile debugging procedures */
#undef DEBUG_graph
#define MAXN    60  /* maximal # of rows */
#define MAXM    60  /* maximal # of columns */

typedef struct {
  int nrows;     /* # of rows in A */
  int ncols;     /* # of columns in A */
  vint degree;   /* sum of the key entries */
  int activ_ctr; /* # of appearances in the edges of the pr. queue */
  vint *Ax;} VERT_DATA;

extern void vertex_print(VERT *);
#endif
