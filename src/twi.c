/*
 *  twi.c - functions for TWI (I2C) - operations
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

#include <avr/io.h>
#include <util/twi.h>

#include "twi.h"

uint8_t twcr_init = 0; /* the initial set bits in the TWCR */

/*
 * public functions
 */

/**
 * twi_init - initialize twi interface
 */
void twi_init()
{
	TWBR = ((F_CPU / 400000) - 16) / 2; /* set the clock rate of TWI to 400 kHz */
	TWSR = 0x00; /* set prescaler TWI */
	twcr_init = (1 << TWEN);
}

/**
 * twi_start - send a start condition or a repeated start condition
 *
 * 		Repeated start condition is transmitted if there was a start
 * 		condition before with no stop condition afterwards
 *
 * 		Start condition is transmitted if the last operation was a
 * 		stop condition
 *
 * 		Returns TWI_OK on success, TWI_ERR on failure
 */
uint8_t twi_start()
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | twcr_init;
	while (!(TWCR & (1 << TWINT)));
	if ((TWSR & 0xF8) != TW_START && (TWSR & 0xF8) != TW_REP_START)	{ 
		return TWI_ERR;
	}
	return TWI_OK;
}

/**
 * twi_send_slave_address - send the given slave address either for reading or 
 * 							writing from twi
 * @rw:			either TWI_READ or TWI_WRITE
 * @address:	the 7 bit slave address (from bit 7 to bit 1), bit zero unused
 *
 * 		Returns TWI_OK on success, TWI_ERR on failure
 */
uint8_t twi_send_slave_address(uint8_t rw, uint8_t address)
{
	if (rw == TWI_READ)	{
		TWDR = address | (1 << 0);
	} else if (rw == TWI_WRITE)	{
		TWDR = address & (~(1 << 0));
	} else	{
		return TWI_ERR;
	}
	TWCR = (1 << TWINT) | twcr_init;
	while (!(TWCR & (1 << TWINT)));
	if (rw == TWI_WRITE)	{
		if ((TWSR & 0xF8) != TW_MT_SLA_ACK)	{
			return TWI_ERR;
		}
	} else {
		if ((TWSR & 0xF8) != TW_MR_SLA_ACK)	{
			return TWI_ERR;
		}
	}
	return TWI_OK;
}

/**
 * twi_send_byte - send a byte via twi interface
 * @byte:		byte to send
 *
 * 		Execute a twi_start() and twi_send_slave_address() with TWI_WRITE
 * 		before using this function
 *
 * 		Returns TWI_OK on success, TWI_ERR on failure
 */
uint8_t twi_send_byte(uint8_t byte)
{
	TWDR = byte; 
	TWCR = (1 << TWINT) | twcr_init;
	while (!(TWCR & (1 << TWINT)));
	if ((TWSR & 0xF8) != TW_MT_DATA_ACK)	{
		return TWI_ERR;
	}
	return TWI_OK;
}

/**
 * twi_read_byte - read a bit from twi interface
 * @ack:		Either TWI_ACK to send an acknowledge or TWI_NACK to
 * 				read byte without sending acknowledge afterwards
 * @byte:		pointer to a variable where the read byte gets stored
 *
 * 		Execute twi_start() and twi_send_slave_address() with TWI_READ
 * 		before using this function
 *
 * 		Returns TWI_OK on success, TWI_ERR on failure
 */
uint8_t twi_read_byte(uint8_t ack, uint8_t* byte)
{
	if (ack == TWI_ACK)	{
		TWCR = (1 << TWINT) | (1 << TWEA) | twcr_init;
	} else if (ack == TWI_NACK) {
		TWCR = (1 << TWINT) | twcr_init;
	} else	{
		return TWI_ERR;
	}
	while (!(TWCR & (1 << TWINT)));
	if (ack == TWI_ACK)	{
		if ((TWSR & 0xF8) != TW_MR_DATA_ACK)	{
			return TWI_ERR;
		}
	} else	{
		if ((TWSR & 0xF8) != TW_MR_DATA_NACK)	{
			return TWI_ERR;
		}
	}
	*byte = TWDR;
	return TWI_OK;
}

/**
 * twi_stop - send a stop condition
 */
void twi_stop()
{
		TWCR = (1<<TWINT) | (1<<TWSTO) | twcr_init;
		while (TWCR & (1<<TWSTO));
}

