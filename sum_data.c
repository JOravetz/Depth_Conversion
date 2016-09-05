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
   int   kount, nx, ny;
   short verbose;
   double x, y, dv;
   double dx, dy;
   double xmin, xmax, ymin, ymax;
   double x0, x1, y0, y1;

   int iloc, jloc, one, zero;
   int **num;
   double value;
   double **x_array, **y_array, **dv_array;
   register int i, j;

   initargs(argc, argv);
   requestdoc (0);

   if (!getparshort("verbose" , &verbose)) verbose = 0;

   dx = dy = 12.50;

   x0 = 511797.36;
   x1 = 527792.88;
   y0 = 152443.84;
   y1 = 169653.51;

   kount = 0;
   xmin = ymin = DBL_MAX;
   xmax = ymax = DBL_MIN;
   while (((char *) NULL) != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf", &x, &y, &dv );
      xmin = min ( xmin, x ); 
      ymin = min ( ymin, y ); 
      xmax = max ( xmax, x ); 
      ymax = max ( ymax, y ); 
      ++kount;
   }

   nx = nint ( ( xmax - xmin ) / dx ) + 1;
   ny = nint ( ( ymax - ymin ) / dy ) + 1;

   if ( verbose ) {
      fprintf ( stderr, "Number of input elements = %5d, xmin = %.2f, xmax = %.2f, ymin = %.2f, ymax = %.2f\n", kount+1, xmin, xmax, ymin, ymax );
      fprintf ( stderr, "dx = %.2f, dy = %.2f, nx = %5d, ny = %5d\n", dx, dy, nx, ny );
   }

   num      = ealloc2int    ( ny, nx );
   x_array  = ealloc2double ( ny, nx );
   y_array  = ealloc2double ( ny, nx );
   dv_array = ealloc2double ( ny, nx );

   for ( i = 0; i < nx; ++i ) for ( j = 0; j < ny; ++j ) num[i][j] = 0;

   one = 1;
   zero = 0;
   rewind ( stdin );
   while (((char *) NULL) != fgets ( temp, sizeof(temp), stdin )) {
      (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf", &x, &y, &dv );
      iloc = min ( max ( nint ( ( x - xmin ) / dx ), 0 ), nx );
      jloc = min ( max ( nint ( ( y - ymin ) / dx ), 0 ), ny );
      num[iloc][jloc]      += one;
      x_array[iloc][jloc]  += x;
      y_array[iloc][jloc]  += y;
      dv_array[iloc][jloc] += dv;
   }

   for ( i = 0; i < nx; ++i ) {
      for ( j = 0; j < ny; ++j ) {
         if ( num[i][j] > zero ) {
            value = num[i][j];
            x  = x_array[i][j]  / value;
            y  = y_array[i][j]  / value;
            dv = dv_array[i][j] / value;
            printf ( "%-.2f %.2f %.4f\n", x, y, dv );
         }
      }
   }

   printf ( "%-.2f %.2f %.4f\n", x0, y0, 0.0);
   printf ( "%-.2f %.2f %.4f\n", x0, y1, 0.0);
   printf ( "%-.2f %.2f %.4f\n", x1, y0, 0.0);
   printf ( "%-.2f %.2f %.4f\n", x1, y1, 0.0);

   free2int    ( num );
   free2double ( x_array );
   free2double ( y_array );
   free2double ( dv_array );

   return EXIT_SUCCESS;
}
