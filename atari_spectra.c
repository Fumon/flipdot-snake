#include "atari_spectra.h"

void controller_init() {
	// Set all inputs to pull down
	gpio_clear(CONTROLLER_PORT_ATARI, 
		STICK_RIGHT | STICK_LEFT | STICK_DOWN | STICK_UP | STICK_BUTTON);
}

u16 controller_state() {
	//u16 tmp = gpio_get(CONTROLLER_PORT_ATARI,
	//	STICK_LEFT | STICK_RIGHT | STICK_DOWN | STICK_UP);
	// Gather
	//tmp = (tmp & (MASK_LEFT | MASK_RIGHT)) | 
	//	(tmp >> 3);

	u16 tmp = (GPIOB_IDR & (STICK_LEFT | STICK_RIGHT | STICK_DOWN | STICK_UP | STICK_BUTTON));

	return tmp;
}