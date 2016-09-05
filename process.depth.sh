
segyread tape=/mnt/hgfs/user/cf43329_GQ14_field_ph4_wgc_time_fullstk_2014_072013_+35deg_Int_Norm.sgy endian=0 > input.su
< input.su joebob-vzerok-double nz=3500 verbose=1 > output.su
segyhdrs < output.su
< output.su segywrite tape=/mnt/hgfs/user/cf43329_GQ14_field_ph4_wgc_fullstk_2014_072013_+35deg_Int_Norm_depth_m.sgy endian=0

segyread tape=/mnt/hgfs/user/cf43330_GQ14_field_ph4_wgc_time_fullstk_4d_diff_2014_minus_2010_072013_+35deg_Int_Norm.sgy endian=0 > input.su
< input.su joebob-vzerok-double nz=3500 verbose=1 > output.su
segyhdrs < output.su
< output.su segywrite tape=/mnt/hgfs/user/cf43330_GQ14_field_ph4_wgc_fullstk_4d_diff_2014_minus_2010_072013_+35deg_Int_Norm_depth_m.sgy endian=0

