#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

int main (int argc, char **argv) {

   FILE *fpp;
   char  well[40], top[40], temp[256];
   cwp_String pfile;
   int   index, kount;
   short verbose;
   double x, y, t, dv;
   double x_loc, y_loc;
   double toler, dist, dist_min;
   double *X, *Y, *T, *DV;
   register int i;

   initargs(argc, argv);
   requestdoc (0);

   if (!getparshort("verbose", &verbose)) verbose = 0;
   if (!getpardouble("toler", &toler)) toler = 12.50;
   if (!getparstring("pfile",&pfile)) pfile = "test3.deltav.smooth.reform.lis";

   if ( verbose ) {
      fprintf ( stderr, "Well-Marker Pick file name containing X and Y values = %s\n", pfile );
      fprintf ( stderr, "Location accuracy (tolerance) = %f meters\n", toler );
   }

   fpp = efopen (pfile, "r");

   rewind ( stdin );
   kount = 0;
   while (((char *) NULL) != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x, &y, &t, &dv );
      ++kount;
   }

   X  = ealloc1double ( kount );
   Y  = ealloc1double ( kount );
   T  = ealloc1double ( kount );
   DV = ealloc1double ( kount );

   rewind ( stdin );
   for ( i = 0; i < kount; ++i ) {
      fgets ( temp, sizeof(temp), stdin );
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x, &y, &t, &dv );
      X[i] = x;
      Y[i] = y;
      T[i] = t;
      DV[i] = dv;
   }

   fpp = efopen (pfile, "r");

   rewind ( fpp );
   while (((char *) NULL) != fgets ( temp, sizeof(temp), fpp )) {
      (void) sscanf ( ((&(temp[0]))), "%s%s%lf%lf", well, top, &x_loc, &y_loc );
      dist = dist_min = DBL_MAX;
      index = -1;   
      for ( i = 0; i < kount; ++i ) {
         dist = sqrt ( pow ( x_loc - X[i], 2.0 ) + pow ( y_loc - Y[i], 2.0 ) );
         if ( dist < dist_min ) {
            dist_min = dist;
            index = i;
         }
      }
      if ( index > -1 && dist_min <= toler ) {
         printf ( "%-10.2f %10.2f %12.6f %12.8f\n", X[index], Y[index], T[index], DV[index] );
         if ( verbose ) fprintf ( stderr, "Well = %-10s Top = %-10s Dist_MIN = %8.4f %10.2f %10.2f %12.6f %12.8f\n", well, top, dist_min, X[index], Y[index], T[index], DV[index] );
      }
   }

   efclose ( fpp );

   free1double ( X );
   free1double ( Y );
   free1double ( T );
   free1double ( DV );

   return EXIT_SUCCESS;
}
