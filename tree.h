/* tree.h - declare structures used by tree.c
 * vix 27jun86 [broken out of tree.c]
 * vix 22jan93 [revisited; uses RCS, ANSI, POSIX; has bug fixes]
 *
 * $Id:$
 */


#ifndef	_TREE_FLAG
#define	_TREE_FLAG


#ifdef __STDC__
typedef	void *	tree_t;
#define __PPP(x) x
#else
typedef	char *	tree_t;
#define	__PPP(x) ()
#endif


typedef	struct	tree_s
	{
		struct	tree_s	*tree_l, *tree_r;
		short		tree_b;
		tree_t		tree_p;
	}
	tree;


void	tree_init	__PPP( (tree **) );
tree_t	tree_srch	__PPP( (tree **, int (*)(), tree_t) );
void	tree_add	__PPP( (tree **, int (*)(), tree_t, void (*)()) );
int	tree_delete	__PPP( (tree **, int (*)(), tree_t, void (*)()) );
int	tree_trav	__PPP( (tree **, int (*)()) );
void	tree_mung	__PPP( (tree **, void (*)()) );


#undef __PPP


#endif	/* _TREE_FLAG */
