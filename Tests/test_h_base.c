#include "defs.h" /* standard common definitions */
#include "h_base.h"

main()
{
  int sum(int n, vint *v)
    {
      int s,i;
      for (i=0,s=0; i<n; i++) s+=*(v++);
      return s;
    }

  vint *a,*p;
  int n, i, r;
  BASE *b;

  scanf("%d",&n);
  a=(vint *)malloc(n*sizeof(vint));
  init_BASE(&b,n,NULL);
  p=a;
  while(scanf("%d",p)) /* ******** the loop NEVER ends !!!! *********** */
    {
      for(i=1; i<n; i++) scanf("%d",++p);
      p=a;
      printf("\n input vector: ");
      for(i=0; i<n; i++) printf("%3d",a[i]);
      printf("\n ");
      r=reducible_vector(b,sum(n,a),1,a);
      printf("\n reducible ? - %d",r);
      if (!r) insert_base_vector(b,sum(n,a),a);
    }
}

/* debugging iternals */
/*
main()
{
  vint z[5]={0,1,5,2,0};
  vint v[5]={0,1,0,2,4};
  vint x[5]={1,1,0,2,3};
  vint y[5]={7,1,0,2,3};
  vint a[5]={7,1,6,2,3};
  vint b[5]={1,1,0,3,2};
  int n;

  V_I *h;

  n=5;

  init_base_v(v,n,&h);
//  dump_base(h);

  insert_base_v(x,n,&h);
//  dump_base(h);

  printf("\n reducible ? - %d",is_reducible_v(y,h));
  printf("\n reducible ? - %d",is_reducible_v(z,h));
//  dump_base(h);

  insert_base_v(z,n,&h);
//  dump_base(h);
  printf("\n reducible ? - %d",is_reducible_v(a,h));
  printf("\n reducible ? - %d",is_reducible_v(b,h));

}
*/

