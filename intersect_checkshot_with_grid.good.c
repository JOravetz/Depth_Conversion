#include "gmt.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))
#define abs(x)   ((x) <  0  ? -(x) : (x))

char *sdoc[] = {NULL};

float *f1;

int main (int argc, char **argv) {

   char *coeff_x, file[BUFSIZ], temp[256];
	
   struct GRD_HEADER grd_x;
   struct GMT_EDGEINFO edgeinfo_x;
   struct GMT_BCR bcr_x;
   
   short verbose;
   int kount, index;
   double slope_twt, slope_vavg, slope_depth, slope_x, slope_y, zero, min_diff, diff, value_coeff_x, x_loc, y_loc, twt, depth, vavg;
   double *diff_array, *x_array, *y_array, *twt_array, *depth_array, *vavg_array;
   register int i;

   FILE *fpp;
   cwp_String pfile;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring("coeff_x", &coeff_x)) coeff_x="80MaSB_final_grid_TWT_msec.dat.trimmed.smooth.grd";
   if (!getparstring("pfile",&pfile)) pfile = "./checkshots/FIELD-01_2013.reform.dat";
   if (!getparshort("verbose" , &verbose)) verbose = 0;

   fpp = efopen (pfile, "r");
   kount = -1;
   while (NULL != fgets ( temp, sizeof(temp), fpp )) {
      ++kount;
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf%lf", &x_loc, &y_loc, &twt, &depth, &vavg );
   }

   x_array     = ealloc1double ( kount );
   y_array     = ealloc1double ( kount );
   twt_array   = ealloc1double ( kount );
   depth_array = ealloc1double ( kount );
   vavg_array  = ealloc1double ( kount );
   diff_array  = ealloc1double ( kount );

   rewind ( fpp );
   for ( i = 0; i < kount; ++i ) {
      fgets ( temp, sizeof(temp), fpp );
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf%lf", &x_loc, &y_loc, &twt, &depth, &vavg );
      x_array[i]     = x_loc;
      y_array[i]     = y_loc;
      twt_array[i]   = twt;
      depth_array[i] = depth;
      vavg_array[i]  = vavg;
   }
   efclose ( fpp );

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "GMT grid file name = %s\n", coeff_x );
      fprintf ( stderr, "Checkshot file name = %s, Number of values in file = %5d\n", pfile, kount );
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

   min_diff = DBL_MAX;
   index = 0;
   zero = 0.0;
   for ( i = 0; i < kount; ++i ) {
      x_loc = x_array[i];
      y_loc = y_array[i];
      twt   = twt_array[i];
      vavg  = vavg_array[i];
      depth = depth_array[i];
      value_coeff_x  = abs ( GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x) );
      if ( !(GMT_is_dnan (value_coeff_x)) ) {
         diff_array[i] = diff = value_coeff_x - twt;
         if ( diff < min_diff && diff >= zero ) {
            min_diff = diff;
            index = i;
         }
         if ( verbose ) fprintf ( stderr, "Num = %5d X = %10.2f Y = %10.2f Vavg = %10.2f Depth = %12.4f TWT = %12.4f TWT_GRID = %12.4f DIFF = %12.6f\n", i, x_loc, y_loc, vavg, depth, twt, value_coeff_x, diff );
      }
   }

   if ( diff_array[kount-1] > zero ) return EXIT_FAILURE;
   slope_x = ( x_array[index] - x_array[index+1] ) / ( diff_array[index] - diff_array[index+1] );
   x_loc = x_array[index] - ( abs ( diff_array[index] ) * slope_x );
   slope_y = ( y_array[index] - y_array[index+1] ) / ( diff_array[index] - diff_array[index+1] );
   y_loc = y_array[index] - ( abs ( diff_array[index] ) * slope_y );
   slope_twt = ( twt_array[index] - twt_array[index+1] ) / ( diff_array[index] - diff_array[index+1] );
   twt = twt_array[index] - ( abs ( diff_array[index] ) * slope_twt );
   slope_vavg = ( vavg_array[index] - vavg_array[index+1] ) / ( diff_array[index] - diff_array[index+1] );
   vavg = vavg_array[index] - ( abs ( diff_array[index] ) * slope_vavg );
   slope_depth = ( depth_array[index] - depth_array[index+1] ) / ( diff_array[index] - diff_array[index+1] );
   depth = depth_array[index] - ( abs ( diff_array[index] ) * slope_depth );

   value_coeff_x  = abs ( GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x) );

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Index = %5d, Minimum difference = %12.4f, slope_x = %12.6f, Interpolated X = %12.6f, slope_y = %12.6f, Interpolated Y = %12.6f\n", index, min_diff, slope_x, x_loc, slope_y, y_loc );
      fprintf ( stderr, "Index = %5d, slope_twt = %12.6f, Interpolated TWT = %12.6f, slope_vavg = %12.6f, Interpolated VAVG = %12.6f, slope_depth = %12.6f, Interpolated DEPTH = %12.6f\n", index, slope_twt, twt, slope_vavg, vavg, slope_depth, depth );
      fprintf ( stderr, "TWT extracted from GRID = %12.6f, DIFF from Interpolated TWT = %12.6f\n", value_coeff_x, value_coeff_x - twt );
   }

   printf ( "%12.4f %12.4f %12.4f %12.4f %12.4f\n", x_loc, y_loc, value_coeff_x, depth, vavg );

   free1double ( x_array );
   free1double ( y_array );
   free1double ( twt_array );
   free1double ( depth_array );
   free1double ( vavg_array );
   free1double ( diff_array );

   GMT_free ((void *)f1);
   GMT_end (argc, argv);

   return EXIT_SUCCESS;
}
