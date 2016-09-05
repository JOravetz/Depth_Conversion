#! /bin/sh

GRID="stuff.trimmed.sample.smooth.grd"

gmtset D_FORMAT %0.4lf
while read -r LINE ; do
   X=`echo ${LINE} | awk '{print $1}'`
   Y=`echo ${LINE} | awk '{print $2}'`
   Vavg_Seismic=`echo "${X} ${Y}" | grdtrack -Qb -G${GRID} | awk '{print $3}' | sed 's/-//g'`
   echo "${LINE} ${Vavg_Seismic}"
done < top_reservoir.twt.checkshots.intersect.dat
