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
double x, A, B, C, D, delrt, dt;
short verbose;
register int i, j;

initargs(argc, argv);
requestdoc (0);

if (!getparshort("verbose", &verbose)) verbose = 1;
if (!getpardouble("A", &A)) A =  6.9883573834307437522284090;
if (!getpardouble("B", &B)) B = -0.0106302922987734482090927;
if (!getpardouble("C", &C)) C =  0.0000062791508140867806855;
if (!getpardouble("D", &D)) D = -0.0000000012341086765951835;

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
   fprintf ( stderr, "A = %.20f, B = %.20f, C = %.20f, D = %.20f\n", A, B, C, D );
   fprintf ( stderr, "\n" );
}

rewind ( stdin );
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   for ( j = 0; j < ns; ++j ) {
      x = tr.data[j];
      tr.data[j] = ( A * x ) + ( B * pow ( x, 2.0 ) ) + ( C * pow ( x, 3.0 ) ) + ( D * pow ( x, 4.0 ) );
   } 
   puttr (&tr);
} 

return EXIT_SUCCESS;

}
