#include <stdio.h>
#include <stddef.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"

#define abs(x)   ( (x) <  0  ? -(x) : (x) )
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

segy tr;

int main(int argc, char **argv) {

int ntr, ns;
double dt, t, factor;
double *vavg, *twt;
short verbose;
register int i, j;

initargs(argc, argv);
requestdoc (0);

if (!getparshort("verbose", &verbose)) verbose = 1;

ntr = gettra (&tr,  0);
ns = tr.ns;
dt  = tr.dt  * 0.001;

vavg  = ealloc1double ( ns );
twt = ealloc1double ( ns );

factor = 0.0005;

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "\n" );
}

int num;
double vavg_last, v;

rewind (stdin);
num = 0;
for ( i = 0; i < ntr; ++i ) {
   gettr ( &tr );
   num = 0;
   twt[0] = 0.0;
   vavg[0] = vavg_last = tr.data[0];
   for ( j = 1; j < ns; ++j ) {
      t = j * dt;
      if ( tr.data[j] > vavg_last ) {
         ++num;
         twt[num] = t;
         vavg[num] = vavg_last = tr.data[j];
      } else {
         vavg_last = tr.data[j];
      }
   }

   for ( j = 0; j < ns; ++j ) {
      t = j * dt;
      dintlin ( num+1, twt, vavg, vavg[0], vavg[num], 1, &t, &v );
      tr.data[j] = v;
   }
   puttr (&tr);
} 

free1double (twt);
free1double (vavg);

return EXIT_SUCCESS;

}
