#! /bin/csh -f
#******************* Overlay Run Script **************************************
#
#  Example of Overlay mode for MIMS Spatial Allocator
#  This example uses a shapefile overlaying a Regular Grid
#  Created 3/30/2005 by Ben Brunk, Carolina Environmental Program
#
#  Dec. 2007, LR -- projection specification changes
#*****************************************************************************

# Set debug output
setenv DEBUG_OUTPUT Y

# Set executable
setenv MIMSDIR ..
setenv EXE "$MIMSDIR/allocator.exe"

# Set Input Directory
setenv DATADIR $MIMSDIR/data
setenv OUTPUT $MIMSDIR/output

# Select method of spatial analysis

setenv MIMS_PROCESSING OVERLAY

setenv TIME time

setenv OVERLAY_SHAPE $DATADIR/cnty_tn.shp
setenv OVERLAY_TYPE ShapeFile
setenv OVERLAY_MAP_PRJN "+proj=latlong"
setenv OVERLAY_ELLIPSOID "+a=6370997.0,+b=6370997.0"
# will print row and col
setenv OVERLAY_ATTRS COL,ROW
setenv INPUT_FILE_NAME M_08_99NASH
setenv INPUT_FILE_TYPE RegularGrid
setenv INPUT_FILE_MAP_PRJN M_08_99NASH
setenv GRIDDESC $DATADIR/GRIDDESC.txt
setenv INPUT_FILE_ELLIPSOID "+a=6370997.0,+b=6370997.0"
setenv OVERLAY_OUT_TYPE Stdout 
setenv OVERLAY_OUT_NAME Stdout 
setenv OVERLAY_OUT_DELIM COMMA
setenv WRITE_HEADER Y


echo "Overlaying grid with county shapefile"
$TIME $EXE
