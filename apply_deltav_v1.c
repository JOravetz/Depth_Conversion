#include <stdio.h>
#include <float.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define abs(x)   ( (x) <  0  ? -(x) : (x) )
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

segy tr;

int main (int argc, char **argv) {

   FILE *fpp;
   cwp_String pfile;
   short verbose;
   register int i, j, k;

   int ntr_vel, ns_vel, nxm1, nym1, ntm1;
   int ntr, ns, kount, iloc, jloc, nx, ny, nt;
   double delrt, xmin, xmax, ymin, ymax, tmin, tmax, delta_x, delta_y, delta_t;
   double dvmin, dvmax, xref, yref, tref, x_loc, y_loc, twt, scalco_vel;
   double *x_dv, *y_dv, ***dv;

   float x, y, time, dt, scale_factor;
   float *twt_array, *vavg_array, *twt_dv_array;

   initargs(argc, argv);

   if (!getparshort("verbose" , &verbose)) verbose = 0;
   if (!getparstring("pfile",&pfile)) pfile = "gridder_checkshots2.su";
   if (!getpardouble("delta_x",&delta_x)) delta_x = 24.23084821;
   if (!getpardouble("delta_y",&delta_y)) delta_y = 24.75703786;
   if (!getpardouble("delta_t",&delta_t)) delta_t = 1.13837919;
  
   fpp = efopen (pfile, "r");
   ntr_vel = fgettra (fpp, &tr, 0);
   ns_vel  = tr.ns;
   scalco_vel = tr.scalco;
   delrt = tr.delrt;
   if (scalco_vel < 0.0 ) scalco_vel *= -1.0;
   if (scalco_vel == 0.0 ) scalco_vel = 1.0;

   kount = 0;
   xmin = ymin = tmin = dvmin = FLT_MAX;
   xmax = ymax = tmax = dvmax = FLT_MIN;
   xref = yref = tref = 0.0;
   for ( i = 0; i < ntr_vel; ++i ) {
      ++kount;
      fgettr ( fpp, &tr );
      x_loc = tr.sx / scalco_vel;
      y_loc = tr.sy / scalco_vel;
      xmin = min ( xmin, x_loc );
      ymin = min ( ymin, y_loc );
      xmax = max ( xmax, x_loc );
      ymax = max ( ymax, y_loc );
      for ( j = 0; j < ns_vel; ++j ) {
         twt   = ( j * delta_t ) + delrt;
         tmin  = min ( tmin, twt );
         dvmin = min ( dvmin, tr.data[j] );
         tmax  = max ( tmax, twt );
         dvmax = max ( dvmax, tr.data[j] );
      }
   }

   nx = nint ( abs ( xmax - xmin ) / delta_x ) + 1;
   ny = nint ( abs ( ymax - ymin ) / delta_y ) + 1;
   nt = nint ( abs ( tmax - tmin ) / delta_t ) + 1;

   nxm1 = nx - 1;
   nym1 = ny - 1;
   ntm1 = nt - 1;

   x_dv = ealloc1double ( nx );
   y_dv = ealloc1double ( ny );
   dv   = ealloc3double ( nt, ny, nx );

   for ( i = 0; i < nx; ++i ) x_dv[i] = ( i * delta_x ) + xmin;
   for ( j = 0; j < ny; ++j ) y_dv[j] = ( j * delta_y ) + ymin;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Delta-V file name = %s, number of input samples = %d\n", pfile, kount);
      fprintf ( stderr, "Xmin = %.2f, Xmax = %.2f, Ymin = %.2f, Ymax = %.2f, Tmin = %.2f, Tmax = %.2f\n", xmin, xmax, ymin, ymax, tmin, tmax );
      fprintf ( stderr, "Delta-X = %.4f, Delta-Y = %.4f, Delta-T = %.4f\n", delta_x, delta_y, delta_t );
      fprintf ( stderr, "NX = %d, NY = %d, NT = %d\n", nx, ny, nt );
      fprintf ( stderr, "Coordinate Scalar = %f, Time Lag (delrt) = %f\n", scalco_vel, delrt );
      fprintf ( stderr, "DV_min = %f, DV_Max = %f\n", dvmin, dvmax );
      fprintf ( stderr, "\n" );
   }

   rewind ( fpp );
   for ( j = 0; j < ny; ++j ) {
      for ( i = 0; i < nx; ++i ) {
         fgettr ( fpp, &tr );
         x_loc = tr.sx / scalco_vel;
         y_loc = tr.sy / scalco_vel;
         i = min ( max ( nint ( ( x_loc - xmin ) / delta_x ), 0 ), nx-1 ); 
         j = min ( max ( nint ( ( y_loc - ymin ) / delta_y ), 0 ), ny-1 ); 
         for ( k = 0; k < nt; ++k ) dv[i][j][k] = tr.data[k];
      }
   }
   efclose (fpp);

   ntr = gettra (&tr, 0);
   ns  = tr.ns;
   dt  = tr.dt * 0.001;
   scale_factor = tr.scalco;
   if (scale_factor < 0.0 ) scale_factor *= -1.0;
   if (scale_factor == 0.0 ) scale_factor = 1.0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Number of input average velocity traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "Time sample rate (milliseconds) = %f\n", dt );
      fprintf ( stderr, "Coordinate scale factor = %f\n", scale_factor );
      fprintf ( stderr, "\n" );
   }

   twt_array     = ealloc1float ( ns );
   vavg_array    = ealloc1float ( ns );
   twt_dv_array  = ealloc1float ( ntm1 );

   for ( i = 0; i < ns; ++i ) twt_array[i] = i * dt;
   for ( i = 0; i < nt; ++i ) twt_dv_array[i] = i * delta_t;

   int signx, signy, ishift;
   float *dv_array, dv_interp, zero;
   double r_iloc, r_jloc, one, four, wx1, wx2, wy1, wy2, tlast;
   double remainder_x, remainder_y;

   dv_array = ealloc1float (ntm1);

   rewind (stdin);
   zero = 0.0;
   one = 1.0;
   four = 4.0;
   signx = signy = 1;
   ishift = nint ( delrt / dt );
   tlast = (ns-1) * dt;

   if ( verbose ) fprintf ( stderr, "Ishift in samples = %d, tlast = %f\n", ishift, tlast );

   for ( i = 0; i < ntr; ++i ) {
      gettr (&tr);
      x = tr.sx / scale_factor;
      y = tr.sy / scale_factor;

      if ( verbose == 2 ) fprintf ( stderr, "trace = %5d, X = %.2f, Y = %.2f\n", i, x, y ); 

      xref = ( x - xmin ) / delta_x;
      yref = ( y - ymin ) / delta_y;

      iloc = min ( max ( nint ( xref ), 1 ), nx-2 );
      jloc = min ( max ( nint ( yref ), 1 ), ny-2 );

      r_iloc = iloc;
      r_jloc = jloc;

      remainder_x = xref - r_iloc;
      remainder_y = yref - r_jloc;

      if ( verbose == 2 ) fprintf ( stderr, "iloc = %5d, jloc = %5d, remainder_x = %.4f, remainder_y = %.4f\n", iloc, jloc, remainder_x, remainder_y ); 

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
         dv_array[k] = min ( max ( dv_array[k], dvmin ), dvmax );
         if ( verbose == 2 ) fprintf ( stderr, "TWT_checkshot = %.2f, DV_checkshot = %.4f\n", twt_dv_array[k], dv_array[k] ); 
      }
      for ( j = ishift; j < ns; ++j ) {
         time = min ( max ( twt_array[j] - delrt, zero ), tlast );
         intlin ( nt, twt_dv_array, dv_array, dv_array[0], dv_array[ntm1], 1, &time, &dv_interp);
         tr.data[j] += dv_interp;
      }
      puttr ( &tr ); 
   }

   free1float (twt_array);
   free1float (vavg_array);

   free1double (x_dv); 
   free1double (y_dv); 
   free3double (dv); 

   return EXIT_SUCCESS;
}
