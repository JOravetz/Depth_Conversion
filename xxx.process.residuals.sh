#! /bin/sh

rm -f residuals.lis
gmtset D_FORMAT %0.16lf
while read -r LINE ; do
   X=`echo $LINE | awk '{print $1}'`
   Y=`echo $LINE | awk '{print $2}'`
   K=`echo $LINE | awk '{print $4}'`
   K_Seismic=`echo "$X $Y" | grdtrack -Qb -GK.seismic.sample.grd | awk '{print $3}'`
   K_DIFF=`bc -l <<END
      scale=16
      ${K} - ${K_Seismic}
END`
   echo "K_DIFF = $K_DIFF"
   echo "$X $Y $K_DIFF" >> residuals.lis 
done < output.simulate.checkshots.lis

sort -k3,3n residuals.lis > residuals.sorted.lis
MIN=`head -1 residuals.sorted.lis | awk '{print $3}'`
MAX=`tail -1 residuals.sorted.lis | awk '{print $3}'`
RANGE=`grdinfo -I721 k.seismic.dat.trimmed.sample.smooth.grd`
gmtset D_FORMAT %0.16lf
surface residuals.sorted.lis -Gresiduals.surface.grd -S16000 -I721+ -V -T0.5 ${RANGE} -Ll${MIN} -Lu${MAX}
grdfilter residuals.surface.grd -Gresiduals.surface.filter.grd -Fm500 -D0 -V

gmtset D_FORMAT %0.16lf
grdmath residuals.surface.filter.grd K.seismic.sample.grd ADD = K.new.grd

gmtset D_FORMAT %0.4lf
mbm_grdplot -IK.new.grd -X -V -G2 -A1000/30 -B2000
