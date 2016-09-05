awk '{if(NR>1){print $2, $3, $4, $10}}' test.poly.scaled.deltav.FILTERED.lis | sort | uniq > test.poly.scaled.deltav.FILTERED.reform.lis
sum_data_t < test.poly.scaled.deltav.FILTERED.reform.lis verbose=1 > test.poly.scaled.deltav.FILTERED.reform.sum.lis

START=2300
END=3200
WINDOW=100
INC=50

STOP=`bc -l <<END
   scale=0
   ${START} + ${WINDOW}
END`

set -x

while [ "${STOP}" -le "${END}" ] ; do
   echo "START = ${START}, STOP = ${STOP}"
   awk '{if($3>='"${START}"'&&$3<='"${STOP}"'){print}}' test.poly.scaled.deltav.FILTERED.reform.sum.lis > test.lis
   python test2.py test.lis > rbf_interp.test.${START}.${STOP}.dat
   awk '{print $2, $3, $1, $4}' rbf_interp.test.${START}.${STOP}.dat > stuff.${START}.${STOP}.dat
   echo "Checking the Delta-V interpolated corrections versus input control points"
   apply_delta_v_check < test.lis pfile=stuff.${START}.${STOP}.dat nx=218 ny=446 nt=100 verbose=1 > bub.${START}.${STOP}.dat
   sort -k 7,7n bub.${START}.${STOP}.dat > bub1
   cat bub1

START=`bc -l <<END
   scale=0
   ${START} + ${INC}
END`

STOP=`bc -l <<END
   scale=0
   ${START} + ${WINDOW}
END`

done

### apply_delta_v < vel.poly.scaled.su pfile=stuff nx=301 ny=301 nt=51 verbose=1 > test.su
### sudepthconvert.v1 < ./SEGY_TWT/input1.trint.su vfile=test.su verbose=1 > output.su
### segyhdrs < output.su
### segywrite < output.su tape=vel.poly.scaled.rb_interp.test.sgy endian=0
