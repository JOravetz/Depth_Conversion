#! /bin/sh

while read -r FILE ; do
   FNAME=`echo ${FILE} | awk -F".segy" '{print $1}'`
   echo ${FNAME}
   segyread tape=${FILE} endian=0 remap=gelev,selev byte=181l,185l | segyclean > input.su
   suresamp < input.su nt=2001 dt=0.002 | suxcor sufile=filter.su | suwind tmin=0.40 tmax=4.40 | sushw key=delrt a=0 > input.trint.su
   sudepthconvert.v1 < input.trint.su vfile=../velout.vavg.su verbose=1 > output.su
   segyhdrs < output.su
   segywrite < output.su tape=../SEGY_DEPTH/${FNAME}.DEPTH_v2.TRINT.sgy endian=0
done < segy_input.lis
