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

   short  check, verbose;
   int    delrt, nz, ntr, ns;
   float  scale_factor;
   double water_depth, ratio, factor1, dz, x_loc, y_loc;
   double delrt_depth, value_coeff_x, value_coeff_x2, value_coeff_x3, tr_msec, dt_msec;
   float *tr_amp, *depth, depth_input, amp_output;
   register int k, n;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring("coeff_x", &coeff_x)) coeff_x="wb.twt.grd";

   if (!getparstring("coeff_x2", &coeff_x2)) {
      fprintf ( stderr, "Must supply Coefficient_X2 GMT grid (COEFF_X2 Parameter or K grid)--> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getparstring("coeff_x3", &coeff_x3)) {
      fprintf ( stderr, "Must supply Coefficient_X3 GMT grid (COEFF_X3 Parameter or VZERO grid)--> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getparshort("verbose" , &verbose)) verbose = 0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "X1 Coefficient GMT grid file name (WB_TWT) = %s\n", coeff_x );
      fprintf ( stderr, "X2 Coefficient GMT grid file name (K Grid) = %s\n", coeff_x2 );
      fprintf ( stderr, "X3 Coefficient GMT grid file name (VZERO Grid) = %s\n", coeff_x3 );
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

   GMT_boundcond_set (&grd_x, &edgeinfo_x, GMT_pad, f1);
   GMT_boundcond_set (&grd_x2, &edgeinfo_x2, GMT_pad, f2);
   GMT_boundcond_set (&grd_x3, &edgeinfo_x3, GMT_pad, f3);

   GMT_bcr_init (&grd_x,  GMT_pad, BCR_BSPLINE, 1, &bcr_x);
   GMT_bcr_init (&grd_x2, GMT_pad, BCR_BSPLINE, 1, &bcr_x2);
   GMT_bcr_init (&grd_x3, GMT_pad, BCR_BSPLINE, 1, &bcr_x3);

   GMT_read_grd (coeff_x,  &grd_x,  f1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x2, &grd_x2, f2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x3, &grd_x3, f3, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   /* Get info from first trace */
   ntr   = gettra (&tr, 0);
   ns    = tr.ns;
   delrt = tr.delrt;
   dt_msec = tr.dt * 0.001;
   scale_factor = tr.scalco;
   if (scale_factor < 0.0 ) scale_factor *= -1.0;
   if (scale_factor == 0.0 ) scale_factor = 1.0;

   if (!getpardouble ("dz",&dz)) dz = 2;
   if (!getparint    ("nz",&nz)) nz = ns;

   if ( verbose ) {
      fprintf ( stderr, "Output depth sample rate (dz) = %f\n", dz );
      fprintf ( stderr, "Number of output depth samples per trace = %d\n", nz );
      fprintf ( stderr, "Number of traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "Time sample rate (milliseconds) = %f\n", dt_msec );
      fprintf ( stderr, "Delay (delrt) = %d milliseconds\n", delrt );
      fprintf ( stderr, "Scale Factor for X and Y Coordinates = %f\n", scale_factor );
      fprintf ( stderr, "\n" );
   }

   rewind (stdin);

   if ( ns > nz ) {
      depth = ealloc1float ( ns );
   } else {
      depth = ealloc1float ( nz );
   }
   tr_amp = ealloc1float ( ns );

   factor1 = 0.0005;
   delrt_depth = 0.0;

   double delrt_depth_min, zero;
   delrt_depth_min = 0.0;
   zero = 0.0;

   if ( delrt ) { 
      delrt_depth_min = FLT_MAX;
      rewind (stdin);
      for ( k = 0; k < ntr; ++k ) {
         gettr (&tr);
         x_loc = tr.sx = nint ( tr.sx / scale_factor );
         y_loc = tr.sy = nint ( tr.sy / scale_factor );

         check = 0;
         if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) check = 1;

         if ( check ) {
            value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
            value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);
            value_coeff_x3 = GMT_get_bcr_z (&grd_x3, x_loc, y_loc, f3, &edgeinfo_x3, &bcr_x3);

            if (!(GMT_is_dnan (value_coeff_x) || GMT_is_dnan (value_coeff_x2) || GMT_is_dnan (value_coeff_x3))) {

               if ( value_coeff_x < zero ) value_coeff_x   *= -1.0;
               if ( value_coeff_x2 < zero ) value_coeff_x2 *= -1.0;
               if ( value_coeff_x3 < zero ) value_coeff_x3 *= -1.0;

               water_depth = value_coeff_x * factor1 * value_coeff_x3;
               ratio = value_coeff_x3 / value_coeff_x2;
               if ( delrt <= value_coeff_x ) {
                  delrt_depth = delrt * factor1 * value_coeff_x3;
               } else if ( delrt > value_coeff_x ) {
                  delrt_depth = ( ratio * (exp (value_coeff_x2*(delrt-value_coeff_x)*factor1) - 1.0) ) + water_depth;
               }
               delrt_depth_min = min ( delrt_depth, delrt_depth_min );
            }
         }
      }
      if ( verbose ) fprintf ( stderr, "Delrt depth min = %.2f\n", delrt_depth_min );
   } 

   /* Main loop over traces */
   rewind (stdin);
   for ( k = 0; k < ntr; ++k ) {
      gettr (&tr);
      x_loc = tr.sx = nint ( tr.sx / scale_factor );
      y_loc = tr.sy = nint ( tr.sy / scale_factor );
      for ( n=ns; n < nz; ++n ) depth[n] = 0;

      check = 0;
      if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) check = 1;

      if ( check ) {
         value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
         value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);
         value_coeff_x3 = GMT_get_bcr_z (&grd_x3, x_loc, y_loc, f3, &edgeinfo_x3, &bcr_x3);

         if (!(GMT_is_dnan (value_coeff_x) || GMT_is_dnan (value_coeff_x2) || GMT_is_dnan (value_coeff_x3))) {

            if ( value_coeff_x < zero ) value_coeff_x   *= -1.0;
            if ( value_coeff_x2 < zero ) value_coeff_x2 *= -1.0;
            if ( value_coeff_x3 < zero ) value_coeff_x3 *= -1.0;

	    if ( verbose == 2 ) fprintf ( stderr, "Trace num = %d, X-Loc = %f, Y-Loc = %f, WB_TWT (msec) = %0.10f, K = %0.10f, VZERO = %.4f\n",
            k+1, x_loc, y_loc, value_coeff_x, value_coeff_x2, value_coeff_x3 );

            water_depth = value_coeff_x * factor1 * value_coeff_x3;
            ratio = value_coeff_x3 / value_coeff_x2;

	    for ( n=0; n < ns; ++n ) {
               tr_amp[n] = tr.data[n];
	       tr_msec   = ( n * dt_msec ) + delrt;
               if ( tr_msec <= value_coeff_x ) {
                  depth[n] = tr_msec * factor1 * value_coeff_x3;
               } else {
	          tr_msec = ( tr_msec - value_coeff_x ) * factor1;
                  depth[n] = ( ratio * (exp (value_coeff_x2*tr_msec) - 1.0) ) + water_depth;
               }
               if ( verbose == 3)  {
                  fprintf ( stderr, "input trace = %6d, xloc = %8.2f yloc = %8.2f, vzero = %8.2f, k = %10.4f one-way time(sec) = %10.4f, depth sample = %6d depth value = %10.2f\n", 
                  k, x_loc, y_loc, value_coeff_x3, value_coeff_x2, tr_msec, n, depth[n] ); 
	       }
	    }
	    for ( n=0; n < nz; ++n ) {
	       depth_input = (n * dz) + delrt_depth_min;
      	       intlin ( ns, depth, tr_amp, tr_amp[0], tr_amp[ns-1], 1, &depth_input, &amp_output );
	       tr.data[n] = amp_output; 
            }
            tr.trid = 1;
            tr.scalco = 0;
            tr.delrt  = nint (delrt_depth_min);
	    tr.ns     = nz;
	    tr.dt     = nint(dz*1000);
	    puttr (&tr);
         }
      }
   }

   GMT_free ((void *)f1);
   GMT_free ((void *)f2);
   GMT_free ((void *)f3);

   free1float (depth);
   free1float (tr_amp);

   GMT_end (argc, argv);

   return (0);
}
