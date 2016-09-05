#include <stdio.h>
#include <stddef.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"

#define abs(x)   ( (x) <  0  ? -(x) : (x) )
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

segy tr;

int main (int argc, char **argv) {

char temp[256];
FILE *fpp;
cwp_String pfile;

int iloc, jloc, inline_num, xline_num;
int gelev_min, gelev_max, selev_min, selev_max;
int **trid;
double sx_min, sx_max, sy_min, sy_max;
double ***data, **x_array, **y_array;

int kount, imin, ns, ntr;
double x, y, dt, dt_orig, factor, delrt;
double *il_array, *xl_array;
short interp, verbose;
register int i, j, k;

initargs(argc, argv);
requestdoc (0);

if (!getparstring("pfile",&pfile)) pfile = "coords.lis";
if (!getparshort("interp", &interp)) interp = 1;
if (!getparshort("verbose", &verbose)) verbose = 0;

fpp = efopen ( pfile, "r" );

ntr = gettra (&tr,  0);
delrt = tr.delrt;
if (!getparint("ns",  &ns)) ns = tr.ns;

dt_orig = tr.dt;
dt  = dt_orig * 0.001;
imin = nint ( delrt / dt );

factor = 0.0005;

kount = 0;
while (NULL != fgets ( temp, sizeof(temp), fpp )) {
   ++kount;
   (void) sscanf ( ((&(temp[0]))), "%lf%lf", &x, &y);
}

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "Delay = %f ms., First sample = %d\n", delrt, imin );
   fprintf ( stderr, "Traverse coordinates file name = %s, number of values in file = %5d\n", pfile, kount );
   fprintf ( stderr, "\n" );
}

il_array = ealloc1double ( kount );
xl_array = ealloc1double ( kount );

rewind ( fpp );

for ( i = 0; i < kount ; ++i ) {
   fgets ( temp, sizeof(temp), fpp );
   (void) sscanf ( ((&(temp[0]))), "%lf%lf", &x, &y);
   il_array[i] = x;
   xl_array[i] = y;
   if ( verbose ) fprintf ( stderr, "kount = %5d, INLINE = %.2f, XLINE = %.2f\n", i+1, il_array[i], xl_array[i] );
}

efclose (fpp);

sx_min = sy_min = DBL_MAX;
sx_max = sy_max = DBL_MIN;
gelev_min = selev_min = INT_MAX;
gelev_max = selev_max = INT_MIN;
inline_num = xline_num = 0;
rewind ( stdin );
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   gelev_min = min ( gelev_min, tr.gelev );
   gelev_max = max ( gelev_max, tr.gelev );
   selev_min = min ( selev_min, tr.selev );
   selev_max = max ( selev_max, tr.selev );
   sx_min = min ( sx_min, tr.sx );
   sx_max = max ( sx_max, tr.sx );
   sy_min = min ( sy_min, tr.sy );
   sy_max = max ( sy_max, tr.sy );
} 

inline_num = gelev_max - gelev_min + 1;
xline_num  = selev_max - selev_min + 1;

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "SEISMIC DATA - sx_min = %.2f, sx_max = %.2f, sy_min = %.2f, sy_max = %.2f\n", sx_min, sx_max, sy_min, sy_max );
   fprintf ( stderr, "Inline min = %5d, Inline max = %5d, Xline min = %5d, Xline max = %5d\n", gelev_min, gelev_max, selev_min, selev_max );
   fprintf ( stderr, "Number of Inlines = %5d, Number of Xlines = %5d\n", inline_num, xline_num );
   fprintf ( stderr, "\n" );
}

trid    = ealloc2int    ( xline_num, inline_num );
data    = ealloc3double ( ns, xline_num, inline_num );
x_array = ealloc2double ( xline_num, inline_num );
y_array = ealloc2double ( xline_num, inline_num );

for ( i = 0; i < inline_num; ++i ) for ( j = 0; j < xline_num; ++j ) trid[i][j] = 0;

rewind ( stdin );
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   iloc = nint ( tr.gelev - gelev_min );
   jloc = nint ( tr.selev - selev_min );
   x_array[iloc][jloc] = tr.sx;
   y_array[iloc][jloc] = tr.sy;
   trid[iloc][jloc] = 1;
   for ( j = 0; j < ns; ++j ) data[iloc][jloc][j] = tr.data[j];
} 

int iy, niy, istart, ii, jj, kk;
double dx, dy, diff, remain, one, zero;
double slope;

istart = 0;
zero = 0.0;
one = 1.0;
diff = remain = zero;
for ( i = 0; i < kount-1; ++i ) {
   dx = il_array[i+1] - il_array[i];
   dy = xl_array[i+1] - xl_array[i];
   slope = dy/dx;
   iloc = nint ( abs ( dx ) );
   if ( verbose ) fprintf ( stderr, "i = %5d, inline_start = %8.2f, inline_stop = %8.2f, nx = %8.0f, xline_start = %8.2f, xline_stop = %8.2f, ny = %8.0f, slope = %12.4f\n", 
   i, il_array[i], il_array[i+1], dx, xl_array[i], xl_array[i+1], dy, slope );
   if ( i > 0 ) istart = 1;
   for ( j = istart; j <= iloc; ++j ) {
      jloc = j;
      if ( dx < zero ) jloc *= -1;
      y = ( slope * jloc ) + xl_array[i]; 
      iy = (int) y;
      niy = nint ( y );
      diff = y - iy;
      remain = one - diff;
      if ( verbose ) fprintf ( stderr, "   i = %5d, j = %5d, inline = %8.2f, xline = %8.2f, int = %5d, nint = %5d, diff = %10.4f, remain=%10.4f\n", i, jloc, il_array[i]+jloc, y, iy, niy, diff, remain );
      tr.gelev = nint ( ( il_array[i]+jloc ) );
      ii = tr.gelev - gelev_min;
      tr.selev = nint ( y );
      jj = iy - selev_min;
      kk = niy - selev_min;
      if ( interp ) {
         for ( k = 0; k < ns; ++k ) tr.data[k] = ( data[ii][jj][k] * diff + data[ii][kk][k] * remain ); 
         tr.sy = tr.gy = ( y_array[ii][jj] * diff + y_array[ii][kk] * remain );
      } else {
         for ( k = 0; k < ns; ++k ) tr.data[k] = data[ii][jj][k]; 
         tr.sy = tr.gy = y_array[ii][jj];
      }
      tr.trid = 1;
      tr.sx = tr.gx = x_array[ii][jj];
      tr.ns = ns;
      tr.dt = dt_orig;
      puttr ( &tr ); 
   } 
}

free2int    (trid);
free2double (x_array);
free2double (y_array);
free3double (data);
free1double (il_array);
free1double (xl_array); 

return EXIT_SUCCESS;

}

