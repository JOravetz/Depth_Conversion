while read -r LINE ; do
   process.deviation.sh -s ${LINE}
done < deviations.lis
