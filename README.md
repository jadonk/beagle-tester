# beagle-tester

## Supported boards

* BeagleBone Black

## Setup

    cd /opt/source
    git clone https://github.com/jadonk/beagle-tester
    cd beagle-tester
    make && make install

## Testing

### Required equipment

* HDMI TV capable of 1280x720p60 (720p) (HDMI monitor if no audio testing)
* Ethernet router (configured to answer DHCP requests and answer pings on the provided gateway)
* BeagleBone Black or other computer (configured to make DHCP requests over USB RNDIS interface and answer pings)
* A supported barcode scanner (listed below) (along with a suitable 16 character barcode on the device under test)

### Test steps

1. Connect HDMI to TV to see output and hear audio
2. Connect an Ethernet device (router) with a DHCP server that has a gateway that will respond to pings
3. (For BeagleBone Black) (optional) connect a wire from TP4 to ground to enable EEPROM writing of board revision and serial number
4. Power the board
5. Connect a USB host (to the USB client port) with RNDIS client support that will perform a DHCP request that will respond to pings
6. Connect one of the supported barcode scanners
7. Wait for the blue/green/red test pattern and audio playback
8. Scan the 16 character barcode (4 character board revision and 12 character serial number) (will be programmed if BeagleBone Black and wire is included)
9. Pass or fail will be indicated by a respectively green or red box on the TV

## Supported scanners

* [Datalogic QuickScan L](http://www.datalogic.com/eng/products/automatic-data-capture/general-duty-handheld-scanners/quickscan-l-qd2300-pd-166.html)
