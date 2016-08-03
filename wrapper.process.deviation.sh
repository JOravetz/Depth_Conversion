#! /bin/bash

### wrapper script to  loop over each deviation survey - stored in deviations.lis ###

ls -alt *deviation_survey_from_Petrel.dat | awk '{print $9}' > deviations.lis
while read -r LINE ; do
   process.deviation.sh -s ${LINE}
done < deviations.lis
