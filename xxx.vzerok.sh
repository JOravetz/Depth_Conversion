#! /bin/sh

rm -f output.simulate.lis
while read -r WELL ; do
   grep -w -e "${WELL}" tops.lis > bub
   sutest.vzerok.simulate pfile=bub verbose=3 >> output.simulate.lis
done < new_well_names.lis
