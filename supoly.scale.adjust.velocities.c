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
/* Credits:  Joe J. Oravetz, 11/08/2013 */

segy tr;

int main(int argc, char **argv) {

int ntr, ns, imin;
double x, A, B, delrt, dt;
short verbose;
register int i, j;

initargs(argc, argv);
requestdoc (0);

if (!getparshort("verbose", &verbose)) verbose = 1;
if (!getpardouble("A", &A)) A = 0.9066915325471314179850424;
if (!getpardouble("B", &B)) B = 0.0000511522968963677346875;

ntr = gettra (&tr,  0);
delrt = tr.delrt;
if (!getparint("ns",  &ns)) ns = tr.ns;

dt  = tr.dt  * 0.001;
imin = nint ( delrt / dt );

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "Delay = %f ms., First sample = %d\n", delrt, imin );
   fprintf ( stderr, "Poly coeff A = %16.10f, Poly Coeff B = %16.10f\n", A, B );
   fprintf ( stderr, "\n" );
}

rewind ( stdin );
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   for ( j = 0; j < ns; ++j ) {
      x = tr.data[j];
      tr.data[j] = ( A * x ) + ( B * pow ( x, 2.0 ) );
   } 
   puttr (&tr);
} 

return EXIT_SUCCESS;

}
