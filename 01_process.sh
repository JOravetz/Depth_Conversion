#! /bin/sh

VWATER="1452.50"
K="0.610836"
ContractorRID="wb.twt.grd"
FNAME="field_well_markers_exported_from_Program_05June2013.lis"

awk '{if(NR>1){print $1}}' ${FNAME} | grep -v -e "Well" | sort | uniq > well_names.lis

rm -f output.lis
while read -r WELL ; do
   grep -w -e "${WELL}" ${FNAME} > bub
   XLOC=`awk -f average.awk 3 bub | tail -1 | awk '{print $7}'`
   YLOC=`awk -f average.awk 4 bub | tail -1 | awk '{print $7}'`
   TWT=`echo "${XLOC} ${YLOC}" | grdtrack -G"${ContractorRID}" -Qc | awk '{print $3}'`
   if [ ! -z "${TWT}" ] ; then
      WD=`bc -l <<END
      scale=6
      ( ${TWT} * ${VWATER} ) * 0.0005;
END`
      echo "Working on Well = ${WELL}, WD = ${WD}, VWATER = ${VWATER}, TWT_WATER = ${TWT}, XLOC = ${XLOC}, YLOC = ${YLOC}"
      awk '{if(NF==7){printf "%-12.4f %12.4f\n", $7-'"${TWT}"', -'"${WD}"'-$5}}' bub | awk '{if($1>=0&&$2>=0){print}}'> bub1
      NUM=`wc -l bub1 | awk '{print $1}'`
      if [ "${NUM}" -ge "3" ] ; then
         xxx.constantv0 < bub1 vzero=${VWATER} k=${K} | tail -1 > bub2
         ### xxx < bub1 vzero=${VWATER} k=${K} | tail -1 > bub2
         echo ${WELL} ${XLOC} ${YLOC} `cat bub2` | awk '{ printf "%-12s %12.2f %12.2f %12.2f %12.6f %12.2f %5d\n", $1, $2, $3, $4, $5, $6, $7 }' >> output.lis
      fi
   fi
done < well_names.lis

rm -f bub*
sort -k 6,6n output.lis > output.1.lis
cat output.1.lis
