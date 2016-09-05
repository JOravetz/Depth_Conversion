#include "gmt.h"
#include "nr.h"
#include "nrutil.h"
#include "su.h"
#include "par.h"
#include "stdio.h"

#define abs(x)  ( (x) <  0  ? -(x) : (x) )

char *sdoc[] = {NULL};

float *f1, *f2, *f3, *f4;

int main(int argc, char **argv) {

   char *coeff_x, *coeff_x2, *coeff_x3, *coeff_x4, file[BUFSIZ];
   struct GRD_HEADER grd_x, grd_x2, grd_x3, grd_x4;
   struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2, edgeinfo_x3, edgeinfo_x4;
   struct GMT_BCR bcr_x, bcr_x2, bcr_x3, bcr_x4;

   double x_loc, y_loc, value_coeff_x, value_coeff_x2, value_coeff_x3, value_coeff_x4;

   char temp[256];
   cwp_String pfile;
   FILE *fpp;
   short  verbose, check;
   int    kount, nump1;
   double sum, error1, sign;
   double delta, thresh;
   double *vzero_opt, *k_opt;
   double *datain, *samp, *xloc_array, *yloc_array;
   double depth_water, factor, num, sample;
   double vzero, k;
   double *wb_twt_array, *wb_z_array;
   double error, x, y, zero;
   register int i;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring("pfile",&pfile)) pfile = "tops.lis";
   if (!getparshort("verbose", &verbose)) verbose = 1;
   if (!getpardouble("delta", &delta)) delta = 0.00001;
   if (!getpardouble("thresh", &thresh)) thresh = 0.01;

   zero = 0.0;

   if (!getparstring("coeff_x", &coeff_x)) coeff_x="wb.twt.grd";
   if (!getparstring("coeff_x2", &coeff_x2)) coeff_x2="wvavg.dat.trimmed.sample.smooth.grd";
   if (!getparstring("coeff_x3", &coeff_x3)) coeff_x3="vzero.seismic.dat.trimmed.sample.smooth.grd";
   if (!getparstring("coeff_x4", &coeff_x4)) coeff_x4="k.seismic.dat.trimmed.sample.smooth.grd";

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "WB TWT (ms.) GMT grid file name = %s\n", coeff_x );
      fprintf ( stderr, "WB VAVG (m)  GMT grid file name = %s\n", coeff_x2 );
      fprintf ( stderr, "Seismic VZERO GMT grid file name = %s\n", coeff_x3 );
      fprintf ( stderr, "Seismic K GMT grid file name = %s\n", coeff_x4 );
      fprintf ( stderr, "Delta = %.10f\n", delta );
      fprintf ( stderr, "Threshold = %f\n", thresh );
   }

   GMT_boundcond_init (&edgeinfo_x);
   GMT_boundcond_init (&edgeinfo_x2);
   GMT_boundcond_init (&edgeinfo_x3);
   GMT_boundcond_init (&edgeinfo_x4);

   GMT_grd_init (&grd_x,  argc, argv, FALSE);
   GMT_grd_init (&grd_x2, argc, argv, FALSE);
   GMT_grd_init (&grd_x3, argc, argv, FALSE);
   GMT_grd_init (&grd_x4, argc, argv, FALSE);

   if (GMT_read_grd_info (coeff_x,  &grd_x))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_x2, &grd_x2)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_x3, &grd_x3)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_x4, &grd_x4)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);

   f1 = (float *) GMT_memory (VNULL, (size_t)((grd_x.nx  + 4) * (grd_x.ny  + 4)), sizeof(float), GMT_program);
   f2 = (float *) GMT_memory (VNULL, (size_t)((grd_x2.nx + 4) * (grd_x2.ny + 4)), sizeof(float), GMT_program);
   f3 = (float *) GMT_memory (VNULL, (size_t)((grd_x3.nx + 4) * (grd_x3.ny + 4)), sizeof(float), GMT_program);
   f4 = (float *) GMT_memory (VNULL, (size_t)((grd_x4.nx + 4) * (grd_x4.ny + 4)), sizeof(float), GMT_program);

   GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

   GMT_boundcond_param_prep (&grd_x,  &edgeinfo_x);
   GMT_boundcond_param_prep (&grd_x2, &edgeinfo_x2);
   GMT_boundcond_param_prep (&grd_x3, &edgeinfo_x3);
   GMT_boundcond_param_prep (&grd_x4, &edgeinfo_x4);

   GMT_boundcond_set (&grd_x, &edgeinfo_x, GMT_pad, f1);
   GMT_boundcond_set (&grd_x2, &edgeinfo_x2, GMT_pad, f2);
   GMT_boundcond_set (&grd_x3, &edgeinfo_x3, GMT_pad, f3);
   GMT_boundcond_set (&grd_x4, &edgeinfo_x4, GMT_pad, f4);

   GMT_bcr_init (&grd_x,  GMT_pad, BCR_BSPLINE, 1, &bcr_x);
   GMT_bcr_init (&grd_x2, GMT_pad, BCR_BSPLINE, 1, &bcr_x2);
   GMT_bcr_init (&grd_x3, GMT_pad, BCR_BSPLINE, 1, &bcr_x3);
   GMT_bcr_init (&grd_x4, GMT_pad, BCR_BSPLINE, 1, &bcr_x4);

   GMT_read_grd (coeff_x,  &grd_x,  f1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x2, &grd_x2, f2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x3, &grd_x3, f3, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_x4, &grd_x4, f4, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   fpp = efopen (pfile, "r");

   kount = 0;
   while (NULL != fgets ( temp, sizeof(temp), fpp )) {
      ++kount;
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x_loc, &y_loc, &num, &sample );
   }

   if ( verbose != 0) {
      fprintf (stderr,"\n");
      fprintf (stderr,"Data file name = %s, number of input samples = %d\n", pfile, kount);
      fprintf (stderr,"\n");
   }

   samp       = ealloc1double ( kount );
   datain     = ealloc1double ( kount );
   xloc_array = ealloc1double ( kount );
   yloc_array = ealloc1double ( kount );
   vzero_opt  = ealloc1double ( kount );
   k_opt      = ealloc1double ( kount );

   wb_twt_array = ealloc1double ( kount );
   wb_z_array   = ealloc1double ( kount );

   rewind ( fpp );
   kount = -1;
   while (NULL != fgets ( temp, sizeof(temp), fpp )) {
      ++kount;
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x_loc, &y_loc, &num, &sample );
      xloc_array[kount] = x_loc;
      yloc_array[kount] = y_loc;
      samp[kount]   = num;
      datain[kount] = sample;
      if ( verbose == 2 ) fprintf ( stderr, "kount = %5d, x_loc = %12.2f, y_loc = %12.2f, num = %8.2f, sample = %8.2f\n", kount, xloc_array[kount], yloc_array[kount], samp[kount], datain[kount] );
   }
   if ( verbose == 2 ) fprintf ( stderr, "\n" );
   efclose (fpp);

   factor = 0.0005;

   nump1 = kount + 1;
   depth_water = error = zero;
   for ( i=0; i<=kount; i++ ) {
      check = 0;
      x_loc = xloc_array[i];
      y_loc = yloc_array[i];
      if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) check = 1;

      if ( check ) {
         value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
         value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);
         value_coeff_x3 = GMT_get_bcr_z (&grd_x3, x_loc, y_loc, f3, &edgeinfo_x3, &bcr_x3);
         value_coeff_x4 = GMT_get_bcr_z (&grd_x4, x_loc, y_loc, f4, &edgeinfo_x4, &bcr_x4);
         if (GMT_is_dnan (value_coeff_x) || GMT_is_dnan (value_coeff_x2) || GMT_is_dnan (value_coeff_x3) || GMT_is_dnan (value_coeff_x4) ) {
            check = 0;
         } else {
            if ( value_coeff_x < 0.0 ) value_coeff_x *= -1.0;
            if ( value_coeff_x2 < 0.0 ) value_coeff_x2 *= -1.0;
            if ( value_coeff_x3 < 0.0 ) value_coeff_x3 *= -1.0;
            if ( value_coeff_x4 < 0.0 ) value_coeff_x4 *= -1.0;
            samp[i] -= value_coeff_x;        
            depth_water = value_coeff_x * value_coeff_x2 * factor;
            datain[i] -= depth_water;        
            wb_twt_array[i] = value_coeff_x;
            wb_z_array[i] = depth_water;
            if ( verbose == 3 ) fprintf ( stderr, "num = %5d, xloc = %10.2f yloc = %10.2f, WB TWT = %8.2f ms., WB Depth = %8.2f m., WB VEL = %8.2f mps., Vzero = %20.15f, K = %20.15f\n", 
              i, x_loc, y_loc, value_coeff_x, depth_water, value_coeff_x2, value_coeff_x3, value_coeff_x4 );
            vzero_opt[i] = value_coeff_x3;
            k_opt[i] = value_coeff_x4;
         }
      }
   }
   if ( verbose == 3 ) fprintf ( stderr, "\n" );

   sum = zero;
   for ( i=0; i<=kount; i++ ) {
      vzero = vzero_opt[i];
      k = k_opt[i];
      x = samp[i];
      y = ( vzero / k ) * ( exp ( k * x * factor ) - 1.0 );
      error += abs ( y - datain[i] );
      if ( verbose == 3 ) fprintf ( stderr, "%-5d %12.2f %12.2f %12.2f %12.4f %12.4f\n", i, x, datain[i], y, y - datain[i], error );

      sign = 1.0;
      error = error1 = y - datain[i];

      for (;;) {
         if ( abs(error1) < thresh ) break;

         vzero = vzero_opt[i];
         k += ( delta * sign );

         x = samp[i];
         y = ( vzero / k ) * ( exp ( k * x * factor ) - 1.0 );
         error1 = y - datain[i];

         if ( verbose == 2 ) fprintf ( stderr, "%-5d %12.2f %12.2f %12.2f %12.4f\n", i, x, datain[i], y, error1 );
         if ( (error1 > zero && error1 > error) || (error1 < zero && error1 < error) ) sign *= -1.0;
      }
      sum += abs ( y - datain[i] );
      if ( verbose ) {
         fprintf ( stderr, "%-5d %12.2f %12.2f %12.2f %12.4f %12.4f\n", i, x, datain[i], y, y - datain[i], sum );
         fprintf ( stderr, "%12.2f %12.2f ", xloc_array[i], yloc_array[i] );
         fprintf ( stderr, "%30.25f %30.25f %8.2f %8.2f %8.2f\n", vzero, k, wb_twt_array[i], wb_z_array[i], sum / (float) nump1 );
      }
      printf ( "%12.2f %12.2f ", xloc_array[i], yloc_array[i] );
      printf ( "%30.25f %30.25f %8.2f %8.2f %8.2f\n", vzero, k, wb_twt_array[i], wb_z_array[i], sum / (float) nump1 );
   }

   free1double (samp);
   free1double (datain);
   free1double (xloc_array);
   free1double (yloc_array);
   free1double (vzero_opt);
   free1double (k_opt);
   free1double (wb_twt_array);
   free1double (wb_z_array);

   GMT_free ((void *)f1);
   GMT_free ((void *)f2);
   GMT_free ((void *)f3);
   GMT_free ((void *)f4);

   GMT_end (argc, argv);

   return EXIT_SUCCESS;
}
