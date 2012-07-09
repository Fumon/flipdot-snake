/*
Flips one dot
*/

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>

#include "constants.h"
#include "atari_spectra.h"

void init_clock();
void init_gpio();
void init_spi();

u16 direction, direction_buf;
u16 xpos, ypos;

void simpleflip(u8 hchip, u8 lchip, u16 x, u16 y);
void flip(u16 x, u16 y, u8 on);
void blank(u8 on);
void flipsection(u8 hchip, u8 lchip);
void line(u8 on, u16 x1, u16 y1, u16 x2, u16 y2 );
void stripes(u8);
void spiral(u8 cw);

u8 x_chips[] = {X_A, X_B, X_C, X_D, X_E};
u8 y_chips[] = {Y_A, Y_B, Y_C};

#define TAILLENGTH 24
struct point
{
	u16 x,y;
};

struct point tail[TAILLENGTH]  = { {0xFFFF, 0xFFFF} };
u8 tailind;

int main(void) {
	init_clock();
	init_gpio();
	init_spi();
	controller_init();

	// Turn the LED off.
	gpio_clear(GPIOC, GPIO9|GPIO8);

	// Raise selects
	gpio_set(GPIOC, X_A|X_B|X_C|X_D|X_E|Y_A|Y_B|Y_C);

	// Raise chip enable
	gpio_set(GPIOC, CH_EN);

	
	int k;

	xpos = 14;
	ypos = 8;

	direction = 0;
	tailind= 0;

	blank(0);

	//flip(xpos, ypos, 1);

	while(1) {
		direction_buf = controller_state();
		if(direction_buf) {
			// Prevents us from stopping
			direction = direction_buf;
		}

		if(!direction) {
			gpio_clear(GPIOC, GPIO9|GPIO8);
			// No movement
			continue;
		} else {
			if(direction & STICK_BUTTON) {
				blank(0);
				continue;
			}
			if(direction & STICK_LEFT) {
				gpio_set(GPIOC, GPIO8);
				if(xpos == 0) {
					xpos = xnum - 1;
				} else {
					xpos -= 1;
				}
			} else {
				gpio_clear(GPIOC, GPIO8);
			}

			if(direction & STICK_RIGHT) {
				gpio_set(GPIOC, GPIO9);
				if(xpos == (xnum - 1)) {
					xpos = 0;
				} else {
					xpos += 1;
				}
			} else {
				gpio_clear(GPIOC, GPIO9);
			}

			if(direction & STICK_DOWN) {
				if(ypos == 0) {
					ypos = ynum - 1;
				} else {
					ypos -= 1;
				}
			}

			if(direction & STICK_UP) {
				if(ypos == (ynum - 1)) {
					ypos = 0;
				} else {
					ypos += 1;
				}
			}
		}

		tail[tailind].x = xpos;
		tail[tailind].y = ypos;
		tailind = ((tailind + 1 ) % TAILLENGTH);
		flip(xpos, ypos, 1);
		flip(tail[tailind].x, tail[tailind].y, 0);

		for(k=0; k < 10000; k++) {
			__asm__("nop");
		}
	}
}

void flip(u16 x, u16 y, u8 on) {
	int i;
	u32 pinx, piny;
	if(x < 6) {
		pinx = X_A;
	} else if(x >= 6 && x < 12) {
		pinx = X_B;
		x -= 6;
	} else if(x >= 12 && x < 18) {
		pinx = X_C;
		x -= 12;
	} else if(x >= 18 && x < 24) {
		pinx = X_D;
		x -= 18;
	} else if(x >= 24 && x < xnum) {
		pinx = X_E;
		x -= 24;
	} else {
		return;
	}
  
	if(y < 6) {
		piny = Y_A;
	} else if(y > 5 && y < 12) {
		piny = Y_B;
		y -= 6;
	} else if(y >= 12 && y < 16) {
		piny = Y_C;
		y -= 12;
	} else {
		return;
	}
  
	u16 xpack, ypack;
	ypack = xpack = CL;
	if(on) {
		xpack |= 0x2 << (x * 2);
		ypack |= 0x4 << (y * 2);
	} 
	else {
		xpack |= 0x4 << (x * 2);
		ypack |= 0x2 << (y * 2);
	}
  
	select(pinx);
	spi_xfer(SPI1, xpack);
	deselect(pinx);
  
	select(piny);
	spi_xfer(SPI1, ypack);
	deselect(piny);

	for(i = 0; i < 2000; i++) {
		__asm__("nop");
	}

	select(pinx|piny);
	spi_xfer(SPI1, CL);
	deselect(pinx|piny);
}

void blank(u8 on) {
	u16 i, j;
	for(i = 0; i < ynum; i++) {
		for (j=0; j < xnum; j++)
		{
			flip(j, i, on);
		}
	}
}

void line(u8 on, u16 x1, u16 y1, u16 x2, u16 y2 ) {
	int xdiff, ydiff, dirx, diry;
	xdiff = (x2 - x1);
	ydiff = (y2 - y1);
	if(xdiff > 0) {
		dirx = 1;
	}else {
		dirx = -1;
		xdiff *= -1;
	}
	if(ydiff > 0) {
		diry = 1;
	} else {
		diry = -1;
		ydiff *= -1;
	}
	u16 i, j;

	i = 0;
	j = 0;
	//int k;
	while(1) {
		flip(x1 + (dirx * j), y1 + (diry * i), on);
		if(i == ydiff && j == xdiff) {
			break;
		}
		i++;
		if(i > ydiff) { i = ydiff; }
		j++;
		if(j > xdiff) { j = xdiff; }
		/*
		for(k = 0; k < 300; k++) {
			__asm__("nop");
		}
		*/
	}
}

void spiral(u8 cw) {
	//int spacebetween_ms = 0;
	//int spacebetween_us = 4000;
	int space = 0;

	blank(0);
	
	int yma, xma, ymi, xmi;
	yma = ynum - 1;
	ymi = 0;
	xma = xnum - 1;
	xmi = 0;
	
	if(space == 0) {
		space = 2;
	}
	
	// ClockWise
	// 0 - LL UL - ymi,xmi -> yma,xmi (xmi++)
	// 1 - UL UR - yma,xmi -> yma,xma (yma--)
	// 2 - UR LR - yma,xma -> ymi,xma (xma--)
	// 3 - LR LL - ymi,xma -> ymi,xmi (ymi++)
	// Anticlockwise
	// 0 - LL LR - ymi,xmi -> ymi,xma (ymi++)
	// 1 - LR UR - ymi,xma -> yma,xma (xma--)
	// 2 - UR UL - yma,xma -> yma,xmi (yma--)
	// 3 - UL LL - yma,xmi -> ymi,xmi (xmi++)
	int stage = 0; 
	
	while(yma - ymi >= 0 && xma - xmi >= 0) {
		if(cw) {
			switch(stage % 4) {
				case 0:
				line(1,xmi,ymi,xmi,yma);
				xmi++;
				ymi += (space - 1);
				break;
				case 1:
				line(1,xmi,yma,xma,yma);
				yma--;
				xmi += (space - 1);
				break;
				case 2:
				line(1,xma,yma,xma,ymi);
				xma--;
				yma -= (space - 1);
				break;
				case 3:
				line(1,xma,ymi,xmi,ymi);
				ymi++;
				xma -= (space -1);
				break;
			}
		} else {
			switch(stage % 4) {
				case 0:
				line(1,xmi,ymi,xma,ymi);
				ymi++;
				xmi += (space - 1);

				break;
				case 1:
				line(1,xma,ymi,xma,yma);
				xma--;
				ymi += (space - 1);
				break;
				case 2:
				line(1,xma,yma,xmi,yma);
				yma--;
				xma -= (space - 1);
				break;
				case 3:
				line(1,xmi,yma,xmi,ymi);
				xmi++;
				yma -= (space - 1);
				break;
			}
		}
		stage++;
	}
	
}

void stripes(u8 on) {
	int k;
	int space = 0;
	space = 2;
	int i;
	u16 y,x;
	for(y = 0; y < ynum; y++) {
		i = (y%space);
		for(x = 0; x < xnum; x++) {
			if(!(i%space)) {
				flip(x, y, on);
			} else {
				flip(x, y, !on);
			}
			for(k = 0; k < 50; k++) {
				__asm__("nop");
			}
			i++;
		}
	}
}

void simpleflip(u8 hchip, u8 lchip, u16 hind, u16 lind) {
	int i;
	gpio_clear(GPIOB, hchip);
	spi_xfer(SPI1, (0x0004 << (hind * 2)) | CL);
	gpio_set(GPIOB, hchip);
	gpio_clear(GPIOB, lchip);
	spi_xfer(SPI1, (0x0002 << (lind * 2)) | CL);
	gpio_set(GPIOB, lchip);
	for(i = 0; i < 3000; i++) {
		__asm__("nop");
	}
	gpio_clear(GPIOB, hchip | lchip );
	spi_xfer(SPI1, CL);
	gpio_set(GPIOB, hchip | lchip);
}

void flipsection(u8 hchip, u8 lchip) {
	u16 i, j;
	int k;
	for(i = 0; i < 6; i++) {
		for(j = 0; j < 6; j++) {
			simpleflip(hchip, lchip, j, i);
			for(k = 0; k < 50; k++) {
				__asm__("nop");
			}
		}
	}
}

void init_clock() {
	rcc_clock_setup_in_hse_8mhz_out_24mhz();

	//LED GPIO clock
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);

	// Chip select clock
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);

	//Button Clock
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);

	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_SPI1EN);
}

void init_gpio() {
	// LED pushpull on C9
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO9);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
			  GPIO_CNF_OUTPUT_PUSHPULL, GPIO8);


	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO0);


	// SPI GPIO setup
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_SCK);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_MOSI);
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
		GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_SPI1_MISO);

	// Slave Selects
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO0 | GPIO1 | GPIO2 |GPIO3|GPIO4|GPIO5|GPIO6|GPIO7);

	// Chip Enable
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, CH_EN);

#ifdef CONTROLLER_ATARI_JOYSTICK
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
		STICK_LEFT | STICK_RIGHT | STICK_DOWN | STICK_UP | STICK_BUTTON);
#endif
}

void init_spi() {
	spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_32, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
		SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_16BIT,
		SPI_CR1_LSBFIRST);
	spi_enable_software_slave_management(SPI1);
	spi_disable_ss_output(SPI1);
	spi_set_nss_high(SPI1);
	//spi_set_bidirectional_transmit_only_mode(SPI1);
	spi_enable(SPI1);
}