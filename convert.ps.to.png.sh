#! /bin/sh

while read -r FILE ; do
   NAME=`echo ${FILE} | awk -F".ps" '{print $1}'`
   echo "Working on File = ${FILE}"
   convert -density 150 ${FILE} ${NAME}.png
done < ps.lis
