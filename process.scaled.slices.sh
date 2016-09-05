#! /bin/sh

START=0
STOP=6000

rm -f vel.scaled.bin
while [ "${START}" -le "${STOP}" ] ; do
   echo "Working on TWT slice = ${START}"
   ### suswapbytes < scaled.${START}.su format=0 > bub && mv bub scaled.${START}.su
   sustrip < scaled.${START}.su head=/dev/null >> vel.scaled.bin
   START=`expr ${START} + 2`
done

suaddhead ns=1101 < vel.scaled.bin | sushw key=tracl a=1 b=1 | supermute n1=1101 n2=901 n3=3001 o1=3 o2=1 o3=2 | sustrip head=/dev/null | supaste ns=3001 head=headers > vel.scaled.test3.su
