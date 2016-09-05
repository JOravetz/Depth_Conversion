#! /bin/sh

echo "Compute mean and standard deviation of average-velocity differences and filter data by 1.5 standard-deviations"

MEAN_STDEV_VALUES=`awk '{for(i=8;i<=NF;i++) {sum[i] += $i; sumsq[i] += ($i)^2}} END {for (i=8;i<=NF;i++) { printf "%.15f %.15f \n", sum[i]/NR, sqrt((sumsq[i]-sum[i]^2/NR)/NR)} }' output1.dat`
MEAN=`echo ${MEAN_STDEV_VALUES} | awk '{print $1}'`
STDEV=`echo ${MEAN_STDEV_VALUES} | awk '{print $2}'`
awk '{ print $0, ($8-'"${MEAN})"'/'"${STDEV}"' }' output1.dat | awk '{if ( sqrt ( $9 *$9 ) < 1.5 ) { $9=""; print $0 }}' > output1.filtered.dat

cat output1.filtered.dat
