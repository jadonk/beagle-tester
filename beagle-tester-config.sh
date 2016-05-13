#!/bin/bash
export XAUTHORITY=~debian/.Xauthority
#xrandr --newmode 1280720t  74.50  1280 1344 1472 1664  720 723 728 748 -hsync +vsync
xrandr --display :0.0 --newmode 1280720 74.48  1280 1336 1472 1664  720 721 724 746  -HSync +Vsync
xrandr --display :0.0 --addmode default 1280720
xrandr --display :0.0 --output default --mode 1280720
aplay /usr/share/sounds/alsa/Front_Center.wav