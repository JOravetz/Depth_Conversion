#include <stdio.h>
#include <stddef.h>
#include "nr.h"
#include "nrutil.h"
#include "par.h"
#include "su.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

int main(int argc, char **argv) {

char temp[256]; 
int  num, kount, *idx, n;

double *x, *y, twt, depth;
double ri, **A, *B, d, sum;
short verbose, intercept;
register int i,j, k;

/* Initialize */
initargs(argc, argv);
requestdoc(0);

if (!getparint("n", &n)) n = 3;
if (!getparshort("intercept", &intercept)) intercept = 0;
if (!getparshort("verbose", &verbose)) verbose = 1;

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Intercept = %d\n", intercept );
   fprintf ( stderr, "\n" );
}

A  = dmatrix (1,n,1,n);
B  = dvector (1,n);

x = dvector (0,1000);
y = dvector (0,1000);

kount = -1;
while (NULL != fgets ( temp, sizeof(temp), stdin )) {
   (void) sscanf ( ((&(temp[0]))), "%lf%lf", &twt, &depth );
   ++kount;
   x[kount] = twt;
   y[kount] = depth;
}

num = kount + 1;
if ( verbose ) fprintf ( stderr, "Number of input values read = %d\n", num );

for (i=1;i<=n;i++) {
   for (j=1;j<=n;j++) {
      sum = 0.0;
      ri = i+j-2;
      for (k=0;k<num;k++) sum += pow ( x[k], ri );
      if (i>1&&j==1&&intercept==0) sum = 0.0;
      A[i][j] = sum;
   }
}

for (i=1;i<=n;i++) {
   sum = 0.0;
   ri = i - 1;
   for (j=0;j<num;j++) sum += pow( x[j], ri ) * y[j];
   B[i] = sum;
}

idx  = ivector (1, n);
ludcmp ( A, n, idx, &d );
lubksb ( A, n, idx, B );

if ( verbose ) {
   if ( intercept ) {
      for (i=1;i<=n;i++) printf ( "coefficient number = %5d, Solution vector = %30.25f\n", i, B[i] );
   } else {
      for (i=2;i<=n;i++) printf ( "coefficient number = %5d, Solution vector = %30.25f\n", i, B[i] );
   }
} else {
   if ( intercept ) {
      for (i=1;i<n;i++) printf ( "%30.25f ", B[i] );
   } else {
      for (i=2;i<n;i++) printf ( "%30.25f ", B[i] );
   }
   printf ( "%30.25f\n", B[n] );
}


return EXIT_SUCCESS;

}
