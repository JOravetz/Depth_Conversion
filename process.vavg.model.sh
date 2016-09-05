###  Water Layer

echo "(1) Working on Water-Layer"

rm top.grd bottom.grd bottom_vavg.grd
ln -s zero.grd top.grd
ln -s JJO_Seafloor_TWT.reform.grd bottom.grd
ln -s JJO_Seafloor_TWT.reform.vavg.grd bottom_vavg.grd
update.average.velocities < Cb_Ph3_TWT_Vavg_062013_32b.su verbose=1 > vel.new.su

### Top Reservoir to Seafloor
echo "(2) Working from Top Reservoir to Seafloor"

rm top.grd bottom.grd bottom_vavg.grd
ln -s JJO_Seafloor_TWT.reform.grd top.grd
ln -s Top_Reservoir_USER_TWT_msec_19Nov2013_picks.grd bottom.grd
ln -s Top_Reservoir_USER_TWT_msec_19Nov2013_picks.vavg.grd bottom_vavg.grd
update.average.velocities < vel.new.su verbose=1 > vel1.new.su

### Top-C to Top Reservoir
echo "(3) Working from Top-C to Top Reservoir"

rm top.grd bottom.grd bottom_vavg.grd
ln -s Top_Reservoir_USER_TWT_msec_19Nov2013_picks.grd top.grd
ln -s Top_C_Sand_TWT.reform.grd bottom.grd
ln -s Top_C_Sand_TWT.reform.vavg.grd bottom_vavg.grd
update.average.velocities < vel1.new.su verbose=1 > vel.new.su

### Base Reservoir to Top Reservoir
echo "(4) Working from Base Reservoir to Top-C"

rm top.grd bottom.grd bottom_vavg.grd
ln -s Top_C_Sand_TWT.reform.grd top.grd
ln -s JJO_80MaSB_Peak_FINAL_TWT_msec.grd bottom.grd
ln -s JJO_80MaSB_Peak_FINAL_TWT_msec.vavg.grd bottom_vavg.grd
update.average.velocities < vel.new.su verbose=1 > vel1.new.su

### Four Seconds to Base Reservoir

echo "(5) Working from Four Seconds to Base Reservoir"

rm top.grd bottom.grd bottom_vavg.grd
ln -s JJO_80MaSB_Peak_FINAL_TWT_msec.grd top.grd
ln -s four_seconds.grd bottom.grd
ln -s four_seconds.vavg.grd bottom_vavg.grd
update.average.velocities < vel1.new.su verbose=1 > vel.new.su
