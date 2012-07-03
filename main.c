/*
Flips one dot
*/

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>

#define CL 0x8000
#define L1 0x8002
#define H1 0x8004


#define X_A GPIO0
#define X_B GPIO1
#define X_C GPIO2
#define X_D GPIO3
#define X_E GPIO4

#define Y_A GPIO5
#define Y_B GPIO6
#define Y_C GPIO7

#define select(x) gpio_clear(GPIOC, x)
#define deselect(x) gpio_set(GPIOC, x)

#define xnum 28
#define ynum 16


void init_clock();
void init_gpio();
void init_spi();

u8 buttonstate;
u8 buttonon;
u8 flippedoff;

void simpleflip(u8 hchip, u8 lchip, u16 x, u16 y);
void flip(u16 x, u16 y, u8 on);
void flipsection(u8 hchip, u8 lchip);

u8 x_chips[] = {X_A, X_B, X_C, X_D, X_E};
u8 y_chips[] = {Y_A, Y_B, Y_C};

int main(void) {
	init_clock();
	init_gpio();
	init_spi();

	// Turn the LED off.
	gpio_clear(GPIOC, GPIO9|GPIO8);

	// Raise selects
	gpio_set(GPIOC, X_A|X_B|X_C|X_D|X_E|Y_A|Y_B|Y_C);

	u16 i, j;
	int k;

	buttonstate = 0;
	buttonon = 0;
	flippedoff = 0;
	
	while(1) {
		buttonstate = ((GPIOA_IDR & 0x0001) != 0);
		if(!buttonstate) {
			if(buttonon) {
				buttonon = 0;
			}
			continue;
		} else {
			if(buttonon) {
				continue;
			} else {
				buttonon = 1;
			}
		}

		// Else
		for(i = 0; i < ynum; i++) {
			for (j=0; j < xnum; j++)
			{
				flip(j, i, flippedoff);
			}
		}
		

		flippedoff = !flippedoff;
		if(flippedoff) {
			//LED on
			gpio_set(GPIOC, GPIO9);
		} else {
			gpio_clear(GPIOC, GPIO9);
		}

		for(k = 0; k < 80000; k++) {
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

	for(i = 0; i < 3000; i++) {
		__asm__("nop");
	}

	select(pinx|piny);
	spi_xfer(SPI1, CL);
	deselect(pinx|piny);
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