#! /bin/sh

rm stuff.combined.dat
while read -r FILE ; do
   dd if=${FILE} >> stuff.combined.dat
done < stuff.lis
