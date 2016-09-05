#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"

char *sdoc[] = {NULL};

int main (int argc, char **argv) {

   short verbose;
   double part, wb, vzero, k, twt, t, depth;

   initargs(argc, argv);

   if (!getparshort("verbose" , &verbose)) verbose = 0;
   if (!getpardouble("wb", &wb)) {
      fprintf ( stderr, "Must input water-bottom (wb) parameter --> exiting" );
      return EXIT_FAILURE;
   }
   if (!getpardouble("twt", &twt)) {
      fprintf ( stderr, "Must input Two-Way Time (twt) parameter --> exiting" );
      return EXIT_FAILURE;
   }
   if (!getpardouble("vzero", &vzero)) {
      fprintf ( stderr, "Must input VZERO (vzero) parameter --> exiting" );
      return EXIT_FAILURE;
   }
   if (!getpardouble("depth", &depth)) {
      fprintf ( stderr, "Must input DEPTH (depth) parameter --> exiting" );
      return EXIT_FAILURE;
   }

   if ( verbose ) {
      fprintf ( stderr, "\n" );
      fprintf ( stderr, "Water-Bottm = %f\n", wb );
      fprintf ( stderr, "TWT = %f\n", twt );
      fprintf ( stderr, "VZERO = %f\n", vzero );
      fprintf ( stderr, "DEPTH = %f\n", depth );
      fprintf ( stderr, "\n" );
   }

   t = ( twt * 0.0005 ) - ( wb / vzero );
   part = vzero * exp ( t );
   k = -1.0 * ( vzero / ( depth - part - wb ) );

   printf ( "K = %.15f\n", k );

   k = 0.48;

   fprintf ( stderr, "DEPTH check = %f\n", (vzero/k) * ( exp ( k * t ) - 1 ) + wb );

   return EXIT_SUCCESS;
}
