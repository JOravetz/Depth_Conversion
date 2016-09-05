#! /bin/sh

RANGE="-R511797/527792/152443/169653"

FNAME="test.deltav.lis"
### FNAME="test.deltav.FILTERED.lis"
### FNAME="FIELD_combined_checkshots_2013.FILTERED.deltav.lis"

awk '{if (NR>1){print $1}}' ${FNAME} | sort | uniq > wells.lis

rm -f bubba 
while read -r WELL ; do
   grep -w -e ${WELL} ${FNAME} > input.whole.dat
   awk '{print $7, $8}' input.whole.dat > input.dat
   AVG_X=`awk -f average.awk 2 input.whole.dat | awk '{print $7}'`
   AVG_Y=`awk -f average.awk 3 input.whole.dat | awk '{print $7}'`
   NUM=`wc -l input.dat | awk '{print $1}'`
   FOO=`supoly < input.dat verbose=0 n=3`
   A=`echo ${FOO} | awk '{print $1}'`
   B=`echo ${FOO} | awk '{print $2}'`
   echo "${WELL} ${AVG_X} ${AVG_Y} ${A} ${B} ${NUM}" | awk '{ printf "%-16s %12.2f %12.2f %20.16f %20.16f %5d\n", $1, $2, $3, $4, $5, $6}' 
   echo "${WELL} ${AVG_X} ${AVG_Y} ${A} ${B} ${NUM}" | awk '{ printf "%-16s %12.2f %12.2f %20.16f %20.16f %5d\n", $1, $2, $3, $4, $5, $6}' >> bubba
done < wells.lis

sort -k 4,4n bubba > output.lis
echo
cat output.lis

gmtset D_FORMAT %0.16lf
awk '{print $2, $3, $4}' output.lis > data
LOW=`minmax -C data | awk '{print $5}'`
HIGH=`minmax -C data | awk '{print $6}'`

rm -f coeff.A.grd
surface data -Gcoeff.A.grd -I901+/1101+ -S15000 -T0 ${RANGE} -Ll${LOW} -Lu${HIGH}

awk '{print $2, $3, $5}' output.lis > data
LOW=`minmax -C data | awk '{print $5}'`
HIGH=`minmax -C data | awk '{print $6}'`

rm -f coeff.B.grd
surface data -Gcoeff.B.grd -I901+/1101+ -S15000 -T0 ${RANGE} -Ll${LOW} -Lu${HIGH}

mbm_grdplot -Icoeff.A.grd -X -V -G2 -A10/30 -B2000 &
mbm_grdplot -Icoeff.B.grd -X -V -G2 -A10/30 -B2000 &
