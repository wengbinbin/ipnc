
DVEVMDIR=/opt/dvr

# Load the dsplink and cmem kernel modules
cd $DVEVMDIR

$DVEVMDIR/av_capture_load.sh
$DVEVMDIR/dvr_netset.sh

$DVEVMDIR/dvrDetectIP &

cd /dev
ln -s rtc0 rtc

cd $DVEVMDIR
sleep 1

$DVEVMDIR/loadkmodules.sh
$DVEVMDIR/loadmodules_dvr.sh

ifconfig lo 127.0.0.1

#./boot_proc 1
# Start the demo application

./system_server &

cd $DVEVMDIR
./boa -c /mnt/nand &

#$DVEVMDIR/autorun.sh
