#! /bin/sh

#GRID="top_reservoir.twt.grd"
GRID="bottom_twt_msec.grd"
FNAME=`echo ${GRID} | awk -F".grd" '{print $1}'`

echo "Measure the difference between trend-corrected seismic average velocities and reference checkshot values"

GRID2="trend.grd"

gmtset D_FORMAT %0.4lf
rm -f bub
while read -r LINE ; do
   X=`echo ${LINE} | awk '{print $1}'`
   Y=`echo ${LINE} | awk '{print $2}'`
   Vavg_Seismic=`echo "${X} ${Y}" | grdtrack -Qb -G${GRID2} | awk '{print $3}' | sed 's/-//g'`
   echo "${LINE} ${Vavg_Seismic}" >> bub
done < output.dat

rm -f output1.dat
while read -r LINE ; do
   DIFF=`echo ${LINE} | awk '{printf "%16.8f", $5-$7}'`
   echo "${LINE} ${DIFF}" >> output1.dat
done < bub

echo "Grid the residual velocity corrections and apply to the trend-corrected values"

awk '{print $1, $2, $8}' output1.dat > residual.vavg.dat
DXDY=`grdinfo -I ${GRID2}`
RANGE=`grdinfo -I721+ ${GRID2}`
MINMAX=`minmax -C residual.vavg.dat`
LOW=`echo ${MINMAX} | awk '{print $5}'`
HIGH=`echo ${MINMAX} | awk '{print $6}'`

echo "DXDY = ${DXDY}, RANGE = ${RANGE}, LOW = ${LOW}, HIGH = ${HIGH}"

surface residual.vavg.dat -Gstuff.grd -S16000 -T0 -I721+ ${RANGE} -Ll${LOW} -Lu${HIGH} -V
grdsample stuff.grd -Gresidual.vavg.grd ${DXDY} ${RANGE} -V -Q
grdmath residual.vavg.grd trend.grd ADD = vavg.grd
grdmath vavg.grd -1 MUL = ${FNAME}.vavg.pos.grd

gmtset D_FORMAT %0.2lf
mbm_grdplot -Ivavg.grd -X -V -G2 -A3/30 -B2000
sleep 5

GRID3="vavg.grd"

echo "Measure the final average-velocity differences after application of residual corrections"

gmtset D_FORMAT %0.4lf
rm -f bub
while read -r LINE ; do
   X=`echo ${LINE} | awk '{print $1}'`
   Y=`echo ${LINE} | awk '{print $2}'`
   Vavg_Seismic=`echo "${X} ${Y}" | grdtrack -Qb -G${GRID3} | awk '{print $3}' | sed 's/-//g'`
   echo "${LINE} ${Vavg_Seismic}" >> bub
done < output1.dat

rm -f output2.dat
while read -r LINE ; do
   DIFF=`echo ${LINE} | awk '{printf "%16.8f", $5-$9}'`
   echo "${LINE} ${DIFF}" >> output2.dat
done < bub

cp output2.dat ${FNAME}.vavg.output.dat
cat output2.dat
ps2pdf vavg.grd.ps
acroread vavg.grd.pdf &
