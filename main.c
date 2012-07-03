/*
	Flips one dot
*/

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>

#define CL 0x8000
#define L1 0x8002
#define H1 0x8004

void init_clock();
void init_gpio();
void init_spi();

u8 buttonstate;
u8 buttonon;
u8 flippedoff;

int main(void) {
	init_clock();
	init_gpio();
	init_spi();

	// Turn the LED off.
	gpio_clear(GPIOC, GPIO9|GPIO8);

	// Raise selects
	gpio_set(GPIOB, GPIO0|GPIO1);

	int i;

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

		// Raise Y_B
		gpio_clear(GPIOB, GPIO0);
		if(!flippedoff){
			spi_xfer(SPI1, L1);
		} else {
			spi_xfer(SPI1, H1);
		}
		gpio_set(GPIOB, GPIO0);
		//Raise X_B
		gpio_clear(GPIOB, GPIO1);
		 if(flippedoff){
			spi_xfer(SPI1, L1);
		} else {
			spi_xfer(SPI1, H1);
		}
		gpio_set(GPIOB, GPIO1);

		gpio_set(GPIOC, GPIO8);

		//Wait for a brief time
		for(i = 0; i < 500000; i++) {
			__asm__("nop");
		}

		gpio_clear(GPIOC, GPIO8);

		// Raise Y_B
		gpio_clear(GPIOB, GPIO0);
		 // clear
		spi_xfer(SPI1, CL);
		gpio_set(GPIOB, GPIO0);
		//Raise X_B
		gpio_clear(GPIOB, GPIO1);
		 // Make Low 1
		spi_xfer(SPI1, CL);
		gpio_set(GPIOB, GPIO1);

		flippedoff = !flippedoff;
		if(flippedoff) {
			//LED on
			gpio_set(GPIOC, GPIO9);
		} else {
			gpio_clear(GPIOC, GPIO9);
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
	// Y_B
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO0);
	// X_A
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
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