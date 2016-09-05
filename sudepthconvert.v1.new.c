#include <omp.h>
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

#define CHUNKSIZE 1

char *sdoc[] = {NULL};

segy tr, vtr;

int main(int argc, char **argv) {

double sx_vmin, sx_vmax, sy_vmin, sy_vmax;
double scalco, scalco_vel;
double sx, sy;

int nsm1;
float zero;
double depth_max;

int  nz1, ifirst, kk, imin, nz, ns, ntr, ntrv, nsv;
double dfirst, delrt, twt, dt, dtv, factor, z, amp_out;
double *depth, *tr_amp;
float ***velocity;
short verbose;
register int i, j, k, l;
cwp_String vfile;
FILE *fv;

double dist_min, dist;

initargs(argc, argv);
requestdoc (0);

if (!getparshort("verbose", &verbose)) verbose = 1;
if (!getparstring("vfile",&vfile)) vfile = "vel.scaled.smooth.su";

ntr = gettra (&tr,  0);
fv = efopen (vfile, "r");
ntrv = fgettra (fv, &vtr, 0);
delrt = tr.delrt;
scalco = tr.scalco;
if (scalco < 0.0 ) scalco *= -1.0;
if (scalco == 0.0 ) scalco = 1.0;
scalco_vel = vtr.scalco;
if (scalco_vel < 0.0 ) scalco_vel *= -1.0;
if (scalco_vel == 0.0 ) scalco_vel = 1.0;

if (!getparint("ns",  &ns))  ns  = tr.ns;
if (!getparint("nsv", &nsv)) nsv = vtr.ns;
if (!getparint("nz", &nz)) nz = 2000;

dt  = tr.dt  * 0.001;
dtv = vtr.dt * 0.001;
imin = nint ( delrt / dt );

depth  = ealloc1double ( ns );
tr_amp = ealloc1double ( ns );

factor = 0.0005;

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of input samples per trace = %d\n", ns );
   fprintf ( stderr, "Number of output samples per trace = %d\n", nz );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "Average velocity SU file name = %s\n", vfile );
   fprintf ( stderr, "Number of VAVG input traces = %d\n", ntrv );
   fprintf ( stderr, "Number of VAVG input samples per trace = %d\n", nsv );
   fprintf ( stderr, "VAVG Sample rate = %f ms.\n", dtv );
   fprintf ( stderr, "Delay = %f ms., First sample = %d\n", delrt, imin );
   fprintf ( stderr, "Seismic trace coordinate scale factor = %f\n", scalco );
   fprintf ( stderr, "Velocity trace coordinate scale factor = %f\n", scalco_vel );
   fprintf ( stderr, "\n" );
}

int nxv, nyv, iline, xline;
int iline_min, iline_max, xline_min, xline_max;

rewind ( fv );

iline_min = xline_min = INT_MAX;
iline_max = xline_max = INT_MIN;
sx_vmin = sy_vmin = DBL_MAX;
sx_vmax = sy_vmax = DBL_MIN;

for ( i = 0; i < ntrv; ++i ) {
   fgettr (fv, &vtr);

   iline = vtr.gelev;
   xline = vtr.selev;
   iline_min = min ( iline, iline_min );
   xline_min = min ( xline, xline_min );
   iline_max = max ( iline, iline_max );
   xline_max = max ( xline, xline_max );

   sx = vtr.sx / scalco_vel;
   sy = vtr.sy / scalco_vel;

   sx_vmin = min ( sx_vmin, sx );
   sx_vmax = max ( sx_vmax, sx );
   sy_vmin = min ( sy_vmin, sy );
   sy_vmax = max ( sy_vmax, sy );
}

nxv = iline_max - iline_min + 1;
nyv = xline_max - xline_min + 1;

if ( verbose ) {
   fprintf ( stderr, "VELOCITY - Minimum inline = %5d, Maximum inline = %5d, number of inlines (NXV) = %5d\n", iline_min, iline_max, nxv );
   fprintf ( stderr, "           Minimum xline  = %5d, Maximum xline  = %5d, number of xlines  (NYV) = %5d\n", xline_min, xline_max, nyv );
   fprintf ( stderr, "           sx_vmin = %.2f, sx_vmax = %.2f, sy_vmin = %.2f, sy_vmax = %.2f\n", sx_vmin, sx_vmax, sy_vmin, sy_vmax );
   fprintf ( stderr, "\n" );
}

velocity = ealloc3float ( nsv, nyv, nxv );

float x, y;
float **x_array, **y_array;
double xi_first, yi_first, xj_first, yj_first;
double avg_dist_dx, avg_dist_dy;

x_array = ealloc2float ( nyv, nxv );
y_array = ealloc2float ( nyv, nxv );

xi_first = yi_first = xj_first = yj_first = avg_dist_dx = avg_dist_dy = 0.0;

rewind ( fv ); 
dfirst = DBL_MAX;
for ( i = 0; i < nxv; ++i ) {
   for ( j = 0; j < nyv; ++j ) {
      fgettr (fv, &vtr);

      x_array[i][j] = x = vtr.sx / scalco_vel;
      y_array[i][j] = y = vtr.sy / scalco_vel;

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

      for ( k = 0; k < nsv; ++k ) velocity[i][j][k] = tr.data[k];
      dfirst = min ( dfirst, velocity[i][j][0] * delrt * factor );
   }
}

efclose ( fv );

ifirst = nint ( dfirst / dt );
nz1 = nz - ifirst;

avg_dist_dx /= ( nxv - 1 );
avg_dist_dy /= ( nyv - 1 ) * nxv;

if ( verbose ) {
   fprintf ( stderr, "Average Dist DX = %10.4f, Average Dist DY = %10.4f\n", avg_dist_dx, avg_dist_dy );
   fprintf ( stderr, "\n" );
}

nsm1 = ns - 1;
zero = 0.0;

int iloc, jloc;
double *vtrace;

vtrace = ealloc1double ( nsv );

rewind (stdin);
for ( l = 0; l < ntr; ++l ) {
   gettr (&tr);
   x = tr.sx / scalco;
   y = tr.sy / scalco;
   if ( x >= sx_vmin && x <= sx_vmax && y >= sy_vmin && y <= sy_vmax ) {
      #pragma omp parallel private (i,j,dist_min,dist,iloc,jloc)
{
      iloc = jloc = 0;
      dist_min = dist = FLT_MAX;
      #pragma omp for schedule(dynamic, CHUNKSIZE)
      for ( i = 0; i < nxv; ++i ) {
         for ( j = 0; j < nyv; ++j ) {
            dist = sqrt ( pow ( x - x_array[i][j], 2.0 ) + pow ( y - y_array[i][j], 2.0 ) );
            if ( dist < dist_min ) {
               iloc = i;
               jloc = j;
               dist_min = dist;
            }
         }
      }
      for ( i = 0; i < nsv; ++i ) vtrace[i] = velocity[iloc][jloc][i];
} /* end of parallel code */

      for ( k = 0; k < ns; ++k ) {
         kk = k + imin;
         twt = kk * dt;
         tr_amp[k] = tr.data[k];
         depth[k] = vtrace[kk] * twt * factor;
      }

      depth_max = depth[nsm1];

      for ( k=0; k < nz1; ++k ) {
         z = ( k * dt ) + dfirst;
         if ( z <= depth_max ) {
            dintlin ( ns, depth, tr_amp, tr_amp[0], tr_amp[nsm1], 1, &z, &amp_out );
            tr.data[k] = (float) amp_out;
         } else {
            tr.data[k] = zero;
         }
      }
      tr.trid = 1;
      tr.ns = nz1;
      tr.delrt = nint ( dfirst );
      tr.dt = nint (dt*1000);
      puttr (&tr);
   } 
} 

free1double (vtrace);
free1double (tr_amp);
free1double (depth);
free3float  (velocity);

return EXIT_SUCCESS;

}
