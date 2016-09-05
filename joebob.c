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

segy tr, dtr;
float  *f1, *f2, *f3;

int main (int argc, char **argv) {

   char *coeff_x, *coeff_x2, *coeff_x3, file[BUFSIZ];
   cwp_Bool active = TRUE;
	
   struct GRD_HEADER grd_x, grd_x2, grd_x3;
   struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2, edgeinfo_x3;
   struct GMT_BCR bcr_x, bcr_x2, bcr_x3;

   short  check, verbose;
   int    nz, ntr, ns;
   double value, scale_factor, dz, x_loc, y_loc;
   double weight_x, weight_x2, weight_x3;
   double value_coeff_x, value_coeff_x2, value_coeff_x3, tr_sec, dt_sec;
   float  depth_input, amp_output, *tr_amp, *depth;
   register int k, n;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring("coeff_x", &coeff_x)) {
      fprintf ( stderr, "Must supply Coefficient_X GMT grid (COEFF_X Parameter) --> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getparstring("coeff_x2", &coeff_x2)) {
      fprintf ( stderr, "Must supply Coefficient_X2 GMT grid (COEFF_X2 Parameter)--> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getparstring("coeff_x3", &coeff_x3)) {
      fprintf ( stderr, "Must supply Coefficient_X3 GMT grid (COEFF_X3 Parameter)--> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getparshort("verbose" , &verbose)) verbose = 0;
   if (!getpardouble("weight_x", &weight_x)) weight_x  = 1.0;
   if (!getpardouble("weight_x2", &weight_x2)) weight_x2  = 1.0;
   if (!getpardouble("weight_x3", &weight_x3)) weight_x3  = 1.0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "X1 Coefficient GMT grid file name = %s\n", coeff_x );
      fprintf ( stderr, "X2 Coefficient GMT grid file name = %s\n", coeff_x2 );
      fprintf ( stderr, "X3 Coefficient GMT grid file name = %s\n", coeff_x3 );
      fprintf ( stderr, "X1 Grid Weighting Value = %f\n", weight_x );
      fprintf ( stderr, "X2 Grid Weighting Value = %f\n", weight_x2 );
      fprintf ( stderr, "X3 Grid Weighting Value = %f\n", weight_x3 );
      fprintf ( stderr, "\n" );
   }

   weight_x  = 1.0 / weight_x;
   weight_x2 = 1.0 / weight_x2;
   weight_x3 = 1.0 / weight_x3;

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

   value = 0.0;
   GMT_bcr_init (&grd_x,  GMT_pad, active, value, &bcr_x);
   GMT_bcr_init (&grd_x2, GMT_pad, active, value, &bcr_x2);
   GMT_bcr_init (&grd_x3, GMT_pad, active, value, &bcr_x3);

   GMT_read_grd (coeff_x,  &grd_x,  f1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x2, &grd_x2, f2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x3, &grd_x3, f3, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   /* Get info from first trace */
   ntr     = gettra (&tr, 0);
   ns      = tr.ns;
   dt_sec  = tr.dt * 0.000001;
   scale_factor = tr.scalco;
   if (scale_factor < 0.0 ) scale_factor *= -1.0;
   if (scale_factor == 0.0 ) scale_factor = 1.0;

   if (!getpardouble ("dz",&dz)) dz = 2.0;
   if (!getparint    ("nz",&nz)) nz = ns;

   if ( verbose ) {
      fprintf ( stderr, "Output depth sample rate = %f\n", dz );
      fprintf ( stderr, "Coordinate scale factor = %f\n", scale_factor );
      fprintf ( stderr, "Number of output depth samples per trace = %d\n", nz );
      fprintf ( stderr, "number of traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "time sample rate (seconds) = %f\n", dt_sec );
   }

   rewind (stdin);

   depth  = ealloc1float ( ns );
   tr_amp = ealloc1float ( nz );

   /* Main loop over traces */
   for ( k = 0; k < ntr; ++k ) {
      gettr (&tr);
      x_loc = tr.sx / scale_factor;
      y_loc = tr.sy / scale_factor;

      check = 0;
      if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) check = 1;

      if ( check ) {
         value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
         value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);
         value_coeff_x3 = GMT_get_bcr_z (&grd_x3, x_loc, y_loc, f3, &edgeinfo_x3, &bcr_x3);

	 if ( verbose ) fprintf ( stderr, "Trace num = %d, X-Loc = %f, Y-Loc = %f, X Coefficient = %0.10f, X2 Coefficient = %0.10f, X3 Coefficient = %0.10f\n", 
         k+1, x_loc, y_loc, value_coeff_x, value_coeff_x2, value_coeff_x3 );

	 for ( n=0; n < ns; ++n ) {
            tr_amp[n] = tr.data[n];
	    tr_sec   = n * dt_sec;
	    depth[n] = (((value_coeff_x*tr_sec)*weight_x) + ((value_coeff_x2*pow(tr_sec,2))*weight_x2) + ((value_coeff_x3*pow(tr_sec,3))*weight_x3)) * -1.0;
            if ( verbose == 2 ) fprintf ( stderr, "Trace no. = %5d, Sample = %5d, TWT (secs.) = %.4f, Depth (feet) = %.4f\n", k, n, tr_sec, depth[n] ); 
	 }
	 for ( n=0; n < nz; ++n ) {
	    depth_input = n * dz;
	    intlin ( ns, depth, tr_amp, tr_amp[0], tr_amp[ns-1], 1, &depth_input, &amp_output );
	    dtr.data[n] = amp_output; 
         }
	 dtr.tracl  = tr.tracl;
	 dtr.tracr  = tr.tracr;
	 dtr.ep     = tr.ep;
	 dtr.ns     = nz;
	 dtr.dt     = nint (dz * 1000.0);
	 dtr.sx     = tr.sx;
	 dtr.sy     = tr.sy;
	 dtr.trid   = 1;
	 dtr.fldr   = tr.fldr;
	 dtr.cdp    = tr.cdp ;
	 puttr (&dtr);
      } else {
         fprintf ( stderr, "input trace = %d, xloc = %.0f yloc = %.0f is out of bounds\n", k, x_loc, y_loc);
      }
   }

   GMT_free ((void *)f1);
   GMT_free ((void *)f2);
   GMT_free ((void *)f3);
   GMT_end  (argc, argv);

   free1float (depth);
   free1float (tr_amp);

   return (0);
}
