#! /bin/sh

FNAME="test.deltav.lis"
### FNAME="test.poly.scaled.deltav.lis"

rm -f output.lis
awk '{print $7, $8}' ${FNAME} > input.dat
FOO=`supoly < input.dat verbose=0 n=3`
A=`echo ${FOO} | awk '{print $1}'`
B=`echo ${FOO} | awk '{print $2}'`
echo "${A} ${B}" | awk '{ printf "%20.16f %20.16f\n", $1, $2}' 
echo "${A} ${B}" | awk '{ printf "%20.16f %20.16f\n", $1, $2}' > output.lis

echo
cat output.lis
