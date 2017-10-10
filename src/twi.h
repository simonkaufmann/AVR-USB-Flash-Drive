/*
 *  twi.h - header file defining functions for TWI (I2C) - operations
 *  Copyright (C) 2016  Simon Kaufmann, HeKa
 *
 *  This file is part of VUSB flash drive
 *
 *  VUSB flash drive is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  VUSB flash drive is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with VUSB flash drive.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TWI_H
#define TWI_H

#define TWI_OK		0
#define TWI_ERR		1

#define TWI_READ	0
#define TWI_WRITE	1

#define TWI_ACK		0
#define TWI_NACK	1

void twi_init(void);

uint8_t twi_start(void); /* for start or repeated start condition */
void twi_stop(void);

uint8_t twi_send_slave_address(uint8_t rw, uint8_t address);
uint8_t twi_send_byte(uint8_t byte);
uint8_t twi_read_byte(uint8_t ack, uint8_t* byte);


#endif
