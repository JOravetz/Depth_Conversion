#include "gmt_dev.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"
#include "header.h"

#define MAX_GRIDS GMT_BUFSIZ

struct GRD_CONTAINER {
        struct GMT_GRID *G;
        int type;
};

struct GRDTRACK_CTRL {
        struct In {
                bool active;
                char *file;
        } In;
        struct Out {    /* -> */
                bool active;
                char *file;
        } Out;
        struct G { 
                bool active;
                unsigned int n_grids;
                char *file[MAX_GRIDS];
                double scale[MAX_GRIDS], lat[MAX_GRIDS];
                int mode[MAX_GRIDS];
                int type[MAX_GRIDS];
        } G;
};

void *New_gmt_jjo_Ctrl (struct GMT_CTRL *GMT) {
        struct GRDTRACK_CTRL *C;
        C = GMT_memory (GMT, NULL, 1, struct GRDTRACK_CTRL);
        return (C);
}

int xxx_sample_all_grids (struct GMT_CTRL *GMT, struct GRD_CONTAINER *GC, unsigned int n_grids, bool img, double x_in, double y_in, double value[]) {

   unsigned int g, n_in, n_set;
   double x, y, x0 = 0.0, y0 = 0.0;

   if (img) GMT_geo_to_xy (GMT, x_in, y_in, &x0, &y0);  /* At least one Mercator IMG grid in use - get Mercator coordinates x,y */

   for (g = n_in = n_set = 0; g < n_grids; g++) {
      y = (GC[g].type == 1) ? y0 : y_in;
      value[g] = GMT->session.d_NaN;    /* In case the point is outside only some of the grids */

      while ((y < GC[g].G->header->wesn[YLO]) && (GC[g].G->header->nyp > 0)) y += (GC[g].G->header->inc[GMT_Y] * GC[g].G->header->nyp);
      if (y < GC[g].G->header->wesn[YLO]) continue;

      while ((y > GC[g].G->header->wesn[YHI]) && (GC[g].G->header->nyp > 0)) y -= (GC[g].G->header->inc[GMT_Y] * GC[g].G->header->nyp);
      if (y > GC[g].G->header->wesn[YHI]) continue;

      if (GC[g].type == 1) {    /* This grid is in Mercator x/y units - must use Mercator x0, y0 */
         x = x0;
         if (x > GC[g].G->header->wesn[XHI]) x -= 360.0;
      } else {  /* Regular Cartesian x,y or lon,lat */
         x = x_in;
         if (GMT_is_geographic (GMT, GMT_IN)) { /* Must wind lonn/lat to fit current grid longitude range */
            while (x > GC[g].G->header->wesn[XHI]) x -= 360.0;
            while (x < GC[g].G->header->wesn[XLO]) x += 360.0;
         }
      }
      while ((x < GC[g].G->header->wesn[XLO]) && (GC[g].G->header->nxp > 0)) x += (GC[g].G->header->inc[GMT_X] * GC[g].G->header->nxp);
      if (x < GC[g].G->header->wesn[XLO]) continue;

      while ((x > GC[g].G->header->wesn[XHI]) && (GC[g].G->header->nxp > 0)) x -= (GC[g].G->header->inc[GMT_X] * GC[g].G->header->nxp);
      if (x > GC[g].G->header->wesn[XHI]) continue;

      n_in++;
      value[g] = GMT_get_bcr_z (GMT, GC[g].G, x, y);
      fprintf ( stderr, "Value = %8.2f, g = %d, x = %f, y = %f\n", value[g], g, x, y );

      if (!GMT_is_dnan (value[g])) n_set++;
   }

   if (n_in == 0) return (-1);
   return (n_set);
}

char *sdoc[] = {NULL};

segy tr;

int main (int argc, char **argv) {

   char *grdfile;
   short verbose;
   double *value, wesn[4];
   double x, y;

   struct GRDTRACK_CTRL *Ctrl = NULL;
   struct GRD_CONTAINER *GC = NULL;
   struct GMT_DATASET *Din = NULL, *Dout = NULL;
   struct GMT_DATATABLE *T = NULL;
   struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
   struct GMT_OPTION *options = NULL;

   initargs(argc, argv);

   void *API = NULL;
   API = GMT_Create_Session ("Session name", 2, 0, NULL);

   if (!getparstring("grdfile", &grdfile)) grdfile = "wb.twt.grd";
   if (!getparshort("verbose" , &verbose)) verbose = 0;

   if ( verbose ) {
      fprintf ( stderr, "Grid file name = %s\n", grdfile );
   }

   Ctrl = New_gmt_jjo_Ctrl (GMT);
   Ctrl->G.n_grids = 1;

   Ctrl->G.file[0] = strdup (grdfile);

   x = 520605.450000; 
   y = 160671.580000;

        int status, error, ks;
        uint64_t n_points = 0, n_read = 0;
        unsigned int g, k;
        bool img_conv_needed = false, some_outside = false;
        char line[GMT_BUFSIZ] = {""}, run_cmd[BUFSIZ] = {""}, *cmd = NULL;
        
        GMT_memset (wesn, 4, double);

        fprintf ( stderr, "getting ready to execute GMT_memory, Ctrl = %d\n", Ctrl->G.n_grids );
        GC = GMT_memory (GMT, NULL, Ctrl->G.n_grids, struct GRD_CONTAINER);

        for (g = 0; g < Ctrl->G.n_grids; g++) {
              if ((GC[g].G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->G.file[g], NULL)) == NULL) return (EXIT_FAILURE);
              if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->G.file[g], GC[g].G) == NULL) return (EXIT_FAILURE);
              GMT_memcpy (GMT->common.R.wesn, wesn, 4, double);
        }
        value = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);

       fprintf ( stderr, "made it to here\n" );

       bool pure_ascii = false;
        int ix, iy, n_fields, rmode;
        uint64_t n_out = 0;
        double *in = NULL, *out = NULL;
        char record[GMT_BUFSIZ];
        bool gmt_skip_output (struct GMT_CTRL *C, double *cols, uint64_t n_cols);

        if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) return (EXIT_FAILURE);
        if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) return (EXIT_FAILURE);
        if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_HEADER_ON) != GMT_OK) return (EXIT_FAILURE);
        if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) return (EXIT_FAILURE);

        GMT_memset (line, GMT_BUFSIZ, char);

        pure_ascii = GMT_is_ascii_record (GMT);
        ix = (GMT->current.setting.io_lonlat_toggle[GMT_IN]);   iy = 1 - ix;
        rmode = (pure_ascii && GMT_get_cols (GMT, GMT_IN) >= 2) ? GMT_READ_MIXED : GMT_READ_DOUBLE;

        do {
           if ((in = GMT_Get_Record (API, rmode, &n_fields)) == NULL) {
              if (GMT_REC_IS_ERROR (GMT)) return (EXIT_FAILURE);
              if (GMT_REC_IS_TABLE_HEADER (GMT)) {
                 GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
                 continue;
              }
            if (GMT_REC_IS_SEGMENT_HEADER (GMT)) {
                 GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
                 continue;
              }
              if (GMT_REC_IS_EOF (GMT)) break;
           }

           if (n_out == 0) {
              n_out = GMT_get_cols (GMT, GMT_IN) + Ctrl->G.n_grids;     /* Get new # of output cols */
              if ((error = GMT_set_cols (GMT, GMT_OUT, n_out))) return (EXIT_FAILURE);
           }
           n_read++;

           status = xxx_sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, in[GMT_X], in[GMT_Y], value);
           if (status == -1) some_outside = true;
           if (pure_ascii && n_fields >= 2) {
              if (gmt_skip_output (GMT, value, Ctrl->G.n_grids)) continue;      /* Suppress output due to NaNs */
              for (k = 0; GMT->current.io.current_record[k]; k++) if (GMT->current.io.current_record[k] == ',') GMT->current.io.current_record[k] = ' ';
              record[0] = 0;
              sscanf (GMT->current.io.current_record, "%*s %*s %[^\n]", line);
              GMT_add_to_record (GMT, record, in[ix], ix, 2);   /* Format our output x value */
              GMT_add_to_record (GMT, record, in[iy], iy, 2);   /* Format our output y value */
              strcat (record, line);
              for (g = 0; g < Ctrl->G.n_grids; g++) GMT_add_to_record (GMT, record, value[g], GMT_Z+g, 1);      /* Format our output y value */
             GMT_Put_Record (API, GMT_WRITE_TEXT, record);     /* Write this to output */
           } else {    
              if (!out) out = GMT_memory (GMT, NULL, n_out, double);
              for (ks = 0; ks < n_fields; ks++) out[ks] = in[ks];
              for (g = 0; g < Ctrl->G.n_grids; g++, ks++) out[ks] = value[g];
              GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
           }
           n_points++;

        } while (true);

        if (GMT_End_IO (API, GMT_IN,  0) != GMT_OK) return (EXIT_FAILURE);
        if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) return (EXIT_FAILURE);
        if (out) GMT_free (GMT, out);
        if (some_outside) GMT_Report (API, GMT_MSG_VERBOSE, "Some input points were outside the grid domain(s).\n");

        for (g = 0; g < Ctrl->G.n_grids; g++) {
           GMT_Report (API, GMT_MSG_VERBOSE, "Sampled %" PRIu64 " points from grid %s (%d x %d)\n", n_points, Ctrl->G.file[g], GC[g].G->header->nx, GC[g].G->header->ny);
           if (Ctrl->G.type[g] == 0 && GMT_Destroy_Data (API, &GC[g].G) != GMT_OK) {
              return (EXIT_FAILURE);
           } else {
            GMT_free_grid (GMT, &GC[g].G, true);
           }
        }

        GMT_free    (GMT, value);
        GMT_free    (GMT, GC);

        return (EXIT_SUCCESS);
}
