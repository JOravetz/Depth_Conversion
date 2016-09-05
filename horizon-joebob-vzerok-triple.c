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

float *f_top_k,  *f_middle_k, *f_bottom_k;
float *f_wb_twt, *f_top_twt,  *f_middle_twt, *f_bottom_twt;
float *f_top_z,  *f_middle_z, *f_bottom_z;

int main (int argc, char **argv) {

   char temp[256], file[BUFSIZ];
   char *coeff_top_k, *coeff_middle_k, *coeff_bottom_k;
   char *coeff_wb_twt, *coeff_top_twt, *coeff_middle_twt, *coeff_bottom_twt;
   char *coeff_top_z, *coeff_middle_z, *coeff_bottom_z;

   struct GRD_HEADER grd_top_k, grd_middle_k, grd_bottom_k;
   struct GRD_HEADER grd_wb_twt, grd_top_twt, grd_middle_twt, grd_bottom_twt;
   struct GRD_HEADER grd_top_z, grd_middle_z, grd_bottom_z;

   struct GMT_EDGEINFO edgeinfo_top_k, edgeinfo_middle_k, edgeinfo_bottom_k;
   struct GMT_EDGEINFO edgeinfo_wb_twt, edgeinfo_top_twt, edgeinfo_middle_twt, edgeinfo_bottom_twt;
   struct GMT_EDGEINFO edgeinfo_top_z, edgeinfo_middle_z, edgeinfo_bottom_z;

   struct GMT_BCR bcr_top_k, bcr_middle_k, bcr_bottom_k;
   struct GMT_BCR bcr_wb_twt, bcr_top_twt, bcr_middle_twt, bcr_bottom_twt;
   struct GMT_BCR bcr_top_z, bcr_middle_z, bcr_bottom_z;

   double value_coeff_top_k, value_coeff_middle_k, value_coeff_bottom_k;
   double value_coeff_wb_twt, value_coeff_top_twt, value_coeff_middle_twt, value_coeff_bottom_twt;
   double value_coeff_top_z, value_coeff_middle_z, value_coeff_bottom_z;

   short  verbose;
   double water_depth, vwater, ratio, factor1, x_loc, y_loc; 
   double delta_twt, delta_k, gradient, K;
   double delta_twt_middle_top, delta_k_middle_top, gradient_middle_top;
   double delta_twt_bottom_middle, delta_k_bottom_middle, gradient_bottom_middle;
   double depth, twt_horizon, twt;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring ("coeff_top_k",       &coeff_top_k))       coeff_top_k       = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/below.ml.K.grd";
   if (!getparstring ("coeff_middle_k",    &coeff_middle_k))    coeff_middle_k    = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/middle.K.grd";
   if (!getparstring ("coeff_bottom_k",    &coeff_bottom_k))    coeff_bottom_k    = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/bottom.K.grd";
   if (!getparstring ("coeff_wb_twt",      &coeff_wb_twt))      coeff_wb_twt      = "/home/user/FIELD.new/PICKS/wb.twt.grd";
   if (!getparstring ("coeff_top_twt",     &coeff_top_twt))     coeff_top_twt     = "/home/user/FIELD.new/PICKS/TWT_GRIDS/01_FIELD_Top_Reservoir_twt.reform.dat.trimmed.grd";
   if (!getparstring ("coeff_middle_twt",  &coeff_middle_twt))  coeff_middle_twt  = "/home/user/FIELD.new/PICKS/TWT_GRIDS/06_FIELD_C_top_twt.reform.dat.trimmed.grd";
   if (!getparstring ("coeff_bottom_twt",  &coeff_bottom_twt))  coeff_bottom_twt  = "/home/user/FIELD.new/PICKS/TWT_GRIDS/10_FIELD_80MaSB_twt.reform.dat.trimmed.grd";
   if (!getparstring ("coeff_top_z",       &coeff_top_z))       coeff_top_z       = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/01_FIELD_Top_Reservoir.depth.new.grd";
   if (!getparstring ("coeff_middle_z",    &coeff_middle_z))    coeff_middle_z    = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/06_FIELD_C_top.depth.new.grd";
   if (!getparstring ("coeff_bottom_z",    &coeff_bottom_z))    coeff_bottom_z    = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/10_FIELD_80MaSB.depth.new.grd";

   if (!getparshort ("verbose", &verbose)) verbose = 0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Top_K       - GMT grid file name = %s\n", coeff_top_k );
      fprintf ( stderr, "Middle_K    - GMT grid file name = %s\n", coeff_middle_k );
      fprintf ( stderr, "Bottom_K    - GMT grid file name = %s\n", coeff_bottom_k );
      fprintf ( stderr, "WB_TWT      - GMT grid file name = %s\n", coeff_wb_twt );
      fprintf ( stderr, "TOP_TWT     - GMT grid file name = %s\n", coeff_top_twt );
      fprintf ( stderr, "MIDDLE_TWT  - GMT grid file name = %s\n", coeff_middle_twt );
      fprintf ( stderr, "BOTTOM_TWT  - GMT grid file name = %s\n", coeff_bottom_twt );
      fprintf ( stderr, "TOP_Z       - GMT grid file name = %s\n", coeff_top_z );
      fprintf ( stderr, "MIDDLE_Z    - GMT grid file name = %s\n", coeff_middle_z );
      fprintf ( stderr, "BOTTOM_Z    - GMT grid file name = %s\n", coeff_bottom_z );
      fprintf ( stderr, "\n" );
   }

   GMT_boundcond_init (&edgeinfo_top_k);
   GMT_boundcond_init (&edgeinfo_middle_k);
   GMT_boundcond_init (&edgeinfo_bottom_k);
   GMT_boundcond_init (&edgeinfo_wb_twt);
   GMT_boundcond_init (&edgeinfo_top_twt);
   GMT_boundcond_init (&edgeinfo_middle_twt);
   GMT_boundcond_init (&edgeinfo_bottom_twt);
   GMT_boundcond_init (&edgeinfo_top_z);
   GMT_boundcond_init (&edgeinfo_middle_z);
   GMT_boundcond_init (&edgeinfo_bottom_z);

   GMT_grd_init (&grd_top_k,       argc, argv, FALSE);
   GMT_grd_init (&grd_middle_k,    argc, argv, FALSE);
   GMT_grd_init (&grd_bottom_k,    argc, argv, FALSE);
   GMT_grd_init (&grd_wb_twt,      argc, argv, FALSE);
   GMT_grd_init (&grd_top_twt,     argc, argv, FALSE);
   GMT_grd_init (&grd_middle_twt,  argc, argv, FALSE);
   GMT_grd_init (&grd_bottom_twt,  argc, argv, FALSE);
   GMT_grd_init (&grd_top_z,       argc, argv, FALSE);
   GMT_grd_init (&grd_middle_z,    argc, argv, FALSE);
   GMT_grd_init (&grd_bottom_z,    argc, argv, FALSE);

   if (GMT_read_grd_info (coeff_top_k, &grd_top_k))             fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_middle_k, &grd_middle_k))       fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_bottom_k, &grd_bottom_k))       fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_wb_twt, &grd_wb_twt))           fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_top_twt, &grd_top_twt))         fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_middle_twt, &grd_middle_twt))   fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_bottom_twt, &grd_bottom_twt))   fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_top_z, &grd_top_z))             fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_middle_z, &grd_middle_z))       fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_bottom_z, &grd_bottom_z))       fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);

   f_top_k       = (float *) GMT_memory (VNULL, (size_t)((grd_top_k.nx  + 4)       * (grd_top_k.ny  + 4)),       sizeof(float), GMT_program);
   f_middle_k    = (float *) GMT_memory (VNULL, (size_t)((grd_middle_k.nx  + 4)    * (grd_middle_k.ny  + 4)),    sizeof(float), GMT_program);
   f_bottom_k    = (float *) GMT_memory (VNULL, (size_t)((grd_bottom_k.nx  + 4)    * (grd_bottom_k.ny  + 4)),    sizeof(float), GMT_program);
   f_wb_twt      = (float *) GMT_memory (VNULL, (size_t)((grd_wb_twt.nx  + 4)      * (grd_wb_twt.ny  + 4)),      sizeof(float), GMT_program);
   f_top_twt     = (float *) GMT_memory (VNULL, (size_t)((grd_top_twt.nx  + 4)     * (grd_top_twt.ny  + 4)),     sizeof(float), GMT_program);
   f_middle_twt  = (float *) GMT_memory (VNULL, (size_t)((grd_middle_twt.nx  + 4)  * (grd_middle_twt.ny  + 4)),  sizeof(float), GMT_program);
   f_bottom_twt  = (float *) GMT_memory (VNULL, (size_t)((grd_bottom_twt.nx  + 4)  * (grd_bottom_twt.ny  + 4)),  sizeof(float), GMT_program);
   f_top_z       = (float *) GMT_memory (VNULL, (size_t)((grd_top_z.nx  + 4)       * (grd_top_z.ny  + 4)),       sizeof(float), GMT_program);
   f_middle_z    = (float *) GMT_memory (VNULL, (size_t)((grd_middle_z.nx  + 4)    * (grd_middle_z.ny  + 4)),    sizeof(float), GMT_program);
   f_bottom_z    = (float *) GMT_memory (VNULL, (size_t)((grd_bottom_z.nx  + 4)    * (grd_bottom_z.ny  + 4)),    sizeof(float), GMT_program);

   GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

   GMT_boundcond_param_prep (&grd_top_k,       &edgeinfo_top_k);
   GMT_boundcond_param_prep (&grd_middle_k,    &edgeinfo_middle_k);
   GMT_boundcond_param_prep (&grd_bottom_k,    &edgeinfo_bottom_k);
   GMT_boundcond_param_prep (&grd_wb_twt,      &edgeinfo_wb_twt);
   GMT_boundcond_param_prep (&grd_top_twt,     &edgeinfo_top_twt);
   GMT_boundcond_param_prep (&grd_middle_twt,  &edgeinfo_middle_twt);
   GMT_boundcond_param_prep (&grd_bottom_twt,  &edgeinfo_bottom_twt);
   GMT_boundcond_param_prep (&grd_top_z,       &edgeinfo_top_z);
   GMT_boundcond_param_prep (&grd_middle_z,    &edgeinfo_middle_z);
   GMT_boundcond_param_prep (&grd_bottom_z,    &edgeinfo_bottom_z);

   GMT_boundcond_set (&grd_top_k,       &edgeinfo_top_k,       GMT_pad, f_top_k);
   GMT_boundcond_set (&grd_middle_k,    &edgeinfo_middle_k,    GMT_pad, f_middle_k);
   GMT_boundcond_set (&grd_bottom_k,    &edgeinfo_bottom_k,    GMT_pad, f_bottom_k);
   GMT_boundcond_set (&grd_wb_twt,      &edgeinfo_wb_twt,      GMT_pad, f_wb_twt);
   GMT_boundcond_set (&grd_top_twt,     &edgeinfo_top_twt,     GMT_pad, f_top_twt);
   GMT_boundcond_set (&grd_middle_twt,  &edgeinfo_middle_twt,  GMT_pad, f_middle_twt);
   GMT_boundcond_set (&grd_bottom_twt,  &edgeinfo_bottom_twt,  GMT_pad, f_bottom_twt);
   GMT_boundcond_set (&grd_top_z,       &edgeinfo_top_z,       GMT_pad, f_top_z);
   GMT_boundcond_set (&grd_middle_z,    &edgeinfo_middle_z,    GMT_pad, f_middle_z);
   GMT_boundcond_set (&grd_bottom_z,    &edgeinfo_bottom_z,    GMT_pad, f_bottom_z);

   GMT_bcr_init (&grd_top_k,       GMT_pad, BCR_BSPLINE, 1, &bcr_top_k);
   GMT_bcr_init (&grd_middle_k,    GMT_pad, BCR_BSPLINE, 1, &bcr_middle_k);
   GMT_bcr_init (&grd_bottom_k,    GMT_pad, BCR_BSPLINE, 1, &bcr_bottom_k);
   GMT_bcr_init (&grd_wb_twt,      GMT_pad, BCR_BSPLINE, 1, &bcr_wb_twt);
   GMT_bcr_init (&grd_top_twt,     GMT_pad, BCR_BSPLINE, 1, &bcr_top_twt);
   GMT_bcr_init (&grd_middle_twt,  GMT_pad, BCR_BSPLINE, 1, &bcr_middle_twt);
   GMT_bcr_init (&grd_bottom_twt,  GMT_pad, BCR_BSPLINE, 1, &bcr_bottom_twt);
   GMT_bcr_init (&grd_top_z,       GMT_pad, BCR_BSPLINE, 1, &bcr_top_z);
   GMT_bcr_init (&grd_middle_z,    GMT_pad, BCR_BSPLINE, 1, &bcr_middle_z);
   GMT_bcr_init (&grd_bottom_z,    GMT_pad, BCR_BSPLINE, 1, &bcr_bottom_z);

   GMT_read_grd (coeff_top_k,       &grd_top_k,       f_top_k,       0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_middle_k,    &grd_middle_k,    f_middle_k,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_bottom_k,    &grd_bottom_k,    f_bottom_k,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_wb_twt,      &grd_wb_twt,      f_wb_twt,      0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_top_twt,     &grd_top_twt,     f_top_twt,     0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_middle_twt,  &grd_middle_twt,  f_middle_twt,  0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_bottom_twt,  &grd_bottom_twt,  f_bottom_twt,  0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_top_z,       &grd_top_z,       f_top_z,       0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_middle_z,    &grd_middle_z,    f_middle_z,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_bottom_z,    &grd_bottom_z,    f_bottom_z,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   if (!getpardouble ("vwater",&vwater)) vwater = 1452.05;

   if ( verbose ) {
      fprintf ( stderr, "Vwater = %f (meters/second)\n", vwater );
      fprintf ( stderr, "\n" );
   }

   rewind (stdin);

   factor1 = 0.0005;

   while (((char *) NULL) != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf", &x_loc, &y_loc, &twt_horizon );

      if ( x_loc >= grd_wb_twt.x_min && x_loc <= grd_wb_twt.x_max && y_loc >= grd_wb_twt.y_min && y_loc <= grd_wb_twt.y_max ) {

         value_coeff_top_k       = GMT_get_bcr_z (&grd_top_k,       x_loc, y_loc, f_top_k,       &edgeinfo_top_k,       &bcr_top_k);
         value_coeff_middle_k    = GMT_get_bcr_z (&grd_middle_k,    x_loc, y_loc, f_middle_k,    &edgeinfo_middle_k,    &bcr_middle_k);
         value_coeff_bottom_k    = GMT_get_bcr_z (&grd_bottom_k,    x_loc, y_loc, f_bottom_k,    &edgeinfo_bottom_k,    &bcr_bottom_k);
         value_coeff_wb_twt      = GMT_get_bcr_z (&grd_wb_twt,      x_loc, y_loc, f_wb_twt,      &edgeinfo_wb_twt,      &bcr_wb_twt);
         value_coeff_top_twt     = GMT_get_bcr_z (&grd_top_twt,     x_loc, y_loc, f_top_twt,     &edgeinfo_top_twt,     &bcr_top_twt);
         value_coeff_middle_twt  = GMT_get_bcr_z (&grd_middle_twt,  x_loc, y_loc, f_middle_twt,  &edgeinfo_middle_twt,  &bcr_middle_twt);
         value_coeff_bottom_twt  = GMT_get_bcr_z (&grd_bottom_twt,  x_loc, y_loc, f_bottom_twt,  &edgeinfo_bottom_twt,  &bcr_bottom_twt);
         value_coeff_top_z       = GMT_get_bcr_z (&grd_top_z,       x_loc, y_loc, f_top_z,       &edgeinfo_top_z,       &bcr_top_z);
         value_coeff_middle_z    = GMT_get_bcr_z (&grd_middle_z,    x_loc, y_loc, f_middle_z,    &edgeinfo_middle_z,    &bcr_middle_z);
         value_coeff_bottom_z    = GMT_get_bcr_z (&grd_bottom_z,    x_loc, y_loc, f_bottom_z,    &edgeinfo_bottom_z,    &bcr_bottom_z);

         if ( ! (GMT_is_dnan (value_coeff_wb_twt)     || GMT_is_dnan (value_coeff_top_k)    || GMT_is_dnan (value_coeff_top_twt) ||\
              GMT_is_dnan (value_coeff_bottom_twt) || GMT_is_dnan (value_coeff_bottom_k) || GMT_is_dnan (value_coeff_top_z)   ||\
              GMT_is_dnan (value_coeff_bottom_z)) ) {

            water_depth = value_coeff_wb_twt * factor1 * vwater;
            ratio = vwater / value_coeff_top_k;

	    if ( verbose == 2 ) {
               fprintf ( stderr, "\n" );
               fprintf ( stderr, "X-Loc = %10.2f Y-Loc = %10.2f\n", x_loc, y_loc );
               fprintf ( stderr, "Top_K          = %8.5f\n", value_coeff_top_k );
               fprintf ( stderr, "Middle_K       = %8.5f\n", value_coeff_middle_k );
               fprintf ( stderr, "Bottom_K       = %8.5f\n", value_coeff_bottom_k );
               fprintf ( stderr, "WB_TWT         = %8.2f\n", value_coeff_wb_twt );
               fprintf ( stderr, "TOP_TWT        = %8.2f\n", value_coeff_top_twt );
               fprintf ( stderr, "MIDDLE_TWT     = %8.2f\n", value_coeff_middle_twt );
               fprintf ( stderr, "BOTTOM_TWT     = %8.2f\n", value_coeff_bottom_twt );
               fprintf ( stderr, "TOP_Z          = %8.2f\n", value_coeff_top_z );
               fprintf ( stderr, "MIDDLE_Z       = %8.2f\n", value_coeff_middle_z );
               fprintf ( stderr, "BOTTOM_Z       = %8.2f\n", value_coeff_bottom_z );
            }

            delta_twt = value_coeff_bottom_twt - value_coeff_top_twt;
            delta_k   = value_coeff_bottom_k - value_coeff_top_k;
            gradient  = delta_k / delta_twt;

            delta_twt_middle_top = value_coeff_middle_twt - value_coeff_top_twt;
            delta_k_middle_top   = value_coeff_middle_k - value_coeff_top_k;
            gradient_middle_top  = delta_k_middle_top / delta_twt_middle_top;

            delta_twt_bottom_middle = value_coeff_bottom_twt - value_coeff_middle_twt;
            delta_k_bottom_middle   = value_coeff_bottom_k - value_coeff_middle_k;
            gradient_bottom_middle  = delta_k_bottom_middle / delta_twt_bottom_middle;

            if ( twt_horizon <= value_coeff_wb_twt ) {
               depth = twt_horizon * factor1 * vwater;
            } else if ( twt_horizon <= value_coeff_top_twt ) {
               twt = ( twt_horizon - value_coeff_wb_twt ) * factor1;
               depth = ( ratio * (exp (value_coeff_top_k*twt) - 1.0) ) + water_depth;
            } else if ( twt_horizon > value_coeff_top_twt && twt_horizon <= value_coeff_bottom_twt ) {
               if ( twt_horizon > value_coeff_top_twt && twt_horizon <= value_coeff_middle_twt ) {
                  K = value_coeff_top_k + ( ( twt_horizon - value_coeff_top_twt ) * gradient_middle_top ); 
               } else if ( twt_horizon > value_coeff_middle_twt && twt_horizon <= value_coeff_bottom_twt ) {
                  K = value_coeff_middle_k + ( ( twt_horizon - value_coeff_middle_twt ) * gradient_bottom_middle ); 
               } else {
                  K = value_coeff_top_k + ( ( twt_horizon - value_coeff_top_twt ) * gradient ); 
               }

               twt = ( twt_horizon - value_coeff_wb_twt ) * factor1;
               ratio = vwater / K;
               depth = ( ratio * (exp (K*twt) - 1.0) ) + water_depth;
            } else if ( twt_horizon > value_coeff_bottom_twt ) {
               twt = ( twt_horizon - value_coeff_wb_twt ) * factor1;
               ratio = vwater / value_coeff_bottom_k;
               depth = ( ratio * (exp (value_coeff_bottom_k*twt) - 1.0) ) + water_depth;
            }
            printf ( "%-12.2f %12.2f %12.4f\n", x_loc, y_loc, depth );
         }
      } else {
         if ( verbose == 3 ) fprintf ( stderr, "Xloc = %.0f Yloc = %.0f is out of bounds\n", x_loc, y_loc);
      }
   }

   GMT_free ((void *)f_top_k);
   GMT_free ((void *)f_middle_k);
   GMT_free ((void *)f_bottom_k);
   GMT_free ((void *)f_wb_twt);
   GMT_free ((void *)f_top_twt);
   GMT_free ((void *)f_middle_twt);
   GMT_free ((void *)f_bottom_twt);
   GMT_free ((void *)f_top_z);
   GMT_free ((void *)f_middle_z);
   GMT_free ((void *)f_bottom_z);

   GMT_end (argc, argv);

   return (0);
}
