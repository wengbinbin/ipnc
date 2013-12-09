# insmod - insert kernel modules - ANR
#modules (.ko) - /home/anand/home1/filesys2/root/modules/
#
#insmod list as follows,
#
#init.d modules
#/opt/dvr/modules/init.d_ins/checkroot.sh
/opt/dvr/modules/init.d_ins/mountall.sh
/opt/dvr/modules/init.d_ins/ifupdown start
/opt/dvr/modules/init.d_ins/hotplug start
/opt/dvr/modules/init.d_ins/portmap start
/opt/dvr/modules/init.d_ins/inetd start
#
#
#
#Kernel modules
#other
insmod /opt/dvr/modules/crc-ccitt.ko
insmod /opt/dvr/modules/libcrc32c.ko
insmod /opt/dvr/modules/deadline-iosched.ko
insmod /opt/dvr/modules/cfq-iosched.ko
insmod /opt/dvr/modules/i2c-emac.ko cmdline="`cat /proc/cmdline`"
insmod /opt/dvr/modules/sbull.ko
#cripto
insmod /opt/dvr/modules/crc32c.ko
insmod /opt/dvr/modules/deflate.ko
insmod /opt/dvr/modules/des.ko
insmod /opt/dvr/modules/md5.ko
insmod /opt/dvr/modules/sha1.ko
#fs
insmod /opt/dvr/modules/nls_ascii.ko
insmod /opt/dvr/modules/nls_utf8.ko
insmod /opt/dvr/modules/exportfs.ko
insmod /opt/dvr/modules/nfsd.ko
#driver
#
#net
insmod /opt/dvr/modules/loop.ko
insmod /opt/dvr/modules/tun.ko
insmod /opt/dvr/modules/slhc.ko
insmod /opt/dvr/modules/netconsole.ko
insmod /opt/dvr/modules/ppp_generic.ko
insmod /opt/dvr/modules/ppp_synctty.ko
insmod /opt/dvr/modules/ppp_async.ko
insmod /opt/dvr/modules/ppp_deflate.ko
insmod /opt/dvr/modules/davinci_emac.ko
#mmc
insmod /opt/dvr/modules/mmc_core.ko
insmod /opt/dvr/modules/mmc_block.ko
insmod /opt/dvr/modules/davinci-mmc.ko
#usb
insmod /opt/dvr/modules/usbcore.ko
insmod /opt/dvr/modules/musb_hdrc.ko
insmod /opt/dvr/modules/usbmon.ko
#insmod /opt/dvr/modules/g_file_storage.ko
#scsi
insmod /opt/dvr/modules/scsi_transport_iscsi.ko
insmod /opt/dvr/modules/libiscsi.ko
insmod /opt/dvr/modules/iscsi_tcp.ko
#
#
#
#/opt/dvr/dhcpcd/dhcpcd
#
#
#
