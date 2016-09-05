#! /bin/sh

GRID="top_reservoir_final.depth.dat.trimmed.sample.smooth.grd"

rm -f Top_Res_WMP_diff.dat temp.dat
while read -r line ; do
   well_name=`echo "${line}" | awk '{print $1}'`
   xloc=`echo ${line} | awk '{print $3}'`
   yloc=`echo ${line} | awk '{print $4}'`
   zval=`echo ${line} | awk '{print $5*-1}'`
   Z_GRID=`echo "${xloc} ${yloc}" | grdtrack -G"${GRID}" -Qc | awk '{print $3*-1}'`
   DIFF=`bc -l <<END
      scale=2
      ${zval} - ${Z_GRID}
END`
   echo "${well_name} ${xloc}, ${yloc}, ${zval}, ${Z_GRID}, ${DIFF}" |\
   awk '{ printf "Working on Well = %-12s  X-LOC = %12.2f  Y-LOC = %12.2f  Depth_WMP (tvdss) = %12.2f  Depth_GRID = %12.2f  Difference = %12.4f\n", $1, $2, $3, $4, $5, $6 }' >> temp.dat
done < Top_Res_WMP.dat

sort -k 21,21n temp.dat > Top_Res_WMP_diff.dat
cat Top_Res_WMP_diff.dat
