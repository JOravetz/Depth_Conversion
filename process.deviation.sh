#!/bin/bash

FNAME=
VERBOSE=1

### Check program options.
while [ X"$1" != X-- ]
do
    case "$1" in
       -s) FNAME="$2"
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

if [ -z "${FNAME}" ] ; then
   echo "Have to code input well deviation survey file name (-s) parameter --> exiting"
   exit
fi

echo "Working on well deviation survey = ${FNAME}"
cat ${FNAME}
awk '{if(NF==10){print $2, $3, $5}}' ${FNAME} | awk '{if (NR>3){print}}' > deviation.dat
echo "X Y and TVD values"
cat deviation.dat
minmax deviation.dat
RANGE=`awk '{if(NF==10){print $2, $3, $5}}' ${FNAME} | awk '{if (NR>3){print}}' | minmax -C -I200/200`
echo "RANGE = ${RANGE}"
XMIN=`echo ${RANGE} | awk '{print $1}'`
XMAX=`echo ${RANGE} | awk '{print $2}'`
YMIN=`echo ${RANGE} | awk '{print $3}'`
YMAX=`echo ${RANGE} | awk '{print $4}'`
echo "Windowing the input seismic average velocity data"
suwind < ../Cb_Ph3_TWT_Vavg_062013_32b.su key=sx min=${XMIN} max=${XMAX} | suwind key=sy min=${YMIN} max=${YMAX} > input.su
surange < input.su
echo "Intersecting well trajectory with seismic average velocity data -> extracting checkshot values at intersection points"
convert.vavg.depth < input.su verbose=${VERBOSE} dfile=deviation.dat > ${FNAME}.checkshot.dat
