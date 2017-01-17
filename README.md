# beagle-tester

## Supported boards

* [BeagleBone Black](#beaglebone-black)
* [BeagleBone Black Wireless](#beaglebone-black-wireless)
* [BeagleBone Blue](#beaglebone-blue)
* [BeagleBoard-xM](#beagleboard-xm)

## Supported scanners

* [Datalogic QuickScan L](http://www.datalogic.com/eng/products/automatic-data-capture/general-duty-handheld-scanners/quickscan-l-qd2300-pd-166.html) - VID:05f9 "PSC Scanning, Inc." PID:2204 
* [Intermec SG20 General Duty 1D/2D Handheld Scanner](http://www.intermec.com/products/scansg20t/) - VID:067E PID: 0801

## Software setup

For production, the boards should be flashed with an approved production image ahead of beginning this test. No additional software setup steps should be performed. The first 12 characters of the EEPROM should also be valid ahead of running this test, though the additional serial number characters need not and should not be programmed until this test is run.

If not already setup, on a recent [BeagleBoard.org Debian image](https://beagleboard.org/latest-images), perform:

    apt-get update
    DEBIAN_FRONTEND=noninteractive apt-get install -y roboticscape
    cd /opt/source
    git clone https://github.com/jadonk/beagle-tester
    cd beagle-tester
    make && make install
    
## Serial number barcode format

Each board that has an on-board EEPROM should have an associated 16 digit serial number placed onto a barcode on the board.

The first 4 ASCII characters indicate the board type:

* BeagleBone Black - 00C0
* BeagleBone Black Wireless - BWA5
* BeagleBone Blue - BLA2

The second 4 characters should indicate the manufacturing week in the format YYWW, where YY is currently 16 and WW is currently 30.

The next 4 characters should be a manufacturer-specific product code. If you are a new manufacturer, please choose something unique you can use to identify your boards.

Allocations include, but are not limited to: 

* BBGW for GHI manufactured BeagleBone Black Wireless

The final 4 characters are a sequential decimal number. If more than 10,000 boards are manufactured that week, roll over the top digit to an ASCII hex character.

# BeagleBone Black

## Required equipment

* _TV_: HDMI TV capable of 1280x720p60 (720p) (HDMI monitor if no audio testing) with HDMI-to-microHDMI cable
* _Router_: Ethernet router (configured to answer DHCP requests and answer pings on the provided gateway) with Ethernet cable
* _Host_: BeagleBone Black or other computer (configured to make DHCP requests over USB RNDIS interface and answer pings)
* _Scanner_: A supported barcode scanner (listed above) (along with a suitable 16 character barcode on the device under test)
* _Power_: Approved 5V power brick
* _DUT_: BeagleBone Black (device) under test

## Test steps

1. Connect microHDMI on _DUT_ to _TV_
2. Connect Ethernet on _DUT_ to _router_
3. (optional) connect a wire from TP4 to ground to enable EEPROM writing of board revision and serial number
4. Connect _DUT_ to _power_
5. Wait for the BeagleBoard.org desktop to show (should be under 2 minutes)
6. Connect _scanner_
7. Wait for the CISPR test animation and audio playback (should be under 15 seconds) ![CISPR image][cispr]
8. Connect USB client port on _DUT_ to _host_
9. Scan the 16 character barcode (4 character board revision and 12 character serial number) (will be programmed if step 3 completed and other tests pass)
10. Pass or fail will be indicated by a respectively green or red box on the _TV_
11. Disconnect _scanner_
12. Disconnect _host_
13. Disconnect _power_
14. Disconnect remaining devices

# BeagleBone Black Wireless

## Required equipment

* _TV_: HDMI TV capable of 1280x720p60 (720p) (HDMI monitor if no audio testing) with HDMI to microHDMI cable
* _AP_: BeagleBone Black Wireless acting as a WiFi access point (should be default on production image) (SSID: BeagleBone-XXXX, PSK: BeagleBone)
* _Host_: BeagleBone Black Wireless or other computer (configured to make DHCP requests over USB RNDIS interface and answer pings)
* _Scanner_: Supported barcode scanner (listed above) (along with a suitable 16 character barcode on the device under test)
* _Power_: Approved 5V power brick
* _DUT_: BeagleBone Black Wireless (device) under test

## Test steps

1. Ensure _AP_ is functioning nearby 
2. Connect microHDMI on _DUT_ to _TV_
3. (optional) connect a wire from TP1 to ground to enable EEPROM writing of board revision and serial number
4. Connect _DUT_ to _power_
5. Wait for the BeagleBoard.org desktop to show (should be under 2 minutes)
6. Connect USB host on _DUT_ to _scanner_
7. Wait for the CISPR test animation and audio playback (should be under 15 seconds) ![CISPR image][cispr]
8. Connect USB client on _DUT_ to _host_
9. Scan the 16 character barcode (4 character board revision and 12 character serial number) (will be programmed if step 3 completed and other tests pass)
10. Pass or fail will be indicated by a respectively green or red box on the _TV_
11. Disconnect _scanner_
12. Disconnect _host_
13. Disconnect _power_
14. Disconnect remaining devices

# BeagleBone Blue

## Required equipment

* _Display_: Adafruit 2.4" TFT LCD subassembly (See https://gist.github.com/jadonk/0e4a190fc01dc5723d1f183737af1d83)
* _AP_: BeagleBone Blue acting as a WiFi access point (should be default on production image) (SSID: BeagleBone-XXXX, PSK: BeagleBone)
* _Host_: BeagleBone Blue or other computer (configured to make DHCP requests over USB RNDIS interface and answer pings)
* _Scanner_: Supported barcode scanner (listed above) (along with a suitable 16 character barcode on the device under test)
* _Power_: Approved 12V power brick
* _DUT_: BeagleBone Blue (device) under test

## Test steps

1. Ensure _AP_ is functioning nearby
2. Connect S1.1/GP0 connectors on _DUT_ to _display_
3. (optional) connect a wire from WP to GND to enable EEPROM writing of board revision and serial number
4. Connect _DUT_ to _power_
5. Connect USB host on _DUT_ to _scanner_
6. Wait for the CISPR test animation (should be under 2 minutes) ![CISPR image][cispr]
7. Connect USB client on _DUT_ to _host_
8. Scan the 16 character barcode (will be programmed if step 3 completed and other tests pass)
9. Pass or fail will be indicated by a respectively green or red box on the _display_
10. Disconnect _scanner_
11. Disconnect _host_
12. Disconnect _power_
13. Disconnect _display_

# BeagleBoard-xM

## Required equipment

* _TV_: DVI-D/HDMI TV capable of 1280x1024 with HDMI cable
* _Router_: Ethernet router (configured to answer DHCP requests and answer pings on the provided gateway)
* _Host_: BeagleBone Black or other computer (configured to make DHCP requests over USB RNDIS interface and answer pings)
* _Scanner_: Supported barcode scanner (listed above) (along with a suitable 16 character barcode on the device under test)
* _Flashdrives_: Three (3) USB 2.0 HS capable flash drives
* _Speaker_: Speaker with 1/8" audio patch cable
* _Power_: Approved 5V power brick
* _DUT_: BeagleBoard-xM (device) under test

## Test steps

1. Connect HDMI on _DUT_ to _TV_
2. Connect Ethernet on _DUT_ to _router_
3. Connect 3x USB host on _DUT_ to _flashdrives_
4. Connect audio output on _DUT_ to _speaker_
5. Connect _DUT_ to _power_
6. Wait for the BeagleBoard.org desktop to show (should be under 2 minutes)
7. Connect USB host on _DUT_ to _scanner_
8. Wait for the CISPR test animation and audio playback (should be under 15 seconds) ![CISPR image][cispr]
9. Connect USB client on _DUT_ to _host_
10. Scan a barcode to begin the test
11. Pass or fail will be indicated by a respectively green or red box on the _TV_ ![xM pass image][xm-pass]
12. Disconnect _scanner_
13. Disconnect _host_
14. Disconnect _power_
15. Disconnect remaining devices

[cispr]: https://raw.githubusercontent.com/jadonk/beagle-tester/master/images/itu-r-bt1729-colorbar-3200x1800.png
[xm-pass]: https://farm1.staticflickr.com/531/31402272653_86721d4fa5_o_d.png
