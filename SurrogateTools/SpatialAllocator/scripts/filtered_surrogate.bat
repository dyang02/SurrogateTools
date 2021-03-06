::******************* Filtered Surrogate Run Script **************************
:: This script generates a filtered surrogate based on the MIMS spatial 
:: allocator test case :: (8km over Tennessee).
::
:: Be sure to specify FILTERED_WEIGHT_SHAPES and FILTER_FILE, as those are the
:: main things different from generating an ordinary surrogate.
:: 
:: Script created by : Alison Eyth, Carolina Environmental Program
:: Last edited : June 2005
:: 
:: Dec. 2007, LR -- projection specification changes
::*********************************************************************

@echo off

:: Set installation directory

set BASDIR=..

set DEBUG_OUTPUT=N

:: Set directory for output surrogate and shape files

set WORK_DIR=%BASDIR%\output

:: Set Location of shapefiles
set DATA=%BASDIR%\data

mkdir %WORK_DIR%

:: Set Grid settings
set GRIDDESC=%DATA%\GRIDDESC.txt
set OUTPUT_GRID_NAME=M_08_99NASH

:: Set Location of executable
set MIMSDIR=%BASDIR%
set EXE=srgcreate.exe

:: Set name and path to temporary surrogate file
set SURROGATE_FILE=%WORK_DIR%\tmp_srg.%OUTPUT_GRID_NAME%.txt

:: Set name and path to final merged surrogate file from the spatial allocator
set SRG_FILE=%WORK_DIR%\filtered_srg_%OUTPUT_GRID_NAME%.txt

:: mode of operation for program
set MIMS_PROCESSING=SURROGATE

:: WRITE_QASUM=YES prints surrogate sums by county in file
:: WRITE_SRG_NUMERATOR=YES writes surrogate numerator as comment in file
:: WRITE_SRG_DENOMINATOR=YES writes denominator (county totals) for srg weight
set WRITE_QASUM=YES
set WRITE_SRG_NUMERATOR=YES
set WRITE_SRG_DENOMINATOR=YES

:: Print header info
set WRITE_HEADER=NO

:: Specify type of data files to use
set OUTPUT_FILE_TYPE=RegularGrid
set OUTPUT_FILE_ELLIPSOID=+a=6370997.0,+b=6370997.0
set DATA_FILE_NAME_TYPE=ShapeFile
set WEIGHT_FILE_TYPE=ShapeFile

:: The data polygons should be the shape file containing polygons for the grid
set DATA_FILE_NAME=%DATA%\cnty_tn
set DATA_ID_ATTR=FIPS_CODE
set DATA_FILE_MAP_PRJN=+proj=latlong
set DATA_FILE_ELLIPSOID=+a=6370997.0,+b=6370997.0

:: Set weight projection to that of the EPA files
set WEIGHT_FILE_MAP_PRJN=+proj=lcc,+lat_1=33,+lat_2=45,+lat_0=40,+lon_0=-97
set WEIGHT_FILE_ELLIPSOID=+a=6370997.0,+b=6370997.0

set TIME="time /t"

:: Set the WEIGHT_FUNCTION to NONE in case it is still set from an earlier
:: script execution
set WEIGHT_FUNCTION=NONE

:: Generate surrogate header line
echo Writing header
%MIMSDIR%\%EXE% -header > %SRG_FILE%
if %errorlevel% NEQ 0 (
   goto error
)

:: Generate surrogates for each category
:: For each one, specify:
:: SURROGATE_ID (integer ID to use for the output surrogate)
:: WEIGHT_FILE_NAME (shapefile for weights), 
:: WEIGHT_ATTR_LIST (attribute to weight by (or NONE for area/length/count)
:: OUTPUT_FILE_NAME (optional - name of shapefile of gridded weights to output)
:: FILTERED_WEIGHT_SHAPES (name of filtered shapefile to read weights from)
:: FILTER_FILE (name of the filter file to use)

echo Generating filtered population surrogate
set SURROGATE_ID=8
set WEIGHT_FILE_NAME=%DATA%\tn_pophous
set WEIGHT_ATTR_LIST=POP2000
set FILTER_FILE=%DATA%\filter_tn_pop.txt
set FILTERED_WEIGHT_SHAPES=%WORK_DIR%\tn_filtered_pop
set OUTPUT_FILE_NAME=%WORK_DIR%\filtered_pop_%OUTPUT_GRID_NAME%
%TIME%
%MIMSDIR%\%EXE%

if %errorlevel% == 0 (
   more %SURROGATE_FILE% >> %SRG_FILE%
   del %SURROGATE_FILE%
)

:error
