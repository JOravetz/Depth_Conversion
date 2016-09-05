
segyread tape=/mnt/hgfs/user/cf43329_GQ14_field_ph4_wgc_time_fullstk_2014_072013_+35deg_Int_Norm.sgy endian=0 > input.su
< input.su joebob-vzerok nz=3500 coeff_x=wb.twt.grd coeff_x2=K.below.ml.lis.trimmed.sample.grd verbose=1 > output.su
segyhdrs < output.su
< output.su segywrite tape=/mnt/hgfs/user/cf43329_GQ14_field_ph4_wgc_fullstk_2014_072013_+35deg_Int_Norm_depth_m_v1.sgy endian=0

segyread tape=/mnt/hgfs/user/cf43330_GQ14_field_ph4_wgc_time_fullstk_4d_diff_2014_minus_2010_072013_+35deg_Int_Norm.sgy endian=0 > input.su
< input.su joebob-vzerok nz=3500 coeff_x=wb.twt.grd coeff_x2=K.below.ml.lis.trimmed.sample.grd verbose=1 > output.su
segyhdrs < output.su
< output.su segywrite tape=/mnt/hgfs/user/cf43330_GQ14_field_ph4_wgc_fullstk_4d_diff_2014_minus_2010_072013_+35deg_Int_Norm_depth_m_v1.sgy endian=0

segyread tape=/mnt/hgfs/user/cf43331_GQ14_field_ph4_wgc_time_fullstk_4d_diff_2014_minus_1999_072013_+35deg_Int_Norm.sgy endian=0 > input.su
< input.su joebob-vzerok nz=3500 coeff_x=wb.twt.grd coeff_x2=K.below.ml.lis.trimmed.sample.grd verbose=1 > output.su
segyhdrs < output.su
< output.su segywrite tape=/mnt/hgfs/user/cf43331_GQ14_field_ph4_wgc_fullstk_4d_diff_2014_minus_1999_072013_+35deg_Int_Norm_depth_m_v1.sgy endian=0
