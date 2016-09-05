#include "gmt.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"
#include "header.h"

#define nint(x)  ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

segy tr;
float *f_top_z, *f_bottom_z;

int main (int argc, char **argv) {

   char file[BUFSIZ];
   char *coeff_top_z, *coeff_bottom_z;
   struct GRD_HEADER grd_top_z, grd_bottom_z;
   struct GMT_EDGEINFO edgeinfo_top_z, edgeinfo_bottom_z;
   struct GMT_BCR bcr_top_z, bcr_bottom_z;
   double value_coeff_top_z, value_coeff_bottom_z;

   short verbose;
   int delrt, isamp, scalar, ntr, ns;
   float number;
   double dz, x_loc, y_loc; 
   register int k;

   initargs(argc, argv);
   argc = GMT_begin (argc, argv);

   if (!getparstring ("coeff_top_z",       &coeff_top_z))       coeff_top_z       = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/01_FIELD_Top_Reservoir.depth.new.grd";
   if (!getparstring ("coeff_bottom_z",    &coeff_bottom_z))    coeff_bottom_z    = "/home/user/FIELD.new/PICKS/DEPTH_GRIDS/10_FIELD_80MaSB.depth.new.grd";

   if (!getparshort ("verbose", &verbose)) verbose = 0;
   if (!getparfloat ("number", &number)) number = 0.5;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "TOP_Z    - GMT grid file name = %s\n", coeff_top_z );
      fprintf ( stderr, "BOTTOM_Z - GMT grid file name = %s\n", coeff_bottom_z );
      fprintf ( stderr, "\n" );
   }

   GMT_boundcond_init (&edgeinfo_top_z);
   GMT_boundcond_init (&edgeinfo_bottom_z);

   GMT_grd_init (&grd_top_z,       argc, argv, FALSE);
   GMT_grd_init (&grd_bottom_z,    argc, argv, FALSE);

   if (GMT_read_grd_info (coeff_top_z, &grd_top_z))             fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);
   if (GMT_read_grd_info (coeff_bottom_z, &grd_bottom_z))       fprintf (stderr, "%s: Error opening file %s\n", GMT_program, file);

   f_top_z       = (float *) GMT_memory (VNULL, (size_t)((grd_top_z.nx  + 4)       * (grd_top_z.ny  + 4)),       sizeof(float), GMT_program);
   f_bottom_z    = (float *) GMT_memory (VNULL, (size_t)((grd_bottom_z.nx  + 4)    * (grd_bottom_z.ny  + 4)),    sizeof(float), GMT_program);

   GMT_pad[0] = GMT_pad[1] = GMT_pad[2] = GMT_pad[3] = 2;

   GMT_boundcond_param_prep (&grd_top_z,       &edgeinfo_top_z);
   GMT_boundcond_param_prep (&grd_bottom_z,    &edgeinfo_bottom_z);

   GMT_boundcond_set (&grd_top_z,       &edgeinfo_top_z,       GMT_pad, f_top_z);
   GMT_boundcond_set (&grd_bottom_z,    &edgeinfo_bottom_z,    GMT_pad, f_bottom_z);

   GMT_bcr_init (&grd_top_z,       GMT_pad, BCR_BSPLINE, 1, &bcr_top_z);
   GMT_bcr_init (&grd_bottom_z,    GMT_pad, BCR_BSPLINE, 1, &bcr_bottom_z);

   GMT_read_grd (coeff_top_z,       &grd_top_z,       f_top_z,       0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);
   GMT_read_grd (coeff_bottom_z,    &grd_bottom_z,    f_bottom_z,    0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE);

   /* Get info from first trace */
   ntr     = gettra (&tr, 0);
   ns      = tr.ns;
   delrt   = tr.delrt;
   scalar  = abs ( tr.scalco );
   if ( scalar == 0 ) scalar = 1; 
   dz = tr.dt * 0.001;

   if ( verbose ) {
      fprintf ( stderr, "Number of traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "Sample rate (meters) = %f\n", dz );
      fprintf ( stderr, "Location scalar = %d\n", scalar );
      fprintf ( stderr, "Starting depth = %d\n", delrt );
      fprintf ( stderr, "Horizon sample value = %f\n", number );
      fprintf ( stderr, "\n" );
   }

   rewind (stdin);

   /* Main loop over traces */
   for ( k = 0; k < ntr; ++k ) {
      gettr (&tr);
      x_loc = tr.sx / scalar;
      y_loc = tr.sy / scalar;
      delrt = tr.delrt;

      value_coeff_top_z    = GMT_get_bcr_z (&grd_top_z,       x_loc, y_loc, f_top_z,       &edgeinfo_top_z,       &bcr_top_z);
      value_coeff_bottom_z = GMT_get_bcr_z (&grd_bottom_z,    x_loc, y_loc, f_bottom_z,    &edgeinfo_bottom_z,    &bcr_bottom_z);

      if ( verbose == 2 ) {
         fprintf ( stderr, "\n" );
         fprintf ( stderr, "Trace num = %5d X-Loc = %10.2f Y-Loc = %10.2f\n", k+1, x_loc, y_loc );
         fprintf ( stderr, "TOP_Z    = %8.2f\n", value_coeff_top_z );
         fprintf ( stderr, "BOTTOM_Z = %8.2f\n", value_coeff_bottom_z );
      }

      if ( ! ( GMT_is_dnan (value_coeff_top_z) ) ) {
         isamp = nint ( ( value_coeff_top_z - delrt ) / dz ); 
         tr.data[isamp-1] = number;
         tr.data[isamp] = number;
         tr.data[isamp+1] = number;
      }

      if ( ! (GMT_is_dnan (value_coeff_bottom_z) ) ) { 
         isamp = nint ( ( value_coeff_bottom_z - delrt ) / dz ); 
         tr.data[isamp-1] = number;
         tr.data[isamp] = number;
         tr.data[isamp+1] = number;
      } 

      puttr (&tr);

   }

   GMT_free ((void *)f_top_z);
   GMT_free ((void *)f_bottom_z);

   GMT_end (argc, argv);

   return (0);
}
