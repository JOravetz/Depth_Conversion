### extract_delta_v < Cb_Ph3_TWT_Vavg_062013_32b.su pfile=ALL_FIELD_Campanian_WMPs_and_Checkshots_COMBINED.dat verbose=1 > ALL_FIELD_Campanian_WMPs_and_Checkshots_COMBINED.deltav.lis
### extract_delta_v < Cb_Ph3_TWT_Vavg_062013_32b.su pfile=FIELD___COMBINED_WMP.reform.dat verbose=1 > FIELD___COMBINED_WMP.reform.deltav.lis
### awk '{if (NR>1){print $7, $8}}' FIELD___COMBINED_WMP.reform.deltav.lis > input.dat
### supoly < input.dat verbose=0 n=3

### supoly.scale.adjust.velocities < Cb_Ph3_TWT_Vavg_062013_32b.su A=1.0191215263238995181183100 B=-0.0000141614798616718897886 > vel.poly.scaled.bill.chase.su
### extract_delta_v < vel.poly.scaled.bill.chase.su pfile=FIELD___COMBINED_WMP.reform.dat verbose=1 > FIELD___COMBINED_WMP.reform.poly.scaled.deltav.lis
### awk '{if(NR>1){print $2, $3, $4, $10}}' FIELD___COMBINED_WMP.reform.poly.scaled.deltav.lis | sort | uniq > test.dat
### python test2.py test.dat > bub
### awk '{print $2, $3, $1, $4}' bub > bub1
### apply_delta_v.new < vel.poly.scaled.bill.chase.su pfile=bub1 nx=218 ny=446 nt=100 verbose=1 > test.su
### extract_delta_v < test.su pfile=FIELD___COMBINED_WMP.reform.dat verbose=1 > FIELD___COMBINED_WMP.reform.poly.scaled.interp.deltav.lis
### sudepthconvert.v1 < ./SEGY_TWT/input1.trint.su vfile=test.su verbose=1 > output.su && segyhdrs < output.su && segywrite < output.su tape=vel.poly.scaled.rb_interp.bill.chase.test.sgy endian=0

### match.wmp.xy.to.deltav < test.MISSING.dat pfile=FIELD_Well_Tops.MISSING.dat verbose=1 > test.MISSING.filtered.by.wmps.dat
### cp test.dat test.new.dat
### awk '{print $1, $2, $3, $4}' test.MISSING.filtered.by.wmps.dat >> test.new.dat
### sort test.new.dat > test.new1.dat

extract_delta_v < vel.poly.scaled.bill.chase.su pfile=ALL_FIELD___MISSING_COMBINED_WMP.reform.dat verbose=1 > new.deltav.lis
awk '{if(NR>1){print $2, $3, $4, $10}}' new.deltav.lis > test.new1.dat

echo "Performing 4D Rbf Interpolation"
python test2.py test.new1.dat > bub
awk '{print $2, $3, $1, $4}' bub > bub1
echo "Appling the Delta-V corrections to background average velocities"
apply_delta_v.new < vel.poly.scaled.bill.chase.su pfile=bub1 nx=210 ny=445 nt=100 verbose=1 > test.su

### awk '{print $1, $2}' test.MISSING.filtered.by.wmps.dat > filter.lis
### cp FIELD___COMBINED_WMP.reform.dat ALL_FIELD___MISSING_COMBINED_WMP.reform.dat
### grep -w -f filter.lis FIELD_wells_with_checkshots_MISSING_WMPs.FILTERED.dat >> ALL_FIELD___MISSING_COMBINED_WMP.reform.dat

echo "Extracting the Residual Delta-V values from the corrected average velocity grid"
extract_delta_v < test.su pfile=ALL_FIELD___MISSING_COMBINED_WMP.reform.dat verbose=1 > ALL_FIELD___MISSING_COMBINED_WMP.reform.deltav.lis
cat ALL_FIELD___MISSING_COMBINED_WMP.reform.deltav.lis
