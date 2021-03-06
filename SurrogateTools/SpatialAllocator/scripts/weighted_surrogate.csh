#!/bin/csh -f
#******************* Weighted Surrogate Run Script ******************
# This script demonstrates the use of a weight function in the MIMS
# Spatial Allocator test case (8 km over Tennessee) using a function
# for the weight.  This is just an example - the weight function is not
# supposed to represent a realistic case.
#
# Script created by : Ben Brunk, Carolina Environmental Program
# Last edited : June 2005
#
# Dec. 2007, LR -- projection specification changes
#*********************************************************************

setenv DEBUG_OUTPUT Y

# Set installation directory
setenv BASDIR ..

# Set directory for output surrogate and shape files
setenv WORK_DIR $BASDIR/output

# Set Location of shapefiles
setenv DATA $BASDIR/data

# Set Grid settings
setenv GRIDDESC $DATA/GRIDDESC.txt
setenv OUTPUT_GRID_NAME M_08_99NASH
setenv OUTPUT_FILE_ELLIPSOID "+a=6370997.0,+b=6370997.0"

# Set Location of executable
setenv MIMSDIR $BASDIR
setenv EXE srgcreate.exe

# Set name and path to temporary surrogate file
setenv SURROGATE_FILE $WORK_DIR/tmp_srg.$OUTPUT_GRID_NAME.txt

# Set name and path to final merged surrogate file from spatial tool
setenv SRG_FILE $WORK_DIR/weighted_srg_$OUTPUT_GRID_NAME.txt

# WRITE_QASUM=YES prints surrogate sums by county in file
# WRITE_SRG_NUMERATOR=YES writes surrogate numerator as comment in file
# WRITE_SRG_DENOMINATOR=YES writes denminator (county totals) for srg weight
setenv WRITE_QASUM YES
setenv WRITE_SRG_NUMERATOR YES
setenv WRITE_SRG_DENOMINATOR YES

# Print header info
setenv WRITE_HEADER NO

# Specify type of data files to use
setenv OUTPUT_FILE_TYPE RegularGrid
setenv DATA_FILE_NAME_TYPE ShapeFile
setenv WEIGHT_FILE_TYPE ShapeFile

# The data polygons should be the shape file containing county polygons 
setenv DATA_FILE_NAME $DATA/cnty_tn
setenv DATA_ID_ATTR FIPS_CODE
setenv DATA_FILE_MAP_PRJN "+proj=latlong"
setenv DATA_FILE_ELLIPSOID "+a=6370997.0,+b=6370997.0"

# Set weight projection to that of the EPA files
setenv WEIGHT_FILE_MAP_PRJN "+proj=lcc,+lat_1=33,+lat_2=45,+lat_0=40,+lon_0=-97"
setenv WEIGHT_FILE_ELLIPSOID "+a=6370997.0,+b=6370997.0"

date +%r

# Generate surrogate header line
echo Writing header
$MIMSDIR/$EXE -header > $SRG_FILE
if ( $status != 0 ) then
   echo "Error writing surrogate header line"
   exit
endif

# set FILTER_FILE to NONE in case it was set in an earlier script execution
setenv FILTER_FILE NONE

echo Generating ports surrogate using a weight function
setenv SURROGATE_ID 4
setenv WEIGHT_FILE_NAME $DATA/tn_ports
setenv WEIGHT_ATTR_LIST USE_FUNCTION
setenv WEIGHT_FUNCTION "BERTHS*2"
setenv OUTPUT_FILE_NAME $WORK_DIR/weighted_ports_$OUTPUT_GRID_NAME

date +%r
$MIMSDIR/$EXE
if ( $status == 0 ) then
   more $SURROGATE_FILE >> $SRG_FILE
   rm $SURROGATE_FILE
endif
