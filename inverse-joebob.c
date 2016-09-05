#include "gmt.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"
#include "header.h"
#include "nr.h"
#include "nrutil.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

#define M 3
#define MP1 (M+1)

char *sdoc[] = {NULL};

segy tr, dtr;
float  *grid1, *grid2, *grid3;

int main (int argc, char **argv) {

   char *coeff_x, *coeff_x2, *coeff_x3, file[BUFSIZ];
   cwp_Bool active = TRUE;
	
   struct GRD_HEADER grd_x, grd_x2, grd_x3;
   struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2, edgeinfo_x3;
   struct GMT_BCR bcr_x, bcr_x2, bcr_x3;

   short  check, verbose;
   int    nz, nt, ntr;
   double units, dz, dt, value, x_loc, y_loc;
   double value_coeff_x, value_coeff_x2, value_coeff_x3;
   float  twt, depth_input, amp_output, *tr_amp, *depth, *aral, *rti, *rtr;
   register int i, k, n;

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

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "X1 Coefficient GMT grid file name = %s\n", coeff_x );
      fprintf ( stderr, "X2 Coefficient GMT grid file name = %s\n", coeff_x2 );
      fprintf ( stderr, "X3 Coefficient GMT grid file name = %s\n", coeff_x3 );
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
		
   grid1 = (float *) GMT_memory (VNULL, (size_t)((grd_x.nx  + 4) * (grd_x.ny  + 4)), sizeof(float), GMT_program);
   grid2 = (float *) GMT_memory (VNULL, (size_t)((grd_x2.nx + 4) * (grd_x2.ny + 4)), sizeof(float), GMT_program);
   grid3 = (float *) GMT_memory (VNULL, (size_t)((grd_x3.nx + 4) * (grd_x3.ny + 4)), sizeof(float), GMT_program);

   GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

   GMT_boundcond_param_prep (&grd_x,  &edgeinfo_x);
   GMT_boundcond_param_prep (&grd_x2, &edgeinfo_x2);
   GMT_boundcond_param_prep (&grd_x3, &edgeinfo_x3);

   GMT_boundcond_set (&grd_x,  &edgeinfo_x,  GMT_pad, grid1);
   GMT_boundcond_set (&grd_x2, &edgeinfo_x2, GMT_pad, grid2);
   GMT_boundcond_set (&grd_x3, &edgeinfo_x3, GMT_pad, grid3);

   value = 0.0;
   GMT_bcr_init (&grd_x,  GMT_pad, active, value, &bcr_x);
   GMT_bcr_init (&grd_x2, GMT_pad, active, value, &bcr_x2);
   GMT_bcr_init (&grd_x3, GMT_pad, active, value, &bcr_x3);

   GMT_read_grd (coeff_x,  &grd_x,  grid1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x2, &grd_x2, grid2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x3, &grd_x3, grid3, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   if (!getpardouble ("dt",&dt)) dt = 0.001;
   if (!getpardouble ("units",&units)) units = 3.2808399;
   if (!getparint    ("nt",&nt)) nt = nz;

   /* Get info from first trace */
   ntr = gettra (&tr, 0);
   nz  = tr.ns;
   dz  = ( tr.dt * 0.001) * units;

   if ( verbose ) {
      fprintf ( stderr, "Number of input traces (ntr) = %d\n", ntr );
      fprintf ( stderr, "Input depth sample rate (dz) = %f\n", dz );
      fprintf ( stderr, "Units conversion factor (units) = %f\n", units );
      fprintf ( stderr, "Number of input depth samples per trace (nz) = %d\n", nz );
      fprintf ( stderr, "Output TWT sample rate (dt, seconds) = %f\n", dt );
      fprintf ( stderr, "Number of output TWT samples per trace (nt) = %d\n", nt );
   }

   rewind (stdin);

   depth  = ealloc1float ( nz );
   tr_amp = ealloc1float ( nz );

   aral = ealloc1float(MP1);
   rti = vector(1,M);
   rtr = vector(1,M);

   /* Main loop over traces */
   for ( k = 0; k < ntr; ++k ) {
      gettr (&tr);
      x_loc = tr.sx;
      y_loc = tr.sy;

      check = 0;
      if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) check = 1;

      if ( check ) {
         value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, grid1, &edgeinfo_x,  &bcr_x);
         value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, grid2, &edgeinfo_x2, &bcr_x2);
         value_coeff_x3 = GMT_get_bcr_z (&grd_x3, x_loc, y_loc, grid3, &edgeinfo_x3, &bcr_x3);

         aral[1] = (float) value_coeff_x  * -1.0;
         aral[2] = (float) value_coeff_x2 * -1.0;
         aral[3] = (float) value_coeff_x3 * -1.0;

	 if ( verbose ) fprintf ( stderr, "Trace num = %6d, X-Loc = %.2f, Y-Loc = %.2f, X Coefficient = %16.10f, X2 Coefficient = %16.10f, X3 Coefficient = %16.10f\n", 
         k+1, x_loc, y_loc, value_coeff_x, value_coeff_x2, value_coeff_x3 );

	 for ( n=0; n < nz; ++n ) {
            tr_amp[n] = tr.data[n];
	    aral[0] = (n * dz) * -1.0;
            zrhqr ( aral, M, rtr, rti );
            for (i=1;i<=M;i++) {
               if ( rti[i] == 0.0 ) {
                  twt = rtr[i];
                  break;
               }
            }
            depth[n] = twt;
            if ( verbose == 2 ) fprintf ( stderr, "Trace = %6d, Sample = %6d, Input Depth = %10.4f, Output TWT = %10.6f\n", k+1, n+1, -aral[0], twt );
         }

         for ( n=0; n < nt; ++n ) {
	    depth_input = n * dt;
	    intlin ( nz, depth, tr_amp, tr_amp[0], tr_amp[nz-1], 1, &depth_input, &amp_output );
	    dtr.data[n] = amp_output; 
         }

	 dtr.tracl  = tr.tracl;
	 dtr.tracr  = tr.tracr;
	 dtr.ep     = tr.ep;
	 dtr.ns     = nt;
	 dtr.dt     = nint (dt * 1000000.0);
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

   GMT_free ((void *)grid1);
   GMT_free ((void *)grid2);
   GMT_free ((void *)grid3);
   GMT_end  (argc, argv);

   free1float (depth);
   free1float (tr_amp);
   free1float (aral);
   free1float (rtr);
   free1float (rti);

   return (0);
}
