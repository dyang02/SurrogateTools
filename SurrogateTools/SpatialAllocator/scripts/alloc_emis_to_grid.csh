#! /bin/csh -f
#******************* Allocate Shapefiles Run Script **************************
# Allocates a set of points (emissions)  data to a Regular Grid
# March 2005, BB
# Dec. 2007, LR -- projection specification changes
#*****************************************************************************


setenv DEBUG_OUTPUT Y

# Set executable
setenv MIMSDIR ..
setenv EXE "$MIMSDIR/allocator.exe"

# Set Input Directory
setenv DATADIR $MIMSDIR/data
setenv OUTPUT $MIMSDIR/output

# Select method of spatial analysis

setenv MIMS_PROCESSING ALLOCATE

setenv TIME time

#set "data" shapefile parameters
setenv GRIDDESC $DATADIR/GRIDDESC.txt

#set parameters for file being allocated
setenv INPUT_FILE_NAME $DATADIR/tn_fake_emis.csv
setenv INPUT_FILE_DELIMITER COMMA
setenv INPUT_FILE_TYPE PointFile
setenv INPUT_FILE_MAP_PRJN "+proj=latlong"
setenv INPUT_FILE_XCOL LONG
setenv INPUT_FILE_YCOL LAT
setenv INPUT_FILE_ELLIPSOID "+a=6370997.0,+b=6370997.0"
setenv INPUT_FILE_DELIM COMMA
setenv ALLOCATE_ATTRS NOX,VOC,PM_10
setenv ALLOC_MODE_FILE ALL_AGGREGATE

# Set name and path of resulting shapefile
setenv OUTPUT_FILE_NAME $OUTPUT/gridded_emis
setenv OUTPUT_FILE_TYPE RegularGrid
setenv OUTPUT_GRID_NAME M_08_99NASH
setenv OUTPUT_FILE_MAP_PRJN M_08_99NASH
setenv OUTPUT_FILE_ELLIPSOID "+a=6370997.0,+b=6370997.0"

#echo "Allocating observations to a grid"
$TIME $EXE
