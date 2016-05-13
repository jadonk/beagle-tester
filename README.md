# beagle-tester

## Setup

    cd /opt/source
    git clone https://github.com/jadonk/beagle-tester
    cd beagle-tester
    make && make install

## Testing

1. Connect HDMI to TV to see output and hear audio
2. Connect an Ethernet device (router) with a DHCP server that has a gateway that will respond to pings
3. (optional) Connect a wire from TP4 to ground to enable EEPROM writing of board revision and serial number
4. Power the board
5. Connect a USB host (to the USB client port) with RNDIS client support that will perform a DHCP request that will respond to pings
6. Connect one of the supported barcode scanners
7. Wait for the blue/green/red test pattern and audio playback
8. Scan the 16 character barcode (4 character board revision and 12 character serial number) -- will be programmed if wire is included
9. Pass or fail will be indicated by a respectively green or red box on the TV

## Supported boards

* BeagleBone Black
