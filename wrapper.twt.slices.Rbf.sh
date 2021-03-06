#! /bin/sh

START=0
STOP=6000

rm -f residual.corrections.bin
sunull nt=1101 ntr=901 | sustrip > null.bin

while [ "${START}" -le "${STOP}" ] ; do

TWT=`bc -l <<END
   scale=0
   ${START} + 2
END`

   awk '{if(NR>1&&$4>='"${START}"'&&$4<='"${TWT}"'){print $2, $3, $10}}' test.poly.scaled.deltav.lis | sort | uniq > test.dat 
   NUM=`wc -l test.dat | awk '{print $1}'`
   echo "Number of control points = ${NUM}"

   if [ ${NUM} -ge 4 ] ; then
      sum_data < test.dat verbose=1 > test1.dat 
      python test.py | a2b n1=1101 >> residual.corrections.bin
   else 
      dd if=null.bin >> residual.corrections.bin
   fi     

START=`expr ${START} + 2`

done
