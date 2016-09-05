### extract_delta_v < Cb_Ph3_TWT_Vavg_062013_32b.su pfile=FIELD_combined_checkshots_2013_sorted.dat verbose=1 > test.deltav.lis
### awk '{if (NR>1){print $7, $8}}' test.deltav.lis > input.dat
### supoly < input.dat verbose=0 n=3
### supoly.scale.adjust.velocities < Cb_Ph3_TWT_Vavg_062013_32b.su A=0.9067097523639227052427714 B=0.0000511409140712852511843 > vel.poly.scaled.su
### extract_delta_v < vel.poly.scaled.su pfile=FIELD_combined_checkshots_2013_sorted.dat verbose=1 > test.poly.scaled.deltav.lis
### minmax test.poly.scaled.deltav.lis
### extract_delta_v < vel.poly.scaled.su pfile=FIELD_combined_checkshots_2013.FILTERED_sorted.dat verbose=1 > test.poly.scaled.deltav.FILTERED.lis
### minmax test.poly.scaled.deltav.FILTERED.lis

### awk '{if(NR>1){print $2, $3, $4, $10}}' test.poly.scaled.deltav.FILTERED.lis | sort | uniq > test.poly.scaled.deltav.FILTERED.reform.lis
### sum_data_t < test.poly.scaled.deltav.FILTERED.reform.lis > test.poly.scaled.deltav.FILTERED.reform.sum.lis
### awk '{if($3>=2700&&$3<=2850){print}}' test.poly.scaled.deltav.FILTERED.reform.sum.lis > test.lis
### python test2.py test.lis > rbf_interp.test.lis
### awk '{print $2, $3, $1, $4}' rbf_interp.test.lis > stuff
### apply_delta_v_check < test.lis pfile=stuff nx=301 ny=301 nt=51 verbose=1 > bub
### sort -k 7,7n bub > bub1
### cat bub1

### apply_delta_v < vel.poly.scaled.su pfile=stuff nx=301 ny=301 nt=51 verbose=1 > test.su

sudepthconvert.v1 < ./SEGY_TWT/input1.trint.su vfile=test.su verbose=1 > output.su
segyhdrs < output.su
segywrite < output.su tape=vel.poly.scaled.rb_interp.test.sgy endian=0
