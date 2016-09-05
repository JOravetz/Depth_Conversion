#! /bin/sh

RANGE="-R511797/527792/152443/169653"

while [ X"$1" != X-- ]
do
    case "$1" in
   -slice) SLICE="$2"
           shift 2
           ;;
   -stuff) STUFF="$2"
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
shift    

TWT=`echo ${SLICE} | awk -F".slice.su" '{print $1}'`
echo "Working on Average-Velocity at TWT = ${TWT} milliseconds"

sustrip < ${SLICE} | b2a n1=1 format=1 > $$.datafile
paste geom $$.datafile > $$.data

gmtset D_FORMAT %0.4lf
MINMAX=`minmax -C $$.data`
LOW=`echo ${MINMAX} | awk '{print $5}'`
HIGH=`echo ${MINMAX} | awk '{print $6}'`

rm -f $$.data.grd
surface $$.data -G$$.data.grd -I901+/1101+ -S100 -T1.0 ${RANGE} -Ll${LOW} -Lu${HIGH}

NUM=`wc -l ${STUFF} | awk '{print $1}'`

if [ "${NUM}" -gt "0" ] ; then
   gmtset D_FORMAT %0.4lf

   MINMAX=`minmax -C ${STUFF}`
   LOW=`echo ${MINMAX} | awk '{print $5}'`
   HIGH=`echo ${MINMAX} | awk '{print $6}'`

   rm -f $$.diff.grd
   surface ${STUFF} -G$$.diff.grd -I901+/1101+ -S16000 -T0 ${RANGE} -Ll${LOW} -Lu${HIGH}

   grdmath $$.diff.grd $$.data.grd ADD = scaled.${TWT}.grd
   grdtrack geom -Gscaled.${TWT}.grd -Qb -Z -bos | supaste head=headers ns=1 > scaled.${TWT}.su

else
   cp ${SLICE} scaled.${TWT}.su
fi

rm -f ${SLICE} ${STUFF} $$.datafile $$.data $$.data.grd $$.bub $$.diff.grd
