#! /bin/csh -f
#******************* Allocate Shapefiles Run Script **************************
# Allocates a polygon shapefile's data to an I/O API gridded file
# July 2005, CAS 
#  Dec. 2007, LR -- projection specification changes
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
setenv INPUT_FILE_NAME $DATADIR/tn_pophous
setenv INPUT_FILE_TYPE ShapeFile
setenv INPUT_FILE_MAP_PRJN "+proj=lcc,+lat_1=33,+lat_2=45,+lat_0=40,+lon_0=-97"
setenv INPUT_FILE_ELLIPSOID "+a=6370997.0,+b=6370997.0"
setenv ALLOCATE_ATTRS POP2000,HOUSEHOLDS
setenv ALLOC_MODE_FILE ALL_AGGREGATE

# Set name and path of resulting shapefile
setenv OUTPUT_FILE_NAME $OUTPUT/grid_pophous.ncf
setenv OUTPUT_FILE_TYPE IoapiFile
setenv OUTPUT_GRID_NAME M_08_99NASH
setenv OUTPUT_FILE_MAP_PRJN M_08_99NASH
setenv OUTPUT_FILE_ELLIPSOID "+a=6370997.0,+b=6370997.0"

#echo "Allocating tracts to county for population and housing"
$TIME $EXE
