/* This program transforms BELD3 land use data to the modeling grid of interest. */

#ifdef USE_IOAPI

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "iodecl3.h"
#include "io.h"
#include "mims_evs.h"
#include "shapefil.h"
#include "mims_spatl.h"

#define MAX_TILES 24    /* number of tiles of data */
#define MAX_VARS  230   /* number of variables in data */
#define NUM_FILES 3     /* number of files per tile */

/* list of abbreviations to use for each file */     
const char abbrev[NUM_FILES][5] = {"_a", "_b", "_tot"};

/* environment variables */
const char IOAPI_FILE_OUT[16] = "IOAPI_FILE_OUT";
const char IOAPI_FILE_IN[16] = "IOAPI_FILE_IN";

/* program name */
char *prog_name;
char *prog_version = "BELD3 to SMOKE convertor Version 1.3 - 1/25/2008\n";

int main(int argc, char *argv[])
{
  /* environment variable values */
  char debug[10];            /* print debug messages flag */
  char gridname[NAMLEN3+1];  /* output grid name */
  char griddesc[256];        /* grid description file */
  char alloc_prog_name[256]; /* name of allocator program */
  char input_dir[128];       /* input data directory */
  char tmp_dir[128];         /* tmp directory for intermediate files */
  char output_pre[128];      /* output prefix */
  char out_type[15];         /*output file type: RegularGrid, EGrid, or Polygon*/
  char out_ellip[256];       /*output ellipsoid*/

  /* allocatable arrays */
  float *inData;             /* values read in from I/O API files */
  float *outData;            /* summed output values */
  float *qaData;             /* summed data across variables for QA */

  /* fixed size arrays */
  char var_names[MAX_VARS][NAMLEN3+1];    /* variable names */
  int  var_files[MAX_VARS][MAX_TILES][2]; /* variable-to-file mapping array */
  int  tilelist[MAX_TILES];               /* list of tiles to process */
  int i;

  /* other local variables */
  int  filenum;              /* loop indices */
  int  tileidx, tilenum;
  int  varidx, varnum;
  int  recno;
  int  row, col;
  int  num_tiles;            /* number of tiles to process */
  int  files_processed;      /* number of files processed */

  float pctdiff;             /* percent difference in summed values */

  IOAPI_Bdesc3 bdesc;        /* I/O API file description - non-character data */
  IOAPI_Bdesc3 bdesc_grid;
  IOAPI_Cdesc3 cdesc;        /* I/O API file description - character data */
  IOAPI_Cdesc3 cdesc_grid;
  
  FILE *fileptr;             /* file pointer */
  struct stat status;        /* file status buffer */

  char file_name[256];       /* generated complete file name */
  char varname[NAMLEN3+1];   /* variable name */
  char mesg[256];            /* message buffer */
  char debugOutput[10];

  extern int debug_output;
/* ========================================================================== */

  prog_name = argv[0];
  if(getEnvtValue(ENVT_DEBUG_OUTPUT, debugOutput))
  {
        if(!strcmp(debugOutput, "Y"))
        {
            debug_output = 1;
        }
        else if(!strcmp(debugOutput, "N"))
        {
            debug_output = 0;
        }
        else
        {
            sprintf(mesg, "%s can only be set to 'Y' or 'N'",
                    ENVT_DEBUG_OUTPUT);
            ERROR(prog_name, mesg, 2);
        }
  }
  else
  {
        sprintf(mesg, "Please specify a %s of 'Y' or 'N' in your script",
                ENVT_DEBUG_OUTPUT);
        ERROR(prog_name, mesg, 2);
  }

  MESG(prog_version);

  if(!getEnvtValue("OUTPUT_GRID_NAME", gridname))
  {
    sprintf(mesg, "%s environment variable not set", "OUTPUT_GRID_NAME");
    ERROR(prog_name, mesg, 1);
  }
  
  if(!getEnvtValue("OUTPUT_FILE_TYPE", out_type))
  {
    sprintf(mesg, "%s environment variable not set", "OUTPUT_FILE_TYPE");
    ERROR(prog_name, mesg, 1);
  }

   if(!getEnvtValue("OUTPUT_FILE_ELLIPSOID", out_ellip))
  {
    sprintf(mesg, "%s environment variable not set", "OUTPUT_FILE_ELLIPSOID");
    ERROR(prog_name, mesg, 1);
  }

  if(!getEnvtValue("GRIDDESC", griddesc))
  {
    sprintf(mesg, "%s environment variable not set", "GRIDDESC");
    ERROR(prog_name, mesg, 1);
  }

  if(!getEnvtValue("INPUT_DATA_DIR", input_dir))
  {
    sprintf(mesg, "%s environment variable not set", "INPUT_DATA_DIR");
    ERROR(prog_name, mesg, 1);
  }

  if(!getEnvtValue("TMP_DATA_DIR", tmp_dir))
  {
    sprintf(mesg, "%s environment variable not set", "TMP_DATA_DIR");
    ERROR(prog_name, mesg, 1);
  }
  
  if(!getEnvtValue("OUTPUT_FILE_PREFIX", output_pre))
  {
    sprintf(mesg, "%s environment variable not set", "OUTPUT_FILE_PREFIX");
    ERROR(prog_name, mesg, 1);
  }

  /* use overlay mode of spatial allocator to determine which tiles to process */
  if(debug_output)
  {
    setenv("DEBUG_OUTPUT", "Y", 1);
  }
  else
  {
    setenv("DEBUG_OUTPUT", "N", 1);
  }
  
  setenv("MIMS_PROCESSING", "OVERLAY", 1);
  
  setenv("OVERLAY_TYPE", out_type, 1);
  setenv("OVERLAY_SHAPE", gridname, 1);
  setenv("OVERLAY_MAP_PRJN", gridname, 1);
  setenv("OVERLAY_ELLIPSOID", out_ellip, 1);
  setenv("GRIDDESC", griddesc, 1);
  
  setenv("OVERLAY_ATTRS", "TILES24_ID", 1);
  setenv("OVERLAY_OUT_TYPE", "DelimitedFile", 1);
  sprintf(file_name, "%stiles.txt", tmp_dir);
  setenv("OVERLAY_OUT_NAME", file_name, 1);
  setenv("OVERLAY_OUT_DELIM", " ", 1);
  setenv("WRITE_HEADER", "N", 1);
  
  setenv("INPUT_FILE_TYPE", "ShapeFile", 1);
  sprintf(file_name, "%stiles24", input_dir);
  setenv("INPUT_FILE_NAME", file_name, 1);
  setenv("INPUT_FILE_MAP_PRJN", 
         "+proj=lcc,+lon_0=-90.0,+lat_1=30.0,+lat_2=60.0,+lat_0=40.0", 1);
  setenv("INPUT_FILE_ELLIPSOID", "+a=6370997.0,+b=6370997.0", 1);
 
  /* AME: made this a variable on 1/25/08 - should now support Windows & Linux */
  if(!getEnvtValue("ALLOCATOR_EXE", alloc_prog_name))
  {
    sprintf(mesg, "%s environment variable is not set", "ALLOCATOR_EXE");
    ERROR(prog_name, mesg, 1);
  }

  if(system(alloc_prog_name))
  {
    sprintf(mesg, "Problem finding allocator.exe in %s ",alloc_prog_name);
    ERROR(prog_name, mesg, 2);
  }
  
  /* read tile list output from spatial allocator */
  sprintf(file_name, "%stiles.txt", tmp_dir);
  if((fileptr = fopen(file_name, "r")) == NULL)
  {
    sprintf(mesg, "Could not open intermediate file %s", file_name);
    ERROR(prog_name, mesg, 2);
  }
  
  while(fgets(mesg, sizeof(mesg), fileptr) != NULL)
  {
     tilenum = atoi(mesg);
     tilelist[tilenum-1] = tilenum;
  }
  
  /*get rid of elements with 0 title number*/
  num_tiles=0;
  for (i=0;i<MAX_TILES;i++)
  {
    if (tilelist[i] != 0)
    {
     tilelist[num_tiles] = tilelist[i];
     num_tiles++;
    } 
  }

  sprintf(mesg,"num_tiles = %d\n", num_tiles);
  MESG(mesg); 

  /* use spatial allocator to process each tile */
  if(debug_output)
  {
    setenv("DEBUG_OUTPUT", "Y", 1);
  }
  else
  {
    setenv("DEBUG_OUTPUT", "N", 1);
  }
  
  setenv("MIMS_PROCESSING", "ALLOCATE", 1);
  
  setenv("GRIDDESC", griddesc, 1);

  files_processed = 0;
  for(tileidx = 0; tileidx < num_tiles; ++tileidx)
  {
    tilenum = tilelist[tileidx];
    for(filenum = 0; filenum < NUM_FILES; ++filenum)
    {
      /* check if file exists */
      buildFileName(file_name, input_dir, filenum, tilenum, 0);
      if(stat(file_name, &status) == 0)
      {
        files_processed++;
      
        setenv("INPUT_FILE_NAME", file_name, 1);
        setenv("INPUT_FILE_TYPE", "IoapiFile", 1);
    
        setenv("ALLOCATE_ATTRS", "ALL", 1);
        setenv("ALLOC_MODE_FILE", "ALL_AVERAGE", 1);
    
        buildFileName(file_name, tmp_dir, filenum, tilenum, 1);
        
        /* delete intermediate file if it exists */
        if(stat(file_name, &status) == 0)
        {
          if(remove(file_name) != 0)
          {
            sprintf(mesg, "Could not delete existing intermediate file %s", 
                    file_name);
            ERROR(prog_name, mesg, 2);
          }
        }
        
        setenv("OUTPUT_FILE_NAME", file_name, 1);
        setenv("OUTPUT_FILE_TYPE", "IoapiFile", 1);
        setenv("OUTPUT_GRID_NAME", gridname, 1);
        setenv("OUTPUT_FILE_ELLIPSOID", out_ellip, 1);

        printf("Processing tile %d, file %d...\n", tilenum, filenum+1);
        fflush(stdout);

        if(system(alloc_prog_name))
        {
          sprintf(mesg, "Problem running %s",alloc_prog_name);
          ERROR(prog_name, mesg, 2);
        }
      }
      else 
      {
         sprintf(mesg, "File %s does not exist",file_name);
         WARN(mesg);
      }
    }  /* end loop over files */
  }  /* end loop over tiles */

  /* check that at least one file was processed */
  if(files_processed == 0)
  {
    sprintf(mesg, "Could not process any tiles files. %s %s",
            "Please check your INPUT_DATA_DIR directory", input_dir);
    ERROR(prog_name, mesg, 2);
  }
  
  /* create output file headers and store master list of variable names */
  
  /* get grid description from just created intermediate file */
  buildFileName(file_name, tmp_dir, 0, tilelist[0], 1);
  getDescription(file_name, &bdesc_grid, &cdesc_grid);
  
  /* get variable names from master variable files */
  varidx = 0;
  for(filenum = 0; filenum < NUM_FILES; ++filenum)
  {
    sprintf(file_name, "%svarlist%s.ncf", input_dir, abbrev[filenum]);
    getDescription(file_name, &bdesc, &cdesc);
    
    /* store list of variable names */
    if(filenum != 2)
    {
      for(varnum = 0; varnum < bdesc.nvars; ++varnum)
      {
        strNullTerminate(var_names[varidx], cdesc.vname[varnum], NAMLEN3);
        ++varidx;
      }
    }
    
    /* reset grid description */
    bdesc.p_alp = bdesc_grid.p_alp;
    bdesc.p_bet = bdesc_grid.p_bet;
    bdesc.p_gam = bdesc_grid.p_gam;
    bdesc.xcent = bdesc_grid.xcent;
    bdesc.ycent = bdesc_grid.ycent;
    bdesc.xorig = bdesc_grid.xorig;
    bdesc.yorig = bdesc_grid.yorig;
    bdesc.xcell = bdesc_grid.xcell;
    bdesc.ycell = bdesc_grid.ycell;
    
    bdesc.gdtyp = bdesc_grid.gdtyp;
    memcpy(cdesc.gdnam, cdesc_grid.gdnam, sizeof(cdesc.gdnam));
    bdesc.ncols = bdesc_grid.ncols;
    bdesc.nrows = bdesc_grid.nrows;

    /* open output file */
    sprintf(file_name, "%s%s.ncf", output_pre, abbrev[filenum]);
    setenv(IOAPI_FILE_OUT, file_name, 1);
    
    if(!open3c(IOAPI_FILE_OUT, &bdesc, &cdesc, FSUNKN3, prog_name))
    {
      sprintf(mesg, "Could not open I/O API file %s", file_name);
      ERROR(prog_name, mesg, 2);
    }
    
    if(!close3c(IOAPI_FILE_OUT))
    {
      sprintf(mesg, "Could not close I/O API file %s", file_name);
      ERROR(prog_name, mesg, 2);
    }
  }
  
  /* loop through intermediate files and create variable-to-file mapping */
  memset(var_files, 0, MAX_VARS * MAX_TILES * 2 * sizeof(int));
  
  for(tileidx = 0; tileidx < num_tiles; ++tileidx)
  {
    tilenum = tilelist[tileidx];
  
    for(filenum = 0; filenum < NUM_FILES-1; ++filenum)
    {
      buildFileName(file_name, tmp_dir, filenum, tilenum, 1);
      if(stat(file_name, &status) == 0)
      {
        getDescription(file_name, &bdesc, &cdesc);
        
        for(varidx = 0; varidx < bdesc.nvars; ++varidx)
        {
          strNullTerminate(varname, cdesc.vname[varidx], NAMLEN3);
          
          for(varnum = 0; varnum < MAX_VARS; ++varnum)
          {
            if(strcmp(varname, var_names[varnum]) == 0)
            {
              var_files[varnum][tilenum-1][filenum] = 1;
              break;
            }
          }  /* end loop over master variables list */
        }  /* end loop over variables in file */
      }
    }  /* end loop over files */
  }  /* end loop over tiles */
  
  /* allocate space to store input and output data */
  inData = malloc(bdesc.nrows * bdesc.ncols * sizeof(float));
  outData = malloc(bdesc.nrows * bdesc.ncols * sizeof(float));
  qaData = malloc(bdesc.nrows * bdesc.ncols * sizeof(float));
  
  /* initialize QA data array */
  memset(qaData, 0, bdesc.nrows * bdesc.ncols * sizeof(float));
  
  /* loop through master list of variables */
  for(varnum = 0; varnum < MAX_VARS; ++varnum)
  {
    strncpy(varname, var_names[varnum], NAMLEN3+1);
    
    /* initialize output data array */
    memset(outData, 0, bdesc.nrows * bdesc.ncols * sizeof(float));
    
    /* loop through tiles */
    for(tileidx = 0; tileidx < num_tiles; ++tileidx)
    {
      tilenum = tilelist[tileidx];
      
      for(filenum = 0; filenum < NUM_FILES-1; ++filenum)
      {
        if(var_files[varnum][tilenum-1][filenum])
        {
          buildFileName(file_name, tmp_dir, filenum, tilenum, 1);
          setenv(IOAPI_FILE_IN, file_name, 1);
          
          if(!open3c(IOAPI_FILE_IN, &bdesc, &cdesc, FSREAD3, prog_name))
          {
            sprintf(mesg, "Could not open I/O API file %s", file_name);
            ERROR(prog_name, mesg, 2);
          }

          if(!read3c(IOAPI_FILE_IN, varname, 1, 0, 0, inData))
          {
            sprintf(mesg, "Could not read variable %s from I/O API file %s",
                    varname, file_name);
            ERROR(prog_name, mesg, 2);
          }
          
          if(!close3c(IOAPI_FILE_IN))
          {
            sprintf(mesg, "Could not close I/O API file %s", file_name);
            ERROR(prog_name, mesg, 2);
          }
          
          /* sum data from each tile into output array */
          for(recno = 0; recno < bdesc.nrows * bdesc.ncols; ++recno)
          {
            outData[recno] = outData[recno] + inData[recno];
            qaData[recno] = qaData[recno] + inData[recno];
          }
          
          break;
        }
      }  /* end loop over files */
    }  /* end loop over tiles */
    
    /* open correct output file if needed */
    if(varnum == 0)
    {
      sprintf(file_name, "%s%s.ncf", output_pre, abbrev[0]);
      setenv(IOAPI_FILE_OUT, file_name, 1);
      
      if(!open3c(IOAPI_FILE_OUT, &bdesc, &cdesc, FSRDWR3, prog_name))
      {
        sprintf(mesg, "Could not open I/O API file %s", file_name);
        ERROR(prog_name, mesg, 2);
      }
    }
    else if(varnum == 120)
    {
      if(!close3c(IOAPI_FILE_OUT))
      {
        sprintf(mesg, "Could not close I/O API file %s", file_name);
        ERROR(prog_name, mesg, 2);
      }
      
      sprintf(file_name, "%s%s.ncf", output_pre, abbrev[1]);
      setenv(IOAPI_FILE_OUT, file_name, 1);
      
      if(!open3c(IOAPI_FILE_OUT, &bdesc, &cdesc, FSRDWR3, prog_name))
      {
        sprintf(mesg, "Could not open I/O API file %s", file_name);
        ERROR(prog_name, mesg, 2);
      }
    }
  
    if(!write3c(IOAPI_FILE_OUT, varname, 0, 0, outData))
    {
      sprintf(mesg, "Could not write variable %s to I/O API file %s",
              varname, file_name);
      ERROR(prog_name, mesg, 2);
    }
    
  }  /* end loop over variables */
  
  /* check that all variables sum to 100% */
  for(row = 0; row < bdesc.nrows; ++row)
  {
    for(col = 0; col < bdesc.ncols; ++col)
    {
      pctdiff = qaData[(row * bdesc.ncols) + col] - 100.;
      
      if(abs(pctdiff) > 0.01)
      {
        printf("WARNING: Summed land use differs from 100%% by %.2f%% for row %d, column %d\n",
               pctdiff, row+1, col+1);
      }
    }
  }
  
  if(!close3c(IOAPI_FILE_OUT))
  {
    sprintf(mesg, "Could not close I/O API file %s", file_name);
    ERROR(prog_name, mesg, 2);
  }
  
  /* combine totals files */
  memset(outData, 0, bdesc.nrows * bdesc.ncols * sizeof(float));
  
  strcpy(varname, "FOREST");
  filenum = 2;  
  for(tileidx = 0; tileidx < num_tiles; ++tileidx)
  {
    tilenum = tilelist[tileidx];
    
    buildFileName(file_name, tmp_dir, filenum, tilenum, 1);
    setenv(IOAPI_FILE_IN, file_name, 1);
    
    if(!open3c(IOAPI_FILE_IN, &bdesc, &cdesc, FSREAD3, prog_name))
    {
      sprintf(mesg, "Could not open I/O API file %s", file_name);
      ERROR(prog_name, mesg, 2);
    }

    if(!read3c(IOAPI_FILE_IN, varname, 1, 0, 0, inData))
    {
      sprintf(mesg, "Could not read variable %s from I/O API file %s",
              varname, file_name);
      ERROR(prog_name, mesg, 2);
    }
      
    if(!close3c(IOAPI_FILE_IN))
    {
      sprintf(mesg, "Could not close I/O API file %s", file_name);
      ERROR(prog_name, mesg, 2);
    }
    
    /* sum data from each tile into output array */
    for(recno = 0; recno < bdesc.nrows * bdesc.ncols; ++recno)
    {
      outData[recno] = outData[recno] + inData[recno];
    }
  }  /* end loop over tiles */
  
  sprintf(file_name, "%s%s.ncf", output_pre, abbrev[filenum]);
  setenv(IOAPI_FILE_OUT, file_name, 1);
  
  if(!open3c(IOAPI_FILE_OUT, &bdesc, &cdesc, FSRDWR3, prog_name))
  {
    sprintf(mesg, "Could not open I/O API file %s", file_name);
    ERROR(prog_name, mesg, 2);
  }
 
  if(!write3c(IOAPI_FILE_OUT, varname, 0, 0, outData))
  {
    sprintf(mesg, "Could not write variable %s to I/O API file %s",
            varname, file_name);
    ERROR(prog_name, mesg, 2);
  }
  
  if(!close3c(IOAPI_FILE_OUT))
  {
    sprintf(mesg, "Could not close I/O API file %s", file_name);
    ERROR(prog_name, mesg, 2);
  }

  return 0;
}

/* ========================================================================== */

int buildFileName(char *dest, const char *dir, 
                  int filenum, int tilenum, int isIntermediate)
{
  return(sprintf(dest, 
                 isIntermediate ? "%sb3%s.tile%d.nzero.out.ncf"
                                : "%sb3%s.tile%d.nzero.ncf", 
                 dir, abbrev[filenum], tilenum));
}

/* ========================================================================== */

int getDescription(const char *file_name, IOAPI_Bdesc3 *bdesc,
                   IOAPI_Cdesc3 *cdesc)
{
  char mesg[256];

  setenv(IOAPI_FILE_IN, file_name, 1);
  
  if(!open3c(IOAPI_FILE_IN, bdesc, cdesc, FSREAD3, prog_name))
  {
    sprintf(mesg, "Could not open I/O API file %s", file_name);
    ERROR(prog_name, mesg, 2);
  }
  
  if(!desc3c(IOAPI_FILE_IN, bdesc, cdesc))
  {
    sprintf(mesg, "Could not get description of I/O API file %s", file_name);
    ERROR(prog_name, mesg, 2);
  }
  
  if(!close3c(IOAPI_FILE_IN))
  {
    sprintf(mesg, "Could not close I/O API file %s", file_name);
    ERROR(prog_name, mesg, 2);
  }
  
  return 0;
}

#else

int main(int argc, char *argv[])
{
    printf("I/O API support is required for beld3smk.exe\n");
    return 0;
}

#endif
