#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"

#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

char *sdoc[] = {NULL};

segy tr;

int main (int argc, char **argv) {

   char well[40], temp[256];
   int imin, ntr, ns, kount, index;
   float factor1, seismic_depth;
   float dt, scale_factor, seismic_vavg, delta_v, delta_z;
   float *x, *y, **data;
   double min_dist, dist, toler;
   double delrt, factor, x_loc, y_loc, twt, depth, average_velocity;
   FILE *fpp;
   cwp_String pfile;
   short verbose;
   register int i, j;

   initargs(argc, argv);

   if (!getparshort("verbose" , &verbose)) verbose = 0;
   if (!getparstring("pfile",&pfile)) pfile = "tops.lis";
   if (!getpardouble("toler",&toler)) toler = 12.50;
  
   fpp = efopen (pfile, "r");

   kount = 0;
   while (NULL != fgets ( temp, sizeof(temp), fpp )) {
      ++kount;
      (void) sscanf ( ((&(temp[0]))), "%s%lf%lf%lf%lf", well, &x_loc, &y_loc, &twt, &depth );
   }

   ntr = gettra (&tr, 0);
   ns  = tr.ns;
   dt  = tr.dt * 0.001;
   scale_factor = tr.scalco;
   delrt = tr.delrt;
   imin = nint ( delrt / dt );
   if (scale_factor < 0.0 ) scale_factor *= -1.0;
   if (scale_factor == 0.0 ) scale_factor = 1.0;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Number of traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "Time sample rate (milliseconds) = %f\n", dt );
      fprintf ( stderr, "Coordinate scale factor = %f\n", scale_factor );
      fprintf ( stderr, "TOPS file name = %s, number of input samples = %d\n", pfile, kount);
      fprintf ( stderr, "Minimum distance tolerance = %8.2f meters\n", toler );
      fprintf ( stderr, "Delrt = %f, imin = %d\n", delrt, imin );
      fprintf ( stderr, "\n" );
   }

   x    = ealloc1float ( ntr );
   y    = ealloc1float ( ntr );
   data = ealloc2float ( ns, ntr );

   factor = 2000.0;
   factor1 = 0.0005;

   int nsm1;
   
   nsm1 = ns - 1;

   rewind (stdin);
   for ( i = 0; i < ntr; ++i ) {
      gettr (&tr);
      x[i] = tr.sx / scale_factor;
      y[i] = tr.sy / scale_factor;
      for ( j = 0; j < ns; ++j ) data[i][j] = tr.data[j];
   }

   rewind ( fpp );
   printf ( "well x_loc y_loc twt well_depth seismic_depth seismic_vavg average_velocity delta_z delta_v\n" );
   for ( i = 0; i < kount; ++i ) {
      fgets ( temp, sizeof(temp), fpp );
      (void) sscanf ( ((&(temp[0]))), "%s%lf%lf%lf%lf", well, &x_loc, &y_loc, &twt, &depth );
      average_velocity = ( depth / twt ) * factor;

      min_dist = DBL_MAX;
      index = -1;
      for ( j = 0; j < ntr; ++j ) {
         dist = sqrt ( pow ( x_loc - x[j], 2.0 ) + pow ( y_loc - y[j], 2.0 ) );
         if ( dist < min_dist && dist <= toler ) {
            min_dist = dist;
            index = j;
         }
      }

      if ( index >= 0 ) {
         if ( verbose ) fprintf ( stderr, "index = %d, x_loc = %f, x_seismic = %f, y_loc = %f, y_seismic = %f, min_dist = %f\n", index, x_loc, x[index], y_loc, y[index], min_dist ); 
         j = min ( max ( nint ( twt / dt ) - imin, 0 ), nsm1 );
         if ( verbose ) fprintf ( stderr, "sample = %d\n", j );
         seismic_vavg = data[index][j];
         delta_v = average_velocity - seismic_vavg;
         seismic_depth = seismic_vavg * twt * factor1;
         delta_z = depth - seismic_depth;
         printf ( "%-16s %10.2f %10.2f %8.2f %8.2f %8.2f %8.2f %8.2f %12.6f %12.6f\n", well, x_loc, y_loc, twt, depth, seismic_depth, seismic_vavg, average_velocity, delta_z, delta_v );
      }

      if ( verbose && ( index >= 0 ) ) {
         fprintf ( stderr, "num = %5d, Well = %-10s x_loc = %10.2f, y_loc = %10.2f, twt = %8.2f, depth = %8.2f\n", i, well, x_loc, y_loc, twt, depth );
         fprintf ( stderr, "min_dist = %8.2f, index = %5d, seismic velocity = %8.2f, well average velocity = %8.2f, delta_z = %10.4f meters, delta_v = %10.4f meters per second\n", 
            min_dist, index, seismic_vavg, average_velocity, delta_z, delta_v );
      }
   }

   efclose ( fpp );

   free1float (x);
   free1float (y);
   free2float (data);

   return EXIT_SUCCESS;
}
