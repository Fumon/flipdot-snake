#include "atari_spectra.h"

void controller_init() {
	// Set all inputs to pull down
	gpio_clear(CONTROLLER_PORT_ATARI, 
		STICK_RIGHT | STICK_LEFT | STICK_DOWN | STICK_UP | STICK_BUTTON);
}

u8 controller_state() {
	u16 tmp = gpio_get(CONTROLLER_PORT_ATARI,
		STICK_LEFT | STICK_RIGHT | STICK_DOWN | STICK_UP | STICK_BUTTON);
	// Gather
	tmp = (tmp & (STICK_LEFT | STICK_RIGHT)) | 
		(tmp >> 3);

	return (u8)tmp;
}