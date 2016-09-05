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

segy tr, vtr;

int main(int argc, char **argv) {

int  gelev_vmin, gelev_vmax, selev_vmin, selev_vmax;
int  gelev_min, gelev_max, selev_min, selev_max;
int  gelev_globalmin, gelev_globalmax, selev_globalmin, selev_globalmax;
int  gelev_num, selev_num;
int  kk, imin, ifirst, nz1, nz, ns, ntr, ntrv, nsv;
double dfirst, delrt, twt, dt, dtv, factor, z, amp_out;
double *depth, *tr_amp;
float ***velocity;
short verbose;
register int i, j, k, l;
cwp_String vfile;
FILE *fv;

initargs(argc, argv);
requestdoc (0);

if (!getparshort("verbose", &verbose)) verbose = 1;
if (!getparstring("vfile",&vfile)) vfile = "output.su";

ntr = gettra (&tr,  0);
fv = efopen (vfile, "r");
ntrv = fgettra (fv, &vtr, 0);
delrt = tr.delrt;
if (!getparint("ns",  &ns))  ns  = tr.ns;
if (!getparint("nsv", &nsv)) nsv = vtr.ns;
if (!getparint("nz", &nz)) nz=2000;

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
   fprintf ( stderr, "\n" );
}

rewind ( fv );
gelev_vmin = selev_vmin = INT_MAX;
gelev_vmax = selev_vmax = INT_MIN;
for ( i = 0; i < ntrv; ++i ) {
   fgettr (fv, &vtr);
   gelev_vmin = min ( gelev_vmin, vtr.gelev );
   gelev_vmax = max ( gelev_vmax, vtr.gelev );
   selev_vmin = min ( selev_vmin, vtr.selev );
   selev_vmax = max ( selev_vmax, vtr.selev );
}

if ( verbose ) fprintf ( stderr, "VELOCITY DATA - gelev_vmin = %d, gelev_vmax = %d, selev_vmin = %d, selev_vmax = %d\n", gelev_vmin, gelev_vmax, selev_vmin, selev_vmax );

rewind (stdin);
gelev_min = selev_min = INT_MAX;
gelev_max = selev_max = INT_MIN;
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   gelev_min = min ( gelev_min, tr.gelev );
   gelev_max = max ( gelev_max, tr.gelev );
   selev_min = min ( selev_min, tr.selev );
   selev_max = max ( selev_max, tr.selev );
}

if ( verbose ) fprintf ( stderr, "INPUT DATA -    gelev_min =  %d, gelev_max =  %d, selev_min =  %d, selev_max =  %d\n", gelev_min, gelev_max, selev_min, selev_max );

gelev_globalmin = max ( gelev_min, gelev_vmin );
selev_globalmin = max ( selev_min, selev_vmin );
gelev_globalmax = min ( gelev_max, gelev_vmax );
selev_globalmax = min ( selev_max, selev_vmax );

gelev_num = gelev_globalmax - gelev_globalmin + 1;
selev_num = selev_globalmax - selev_globalmin + 1;

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "gelev_globalmin = %d, gelev_globalmax = %d, selev_globalmin = %d, selev_globalmax = %d\n", gelev_globalmin, gelev_globalmax, selev_globalmin, selev_globalmax );
   fprintf ( stderr, "gelev_num = %d, selev_num = %d\n", gelev_num, selev_num );
}

velocity = ealloc3float ( nsv, selev_num, gelev_num );

rewind ( fv );
ifirst = 0;
dfirst = 9999999999999999.9;
for ( k = 0; k < ntrv; ++k ) {
   fgettr (fv, &vtr);
   if ( vtr.gelev >= gelev_globalmin && vtr.gelev <= gelev_globalmax && vtr.selev >= selev_globalmin && vtr.selev <= selev_globalmax ) {
      i = vtr.gelev - gelev_globalmin;
      j = vtr.selev - selev_globalmin;
      memcpy( (void *) &velocity[i][j][0], (const void *) &vtr.data, nsv * FSIZE );
      dfirst = min ( dfirst, velocity[i][j][0] * delrt * factor );
   }
}
efclose (fv);
ifirst = nint ( dfirst / dt );
nz1 = nz - ifirst;
if ( verbose ) {
   fprintf ( stderr, "ifirst = %d, dfirst = %f, nz1 = %d\n", ifirst, dfirst, nz1 );
}

rewind (stdin);
for ( l = 0; l < ntr; ++l ) {
   gettr (&tr);
   if ( tr.gelev >= gelev_globalmin && tr.gelev <= gelev_globalmax && tr.selev >= selev_globalmin && tr.selev <= selev_globalmax ) {
      i = tr.gelev - gelev_globalmin;
      j = tr.selev - selev_globalmin;
      for ( k = 0; k < ns; ++k ) {
         kk = k + imin;
         twt = kk * dt;
         tr_amp[k] = tr.data[k];
         depth[k] = velocity[i][j][kk] * twt * factor;
      }
      for ( k=0; k < nz1; ++k ) {
         z = ( k * dt ) + dfirst;
         dintlin ( ns, depth, tr_amp, tr_amp[0], tr_amp[ns-1], 1, &z, &amp_out );
         tr.data[k] = (float) amp_out;
      }
      tr.trid = 1;
      tr.ns = nz1;
      tr.delrt = nint ( dfirst );
      tr.dt = nint(dt*1000);
      puttr (&tr);
   } 
} 

free1double (tr_amp);
free1double (depth);
free3float  (velocity);

return EXIT_SUCCESS;

}
