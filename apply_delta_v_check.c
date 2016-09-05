#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define abs(x)   ( (x) <  0  ? -(x) : (x) )
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

int main (int argc, char **argv) {

   char temp[256];
   FILE *fpp;
   cwp_String pfile;
   short verbose;
   register int i, j, k, l;

   int nxm1, nym1, ntm1;
   int kount, iloc, jloc, nx, ny, nt;
   double xmin, xmax, ymin, ymax, tmin, tmax, delta_x, delta_y, delta_t;
   double xref, yref, tref, x_loc, y_loc, twt, delta_v;
   double *x_dv, *y_dv, ***dv;

   double x, y, time;
   double *vavg_array, *twt_dv_array;

   initargs(argc, argv);

   if (!getparshort("verbose" , &verbose)) verbose = 0;
   if (!getparstring("pfile",&pfile)) pfile = "Gridder.dat";
   if (!getparint("nx",&nx)) nx = 51;
   if (!getparint("ny",&ny)) ny = 51;
   if (!getparint("nt",&nt)) nt = 101;
  
   fpp = efopen (pfile, "r");

   kount = 0;
   xmin = ymin = tmin = FLT_MAX;
   xmax = ymax = tmax = FLT_MIN;
   xref = yref = tref = 0.0;
   while (NULL != fgets ( temp, sizeof(temp), fpp )) {
      ++kount;
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x_loc, &y_loc, &twt, &delta_v);

      xmin = min ( xmin, x_loc );
      ymin = min ( ymin, y_loc );
      tmin = min ( tmin, twt );

      xmax = max ( xmax, x_loc );
      ymax = max ( ymax, y_loc );
      tmax = max ( tmax, twt );
   }

   nxm1 = nx - 1;
   nym1 = ny - 1;
   ntm1 = nt - 1;

   delta_x = ( xmax - xmin ) / ( nxm1 );
   delta_y = ( ymax - ymin ) / ( nym1 );
   delta_t = ( tmax - tmin ) / ( ntm1 );

   x_dv = ealloc1double ( nx );
   y_dv = ealloc1double ( ny );
   dv   = ealloc3double ( nt, ny, nx );

   for ( i = 0; i < nx; ++i ) x_dv[i] = ( (double) i * delta_x ) + xmin;
   for ( j = 0; j < ny; ++j ) y_dv[j] = ( (double) j * delta_y ) + ymin;

   rewind ( fpp );
   for ( l = 0; l < kount; ++l ) {
      fgets ( temp, sizeof(temp), fpp );
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x_loc, &y_loc, &twt, &delta_v);
      i = min ( max ( nint ( ( x_loc - xmin ) / delta_x ), 0 ), nxm1 ); 
      j = min ( max ( nint ( ( y_loc - ymin ) / delta_y ), 0 ), nym1 ); 
      k = min ( max ( nint ( ( twt   - tmin ) / delta_t ), 0 ), ntm1 ); 
      dv[i][j][k] = delta_v;
   }
   efclose (fpp);

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Delta-V file name = %s, number of input samples = %d\n", pfile, kount);
      fprintf ( stderr, "Xmin = %.2f, Xmax = %.2f, Ymin = %.2f, Ymax = %.2f, Tmin = %.2f, Tmax = %.2f\n", xmin, xmax, ymin, ymax, tmin, tmax );
      fprintf ( stderr, "Delta-X = %.4f, Delta-Y = %.4f, Delta-T = %.4f\n", delta_x, delta_y, delta_t );
      fprintf ( stderr, "NX = %d, NY = %d, NT = %d\n", nx, ny, nt );
      fprintf ( stderr, "\n" );
   }

   vavg_array    = ealloc1double ( nt );
   twt_dv_array  = ealloc1double ( nt );

   for ( i = 0; i < nt; ++i ) twt_dv_array[i] = ( (double) i * delta_t ) + tmin;

   int num, signx, signy;
   double *dv_array, dv_interp, zero;
   double r_iloc, r_jloc, one, four, wx1, wx2, wy1, wy2;
   double remainder_x, remainder_y;
   double rx, ry;

   dv_array = ealloc1double ( nt );

   rewind (stdin);
   zero = 0.0;
   one = 1.0;
   four = 4.0;
   signx = signy = 1;

   num = 0;
   while (NULL != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x, &y, &twt, &delta_v);

      ++num;
      if ( verbose == 2 ) fprintf ( stderr, "trace = %5d, X = %.4f, Y = %.4f, TWT = %.4f, Delta-V = %12.6f\n", num, x, y, twt, delta_v );

      xref = ( x - xmin ) / delta_x;
      yref = ( y - ymin ) / delta_y;

      iloc = min ( max ( nint ( xref ), 1 ), nx-2 );
      jloc = min ( max ( nint ( yref ), 1 ), ny-2 );

      r_iloc = iloc;
      r_jloc = jloc;

      remainder_x = xref - r_iloc;
      remainder_y = yref - r_jloc;

      if ( verbose == 2 ) {
         rx = ( (double) iloc * delta_x ) + xmin; 
         ry = ( (double) jloc * delta_y ) + ymin; 
         fprintf ( stderr, "iloc = %5d, jloc = %5d, Original_X = %.4f Original_Y = %.4f remainder_x = %.6f, remainder_y = %.4f\n", iloc, jloc, rx, ry, remainder_x, remainder_y );
      }

      if ( (remainder_x >= zero) && (remainder_y >= zero) ) {
         signx = signy = 1;
         wx2 = remainder_x;
         wx1 = one - wx2;
         wy2 = remainder_y;
         wy1 = one - wy2;
      } else if ( (remainder_x < zero) && (remainder_y < zero) ) {
         signx = signy = -1;
         wx2 = -remainder_x;
         wx1 = -wx2 + one ;
         wy2 = -remainder_y;
         wy1 = -wy2 + one;
      } else if ( (remainder_x < zero) && (remainder_y > zero) ) {
         signx = -1;
         signy =  1;
         wx2 = -remainder_x;
         wx1 = -wx2 + one ;
         wy2 = remainder_y;
         wy1 = one - wy2;
      } else if ( (remainder_x > zero) && (remainder_y < zero) ) {
         signx =  1;
         signy = -1;
         wx2 = remainder_x;
         wx1 = one - wx2;
         wy2 = -remainder_y;
         wy1 = -wy2 + one;
      } else {
         fprintf ( stderr, "ERROR, something wrong with remainders --> exiting\n" );
         return EXIT_FAILURE;
      }

      for ( k = 0; k < nt; ++k ) {
         dv_array[k] = zero;
         dv_array[k] += ( ( dv[iloc][jloc][k]           * wx1 ) + ( dv[iloc+(1*signx)][jloc][k]           * wx2 ) + 
                          ( dv[iloc][jloc+(1*signy)][k] * wx1 ) + ( dv[iloc+(1*signx)][jloc+(1*signy)][k] * wx2 ) +
                          ( dv[iloc][jloc][k]           * wy1 ) + ( dv[iloc][jloc+(1*signy)][k]           * wy2 ) + 
                          ( dv[iloc+(1*signx)][jloc][k] * wy1 ) + ( dv[iloc+(1*signx)][jloc+(1*signy)][k] * wy2 ) ) / four;
         if ( verbose == 2 ) fprintf ( stderr, "TWT sample = %5d, TWT_checkshot = %.2f, DV_checkshot = %.4f\n", k, twt_dv_array[k], dv_array[k] );
      }

      time = twt;
      dintlin ( nt, twt_dv_array, dv_array, dv_array[0], dv_array[ntm1], 1, &time, &dv_interp);
      printf ( "%5d %12.2f %12.2f %8.2f %12.6f %12.6f %12.8f\n", num, x, y, twt, delta_v, dv_interp, delta_v - dv_interp );

   }

   free1double (vavg_array);
   free1double ( twt_dv_array );

   free1double (x_dv); 
   free1double (y_dv); 
   free3double (dv); 

   return EXIT_SUCCESS;
}
