#!/bin/bash -e

if ! [ $(id -u) = 0 ] ; then
	echo "$0 must be run as sudo user or root"
	exit 1
fi

#We need to work around these default ip ranges, to make sure the tests pass
#192.168.7.x = default usb0
#192.168.8.x = default SoftAP0
#192.168.9.x = (bbxm, work around 192.168.7.x usb0)

#usb0 192.168.7.x -> 192.168.10.x

# Used by: /opt/scripts/boot/autoconfigure_usb0.sh
#iface usb0 inet static
#    address 192.168.7.2
#    netmask 255.255.255.252
#    network 192.168.7.0
#    gateway 192.168.7.1

unset deb_iface_range_regex
unset deb_usb_address
unset deb_usb_network
unset deb_usb_gateway

deb_iface_range_regex="/^[[:space:]]*iface[[:space:]]+usb0/,/iface/"

deb_usb_address=$(sed -nr "${deb_iface_range_regex} p" /etc/network/interfaces |\
		  sed -nr "s/^[[:space:]]*address[[:space:]]+([0-9.]+)/\1/p")

deb_usb_network=$(sed -nr "${deb_iface_range_regex} p" /etc/network/interfaces |\
		  sed -nr "s/^[[:space:]]*network[[:space:]]+([0-9.]+)/\1/p")

deb_usb_gateway=$(sed -nr "${deb_iface_range_regex} p" /etc/network/interfaces |\
		  sed -nr "s/^[[:space:]]*gateway[[:space:]]+([0-9.]+)/\1/p")

sudo sed -i -e 's:'$deb_usb_address':192.168.10.2:g' /etc/network/interfaces
sudo sed -i -e 's:'$deb_usb_network':192.168.10.0:g' /etc/network/interfaces
sudo sed -i -e 's:'$deb_usb_gateway':192.168.10.1:g' /etc/network/interfaces

deb_usb_address=$(sed -nr "${deb_iface_range_regex} p" /etc/network/interfaces |\
		  sed -nr "s/^[[:space:]]*address[[:space:]]+([0-9.]+)/\1/p")

deb_usb_network=$(sed -nr "${deb_iface_range_regex} p" /etc/network/interfaces |\
		  sed -nr "s/^[[:space:]]*network[[:space:]]+([0-9.]+)/\1/p")

deb_usb_gateway=$(sed -nr "${deb_iface_range_regex} p" /etc/network/interfaces |\
		  sed -nr "s/^[[:space:]]*gateway[[:space:]]+([0-9.]+)/\1/p")

echo "usb0, updated: [${deb_usb_address}],[${deb_usb_network}],[${deb_usb_gateway}]"

#SoftAP0 192.168.8.x -> 192.168.11.x
. /etc/default/bb-wl18xx
sudo sed -i -e 's:'$USE_WL18XX_IP_PREFIX':192.168.11:g' /etc/default/bb-wl18xx

sudo sed -i -e 's:#USE_PERSONAL_SSID=:USE_PERSONAL_SSID=BeagleBone-TESTER:g' /etc/default/bb-wl18xx

. /etc/default/bb-wl18xx

echo "wlan0, updated: [${USE_WL18XX_IP_PREFIX}],[${USE_PERSONAL_SSID}]"

wfile="beaglebone-getting-started-2016-11-07.img"
cp /var/cache/doc-beaglebone-getting-started/${wfile} /var/www/html/${wfile}
sha256sum /var/www/html/${wfile} > /var/www/html/${wfile}.sha256sum

echo "wlan0, dl file: [${wfile}] setup on port 8080"

echo "Please Reboot"

