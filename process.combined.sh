#! /bin/sh

SEISMIC_VELOCITIES="Cb_Ph3_TWT_Vavg_062013_32b.su"

### GRID="top_reservoir.twt.grd"
### GRID="JJO_Top_C01_Peak_TWT_msec.dat.trimmed.smooth.grd"
### GRID="JJO_80MaSB_Peak_FINAL_TWT_msec.dat.trimmed.smooth.grd"

### --------------------------- Start Here ------------------------------------- ###

### SURFACE="JJO_Seafloor_TWT.reform.dat"
SURFACE="Top_Reservoir_ROB_TWT_msec_19Nov2015_picks.dat"
### SURFACE="Top_C_Sand_TWT.reform.dat"
### SURFACE="JJO_80MaSB_Peak_FINAL_TWT_msec.dat"

FNAME=`echo ${SURFACE} | awk -F".dat" '{print $1}'`
GRID="${FNAME}.grd"

RANGE_VELOCITIES="-R511797/527792/152443/169653"
MINMAX=`minmax -C ${SURFACE}`
LOW_TWT=`echo ${MINMAX} | awk '{print $5}'`
HIGH_TWT=`echo ${MINMAX} | awk '{print $6}'`

surface ${SURFACE} -G${GRID} -S2000 -T0.5 -I721+ ${RANGE_VELOCITIES} -Ll${LOW_TWT} -Lu${HIGH_TWT} -V

### FNAME=`echo ${GRID} | awk -F".grd" '{print $1}'`

echo "Intersecting checkshots with horizon = ${FNAME}"

rm -f ${FNAME}.checkshots.intersect.dat
while read -r LINE ; do
   intersect_checkshot_with_grid verbose=1 pfile=./Bill_Hay_Checkshots_2016/${LINE} coeff_x=${GRID} >> ${FNAME}.checkshots.intersect.dat
done < ./Bill_Hay_Checkshots_2016/checkshots.reform.lis
echo
sort ${FNAME}.checkshots.intersect.dat | grep -v -e "nan" > bub
mv bub ${FNAME}.checkshots.intersect.dat
cat ${FNAME}.checkshots.intersect.dat

echo "Extracting seismic average velocities from SEG-Y volume at the TWT horizon"
suextract < ${SEISMIC_VELOCITIES} coeff_x1=${GRID} verbose=1 > stuff

MINMAX=`minmax -C stuff`
LOW_TWT=`echo ${MINMAX} | awk '{print $5}'`
HIGH_TWT=`echo ${MINMAX} | awk '{print $6}'`
surface stuff -Gstuff.surface.grd -S2000 -T0.5 -I721+ ${RANGE_VELOCITIES} -Ll${LOW_TWT} -Lu${HIGH_TWT} -V

### make.gmt.grid.sh -s stuff
### sleep 10

echo "Compare extracted seismic with reference well average velocities"

GRID1="stuff.surface.grd"

gmtset D_FORMAT %0.4lf
rm -f output.dat
while read -r LINE ; do
   X=`echo ${LINE} | awk '{print $1}'`
   Y=`echo ${LINE} | awk '{print $2}'`
   Vavg_Seismic=`echo "${X} ${Y}" | grdtrack -Qb -G${GRID1} | awk '{print $3}' | sed 's/-//g'`
   echo "${LINE} ${Vavg_Seismic}" >> output.dat
done < ${FNAME}.checkshots.intersect.dat

echo "Compute differences between Checkshot and Seismic average velocities"
awk '{print $0, $5-$6}' output.dat > output1.dat

echo "Compute mean and standard deviation of average-velocity differences and filter data by 1.5 standard-deviations"

MEAN_STDEV_VALUES=`awk '{for(i=7;i<=NF;i++) {sum[i] += $i; sumsq[i] += ($i)^2}} END {for (i=7;i<=NF;i++) { printf "%.15f %.15f \n", sum[i]/NR, sqrt((sumsq[i]-sum[i]^2/NR)/NR)} }' output1.dat`
MEAN=`echo ${MEAN_STDEV_VALUES} | awk '{print $1}'`
STDEV=`echo ${MEAN_STDEV_VALUES} | awk '{print $2}'`
echo "Mean average velocity difference between checkshot and seismic data = ${MEAN}, Standard Deviation = ${STDEV}"
awk '{ print $0, ($7-'"${MEAN})"'/'"${STDEV}"' }' output1.dat | awk '{if ( sqrt ( $8 *$8 ) < 1.5 ) { $8=""; print $0 }}' > output1.filtered.dat

echo "Compute annd apply a polynomial trend relationship to Seismic average velocities"

awk '{print $6, $5}' output1.filtered.dat > input.dat
FOO=`supoly < input.dat verbose=0 n=3`
A_COEFF=`echo ${FOO} | awk '{print $1}'`
B_COEFF=`echo ${FOO} | awk '{print $2}'`

gmtset D_FORMAT %0.16lf
### grdmath ${GRID1} -1 MUL = stuff.grd
grdmath ${GRID1} ${GRID1} MUL ${B_COEFF} MUL = part1.grd
grdmath ${GRID1} ${A_COEFF} MUL part1.grd ADD = trend.grd

echo "Measure the difference between trend-corrected seismic average velocities and reference checkshot values"

GRID2="trend.grd"

gmtset D_FORMAT %0.4lf
rm -f bub
while read -r LINE ; do
   X=`echo ${LINE} | awk '{print $1}'`
   Y=`echo ${LINE} | awk '{print $2}'`
   Vavg_Seismic=`echo "${X} ${Y}" | grdtrack -Qb -G${GRID2} | awk '{print $3}' | sed 's/-//g'`
   echo "${LINE} ${Vavg_Seismic}" >> bub
done < output1.filtered.dat

rm -f output2.dat
while read -r LINE ; do
   DIFF=`echo ${LINE} | awk '{printf "%16.8f", $5-$8}'`
   echo "${LINE} ${DIFF}" >> output2.dat
done < bub

echo "Grid the residual velocity corrections and apply to the trend-corrected values"
awk '{print $1, $2, $9}' output2.dat > residual.vavg.dat

### DXDY=`grdinfo -I ${GRID2}`
### RANGE=`grdinfo -I721+ ${GRID2}`
### MINMAX=`minmax -C residual.vavg.dat`
### LOW=`echo ${MINMAX} | awk '{print $5}'`
### HIGH=`echo ${MINMAX} | awk '{print $6}'`
### echo "DXDY = ${DXDY}, RANGE = ${RANGE}, LOW = ${LOW}, HIGH = ${HIGH}"

MINMAX=`minmax -C residual.vavg.dat`
LOW_TWT=`echo ${MINMAX} | awk '{print $5}'`
HIGH_TWT=`echo ${MINMAX} | awk '{print $6}'`

surface residual.vavg.dat -Gresidual.vavg.grd -S16000 -T0 -I721+ ${RANGE_VELOCITIES} -Ll${LOW_TWT} -Lu${HIGH_TWT} -V

### surface residual.vavg.dat -Gstuff.grd -S16000 -T0 -I721+ ${RANGE} -Ll${LOW} -Lu${HIGH} -V
### grdsample stuff1.surface.grd -Gresidual.vavg.grd ${DXDY} ${RANGE} -V -Q

grdmath residual.vavg.grd trend.grd ADD = ${FNAME}.vavg.grd

gmtset D_FORMAT %0.2lf
mbm_grdplot -I${FNAME}.vavg.grd -X -V -G2 -A3/30 -B2000
sleep 10

GRID3="${FNAME}.vavg.grd"

echo "Measure the final average-velocity differences after application of residual corrections"

gmtset D_FORMAT %0.4lf
rm -f bub
while read -r LINE ; do
   X=`echo ${LINE} | awk '{print $1}'`
   Y=`echo ${LINE} | awk '{print $2}'`
   Vavg_Seismic=`echo "${X} ${Y}" | grdtrack -Qb -G${GRID3} | awk '{print $3}' | sed 's/-//g'`
   echo "${LINE} ${Vavg_Seismic}" >> bub
done < output2.dat

rm -f output3.dat
while read -r LINE ; do
   DIFF=`echo ${LINE} | awk '{printf "%16.8f", $5-$10}'`
   echo "${LINE} ${DIFF}" >> output3.dat
done < bub

cp output3.dat ${FNAME}.vavg.output.dat
grdmath ${FNAME}.vavg.grd -1 MUL = ${FNAME}.vavg.neg.grd
gmtset D_FORMAT %0.2lf
mbm_grdplot -I${FNAME}.vavg.neg.grd -X -V -G2 -A3/30 -B2000
sleep 10
ps2pdf ${FNAME}.vavg.neg.grd.ps
acroread ${FNAME}.vavg.neg.grd.pdf &
cat output3.dat
