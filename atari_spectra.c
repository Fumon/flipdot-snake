#include "atari_spectra.h"

void controller_init() {
	// Set all inputs to pull down
	gpio_set(CONTROLLER_PORT_ATARI, 
		STICK_RIGHT | STICK_LEFT | STICK_DOWN | STICK_UP | STICK_BUTTON);
}

char detect_opposite(u16 cur, u16 new) {
	switch(cur) {
		case STICK_LEFT:
			if(new & STICK_LEFT_OP) return 1;
			break;
		case STICK_RIGHT:
			if(new & STICK_RIGHT_OP) return 1;
			break;
		case STICK_UP:
			if(new & STICK_UP_OP) return 1;
			break;
		case STICK_DOWN:
			if(new & STICK_DOWN_OP) return 1;
			break;
		default:
			return 0;
	}
	return 0;
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