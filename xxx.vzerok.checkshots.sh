#! /bin/sh

rm -f output.simulate.checkshots.lis
#sutest.vzerok.checkshots.simulate pfile=/u/000000/FIELD.new/checkshots/FIELD_combined_checkshots.dat verbose=3 >> output.simulate.checkshots.lis
#sutest.vzerok.checkshots.simulate pfile=FIELD_combined_checkshots.FILTERED.dat verbose=3 >> output.simulate.checkshots.lis
sutest.vzerok.checkshots.simulate pfile=tops.lis verbose=3 >> output.simulate.checkshots.lis
