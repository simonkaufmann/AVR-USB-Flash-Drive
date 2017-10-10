/**
 *  ram.c - library functions for accessing ram 6c4008
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

#include <avr/io.h>

#include "ram.h"


void ram_init(void)
{
	ADDR0_DDR = 0xff;
	ADDR1_DDR |= 0b11111100;
	ADDR2_DDR |= 0b11000000;

	CHIP_PORT |= (1 << CE) | (1 << WE) | (1 << OE);
	CHIP_DDR |= (1 << CE) | (1 << WE) | (1 << OE);
}

void ram_write_byte(uint16_t addr, uint8_t data)
{
	ADDR0_PORT = addr & 0xff;
	ADDR1_PORT &= 0b00000011;
	ADDR1_PORT |= (addr >> 6) & 0b11111100;
	ADDR2_PORT &= 0b00111111;
	ADDR2_PORT |= (addr >> 8) & 0b11000000;

	ADDR0_PORT = addr & 0xff;
	ADDR1_PORT &= 0b00000011;
	ADDR1_PORT |= (addr >> 6) & 0b11111100;
	ADDR2_PORT &= 0b00111111;
	ADDR2_PORT |= (addr >> 8) & 0b11000000;

	DATA_DDR = 0xff;
	DATA_PORT = data;

	CHIP_PORT &= ~(1 << CE);
	CHIP_PORT &= ~(1 << WE);

	DATA_DDR = 0xff;
	DATA_PORT = data;


	CHIP_PORT |= (1 << WE);
	CHIP_PORT |= (1 << CE);

	DATA_DDR = 0x00;

}

uint8_t ram_read_byte(uint16_t addr)
{
	DATA_DDR = 0x00;

	ADDR0_PORT = addr & 0xff;
	ADDR1_PORT &= 0b00000011;
	ADDR1_PORT |= (addr >> 6) & 0b11111100;
	ADDR2_PORT &= 0b00111111;
	ADDR2_PORT |= (addr >> 8) & 0b11000000;

	ADDR0_PORT = addr & 0xff;
	ADDR1_PORT &= 0b00000011;
	ADDR1_PORT |= (addr >> 6) & 0b11111100;
	ADDR2_PORT &= 0b00111111;
	ADDR2_PORT |= (addr >> 8) & 0b11000000;

	CHIP_PORT &= ~(1 << CE);
	CHIP_PORT &= ~(1 << OE);

	uint8_t temp = DATA_PIN;
	temp = DATA_PIN;
	temp = DATA_PIN;

	CHIP_PORT |= (1 << OE);
	CHIP_PORT |= (1 << CE);

	return temp;
}
