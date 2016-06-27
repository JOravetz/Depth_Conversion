#!/bin/bash

set -x

HORIZON=
DX_OUT=25
DX_SMOOTH=4000
NGRID=700+
NGRID1=200+
RADIUS=1000
RADIUS2=1000
TFACTOR=1.0
SIGN=-1.0

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
RANGE=`minmax -I${NGRID} ${HORIZON}`

minmax ${HORIZON} | sed 's/<//g' | sed 's/>//g' | sed 's|/| |g' > ${HORIZON}.minmax
Ll=`cat ${HORIZON}.minmax | awk '{print $9}'`
Lu=`cat ${HORIZON}.minmax | awk '{print $10}'`

echo "Lower data bound = ${Ll}, Upper data bound = ${Lu}"

blockmean ${HORIZON} -I${NGRID} ${RANGE} -bo > ${HORIZON}.blockmean.dat
surface ${HORIZON}.blockmean.dat -bi3 -G${HORIZON}.surface.grd -I${NGRID} ${RANGE} -V -T${TFACTOR} -Ll${Ll} -Lu${Lu}
nearneighbor ${HORIZON}.blockmean.dat -bi3 -G${HORIZON}.neighbor.grd -I${NGRID1} ${RANGE} -V -S${RADIUS2}
grdsample ${HORIZON}.neighbor.grd -G${HORIZON}.neighbor.resamp.grd -I${NGRID} -V -Q ${RANGE}
grdmath ${HORIZON}.surface.grd ${HORIZON}.neighbor.resamp.grd OR = ${HORIZON}.temp.grd
grdmath ${HORIZON}.temp.grd ${SIGN} MUL = ${HORIZON}.trimmed.grd
### grdfilter ${HORIZON}.trimmed.grd -G${HORIZON}.trimmed.smooth.grd -Fm${DX_SMOOTH} -D0 ${RANGE} -I${NGRID} -V
cp ${HORIZON}.trimmed.grd ${HORIZON}.trimmed.smooth.grd
grdsample ${HORIZON}.trimmed.smooth.grd -G${HORIZON}.trimmed.sample.smooth.grd -I${DX_OUT} -V -Q ${RANGE}
gmtset D_FORMAT %0.2lf
mbm_grdplot -I${HORIZON}.trimmed.sample.smooth.grd -X -V -G2 -A2/30 -Q
