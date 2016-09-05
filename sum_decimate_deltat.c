#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

int main (int argc, char **argv) {

   char  temp[256];
   int   kount, nx, ny, nt;
   short verbose;
   double x, y, t, dv;
   double dx, dy, dt;
   double xmin, xmax, ymin, ymax, tmin, tmax;
   double x0, x1, y0, y1;

   int iloc, jloc, kloc, one, zero;
   int ***num;
   double value;
   double ***x_array, ***y_array, ***t_array, ***dv_array;
   register int i, j, k;

   initargs(argc, argv);
   requestdoc (0);

   if (!getparshort("verbose", &verbose)) verbose = 0;
   if (!getpardouble("dt", &dt)) dt = 2.0;
   if (!getpardouble("dx", &dx)) dx = 25.0;
   if (!getpardouble("dy", &dy)) dy = 25.0;

   x0 = 511797.36;
   x1 = 527792.88;
   y0 = 152443.84;
   y1 = 169653.51;

   kount = 0;
   xmin = ymin = tmin = DBL_MAX;
   xmax = ymax = tmax = DBL_MIN;
   while (((char *) NULL) != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x, &y, &t, &dv );
      xmin = min ( xmin, x ); 
      ymin = min ( ymin, y ); 
      tmin = min ( tmin, t ); 
      xmax = max ( xmax, x ); 
      ymax = max ( ymax, y ); 
      tmax = max ( tmax, t ); 
      ++kount;
   }

   nx = nint ( ( xmax - xmin ) / dx ) + 1;
   ny = nint ( ( ymax - ymin ) / dy ) + 1;
   nt = nint ( ( tmax - tmin ) / dt ) + 1;

   if ( verbose ) {
      fprintf ( stderr, "Number of input elements = %5d, xmin = %.2f, xmax = %.2f, ymin = %.2f, ymax = %.2f, tmin = %.2f, tmax = %.2f\n", kount+1, xmin, xmax, ymin, ymax, tmin, tmax );
      fprintf ( stderr, "dx = %.2f, dy = %.2f, dt = %.2f, nx = %5d, ny = %5d, nt = %5d\n", dx, dy, dt, nx, ny, nt );
   }

   num      = ealloc3int    ( nt, ny, nx );
   x_array  = ealloc3double ( nt, ny, nx );
   y_array  = ealloc3double ( nt, ny, nx );
   t_array  = ealloc3double ( nt, ny, nx );
   dv_array = ealloc3double ( nt, ny, nx );

   for ( i = 0; i < nx; ++i ) for ( j = 0; j < ny; ++j ) for ( k = 0; k < nt; ++k ) num[i][j][k] = 0;

   one = 1;
   zero = 0;
   rewind ( stdin );
   while (((char *) NULL) != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x, &y, &t, &dv );
      iloc = min ( max ( nint ( ( x - xmin ) / dx ), 0 ), nx );
      jloc = min ( max ( nint ( ( y - ymin ) / dx ), 0 ), ny );
      kloc = min ( max ( nint ( ( t - tmin ) / dt ), 0 ), nt );
      num[iloc][jloc][kloc]      += one;
      x_array[iloc][jloc][kloc]  += x;
      y_array[iloc][jloc][kloc]  += y;
      t_array[iloc][jloc][kloc]  += t;
      dv_array[iloc][jloc][kloc] += dv;
   }

   for ( i = 0; i < nx; ++i ) {
      for ( j = 0; j < ny; ++j ) {
         for ( k = 0; k < nt; ++k ) {
            if ( num[i][j][k] > zero ) {
               value = num[i][j][k];
               x  = x_array[i][j][k]  / value;
               y  = y_array[i][j][k]  / value;
               t  = t_array[i][j][k]  / value;
               dv = dv_array[i][j][k] / value;
               printf ( "%-.2f %.2f %.2f %.4f\n", x, y, t, dv );
            }
         }
      }
   }

   tmin = nint ( tmin );
   tmax = nint ( tmax );

   printf ( "%-.2f %.2f %.2f %.4f\n", x0, y0, tmin, 0.0);
   printf ( "%-.2f %.2f %.2f %.4f\n", x0, y1, tmin, 0.0);
   printf ( "%-.2f %.2f %.2f %.4f\n", x1, y0, tmin, 0.0);
   printf ( "%-.2f %.2f %.2f %.4f\n", x1, y1, tmin, 0.0);

   printf ( "%-.2f %.2f %.2f %.4f\n", x0, y0, tmax, 0.0);
   printf ( "%-.2f %.2f %.2f %.4f\n", x0, y1, tmax, 0.0);
   printf ( "%-.2f %.2f %.2f %.4f\n", x1, y0, tmax, 0.0);
   printf ( "%-.2f %.2f %.2f %.4f\n", x1, y1, tmax, 0.0);

   free3int    ( num );
   free3double ( x_array );
   free3double ( y_array );
   free3double ( t_array );
   free3double ( dv_array );

   return EXIT_SUCCESS;
}
