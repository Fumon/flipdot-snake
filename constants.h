/*
Constants and simple macros
*/
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define CL 0x8000
#define L1 0x8002
#define H1 0x8004

//PORTC
#define X_A GPIO0
#define X_B GPIO1
#define X_C GPIO2
#define X_D GPIO3
#define X_E GPIO4

#define Y_A GPIO5
#define Y_B GPIO6
#define Y_C GPIO7

#define CH_EN GPIO12
 //END PORTC

#define CONTROLLER_ATARI_JOYSTICK
#ifdef CONTROLLER_ATARI_JOYSTICK
//PORTB
#define CONTROLLER_PORT_ATARI GPIOB
#define STICK_RIGHT GPIO0
#define STICK_LEFT GPIO1
#define STICK_DOWN GPIO8
#define STICK_UP GPIO7
#define STICK_BUTTON GPIO9

 #endif

#define select(x) gpio_clear(GPIOC, x)
#define deselect(x) gpio_set(GPIOC, x)

#define xnum 28
#define ynum 16

#endif