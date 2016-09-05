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
float  *f_wb_twt, *f_k_top, *f_top, *f_k_middle, *f_middle, *f_k_bottom, *f_bottom;

int main (int argc, char **argv) {

   char *coeff_wb_twt,   *coeff_k_top, *coeff_top, file[BUFSIZ];
   char *coeff_k_middle, *coeff_middle;
   char *coeff_k_bottom, *coeff_bottom;
	
   struct GRD_HEADER   grd_wb_twt, grd_k_top, grd_top;
   struct GRD_HEADER   grd_k_middle, grd_middle;
   struct GRD_HEADER   grd_k_bottom, grd_bottom;
   struct GMT_EDGEINFO edgeinfo_wb_twt, edgeinfo_k_top, edgeinfo_top;
   struct GMT_EDGEINFO edgeinfo_k_middle, edgeinfo_middle;
   struct GMT_EDGEINFO edgeinfo_k_bottom, edgeinfo_bottom;
   struct GMT_BCR      bcr_wb_twt, bcr_k_top, bcr_top;
   struct GMT_BCR      bcr_k_middle, bcr_middle;
   struct GMT_BCR      bcr_k_bottom, bcr_bottom;

   short  verbose;
   int    kount, scalar, nz, ntr, ns;
   double water_depth, vwater, ratio, factor1, dz, x_loc, y_loc;
   double value_coeff_wb_twt, value_coeff_k_top, value_coeff_top; 
   double value_coeff_k_middle, value_coeff_middle; 
   double K, value_coeff_k_bottom, value_coeff_bottom; 
   double tr_msec, dt_msec;
   double *K_values, *TWT_values, *K_array, *Vavg_array, *Depth_array;
   float *tr_amp, *depth, depth_input, amp_output;
   register int k, n;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring("coeff_wb_twt",   &coeff_wb_twt))   coeff_wb_twt   = "wb.twt.grd";
   if (!getparstring("coeff_k_top",    &coeff_k_top))    coeff_k_top    = "top.k.grd";
   if (!getparstring("coeff_top",      &coeff_top))      coeff_top      = "top.grd";
   if (!getparstring("coeff_k_middle", &coeff_k_middle)) coeff_k_middle = "middle.k.grd";
   if (!getparstring("coeff_middle",   &coeff_middle))   coeff_middle   = "middle.grd";
   if (!getparstring("coeff_k_bottom", &coeff_k_bottom)) coeff_k_bottom = "bottom.k.grd";
   if (!getparstring("coeff_bottom",   &coeff_bottom))   coeff_bottom   = "bottom.grd";

   if (!getparshort("verbose" , &verbose)) verbose = 0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "WB_TWT   GMT grid file name = %s\n", coeff_wb_twt );
      fprintf ( stderr, "K_Top    GMT grid file name = %s\n", coeff_k_top );
      fprintf ( stderr, "K_Middle GMT grid file name = %s\n", coeff_k_middle );
      fprintf ( stderr, "K_Bottom GMT grid file name = %s\n", coeff_k_bottom );
      fprintf ( stderr, "Top      GMT grid file name = %s\n", coeff_top );
      fprintf ( stderr, "Middle   GMT grid file name = %s\n", coeff_middle );
      fprintf ( stderr, "Bottom   GMT grid file name = %s\n", coeff_bottom );
      fprintf ( stderr, "\n" );
   }

   GMT_boundcond_init (&edgeinfo_wb_twt);
   GMT_boundcond_init (&edgeinfo_k_top);
   GMT_boundcond_init (&edgeinfo_top);
   GMT_boundcond_init (&edgeinfo_k_middle);
   GMT_boundcond_init (&edgeinfo_middle);
   GMT_boundcond_init (&edgeinfo_k_bottom);
   GMT_boundcond_init (&edgeinfo_bottom);

   GMT_grd_init (&grd_wb_twt, argc, argv, FALSE);
   GMT_grd_init (&grd_k_top,  argc, argv, FALSE);
   GMT_grd_init (&grd_top,  argc, argv, FALSE);
   GMT_grd_init (&grd_k_middle,  argc, argv, FALSE);
   GMT_grd_init (&grd_middle,  argc, argv, FALSE);
   GMT_grd_init (&grd_k_bottom,  argc, argv, FALSE);
   GMT_grd_init (&grd_bottom,  argc, argv, FALSE);

   if (GMT_read_grd_info (coeff_wb_twt, &grd_wb_twt)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_k_top,  &grd_k_top))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_top,  &grd_top))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_k_middle,  &grd_k_middle))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_middle,  &grd_middle))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_k_bottom,  &grd_k_bottom))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_bottom,  &grd_bottom))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
		
   f_wb_twt = (float *) GMT_memory (VNULL, (size_t)((grd_wb_twt.nx  + 4) * (grd_wb_twt.ny  + 4)), sizeof(float), GMT_program);
   f_k_top  = (float *) GMT_memory (VNULL, (size_t)((grd_k_top.nx + 4)   * (grd_k_top.ny + 4)),   sizeof(float), GMT_program);
   f_top = (float *) GMT_memory (VNULL, (size_t)((grd_top.nx + 4)   * (grd_top.ny + 4)),   sizeof(float), GMT_program);
   f_k_middle  = (float *) GMT_memory (VNULL, (size_t)((grd_k_middle.nx + 4)   * (grd_k_middle.ny + 4)),   sizeof(float), GMT_program);
   f_middle = (float *) GMT_memory (VNULL, (size_t)((grd_middle.nx + 4)   * (grd_middle.ny + 4)),   sizeof(float), GMT_program);
   f_k_bottom  = (float *) GMT_memory (VNULL, (size_t)((grd_k_bottom.nx + 4)   * (grd_k_bottom.ny + 4)),   sizeof(float), GMT_program);
   f_bottom  = (float *) GMT_memory (VNULL, (size_t)((grd_bottom.nx + 4)   * (grd_bottom.ny + 4)),   sizeof(float), GMT_program);

   GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

   GMT_boundcond_param_prep (&grd_wb_twt, &edgeinfo_wb_twt);
   GMT_boundcond_param_prep (&grd_k_top,  &edgeinfo_k_top);
   GMT_boundcond_param_prep (&grd_top,  &edgeinfo_top);
   GMT_boundcond_param_prep (&grd_k_middle,  &edgeinfo_k_middle);
   GMT_boundcond_param_prep (&grd_middle,  &edgeinfo_middle);
   GMT_boundcond_param_prep (&grd_k_bottom,  &edgeinfo_k_bottom);
   GMT_boundcond_param_prep (&grd_bottom,  &edgeinfo_bottom);

   GMT_boundcond_set (&grd_wb_twt, &edgeinfo_wb_twt, GMT_pad, f_wb_twt);
   GMT_boundcond_set (&grd_k_top,  &edgeinfo_k_top,  GMT_pad, f_k_top);
   GMT_boundcond_set (&grd_top,  &edgeinfo_top,  GMT_pad, f_top);
   GMT_boundcond_set (&grd_k_middle,  &edgeinfo_k_middle,  GMT_pad, f_k_middle);
   GMT_boundcond_set (&grd_middle,  &edgeinfo_middle,  GMT_pad, f_middle);
   GMT_boundcond_set (&grd_k_bottom,  &edgeinfo_k_bottom,  GMT_pad, f_k_bottom);
   GMT_boundcond_set (&grd_bottom,  &edgeinfo_bottom,  GMT_pad, f_bottom);

   GMT_bcr_init (&grd_wb_twt, GMT_pad, BCR_BSPLINE, 1, &bcr_wb_twt);
   GMT_bcr_init (&grd_k_top,  GMT_pad, BCR_BSPLINE, 1, &bcr_k_top);
   GMT_bcr_init (&grd_top,  GMT_pad, BCR_BSPLINE, 1, &bcr_top);
   GMT_bcr_init (&grd_k_middle,  GMT_pad, BCR_BSPLINE, 1, &bcr_k_middle);
   GMT_bcr_init (&grd_middle,  GMT_pad, BCR_BSPLINE, 1, &bcr_middle);
   GMT_bcr_init (&grd_k_bottom,  GMT_pad, BCR_BSPLINE, 1, &bcr_k_bottom);
   GMT_bcr_init (&grd_bottom,  GMT_pad, BCR_BSPLINE, 1, &bcr_bottom);

   GMT_read_grd (coeff_wb_twt, &grd_wb_twt, f_wb_twt, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_k_top,  &grd_k_top,  f_k_top,  0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_top,  &grd_top,  f_top,  0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_k_middle,  &grd_k_middle,  f_k_middle,  0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_middle,  &grd_middle,  f_middle,  0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_k_bottom,  &grd_k_bottom,  f_k_bottom,  0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_bottom,  &grd_bottom,  f_bottom,  0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   /* Get info from first trace */
   ntr     = gettra (&tr, 0);
   ns      = tr.ns;
   scalar  = abs ( tr.scalel );
   if ( scalar == 0 ) scalar = 1; 
   dt_msec = tr.dt * 0.001;

   if (!getpardouble ("dz",&dz)) dz = 1;
   if (!getpardouble ("vwater",&vwater)) vwater = 1452.05;
   if (!getparint    ("nz",&nz)) nz = ns;

   if ( verbose ) {
      fprintf ( stderr, "Output depth sample rate (dz) = %f\n", dz );
      fprintf ( stderr, "Number of output depth samples per trace = %d\n", nz );
      fprintf ( stderr, "Number of traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "Time sample rate (milliseconds) = %f\n", dt_msec );
      fprintf ( stderr, "Vwater = %f (meters/second)\n", vwater );
      fprintf ( stderr, "Location scalar = %d\n", scalar );
      fprintf ( stderr, "\n" );
   }

   rewind (stdin);

   if ( ns > nz ) {
      depth = ealloc1float ( ns );
      K_values = ealloc1double ( ns ); 
      TWT_values = ealloc1double ( ns ); 
      K_array = ealloc1double ( ns ); 
      Vavg_array = ealloc1double ( ns ); 
      Depth_array = ealloc1double ( ns ); 
   } else {
      depth = ealloc1float ( nz );
      K_values = ealloc1double ( nz ); 
      TWT_values = ealloc1double ( nz ); 
      K_array = ealloc1double ( nz ); 
      Depth_array = ealloc1double ( nz ); 
   }
   tr_amp = ealloc1float ( ns );

   factor1 = 0.0005;

   /* Main loop over traces */
   for ( k = 0; k < ntr; ++k ) {
      gettr (&tr);
      x_loc = tr.sx / scalar;
      y_loc = tr.sy / scalar;
      kount = -1;
      for ( n=ns; n < nz; ++n ) depth[n] = 0;

      if ( x_loc >= grd_wb_twt.x_min && x_loc <= grd_wb_twt.x_max && y_loc >= grd_wb_twt.y_min && y_loc <= grd_wb_twt.y_max ) {
         value_coeff_wb_twt = GMT_get_bcr_z (&grd_wb_twt, x_loc, y_loc, f_wb_twt, &edgeinfo_wb_twt, &bcr_wb_twt);
         value_coeff_k_top  = GMT_get_bcr_z (&grd_k_top,  x_loc, y_loc, f_k_top,  &edgeinfo_k_top,  &bcr_k_top);
         value_coeff_top  = GMT_get_bcr_z (&grd_top,  x_loc, y_loc, f_top,  &edgeinfo_top,  &bcr_top);
         value_coeff_k_middle  = GMT_get_bcr_z (&grd_k_middle,  x_loc, y_loc, f_k_middle,  &edgeinfo_k_middle,  &bcr_k_middle);
         value_coeff_middle  = GMT_get_bcr_z (&grd_middle,  x_loc, y_loc, f_middle,  &edgeinfo_middle,  &bcr_middle);
         value_coeff_k_bottom  = GMT_get_bcr_z (&grd_k_bottom,  x_loc, y_loc, f_k_bottom,  &edgeinfo_k_bottom,  &bcr_k_bottom);
         value_coeff_bottom  = GMT_get_bcr_z (&grd_bottom,  x_loc, y_loc, f_bottom,  &edgeinfo_bottom,  &bcr_bottom);

         if (GMT_is_dnan (value_coeff_wb_twt) || GMT_is_dnan (value_coeff_k_top)) {
            for ( n=0; n < nz; ++n )  tr.data[n] = 0;
            tr.trid = 0;
         } else {
	    if ( verbose == 2 ) {
               fprintf ( stderr, "Trace num = %d, X-Loc = %f, Y-Loc = %f, WB_TWT = %0.10f, K_Top = %0.10f\n", k+1, x_loc, y_loc, value_coeff_wb_twt, value_coeff_k_top );
	       fprintf ( stderr, "Top_TWT = %12.4f, Middle_TWT = %12.4f, Bottom_TWT = %12.4f, K_middle = %0.10f, K_bottom = %0.10f\n", 
               value_coeff_top, value_coeff_middle, value_coeff_bottom, value_coeff_k_middle, value_coeff_k_bottom );
            }

            if ( ! ( GMT_is_dnan (value_coeff_top) ) && ! ( GMT_is_dnan (value_coeff_k_top)) ) {
               ++kount;
               K_values[kount] = value_coeff_k_top;   
               TWT_values[kount] = value_coeff_top;
            }
            if ( ! ( GMT_is_dnan (value_coeff_middle) ) && ! ( GMT_is_dnan (value_coeff_k_middle)) ) {
               ++kount;
               K_values[kount] = value_coeff_k_middle;   
               TWT_values[kount] = value_coeff_middle;
            }
            if ( ! ( GMT_is_dnan (value_coeff_bottom) ) && ! ( GMT_is_dnan (value_coeff_k_bottom)) ) {
               ++kount;
               K_values[kount] = value_coeff_k_bottom;   
               TWT_values[kount] = value_coeff_bottom;
            }

            if ( kount > -1 && verbose == 2 ) {
               water_depth = value_coeff_wb_twt * factor1 * vwater;
               ratio = vwater / value_coeff_k_top;
               for ( n=0; n <= kount; ++n ) {
	          tr_msec = ( TWT_values[n] - value_coeff_wb_twt ) * factor1;
                  Depth_array[n] = ( ratio * (exp (K_values[n]*tr_msec) - 1.0) ) + water_depth;
                  fprintf ( stderr, "Sample = %5d, Water depth = %f, ratio = %f, tr_msec = %f, depth = %f, K = %f\n", n, water_depth, ratio, tr_msec, Depth_array[n], K_values[n] );
                  if ( n == 0 ) {
                     Vavg_array[0] = Depth_array[0] / ( ( TWT_values[0] - value_coeff_wb_twt ) * factor1 );
                  } else if ( n > 0 && ( Depth_array[n] > Depth_array[n-1] ) ) {
                     Vavg_array[n] = ( Depth_array[n] - Depth_array[n-1] ) / ( ( ( TWT_values[n] - value_coeff_wb_twt ) * factor1 ) - ( ( TWT_values[n-1] - value_coeff_wb_twt ) * factor1 ) );
                  } else {
                     Depth_array[n] = Depth_array[n-1];
                     Vavg_array[n] = Vavg_array[n-1];
                  }
                  fprintf ( stderr, "Sample = %5d, TWT value = %.5f, K value = %.6f, Depth = %.4f, Average_Velocity = %.2f\n", n, TWT_values[n], K_values[n], Depth_array[n], Vavg_array[n] );
               }
            }

            if ( kount >= 0 ) {
	       for ( n=0; n < ns; ++n ) {
	          tr_msec = n * dt_msec;
      	          dintlin ( kount+1, TWT_values, K_values, K_values[0], K_values[kount], 1, &tr_msec, &K );
	          K_array[n] = K; 
               }

               water_depth = value_coeff_wb_twt * factor1 * vwater;
               ratio = vwater / value_coeff_k_top;
	       for ( n=0; n < ns; ++n ) {
                  tr_amp[n] = tr.data[n];
	          tr_msec   = n * dt_msec;
                  if ( tr_msec <= value_coeff_wb_twt ) {
                     depth[n] = tr_msec * factor1 * vwater;
                  } else {
	             tr_msec = ( tr_msec - value_coeff_wb_twt ) * factor1;
                     depth[n] = ( ratio * (exp (K_array[n]*tr_msec) - 1.0) ) + water_depth;
                  }
                  /* if ( verbose == 2 ) fprintf ( stderr, "trace = %6d, sample = %6d, One-way time(sec) = %10.4f, K = %12.6f, Depth value = %10.2f\n", k, n, tr_msec, K_array[n], depth[n] ); */
               }

               if ( verbose == 3 )  {
                  fprintf ( stderr, "Input trace = %6d, Xloc = %8.2f Yloc = %8.2f, TWT_Water = %8.2f, K = %10.4f One-way time(sec) = %10.4f, Depth sample = %6d Depth value = %10.2f\n", 
                  k, x_loc, y_loc, value_coeff_wb_twt, K, tr_msec, n, depth[n] ); 
	       }
	    }
	    for ( n=0; n < nz; ++n ) {
	       depth_input = n * dz;
      	       intlin ( ns, depth, tr_amp, tr_amp[0], tr_amp[ns-1], 1, &depth_input, &amp_output );
	       tr.data[n] = amp_output; 
            }
            tr.trid = 1;
         }

	 tr.ns     = nz;
	 tr.dt     = nint(dz*1000);
	 puttr (&tr); 
      } else {
         fprintf ( stderr, "Input trace = %d, Xloc = %.0f Yloc = %.0f is out of bounds\n", k, x_loc, y_loc);
      }
   }

   GMT_free ((void *)f_wb_twt);
   GMT_free ((void *)f_k_top);
   GMT_free ((void *)f_top);
   GMT_free ((void *)f_k_middle);
   GMT_free ((void *)f_middle);
   GMT_free ((void *)f_k_bottom);

   free1double (K_values);
   free1double (TWT_values);
   free1double (K_array);
   free1double (Vavg_array);
   free1double (Depth_array);
   free1float  (depth);
   free1float  (tr_amp);

   GMT_end (argc, argv);

   return (0);
}
