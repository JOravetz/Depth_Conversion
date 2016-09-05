#include "gmt.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"
#include "header.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

segy tr;
float  *f1, *f2, *f3;

int main (int argc, char **argv) {

   char *coeff_x, *coeff_x2, *coeff_x3, file[BUFSIZ];
	
   struct GRD_HEADER grd_x, grd_x2, grd_x3;
   struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2, edgeinfo_x3;
   struct GMT_BCR bcr_x, bcr_x2, bcr_x3;

   short  verbose;
   int    scalar, ntr, ns;
   double water_depth, ratio, factor1, x_loc, y_loc;
   double value_coeff_x, value_coeff_x2, value_coeff_x3, tr_msec, dt_msec;
   float depth;
   register int k, n;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring("coeff_x", &coeff_x)) coeff_x="wb.twt.grd";

   if (!getparstring("coeff_x2", &coeff_x2)) {
      fprintf ( stderr, "Must supply Coefficient_X2 GMT grid (COEFF_X2 Parameter)--> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getparstring("coeff_x3", &coeff_x3)) {
      fprintf ( stderr, "Must supply Coefficient_X3 GMT grid (COEFF_X2 Parameter)--> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getparshort("verbose" , &verbose)) verbose = 0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "WB (Smooth) GMT grid file name = %s\n", coeff_x );
      fprintf ( stderr, "VZERO Coefficient GMT grid file name = %s\n", coeff_x2);
      fprintf ( stderr, "K Coefficient GMT grid file name = %s\n", coeff_x3 );
      fprintf ( stderr, "\n" );
   }

   GMT_boundcond_init (&edgeinfo_x);
   GMT_boundcond_init (&edgeinfo_x2);
   GMT_boundcond_init (&edgeinfo_x3);

   GMT_grd_init (&grd_x,  argc, argv, FALSE);
   GMT_grd_init (&grd_x2, argc, argv, FALSE);
   GMT_grd_init (&grd_x3, argc, argv, FALSE);

   if (GMT_read_grd_info (coeff_x,  &grd_x))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_x2, &grd_x2)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_x3, &grd_x3)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
		
   f1 = (float *) GMT_memory (VNULL, (size_t)((grd_x.nx  + 4) * (grd_x.ny  + 4)), sizeof(float), GMT_program);
   f2 = (float *) GMT_memory (VNULL, (size_t)((grd_x2.nx + 4) * (grd_x2.ny + 4)), sizeof(float), GMT_program);
   f3 = (float *) GMT_memory (VNULL, (size_t)((grd_x3.nx + 4) * (grd_x3.ny + 4)), sizeof(float), GMT_program);

   GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

   GMT_boundcond_param_prep (&grd_x,  &edgeinfo_x);
   GMT_boundcond_param_prep (&grd_x2, &edgeinfo_x2);
   GMT_boundcond_param_prep (&grd_x3, &edgeinfo_x3);

   GMT_boundcond_set (&grd_x,  &edgeinfo_x,  GMT_pad, f1);
   GMT_boundcond_set (&grd_x2, &edgeinfo_x2, GMT_pad, f2);
   GMT_boundcond_set (&grd_x3, &edgeinfo_x3, GMT_pad, f3);

   GMT_bcr_init (&grd_x,  GMT_pad, BCR_BSPLINE, 1, &bcr_x);
   GMT_bcr_init (&grd_x2, GMT_pad, BCR_BSPLINE, 1, &bcr_x2);
   GMT_bcr_init (&grd_x3, GMT_pad, BCR_BSPLINE, 1, &bcr_x3);

   GMT_read_grd (coeff_x,  &grd_x,  f1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x2, &grd_x2, f2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x3, &grd_x3, f3, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   /* Get info from first trace */
   ntr     = gettra (&tr, 0);
   ns      = tr.ns;
   scalar  = abs ( tr.scalel );
   if ( scalar == 0 ) scalar = 1; 
   dt_msec = tr.dt * 0.001;

   if ( verbose ) {
      fprintf ( stderr, "Number of traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "Time sample rate (milliseconds) = %f\n", dt_msec );
      fprintf ( stderr, "Location scalar = %d\n", scalar );
      fprintf ( stderr, "\n" );
   }

   rewind (stdin);

   factor1 = 0.0005;
   depth = 0.0;

   /* Main loop over traces */
   for ( k = 0; k < ntr; ++k ) {
      gettr (&tr);
      x_loc = tr.sx / scalar;
      y_loc = tr.sy / scalar;

      if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) {

         value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
         value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);
         value_coeff_x3 = GMT_get_bcr_z (&grd_x3, x_loc, y_loc, f3, &edgeinfo_x3, &bcr_x3);

         if (GMT_is_dnan (value_coeff_x) || GMT_is_dnan (value_coeff_x2)|| GMT_is_dnan (value_coeff_x3)) {
            for ( n=0; n < ns; ++n )  tr.data[n] = 0;
            tr.trid = 0;
         } else {
	    if ( verbose == 2 ) fprintf ( stderr, "Trace num = %d, X-Loc = %f, Y-Loc = %f, WB = %0.10f, VZERO = %0.10f, K = %0.10f\n", 
              k+1, x_loc, y_loc, value_coeff_x, value_coeff_x2, value_coeff_x3 );

            water_depth = value_coeff_x * factor1 * value_coeff_x2;
            ratio = value_coeff_x2 / value_coeff_x3;
	    for ( n=0; n < ns; ++n ) {
	       tr_msec   = n * dt_msec;
               if ( tr_msec <= value_coeff_x ) {
                  tr.data[n] = value_coeff_x2;
               } else {
	          tr_msec = ( tr_msec - value_coeff_x ) * factor1;
                  depth = ( ratio * (exp (value_coeff_x3*tr_msec) - 1.0) ) + water_depth;
                  tr.data[n] = depth / ( n * dt_msec * factor1 );
               }
               if ( verbose == 2)  {
                  fprintf ( stderr, "input trace = %6d, xloc = %8.2f yloc = %8.2f, vzero = %8.2f, k = %10.4f one-way time(sec) = %10.4f, depth sample = %6d depth = %10.2f, average velocity = %8.2f\n", 
                  k, x_loc, y_loc, value_coeff_x2, value_coeff_x3, tr_msec, n, depth, tr.data[n] ); 
	       }
	    }
            tr.trid = 1;
         }
	 puttr (&tr);
      }
   }

   GMT_free ((void *)f1);
   GMT_free ((void *)f2);
   GMT_free ((void *)f3);

   GMT_end (argc, argv);

   return (0);
}
