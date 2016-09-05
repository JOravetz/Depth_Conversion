#! /bin/sh

while read -r SURFACE ; do
   process.combined.sh -s ${SURFACE}
done < surfaces.lis
