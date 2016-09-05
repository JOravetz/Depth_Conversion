#include "gmt.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"
#include "header.h"

#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

segy tr;
float *f_top_k,  *f_bottom_k;
float *f_wb_twt, *f_top_twt,  *f_bottom_twt;
float *f_coeff_a, *f_coeff_b, *f_coeff_c;

int main (int argc, char **argv) {

   char file[BUFSIZ];
   char *coeff_top_k, *coeff_bottom_k;
   char *coeff_wb_twt, *coeff_top_twt, *coeff_bottom_twt;
   char *coeff_a, *coeff_b, *coeff_c;

   struct GRD_HEADER grd_top_k, grd_bottom_k;
   struct GRD_HEADER grd_wb_twt, grd_top_twt, grd_bottom_twt;
   struct GRD_HEADER grd_coeff_a, grd_coeff_b, grd_coeff_c;

   struct GMT_EDGEINFO edgeinfo_top_k, edgeinfo_bottom_k;
   struct GMT_EDGEINFO edgeinfo_wb_twt, edgeinfo_top_twt, edgeinfo_bottom_twt;
   struct GMT_EDGEINFO edgeinfo_coeff_a, edgeinfo_coeff_b, edgeinfo_coeff_c;

   struct GMT_BCR bcr_top_k, bcr_bottom_k;
   struct GMT_BCR bcr_wb_twt, bcr_top_twt, bcr_bottom_twt;
   struct GMT_BCR bcr_coeff_a, bcr_coeff_b, bcr_coeff_c;

   double value_coeff_top_k, value_coeff_bottom_k;
   double value_coeff_wb_twt, value_coeff_top_twt, value_coeff_bottom_twt;

   short  verbose;
   int    scalar, nz, ntr, ns;
   double water_depth, vwater, ratio, factor1, dz, x_loc, y_loc; 
   double tr_msec, tr_msec_orig, dt_msec, depth_input, amp_output;
   double delrt_depth, value_coeff_a, value_coeff_b, value_coeff_c;
   double dlast, factor_a, factor_b, factor_c;
   double *tr_amp, *depth;
   register int k, n;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring ("coeff_top_k",      &coeff_top_k))      coeff_top_k      = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/below.ml.K.grd";
   if (!getparstring ("coeff_bottom_k",   &coeff_bottom_k))   coeff_bottom_k   = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/bottom.K.grd";
   if (!getparstring ("coeff_wb_twt",     &coeff_wb_twt))     coeff_wb_twt     = "/home/user/FIELD.new/PICKS/wb.twt.grd";
   if (!getparstring ("coeff_top_twt",    &coeff_top_twt))    coeff_top_twt    = "/home/user/FIELD.new/PICKS/TWT_GRIDS/01_FIELD_Top_Reservoir_twt.reform.dat.trimmed.grd";
   if (!getparstring ("coeff_bottom_twt", &coeff_bottom_twt)) coeff_bottom_twt = "/home/user/FIELD.new/PICKS/TWT_GRIDS/10_FIELD_80MaSB_twt.reform.dat.trimmed.grd";
   if (!getparstring ("coeff_a",          &coeff_a))          coeff_a          = "/home/user/FIELD.new/POLY/a.coeffs.100.dat.trimmed.sample.grd";
   if (!getparstring ("coeff_b",          &coeff_b))          coeff_b          = "/home/user/FIELD.new/POLY/b.coeffs.100000.dat.trimmed.sample.grd";
   if (!getparstring ("coeff_c",          &coeff_c))          coeff_c          = "/home/user/FIELD.new/POLY/c.coeffs.100000000.dat.trimmed.sample.grd";

   if (!getpardouble("factor_a", &factor_a)) factor_a = 100.0;
   if (!getpardouble("factor_b", &factor_b)) factor_b = 100000.0;
   if (!getpardouble("factor_c", &factor_c)) factor_c = 100000000.0;

   if (!getparshort ("verbose", &verbose)) verbose = 0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Top_K      - GMT grid file name = %s\n", coeff_top_k );
      fprintf ( stderr, "Bottom_K   - GMT grid file name = %s\n", coeff_bottom_k );
      fprintf ( stderr, "WB_TWT     - GMT grid file name = %s\n", coeff_wb_twt );
      fprintf ( stderr, "TOP_TWT    - GMT grid file name = %s\n", coeff_top_twt );
      fprintf ( stderr, "BOTTOM_TWT - GMT grid file name = %s\n", coeff_bottom_twt );
      fprintf ( stderr, "COEFF_A    - GMT grid file name = %s\n", coeff_a );
      fprintf ( stderr, "COEFF_B    - GMT grid file name = %s\n", coeff_b );
      fprintf ( stderr, "COEFF_C    - GMT grid file name = %s\n", coeff_c );
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "FACTOR_A = %-10.2f\n", factor_a );
      fprintf ( stderr, "FACTOR_B = %-10.2f\n", factor_b );
      fprintf ( stderr, "FACTOR_C = %-10.2f\n", factor_c );
      fprintf ( stderr, "\n" );
   }

   GMT_boundcond_init (&edgeinfo_top_k);
   GMT_boundcond_init (&edgeinfo_bottom_k);
   GMT_boundcond_init (&edgeinfo_wb_twt);
   GMT_boundcond_init (&edgeinfo_top_twt);
   GMT_boundcond_init (&edgeinfo_bottom_twt);
   GMT_boundcond_init (&edgeinfo_coeff_a);
   GMT_boundcond_init (&edgeinfo_coeff_b);
   GMT_boundcond_init (&edgeinfo_coeff_c);

   GMT_grd_init (&grd_top_k,       argc, argv, FALSE);
   GMT_grd_init (&grd_bottom_k,    argc, argv, FALSE);
   GMT_grd_init (&grd_wb_twt,      argc, argv, FALSE);
   GMT_grd_init (&grd_top_twt,     argc, argv, FALSE);
   GMT_grd_init (&grd_bottom_twt,  argc, argv, FALSE);
   GMT_grd_init (&grd_coeff_a, argc, argv, FALSE);
   GMT_grd_init (&grd_coeff_b, argc, argv, FALSE);
   GMT_grd_init (&grd_coeff_c, argc, argv, FALSE);

   if (GMT_read_grd_info (coeff_top_k,      &grd_top_k))      fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_bottom_k,   &grd_bottom_k))   fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_wb_twt,     &grd_wb_twt))     fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_top_twt,    &grd_top_twt))    fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_bottom_twt, &grd_bottom_twt)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_a,          &grd_coeff_a))    fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_b,          &grd_coeff_b))    fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_c,          &grd_coeff_c))    fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);

   f_top_k      = (float *) GMT_memory (VNULL, (size_t)((grd_top_k.nx + 4)       * (grd_top_k.ny  + 4)),       sizeof(float), GMT_program);
   f_bottom_k   = (float *) GMT_memory (VNULL, (size_t)((grd_bottom_k.nx + 4)    * (grd_bottom_k.ny  + 4)),    sizeof(float), GMT_program);
   f_wb_twt     = (float *) GMT_memory (VNULL, (size_t)((grd_wb_twt.nx + 4)      * (grd_wb_twt.ny  + 4)),      sizeof(float), GMT_program);
   f_top_twt    = (float *) GMT_memory (VNULL, (size_t)((grd_top_twt.nx + 4)     * (grd_top_twt.ny  + 4)),     sizeof(float), GMT_program);
   f_bottom_twt = (float *) GMT_memory (VNULL, (size_t)((grd_bottom_twt.nx + 4)  * (grd_bottom_twt.ny  + 4)),  sizeof(float), GMT_program);
   f_coeff_a    = (float *) GMT_memory (VNULL, (size_t)((grd_coeff_a.nx + 4)     * (grd_coeff_a.ny + 4)),      sizeof(float), GMT_program);
   f_coeff_b    = (float *) GMT_memory (VNULL, (size_t)((grd_coeff_b.nx + 4)     * (grd_coeff_b.ny + 4)),      sizeof(float), GMT_program);
   f_coeff_c    = (float *) GMT_memory (VNULL, (size_t)((grd_coeff_c.nx + 4)     * (grd_coeff_c.ny + 4)),      sizeof(float), GMT_program);

   GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

   GMT_boundcond_param_prep (&grd_top_k,      &edgeinfo_top_k);
   GMT_boundcond_param_prep (&grd_bottom_k,   &edgeinfo_bottom_k);
   GMT_boundcond_param_prep (&grd_wb_twt,     &edgeinfo_wb_twt);
   GMT_boundcond_param_prep (&grd_top_twt,    &edgeinfo_top_twt);
   GMT_boundcond_param_prep (&grd_bottom_twt, &edgeinfo_bottom_twt);
   GMT_boundcond_param_prep (&grd_coeff_a,    &edgeinfo_coeff_a);
   GMT_boundcond_param_prep (&grd_coeff_b,    &edgeinfo_coeff_b);
   GMT_boundcond_param_prep (&grd_coeff_c,    &edgeinfo_coeff_c);

   GMT_boundcond_set (&grd_top_k,      &edgeinfo_top_k,      GMT_pad, f_top_k);
   GMT_boundcond_set (&grd_bottom_k,   &edgeinfo_bottom_k,   GMT_pad, f_bottom_k);
   GMT_boundcond_set (&grd_wb_twt,     &edgeinfo_wb_twt,     GMT_pad, f_wb_twt);
   GMT_boundcond_set (&grd_top_twt,    &edgeinfo_top_twt,    GMT_pad, f_top_twt);
   GMT_boundcond_set (&grd_bottom_twt, &edgeinfo_bottom_twt, GMT_pad, f_bottom_twt);
   GMT_boundcond_set (&grd_coeff_a,    &edgeinfo_coeff_a,    GMT_pad, f_coeff_a);
   GMT_boundcond_set (&grd_coeff_b,    &edgeinfo_coeff_b,    GMT_pad, f_coeff_b);
   GMT_boundcond_set (&grd_coeff_c,    &edgeinfo_coeff_c,    GMT_pad, f_coeff_c);

   GMT_bcr_init (&grd_top_k,      GMT_pad, BCR_BSPLINE, 1, &bcr_top_k);
   GMT_bcr_init (&grd_bottom_k,   GMT_pad, BCR_BSPLINE, 1, &bcr_bottom_k);
   GMT_bcr_init (&grd_wb_twt,     GMT_pad, BCR_BSPLINE, 1, &bcr_wb_twt);
   GMT_bcr_init (&grd_top_twt,    GMT_pad, BCR_BSPLINE, 1, &bcr_top_twt);
   GMT_bcr_init (&grd_bottom_twt, GMT_pad, BCR_BSPLINE, 1, &bcr_bottom_twt);
   GMT_bcr_init (&grd_coeff_a,    GMT_pad, BCR_BSPLINE, 1, &bcr_coeff_a);
   GMT_bcr_init (&grd_coeff_b,    GMT_pad, BCR_BSPLINE, 1, &bcr_coeff_b);
   GMT_bcr_init (&grd_coeff_c,    GMT_pad, BCR_BSPLINE, 1, &bcr_coeff_c);

   GMT_read_grd (coeff_top_k,      &grd_top_k,      f_top_k,      0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_bottom_k,   &grd_bottom_k,   f_bottom_k,   0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_wb_twt,     &grd_wb_twt,     f_wb_twt,     0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_top_twt,    &grd_top_twt,    f_top_twt,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_bottom_twt, &grd_bottom_twt, f_bottom_twt, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_a,          &grd_coeff_a,    f_coeff_a,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_b,          &grd_coeff_b,    f_coeff_b,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_c,          &grd_coeff_c,    f_coeff_c,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

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
      depth = ealloc1double ( ns );
   } else {
      depth = ealloc1double ( nz );
   }
   tr_amp = ealloc1double ( ns );

   factor1 = 0.0005;
   delrt_depth = dlast = 0.0;

   /* Main loop over traces */
   for ( k = 0; k < ntr; ++k ) {
      gettr (&tr);
      x_loc = tr.sx / scalar;
      y_loc = tr.sy / scalar;
      for ( n=ns; n < nz; ++n ) depth[n] = 0;

      if ( x_loc >= grd_wb_twt.x_min && x_loc <= grd_wb_twt.x_max && y_loc >= grd_wb_twt.y_min && y_loc <= grd_wb_twt.y_max ) {

         value_coeff_top_k      = GMT_get_bcr_z (&grd_top_k,      x_loc, y_loc, f_top_k,      &edgeinfo_top_k,      &bcr_top_k);
         value_coeff_bottom_k   = GMT_get_bcr_z (&grd_bottom_k,   x_loc, y_loc, f_bottom_k,   &edgeinfo_bottom_k,   &bcr_bottom_k);
         value_coeff_wb_twt     = GMT_get_bcr_z (&grd_wb_twt,     x_loc, y_loc, f_wb_twt,     &edgeinfo_wb_twt,     &bcr_wb_twt);
         value_coeff_top_twt    = GMT_get_bcr_z (&grd_top_twt,    x_loc, y_loc, f_top_twt,    &edgeinfo_top_twt,    &bcr_top_twt);
         value_coeff_bottom_twt = GMT_get_bcr_z (&grd_bottom_twt, x_loc, y_loc, f_bottom_twt, &edgeinfo_bottom_twt, &bcr_bottom_twt);
         value_coeff_a          = GMT_get_bcr_z (&grd_coeff_a,    x_loc, y_loc, f_coeff_a,    &edgeinfo_coeff_a,    &bcr_coeff_a);
         value_coeff_b          = GMT_get_bcr_z (&grd_coeff_b,    x_loc, y_loc, f_coeff_b,    &edgeinfo_coeff_b,    &bcr_coeff_b);
         value_coeff_c          = GMT_get_bcr_z (&grd_coeff_c,    x_loc, y_loc, f_coeff_c,    &edgeinfo_coeff_c,    &bcr_coeff_c);

         if ( GMT_is_dnan (value_coeff_wb_twt)     || GMT_is_dnan (value_coeff_top_k)    || GMT_is_dnan (value_coeff_top_twt) ||\
              GMT_is_dnan (value_coeff_bottom_twt) || GMT_is_dnan (value_coeff_bottom_k) || GMT_is_dnan (value_coeff_a)       || GMT_is_dnan (value_coeff_b) || GMT_is_dnan (value_coeff_c)) {

            for ( n=0; n < nz; ++n )  tr.data[n] = 0;
            tr.delrt = 0;
            tr.trid = 0;

         } else {

            water_depth = value_coeff_wb_twt * factor1 * vwater;
            ratio = vwater / value_coeff_top_k;

            value_coeff_a /= factor_a;
            value_coeff_b /= factor_b;
            value_coeff_c /= factor_c;

            if ( verbose == 2 ) {
               fprintf ( stderr, "\n" );
               fprintf ( stderr, "Trace num = %5d X-Loc = %10.2f Y-Loc = %10.2f\n", k+1, x_loc, y_loc );
               fprintf ( stderr, "Top_K      = %8.5f\n", value_coeff_top_k );
               fprintf ( stderr, "Bottom_K   = %8.5f\n", value_coeff_bottom_k );
               fprintf ( stderr, "WB_TWT     = %8.2f\n", value_coeff_wb_twt );
               fprintf ( stderr, "TOP_TWT    = %8.2f\n", value_coeff_top_twt );
               fprintf ( stderr, "BOTTOM_TWT = %8.2f\n", value_coeff_bottom_twt );
               fprintf ( stderr, "COEFF_A    = %20.15f\n", value_coeff_a );
               fprintf ( stderr, "COEFF_B    = %20.15f\n", value_coeff_b );
               fprintf ( stderr, "COEFF_C    = %20.15f\n", value_coeff_c );
            }

            for ( n=0; n < ns; ++n ) {
               tr_amp[n] = tr.data[n];

               if ( tr.delrt == 0 ) {
                  tr_msec = n * dt_msec;
               } else {
                  tr_msec = tr.delrt + ( n * dt_msec );
               }
 
               if ( tr_msec <= value_coeff_wb_twt ) {
                  depth[n] = tr_msec * factor1 * vwater;
                  if ( tr_msec <= tr.delrt ) delrt_depth = depth[n];
               } else if ( tr_msec <= value_coeff_top_twt ) {
                  tr_msec_orig = tr_msec;
                  tr_msec = ( tr_msec - value_coeff_wb_twt ) * factor1;
                  depth[n] = dlast = ( ratio * (exp (value_coeff_top_k*tr_msec) - 1.0) ) + water_depth;
                  if ( tr_msec_orig <= tr.delrt ) delrt_depth = depth[n];
               } else if ( tr_msec > value_coeff_top_twt && tr_msec <= value_coeff_bottom_twt ) {
                  tr_msec_orig = tr_msec;
                  tr_msec -= value_coeff_top_twt;
                  depth[n] = (tr_msec*value_coeff_a) + (value_coeff_b*pow(tr_msec,2.0)) + (value_coeff_c*pow(tr_msec,3.0)) + dlast;
                  if ( verbose == 2 ) fprintf ( stderr, "Trace = %5d, Sample = %5d, tr_msec = %8.2f, dlast = %8.2f, depth = %8.2f, delta_z = %8.4f\n", k+1, n, tr_msec, dlast, depth[n], depth[n]-dlast );
                  if ( tr_msec_orig <= tr.delrt ) delrt_depth = depth[n]; 
                  /* tr_amp[n] = 3.5; */
               } else if ( tr_msec > value_coeff_bottom_twt ) {
                  tr_msec_orig = tr_msec;
                  tr_msec = ( tr_msec - value_coeff_wb_twt ) * factor1;
                  ratio = vwater / value_coeff_bottom_k;
                  depth[n] = ( ratio * (exp (value_coeff_bottom_k*tr_msec) - 1.0) ) + water_depth;
                  if ( tr_msec_orig <= tr.delrt ) delrt_depth = depth[n];
               }
            }

            for ( n=0; n < nz; ++n ) {
               depth_input = n * dz;
               if ( depth_input < delrt_depth ) {
                  tr.data[n] = 0.0;
               } else {
                  dintlin ( ns, depth, tr_amp, tr_amp[0], tr_amp[ns-1], 1, &depth_input, &amp_output );
                  tr.data[n] = (float) amp_output;
               }
            }
            tr.trid = 1;
         }

         tr.ns = nz;
         tr.delrt = 0;
         tr.dt = nint(dz*1000);
         puttr (&tr);

      } else {
         if ( verbose == 3 ) fprintf ( stderr, "Input trace = %d, Xloc = %.0f Yloc = %.0f is out of bounds\n", k, x_loc, y_loc);
      }
   }

   GMT_free ((void *) f_top_k);
   GMT_free ((void *) f_bottom_k);
   GMT_free ((void *) f_wb_twt);
   GMT_free ((void *) f_top_twt);
   GMT_free ((void *) f_bottom_twt);
   GMT_free ((void *) f_coeff_a);
   GMT_free ((void *) f_coeff_b);
   GMT_free ((void *) f_coeff_c);

   free1double (depth);
   free1double (tr_amp);

   GMT_end (argc, argv);

   return (0);
}
