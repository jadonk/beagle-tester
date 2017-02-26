#!/bin/bash
board=$(cat /proc/device-tree/model | sed "s/ /_/g")
case "${board}" in
TI_AM335x_BeagleBone_Blue)
	# disable TFT LCD driver
	#rmmod fb_ili9341
	#rmmod fbtft_device
	#rmmod fbtft
	# restore battery LEDs
	echo 0 > /sys/class/leds/bat0/brightness
	echo 0 > /sys/class/leds/bat1/brightness
	echo 0 > /sys/class/leds/bat2/brightness
	echo 0 > /sys/class/leds/bat3/brightness
	systemctl start battery_monitor.service
	;;
*)
	systemctl restart getty@tty1.service
	systemctl restart lightdm.service
	;;
esac

echo Cleaned up console. Exiting.
