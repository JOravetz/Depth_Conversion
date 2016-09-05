#include "gmt.h"
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
float *f1, *f2;

int main(int argc, char **argv) {

char *coeff_x, *coeff_x2, file[BUFSIZ];

struct GRD_HEADER grd_x, grd_x2;
struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2;
struct GMT_BCR bcr_x, bcr_x2;

int ntr, ns, imin;
float scale_factor;
double x_loc, y_loc, x, A, B, delrt, dt;
short verbose;
register int i, j;

initargs(argc, argv);
argc = GMT_begin (argc, argv);

if (!getparshort("verbose", &verbose)) verbose = 1;
if (!getparstring("coeff_x", &coeff_x)) coeff_x="coeff.A.grd";
if (!getparstring("coeff_x2", &coeff_x2)) coeff_x2="coeff.B.grd";

ntr = gettra (&tr,  0);
delrt = tr.delrt;
if (!getparint("ns",  &ns)) ns = tr.ns;

dt  = tr.dt  * 0.001;
imin = nint ( delrt / dt );

scale_factor = tr.scalco;
if (scale_factor < 0.0 ) scale_factor *= -1.0;
if (scale_factor == 0.0 ) scale_factor = 1.0;

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "Delay = %f ms., First sample = %d\n", delrt, imin );
   fprintf ( stderr, "Poly coeff A GRID = %s, Poly Coeff B GRID = %s\n", coeff_x, coeff_x2 );
   fprintf ( stderr, "\n" );
}

GMT_boundcond_init (&edgeinfo_x);
GMT_boundcond_init (&edgeinfo_x2);

GMT_grd_init (&grd_x,  argc, argv, FALSE);
GMT_grd_init (&grd_x2, argc, argv, FALSE);

if (GMT_read_grd_info (coeff_x,  &grd_x))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
if (GMT_read_grd_info (coeff_x2, &grd_x2)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);

f1 = (float *) GMT_memory (VNULL, (size_t)((grd_x.nx  + 4) * (grd_x.ny  + 4)), sizeof(float), GMT_program);
f2 = (float *) GMT_memory (VNULL, (size_t)((grd_x2.nx + 4) * (grd_x2.ny + 4)), sizeof(float), GMT_program);

GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

GMT_boundcond_param_prep (&grd_x,  &edgeinfo_x);
GMT_boundcond_param_prep (&grd_x2, &edgeinfo_x2);

GMT_boundcond_set (&grd_x, &edgeinfo_x, GMT_pad, f1);
GMT_boundcond_set (&grd_x2, &edgeinfo_x2, GMT_pad, f2);

GMT_bcr_init (&grd_x,  GMT_pad, BCR_BSPLINE, 1, &bcr_x);
GMT_bcr_init (&grd_x2, GMT_pad, BCR_BSPLINE, 1, &bcr_x2);

GMT_read_grd (coeff_x,  &grd_x,  f1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
GMT_read_grd (coeff_x2, &grd_x2, f2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

rewind ( stdin );
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   x_loc = tr.sx = nint ( tr.sx / scale_factor );
   y_loc = tr.sy = nint ( tr.sy / scale_factor );

   if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) {
      A = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
      B = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);
      if ( verbose == 2 ) fprintf ( stderr, "Trace = %5d, X = %12.2f, Y = %12.2f, A_Coeff = %20.16f, B_Coeff = %20.16f\n", i+1, x_loc, y_loc, A, B );
      if ( ! ( GMT_is_dnan (A) || GMT_is_dnan (B) ) ) {
         for ( j = 0; j < ns; ++j ) {
            x = tr.data[j];
            tr.data[j] = ( A * x ) + ( B * pow ( x, 2.0 ) );
         } 
         puttr (&tr);
      } 
   } 
} 

return EXIT_SUCCESS;

}
