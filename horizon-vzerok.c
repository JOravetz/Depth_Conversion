#include "gmt.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"

char *sdoc[] = {NULL};

float  *f1, *f2;

int main (int argc, char **argv) {

   char temp[256];
   char *coeff_x, *coeff_x2, file[BUFSIZ];
	
   struct GRD_HEADER grd_x, grd_x2;
   struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2;
   struct GMT_BCR bcr_x, bcr_x2;

   short  verbose;
   double water_depth, vzero, factor, x_loc, y_loc, depth;
   double value_coeff_x, value_coeff_x2, twt_horizon, twt;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring("coeff_x", &coeff_x)) coeff_x = "wb.twt.grd"; 

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

   if (!getpardouble ("vzero",&vzero)) vzero = 1452.05;

   if ( verbose ) {
      fprintf ( stderr, "Vzero = %f (meters/second)\n", vzero );
      fprintf ( stderr, "\n" );
   }

   factor = 0.0005;

   while (((char *) NULL) != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf", &x_loc, &y_loc, &twt_horizon );

      if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) {
         value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
         value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);

         if ( ! (GMT_is_dnan (value_coeff_x) || GMT_is_dnan (value_coeff_x2)) ) {
	    if ( verbose == 2 ) fprintf ( stderr, "X-Loc = %f, Y-Loc = %f, X Coefficient = %0.10f, X2 Coefficient = %0.10f\n", x_loc, y_loc, value_coeff_x, value_coeff_x2 );

            if ( twt_horizon <= value_coeff_x ) {
               depth = value_coeff_x * factor * vzero;
            } else {
               twt = ( twt_horizon - value_coeff_x ) * factor;
               water_depth = value_coeff_x * factor * vzero;
               depth = ( ( vzero / value_coeff_x2 ) * (exp (value_coeff_x2*twt) - 1.0) ) + water_depth;
            }

            if ( verbose == 2) fprintf ( stderr, "xloc = %8.2f, yloc = %8.2f, vzero = %8.2f, k = %10.6f, horizon time(msec) = %10.4f, depth value = %10.2f\n", x_loc, y_loc, value_coeff_x, value_coeff_x2, twt_horizon, depth ); 
            printf ( "%-12.2f %12.2f %12.4f\n", x_loc, y_loc, depth );
         }
      } else {
         fprintf ( stderr, "xloc = %12.2f yloc = %12.2f is out of bounds\n", x_loc, y_loc);
      }
   }

   GMT_free ((void *)f1);
   GMT_free ((void *)f2);

   GMT_end (argc, argv);

   return (0);
}
