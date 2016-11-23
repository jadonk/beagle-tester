#!/bin/bash
board=$(cat /proc/device-tree/model | sed "s/ /_/g")
case "${board}" in
TI_AM335x_BeagleBone_Blue)
	# https://gist.github.com/jadonk/0e4a190fc01dc5723d1f183737af1d83
	echo 49 > /sys/class/gpio/unexport # RESET - V14 - GP0_PIN4
	echo 57 > /sys/class/gpio/unexport # D/C - U16 - GP0_PIN3
	echo 113 > /sys/class/gpio/unexport # CS - C12
	echo gpio > /sys/devices/platform/ocp/ocp\:P9_23_pinmux/state # RESET - V14 - GP0_4
	echo gpio > /sys/devices/platform/ocp/ocp\:U16_pinmux/state # D/C - U16 - GP0_3
	echo spi > /sys/devices/platform/ocp/ocp\:P9_31_pinmux/state # SCLK - A13 - S1.1_5
	echo spi > /sys/devices/platform/ocp/ocp\:P9_29_pinmux/state # MISO - B13 - S1.1_4
	echo spi > /sys/devices/platform/ocp/ocp\:P9_30_pinmux/state # MOSI - D12 - S1.1_3
	#echo gpio > /sys/devices/platform/ocp/ocp\:P9_28_pinmux/state # CS - C12 - S1.1_6
	echo spi > /sys/devices/platform/ocp/ocp\:P9_28_pinmux/state # CS - C12 - S1.1_6
	#modprobe fbtft_device name=adafruit28 busnum=1 rotate=90 gpios=reset:49,dc:57,cs:113
	modprobe fbtft_device name=adafruit28 busnum=1 rotate=90 gpios=reset:49,dc:57
	;;
*)
	;;
esac

systemctl stop getty@tty1.service
export XAUTHORITY=~debian/.Xauthority
#xrandr --newmode 1280720t  74.50  1280 1344 1472 1664  720 723 728 748 -hsync +vsync
xrandr --display :0.0 --newmode 1280720 74.48  1280 1336 1472 1664  720 721 724 746  -HSync +Vsync
xrandr --display :0.0 --addmode default 1280720
xrandr --display :0.0 --output default --mode 1280720
aplay /usr/share/sounds/alsa/Front_Center.wav

