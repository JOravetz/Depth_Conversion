#include "gmt.h"
#include <stdio.h>
#include <stddef.h>
#include "par.h"
#include "su.h"
#include "segy.h"

#define abs(x)   ( (x) <  0  ? -(x) : (x) )
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

segy  tr;
float *f1, *f2, *f3;

int main(int argc, char **argv) {

char   *coeff_x1, *coeff_x2, *coeff_x3, file[BUFSIZ];
struct GRD_HEADER grd_x1, grd_x2, grd_x3;
struct GMT_EDGEINFO edgeinfo_x1, edgeinfo_x2, edgeinfo_x3;
struct GMT_BCR bcr_x1, bcr_x2, bcr_x3;
double value_coeff_x1, value_coeff_x2, value_coeff_x3;

int imin, ntr, ns;
float scalar, x, y, dt, delrt;
short verbose;
register int i;

initargs(argc, argv);
argc = GMT_begin (argc, argv);

if (!getparshort("verbose", &verbose)) verbose = 1;
if (!getparstring("coeff_x1", &coeff_x1)) coeff_x1="top.grd";
if (!getparstring("coeff_x2", &coeff_x2)) coeff_x2="bottom.grd";
if (!getparstring("coeff_x3", &coeff_x3)) coeff_x3="bottom_vavg.grd";

ntr = gettra (&tr,  0);
delrt = tr.delrt;
if (!getparint("ns",  &ns))  ns  = tr.ns;
scalar = abs ( tr.scalco );
if ( scalar == 0 ) scalar = 1;

dt  = tr.dt  * 0.001;
imin = nint ( delrt / dt );

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "Delay = %f ms., First sample = %d\n", delrt, imin );
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "GMT TOP TWT grid file name = %s\n", coeff_x1 );
   fprintf ( stderr, "GMT BOTTOM TWT grid file name = %s\n", coeff_x2 );
   fprintf ( stderr, "GMT BOTTOM Average-Velocity grid file name = %s\n", coeff_x3 );
   fprintf ( stderr, "\n" );
}

GMT_boundcond_init (&edgeinfo_x1);
GMT_boundcond_init (&edgeinfo_x2);
GMT_boundcond_init (&edgeinfo_x3);

GMT_grd_init (&grd_x1, argc, argv, FALSE);
GMT_grd_init (&grd_x2, argc, argv, FALSE);
GMT_grd_init (&grd_x3, argc, argv, FALSE);

if (GMT_read_grd_info (coeff_x1, &grd_x1)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
if (GMT_read_grd_info (coeff_x2, &grd_x2)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
if (GMT_read_grd_info (coeff_x3, &grd_x3)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);

f1 = (float *) GMT_memory (VNULL, (size_t)((grd_x1.nx + 4) * (grd_x1.ny + 4)), sizeof(float), GMT_program);
f2 = (float *) GMT_memory (VNULL, (size_t)((grd_x2.nx + 4) * (grd_x2.ny + 4)), sizeof(float), GMT_program);
f3 = (float *) GMT_memory (VNULL, (size_t)((grd_x3.nx + 4) * (grd_x3.ny + 4)), sizeof(float), GMT_program);

GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

GMT_boundcond_param_prep (&grd_x1, &edgeinfo_x1);
GMT_boundcond_param_prep (&grd_x2, &edgeinfo_x2);
GMT_boundcond_param_prep (&grd_x3, &edgeinfo_x3);

GMT_boundcond_set (&grd_x1, &edgeinfo_x1, GMT_pad, f1);
GMT_boundcond_set (&grd_x2, &edgeinfo_x2, GMT_pad, f2);
GMT_boundcond_set (&grd_x3, &edgeinfo_x3, GMT_pad, f3);

GMT_bcr_init (&grd_x1, GMT_pad, BCR_BSPLINE, 1, &bcr_x1);
GMT_bcr_init (&grd_x2, GMT_pad, BCR_BSPLINE, 1, &bcr_x2);
GMT_bcr_init (&grd_x3, GMT_pad, BCR_BSPLINE, 1, &bcr_x3);

GMT_read_grd (coeff_x1, &grd_x1, f1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
GMT_read_grd (coeff_x2, &grd_x2, f2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
GMT_read_grd (coeff_x3, &grd_x3, f3, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

int istart, istop;
float gradient, diff, velocity;
register int j;

istart = istop = 0;
velocity = 0.0;

rewind (stdin);
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   x = tr.sx / scalar;
   y = tr.sy / scalar;

   if ( x >= grd_x1.x_min && x <= grd_x1.x_max && y >= grd_x1.y_min && y <= grd_x1.y_max ) {
      value_coeff_x1 = abs ( GMT_get_bcr_z (&grd_x1, x, y, f1, &edgeinfo_x1, &bcr_x1) );
      value_coeff_x2 = abs ( GMT_get_bcr_z (&grd_x2, x, y, f2, &edgeinfo_x2, &bcr_x2) );
      value_coeff_x3 = abs ( GMT_get_bcr_z (&grd_x3, x, y, f3, &edgeinfo_x3, &bcr_x3) );

      if ( ! ( (GMT_is_dnan (value_coeff_x1) || GMT_is_dnan (value_coeff_x2) ) ) ) {
         istart = min ( max ( nint ( value_coeff_x1 / dt ) - imin, 0 ), ns );
         istop  = min ( max ( nint ( value_coeff_x2 / dt ) - imin, 0 ), ns );
      }

      if ( ! ( GMT_is_dnan (value_coeff_x3) ) ) {
         if ( istop > istart ) {
            velocity = tr.data[istop]; 
            diff = value_coeff_x3 - velocity;
            gradient = diff / ( istop - istart ); 
            for ( j = istart+1; j <= istop ; ++j ) tr.data[j] += ( j - istart ) * gradient;
            if ( verbose == 2 ) fprintf ( stderr, "Trace = %5d, X = %12.2f Y = %12.2f, Top TWT = %10.2f, Bottom TWT = %10.2f, Bottom VAVG = %12.4f, Original Seismic Velocity = %12.4f, Difference = %12.6f, Final Seismic Velocity = %12.4f, Istart = %5d, Istop = %5d\n", 
               i, x, y, value_coeff_x1, value_coeff_x2, value_coeff_x3, velocity, diff, tr.data[istop], istart, istop ); 
         }
         puttr ( &tr );
      } else {
         fprintf ( stderr, "Trace = %5d, X = %12.2f Y = %12.2f, Top TWT = %10.2f, Bottom TWT = %10.2f, Bottom VAVG = %12.4f, Original Seismic Velocity = %12.4f\n", i, x, y, value_coeff_x1, value_coeff_x2, value_coeff_x3, velocity ); 
      }
   }
} 

GMT_free ((void *)f1);
GMT_free ((void *)f2);
GMT_free ((void *)f3);

GMT_end (argc, argv);

return EXIT_SUCCESS;

}
