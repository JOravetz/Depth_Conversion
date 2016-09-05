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

sustrip < ${SLICE} | b2a n1=1 format=1 > $$.data
paste geom $$.data > $$.datain
xyz2grd $$.datain -G$$.data.grd -I901+/1101+ ${RANGE} -V

gmtset D_FORMAT %0.4lf

sum_data_t < ${STUFF} verbose=1 > $$.test1.dat
python test1.py $$.test1.dat > $$.test2.dat
awk '{print $2, $3, $4}' $$.test2.dat > $$.test3.dat
rm -f $$.diff.grd
xyz2grd $$.test3.dat -G$$.diff.grd -I901+/1101+ ${RANGE} -V

grdmath $$.diff.grd $$.data.grd ADD = scaled.${TWT}.grd
grdtrack geom -Gscaled.${TWT}.grd -Q0.10b -S -Z -bos | suaddhead ns=1101 | sushw key=dt a=1000 > scaled.${TWT}.su

rm -f ${SLICE} ${STUFF} $$.datain $$.data $$.data.grd $$.diff.grd $$.test?.dat
