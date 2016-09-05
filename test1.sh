#! /bin/sh

START=0
STOP=3000
RANGE="-R511797/527792/152443/169653"

### supermute n1=3001 n2=1101 n3=901 o1=2 o2=3 o3=1 verbose=1 < Cb_Ph3_TWT_Vavg_062013_32b.su | sushw key=tracl a=1 b=1 | sushw key=d1,f1,d2,f2,ntr a=0,0,0,0,0 > vel.su
### suwind < Cb_Ph3_TWT_Vavg_062013_32b.su tmin=0 tmax=0 count=992001 | sugethw key=sx,sy output=geom > hdrfile

while [ "${START}" -le "${STOP}" ] ; do

SKIP=`bc -l <<END
   scale=0
   ${START} * 901
END`

suwind < vel.su ordered=1 skip=${SKIP} count=901 > slice.su

TWT=`bc -l <<END
   scale=0
   ${START} * 2
END`
TWT1=`bc -l <<END
   scale=0
   ${TWT} + 2
END`

echo "Working on Average-Velocity at TWT = ${TWT} milliseconds"

awk '{if($3>='"${TWT}"'&&$3<='"${TWT1}"'){print $1, $2, $4}}' BIG.combined.reform.dat | sort -n > stuff

### suwind < Cb_Ph3_TWT_Vavg_062013_32b.su tmin=${TWT_SEC} tmax=${TWT_SEC} > slice.su
### sugethw < slice.su key=sx,sy output=geom > hdrfile

sustrip < slice.su | b2a n1=1 format=1 > datafile
paste hdrfile datafile > data

### Number of Inlines =   901, Number of Xlines =  1101

gmtset D_FORMAT %0.4lf
### RANGE=`minmax -I901+/1101+ data`
LOW=`minmax -C data | awk '{print $5}'`
HIGH=`minmax -C data | awk '{print $6}'`

rm -f data.grd
surface data -Gdata.grd -I901+/1101+ -S500 -T0 ${RANGE} -Ll${LOW} -Lu${HIGH}

NUM=`wc -l stuff | awk '{print $1}'`
echo "Number of control points = ${NUM}"

if [ "${NUM}" -gt "0" ] ; then
   gmtset D_FORMAT %0.4lf
   rm -f bub
   while read -r LINE ; do
      X=`echo ${LINE} | awk '{print $1}'`
      Y=`echo ${LINE} | awk '{print $2}'`
      Vavg_Seismic=`echo "${X} ${Y}" | grdtrack -Qb -Gdata.grd | awk '{print $3}' | sed 's/-//g'`
      echo "${LINE} ${Vavg_Seismic}" >> bub
   done < stuff
   awk '{printf "%-36s %12.6f\n", $0, $3-$4}' bub | awk '{print $1, $2, $5}' > diff

###   RANGE=`minmax -I901+/1101+ data`
   LOW=`minmax -C diff | awk '{print $5}'`
   HIGH=`minmax -C diff | awk '{print $6}'`

   rm -f diff.grd
   surface diff -Gdiff.grd -I901+/1101+ -S16000 -T0 ${RANGE} -Ll${LOW} -Lu${HIGH}

   grdmath diff.grd data.grd ADD = scaled.${TWT}.grd

###   mbm_grdplot -Idiff.grd -X -V -G2 -A10/30 -B2000
###   sleep 2
###   ps2pdf diff.grd.ps
###   acroread diff.grd.pdf &
else
###   grdmath data.grd 0.0 MUL = diff.grd
   cp data.grd scaled.${TWT}.grd
fi

### gmtset D_FORMAT %0.2lf
### mbm_grdplot -Iscaled.grd -X -V -G2 -A10/30 -B2000
### sleep 2
### ps2pdf scaled.grd.ps
### acroread scaled.grd.pdf &

### mbm_grdplot -Idata.grd -X -V -G2 -A10/30 -B2000
### sleep 2
### ps2pdf data.grd.ps
### acroread data.grd.pdf &

START=`expr ${START} + 1`

done
