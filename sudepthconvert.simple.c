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

int ntr, ntrv, ns, nsv, nz;
double twt, dt, dtv, factor; 
float z, amp_out, *depth, *tr_amp, ***velocity;
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
if (!getparint("ns",  &ns))  ns  = tr.ns;
if (!getparint("nsv", &nsv)) nsv = vtr.ns;
if (!getparint("nz", &nz)) nz = ns;

dt  = tr.dt  * 0.001;
dtv = vtr.dt * 0.001;

depth  = ealloc1float ( ns );
tr_amp = ealloc1float ( ns );

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
   fprintf ( stderr, "\n" );
}

int ivmin, ivmax, xvmin, xvmax;

rewind ( fv );
ivmin = xvmin = INT_MAX;
ivmax = xvmax = INT_MIN;
for ( i = 0; i < ntrv; ++i ) {
   fgettr (fv, &vtr);
   ivmin = min ( ivmin, vtr.gelev );
   ivmax = max ( ivmax, vtr.gelev );
   xvmin = min ( xvmin, vtr.selev );
   xvmax = max ( xvmax, vtr.selev );
}

if ( verbose ) fprintf ( stderr, "VELOCITY DATA - inline min = %d, inline max = %d, xline min = %d, xline max = %d\n", ivmin, ivmax, xvmin, xvmax );

int imin, imax, xmin, xmax;

rewind (stdin);
imin = xmin = INT_MAX;
imax = xmax = INT_MIN;
for ( i = 0; i < ntr; ++i ) {
   gettr (&tr);
   imin = min ( imin, tr.gelev );
   imax = max ( imax, tr.gelev );
   xmin = min ( xmin, tr.selev );
   xmax = max ( xmax, tr.selev );
}

if ( verbose ) fprintf ( stderr, "INPUT DATA - inline min = %d, inline max = %d, xline min = %d, xline max = %d\n", imin, imax, xmin, xmax );

int nxv, nyv;

nxv = ivmax - ivmin + 1;
nyv = xvmax - xvmin + 1;

velocity = ealloc3float ( nsv, nyv, nxv ); 

if ( verbose ) fprintf ( stderr, "nxv = %d, nyv = %d, nsv = %d\n", nxv, nyv, nsv );

rewind ( fv );
for ( i = 0; i < nxv; ++i ) {
   for ( j = 0; j < nyv; ++j ) {
      fgettr (fv, &vtr);
      for ( k = 0; k < nsv; ++k ) velocity[i][j][k] = vtr.data[k];
   }
}
efclose (fv);

int nsm1;
nsm1 = ns - 1;

rewind (stdin);
for ( l = 0; l < ntr; ++l ) {
   gettr (&tr);
   i = tr.gelev - ivmin;
   j = tr.selev - xvmin;
   for ( k = 0; k < ns; ++k ) {
      twt = k * dt;
      depth[k] = velocity[i][j][k] * twt * factor;
      tr_amp[k] = tr.data[k];
   }
   for ( k=0; k < nz; ++k ) {
      z = k * dt;
      intlin ( ns, depth, tr_amp, tr_amp[0], tr_amp[nsm1], 1, &z, &amp_out );
      tr.data[k] = amp_out;
   }
   tr.trid = 1;
   tr.ns = nz;
   tr.dt = nint(dt*1000);
   puttr (&tr);
} 

free1float (tr_amp);
free1float (depth);
free3float (velocity);

return EXIT_SUCCESS;

}
