/**
 *  ram.h - library functions for accessing ram 6c4008
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


#ifndef RAM_H
#define RAM_H

void ram_init(void);

void ram_write_byte(uint16_t addr, uint8_t data);

uint8_t ram_read_byte(uint16_t addr);



#define DATA_PORT 	PORTA
#define DATA_DDR	DDRA
#define DATA_PIN	PINA

#define ADDR0_PORT	PORTB
#define ADDR0_DDR	DDRB

#define ADDR1_PORT	PORTC /* a8 - a13 are on portc2 - portc7 */
#define ADDR1_DDR 	DDRC

#define ADDR2_PORT	PORTD /* a14 - a15 are on portd6 - portd7 */
#define ADDR2_DDR	DDRD

#define CHIP_PORT	PORTD
#define CHIP_DDR	DDRD
#define WE			PD0
#define CE			PD3
#define OE			PD5

#endif
