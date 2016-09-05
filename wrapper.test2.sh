#! /bin/sh

START=0
STOP=3000

RANGE="-R511797/527792/152443/169653"

SEISMIC_VELOCITY="vel.poly.scaled.su"
DELTAV_LISTING="test.poly.scaled.deltav.lis"

CHECK=`bc -l <<END
   scale=0
   ${START} / 50
END`

FIRST=1

### supermute n1=3001 n2=1101 n3=901 o1=2 o2=3 o3=1 verbose=1 < ${SEISMIC_VELOCITY} | sushw key=tracl a=1 b=1 | sushw key=d1,f1,d2,f2,ntr a=0,0,0,0,0 > vel.su

while [ "${START}" -le "${STOP}" ] ; do

SKIP=`bc -l <<END
   scale=0
   ${START} * 901
END`

TWT=`bc -l <<END
   scale=0
   ${START} * 2
END`

TWT1=`bc -l <<END
   scale=0
   ${TWT} + 2
END`

   suwind < vel.su ordered=1 skip=${SKIP} count=901 > ${TWT}.slice.su

   if [ "${FIRST}" -eq "1" ] ; then
      suwind < ${SEISMIC_VELOCITY} tmin=0 tmax=0 > zero.su
      sugethw < zero.su key=sx,sy output=geom > geom
      sustrip < zero.su head=headers > /dev/null
      FIRST=0
   fi

   awk '{if(NR>1&&$4>='"${TWT}"'&&$4<='"${TWT1}"'){print $2, $3, $10}}' ${DELTAV_LISTING} | sort -n | uniq > ${TWT}.stuff

   process.test2.sh -slice ${TWT}.slice.su -stuff ${TWT}.stuff &

CHECK1=`bc -l <<END
   scale=0
   ${START} / 50
END`

if [ "${CHECK}" -ne "${CHECK1}" ] ; then
   CHECK=${CHECK1}
   sleep 60
fi

START=`expr ${START} + 1`

done
