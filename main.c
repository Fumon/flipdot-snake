/*
Flips one dot
*/

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/systick.h>

#include "simplerand.h"
//#include <stdlib.h>

#include "constants.h"
#include "atari_spectra.h"
#include "ringlib.h"
#include "fumem.h"

struct point
{
	u16 x,y;
};

void init_clock();
void init_gpio();
void init_spi();
void systick_init();

u16 direction, direction_buf;
u16 xpos, ypos;

void flip(u16 x, u16 y, u8 on);
void blank(u8 on);
void line(u8 on, u16 x1, u16 y1, u16 x2, u16 y2 );
void stripes(u8);
void spiral(u8 cw);

int check_point(ring* r, struct point* testpoint);

struct point snakemem[(xnum) * (ynum)];

ring snakebase = {
	.memstore = snakemem,
	.nelem = (xnum * ynum),
	.head = 0,
	.tail = 0,
	.unitsize = sizeof(struct point),
	.current_element_count = 0,
};

struct point food;

int main(void) {
	ring *snake = &snakebase;
	struct point* poppoint = NULL;

	init_clock();
	init_gpio();
	init_spi();
	controller_init();
	systick_init();

	struct point staging_point;
	// Turn the LED off.
	gpio_clear(GPIOC, GPIO9|GPIO8);

	// Raise selects
	gpio_set(GPIOC, X_A|X_B|X_C|X_D|X_E|Y_A|Y_B|Y_C);

	// Raise chip enable
	gpio_set(GPIOC, CH_EN);

	RESET:

	fumemclear(snakemem, (sizeof(struct point) * xnum * ynum));
	snake->head = 0;
	snake->tail = 0;
	snake->current_element_count = 0;

	food.x = 99;
	food.y = 99;

	blank(1);

	while(controller_state() != STICK_BUTTON) {
		__asm__("nop");
	}

	srsrand(systick_get_value());

	
	xpos = 14;
	ypos = 8;

	staging_point.x = xpos;
	staging_point.y = ypos;

	push(snake, &staging_point);
	flip(xpos, ypos, 1);

	direction = 0;

	blank(0);

	u32 cnt = 0;

	while(1) {
		// Generate food check
		if(food.x == 99) {
			do {
				food.x = srrand() % xnum;
				food.y = srrand() % ynum;
			} while(check_point(snake, &food));
			flip(food.x, food.y, 1);
		}

		direction_buf = controller_state();
		if(direction_buf == STICK_BUTTON) {
			goto RESET;
		} else if(direction_buf && !detect_opposite(direction, direction_buf)) {
			// Prevents us from stopping
			direction = direction_buf;
		}
		cnt++;

		if(cnt < 88000) {
			continue;
		} else {
			cnt = 0;
		}

		if(!direction) {
			srsrand(systick_get_value());
			// No movement
			continue;
		} else {
			if(direction & STICK_BUTTON) {
				blank(0);
				continue;
			}
			if(direction & STICK_LEFT) {
				if(xpos == 0) {
					xpos = xnum - 1;
				} else {
					xpos -= 1;
				}
			} else if(direction & STICK_RIGHT) {
				if(xpos == (xnum - 1)) {
					xpos = 0;
				} else {
					xpos += 1;
				}
			} else if(direction & STICK_DOWN) {
				if(ypos == 0) {
					ypos = ynum - 1;
				} else {
					ypos -= 1;
				}
			} else if(direction & STICK_UP) {
				if(ypos == (ynum - 1)) {
					ypos = 0;
				} else {
					ypos += 1;
				}
			}
		}

		// Staging point
		staging_point.x = xpos;
		staging_point.y = ypos;

		if(check_point(snake, &staging_point)) {
			// GAME OVER
			goto RESET;
		}

		// Check Food collision
		if (staging_point.x == food.x && staging_point.y == food.y) {
			food.x = 99;
			food.y = 99;
		} else {
			pop(snake, (void**)&poppoint);
			flip(poppoint->x, poppoint->y, 0);
		}

		push(snake, &staging_point);
		flip(xpos, ypos, 1);
	}
}

int check_point(ring* r, struct point* testpoint) {
	struct point* tmppoint = NULL;
	// Check for tail collision
	int i;
	for (i = 0; i < r->current_element_count; ++i) {
		get(r, i, (void**)&tmppoint);
		if(testpoint->x == tmppoint->x 
			&& testpoint->y == tmppoint->y) {
			return 1;
		}
	}
	return 0;
}

void flip(u16 x, u16 y, u8 on) {
	int i;
	u16 pinx, piny;
	pinx = x_chips[x_map[x]];
	piny = y_chips[y_map[y]];

	x += x_diff[x];
	y += y_diff[y];
  
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
		int k;
		
		for(k = 0; k < 30000; k++) {
			__asm__("nop");
		}
		
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
		space = 1;
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
		SPI_CR1_CPHA, SPI_CR1_DFF_16BIT,
		SPI_CR1_LSBFIRST);
	spi_enable_software_slave_management(SPI1);
	spi_disable_ss_output(SPI1);
	spi_set_nss_high(SPI1);
	//spi_set_bidirectional_transmit_only_mode(SPI1);
	spi_enable(SPI1);
}

void systick_init() {
	systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB_DIV8);
	systick_set_reload(9000);
	systick_counter_enable();
}
