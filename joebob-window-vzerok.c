#include"gmt.h"
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
float  *f1, *f2;

int main (int argc, char **argv) {

   char *coeff_x, *coeff_x2, file[BUFSIZ];
	
   struct GRD_HEADER grd_x, grd_x2;
   struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2;
   struct GMT_BCR bcr_x, bcr_x2;

   short  verbose;
   int    scalar, ntr, ns;
   double x_loc, y_loc;
   double value_coeff_x, value_coeff_x2;
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

   if (!getparshort("verbose" , &verbose)) verbose = 0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "X1 Coefficient GMT grid file name = %s\n", coeff_x );
      fprintf ( stderr, "X2 Coefficient GMT grid file name = %s\n", coeff_x2 );
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

   /* Get info from first trace */
   ntr     = gettra (&tr, 0);
   ns      = tr.ns;
   scalar  = abs ( tr.scalel );
   if ( scalar == 0 ) scalar = 1;

   if ( verbose ) {
      fprintf ( stderr, "Number of samples per trace = %d\n", ns );
      fprintf ( stderr, "Number of traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "Location scalar = %d\n", scalar );
      fprintf ( stderr, "\n" );
   }

   rewind (stdin);

   /* Main loop over traces */
   for ( k = 0; k < ntr; ++k ) {
      gettr (&tr);
      x_loc = tr.sx / scalar;
      y_loc = tr.sy / scalar;

      if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) {
         value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
         value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);

         if (GMT_is_dnan (value_coeff_x) || GMT_is_dnan (value_coeff_x2)) {
            for ( n=0; n < ns; ++n )  tr.data[n] = 0;
            tr.trid = 0;
         } else {
	    if ( verbose == 2 ) fprintf ( stderr, "Trace num = %d, X-Loc = %f, Y-Loc = %f, X Coefficient = %0.10f, X2 Coefficient = %0.10f\n", k+1, x_loc, y_loc, value_coeff_x, value_coeff_x2 );
	    for ( n=0; n < ns; ++n ) if ( n < value_coeff_x || n > value_coeff_x2 ) tr.data[n] = 0.0;
            tr.trid = 1;
         }
	 puttr (&tr);
      } else {
         fprintf ( stderr, "input trace = %d, xloc = %.0f yloc = %.0f is out of bounds\n", k, x_loc, y_loc);
      }
   }

   GMT_free ((void *)f1);
   GMT_free ((void *)f2);

   GMT_end (argc, argv);

   return (0);
}
