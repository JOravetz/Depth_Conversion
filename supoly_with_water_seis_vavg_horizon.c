#include "gmt.h"
#include <stdio.h>
#include <stddef.h>
#include "nr.h"
#include "nrutil.h"
#include "par.h"
#include "su.h"
#include "segy.h"

#define abs(x)   ( (x) <  0  ? -(x) : (x) )
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

/*********************** self documentation *********************/
char *sdoc[] = {
"                                                               ",
" SUPOLY_WITH_WATER_SEIS_VAVG -- Given matrices of the form     ",
"    A*x=b, will find soln. vector (x) coefficients             ",
"                                                               ",
" supoly_with_water_seis_vavg < indata n > outdata              ",
"                                                               ",
" Optional Parameters:                                          ",
"   n         (4)       degree of approximating polynomial      ", 
"   intercept (0)       do not compute intercept coefficient    ", 
"   verbose   (0)       print out information if set to (1)     ",
NULL};
/**************** end self doc **********************************/
/*Credits:  Joe J. Oravetz, 26/04/1998                          */

segy tr;
float *f1, *f2;

int main(int argc, char **argv) {

char *coeff_x, *coeff_x2, file[BUFSIZ];

struct GRD_HEADER grd_x, grd_x2;
struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2;
struct GMT_BCR bcr_x, bcr_x2;

int  *idx, n, ns, ntr, istart, istop, index;
float scale_factor;
double *x, *y, dt, zero, factor, rns;
double ri, **A, *B, d, sum;
double water_depth, error, rx, ry;
double value_coeff_x, value_coeff_x2, water_velocity, x_loc, y_loc;
short check, verbose, intercept;
register int i, j, k, l;

initargs(argc, argv);
argc = GMT_begin (argc, argv);

if (!getparint("n", &n)) n = 4;
if (!getparshort("intercept", &intercept)) intercept = 0;
if (!getparshort("verbose", &verbose)) verbose = 1;

if (!getparstring("coeff_x", &coeff_x)) coeff_x="/home/user/Field/GRIDS/wb.twt.sm2500.new.grd";
if (!getparstring("coeff_x2", &coeff_x2)) coeff_x2="/home/user/Field/HORIZONS/DC_Base_Reservoir.reform.dat.trimmed.sample.smooth.grd";

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "WB TWT GMT grid file name = %s\n", coeff_x );
   fprintf ( stderr, "Bottom Horizon GMT grid file name = %s\n", coeff_x2 );
}

GMT_boundcond_init (&edgeinfo_x);
GMT_boundcond_init (&edgeinfo_x2);

GMT_grd_init (&grd_x,  argc, argv, FALSE);
GMT_grd_init (&grd_x2, argc, argv, FALSE);

if (GMT_read_grd_info (coeff_x,  &grd_x))  fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
if (GMT_read_grd_info (coeff_x2, &grd_x2)) fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);

f1 = (float *) GMT_memory (VNULL, (size_t)((grd_x.nx  + 4) * (grd_x.ny  + 4)), sizeof(float), GMT_program);
f2 = (float *) GMT_memory (VNULL, (size_t)((grd_x2.nx + 4) * (grd_x2.ny + 4)), sizeof(float), GMT_program);

GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

GMT_boundcond_param_prep (&grd_x,  &edgeinfo_x);
GMT_boundcond_param_prep (&grd_x2, &edgeinfo_x2);

GMT_boundcond_set (&grd_x, &edgeinfo_x, GMT_pad, f1);
GMT_boundcond_set (&grd_x2, &edgeinfo_x2, GMT_pad, f2);

GMT_bcr_init (&grd_x,  GMT_pad, BCR_BSPLINE, 1, &bcr_x);
GMT_bcr_init (&grd_x2, GMT_pad, BCR_BSPLINE, 1, &bcr_x2);

GMT_read_grd (coeff_x,  &grd_x,  f1, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
GMT_read_grd (coeff_x2, &grd_x2, f2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

ntr = gettra (&tr, 0);
ns  = tr.ns;
dt  = tr.dt * 0.001;
scale_factor = tr.scalco;
if (scale_factor < 0.0 ) scale_factor *= -1.0;
if (scale_factor == 0.0 ) scale_factor = 1.0;

zero = 0.0;
factor = 0.0005;
rns = ns;

if ( verbose ) {
   fprintf ( stderr, "\n" );
   fprintf ( stderr, "Degree of Polynomial = %d\n", n );
   fprintf ( stderr, "Intercept = %d\n", intercept );
   fprintf ( stderr, "Number of input traces = %d\n", ntr );
   fprintf ( stderr, "Number of samples per trace = %d\n", ns );
   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
   fprintf ( stderr, "Scale Factor for X and Y Coordinates = %f\n", scale_factor );
   fprintf ( stderr, "\n" );
}

A = dmatrix (1,n,1,n);
B = dvector (1,n);

x = ealloc1double (ns);
y = ealloc1double (ns);

rewind (stdin);
for (l=0;l<ntr;l++) {
   gettr (&tr);
   x_loc = tr.sx = nint ( tr.sx / scale_factor );
   y_loc = tr.sy = nint ( tr.sy / scale_factor );

   check = 0;
   istop = ns;
   if ( x_loc >= grd_x.x_min && x_loc <= grd_x.x_max && y_loc >= grd_x.y_min && y_loc <= grd_x.y_max ) check = 1;

   if ( check ) {
      value_coeff_x  = GMT_get_bcr_z (&grd_x,  x_loc, y_loc, f1, &edgeinfo_x,  &bcr_x);
      value_coeff_x2 = GMT_get_bcr_z (&grd_x2, x_loc, y_loc, f2, &edgeinfo_x2, &bcr_x2);
      if (GMT_is_dnan (value_coeff_x) || GMT_is_dnan (value_coeff_x2)) {
         check = 0;
      } else {
         if ( value_coeff_x < 0.0 ) value_coeff_x *= -1.0;
         if ( value_coeff_x2 < 0.0 ) value_coeff_x2 *= -1.0;
         value_coeff_x2 += 100.0;
         istop = nint ( value_coeff_x2 / dt );
         istop = max ( min ( istop, ns ), 0 ); 
         if ( tr.wevel > 0 ) {
            water_velocity = tr.wevel;
         } else {
            index = max ( min ( nint(value_coeff_x/dt), ns ), 0 );
            water_velocity = tr.data[index];
         }
         water_depth = water_velocity * value_coeff_x * factor;
      }
   }

   if ( verbose == 3 && check )  {
      fprintf ( stderr, "input trace = %6d, xloc = %8.2f yloc = %8.2f, WB TWT = %8.2f ms., WB Depth = %10.4f meters, Water Velocity = %8.2f mps, Bottom Horizon = %8.2f\n",
      l, x_loc, y_loc, value_coeff_x, water_depth, water_velocity, value_coeff_x2 );
   }

   if ( check ) {
      for (k=0;k<=istop;k++) {
         x[k] = k * dt;
         y[k] = ( ( tr.data[k] * x[k] ) * factor ) - water_depth; 
         x[k] -= value_coeff_x;
      }

      istart = 0;
      for (k=0;k<=istop;k++) {
         if ( x[k] >= zero && y[k] >= zero ) {
            istart = k;
            break;
         }
      }

      if ( verbose == 3 ) fprintf ( stderr, "istart = %d, istop = %d\n", istart, istop );

      for (i=1;i<=n;i++) {
         for (j=1;j<=n;j++) {
            sum = zero;
            ri = i+j-2;
            for (k=istart;k<=istop;k++) sum += pow ( x[k], ri );
            if (i>1&&j==1&&intercept==0) sum = zero;
            A[i][j] = sum;
         }
      }

      for (i=1;i<=n;i++) {
         sum = zero;
         ri = i - 1;
         for (j=istart;j<=istop;j++) sum += pow( x[j], ri ) * y[j];
         B[i] = sum;
      }

      idx  = ivector (1, n); 
      ludcmp ( A, n, idx, &d ); 
      lubksb ( A, n, idx, B ); 

      error = ry = zero;
      if ( intercept ) {
         for ( i=istart; i<=istop; i++ ) {
            rx = x[i];
            ry = B[1]; 
            for (j=2;j<=n;j++) ry += B[j] * pow ( rx, j-1 );
            error += abs ( ry - y[i] );
            if ( verbose == 2 ) fprintf ( stderr, "%-6d %12.2f %12.2f %12.2f %12.4f %12.4f\n", i, rx, y[i], ry, ry - y[i], error );
         }
      } else {
         for ( i=istart; i<=istop; i++ ) {
            rx = x[i];
            ry = zero;
            for (j=1;j<=n;j++) ry += B[j] * pow ( rx, j-1 );
            error += abs ( ry - y[i] );
            if ( verbose == 2 ) fprintf ( stderr, "%-6d %12.2f %12.2f %12.2f %12.4f %12.4f\n", i, rx, y[i], ry, ry - y[i], error );
         }
      }

      rns = istop - istart + 1;
      if ( verbose ) {
         if ( intercept ) {
            for (i=1;i<=n;i++) printf ( "coefficient number = %5d, Solution vector = %30.25f, Average Error = %12.4f\n", i, B[i], error / rns );
         } else {
            for (i=2;i<=n;i++) printf ( "coefficient number = %5d, Solution vector = %30.25f, Average Error = %12.4f\n", i, B[i], error / rns );
         }
      } else {
         printf ( "%-12.2f %12.2f ", x_loc, y_loc );
         if ( intercept ) {
            for (i=1;i<n;i++) printf ( "%30.25f ", B[i] );
         } else {
            for (i=2;i<n;i++) printf ( "%30.25f ", B[i] );
         }
         printf ( "%30.25f %12.4f %12.4f %12.4f %12.4f %12.4f\n", B[n], error / rns, water_velocity, water_depth, value_coeff_x, value_coeff_x2 );
      }
   }
}

free_dmatrix (A,1,n,1,n);
free_dvector (B,1,n);

free1double (x);
free1double (y);

GMT_free ((void *)f1);
GMT_free ((void *)f2);

GMT_end (argc, argv);

return EXIT_SUCCESS;

}
