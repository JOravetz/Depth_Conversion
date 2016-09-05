#include <omp.h>
#include <stdio.h>
#include <float.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"
#include "header.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define abs(x)   ( (x) <  0  ? -(x) : (x) )
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

#define CHUNKSIZE 1

char *sdoc[] = {NULL};

segy tr;

int main (int argc, char **argv) {

   char temp[256];
   FILE *fpp, *fpo, *fpi;
   cwp_String pfile;
   short verbose;
   register int i, j, k;

   int ntr, ns, imin, nx, ny;
   float ***hdr, ***data;
   double delrt, scale_factor, dt;

   initargs(argc, argv);

   if (!getparshort("verbose" , &verbose)) verbose = 0;
   if (!getparstring("pfile",&pfile)) pfile = "test3.deltav.smooth.reform.lis";
   if (!getparint("nx",&nx)) nx = 901;
   if (!getparint("ny",&ny)) ny = 1101;

   /* read in background average velocity data - compute limits and ranges */
   ntr = gettra (&tr,  0);
   delrt = tr.delrt;
   if (!getparint("ns",  &ns)) ns = tr.ns;
   scale_factor = tr.scalco;
   if (scale_factor < 0.0 )  scale_factor *= -1.0;
   if (scale_factor == 0.0 ) scale_factor = 1.0;

   dt  = tr.dt  * 0.001;
   imin = nint ( delrt / dt );

   hdr  = ealloc3float ( HDRBYTES, ny, nx );
   data = ealloc3float ( ns, ny, nx );

   float x, y;
   float xmin_seis, ymin_seis, xmax_seis, ymax_seis;
   float **x_array, **y_array;
   double xi_first, yi_first, xj_first, yj_first;
   double avg_dist_dx, avg_dist_dy;

   x_array = ealloc2float ( ny, nx );
   y_array = ealloc2float ( ny, nx );

   xmin_seis = ymin_seis = FLT_MAX;
   xmax_seis = ymax_seis = FLT_MIN;

   xi_first = yi_first = xj_first = yj_first = avg_dist_dx = avg_dist_dy = 0.0;

   rewind ( stdin );
   for ( i = 0; i < nx; ++i ) {
      for ( j = 0; j < ny; ++j ) {
         gettr (&tr);
         x_array[i][j] = x = tr.sx / scale_factor;
         y_array[i][j] = y = tr.sy / scale_factor;

         if ( i == 0 && j == 0 ) {
            xi_first = x;
            yi_first = y;
         } else if ( j == 0 ) {
            avg_dist_dx += sqrt ( pow ( x - xi_first, 2.0 ) + pow ( y - yi_first, 2.0 ) );
            xi_first = x;
            yi_first = y;
         }

         if ( j == 0 ) {
            xj_first = x;
            yj_first = y;
         } else {
            avg_dist_dy += sqrt ( pow ( x - xj_first, 2.0 ) + pow ( y - yj_first, 2.0 ) );
            xj_first = x;
            yj_first = y;
         }

         xmin_seis = min ( x, xmin_seis );
         xmax_seis = max ( x, xmax_seis );
         ymin_seis = min ( y, ymin_seis );
         ymax_seis = max ( y, ymax_seis );

         for ( k = 0; k < ns; ++k ) data[i][j][k] = tr.data[k];
         memcpy( (void *) &hdr[i][j][0], (const void *) &tr,  HDRBYTES );
      }
   }

   avg_dist_dx /= ( nx - 1 );
   avg_dist_dy /= ( ny - 1 ) * nx;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Number of input traces = %d\n", ntr );
      fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
      fprintf ( stderr, "Sample rate = %f ms.\n", dt );
      fprintf ( stderr, "Delay = %f ms., First sample = %d\n", delrt, imin );
      fprintf ( stderr, "Scale Factor for X and Y Coordinates = %f\n", scale_factor );
      fprintf ( stderr, "Number of inlines (nx) = %5d, Number of crosslines (ny) = %5d\n", nx, ny );
      fprintf ( stderr, "Xmin_seis = %.2f, Xmax_seis = %.2f, Ymin_seis = %.2f, Ymax_seis = %.2f\n", xmin_seis, xmax_seis, ymin_seis, ymax_seis );
      fprintf ( stderr, "Average Dist DX = %10.4f, Average Dist DY = %10.4f\n", avg_dist_dx, avg_dist_dy );
      fprintf ( stderr, "\n" );
   }

   int kount, iloc, jloc, kloc;
   double x_loc, y_loc, twt, delta_v;
   double xmin, xmax, ymin, ymax, tmin, tmax;
   double dist_min, dist;
   int ***num_dv, **ilive;
   float ***data_dv;

   ilive   = ealloc2int   ( ny, nx );
   num_dv  = ealloc3int   ( ns, ny, nx );
   data_dv = ealloc3float ( ns, ny, nx );
   
   /* read in residual average velocity corrections */
   fpp = efopen (pfile, "r");

   xmin = ymin = tmin = DBL_MAX;
   xmax = ymax = tmax = DBL_MIN;

   for ( i = 0; i < nx; ++i ) {
      for ( j = 0; j < ny; ++j ) {
         ilive[i][j] = 0;
         for ( k = 0; k < ns; ++k ) {
            num_dv[i][j][k] = 0;
            data_dv[i][j][k] = 0.0;
         }
      }
   }

   kloc = 0;

   kount = -1;
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

   double *x_loc_array, *y_loc_array, *twt_array, *delta_v_array;

   x_loc_array   = ealloc1double ( kount+1 );
   y_loc_array   = ealloc1double ( kount+1 );
   twt_array     = ealloc1double ( kount+1 );
   delta_v_array = ealloc1double ( kount+1 );

   rewind ( fpp );
   for ( i = 0; i <= kount; ++i ) {
      fgets ( temp, sizeof(temp), fpp );
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x_loc, &y_loc, &twt, &delta_v);
       x_loc_array[i]   = x_loc;
       y_loc_array[i]   = y_loc;
       twt_array[i]     = twt;
       delta_v_array[i] = delta_v;
   }
   efclose ( fpp );

   #pragma omp parallel private (i,j,k,dist_min,dist,iloc,jloc,kloc)
{

   #pragma omp for schedule(dynamic, CHUNKSIZE)
   for ( k = 0; k <= kount; ++k ) {
      iloc = jloc = 0;
      dist_min = dist = FLT_MAX;
      kloc = min ( max ( nint ( twt_array[k] / dt ), 0 ), ns );
      for ( i = 0; i < nx; ++i ) {
         for ( j = 0; j < ny; ++j ) {
            dist = sqrt ( pow ( x_loc_array[k] - x_array[i][j], 2.0 ) + pow ( y_loc_array[k] - y_array[i][j], 2.0 ) );
            if ( dist < dist_min ) {
               iloc = i;
               jloc = j;
               dist_min = dist;
            }
         }
      }
      ilive[iloc][jloc] += 1;
      num_dv[iloc][jloc][kloc] += 1;
      data_dv[iloc][jloc][kloc] += delta_v_array[k];

      if ( verbose > 1 ) { 
         fprintf ( stderr, "Finished reading delta_v sample = %5d, iloc = %5d, jloc = %5d, kloc = %5d, dist_min = %.4f, x_loc = %.2f, x_array = %.2f, y_loc = %.2f, y_array = %.2f, delta_v = %10.6f\n",
         k, iloc, jloc, kloc, dist_min, x_loc_array[k], x_array[iloc][jloc], y_loc_array[k], y_array[iloc][jloc], delta_v_array[k] );
      }

} /* end of parallel code */

   }

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Delta-V file name = %s, number of input samples = %d\n", pfile, kount+1);
      fprintf ( stderr, "Xmin = %.2f, Xmax = %.2f, Ymin = %.2f, Ymax = %.2f, Tmin = %.2f, Tmax = %.2f\n", xmin, xmax, ymin, ymax, tmin, tmax );
      fprintf ( stderr, "\n" );
   }

   /* normalize the delta_v array values by number of hits per sample */
   for ( i = 0; i < nx; ++i ) for ( j = 0; j < ny; ++j ) if ( ilive[i][j] > 0 ) for ( k = 0; k < ns; ++k ) if ( num_dv[i][j][k] > 0 ) data_dv[i][j][k] /= num_dv[i][j][k];

   int num, nump1;
   double rk, dv_interp, zero;
   double *tindex, *dv_vertical;

   dv_vertical = ealloc1double ( ns );
   tindex = ealloc1double ( ns );

   /* all the data read in - now let's loop thru the populated delta_v corrections and interpolate vertically in time */
   zero = 0.0;
   for ( i = 0; i < nx; ++i ) {
      for ( j = 0; j < ny; ++j ) {
         num = -1;
         for ( k = 0; k < ns; ++k ) tindex[k] = dv_vertical[k] = zero;
         for ( k = 0; k < ns; ++k ) {
            if ( num_dv[i][j][k] > 0 ) {
               ++num;
               tindex[num] = k;
               dv_vertical[num] = data_dv[i][j][k];
               if ( verbose ) fprintf ( stderr, "TWT index = %5d, X index = %5d, Y index = %5d, Delta_V = %10.6f\n", k, i, j, data_dv[i][j][k] );
            }
         }
         nump1 = num + 1;
         if ( nump1 > 0 ) {
            if ( verbose ) fprintf ( stderr, "number of DV samples = %5d\n", nump1 );
            for ( k = 0; k < ns; ++k ) {
               rk = k;
               dintlin ( nump1, tindex, dv_vertical, dv_vertical[0], dv_vertical[num], 1, &rk, &dv_interp );
               data[i][j][k] += dv_interp;
            }
         }
         memcpy( (void *) &tr, (const void *) &hdr[i][j][0], HDRBYTES);
         for ( k = 0; k < ns; ++k ) tr.data[k] = data[i][j][k];
         puttr ( &tr );
      }
   }

   /*
               ++num;
               if ( num == 0 ) fpo = efopen ( "input.dat", "w" );
               sum += data_dv[i][j][k];
               fprintf ( stderr, "TWT index = %5d, X index = %5d, Y index = %5d, Delta_V = %10.6f\n", k, i, j, data_dv[i][j][k] );
               fprintf ( fpo, "%d %d %f\n", i, j, data_dv[i][j][k] );
            }
         }
      }
      if ( num > -1 ) {
         sum /= (num+1);
         fprintf (stderr, "Number of DV corrections = %5d, average = %10.6f\n", num+1, sum );
         fprintf ( fpo, "%d %d %f\n", 0, 0, sum );
         fprintf ( fpo, "%d %d %f\n", 0, ny-1, sum );
         fprintf ( fpo, "%d %d %f\n", nx-1, 0, sum );
         fprintf ( fpo, "%d %d %f\n", nx-1, ny-1, sum );
         efclose ( fpo );
         system ("python rbf_interpolate.py > output.dat"); 
         fpi = efopen ( "output.dat", "r" );
         for ( i = 0; i < nx; ++i ) {
            for ( j = 0; j < ny; ++j ) {
               fgets ( temp, sizeof(temp), fpi );
               (void) sscanf ( ((&(temp[0]))), "%lf", &delta_v);
               dv_interp[i][j] = delta_v;
               data[i][j][k] += delta_v;
            }
         }
         efclose ( fpi );
      }
   }

   for ( i = 0; i < nx; ++i ) {
      for ( j = 0; j < ny; ++j ) {
         memcpy( (void *) &tr, (const void *) &hdr[i][j][0], HDRBYTES);
         for ( k = 0; k < ns; ++k ) tr.data[k] = data[i][j][k];
         puttr ( &tr );
      }
   }

   */
         
   free2int   ( ilive );
   free3int   ( num_dv );

   free2float ( x_array );
   free2float ( y_array );

   free3float ( hdr );
   free3float ( data );
   free3float ( data_dv );

   free1double ( x_loc_array );
   free1double ( y_loc_array );
   free1double ( twt_array );
   free1double ( delta_v_array );
   free1double ( tindex );
   free1double ( dv_vertical );


   return EXIT_SUCCESS;
}
