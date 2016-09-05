#! /bin/sh

VWATER="1524.0"
BGRID="wb.twt.grd"
WB_FILE="water.depths.lis"

rm -f header output.lis
while read -r WELL ; do
   WD=`grep -w -e "${WELL}" ${WB_FILE} | awk '{print $2}'`
   if [ ! -z "${WD}" ] ; then
      TWT=`bc -l <<END
        scale=6
        ( ${WD} / ${VWATER} ) * 2000.0
END`

      FNAME=`echo "../../FIELD_RFC_Full_Stack_Tied_TD/${WELL}_Ph2B_2003_ZPh_RFC_Full_Stack_Tied_TD.txt" | sed 's/C-/FIELD-/g'`
      if [ -s "${FNAME}" ] ; then
         XLOC=`grep -w -e "X_LO" ${FNAME} | awk '{print $3}'`
         YLOC=`grep -w -e "Y_LO" ${FNAME} | awk '{print $3}'`
         WB_TWT=`echo "${XLOC} ${YLOC}" | grdtrack -G"${BGRID}" -Qc | awk '{print $3}'`
         DIFF=`bc -l <<END
           scale=6
           ${WB_TWT} - ${TWT}
END`
         VWATER_NEW=`bc -l <<END
           scale=6
           ( ${WD} / ${WB_TWT} ) * 2000.0
END`
         echo "${WELL} ${WD}, ${XLOC}, ${YLOC}, ${TWT}, ${WB_TWT}, ${DIFF}, ${VWATER_NEW}" |\
         awk '{ printf "Working on Well = %-12s WD = %8.2f, XLOC = %10.2f, YLOC = %10.2f, TWT_Water = %8.2f, TWT_Grid = %8.2f, DIFF = %8.4f, Vwater_New = %8.2f\n",\
          $1, $2, $3, $4, $5, $6, $7, $8}' >> header
      else
         echo "Could not find well file = ${FNAME}"
      fi
   fi
done < well_names.lis

cat header
awk -f average.awk 26 header
awk '{print $11, $14, $26}' header | sed 's/,//g' > water_velocity.lis
