#!/bin/sh
#/etc/rc.d/rc.network 
#

#Attach the loopback device.
/sbin/ifconfig lo 127.0.0.1
/sbin/route add -net 127.0.0.0 netmask 255.0.0.0 lo

# Edit these values to set up a static IP address:
DHCP=0
MAC=""
IPADDR="192.168.1.239"     # REPLACE with YOUR IP address!
NETMASK="255.255.255.0"    # REPLACE with YOUR netmask!
GATEWAY="192.168.1.1"      # REPLACE with YOUR gateway address!
BROADCAST="192.168.1.255"  # REPLACE with YOUR broadcast address, if you have one. If not, leave blank and edit below.
PORT=7620                  #
NETWORK=""                 # REPLACE with YOUR network address!
 
CFG_NETWORK="/mnt/nand/cfg-network"
#CFG_NET_ORG=

if [ -f "$CFG_NETWORK" ]; then
  . $CFG_NETWORK  

  echo "Read Network Config from $CFG_NETWORK"
  
  DHCP=`cat $CFG_NETWORK | grep "DHCP" | sed 's/DHCP=//'`
  MAC=`cat $CFG_NETWORK | grep "MAC" | sed 's/MAC=//'`
  IPADDR=`cat $CFG_NETWORK | grep "IPADDR" | sed 's/IPADDR=//'`
  NETMASK=`cat $CFG_NETWORK | grep "NETMASK" | sed 's/NETMASK=//'`
  GATEWAY=`cat $CFG_NETWORK | grep "GATEWAY" | sed 's/GATEWAY=//'`
  BROADCAST=`cat $CFG_NETWORK | grep "BROADCAST" | sed 's/BROADCAST=//'`
  #PORT=`cat $CFG_NETWORK | grep "PORT" | sed 's/PORT=//'`
  
  echo "file open result"
  echo "DHCP=$DHCP"
  echo "MAC=$MAC"
  echo "IPADDR=$IPADDR"
  echo "NETMASK=$NETMASK"
  echo "GATEWAY=$GATEWAY"
  echo "BROADCAST=$BROADCAST"
  #echo "PORT=$PORT"

fi
echo "Configured eth0 as ${IPADDR} ..."
    
    #/sbin/dhcpcd -k eth0
		echo "DHCP=$DHCP"
		echo "MAC=$MAC"
		echo "IPADDR=${IPADDR}"
		echo "NETMASK=${BROADCAST}"
		echo "GATEWAY=${NETMASK}"
		/sbin/ifconfig eth0 ${IPADDR} broadcast ${BROADCAST} netmask ${NETMASK}

   #if [ ! "$MAC" = "" ]; then
   #    /sbin/ifconfig eth0 hw ether $MAC
   #fi

    /sbin/ifconfig eth0 ${IPADDR} up
    #MAC=`/sbin/ifconfig eth0 | grep "HWaddr" | awk '{print $5}'`
    
    if [ ! "$MAC" = "" ]; then
        /sbin/ifconfig eth0 hw ether $MAC
    fi

    # Older kernel versions need this to set up the eth0 routing table:
    #KVERSION=`uname -r | cut -f 1,2 -d .`
    #if [ "$KVERSION" = "1.0" -o "$KVERSION" = "1.1" \
    # -o "$KVERSION" = "1.2" -o "$KVERSION" = "2.0" -o "$KVERSION" = "" ]; then
    #  /sbin/route add -net ${NETWORK} netmask ${NETMASK} eth0
    #fi
    
    # If there is a gateway defined, then set it up:
    if [ ! "$GATEWAY" = "" ]; then
      /sbin/route add default gw ${GATEWAY} netmask 0.0.0.0 metric 1
      /sbin/route add -net 239.0.0.0 netmask 255.0.0.0 eth0
    fi
    
    if [ ! -f $CFG_NETWORK ]; then
       echo DHCP=$DHCP > $CFG_NETWORK
       echo MAC=\"$MAC\" >> $CFG_NETWORK
       echo IPADDR=\"$IPADDR\" >> $CFG_NETWORK
       echo NETMASK=\"$NETMASK\" >> $CFG_NETWORK
       echo GATEWAY=\"$GATEWAY\" >> $CFG_NETWORK
       echo BROADCAST=\"$BROADCAST\" >> $CFG_NETWORK
       #echo PORT=$PORT >> $CFG_NETWORK
       #echo NETWORK=\"$NETWORK\" >> $CFG_NETWORK              
    fi    
hostname $IPADDR

#/sbin/ifconfig eth0 down
#/sbin/ifconfig eth0 ${IPADDR} up
#/sbin/route add default gw ${GATEWAY} netmask 0.0.0.0 metric 1
