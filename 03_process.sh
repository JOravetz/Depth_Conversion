#!/bin/bash

set -x

HORIZON=
DX_OUT=25
NGRID=400+
RADIUS=10000
RADIUS2=10000
TFACTOR=0.20
SIGN=-1.0
ContractorRID="wb.twt.grd"

### Check program options.
while [ X"$1" != X-- ]
do
    case "$1" in
       -s) HORIZON="$2"
           shift 2
           ;;
      -dx) DX_OUT="$2"
           shift 2
           ;;
      -ng) NGRID="$2"
           shift 2
           ;;
       -r) RADIUS="$2"
           shift 2
           ;;
      -r2) RADIUS2="$2"
           shift 2
           ;;
      -sm) DX_SMOOTH="$2"
           shift 2
           ;;
       -t) TFACTOR="$2"
           shift 2
           ;;
       -v) VERBOSE="$2"
           shift 2
           ;;
   -debug) echo "DEBUG ON"
           set -x
           DEBUG="yes"
           trap '' SIGHUP SIGINT SIGQUIT SIGTERM
           shift 1
           ;;
       -*) echo "${program}: Invalid parameter $1: ignored." 1>&2
           shift
           ;;
        *) set -- -- $@
           ;;
    esac
done
shift           # remove -- of arguments

if [ -z "${HORIZON}" ] ; then
   echo "Have to code input horizon (-s) parameter -> exiting"
   exit
fi

echo "Working on horizon = ${HORIZON}"

gmtset D_FORMAT %0.10lf
### RANGE=`minmax -I${NGRID} ${HORIZON}`
RANGE=`grdinfo -I25/25 ${ContractorRID}`

minmax ${HORIZON} | sed 's/<//g' | sed 's/>//g' | sed 's|/| |g' > ${HORIZON}.minmax
Ll=`cat ${HORIZON}.minmax | awk '{print $9}'`
Lu=`cat ${HORIZON}.minmax | awk '{print $10}'`

echo "Lower data bound = ${Ll}, Upper data bound = ${Lu}"

### blockmean ${HORIZON} -I${NGRID} ${RANGE} -bo > ${HORIZON}.blockmean.dat
cp ${HORIZON} ${HORIZON}.blockmean.dat
### surface ${HORIZON}.blockmean.dat -bi3 -G${HORIZON}.surface.grd -I${NGRID} ${RANGE} -V -T${TFACTOR} -Ll${Ll} -Lu${Lu} -S${RADIUS}
surface ${HORIZON}.blockmean.dat -G${HORIZON}.surface.grd -I${NGRID} ${RANGE} -V -T${TFACTOR} -Ll${Ll} -Lu${Lu} -S${RADIUS}
grdmath ${HORIZON}.surface.grd ${SIGN} MUL = ${HORIZON}.trimmed.grd
grdsample ${HORIZON}.trimmed.grd -G${HORIZON}.trimmed.sample.grd -I${DX_OUT} -V -Q ${RANGE}
gmtset D_FORMAT %0.2lf
mbm_grdplot -I${HORIZON}.trimmed.sample.grd -X -V -G2 -A0.1/270 ${RANGE} -Q
