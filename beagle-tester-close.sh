#!/bin/bash
board=$(cat /proc/device-tree/model | sed "s/ /_/g")
case "${board}" in
TI_AM335x_BeagleBone_Blue)
	connmanctl tether wifi on
	rmmod fbtft_device
	;;
TI_AM335x_BeagleBone_Black_Wireless)
	connmanctl tether wifi on
	;;
*)
	;;
esac

systemctl restart getty@tty1.service

