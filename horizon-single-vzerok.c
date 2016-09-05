#include "gmt.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"

#define abs(x)   ( (x) <  0  ? -(x) : (x) )

char *sdoc[] = {NULL};

float  *f1;

int main (int argc, char **argv) {

   char temp[256];
   char *coeff_x, file[BUFSIZ];
	
   struct GRD_HEADER grd_x;
   struct GMT_EDGEINFO edgeinfo_x;
   struct GMT_BCR bcr_x;

   short  verbose;
   double dk, deltaz, water_depth, vzero, factor, x_loc, y_loc, depth, value_coeff_x, twt_horizon, twt, K, Z;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring("coeff_x", &coeff_x)) {
      fprintf ( stderr, "Must supply Coefficient_X GMT grid (COEFF_X Parameter) --> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getpardouble("K", &K)) {
      fprintf ( stderr, "Must supply K Coefficient Parameter (K) --> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getpardouble("Z", &Z)) {
      fprintf ( stderr, "Must supply Well-Marker Depth Parameter (Z) --> exiting\n" );
      return EXIT_FAILURE;
   }

   if (!getparshort("verbose" , &verbose)) verbose = 0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "X1 Coefficient GMT grid file name = %s\n", coeff_x );
      fprintf ( stderr, "K Coefficient = %f\n", K );
      fprintf ( stderr, "Well-Marker Pick Depth (tvdss) = %f\n", Z );
      fprintf ( stderr, "\n" );
   }

   GMT_boundcond_init (&edgeinfo_x);

   GMT_grd_init (&grd_x,  argc, argv, FALSE);
   if (GMT_read_grd_info (coeff_x,  &grd_x))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
		
   f1 = (float *) GMT_memory (VNULL, (size_t)((grd_x.nx  + 4) * (grd_x.ny  + 4)), sizeof(float), GMT_program);

   GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;
   GMT_boundcond_param_prep (&grd_x,  &edgeinfo_x);
   GMT_boundcond_set (&grd_x, &edgeinfo_x, GMT_pad, f1);
   GMT_bcr_init (&grd_x,  GMT_pad, BCR_BSPLINE, 1, &bcr_x);
   GMT_read_grd (coeff_x,  &grd_x,  f1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   if (!getpardouble ("vzero",&vzero)) vzero = 1452.50;

   if ( verbose ) {
      fprintf ( stderr, "Vzero = %f (meters/second)\n", vzero );
      fprintf ( stderr, "\n" );
   }

   factor = 0.0005;
   dk = 0.00000001;

   while (((char *) NULL) != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf", &x_loc, &y_loc, &twt_horizon );

      if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) {
         value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);

         if ( ! (GMT_is_dnan (value_coeff_x)) ) {
	    if ( verbose == 2 ) fprintf ( stderr, "X-Loc = %f, Y-Loc = %f, X Coefficient = %0.10f, K Coefficient = %0.10f\n", x_loc, y_loc, value_coeff_x, K );

            if ( twt_horizon <= value_coeff_x ) {
               depth = value_coeff_x * factor * vzero;
            } else {
               twt = ( twt_horizon - value_coeff_x ) * factor;
               water_depth = value_coeff_x * factor * vzero;
               depth = ( ( vzero / K ) * (exp (K*twt) - 1.0) ) + water_depth;
               deltaz = depth - Z;
               do {
                  if ( depth > Z ) {
                     K -= dk;
                  } else {
                     K += dk;
                  }
                  depth = ( ( vzero / K ) * (exp (K*twt) - 1.0) ) + water_depth;
                  deltaz = depth - Z;
                  if ( verbose == 3 ) fprintf ( stderr, "xloc = %8.2f, yloc = %8.2f, TWT_water = %8.2f, K = %10.6f, horizon time(msec) = %10.4f, depth value = %10.2f\n", x_loc, y_loc, value_coeff_x, K, twt_horizon, depth ); 
               } while ( abs(deltaz) > 0.00001 );
            }

            if ( verbose == 2 ) fprintf ( stderr, "xloc = %8.2f, yloc = %8.2f, TWT_water = %8.2f, K = %10.6f, horizon time(msec) = %10.4f, depth value = %10.2f\n", x_loc, y_loc, value_coeff_x, K, twt_horizon, depth ); 
            printf ( "%-12.2f %12.2f %12.8f\n", x_loc, y_loc, K );
         }
      } else {
         fprintf ( stderr, "xloc = %12.2f yloc = %12.2f is out of bounds\n", x_loc, y_loc);
      }
   }

   GMT_free ((void *)f1);

   GMT_end (argc, argv);

   return (0);
}
