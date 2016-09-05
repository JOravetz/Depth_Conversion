XMIN=511797
XMAX=527792
YMIN=152443
YMAX=169653
TMIN=0
TMAX=4000

NX=500
NY=650
NT=800

suaddhead < Gridder1.raw ns=${NX} > gridder.su
supermute < gridder.su n1=${NX} n2=${NY} n3=${NT} o1=3 o2=1 o3=2 > gridder1.su

DX=`bc -l <<END
   scale=4
   ( ${XMAX} - ${XMIN} ) / ( ${NX} - 1 ) + 0.0001
END`

DY=`bc -l <<END
   scale=4
   ( ${YMAX} - ${YMIN} ) / ( ${NY} - 1 ) + 0.0001
END`

DT=`bc -l <<END
   scale=0
   ( ${TMAX} - ${TMIN} ) / ${NT} * 1000
END`

echo "DX = ${DX}, DY = ${DY}, DT (microseconds) = ${DT}"

sushw < gridder1.su key=sx a=${XMIN} b=${DX} j=${NX} | sushw key=sy a=${YMIN} b=0 j=${NX} c=${DY} | sushw key=dt a=${DT} |\
sushw key=d1,f1,d2,f2,ntr a=0,0,0,0,0 > gridder2.su
