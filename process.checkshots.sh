#! /bin/sh

GRID="bottom_twt_msec.grd"
FNAME=`echo ${GRID} | awk -F".grd" '{print $1}'`

rm -f ${FNAME}.checkshots.intersect.dat
while read -r LINE ; do
   intersect_checkshot_with_grid verbose=1 pfile=./checkshots/${LINE} coeff_x=${GRID} >> ${FNAME}.checkshots.intersect.dat
done < ./checkshots/checkshots.reform.lis
echo
sort ${FNAME}.checkshots.intersect.dat | grep -v -e "nan" > bub
mv bub ${FNAME}.checkshots.intersect.dat
cat ${FNAME}.checkshots.intersect.dat
