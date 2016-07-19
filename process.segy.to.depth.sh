#! /bin/sh

while read -r FILE ; do
   FNAME=`echo ${FILE} | awk -F".segy" '{print $1}'`
   echo ${FNAME}
   segyread tape=${FILE} endian=0 remap=gelev,selev byte=181l,185l | segyclean > input.su
   sudepthconvert.v1 < input.su vfile=../velout.vavg.su verbose=1 > output.su
   segyhdrs < output.su
   segywrite < output.su tape=../SEGY_DEPTH/${FNAME}.DEPTH_v2.sgy endian=0
done < segy_input.lis
