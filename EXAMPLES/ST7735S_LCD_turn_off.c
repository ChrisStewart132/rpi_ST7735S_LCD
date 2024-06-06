/**
 * Author: Christopher Stewart (Christopher.ray.stewart@gmail.com)
 * Date: 06062024
 * Description: program to turn off a ST7735S 1.8" 128x160 LCD
 * 
 * gcc -o ST7735S_LCD_turn_off ST7735S_LCD_turn_off.c -lgpiod
 * ./ST7735S_LCD_turn_off
 */

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define GPIO_CHIP_NAME "gpiochip0"
#define GPIO_OFFSET_DC 26 	// DATA(high)/COMMAND(low) aka WRX
#define GPIO_OFFSET_RST 16 	// RESET active low

/**
 * Note to use SPI1 on rpi you must edit boot/config.txt and add (then reboot): dtoverlay=spi1-2cs
 * this makes GPIO 16,17 chip selects/enables (CE0, CE1) for SPI1
*/
#define SPI_DEVICE "/dev/spidev1.0"
#define SPI_HZ 4000000
/**
 * SCE
 * MOSI
 * CLK
 */

/**
 * commands
*/
#define NOP 0x00
#define SWRESET 0x01	// loads default settings
#define SLPOUT 0x11		// Sleep Out & Booster On
#define NORON 0x13		// normal display mode on
#define DISPON 0x29		// default display is off...

#define GAMSET 0x26		
#define COLMOD 0x3a

#define CASET 0x2a	// cursor xs-xe
#define RASET 0x2b	// cursor ys-ye
#define RAMWR 0x2c	// continuously write to fill xs,ys - xe,ye


#define INVON 0x20
#define INVOFF 0x21

// rgb order?
#define RGBSET 0x2d	
// ??
#define SCRLAR 0x33
#define MADCTL 0x36
#define VSCSAD 0x37

         
void _spi_transfer(int fd, uint8_t* tx_buffer, uint8_t* rx_buffer, int size) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = size,
        .speed_hz = SPI_HZ,
        .delay_usecs = 0,
        .bits_per_word = 8,
        .cs_change = 0,
    };
    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("Failed to send SPI message");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void _gpio_high(struct gpiod_line* line) {
    gpiod_line_set_value(line, 1);
}

void _gpio_low(struct gpiod_line* line) {
    gpiod_line_set_value(line, 0);
}

void init_gpio(struct gpiod_chip** chip, struct gpiod_line** dc, struct gpiod_line** rst) {
    *chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
    if (!*chip) {
        perror("gpiod_chip_open_by_name");
        exit(EXIT_FAILURE);
    }

    *rst = gpiod_chip_get_line(*chip, GPIO_OFFSET_RST);
    if (!*rst) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(*chip);
        exit(EXIT_FAILURE);
    }
    if (gpiod_line_request_output(*rst, "rst", 0) < 0) {
        perror("gpiod_line_request_output");
        gpiod_chip_close(*chip);
        exit(EXIT_FAILURE);
    }

	*dc = gpiod_chip_get_line(*chip, GPIO_OFFSET_DC);
    if (!*dc) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(*chip);
        exit(EXIT_FAILURE);
    }
    if (gpiod_line_request_output(*dc, "dc", 0) < 0) {
        perror("gpiod_line_request_output");
        gpiod_chip_close(*chip);
        exit(EXIT_FAILURE);
    }
}

void init_spi(int* fd) {
    *fd = open(SPI_DEVICE, O_RDWR);
    if (*fd < 0) {
        perror("Failed to open SPI device");
        exit(EXIT_FAILURE);
    }
}

void command(int fd, struct gpiod_line* dc, uint8_t cmd){
	uint8_t rx;
	_gpio_low(dc);
    _spi_transfer(fd, &cmd, &rx, 1);
    usleep(120000); // 120ms delay
}

void init(int fd, struct gpiod_line* dc, struct gpiod_line* rst){
	/**
	 * 9.14 Power Level Definition
		* 6. Power Off Mode In this mode, both VDD and VDDI are removed.
		* 5. Sleep In Mode In this mode, the DC: DC converter, internal oscillator and panel driver circuit are stopped. Only the MCU interface and memory works with VDDI power supply. Contents of the memory are safe.
		* 1. Normal Mode On (full display), Idle Mode Off, Sleep Out. In this mode, the display is able to show maximum 262,144 colors.
	 * 
	 * 
	 * 
	 * Power on (HW reset, SW reset): Sleep in, Normal display mode on, idle mode off
	 * 
	 * CMD SLPOUT: Sleep out, Normal display mode on, idle mode off (maximum power usage, full screen, and full color mode)
	 * 
	 * 
	*/
	uint8_t tx_buffer[33] = {NOP};
	uint8_t rx_buffer[33] = {0};
	
	// Hardware reset
    _gpio_low(rst);
    usleep(100000); // 100ms delay
    _gpio_high(rst);
    usleep(100000); // 100ms delay

	// Software reset
	command(fd, dc, SWRESET);

	// off
	_gpio_low(rst);
}

int main() {
	struct gpiod_chip* chip;
	struct gpiod_line* dc;
	struct gpiod_line* rst;
	
	init_gpio(&chip, &dc, &rst);
	
	int fd;// SPI file
	init_spi(&fd);

	init(fd, dc, rst);
	

	if(dc)
		gpiod_line_release(dc);
	if(rst)
		gpiod_line_release(rst);
	if(chip)
		gpiod_chip_close(chip);

	close(fd);
	return EXIT_SUCCESS;
}