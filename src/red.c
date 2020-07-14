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



