#! /bin/sh

### rm -f output
### while read -r LINE ; do
###    X=`echo $LINE | awk '{print $1}'`
###   Y=`echo $LINE | awk '{print $2}'`
###   VALUE=`echo "$X $Y" | grdtrack -Qb -Gsum.grd | awk '{print $3}'`
###   echo $VALUE >> output
###done < geom

grdtrack geom -Gsum.grd -Qb -Z -bos | supaste head=headers ns=1 > slab.su
