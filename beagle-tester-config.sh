#!/bin/bash
export XAUTHORITY=~debian/.Xauthority 
export DISPLAY=:0.0 
#xrandr --newmode 1280720t  74.50  1280 1344 1472 1664  720 723 728 748 -hsync +vsync
xrandr --newmode 1280x720 74.48  1280 1336 1472 1664  720 721 724 746  -HSync +Vsync
xrandr --addmode default 1280x720
xrandr --output default --mode 1280x720
aplay /usr/share/sounds/alsa/Front_Center.wav