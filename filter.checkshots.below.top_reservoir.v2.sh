#! /bin/sh

GRID="top_reservoir.twt.grd"

rm -f _FIELD_combined_checkshots.FILTERED.dat
while read -r LINE ; do
   XLOC=`echo ${LINE} | awk '{print $2}'`
   YLOC=`echo ${LINE} | awk '{print $3}'`
   TWT=`echo ${LINE} | awk '{print $4}'`
   TWT_SURFACE=`echo "${XLOC} ${YLOC}" | grdtrack -Qb -G${GRID} | awk '{print -$3}'`
   DIFF=`bc -l <<END
      scale=6
      ${TWT} - ${TWT_SURFACE} + 100.0
END`
   echo "TWT Checkshot = ${TWT}, TWT GRID = ${TWT_SURFACE}, DIFF = ${DIFF}"
   TEST=
   TEST=`echo ${DIFF} | grep -e "-"`
   if [ -z "${TEST}" ] ; then
      echo "${TWT}, ${TWT_SURFACE}, ${DIFF}"
      echo ${LINE} >> _FIELD_combined_checkshots.FILTERED.dat
   fi
done < _FIELD_combined_checkshots.dat 
