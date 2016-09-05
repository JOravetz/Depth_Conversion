# Depth_Conversion

#### Convert depth horizon picks to TWT using current V0/K model

```
inverse-horizon-vzerok coeff_x=wb.twt.grd coeff_x2=K.below.ml.lis.trimmed.sample.grd verbose=1 < 80MaSB_final_grid.dat > 80MaSB_final_grid_TWT_msec.dat
```
#### Grid the TWT horizon and link it to bottom_twt_msec.grd

```
make.gmt.grid.sh -s 80MaSB_final_grid_TWT_msec.dat
```
#### Compute Vzero and K from the TWT/Average Velocity seismic processing velocities 

```
suvzerok_with_water_seis_vavg_horizon < Cb_Ph3_TWT_Vavg_062013_32b.su coeff_x=wb.twt.grd coeff_x2=bottom_twt_msec.grd verbose=1 > output.dat
awk '{print $9, $11, $14}' output.dat > vzero.seismic.dat
make.gmt.grid.sh -s vzero.seismic.dat
awk '{print $9, $11, $17}' output.dat > k.seismic.dat
make.gmt.grid.sh -s k.seismic.dat
```
