/**
 *  main.h - main declarations
 *  Copyright (C) 2016  Simon Kaufmann, HeKa
 *
 *  This file is part of VUSB flash drive
 *
 *  VUSB flash drive is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  VUSB flash drive is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with VUSB flash drive.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAIN_H
#define MAIN_H

#define TRUE	1
#define FALSE	0

extern const uint8_t cbw_signature[4];
extern const uint8_t csw_signature[4];
extern uint8_t is_writing;
extern uint8_t is_writing_beginning;

void set_writing(uint16_t writing_int, uint16_t transfer_length);
void set_writing_beginning(void);

#define BLOCK_SIZE	512
#define BLOCKS		128
//#define PAGES		(BLOCKS * BLOCK_SIZE / PAGE_SIZE)
#define PAGES 		512
#define PAGE_SIZE	128

#define LED_PORT	PORTD
#define LED_DDR 	DDRD
#define LED			PD1
#define LED_TOGGLE()	LED_PORT ^= (1 << LED)
#define LED_ON()		LED_PORT |= (1 << LED)
#define LED_OFF()		LED_PORT &= ~(1 << LED)


#endif
