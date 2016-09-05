#! /bin/sh

START=2999
STOP=3000

while [ "${START}" -le "${STOP}" ] ; do

SKIP=`bc -l <<END
   scale=0
   ${START} * 901
END`

TWT=`bc -l <<END
   scale=0
   ${START} * 2
END`

   suwind < vel.su ordered=1 skip=${SKIP} count=901 > scaled.${TWT}.su

START=`expr ${START} + 1`

done
