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


