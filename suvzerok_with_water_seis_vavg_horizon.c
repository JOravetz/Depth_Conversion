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

char *sdoc[] = {NULL};

segy tr;
float *f1, *f2;

extern int vzerok ( short *, int *, double *, double *, double *, double *, double *, double * );

int main(int argc, char **argv) {

	char *coeff_x, *coeff_x2, file[BUFSIZ];

	struct GRD_HEADER grd_x, grd_x2;
	struct GMT_EDGEINFO edgeinfo_x, edgeinfo_x2;
	struct GMT_BCR bcr_x, bcr_x2;

	int  num, ns, ntr, istart, istop, index;
	float scale_factor;
	double vzero, k, vzero_out, k_out;
	double *x, *y, *xin, *yin, dt, zero, factor;
	double water_depth, value_coeff_x, value_coeff_x2, water_velocity, x_loc, y_loc;
	short mode, check, verbose;
	register int i, l;

	initargs(argc, argv);
	argc = GMT_begin (argc, argv);

	if (!getparshort("verbose", &verbose)) verbose = 1;
        if (!getparshort("mode", &mode)) mode = 2;
	if (!getpardouble("vzero", &vzero)) vzero = 1503.26;
	if (!getpardouble("k", &k)) k = 0.634713;
	if (!getparstring("coeff_x", &coeff_x)) coeff_x="wb.twt.grd";
	if (!getparstring("coeff_x2", &coeff_x2)) coeff_x2="bottom_twt_msec.grd";

	if ( verbose ) {
	   fprintf ( stderr, "\n" );
	   fprintf ( stderr, "WB TWT GMT grid file name = %s\n", coeff_x );
	   fprintf ( stderr, "Bottom Horizon GMT grid file name = %s\n", coeff_x2 );
	}

        ntr = gettra (&tr, 0);
	ns  = tr.ns;
	dt  = tr.dt * 0.001;
	scale_factor = tr.scalco;
	if (scale_factor < 0.0 ) scale_factor *= -1.0;
	if (scale_factor == 0.0 ) scale_factor = 1.0;

	zero = 0.0;
	factor = 0.0005;

	if ( verbose ) {
	   fprintf ( stderr, "\n" );
	   fprintf ( stderr, "Number of input traces = %d\n", ntr );
	   fprintf ( stderr, "Number of samples per trace = %d\n", ns );
	   fprintf ( stderr, "Sample rate = %f ms.\n", dt );
	   fprintf ( stderr, "Scale Factor for X and Y Coordinates = %f\n", scale_factor );
	   fprintf ( stderr, "Initial VZERO = %f, Initial K = %f\n", vzero, k );
           fprintf ( stderr, "Mode = %d\n", mode );
	   fprintf ( stderr, "\n" );
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

	x = ealloc1double (ns);
	y = ealloc1double (ns);
	xin = ealloc1double (ns);
	yin = ealloc1double (ns);

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
	         /* value_coeff_x2 += 100.0; */
	         istop = nint ( value_coeff_x2 / dt );
	         istop = max ( min ( istop, ns ), 0 ); 
	         if ( tr.wevel > 0 ) {
	            water_velocity = tr.wevel;
	         } else {
	            index = max ( min ( nint(value_coeff_x/dt), ns ), 0 );
	            water_velocity = tr.data[index];
	         }
                 if ( mode == 2 ) vzero = water_velocity;
	         water_depth = water_velocity * value_coeff_x * factor;
	      }
	   }

	   if ( verbose == 2 && check )  {
	      fprintf ( stderr, "input trace = %6d xloc = %8.2f yloc = %8.2f WB TWT = %8.2f ms. WB Depth = %10.4f meters Water Velocity = %8.2f mps Bottom Horizon = %8.2f\n",
	      l, x_loc, y_loc, value_coeff_x, water_depth, water_velocity, value_coeff_x2 );
	   }

	   if ( check ) {
	      for (i=0;i<=istop;i++) {
	         x[i] = i * dt;
	         y[i] = ( ( tr.data[i] * x[i] ) * factor ) - water_depth; 
	         x[i] -= value_coeff_x;
	      }

	      istart = 0;
	      for (i=0;i<=istop;i++) {
	         if ( x[i] >= zero && y[i] >= zero ) {
	            istart = i;
	            break;
	         }
	      }

	      num = 0;
	      for (i=istart;i<=istop;i++) {
	         xin[num] = x[i];
	         yin[num] = y[i];
	         ++num;
	      }

	      if ( num > 1 ) {
	         vzerok ( &mode, &num, xin, yin, &vzero, &k, &vzero_out, &k_out );
	         printf ( "trace = %5d num = %4d x_loc = %12.2f y_loc=%12.2f vzero = %20.12f k = %20.12f water_velocity = %12.4f water_depth_m = %12.4f WB_TWT = %12.4f Bottom_Horizon_TWT = %12.4f\n", 
                 l, num, x_loc, y_loc, vzero_out, k_out, water_velocity, water_depth, value_coeff_x, value_coeff_x2 );
	      }
	   }
	}

	free1double (x);
	free1double (y);
	free1double (xin);
	free1double (yin);

	GMT_free ((void *)f1);
	GMT_free ((void *)f2);

	GMT_end (argc, argv);

	return EXIT_SUCCESS;
}

#define SPREAD 1.0

int vzerok ( short *mode, int *imax, double *tval, double *dval, double *vzero, double *k, double *vzero_out, double *k_out ) {

  int *ia, j;
  double alamda, chisq, ochisq, **covar, **alpha;
  double *a, *time, *depth, *sig;

  ia    = ivector(1,2);
  a     = dvector(1,2);
  covar = dmatrix(1,2,1,2);
  alpha = dmatrix(1,2,1,2);
        
  sig   = dvector(1,*imax);
  time  = dvector(1,*imax);
  depth = dvector(1,*imax);

  a[1] = *vzero;
  a[2] = *k;
  ia[1] = 1;
  ia[2] = 1;
  alamda = -1.f;
  for (j=1;j<=*imax;j++) {
     time[j] = tval[j-1] * 0.0005f;
     depth[j] = dval[j-1];
     sig[j] = SPREAD;
  }
  if ( *imax > 2 ) {
     if ( *mode > 0 ) {
        a[1]  = *vzero;
        ia[1] = 0;
     }
     mrqmin (time, depth, sig, *imax, a, ia, 2, covar, alpha, &chisq, depthfunc, &alamda);
     ochisq = chisq;
     for (;;) {
        mrqmin (time, depth, sig, *imax, a, ia, 2, covar, alpha, &chisq, depthfunc, &alamda);
        if ( abs( ochisq - chisq ) < 0.001f ) break;
        ochisq = chisq;
     }
     alamda = 0.f;
     mrqmin (time, depth, sig, *imax, a, ia, 2, covar, alpha, &chisq, depthfunc, &alamda);
     *vzero_out = a[1];
     *k_out = a[2];
  }

  free_dmatrix(alpha,1,2,1,2);
  free_dmatrix(covar,1,2,1,2);
  free_dvector(a,1,2);
  free_dvector(time,1,*imax);
  free_dvector(depth,1,*imax);
  free_dvector(sig,1,*imax);
  free_ivector(ia,1,2);

  return 0;
}
