#include "tree.h"
#include "vixie.h"
#include <stdio.h>

/* passing extra parameter(s) to pfi_uar - something one
   cannot do in the original tree_trav() */
int tree_trav_pars(tree	**ppr_tree, int	(*pfi_uar)(), void *pars)
{
	ENTER("tree_trav")

	if (!*ppr_tree)
		EXIT(TRUE)

	if (!tree_trav_pars(&(**ppr_tree).tree_l, pfi_uar, pars))
		EXIT(FALSE)
	if (!(*pfi_uar)((**ppr_tree).tree_p, pars))
		EXIT(FALSE)
	if (!tree_trav_pars(&(**ppr_tree).tree_r, pfi_uar, pars))
		EXIT(FALSE)
	EXIT(TRUE)
}

/* a function to find the minimal elt in a tree 
 (derived from tree_srch) */
tree_t tree_min(tree **ppr_tree)
{
  register tree *p;

  p=*ppr_tree;
  if (!p) return NULL; /* empty tree */
  while (p->tree_l) p=p->tree_l;
  return p->tree_p;
}

