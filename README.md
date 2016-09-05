# Objective 

Intersect well-deviation surveys with a seismic average-velocity SU (SEG-Y) volume to extract twt/average-velocity checkshot values along the wellbore.

## Procedure 

Download __convert.vavg.depth.c__ and __Makefile_convert.vavg.depth_program__.  Edit the makefile example and save as "Makefile".  You must have Seismic Unix installed and in your PATH environment variable

Make a listing of deviation surveys and execute a script to loop over the list (__wrapper.process.deviation.sh__)

```
#! /bin/bash

### wrapper script to loop over each deviation survey - stored in deviations.lis ###

ls -alt *deviation_survey_from_Program.dat | awk '{print $9}' > deviations.lis
while read -r LINE ; do
   process.deviation.sh -s ${LINE}
done < deviations.lis

```
Here is a listing of the __process.deviation.sh__ bash script

```
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
```
Here is an example of the __convert.vavg.depth__ program output:

```
   520645.65    160690.85       0.0000       0.0000    1540.9706
   520645.65    160690.85     926.1871     697.0000    1505.0954
   520645.84    160691.12    1041.2964     784.4692    1506.7164
   520645.90    160691.20    1057.3697     796.8488    1507.2283

   ...

   520568.85    160227.16    2734.8805    2402.0611    1756.6114
   520565.11    160212.98    2752.7253    2425.6311    1762.3489
   520563.03    160205.15    2762.5082    2438.6317    1765.5200
   520560.11    160194.31    2775.9055    2456.6332    1769.9689
   520559.97    160193.80    2776.5351    2457.4821    1770.1791
```
