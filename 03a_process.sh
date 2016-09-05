#! /bin/sh

VWATER="1452.50"
WB_TWT="wb.twt.grd"
T_RES_TWT="top_reservoir_final.dat"

awk '{print $2, $3, $5}' output.1.lis > k.lis
gmtset D_FORMAT %0.10lf
./03_process.sh -s k.lis
grdmath k.lis.trimmed.sample.grd -1.0 MUL = k.grd
### grdmath ${WB_TWT} -1.0 MUL = wb.twt.grd
horizon-vzerok < ${T_RES_TWT} vzero=${VWATER} coeff_x=${WB_TWT} coeff_x2=k.grd verbose=1 > top_reservoir_final.depth.dat
make.gmt.grid.sh -s top_reservoir_final.depth.dat
