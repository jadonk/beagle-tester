#!/bin/bash
board=$(cat /proc/device-tree/model | sed "s/ /_/g")
case "${board}" in
TI_AM335x_BeagleBone_Blue)
	rmmod fbtft_device
	rmmod fb_ili9341
	rmmod fbtft
	;;
*)
	systemctl restart getty@tty1.service
	systemctl restart lightdm.service
	;;
esac

echo Cleaned up console. Exiting.
