#! /bin/sh

GRID="trend.grd"

gmtset D_FORMAT %0.4lf
rm -f bub
while read -r LINE ; do
   X=`echo ${LINE} | awk '{print $1}'`
   Y=`echo ${LINE} | awk '{print $2}'`
   Vavg_Seismic=`echo "${X} ${Y}" | grdtrack -Qb -G${GRID} | awk '{print $3}' | sed 's/-//g'`
   echo "${LINE} ${Vavg_Seismic}" >> bub
done < output.dat

while read -r LINE ; do
   DIFF=`echo ${LINE} | awk '{printf "%16.8f", $5-$7}'`
   echo "${LINE} ${DIFF}" 
done < bub
