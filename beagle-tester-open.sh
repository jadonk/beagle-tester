#!/bin/bash
board=$(cat /proc/device-tree/model | sed "s/ /_/g")
case "${board}" in
TI_AM335x_BeagleBone_Blue)
	# configure battery LEDs
	systemctl stop battery_monitor.service
	echo 1 > /sys/class/leds/bat0/brightness
	echo 1 > /sys/class/leds/bat1/brightness
	echo 1 > /sys/class/leds/bat2/brightness
	echo 1 > /sys/class/leds/bat3/brightness
	# enable TFT LCD driver
	# see https://gist.github.com/jadonk/0e4a190fc01dc5723d1f183737af1d83
	#echo 49 > /sys/class/gpio/unexport # RESET - V14 - GP0_PIN4
	#echo 57 > /sys/class/gpio/unexport # D/C - U16 - GP0_PIN3
	#echo 29 > /sys/class/gpio/unexport # CS - H18
	#echo gpio > /sys/devices/platform/ocp/ocp\:P9_23_pinmux/state # RESET - V14 - GP0_4
	#echo gpio > /sys/devices/platform/ocp/ocp\:U16_pinmux/state # D/C - U16 - GP0_3
	#echo spi > /sys/devices/platform/ocp/ocp\:P9_31_pinmux/state # SCLK - A13 - S1.1_5
	#echo spi > /sys/devices/platform/ocp/ocp\:P9_29_pinmux/state # MISO - B13 - S1.1_4
	#echo spi > /sys/devices/platform/ocp/ocp\:P9_30_pinmux/state # MOSI - D12 - S1.1_3
	#echo spi > /sys/devices/platform/ocp/ocp\:H18_pinmux/state # CS - H18 - S1.1_6
	#modprobe fbtft_device name=adafruit28 busnum=1 rotate=90 gpios=reset:49,dc:57
	#sleep 2
	;;
TI_OMAP3_BeagleBoard_xM)
	amixer -c0 sset 'DAC1 Digital Fine' 40
	amixer -c0 sset 'Headset' 2
	amixer -c0 sset 'HeadsetL Mixer AudioL1' on
	amixer -c0 sset 'HeadsetR Mixer AudioR1' on
	sed -i -e 's:192.168.7.1:192.168.9.1:g' /etc/udhcpd.conf
	ifconfig usb0 192.168.9.2 netmask 255.255.255.252
	systemctl restart udhcpd.service
	;;
*)
	;;
esac

systemctl stop getty@tty1.service
systemctl stop lightdm.service
export XAUTHORITY=~debian/.Xauthority
#xrandr --newmode 1280720t  74.50  1280 1344 1472 1664  720 723 728 748 -hsync +vsync
#xrandr --display :0.0 --newmode 1280720 74.48  1280 1336 1472 1664  720 721 724 746  -HSync +Vsync
#xrandr --display :0.0 --addmode default 1280720
#xrandr --display :0.0 --output default --mode 1280720
aplay /usr/share/sounds/alsa/Front_Center.wav

