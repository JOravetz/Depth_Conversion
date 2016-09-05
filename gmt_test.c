#include "gmt_dev.h"
#include <limits.h>
#include <stdio.h>
#include "par.h"
#include "su.h"
#include "cwp.h"
#include "segy.h"
#include "header.h"

#define THIS_MODULE_NAME    "gmt_test1"
#define THIS_MODULE_LIB     "core"
#define THIS_MODULE_PURPOSE "Sample grids at specified (x,y) locations"
#define GMT_PROG_OPTIONS "-:>RVabfghinos" GMT_OPT("HMmQ")
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
        struct G {      /* -G<grdfile> */
                bool active;
                unsigned int n_grids;
                char *file[MAX_GRIDS];
                double scale[MAX_GRIDS], lat[MAX_GRIDS];
                int mode[MAX_GRIDS];
                int type[MAX_GRIDS];    /*HIDDEN */
        } G;
};

void *New_gmt_jjo_Ctrl (struct GMT_CTRL *GMT) {     /* Allocate and initialize a new control structure */
        struct GRDTRACK_CTRL *C;
        C = GMT_memory (GMT, NULL, 1, struct GRDTRACK_CTRL);
        return (C);
}

void Free_gmt_testxxx_Ctrl (struct GMT_CTRL *GMT, struct GRDTRACK_CTRL *C) {    /* Deallocate control structure */
        unsigned int g;
        if (!C) return;
        if (C->In.file) free (C->In.file);
        /* for (g = 0; g < C->G.n_grids; g++) if (C->G.file[g]) free (C->G.file[g]); */
        if (C->Out.file) free (C->Out.file);
        GMT_free (GMT, C);
}

int GMT_gmt_testxxx_parse (struct GMT_CTRL *GMT, struct GRDTRACK_CTRL *Ctrl, struct GMT_OPTION *options) {

   int j, mode;
   unsigned int pos, n_errors = 0, ng = 0, n_files = 0, n_units = 0, n_modes = 0;
   char line[GMT_BUFSIZ] = {""}, ta[GMT_LEN64] = {""}, tb[GMT_LEN64] = {""};
   char tc[GMT_LEN64] = {""}, p[GMT_LEN256] = {""}, *c = NULL, X;
   struct GMT_OPTION *opt = NULL;
   struct GMTAPI_CTRL *API = GMT->parent;

   for (opt = options; opt; opt = opt->next) {
      switch (opt->option) {
         case '<':      /* Skip input files */
            if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
            break;
         case '>':      /* Specified output file */
            if (n_files++ == 0 && GMT_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_DATASET))
               Ctrl->Out.file = strdup (opt->arg);
            else
               n_errors++;
               break;
         case 'G':
            Ctrl->G.active = true;
            Ctrl->G.scale[ng] = 1.0;
            if (GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) {
               /* fprintf ( stderr, "Opt->arg = %s\n", opt->arg);
               Ctrl->G.file[ng] = strdup (opt->arg); */
              Ctrl->G.file[ng] = "wb.twt.grd";
               fprintf ( stderr, "Grid File name = %s, ng = %d\n", Ctrl->G.file[ng], ng );
            } else {
               n_errors++;
            }
            ng++;
            break;
            default:    /* Report bad options */
            n_errors += GMT_default_error (GMT, opt->option);
            break;
         }
      }
      Ctrl->G.n_grids = ng;
      n_errors += GMT_check_condition (GMT, Ctrl->G.n_grids == 0, "Syntax error: Must specify -G at least once\n");
      n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");
      n_errors += GMT_check_binary_io (GMT, 2);

      return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int xxx_sample_all_grids (struct GMT_CTRL *GMT, struct GRD_CONTAINER *GC, unsigned int n_grids, bool img, double x_in, double y_in, double value[]) {

   unsigned int g, n_in=0, n_set=0;
   double x, y, x0 = 0.0, y0 = 0.0;

   y = y_in;
   x = x_in;

   n_in++;
   value[g] = GMT_get_bcr_z (GMT, GC[g].G, x, y);
   fprintf ( stderr, "Value = %8.2f, g = %d, x = %f, y = %f\n", value[g], g, x, y );

   if (!GMT_is_dnan (value[g])) n_set++;

   if (n_in == 0) return (-1);
   return (n_set);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmt_testxxx_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

char *sdoc[] = {NULL};

segy tr;

int main (int argc, char **argv) {

   char *grdfile;
   short verbose;
   double *value, wesn[4];
   double x, y;

        int status, error, ks;
        uint64_t n_points = 0, n_read = 0;
        unsigned int g, k;
        bool img_conv_needed = false, some_outside = false;
        char line[GMT_BUFSIZ] = {""}, run_cmd[BUFSIZ] = {""}, *cmd = NULL;

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

           /*----------------------- Standard module initialization and parsing ----------------------*/

        if (API == NULL) return (GMT_NOT_A_SESSION);
        options = GMT_Create_Options (API, mode, args); if (API->error) return (API->error);    /* Set or get option list */

        /* Parse the command-line arguments */

        GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
        if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
        Ctrl = New_gmt_testxxx_Ctrl (GMT);      /* Allocate and initialize a new control structure */
        if ((error = GMT_gmt_testxxx_parse (GMT, Ctrl, options))) Return (error);

        /*---------------------------- This is the gmt_test1 main code ----------------------------*/

        cmd = GMT_Create_Cmd (API, options);
        sprintf (run_cmd, "# %s %s", GMT->init.module_name, cmd);       /* Build command line argument string */
        GMT_free (GMT, cmd);

        GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid(s)\n");

        GMT_memset (wesn, 4, double);
        if (GMT->common.R.active) GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);     /* Specified a subset */
        GMT_set_pad (GMT, 2U);  /* Ensure space for BCs in case an API passed pad == 0 */

        fprintf ( stderr, "getting ready to execute GMT_memory, Ctrl = %d\n", Ctrl->G.n_grids );
        GC = GMT_memory (GMT, NULL, Ctrl->G.n_grids, struct GRD_CONTAINER);
        for (g = 0; g < Ctrl->G.n_grids; g++) {
           GC[g].type = Ctrl->G.type[g];
           if (Ctrl->G.type[g] == 0) {  /* Regular GMT grids */
              if ((GC[g].G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->G.file[g], NULL)) == NULL) Return (API->error);
              if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, GC[g].G->header->wesn, 4, double);
              if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->G.file[g], GC[g].G) == NULL) Return (API->error);
              GMT_memcpy (GMT->common.R.wesn, wesn, 4, double);
           }
        }
        value = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);

        bool pure_ascii = false;
        int ix, iy, n_fields, rmode;
        uint64_t n_out = 0;
        double *in = NULL, *out = NULL;
        char record[GMT_BUFSIZ];
        bool gmt_skip_output (struct GMT_CTRL *C, double *cols, uint64_t n_cols);

        if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) Return (API->error);
        if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) Return (API->error);
        if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_HEADER_ON) != GMT_OK) Return (API->error);
        if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) Return (API->error);

        GMT_memset (line, GMT_BUFSIZ, char);
        pure_ascii = GMT_is_ascii_record (GMT);
        ix = (GMT->current.setting.io_lonlat_toggle[GMT_IN]);   iy = 1 - ix;
        rmode = (pure_ascii && GMT_get_cols (GMT, GMT_IN) >= 2) ? GMT_READ_MIXED : GMT_READ_DOUBLE;

        do {
           if ((in = GMT_Get_Record (API, rmode, &n_fields)) == NULL) {
              if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);
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
              if ((error = GMT_set_cols (GMT, GMT_OUT, n_out))) Return (error);
           }
           n_read++;

           status = xxx_sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, in[GMT_X], in[GMT_Y], value);
           fprintf ( stderr, "status = %d\n", status );

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

        if (GMT_End_IO (API, GMT_IN,  0) != GMT_OK) Return (API->error);
        if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) Return (API->error);
        if (out) GMT_free (GMT, out);
        if (some_outside) GMT_Report (API, GMT_MSG_VERBOSE, "Some input points were outside the grid domain(s).\n");

        for (g = 0; g < Ctrl->G.n_grids; g++) {
           GMT_Report (API, GMT_MSG_VERBOSE, "Sampled %" PRIu64 " points from grid %s (%d x %d)\n", n_points, Ctrl->G.file[g], GC[g].G->header->nx, GC[g].G->header->ny);
           if (Ctrl->G.type[g] == 0 && GMT_Destroy_Data (API, &GC[g].G) != GMT_OK) {
              Return (API->error);
           } else {
              GMT_free_grid (GMT, &GC[g].G, true);
           }
        }

        GMT_free    (GMT, value);
        GMT_free    (GMT, GC);
        GMT_set_pad (GMT, API->pad);

   return (0);
}
