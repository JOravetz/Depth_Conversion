#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"

#define nint(x) ((x) < 0 ? ceil ( (x) - 0.5 ) : floor ( (x) + 0.5 ))

char *sdoc[] = {NULL};

segy tr;

int main (int argc, char **argv) {

   char temp[256];
   int ntr, ns;
   double x_loc, y_loc, scaler;
   FILE *fpp;
   cwp_String pfile;
   short verbose;
   register int i;

   initargs(argc, argv);

   if (!getparshort("verbose" , &verbose)) verbose = 0;
   if (!getparstring("pfile",&pfile)) pfile = "Gridder_Bill_checkshots_XY_coords.lis";
   if (!getpardouble("scaler",&scaler)) scaler = 100.0;
  
   ntr = gettra (&tr, 0);
   ns  = tr.ns;

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Number of traces = %d, number of samples per trace = %d\n", ntr, ns );
      fprintf ( stderr, "XY Coordinates file name = %s\n", pfile );
      fprintf ( stderr, "\n" );
   }

   rewind (stdin);
   fpp = efopen (pfile, "r");
   for ( i = 0; i < ntr; ++i ) {
      gettr (&tr);
      fgets ( temp, sizeof(temp), fpp );
      (void) sscanf ( ((&(temp[0]))), "%lf%lf", &x_loc, &y_loc );
      tr.sx = nint ( x_loc * scaler );
      tr.sy = nint ( y_loc * scaler );
      tr.scalco = nint ( -scaler );
      tr.tracl = i+1;
      tr.f2 = 0;
      puttr ( &tr );
   }
   efclose (fpp);

   return EXIT_SUCCESS;
}
