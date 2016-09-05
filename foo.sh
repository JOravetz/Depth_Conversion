#! /bin/sh

while read -r LINE ; do
   X=`echo $LINE | awk '{print $2}'`
   Y=`echo $LINE | awk '{print $3}'`
   TWT=`echo $LINE | awk '{print $4}'`
   Z=`echo $LINE | awk '{print $5}'`
   VEL=`echo "$X $Y" | grdtrack -Qb -Gsum.grd | awk '{print $3}'`

   VEL_WELL=`bc -l <<END
      scale=6
      ( $Z / $TWT ) * 2000.0
END`

   DIFF=`bc -l <<END
      scale=6
      $VEL_WELL - $VEL
END`

   echo "$X $Y $VEL_WELL $VEL $DIFF" 

done < check.2700.lis
