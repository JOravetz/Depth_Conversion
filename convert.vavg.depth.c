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

/*********************** self documentation *********************/
char *sdoc[] = {
"                                                                  ",
" **************************************************************** ",
"                                                                  ",
" CONVERT.VAVG.DEPTH -- SU program to intersect well deviation     ",
"                       survey with seismic average-velocity       ",
"                       volume.  Output a checkshot table          ", 
"                                                                  ",
" convert.vavg.depth < indata dfile kb verbose > outdata           ",
"                                                                  ",
" Optional Parameters:                                             ",
"   dfile (deviation.dat) name of ASCII deviation survey file name ",
"                         contains X, Y, and TVD values from well  ",
"   kb (26.0 m.)          kelly-bushing height for TVD datum       ",
"   verbose (0)           print out information if set to (1)      ",
"                                                                  ",
" **************************************************************** ",
NULL};
/**************** end self doc **********************************/
/*Credits:  Joe J. Oravetz, 03/08/2016                          */

segy tr;

int main(int argc, char **argv) {

char temp[256];
FILE *fpd;
cwp_String dfile;

int kount, iloc, jloc, inline_num, xline_num;
int gelev_min, gelev_max, selev_min, selev_max;
int imin, ns, ntr, nsm1, factor1;
int **trid;
double dist, dist_min, kb, x, y, tvd, tvdss;
double sx_min, sx_max, sy_min, sy_max;
double amp_output, delrt, twt, dt, factor;
double *depth, *tr_amp;
double ***data, **x_array, **y_array;
short verbose;
register int i, j, k;

initargs(argc, argv);
requestdoc (0);

/* read in some parameters from the command-line */
if (!getparstring("dfile",&dfile)) dfile = "deviation.dat";
if (!getparshort("verbose", &verbose)) verbose = 1;
if (!getpardouble("kb", &kb)) kb = 26.0;

fpd = efopen ( dfile, "r" );

/* get some basic information from the seismic velocity trace-header values */
ntr = gettra (&tr,  0);
delrt = tr.delrt;
if (!getparint("ns",  &ns)) ns = tr.ns;

dt  = tr.dt  * 0.001;
imin = nint ( delrt / dt );

depth  = ealloc1double ( ns );
tr_amp = ealloc1double ( ns );

factor = 0.0005;

/* scan the deviation file data */
kount = 0;
while (NULL != fgets ( temp, sizeof(temp), fpd )) {
   ++kount;
   (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf", &x, &y, &tvd);
}

double *x_dev, *y_dev, *tvdss_dev;
x_dev     = ealloc1double ( kount );
y_dev     = ealloc1double ( kount );
tvdss_dev = ealloc1double ( kount );

rewind ( fpd );
for ( i = 0; i < kount ; ++i ) {
   fgets ( temp, sizeof(temp), fpd );
   (void) sscanf ( ((&(temp[0]))), "%lf%lf%lf", &x, &y, &tvd);
   x_dev[i] = x;
   y_dev[i] = y;
   tvdss_dev[i] = max ( tvd - kb, 0.0 );
   if ( verbose ) fprintf ( stderr, "kount = %5d, X_DEV = %.2f, Y_DEV = %.2f, Z_TVDSS = %.2f\n", i+1, x_dev[i], y_dev[i], tvdss_dev[i] ); 
}

efclose (fpd);

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "Delay = %f ms., First sample = %d\n", delrt, imin );
   fprintf ( stderr, "Deviation survey file name = %s, number of values in file = %5d\n", dfile, kount );
   fprintf ( stderr, "\n" );
}

/* determine the boundaries of the seismic velocity volume */
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
   fprintf ( stderr, "VELOCITY DATA - sx_min = %.2f, sx_max = %.2f, sy_min = %.2f, sy_max = %.2f\n", sx_min, sx_max, sy_min, sy_max );
   fprintf ( stderr, "Inline min = %5d, Inline max = %5d, Xline min = %5d, Xline max = %5d\n", gelev_min, gelev_max, selev_min, selev_max );
   fprintf ( stderr, "Number of Inlines = %5d, Number of Xlines = %5d\n", inline_num, xline_num );
}

/* allocate some memory arrays */
trid    = ealloc2int    ( xline_num, inline_num ); 
data    = ealloc3double ( ns, xline_num, inline_num ); 
x_array = ealloc2double ( xline_num, inline_num ); 
y_array = ealloc2double ( xline_num, inline_num ); 

/* initialize the live trace-header flag array */
for ( i = 0; i < inline_num; ++i ) for ( j = 0; j < xline_num; ++j ) trid[i][j] = 0;

/* read the velocity data into a memory array */
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

/* the real work starts here - loop over deviation data, find the associated velocity value, convert to depth, and interpolate the result for output */
factor1 = 2000.0;
nsm1 = ns - 1;
for ( i = 0; i < kount; ++i ) {
   x     = x_dev[i];
   y     = y_dev[i];
   tvdss = tvdss_dev[i];
   iloc = jloc = 0;
   dist_min = dist = DBL_MAX;
   for ( j = 0; j < inline_num; ++j ) {
      for ( k = 0; k < xline_num; ++k ) {
         dist = sqrt ( pow ( x - x_array[j][k], 2 ) + pow ( y - y_array[j][k], 2 ) );
         if ( dist < dist_min ) {
            iloc = j;
            jloc = k;
            dist_min = dist;
         }
      }
   }
   if ( verbose ) fprintf ( stderr, "kount = %5d, x_well = %.2f, y_well = %.2f, x_seismic = %.2f, y_seismic = %.2f, dist_min = %.4f\n", i, x, y, x_array[iloc][jloc], y_array[iloc][jloc], dist_min );
   for ( j = 0; j < ns; ++j ) {
      depth[j]  = data[iloc][jloc][j] * ( j * dt * factor );
      tr_amp[j] = data[iloc][jloc][j];
      if ( verbose ) fprintf ( stderr, "j = %5d, twt = %8.2f, average_velocity = %8.2f, depth = %8.2f\n", j, j*dt, tr_amp[j], depth[j] );
   }
   dintlin ( ns, depth, tr_amp, tr_amp[0], tr_amp[nsm1], 1, &tvdss, &amp_output );
   twt = ( tvdss / amp_output ) * factor1;
   printf ( "%12.2f %12.2f %12.4f %12.4f %12.4f\n", x, y, twt, tvdss, amp_output );
}

/* this is only for QC purposes only - to check the velocity array parameters and structure */
/* tr.trid = 1;
for ( i = 0; i < inline_num; ++i ) {
   for ( j = 0; j < xline_num; ++j ) {
      if ( trid[i][j] == 1 ) {
         for ( k = 0; k < ns; ++k ) tr.data[k] = data[i][j][k];
         tr.gelev = i + gelev_min;
         tr.selev = j + selev_min;
         tr.sx = x_array[i][j];
         tr.sy = y_array[i][j];
         tr.gx = 0.0;
         puttr ( &tr );
      }
   }
} */

/* clean up the memory - release the allocated segments */
free1double (tr_amp);
free1double (x_dev);
free1double (y_dev);
free1double (tvdss_dev);
free1double (depth);
free2int    (trid);
free2double (x_array);
free2double (y_array);
free3double (data);

return EXIT_SUCCESS;

}
