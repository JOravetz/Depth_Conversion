FNAME="poly.seismic"
awk '{print $1, $2, $3*100}' output.${FNAME}.lis > A.100.${FNAME}.dat
awk '{print $1, $2, $4*100000}' output.${FNAME}.lis > B.100000.${FNAME}.dat
awk '{print $1, $2, $5*1000000000}' output.${FNAME}.lis > C.1000000000.${FNAME}.dat

sleep 3
make.gmt.grid.seismic.sh -s A.100.${FNAME}.dat
sleep 3
make.gmt.grid.seismic.sh -s B.100000.${FNAME}.dat
sleep 3
make.gmt.grid.seismic.sh -s C.1000000000.${FNAME}.dat
