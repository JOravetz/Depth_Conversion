 segyread tape=2014_fullstk_xline_2600.sgy endian=0 remap=gelev,selev byte=181l,185l | segyclean > 2014_fullstk_xline_2600.su
 segyread tape=2010_phase3_fullstk_xline_2600.sgy endian=0 remap=gelev,selev byte=181l,185l | segyclean | suwind tmax=3.5 > 2010_phase3_fullstk_xline_2600.su

 cp 2014_fullstk_xline_2600.su movie.su
 dd if=2010_phase3_fullstk_xline_2600.su >> movie.su
 suxmovie < movie.su n2=901 perc=99 loop=2 f2=1100 d2=1 &

 suresamp < 2014_fullstk_xline_2600.su nt=3500 dt=0.001 > stuff.su
 mv stuff.su 2014_fullstk_xline_2600.su
 suresamp < 2010_phase3_fullstk_xline_2600.su nt=3500 dt=0.001 > stuff.su
 mv stuff.su 2010_phase3_fullstk_xline_2600.su
 suxcor < 2014_fullstk_xline_2600.su sufile=2010_phase3_fullstk_xline_2600.su panel=1 > xcor.su
 suwind < xcor.su tmin=3.25 tmax=3.75 > xcor.window.su
 suximage < xcor.window.su perc=99.9 grid1=dot grid2=dot f2=1100 d2=1 cmap=hsv2 title="Cross-Correlation 2010-2014 Contractor Processing - FIELD" &
 suxwigb < xcor.window.su perc=99.99 grid1=dot grid2=dot f2=1100 d2=1 title="Cross-Correlation 2010-2014 Contractor Processing - FIELD" &
 # suedit xcor.window.su

 sufrac < 2014_fullstk_xline_2600.su power=0 phasefac=0.1944 > test.su
 suxcor < test.su sufile=2010_phase3_fullstk_xline_2600.su panel=1 > xcor.su
 suwind < xcor.su tmin=3.25 tmax=3.75 > xcor.window.su
 suximage < xcor.window.su perc=99.9 grid1=dot grid2=dot f2=1100 d2=1 cmap=hsv2 title="2014 Contractor Processing - FIELD - Phase-Shift +35 Degrees" &
 suxwigb < xcor.window.su perc=99.99 grid1=dot grid2=dot f2=1100 d2=1 title="2014 Contractor Processing - FIELD - Phase-Shift +35 Degrees" &
 # suedit xcor.window.su

 cp test.su movie1.su
 dd if=2010_phase3_fullstk_xline_2600.su >> movie1.su
 suxmovie < movie1.su n2=901 perc=99 loop=2 f2=1100 d2=1 &
