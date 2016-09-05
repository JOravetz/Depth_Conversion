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
double x, y, dt, dt_orig, factor, delrt, twt, vavg, scale_factor;
double *cx_array, *cy_array, *twt_array, *vavg_array;
short verbose;
register int i, j;

initargs(argc, argv);
requestdoc (0);

if (!getparstring("pfile",&pfile)) pfile = "combined.reform.dat";
if (!getparshort("verbose", &verbose)) verbose = 0;

fpp = efopen ( pfile, "r" );

ntr = gettra (&tr,  0);
delrt = tr.delrt;
if (!getparint("ns",  &ns)) ns = tr.ns;

dt_orig = tr.dt;
dt  = dt_orig * 0.001;
imin = nint ( delrt / dt );
scale_factor = tr.scalco;
if (scale_factor < 0.0 ) scale_factor *= -1.0;
if (scale_factor == 0.0 ) scale_factor = 1.0;

factor = 0.0005;

kount = 0;
while (NULL != fgets ( temp, sizeof(temp), fpp )) {
   ++kount;
   (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x, &y, &twt, &vavg );
}

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "Delay = %f ms., First sample = %d\n", delrt, imin );
   fprintf ( stderr, "Combined checkshot file name = %s, number of values in file = %5d\n", pfile, kount );
   fprintf ( stderr, "\n" );
}

cx_array   = ealloc1double ( kount );
cy_array   = ealloc1double ( kount );
twt_array  = ealloc1double ( kount );
vavg_array = ealloc1double ( kount );

rewind ( fpp );

for ( i = 0; i < kount ; ++i ) {
   fgets ( temp, sizeof(temp), fpp );
   (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf%lf", &x, &y, &twt, &vavg );
   cx_array[i]   = x;
   cy_array[i]   = y;
   twt_array[i]  = twt;
   vavg_array[i] = vavg;
   if ( verbose ) fprintf ( stderr, "kount = %5d, X = %10.2f, Y = %10.2f, TWT = %8.2f, VAVG = %8.2f\n", i+1, cx_array[i], cy_array[i], twt_array[i], vavg_array[i] );
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
   fprintf ( stderr, "VELOCITY DATA - sx_min = %.2f, sx_max = %.2f, sy_min = %.2f, sy_max = %.2f\n", sx_min, sx_max, sy_min, sy_max );
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
iloc = jloc = 0;
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   iloc = nint ( tr.gelev - gelev_min );
   jloc = nint ( tr.selev - selev_min );
   x_array[iloc][jloc] = tr.sx / scale_factor;
   y_array[iloc][jloc] = tr.sy / scale_factor;
   trid[iloc][jloc] = 1;
   for ( j = 0; j < ns; ++j ) data[iloc][jloc][j] = tr.data[j];
} 

int isamp;
double dist, dist_min, dx, dy, CX, CY, dv;
register int k;

dx = dy = 0.0;
for ( k = 0; k < kount ; ++k ) {
   dist_min = DBL_MAX;
   CX = cx_array[k];
   CY = cy_array[k];
   isamp = nint ( twt_array[k] / dt );
   for ( i = 0; i < inline_num ; ++i ) {
      for ( j = 0; j < xline_num ; ++j ) {
         dist = sqrt ( pow ( x_array[i][j] - CX, 2 ) + pow ( y_array[i][j] - CY, 2 ) );
         if ( dist < dist_min ) {
            iloc = i;
            jloc = j;
            dx = CX - x_array[i][j];
            dy = CY - y_array[i][j];
            dist_min = dist;
         }
      }
   }
   dv = vavg_array[k] - data[iloc][jloc][isamp];
   if ( verbose ) {
      fprintf ( stderr, "Checkshot num = %5d, CX = %10.2f, X = %10.2f, ILOC = %5d, CY = %10.2f, Y = %10.2f, JLOC = %5d, DIST_MIN = %6.2f, DX = %6.2f, DY = %6.2f, Vavg_Seismic = %8.2f, Vavg_Checkshot = %8.2f, Diff = %8.4f\n", 
      k+1, CX, x_array[iloc][jloc], iloc+gelev_min, CY, y_array[iloc][jloc], jloc+selev_min, dist_min, dx, dy, data[iloc][jloc][isamp], vavg_array[k], dv );
   }
   /* printf ( "Checkshot num = %5d, CX = %10.2f, X = %10.2f, ILOC = %5d, CY = %10.2f, Y = %10.2f, JLOC = %5d, DIST_MIN = %6.2f, DX = %6.2f, DY = %6.2f, Vavg_Seismic = %8.2f, Vavg_Checkshot = %8.2f, Diff = %8.4f\n", 
   k+1, CX, x_array[iloc][jloc], iloc+gelev_min, CY, y_array[iloc][jloc], jloc+selev_min, dist_min, dx, dy, data[iloc][jloc][isamp], vavg_array[k], dv ); */

   data[iloc][jloc][isamp] += dv;
}

for ( i = 0; i < inline_num; ++i ) {
   for ( j = 0; j < xline_num; ++j ) {
      if ( trid[i][j] == 1 ) {
         for ( k = 0; k < ns; ++k ) tr.data[k] = data[i][j][k];
         tr.gelev = i + gelev_min;
         tr.selev = j + selev_min;
         tr.sx = tr.gx = x_array[i][j];
         tr.sy = tr.sx = y_array[i][j];
         puttr ( &tr );
      }
   }
} 
         
free2int    (trid);
free2double (x_array);
free2double (y_array);
free3double (data);
free1double (cx_array);
free1double (cy_array); 

return EXIT_SUCCESS;

}

