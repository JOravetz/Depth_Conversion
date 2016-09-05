NX=218
NY=446
NT=100

START=2300
STOP=3200

### set -x

### extract_delta_v < vel.poly.scaled.su pfile=FIELD_combined_checkshots_2013.FILTERED_sorted.dat verbose=1 > test.poly.scaled.deltav.FILTERED.lis
### awk '{if(NR>1){print $2, $3, $4, $10}}' test.poly.scaled.deltav.FILTERED.lis | sort | uniq > test.poly.scaled.deltav.FILTERED.reform.lis
### sum_data_t < test.poly.scaled.deltav.FILTERED.reform.lis > test.poly.scaled.deltav.FILTERED.reform.sum.lis

### awk '{if($3>='"${START}"'&&$3<='"${STOP}"'){print}}' test.poly.scaled.deltav.FILTERED.reform.sum.lis > test.lis
### echo "Executing 4D interpolation"
### python test2.py test.lis > rbf_interp.test.lis
### awk '{print $2, $3, $1, $4}' rbf_interp.test.lis > stuff
### apply_delta_v_check < test.lis pfile=stuff nx=${NX} ny=${NY} nt=${NT} verbose=1 > bub
### sort -k 7,7n bub > bub1
### cat bub1
### echo

### apply_delta_v < vel.poly.scaled.su pfile=stuff nx=${NX} ny=${NY} nt=${NT} verbose=1 > test.su
### extract_delta_v < test.su pfile=FIELD_combined_checkshots_2013.FILTERED_sorted.dat verbose=1 > foo
### awk '{if($4>='"${START}"'&&$4<='"${STOP}"'){print}}' foo > foo1
### cat foo1

STUFF="stuff.combined.dat"

apply_delta_v.new < vel.poly.scaled.su pfile=${STUFF} nx=${NX} ny=${NY} nt=${NT} verbose=1 > test.su
extract_delta_v < test.su pfile=FIELD_combined_checkshots_2013.FILTERED_sorted.dat verbose=1 > foo
awk '{if($4>='"${START}"'&&$4<='"${STOP}"'){print}}' foo > foo1
cat foo1

sudepthconvert.v1 < ./SEGY_TWT/input1.trint.su vfile=test.su verbose=1 > output.su
segyhdrs < output.su
segywrite < output.su tape=vel.poly.scaled.rb_interp.test.sgy endian=0
