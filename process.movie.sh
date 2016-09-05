#! /bin/sh

### set -x

### Check program options.
while [ X"$1" != X-- ]
do
    case "$1" in
   -debug) echo "DEBUG ON"
           set -x
           DEBUG="yes"
           trap '' SIGHUP SIGINT SIGQUIT SIGTERM
           shift 1
           ;;
       -*) echo "${program}: Invalid parameter $1: ignored." 1>&2
           shift
           ;;
        *) set -- -- $@
           ;;
    esac
done
shift           # remove -- of arguments

xline_start=2050
xline_stop=3150

delta=2
traces=901

rm -f png.image.list.lis field.*.xwd field.*.png temp.su

while [ "${xline_start}" -le "${xline_stop}" ] ; do
   echo "Working on XLINE = ${xline_start}"

   suwind < xline.2014.traces.su key=selev min=${xline_start} max=${xline_start} count=${traces} > stuff1.su
   < stuff1.su insert-campanian-surfaces verbose=0 number=-5.0 > temp1.su
   FLDR_RANGE=`surange < temp1.su | grep -w "gelev"`
   FLDR_MIN=`echo ${FLDR_RANGE} | awk '{print $2}'`
   FLDR_MAX=`echo ${FLDR_RANGE} | awk '{print $3}'`
   suximage < temp1.su perc=99 f1=1500 d1=2 f2=${FLDR_MIN} d2=1 grid1=dot gridcolor=yellow label1="Depth (meters)" \
   xbox=0 ybox=53 wbox=1680 hbox=972 grid2=dot label2="X-LINE Number" title="FIELD Xline = ${xline_start}" &
   sleep 0.25
   xwd -nobdrs -name ximage -out field.${xline_start}.1.xwd
   id=`xwininfo -name ximage | grep "id:" | awk -F"id: " '{print $2}' | awk '{print $1}'`
   xkill -id ${id}
   convert field.${xline_start}.1.xwd field.${xline_start}.1.png

   suwind < xline.4d.diff.su key=selev min=${xline_start} max=${xline_start} count=${traces} > stuff.su
   < stuff.su insert-campanian-surfaces verbose=0 number=0.80 > temp.su
   suximage < temp.su perc=98 clip=-0.90 cmap=hsv4 f1=1500 d1=2 f2=${FLDR_MIN} d2=1 grid1=dot gridcolor=yellow label1="Depth (meters)" \
   xbox=0 ybox=53 wbox=1680 hbox=972 grid2=dot label2="X-LINE Number" title="FIELD Xline = ${xline_start}" legend=1 &
   sleep 0.25
   xwd -nobdrs -name ximage -out field.${xline_start}.xwd
   id=`xwininfo -name ximage | grep "id:" | awk -F"id: " '{print $2}' | awk '{print $1}'`
   xkill -id ${id}
   convert field.${xline_start}.xwd field.${xline_start}.png
   # echo "field.${xline_start}.png" >> png.image.list.lis

   convert -morph 5 field.${xline_start}.1.png field.${xline_start}.png morph.png
   mv morph-4.png field.morph.${xline_start}.png
   echo "field.morph.${xline_start}.png" >> png.image.list.lis

   xline_start=`expr $xline_start + $delta`
done
convert @png.image.list.lis field.movie.gif
