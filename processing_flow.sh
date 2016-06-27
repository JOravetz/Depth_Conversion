suvzerok_with_water_seis_vavg_horizon < Cb_Ph3_TWT_Vavg_062013_32b.su coeff_x=wb.twt.grd coeff_x2=bottom_twt_msec.grd mode=2 verbose=1 > output.mode_2.variable.vzero.k.dat
awk '{print $9, $11, $14}' output.mode_2.variable.vzero.k.dat > vzero.seismic.dat
awk '{print $9, $11, $17}' output.mode_2.variable.vzero.k.dat > k.seismic.dat
make.gmt.grid.sh -s vzero.seismic.dat
make.gmt.grid.sh -s k.seismic.dat
