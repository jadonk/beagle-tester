/*
 * beagle-tester.c
 *
 * based on evtest and fb-test
 *
 * Author: Jason Kridner <jdk@ti.com>
 * Copyright (c) 1999-2000 Vojtech Pavlik
 * Copyright (c) 2009-2011 Red Hat, Inc
 * Copyright (C) 2009-2012 Tomi Valkeinen
 * Copyright (C) 2016-2018 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef ENABLE_BLUE
#include <rc/adc.h>
#include <rc/bmp.h>
#include <rc/mpu.h>
#include <rc/start_stop.h>
#include <rc/time.h>
#endif

#include <common.h>
void do_fill_screen(struct fb_info *fb_info, int pattern);
void draw_pixel(struct fb_info *fb_info, int x, int y, unsigned color);

#define MODEL_BOARD "TI OMAP3 BeagleBoard"
#define MODEL_XM    "TI OMAP3 BeagleBoard xM"
#define MODEL_X15   "TI AM5728 BeagleBoard-X15"
#define MODEL_BONE  "TI AM335x BeagleBone"
#define MODEL_BLACK "TI AM335x BeagleBone Black"
#define MODEL_WIFI  "TI AM335x BeagleBone Black Wireless"
#define MODEL_AI    "TI AM5729 BeagleBone AI"
#define MODEL_BLUE  "TI AM335x BeagleBone Blue"
#define MODEL_OSD3358_BSM_REF "Octavo Systems OSD3358-SM-RED"
#define COLOR_TEXT 0xffffffu
#define COLOR_PASS 0x00ff00u
#define COLOR_FAIL 0xff0000u
#define SCAN_VALUE_REPEAT "BURN-IN"
#define SCAN_VALUE_COLORBAR "COLORBAR"
#define SCAN_VALUE_STOP "STOP"
int fail = 0;
int notice_line = 0;
int display = 1;

void beagle_test(const char *scan_value);
void beagle_notice(const char *test, const char *status);
void do_colorbar();
#ifdef ENABLE_BLUE
int blue_specific_tests();
#endif
int osd3358_sm_ref_design_tests();
void set_led_trigger(const char * led, const char * mode);
void set_user_leds(int code);
int gpio_out_test(const char *name, unsigned pin);

struct cape
{
	const char prefix[5];
	const char id_str[21];
	int eeprom_addr;
	const char name[33];
	int (*test)(const char *scan_value, unsigned id);
};

int test_comms_cape(const char *scan_value, unsigned id);
int test_display18_cape(const char *scan_value, unsigned id);
int test_display50_cape(const char *scan_value, unsigned id);
int test_display70_cape(const char *scan_value, unsigned id);
int test_load_cape(const char *scan_value, unsigned id);
int test_motor_cape(const char *scan_value, unsigned id);
int test_power_cape(const char *scan_value, unsigned id);
int test_proto_cape(const char *scan_value, unsigned id);
int test_relay_cape(const char *scan_value, unsigned id);
int test_robotics_cape(const char *scan_value, unsigned id);
int test_servo_cape(const char *scan_value, unsigned id);
int test_gamepup_cape(const char *scan_value, unsigned id);
int test_techlab_cape(const char *scan_value, unsigned id);
int test_ppilot_cape(const char *scan_value, unsigned id);
void install_overlay(const char *scan_value, const char *id_str);

/********************************************/
/** This structure matches the barcode     **/
/** header with the test function and info **/
/** needed to program the cape EEPROM      **/
/********************************************/
/* Per https://github.com/beagleboard/capes/blob/master/README.mediawiki */
static struct cape capes[] = {
	{ "BC00", "BBORG_COMMS", 0x56, "Industrial Comms Cape", test_comms_cape },
	{ "BC01", "BBORG_DISPLAY18", 0x57, "1.8\" Display Cape", test_display18_cape },
	{ "BC02", "BBORG_DISPLAY50", 0x57, "5\" Display Cape", test_display50_cape },
	{ "BC03", "BBORG_DISPLAY70", 0x57, "7\" Display Cape", test_display70_cape },
	{ "BC04", "BBORG_LOAD", 0x54, "Load Driver Cape", test_load_cape },
	{ "BC05", "BBORG_MOTOR", 0x55, "Motor Driver Cape", test_motor_cape },
	{ "BC06", "BBORG_POWER", 0, "Power Supply Cape", test_power_cape },
	{ "BC07", "BBORG_PROTO", 0x57, "Prototyping Cape", test_proto_cape },
	{ "BC08", "BBORG_RELAY", 0x54, "Relay Cape", test_relay_cape },
	{ "BC09", "BBORG_ROBOTICS", 0, "Robotics Cape", test_robotics_cape },
	{ "BC0A", "BBORG_SERVO", 0x54, "Servo Cape", test_servo_cape },
	{ "PC00", "BBORG_GAMEPUP", 0x57, "GamePup Cape", test_gamepup_cape },
	{ "PC01", "BBORG_TECHLAB", 0x57, "TechLab Cape", test_techlab_cape },
	{ "PC02", "BBORG_PPILOT", 0x57, "PocketPilot Cape", test_ppilot_cape },
};

/* Per https://github.com/beagleboard/beaglebone-black/wiki/System-Reference-Manual#824-eeprom-data-format */
const char cape_eeprom[88] = {
	0xAA, 0x55, 0x33, 0xEE,		/* header (4) */
	0x41, 0x31,			/* format version (2) */
	0, 0, 0, 0, 0, 0, 0, 0,		/* board name (32)@6 */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0x30, 0x30, 0x30, 0x30,		/* board version (4)@38 */
	'B', 'e', 'a', 'g', 'l', 'e',	/* manufacturer (16)@42 */
	'B', 'o', 'a', 'r', 'd', '.',
	'o', 'r', 'g', 0,
	0, 0, 0, 0, 0, 0, 0, 0,		/* part number (16)@58 */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,				/* number of pins (2)@60 */
	0, 0, 0, 0, 0, 0, 0, 0,		/* serial number (12)@76 */
	0, 0, 0, 0
};

#ifndef ENABLE_BLUE
volatile int exiting = 0;
#endif

static void do_stop()
{
#ifdef ENABLE_BLUE
	rc_set_state(EXITING);
#else
	exiting = 1;
#endif
}

int main(int argc, char** argv)
{
	unsigned short barcode_id[4];
	int barcode = open("/dev/input/beagle-barcode", O_RDONLY);
	fd_set rdfs;
	struct input_event ev[256];
	int i, n, rd = 0, run = 0;
	struct timeval timeout;
	char scan_value[32];
	int scan_i = 0;

	fprintf(stderr, "Starting beagle-tester.\n");
	fflush(stderr);

	if (!barcode) {
		fprintf(stderr, "ERROR: valid barcode scanner not found.\n");
		fflush(stderr);
	}

	//FILE *errlog = fopen("/var/log/beagle-tester.log", "w");
	ioctl(barcode, EVIOCGID, barcode_id);
	fprintf(stderr, "Found input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
		barcode_id[ID_BUS], barcode_id[ID_VENDOR], barcode_id[ID_PRODUCT], barcode_id[ID_VERSION]);
	fflush(stderr);

	system("/usr/sbin/beagle-tester-open.sh");
	set_led_trigger("red", "default-on");
	set_led_trigger("green", "default-on");
	set_led_trigger("beaglebone:green:usr0", "default-on");
	set_led_trigger("beaglebone:green:usr1", "default-on");
	set_led_trigger("beaglebone:green:usr2", "default-on");
	set_led_trigger("beaglebone:green:usr3", "default-on");
	set_led_trigger("beaglebone:green:usr4", "default-on");

	if (access("/dev/fb0", W_OK)) {
		fprintf(stderr, "Unable to write to /dev/fb0\n");
		fflush(stderr);
		display = 0;
	} else {
		fb_open(0, &fb_info);
		do_colorbar();
	}

	signal(SIGINT, do_stop);
	signal(SIGTERM, do_stop);

	/********************************************/
	/** This is the main execution loop        **/
	/********************************************/
#ifdef ENABLE_BLUE
	while (rc_get_state()!=EXITING) {
#else
	while (!exiting) {
#endif
		FD_ZERO(&rdfs);
		FD_SET(barcode, &rdfs);
		timeout.tv_sec = 0;
		timeout.tv_usec = 4000;
		rd = select(barcode + 1, &rdfs, NULL, NULL, &timeout);
#ifdef ENABLE_BLUE
		if (rc_get_state()==EXITING)
#else
		if (exiting)
#endif
			break;
		if (rd > 0) {
			rd = read(barcode, ev, sizeof(ev));
		}

		/********************************************/
		/** Handle barcode scanner raw input       **/
		/********************************************/
		for (i = 0; i < rd; i += (int)sizeof(struct input_event)) {
			unsigned int type, code, value;

			if ((rd % (int) sizeof(struct input_event)) != 0) {
				//fprintf(stderr, "Data too small: %d\n", rd); fflush(stderr);
				break;
			}

			n = i / (int) sizeof(struct input_event);

			type = ev[n].type;
			code = ev[n].code;
			value = ev[n].value;

			//fprintf(stderr, "Event: %02x %02x %06x\n", type, code, value); fflush(stderr);

			if ((type == 1) && (value == 1)) {
				switch (code) {
				default:
					break;
				case KEY_ENTER:
					scan_value[scan_i] = 0;
					fprintf(stderr, "Got scanned value: %s\n", scan_value);
					fflush(stderr);
					run = 1;
					scan_i = 0;
					fail = 0;
					break;
				case KEY_MINUS:
					scan_value[scan_i] = '-';
					scan_i++;
					break;
				case KEY_0:
					scan_value[scan_i] = '0';
					scan_i++;
					break;
				case KEY_1:
					scan_value[scan_i] = '1';
					scan_i++;
					break;
				case KEY_2:
					scan_value[scan_i] = '2';
					scan_i++;
					break;
				case KEY_3:
					scan_value[scan_i] = '3';
					scan_i++;
					break;
				case KEY_4:
					scan_value[scan_i] = '4';
					scan_i++;
					break;
				case KEY_5:
					scan_value[scan_i] = '5';
					scan_i++;
					break;
				case KEY_6:
					scan_value[scan_i] = '6';
					scan_i++;
					break;
				case KEY_7:
					scan_value[scan_i] = '7';
					scan_i++;
					break;
				case KEY_8:
					scan_value[scan_i] = '8';
					scan_i++;
					break;
				case KEY_9:
					scan_value[scan_i] = '9';
					scan_i++;
					break;
				case KEY_A:
					scan_value[scan_i] = 'A';
					scan_i++;
					break;
				case KEY_B:
					scan_value[scan_i] = 'B';
					scan_i++;
					break;
				case KEY_C:
					scan_value[scan_i] = 'C';
					scan_i++;
					break;
				case KEY_D:
					scan_value[scan_i] = 'D';
					scan_i++;
					break;
				case KEY_E:
					scan_value[scan_i] = 'E';
					scan_i++;
					break;
				case KEY_F:
					scan_value[scan_i] = 'F';
					scan_i++;
					break;
				case KEY_G:
					scan_value[scan_i] = 'G';
					scan_i++;
					break;
				case KEY_H:
					scan_value[scan_i] = 'H';
					scan_i++;
					break;
				case KEY_I:
					scan_value[scan_i] = 'I';
					scan_i++;
					break;
				case KEY_J:
					scan_value[scan_i] = 'J';
					scan_i++;
					break;
				case KEY_K:
					scan_value[scan_i] = 'K';
					scan_i++;
					break;
				case KEY_L:
					scan_value[scan_i] = 'L';
					scan_i++;
					break;
				case KEY_M:
					scan_value[scan_i] = 'M';
					scan_i++;
					break;
				case KEY_N:
					scan_value[scan_i] = 'N';
					scan_i++;
					break;
				case KEY_O:
					scan_value[scan_i] = 'O';
					scan_i++;
					break;
				case KEY_P:
					scan_value[scan_i] = 'P';
					scan_i++;
					break;
				case KEY_Q:
					scan_value[scan_i] = 'Q';
					scan_i++;
					break;
				case KEY_R:
					scan_value[scan_i] = 'R';
					scan_i++;
					break;
				case KEY_S:
					scan_value[scan_i] = 'S';
					scan_i++;
					break;
				case KEY_T:
					scan_value[scan_i] = 'T';
					scan_i++;
					break;
				case KEY_U:
					scan_value[scan_i] = 'U';
					scan_i++;
					break;
				case KEY_V:
					scan_value[scan_i] = 'V';
					scan_i++;
					break;
				case KEY_W:
					scan_value[scan_i] = 'W';
					scan_i++;
					break;
				case KEY_X:
					scan_value[scan_i] = 'X';
					scan_i++;
					break;
				case KEY_Y:
					scan_value[scan_i] = 'Y';
					scan_i++;
					break;
				case KEY_Z:
					scan_value[scan_i] = 'Z';
					scan_i++;
					break;
				}
			}
			//fprintf(stderr, "*"); fflush(stderr);
		}

		/********************************************/
		/** Process command-line inputs            **/
		/********************************************/
		if(scan_i == 0 && argc > 1) {
			strcpy(scan_value, argv[argc-1]);
			run = 1;
			argc--;
		}

		/********************************************/
		/** Display CISPR colorbar while idle      **/
		/********************************************/
		if (run == 0) {
			if (display) do_colorbar();
		}
		/********************************************/
		/** Handle case where STOP is requested    **/
		/********************************************/
		if (!strcmp(scan_value, SCAN_VALUE_STOP)) {
#ifdef ENABLE_BLUE
			rc_set_state(EXITING);
#else
			exiting = 1;
#endif
			break;
		}
		/********************************************/
		/** Handle a request to run a test         **/
		/********************************************/
		else if (run == 1) {
			if (display) do_fill_screen(&fb_info, 0);
			beagle_test(scan_value);
			fprintf(stderr, "Test fails: %d\n", fail);
			fflush(stderr);
			if (fail > 0) {
				printf("RESULT: \033[41;30;5m FAIL \033[0m\n");
			} else {
 				printf("RESULT: PASS \n");
			}
#ifdef ENABLE_BLUE
			if (rc_get_state()==EXITING) {
#else
			if (exiting) {
#endif
				run = 0;
				break;	
			} else if (!strcmp(scan_value, SCAN_VALUE_REPEAT)) {
				// pause 4 seconds and run again
				sleep(4);
			} else {
				memset(scan_value, 0, sizeof(scan_value));
				run = 2;
			}
		}
	}

	if (display) do_fill_screen(&fb_info, 4);
	set_user_leds(-1);
	system("/usr/sbin/beagle-tester-close.sh");
	set_led_trigger("red", "none");
	set_led_trigger("green", "none");

	return 0;
}

void beagle_test(const char *scan_value)
{
	int r;
	int fd_sn;
	char str[120];
	char str2[50];
	char wlan0_ap[50];
	char wlan0_host[50];
        int len, off;
	char model[70];
	FILE *fp;
	unsigned x, y;
	unsigned color;

	notice_line = 0;
	beagle_notice("scan", scan_value);

#ifdef VERSION
	beagle_notice("tester", VERSION);
#else
	beagle_notice("tester", "$Id$");
#endif

	/********************************************/
	/** Handle case test is on a cape          **/
	/** Use the ID and cape array to call test **/
	/********************************************/
	if(!strncmp(scan_value, "BC", 2) || !strncmp(scan_value, "PC", 2)) {
		for(x = 0; x < sizeof(capes) / sizeof(capes[0]); x++) {
			if(!strncmp(scan_value, capes[x].prefix, 4)) {
				beagle_notice("model", capes[x].name);
				fail = capes[x].test(scan_value, x);
				goto done;
			}
		}
	}

	fp = fopen("/proc/device-tree/model", "r");
	fgets(str, sizeof(str), fp);
	fclose(fp);
	strcpy(model, str);
        len = strlen(str);
	len--; str[len] = 0; // remove trailing character
        off = (len > 25) ? len-25 : 0;
	beagle_notice("model", &str[off]);
	if(!strcmp(model, MODEL_BLACK)) {
		if(strncmp(scan_value, "00", 2)) {
			beagle_notice("model", "fail");
			fail = 1;
			goto done;
		}
	}
	else if(!strcmp(model, MODEL_WIFI)) {
		if(strncmp(scan_value, "BW", 2)) {
			beagle_notice("model", "fail");
			fail = 1;
			goto done;
		}
	}
	else if(!strcmp(model, MODEL_BLUE)) {
		if(strncmp(scan_value, "BL", 2)) {
			beagle_notice("model", "fail");
			fail = 1;
			goto done;
		}
	}

	fd_sn = open("/sys/bus/i2c/devices/i2c-0/0-0050/0-00500/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 28);
	str[28] = 0;
	beagle_notice("eeprom", str);

	fp = fopen("/etc/dogtag", "r");
	fgets(str, sizeof(str), fp);
	fclose(fp);
        len = strlen(str);
	len--; str[len] = 0; // remove trailing character
        off = (len > 25) ? len-25 : 0;
	beagle_notice("dogtag", &str[off]);

	strcpy(str, "memtester 1M 1 > /dev/null");
	fprintf(stderr, str);
	fprintf(stderr, "\n");
	fflush(stderr);
	r = system(str);
	beagle_notice("memory", r ? "fail" : "pass");

	// if we have WiFi
	if(!strcmp(model, MODEL_WIFI) || !strcmp(model, MODEL_BLUE)) {
		// connect to ap
		system("bb-connect-ap > /tmp/beagle-tester-ap");
		fp = fopen("/tmp/beagle-tester-ap", "r");
		if (fp != NULL) {
			fgets(str2, sizeof(str2)-1, fp);
			str2[25] = 0;
			fclose(fp);
		} else {
			str2[0] = 0;
		}
		beagle_notice("ap", str2);

		// fetch wlan0 address
		system("ip -4 addr show wlan0 | grep inet | awk '{print $2}' | cut -d/ -f1 | tr -d '\n' | tr -d '\r' > /tmp/beagle-tester-wlan0-host");
		fp = fopen("/tmp/beagle-tester-wlan0-host", "r");
		if (fp != NULL) {
			fgets(wlan0_host, sizeof(wlan0_host)-1, fp);
			fclose(fp);
		} else {
			wlan0_host[0] = 0;
		}

		// fetch wlan0 gateway
		system("ip route | grep -E 'wlan0|link' | grep -Ev 'src|default' | awk '{print $1}' > /tmp/beagle-tester-wlan0-gw");
		fp = fopen("/tmp/beagle-tester-wlan0-gw", "r");
		if (fp != NULL) {
			fgets(wlan0_ap, sizeof(wlan0_ap)-1, fp);
			fclose(fp);
		} else {
			wlan0_ap[0] = 0;
		}

		sprintf(str, "ping -c 4 -w 10 -I %s %s",
			wlan0_host, wlan0_ap);
		fprintf(stderr, str);
		fprintf(stderr, "\n");
		fflush(stderr);
		r = system(str);
		fprintf(stderr, "ping returned: %d\n", r);
		beagle_notice("wifi", r ? "fail" : "pass");
	} else { // Ethernet
		fp = popen("ip route get 1.1.1.1 | perl -n -e 'print $1 if /via (.*) dev/'",
			 "r"); // fetch gateway
		if (fp != NULL) {
			fgets(str2, sizeof(str2)-1, fp);
			pclose(fp);
		} else {
			str2[0] = 0;
		}
		sprintf(str, "ping -s 8184 -i 0.01 -q -c 150 -w 2 -I eth0 %s",
			 str2);
		fprintf(stderr, str);
		fprintf(stderr, "\n");
		fflush(stderr);
		r = system(str);
		beagle_notice("ethernet", r ? "fail" : "pass");
	}

	// if BeagleBoard-xM
	if(!strcmp(model, MODEL_XM)) {
		sprintf(str, "ping -s 8184 -i 0.01 -q -c 90 -w 2 -I 192.168.9.2 192.168.9.1");
	} else {
		sprintf(str, "ping -s 8184 -i 0.01 -q -c 150 -w 2 -I 192.168.7.2 192.168.7.1");
	}
	fprintf(stderr, str);
	fprintf(stderr, "\n");
	fflush(stderr);
	r = system(str);
	beagle_notice("usb dev", r ? "fail" : "pass");

	// if BeagleBoard-xM
	if(!strcmp(model, MODEL_XM)) {
		sprintf(str, "timeout 8 hdparm -q -t --direct /dev/sda");
		fprintf(stderr, str);
		fprintf(stderr, "\n");
		fflush(stderr);
		r = system(str);
		beagle_notice("usb 1", r ? "fail" : "pass");

		sprintf(str, "timeout 8 hdparm -q -t --direct /dev/sdb");
		fprintf(stderr, str);
		fprintf(stderr, "\n");
		fflush(stderr);
		r = system(str);
		beagle_notice("usb 2", r ? "fail" : "pass");

		sprintf(str, "timeout 8 hdparm -q -t --direct /dev/sdc");
		fprintf(stderr, str);
		fprintf(stderr, "\n");
		fflush(stderr);
		r = system(str);
		beagle_notice("usb 3", r ? "fail" : "pass");
	}

#ifdef ENABLE_BLUE
	// if BeagleBone Blue
	if(!strcmp(model, MODEL_BLUE)) {
		r = blue_specific_tests();
		beagle_notice("sensors", r ? "fail" : "pass");
	}
#endif
	
	// If OSD3358-SM-RED
	if(!strcmp(model, MODEL_OSD3358_BSM_REF)){
		r = osd3358_sm_ref_design_tests();
		beagle_notice("OSD3358-SM Reference Design board components", r ? "fail" : "pass");
	}

	// if not xM nor X15, didn't fail and we aren't in repeat mode
	if(strcmp(model, MODEL_XM) && strcmp(model, MODEL_X15) &&
			!fail && strcmp(scan_value, SCAN_VALUE_REPEAT)) {
		lseek(fd_sn, 0, SEEK_SET);
		r = read(fd_sn, str, 12);

		/* TODO: How do we properly decide how to assign the EEPROM? */
		/* DANGEROUS!!!: This will make everything a BeagleBone
		 * Black derivative. This will break PocketBeagles and X15s */
		memcpy(&str[0], "\xaa\x55\x33\xee\x41\x33\x33\x35\x42\x4e\x4c\x54", 12);

		memcpy(&str[12], scan_value, 16);
		str[28] = 0;
		lseek(fd_sn, 0, SEEK_SET);
		r = write(fd_sn, str, 28);
		lseek(fd_sn, 0, SEEK_SET);
		r = read(fd_sn, str2, 28);
		str2[28] = 0;
		beagle_notice("eeprom", str2);
		fail = strcmp(str, str2) ? 1 : 0;
		beagle_notice("eeprom", fail ? "fail" : "pass");
	}

	close(fd_sn);

done:
	color = fail ? COLOR_FAIL : COLOR_PASS;
	if (display) {
		for (y = fb_info.var.yres/2; y < fb_info.var.yres; y++) {
			for (x = fb_info.var.xres/2; x < fb_info.var.xres; x++)
				draw_pixel(&fb_info, x, y, color);
		}
	}

	if (fail) {
		set_led_trigger("red", "timer");
		set_led_trigger("green", "none");
	} else {
		set_led_trigger("beaglebone:green:usr0", "default-on");
		set_led_trigger("beaglebone:green:usr1", "default-on");
		set_led_trigger("beaglebone:green:usr2", "default-on");
		set_led_trigger("beaglebone:green:usr3", "default-on");
		set_led_trigger("beaglebone:green:usr4", "default-on");
		set_led_trigger("red", "none");
		set_led_trigger("green", "timer");
	}
}

void beagle_notice(const char *test, const char *status)
{
	const char *fmt = "%8.8s: %-25.25s";
	unsigned color = COLOR_TEXT;
	char str[70];

	set_user_leds(notice_line);

	if(!strcmp(status, "fail")) {
		fail++;
		color = COLOR_FAIL;
	}
	sprintf(str, fmt, test, status);
	fprintf(stderr, str);
	fprintf(stderr, "\n");
	fflush(stderr);
	if (display)
		fb_put_string(&fb_info, 20, 50+notice_line*10, str, 70, color, 1, 70);
	notice_line++;
}

void do_colorbar()
{
	static int init = 0;
	static int cur_x = 0, cur_dir = 0;
	int x, y;

	if (!init) {
		if (fb_info.var.xres == 1920 && fb_info.var.bits_per_pixel == 32)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-2048x1080-32.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 1280 && fb_info.var.bits_per_pixel == 32)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-1280x1024-32.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 1024 && fb_info.var.bits_per_pixel == 32)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-1024x768-32.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 320)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-320x240.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 480)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-480x272.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 800)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-800x600.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 1024)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-1024x768.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 1088)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-1088x1920.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 1280)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-1280x720.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 1360)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-1360x768.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 1920)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-1920x1080.raw.xz > /dev/fb0");
		else if (fb_info.var.xres == 128)
			system("xzcat /usr/share/beagle-tester/itu-r-bt1729-colorbar-128x160.raw.xz > /dev/fb0");
		else
			system("cat /dev/zero > /dev/fb0");
		init = 1;
	}

	for (x = cur_x; x < cur_x+4; x++)
		for (y = (388*(int)fb_info.var.yres)/480; y < (405*(int)fb_info.var.yres)/480; y++)
			draw_pixel(&fb_info, x+(int)fb_info.var.xres/2, y, 0x000000);

	if (cur_dir == 0) {
		cur_x++;
		if (cur_x >= 56*(int)fb_info.var.xres/400) cur_dir = 1;
	} else {
		cur_x--;
		if (cur_x <= -58*(int)fb_info.var.xres/400) cur_dir = 0;
	}

	for (x = cur_x; x < cur_x+4; x++)
		for (y = (388*(int)fb_info.var.yres)/480; y < (405*(int)fb_info.var.yres)/480; y++)
			draw_pixel(&fb_info, x+(int)fb_info.var.xres/2, y, 0xffffff);
	
	//usleep(4444);
}

#ifdef ENABLE_BLUE
int blue_specific_tests() {
	int ret;

	// use defaults for now, except also enable magnetometer.
	float v;
	rc_mpu_data_t data;
	rc_mpu_config_t conf = rc_mpu_default_config();
	conf.i2c_bus = 2;
	conf.gpio_interrupt_pin = 117;
	conf.enable_magnetometer = 1;

	if(rc_adc_init()){
		fprintf(stderr, "ERROR: rc_adc_init() failed to initialize adc\n");
		return -1;
	}

	// check charger by checking for the right voltage on the batt line
	v = rc_adc_batt();
	fprintf(stderr, "battery input/charger voltage: %.2fV\n", v);
	if(v>10.0 || v<6.0) {
		fprintf(stderr, "ERROR: battery input voltage out of spec\n");
		rc_adc_cleanup();
		return -1;
	}

	// make sure 12V DC supply is connected
	v = rc_adc_dc_jack();
	fprintf(stderr, "dc jack input voltage: %.2fV\n", v);
	if(v<10.0) {
		fprintf(stderr, "ERROR: dc jack voltage too low\n");
		rc_adc_cleanup();
		return -2;
	}

	rc_adc_cleanup();

	// test imu
	ret = rc_mpu_initialize_dmp(&data, conf);
	rc_mpu_power_off();
	if(ret<0) {
		fprintf(stderr, "failed: mpu9250 imu\n");
		return -3;
	}

	// test barometer
	ret = rc_bmp_init(BMP_OVERSAMPLE_16,BMP_FILTER_OFF);
	rc_bmp_power_off();
	if(ret<0) {
		fprintf(stderr, "failed: bmp280 barometer\n");
		rc_mpu_power_off();
		return -4;
	}

	//cleanup_cape();
	return 0;
}
#endif

void set_led_trigger(const char * led, const char * mode)
{
	int fd;
	char path[100];
	int mode_len;

	sprintf(path, "/sys/class/leds/%s/trigger", led);
	mode_len = strlen(mode);
	fd = open(path, O_WRONLY);
	if(!fd) return;
	write(fd, mode, mode_len);
	close(fd);
}

void set_user_leds(int code)
{
	if (code < 0) {
		set_led_trigger("beaglebone:green:usr0", "heartbeat");
		set_led_trigger("beaglebone:green:usr1", "mmc0");
		set_led_trigger("beaglebone:green:usr2", "cpu");
		set_led_trigger("beaglebone:green:usr3", "mmc1");
		set_led_trigger("beaglebone:green:usr4", "phy0assoc");
	} else {
		set_led_trigger("beaglebone:green:usr0", (code & 1) ? "timer" : "none");
		set_led_trigger("beaglebone:green:usr1", (code & 2) ? "timer" : "none");
		set_led_trigger("beaglebone:green:usr2", (code & 4) ? "timer" : "none");
		set_led_trigger("beaglebone:green:usr3", (code & 8) ? "timer" : "none");
		set_led_trigger("beaglebone:green:usr4", (code & 16) ? "timer" : "none");
	}
}

/********************************************/
/** Cape tests start here                  **/
/********************************************/
		
/* BC0000A2yywwnnnnnnnn */
int test_comms_cape(const char *scan_value, unsigned id)
{
	int r;
	int fd_sn;
	char str[120];
	char str2[120];

	install_overlay(scan_value, capes[id].id_str);

	fd_sn = open("/sys/bus/i2c/devices/i2c-2/2-0056/2-00560/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 88);
	if(r < 0)
		printf("EEPROM read failure in test_comms_cape()\n");
	str[89] = 0;
	beagle_notice("name", &str[6]);

	gpio_out_test("sinkA", 49);
	gpio_out_test("sinkB", 48);

	memcpy(str, cape_eeprom, 88);
	strcpy(&str[6], capes[id].name);	/* board name */
	memcpy(&str[38], &scan_value[4], 4);	/* board version */
	strcpy(&str[58], capes[id].id_str);	/* part number */
	strncpy(&str[76], &scan_value[8], 16);	/* serial number */
	str[89] = 0;
	lseek(fd_sn, 0, SEEK_SET);
	r = write(fd_sn, str, 88);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str2, 88);
	str2[89] = 0;
	beagle_notice("name", &str2[6]);
	beagle_notice("ver/mfr", &str2[38]);
	beagle_notice("partno", &str2[58]);
	beagle_notice("serial", &str2[76]);
	fail = memcmp(str, str2, 88) ? 1 : 0;
	beagle_notice("eeprom", fail ? "fail" : "pass");

	close(fd_sn);
	return(fail);
}

int test_display18_cape(const char *scan_value, unsigned id)
{
	printf("%s %d - not supported\n", scan_value, id);
	fail++;
	return(fail);
}

int test_display50_cape(const char *scan_value, unsigned id)
{
	printf("%s %d - not supported\n", scan_value, id);
	fail++;
	return(fail);
}

int test_display70_cape(const char *scan_value, unsigned id)
{
	printf("%s %d - not supported\n", scan_value, id);
	fail++;
	return(fail);
}

/* BC0400A2yywwnnnnnnnn */
int test_load_cape(const char *scan_value, unsigned id)
{
	int r;
	int fd_sn;
	char str[120];
	char str2[120];

	install_overlay(scan_value, capes[id].id_str);

	fd_sn = open("/sys/bus/i2c/devices/i2c-2/2-0054/2-00540/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 88);
	if(r < 0)
		printf("EEPROM read failure in test_load_cape()\n");
	str[89] = 0;
	beagle_notice("name", &str[6]);

	gpio_out_test("sink1", 7);
	gpio_out_test("sink2", 20);
	gpio_out_test("sink3", 112);
	gpio_out_test("sink4", 115);
	gpio_out_test("sink5", 44);
	gpio_out_test("sink6", 45);
	gpio_out_test("sink7", 47);
	gpio_out_test("sink8", 27);

	memcpy(str, cape_eeprom, 88);
	strcpy(&str[6], capes[id].name);	/* board name */
	memcpy(&str[38], &scan_value[4], 4);	/* board version */
	strcpy(&str[58], capes[id].id_str);	/* part number */
	strncpy(&str[76], &scan_value[8], 16);	/* serial number */
	str[89] = 0;
	lseek(fd_sn, 0, SEEK_SET);
	r = write(fd_sn, str, 88);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str2, 88);
	str2[89] = 0;
	beagle_notice("name", &str2[6]);
	beagle_notice("ver/mfr", &str2[38]);
	beagle_notice("partno", &str2[58]);
	beagle_notice("serial", &str2[76]);
	fail = memcmp(str, str2, 88) ? 1 : 0;
	beagle_notice("eeprom", fail ? "fail" : "pass");

	close(fd_sn);

	return(fail);
}

/* BC0500A2yywwnnnnnnnn */
int test_motor_cape(const char *scan_value, unsigned id)
{
	int r;
	int fd_sn;
	char str[120];
	char str2[120];
	const char *sleep = "sleep 1";

	install_overlay(scan_value, capes[id].id_str);

	fd_sn = open("/sys/bus/i2c/devices/i2c-2/2-0055/2-00550/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 88);
	if(r < 0)
		printf("EEPROM read failure in test_motor_cape()\n");
	str[89] = 0;
	beagle_notice("name", &str[6]);

	/* Export PWMs */
	/* Motor 1 */
	system("bash -c 'echo 1 > /sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip*/export'");
	/* Motor 2 */
	system("bash -c 'echo 0 > /sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip*/export'");
	/* Motor 3 */
	system("bash -c 'echo 1 > /sys/devices/platform/ocp/48304000.epwmss/48304200.pwm/pwm/pwmchip*/export'");
	/* Motor 4 */
	system("bash -c 'echo 0 > /sys/devices/platform/ocp/48304000.epwmss/48304200.pwm/pwm/pwmchip*/export'");
	system(sleep);

	/* Test Motor 1 */
	beagle_notice("M1", "low");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*1/period'");
	system("bash -c 'echo 50000 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*1/duty_cycle'");
	system("bash -c 'echo 1 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*1/enable'");
	system("echo pwm > /sys/devices/platform/ocp/ocp:P9_16_pinmux/state");
	system(sleep);
	beagle_notice("M1", "high");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*1/duty_cycle'");
	system(sleep);
	system("echo gpio > /sys/devices/platform/ocp/ocp:P9_16_pinmux/state");
	system("bash -c 'echo 0 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*1/enable'");

	/* Test Motor 2 */
	beagle_notice("M2", "low");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*0/period'");
	system("bash -c 'echo 50000 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*0/duty_cycle'");
	system("bash -c 'echo 1 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*0/enable'");
	system("echo pwm > /sys/devices/platform/ocp/ocp:P9_14_pinmux/state");
	system(sleep);
	beagle_notice("M2", "high");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*0/duty_cycle'");
	system(sleep);
	system("echo gpio > /sys/devices/platform/ocp/ocp:P9_14_pinmux/state");
	system("bash -c 'echo 0 > /sys/devices/platform/ocp/48302000.*/48302200.*/pwm/pwmchip*/pwm*0/enable'");

	/* Test Motor 3 */
	beagle_notice("M3", "low");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*1/period'");
	system("bash -c 'echo 50000 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*1/duty_cycle'");
	system("bash -c 'echo 1 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*1/enable'");
	system("echo pwm > /sys/devices/platform/ocp/ocp:P8_13_pinmux/state");
	system(sleep);
	beagle_notice("M3", "high");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*1/duty_cycle'");
	system(sleep);
	system("echo gpio > /sys/devices/platform/ocp/ocp:P8_13_pinmux/state");
	system("bash -c 'echo 0 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*1/enable'");

	/* Test Motor 4 */
	beagle_notice("M4", "low");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*0/period'");
	system("bash -c 'echo 50000 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*0/duty_cycle'");
	system("bash -c 'echo 1 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*0/enable'");
	system("echo pwm > /sys/devices/platform/ocp/ocp:P8_19_pinmux/state");
	system(sleep);
	beagle_notice("M4", "high");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*0/duty_cycle'");
	system(sleep);
	system("echo gpio > /sys/devices/platform/ocp/ocp:P8_19_pinmux/state");
	system("bash -c 'echo 0 > /sys/devices/platform/ocp/48304000.*/48304200.*/pwm/pwmchip*/pwm*0/enable'");

	memcpy(str, cape_eeprom, 88);
	strcpy(&str[6], capes[id].name);	/* board name */
	memcpy(&str[38], &scan_value[4], 4);	/* board version */
	strcpy(&str[58], capes[id].id_str);	/* part number */
	strncpy(&str[76], &scan_value[8], 16);	/* serial number */
	str[89] = 0;
	lseek(fd_sn, 0, SEEK_SET);
	r = write(fd_sn, str, 88);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str2, 88);
	str2[89] = 0;
	beagle_notice("name", &str2[6]);
	beagle_notice("ver/mfr", &str2[38]);
	beagle_notice("partno", &str2[58]);
	beagle_notice("serial", &str2[76]);
	fail = memcmp(str, str2, 88) ? 1 : 0;
	beagle_notice("eeprom", fail ? "fail" : "pass");

	close(fd_sn);

	return(fail);
}

int test_power_cape(const char *scan_value, unsigned id)
{
	printf("%s %d - not supported\n", scan_value, id);
	fail++;
	return(fail);
}

/* BC0700A2yywwnnnnnnnn */
int test_proto_cape(const char *scan_value, unsigned id)
{
	int r;
	int fd_sn;
	char str[120];
	char str2[120];

	install_overlay(scan_value, capes[id].id_str);

	fd_sn = open("/sys/bus/i2c/devices/i2c-2/2-0057/2-00570/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 88);
	if(r < 0)
		printf("EEPROM read failure in test_proto_cape()\n");
	str[89] = 0;
	beagle_notice("name", &str[6]);

	gpio_out_test("LED", 68);
	gpio_out_test("Blue", 44);
	gpio_out_test("Red", 26);
	gpio_out_test("Green", 46);

	memcpy(str, cape_eeprom, 88);
	strcpy(&str[6], capes[id].name);	/* board name */
	memcpy(&str[38], &scan_value[4], 4);	/* board version */
	strcpy(&str[58], capes[id].id_str);	/* part number */
	strncpy(&str[76], &scan_value[8], 16);	/* serial number */
	str[89] = 0;
	lseek(fd_sn, 0, SEEK_SET);
	r = write(fd_sn, str, 88);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str2, 88);
	str2[89] = 0;
	beagle_notice("name", &str2[6]);
	beagle_notice("ver/mfr", &str2[38]);
	beagle_notice("partno", &str2[58]);
	beagle_notice("serial", &str2[76]);
	fail = memcmp(str, str2, 88) ? 1 : 0;
	beagle_notice("eeprom", fail ? "fail" : "pass");

	close(fd_sn);
	return(fail);
}

/* BC0800A2yywwnnnnnnnn */
int test_relay_cape(const char *scan_value, unsigned id)
{
	int r;
	int fd_sn;
	char str[120];
	char str2[120];

	install_overlay(scan_value, capes[id].id_str);

	fd_sn = open("/sys/bus/i2c/devices/i2c-2/2-0054/2-00540/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 88);
	if(r < 0)
		printf("EEPROM read failure in test_relay_cape()\n");
	str[89] = 0;
	beagle_notice("name", &str[6]);

	gpio_out_test("relay1", 20);
	gpio_out_test("relay2", 7);
	gpio_out_test("relay3", 112);
	gpio_out_test("relay4", 115);

	//for(r = 0; r < 88; r++) printf("%02x", cape_eeprom[r]); printf("\n");
	memcpy(str, cape_eeprom, 88);
	//for(r = 0; r < 88; r++) printf("%02x", str[r]); printf("\n");
	strcpy(&str[6], capes[id].name);	/* board name */
	//for(r = 0; r < 88; r++) printf("%02x", str[r]); printf("\n");
	memcpy(&str[38], &scan_value[4], 4);	/* board version */
	//for(r = 0; r < 88; r++) printf("%02x", str[r]); printf("\n");
	strcpy(&str[58], capes[id].id_str);	/* part number */
	//for(r = 0; r < 88; r++) printf("%02x", str[r]); printf("\n");
	strncpy(&str[76], &scan_value[8], 16);	/* serial number */
	str[89] = 0;
	//for(r = 0; r < 88; r++) printf("%02x", str[r]); printf("\n");
	lseek(fd_sn, 0, SEEK_SET);
	r = write(fd_sn, str, 88);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str2, 88);
	str2[89] = 0;
	beagle_notice("name", &str2[6]);
	beagle_notice("ver/mfr", &str2[38]);
	beagle_notice("partno", &str2[58]);
	beagle_notice("serial", &str2[76]);
	//for(r = 0; r < 88; r++) printf("%02x", str[r]); printf("\n");
	//for(r = 0; r < 88; r++) printf("%02x", str2[r]); printf("\n");
	fail = memcmp(str, str2, 88) ? 1 : 0;
	beagle_notice("eeprom", fail ? "fail" : "pass");

	close(fd_sn);

	return(fail);
}

int test_robotics_cape(const char *scan_value, unsigned id)
{
	printf("%s %d - not supported\n", scan_value, id);
	fail++;
	return(fail);
}

/* BC0A00A2yywwnnnnnnnn */
int test_servo_cape(const char *scan_value, unsigned id)
{
	int r;
	int fd_sn;
	char str[120];
	char str2[120];
	const char *sleep = "sleep 1";

	install_overlay(scan_value, capes[id].id_str);

	fd_sn = open("/sys/bus/i2c/devices/i2c-2/2-0054/2-00540/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 88);
	if(r < 0)
		printf("EEPROM read failure in test_servo_cape()\n");
	str[89] = 0;
	beagle_notice("name", &str[6]);

	/* Enable pca9685 */
	system("echo pca9685 0x70 > /sys/bus/i2c/devices/i2c-2/new_device");
	system("echo out > /sys/class/gpio/gpio68/direction");
	system("echo 0 > /sys/class/gpio/gpio68/value");
	system(sleep);

	/* Export PWMs */
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 2 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 3 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 4 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 5 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 6 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 7 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 8 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 9 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 10 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 11 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 12 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 13 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 14 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");
	system("bash -c 'echo 15 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/export'");

	/* Set periods to 100Hz (10ms) */
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:0/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:1/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:2/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:3/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:4/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:5/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:6/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:7/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:8/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:9/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:10/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:11/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:12/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:13/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:14/period'");
	system("bash -c 'echo 10000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:15/period'");

	/* Set duty cycles to 1ms */
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:0/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:1/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:2/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:3/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:4/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:5/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:6/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:7/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:8/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:9/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:10/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:11/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:12/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:13/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:14/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:15/duty_cycle'");

	/* Enable PWM outputs */
	beagle_notice("pwms", "1ms");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:0/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:1/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:2/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:3/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:4/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:5/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:6/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:7/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:8/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:9/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:10/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:11/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:12/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:13/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:14/enable'");
	system("bash -c 'echo 1 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:15/enable'");

	/* Set duty cycles to 2ms */
	beagle_notice("pwms", "2ms");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:0/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:1/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:2/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:3/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:4/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:5/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:6/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:7/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:8/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:9/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:10/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:11/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:12/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:13/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:14/duty_cycle'");
	system("bash -c 'echo 2000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:15/duty_cycle'");

	/* Set duty cycles to 1ms */
	beagle_notice("pwms", "1ms");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:0/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:1/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:2/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:3/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:4/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:5/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:6/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:7/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:8/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:9/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:10/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:11/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:12/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:13/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:14/duty_cycle'");
	system("bash -c 'echo 1000000 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:15/duty_cycle'");

	/* Enable PWM outputs */
	beagle_notice("pwms", "off");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:0/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:1/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:2/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:3/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:4/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:5/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:6/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:7/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:8/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:9/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:10/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:11/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:12/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:13/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:14/enable'");
	system("bash -c 'echo 0 > /sys/bus/i2c/drivers/pca9685-pwm/2-0070/pwm/pwmchip*/pwm*:15/enable'");

	memcpy(str, cape_eeprom, 88);
	strcpy(&str[6], capes[id].name);	/* board name */
	memcpy(&str[38], &scan_value[4], 4);	/* board version */
	strcpy(&str[58], capes[id].id_str);	/* part number */
	strncpy(&str[76], &scan_value[8], 16);	/* serial number */
	str[89] = 0;
	lseek(fd_sn, 0, SEEK_SET);
	r = write(fd_sn, str, 88);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str2, 88);
	str2[89] = 0;
	beagle_notice("name", &str2[6]);
	beagle_notice("ver/mfr", &str2[38]);
	beagle_notice("partno", &str2[58]);
	beagle_notice("serial", &str2[76]);
	fail = memcmp(str, str2, 88) ? 1 : 0;
	beagle_notice("eeprom", fail ? "fail" : "pass");

	close(fd_sn);

	return(fail);
}

/* PC0000Axyywwnnnnnnnn */
int test_gamepup_cape(const char *scan_value, unsigned id)
{
	int r;
	int fd_sn;
	char str[120];
	char str2[120];
	const char *sleep = "sleep 1";

	install_overlay(scan_value, capes[id].id_str);

	/* Enable EEPROM */
	system("echo 24c256 0x57 > /sys/bus/i2c/devices/i2c-2/new_device");
	system(sleep);

	/* Read EEPROM */
	fd_sn = open("/sys/bus/i2c/devices/i2c-2/2-0057/2-00570/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 88);
	if(r < 0)
		printf("EEPROM read failure in test_gamepup_cape()\n");
	str[89] = 0;
	beagle_notice("name", &str[6]);

	/* Light up LEDs */
	beagle_notice("leds", "on");
	set_led_trigger("gamepup::left", "default-on");
	set_led_trigger("gamepup::right", "default-on");

	/* Make tone on buzzer */
	system("echo pwm > /sys/devices/platform/ocp/ocp:P1_33_pinmux/state");
	system("bash -c 'echo 1 > /sys/devices/platform/ocp/48300000.epwmss/48300200.pwm/pwm/pwmchip*/export'");
	system(sleep);
	beagle_notice("buzzer", "tone");
	system("bash -c 'echo 1000000 > /sys/devices/platform/ocp/48300000.*/48300200.*/pwm/pwmchip*/pwm*1/period'");
	system("bash -c 'echo 500000 > /sys/devices/platform/ocp/48300000.*/48300200.*/pwm/pwmchip*/pwm*1/duty_cycle'");
	system("bash -c 'echo 1 > /sys/devices/platform/ocp/48300000.*/48300200.*/pwm/pwmchip*/pwm*1/enable'");
	system(sleep);
	system("bash -c 'echo 0 > /sys/devices/platform/ocp/48300000.*/48300200.*/pwm/pwmchip*/pwm*1/enable'");

	/* Switch buzzer to PRU enabled via buttons */
	system("echo pruout > /sys/devices/platform/ocp/ocp:P1_33_pinmux/state");
	system("echo stop > /sys/class/remoteproc/remoteproc1/state");
	system("echo gamepup-buzz-on-buttons.out > /sys/class/remoteproc/remoteproc1/firmware");
	system("echo start > /sys/class/remoteproc/remoteproc1/state");

	/* Write EEPROM */
	memcpy(str, cape_eeprom, 88);
	strcpy(&str[6], capes[id].name);	/* board name */
	memcpy(&str[38], &scan_value[4], 4);	/* board version */
	strcpy(&str[58], capes[id].id_str);	/* part number */
	strncpy(&str[76], &scan_value[8], 16);	/* serial number */
	str[89] = 0;
	lseek(fd_sn, 0, SEEK_SET);
	r = write(fd_sn, str, 88);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str2, 88);
	str2[89] = 0;
	beagle_notice("name", &str2[6]);
	beagle_notice("ver/mfr", &str2[38]);
	beagle_notice("partno", &str2[58]);
	beagle_notice("serial", &str2[76]);
	fail = memcmp(str, str2, 88) ? 1 : 0;
	beagle_notice("eeprom", fail ? "fail" : "pass");

	if(fail) {
		set_led_trigger("gamepup::left", "timer");
		set_led_trigger("gamepup::right", "timer");
	}

	/* Finish */
	close(fd_sn);
	return(fail);
}

/* PC0100Axyywwnnnnnnnn */
int test_techlab_cape(const char *scan_value, unsigned id)
{
	int r;
	int fd_sn;
	int fd_light;
	char str[120];
	char str2[120];
	char *ptr;
	const char *sleep = "sleep 1";
	int fd_accel;

	install_overlay(scan_value, capes[id].id_str);

	/* Enable EEPROM */
	system("echo 24c256 0x57 > /sys/bus/i2c/devices/i2c-2/new_device");
	system(sleep);

	/* Read EEPROM */
	fd_sn = open("/sys/bus/i2c/devices/i2c-2/2-0057/2-00570/nvmem", O_RDWR);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str, 88);
	if(r < 0)
		printf("EEPROM read failure in test_techlab_cape()\n");
	str[89] = 0;
	beagle_notice("name", &str[6]);

	beagle_notice("7seg", "on");

	/* Put buttons into GPIO mode */
	system("echo gpio > /sys/devices/platform/ocp/ocp:P2_33_pinmux/state");
	system("echo gpio > /sys/devices/platform/ocp/ocp:P1_29_pinmux/state");

	/* Tie left button to left SPI GPIO expander seven segment LED */
	set_led_trigger("techlab::seg0", "gpio");
	set_led_trigger("techlab::seg1", "gpio");
	set_led_trigger("techlab::seg2", "gpio");
	set_led_trigger("techlab::seg3", "gpio");
	set_led_trigger("techlab::seg4", "gpio");
	set_led_trigger("techlab::seg5", "gpio");
	set_led_trigger("techlab::seg6", "gpio");
	system("echo 45 > /sys/class/leds/techlab::seg0/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg0/brightness");
	system("echo 45 > /sys/class/leds/techlab::seg1/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg1/brightness");
	system("echo 45 > /sys/class/leds/techlab::seg2/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg2/brightness");
	system("echo 45 > /sys/class/leds/techlab::seg3/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg3/brightness");
	system("echo 45 > /sys/class/leds/techlab::seg4/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg4/brightness");
	system("echo 45 > /sys/class/leds/techlab::seg5/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg5/brightness");
	system("echo 45 > /sys/class/leds/techlab::seg6/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg6/brightness");

	/* Tie right button to left SPI GPIO expander seven segment LED */
	set_led_trigger("techlab::seg8", "gpio");
	set_led_trigger("techlab::seg9", "gpio");
	set_led_trigger("techlab::seg10", "gpio");
	set_led_trigger("techlab::seg11", "gpio");
	set_led_trigger("techlab::seg12", "gpio");
	set_led_trigger("techlab::seg13", "gpio");
	set_led_trigger("techlab::seg14", "gpio");
	system("echo 117 > /sys/class/leds/techlab::seg8/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg8/brightness");
	system("echo 117 > /sys/class/leds/techlab::seg9/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg9/brightness");
	system("echo 117 > /sys/class/leds/techlab::seg10/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg10/brightness");
	system("echo 117 > /sys/class/leds/techlab::seg11/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg11/brightness");
	system("echo 117 > /sys/class/leds/techlab::seg12/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg12/brightness");
	system("echo 117 > /sys/class/leds/techlab::seg13/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg13/brightness");
	system("echo 117 > /sys/class/leds/techlab::seg14/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg14/brightness");
	system("echo 117 > /sys/class/leds/techlab::seg15/gpio");
	system("echo 255 > /sys/class/leds/techlab::seg15/brightness");

	/* Make tone on buzzer */
	beagle_notice("buzzer", "tone");
	system("bash -c 'echo pruout > /sys/devices/platform/ocp/ocp:P2_30_pinmux/state'");
	system("bash -c 'echo stop > /sys/class/remoteproc/remoteproc1/state'");
	system("echo techlab-buzz.out > /sys/class/remoteproc/remoteproc1/firmware");
	system("bash -c 'echo start > /sys/class/remoteproc/remoteproc1/state'");

	/* Turn on red LED */
	beagle_notice("led", "red");
	system("echo pwm > /sys/devices/platform/ocp/ocp:P1_33_pinmux/state");
	set_led_trigger("techlab::red", "default-on");
	system(sleep);
	set_led_trigger("techlab::red", "none");

	/* Turn on green LED */
	beagle_notice("led", "green");
	set_led_trigger("techlab::green", "default-on");
	system(sleep);
	set_led_trigger("techlab::green", "none");

	/* Turn on blue LED */
	beagle_notice("led", "blue");
	set_led_trigger("techlab::blue", "default-on");
	system(sleep);
	set_led_trigger("techlab::blue", "none");

	/* Just a single light sensor reading */
	fd_light = open("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", O_RDWR);
	lseek(fd_light, 0, SEEK_SET);
	r = read(fd_light, str, 5);
	if(r < 0)
		printf("Light sensor read failure in test_techlab_cape()\n");
	ptr = strtok(str, "\n");
	beagle_notice("light", ptr);

	/* Read accelerometer */
	fd_accel = open("/sys/bus/iio/devices/iio:device1/in_accel_x_raw", O_RDWR);
	lseek(fd_accel, 0, SEEK_SET);
	r = read(fd_accel, str, 5);
	if(r < 0) {
		fail++;
		ptr = (char *)"fail";
	} else {
		ptr = strtok(str, "\n");
	}
	beagle_notice("accel", str);

	/* Write EEPROM */
	memcpy(str, cape_eeprom, 88);
	strcpy(&str[6], capes[id].name);	/* board name */
	memcpy(&str[38], &scan_value[4], 4);	/* board version */
	strcpy(&str[58], capes[id].id_str);	/* part number */
	strncpy(&str[76], &scan_value[8], 16);	/* serial number */
	str[89] = 0;
	lseek(fd_sn, 0, SEEK_SET);
	r = write(fd_sn, str, 88);
	lseek(fd_sn, 0, SEEK_SET);
	r = read(fd_sn, str2, 88);
	str2[89] = 0;
	beagle_notice("name", &str2[6]);
	beagle_notice("ver/mfr", &str2[38]);
	beagle_notice("partno", &str2[58]);
	beagle_notice("serial", &str2[76]);
	fail = memcmp(str, str2, 88) ? (fail+1) : fail;
	beagle_notice("eeprom", fail ? "fail" : "pass");

	if(fail)
		set_led_trigger("techlab::red", "timer");
	else
		set_led_trigger("techlab::green", "timer");

	/* Finish */
	close(fd_sn);
	return(fail);
}

/* PC0200Axyywwnnnnnnnn */
int test_ppilot_cape(const char *scan_value, unsigned id)
{
	printf("%s %d - not supported\n", scan_value, id);
	fail++;
	return(fail);
}

void install_overlay(const char *scan_value, const char *id_str)
{
	printf("Building overlay for %s\n", scan_value);
	/* #dtb_overlay=/lib/firmware/<file8>.dtbo */
	const char *cmd = "perl -i.bak -pe 's!^.*dtb_overlay=/lib/firmware/.+\\.dtbo.*!dtb_overlay=/lib/firmware/%s.dtbo!;' /boot/uEnv.txt";
	printf("Optional command to force overlay loading:\n");
	printf(cmd, id_str);
	printf("\n");
}

int gpio_out_test(const char *name, unsigned pin)
{
	const char *sleep = "sleep 1";
	const char *pinfile = "/sys/class/gpio/gpio%i";
	const char *export = "echo %i > /sys/class/gpio/export";
	const char *dir_out = "echo out > /sys/class/gpio/gpio%i/direction";
	const char *value_high = "echo 1 > /sys/class/gpio/gpio%i/value";
	const char *value_low = "echo 0 > /sys/class/gpio/gpio%i/value";
	char buffer[50];
	struct stat mystat;

	sprintf(buffer, pinfile, pin);
	if(stat(buffer, &mystat) != 0) {
		sprintf(buffer, export, pin);
		system(buffer);
	}
	sprintf(buffer, dir_out, pin);
	system(buffer);
	sprintf(buffer, value_high, pin);
	system(buffer);
	beagle_notice(name, "on");
	system(sleep);
	sprintf(buffer, value_low, pin);
	system(buffer);
	beagle_notice(name, "off");
	system(sleep);

	return(0);
}

/*
 * red.h
 *
 * Author: Neeraj Dantu <neeraj.dantu@octavosystems.com>
 * Copyright (C) 2018 Octavo Systems

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
#define MAP_SIZE							4096UL
#define MAP_MASK							(MAP_SIZE - 1)

unsigned long read_write_mem(int argc, unsigned int address, char type, unsigned long writeval);


#define TEMP_I2C_ADD						0x48
#define TEMP_MFG_ID 						0xfe
#define TEMP_I2C_BUS						0
#define TEMP_ID								1128


int test_tmp480();

#define TPM_I2C_ADD 						0x29
#define TPM_REV_ID							0x4c
#define TPM_I2C_BUS 						0

int test_tpm();

#define CONTROL_BASE_ADDR               	(0x44E10000)
#define BOOTMODE                        	(CONTROL_BASE_ADDR + 0x40)
#define BOOTMODE_SD							(0x00400318)

int check_boot_mode();
int test_imu();
int test_bmp();


#define NOR_READ_ID 						0x9e
struct spi_ioc_transfer xfer[2]; 

int test_nor();

/*
 * red.c
 *
 * Author: Neeraj Dantu <neeraj.dantu@octavosystems.com>
 * Copyright (C) 2018 Octavo Systems

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

int osd3358_sm_ref_design_tests(){
	
	int ret;
	//Test bootmode
	ret = check_boot_mode();
	if(ret<0) {
		fprintf(stderr, "failed: boot mode test\n");
		beagle_notice("BOOT MODE", "fail");
		return -3;
	}
	else
	{
		beagle_notice("BOOT MODE", "pass");
	}

	
	//Test IMU using driver
	ret = test_imu();
	if(ret<0) {
		fprintf(stderr, "failed: mpu9250 imu\n");
		beagle_notice("IMU", "fail");
		return -3;
	}
	else
	{
		beagle_notice("IMU", "pass");
	}

	// test barometer
	ret = test_bmp();
	if(ret<0) 
	{
		fprintf(stderr, "failed: bmp280 barometer\n");
		beagle_notice("BMP", "fail");
		return -4;
	}
	else
	{
		beagle_notice("BMP", "pass");
	}

	//test temp sensor
	ret = test_tmp480();
	if(ret<0) 
	{
		fprintf(stderr, "failed: tmp480 temp sensor\n");
		beagle_notice("TEMP", "fail");
		return -5;
	}
	else
	{
		beagle_notice("TEMP", "pass");
	}
	
	//test TPM
	ret = test_tpm();
	if(ret<0) {
		fprintf(stderr, "failed: TPM\n");
		beagle_notice("TPM", "fail");
		return -6;
	}
	else
	{
		beagle_notice("TPM", "pass");
	}
	
	
	//Test NOR Flash on SPI0
	ret = test_nor();
	if(ret<0){
		fprintf(stderr, "failed: NOR\n");
		beagle_notice("NOR", "fail");
		return -7;
	}
	else
	{
		beagle_notice("NOR", "pass");
	}
	

	return 0;
}


unsigned long read_write_mem(int argc, unsigned int address, char type, unsigned long writeval) {
    int fd;
    void *map_base, *virt_addr; 
    unsigned long read_result = 0;
    off_t target;
    int access_type = 'w';
    
    if(argc < 1) { exit(1); }

    target = address;

    if(argc > 1)
        access_type = tolower(type);


    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
    fflush(stdout);
    
    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) FATAL;
    if (0) { printf("Memory mapped at address %p.\n", map_base); }
    fflush(stdout);
    
    virt_addr = map_base + (target & MAP_MASK);
    
    if(argc <= 2) {
        switch(access_type) {
            case 'b':
                read_result = *((unsigned char *) virt_addr);
                break;
            case 'h':
                read_result = *((unsigned short *) virt_addr);
                break;
            case 'w':
                read_result = *((unsigned long *) virt_addr);
                break;
            default:
                fprintf(stderr, "Illegal data type '%c'.\n", access_type);
                exit(2);
        }


    } else {
        switch(access_type) {
            case 'b':
                *((unsigned char *) virt_addr) = writeval;
                read_result = *((unsigned char *) virt_addr);
                break;
            case 'h':
                *((unsigned short *) virt_addr) = writeval;
                read_result = *((unsigned short *) virt_addr);
                break;
            case 'w':
                *((unsigned long *) virt_addr) = writeval;
                read_result = *((unsigned long *) virt_addr);
                break;
        }

    }
    
    if(munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);
    return read_result;
}


int test_tmp480()
{
	
	uint8_t c = 0xFF;
	int file, ret, i;
	char buf[10] = "";
	uint16_t *data;
	
	file = open("/dev/i2c-0", O_RDWR);
	
	
	if(ioctl(file, I2C_SLAVE, TEMP_I2C_ADD) < 0)
	{
		printf("Address of temp sensor was not set\n");
	}

	ret = write(file, &c, 1);
	if(ret!=1)
	{
		printf("write to i2c bus with temp sensor failed\n");
		return -1;
	}
	
	ret = read(file, buf, 2);
	if(ret!=(2))
	{
		printf("i2c device returned %d bytes\n",ret);
		return -1;
	}
	
	// form words from bytes and put into user's data array
	for(i=0;i<1;i++)
	{
		data[i] = (((uint16_t)buf[0])<<8 | buf[1]); 
	}
	
	if(data[0] != TEMP_ID)
	{
		return -1;
	}
	else
	{
		return 0;
	}  
}


int test_tpm()
{
	FILE *fp;
	char buff[10];
	char string[] = "0xc6";
	
	system("touch /var/tpmtest.txt");
	system("i2cget -f -y 0 0x29 0x4c c > /var/tpmtest.txt");
	fp = fopen("/var/tpmtest.txt", "r");
	fscanf(fp, "%s", buff);
	fclose(fp);
	system("rm /var/tpmtest.txt");
	
	if(strcmp(buff,string) == 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

int check_boot_mode() {

    unsigned long temp = 0;
    temp = read_write_mem(2, BOOTMODE, 'w', 0x00000000);
    if(temp == BOOTMODE_SD)
    {
    	return 1;
    }
    else
    {
    	return -1;
    }
}

int test_imu()
{
	FILE *fp;
	char buff[10];
	char string[] = "0x71";
	
	system("touch /var/imutest.txt");
	system("i2cget -f -y 0 0x68 0x75 > /var/imutest.txt");
	fp = fopen("/var/imutest.txt", "r");
	fscanf(fp, "%s", buff);
	fclose(fp);
	system("rm /var/imutest.txt");
	
	if(strcmp(buff,string) == 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}


int test_bmp()
{
	FILE *fp;
	char buff[10];
	char string[] = "0x58";
	
	system("touch /var/bmptest.txt");
	system("i2cget -f -y 0 0x76 0xD0 > /var/bmptest.txt");
	fp = fopen("/var/bmptest.txt", "r");
	fscanf(fp, "%s", buff);
	fclose(fp);
	system("rm /var/bmptest.txt");
	
	if(strcmp(buff,string) == 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

int test_nor()
{
	/* int spi_speed_hz, slave, ret, fd, tx_bytes, i, mode_proper, bits, rx_bytes;*/
	int spi_speed_hz, ret, mode_proper, bits, rx_bytes;
	mode_proper = SPI_MODE_0;
	bits = 8;
	rx_bytes = 8;
	spi_speed_hz = 10000;
	char command[8];
	command[0] = 0x90;
	command[1] = 0x00;
	command[2] = 0x00;
	command[3] = 0x00;
	command[4] = 0x00;
	command[5] = 0x00;
	command[6] = 0x00;
	command[7] = 0x00;

	char *rx_id = malloc(8*sizeof(char));
	/* slave = 1; */
	spi_speed_hz = 10000;
	/* tx_bytes = 4; */
	
	int fdsp;
	//Setting spi mode
	
	system("config-pin P9_17 spi_cs");
	system("config-pin P9_18 spi");
	system("config-pin P9_21 spi");
	system("config-pin P9_22 spi_sclk");
	
	fdsp = open("/dev/spidev1.0", O_RDWR);
	ret = ioctl(fdsp, SPI_IOC_WR_MODE);
	if(ioctl(fdsp, SPI_IOC_WR_MODE, &mode_proper)<0){
		printf("can't set spi mode");
		close(fdsp);
		return -1;
	}
	if(ioctl(fdsp, SPI_IOC_RD_MODE, &mode_proper)<0){
		printf("can't get spi mode");
		close(fdsp);
		return -1;
	}
	if(ioctl(fdsp, SPI_IOC_WR_BITS_PER_WORD, &bits)<0){
		printf("can't set bits per word");
		close(fdsp);
		return -1;
	}
	if(ioctl(fdsp, SPI_IOC_RD_BITS_PER_WORD, &bits)<0){
		printf("can't get bits per word");
		close(fdsp);
		return -1;
	}
	if(ioctl(fdsp, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed_hz)<0){
		printf("can't set max speed hz");
		close(fdsp);
		return -1;
	}
	if(ioctl(fdsp, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed_hz)<0){
		printf("can't get max speed hz");
		close(fdsp);
		return -1;
	}
	
	//Filling in the xfer struct
	xfer[0].cs_change = 1;
	xfer[0].delay_usecs = 0;
	xfer[0].speed_hz = spi_speed_hz;
	xfer[0].bits_per_word = bits;
	xfer[0].rx_buf = 0;
	xfer[0].tx_buf = (unsigned long) command;
	xfer[0].len = 8;
	
	ret = ioctl(fdsp, SPI_IOC_MESSAGE(1), &xfer[0]);
	if(ret<0){
		printf("ERROR: SPI_IOC_MESSAGE_FAILED %x\n", ret);
	}
	
	xfer[1].cs_change = 1;
	xfer[1].delay_usecs = 0;
	xfer[1].speed_hz = spi_speed_hz;
	xfer[1].bits_per_word = bits;
	xfer[1].rx_buf = (unsigned long) rx_id;
	xfer[1].tx_buf = 0;
	xfer[1].len = rx_bytes;
	// receive
	ret=ioctl(fdsp, SPI_IOC_MESSAGE(1), &xfer[1]);
	if(ret<0){
		printf("ERROR: SPI read command failed\n");
		return -1;
	}
	
	if (*rx_id == 0x01){
		return 1;
	}
	else{
		return -1;
	}
}


