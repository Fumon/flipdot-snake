/*
 Constants and simple macros
*/
#ifndef CONSTANTS_H
#define CONSTANTS_H

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

#define CH_EN GPIO12

#define select(x) gpio_clear(GPIOC, x)
#define deselect(x) gpio_set(GPIOC, x)

#define xnum 28
#define ynum 16

#endif