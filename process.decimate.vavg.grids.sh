#! /bin/sh

gmtset D_FORMAT %0.8lf
rm -f combined.twt.vavg.dat
while read -r LINE ; do
   FNAME=`echo ${LINE} | awk -F".vavg.grd" '{print $1}'`
   grdsample ${LINE} -G${FNAME}.vavg.sample.grd -I100+ -V -Q
   grdsample ${FNAME}.grd -G${FNAME}.sample.grd -I100+ -V -Q
   grd2xyz -S ${FNAME}.sample.grd > part1
   grd2xyz -S ${FNAME}.vavg.sample.grd | awk '{print $3}' > part2
   paste part1 part2 > ${FNAME}.sample.twt.vavg.dat
   paste part1 part2 >> combined.twt.vavg.dat
done < vavg.lis
