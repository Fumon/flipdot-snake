/*
 Atari SPECTRAVIDEO Controller
*/
#ifndef ATARI_SPECTRA_H
#define ATARI_SPECTRA_H

#include <libopencm3/stm32/f1/gpio.h>
#include "constants.h"

#ifdef CONTROLLER_ATARI_JOYSTICK
void controller_init();

u8 controller_state();

#endif
#endif