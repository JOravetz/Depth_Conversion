set -x

START=2700
TWT=`bc -l <<END
   scale=0
   ${START} + 2
END`

RANGE="-R511797/527792/152443/169653"
SEISMIC_VELOCITY="vel.poly.scaled.su"
DELTAV_LISTING="test.poly.scaled.deltav.lis"
CHECKSHOTS="FIELD_combined_checkshots_2013.dat"

SECS=`bc -l <<END
   scale=3
   ${START} / 1000.0
END`

suwind < ${SEISMIC_VELOCITY} tmin=${SECS} tmax=${SECS} > vel.slice.${START}.su
awk '{if($4>='"${START}"'&&$4<='"${TWT}"'){print}}' ${CHECKSHOTS} > check.${START}.lis
awk '{if(NR>1&&$4>='"${START}"'&&$4<='"${TWT}"'){print $2, $3, $10}}' ${DELTAV_LISTING} | sort | uniq > test.dat

gmtset D_FORMAT %0.4lf
MINMAX=`minmax -C test.dat`
LOW=`echo ${MINMAX} | awk '{print $5}'`
HIGH=`echo ${MINMAX} | awk '{print $6}'`

surface test.dat -Gtest.grd -I901+/1101+ -S16000 -T0.0 ${RANGE} -Ll${LOW} -Lu${HIGH}

sustrip < vel.slice.${START}.su head=headers | b2a n1=1 format=1 > data
sugethw < vel.slice.${START}.su key=sx,sy output=geom > geom
paste geom data > vels

gmtset D_FORMAT %0.4lf
MINMAX=`minmax -C vels`
LOW=`echo ${MINMAX} | awk '{print $5}'`
HIGH=`echo ${MINMAX} | awk '{print $6}'`

surface vels -Gvels.grd -I901+/1101+ -S100 -T1.0 ${RANGE} -Ll${LOW} -Lu${HIGH}
grdmath vels.grd test.grd ADD = sum.grd
grdtrack geom -Gsum.grd -Qb -Z -bos | supaste head=headers ns=1 > slab.su

extract_delta_v < slab.su pfile=check.${START}.lis verbose=0 > foo
cat foo
