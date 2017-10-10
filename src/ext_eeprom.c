/**
 *  ext_eeprom.c - library for eeprom chip 24AA512
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
#include <util/delay.h>

#include "main.h"
#include "twi.h"
#include "ext_eeprom.h"

#define EEPROM_ADDRESS		0b10100000
#define EEPROM_ADDRESS_W	0b10100000
#define EEPROM_ADDRESS_R	0b10100001

#define RETURN_IF_ERROR(ret)		if (ret != TWI_OK) { return ret; }
#define RETURN_MACRO(ret)			return ret;

#define EEPROM_PAGE_SIZE	128

/*
 * internal functions
 */

/**
 * ext_eeprom_wait - wait until eeprom has finished its write process
 *
 * 		Returns TWI_OK if writing ist finished, TWI_ERR if writing is not finished
 * 		after 5ms or if eeprom is not responding at all
 */
static uint8_t ext_eeprom_wait(void)
{
	uint8_t ret = TWI_OK, count = 0;
	while (ext_eeprom_is_ready() != TWI_OK)	{
		if (count >= 10)	{
			ret = TWI_ERR;
			break;
		}
		_delay_us(500);
		count++;
	}
	return ret;
}

/**
 * address_write_prefix - execute the eeprom addressing
 * addr: address for the byte/s to write
 *
 * 		Returns TWI_OK on success, TWI_ERR on failure
 */
static uint8_t address_write_prefix(uint16_t addr)
{
	uint8_t ret;
	ret = twi_start();
	RETURN_IF_ERROR(ret);
	ret = twi_send_slave_address(TWI_WRITE, EEPROM_ADDRESS_W);	
	RETURN_IF_ERROR(ret);
	ret = twi_send_byte(addr >> 8);
	RETURN_IF_ERROR(ret);
	ret = twi_send_byte(addr & 0xFF);
	RETURN_IF_ERROR(ret);
	RETURN_MACRO(ret);
}

/**
 * address_read_prefix - execute eeprom addressing for reading
 * addr: address for the byte/s to read
 *
 * 		Returns TWI_OK on success, TWI_ERR on failure
 */
static uint8_t address_read_prefix(uint16_t addr)
{
	uint8_t ret;
	ret = address_write_prefix(addr);
	RETURN_IF_ERROR(ret);
	ret = twi_start();
	RETURN_IF_ERROR(ret);
	ret = twi_send_slave_address(TWI_READ, EEPROM_ADDRESS_R);
	RETURN_IF_ERROR(ret);
	RETURN_MACRO(ret);
}

/**
 * ext_eeprom_write_page - write maximum of one page to eeprom
 * @data:	pointer to data buffer with data to be written
 * @addr: 	address of first byte written in eeprom 
 * @number:	number of bytes to be written
 *
 * 		The addresses addr to (addr + number - 1) should be inside
 * 		a physical page of the eeprom. Otherwise beginning of the page
 * 		will be overwritten!
 * 		
 * 		Returns TWI_OK on success, TWI_ERR on failure
 */
static inline uint8_t ext_eeprom_write_page(const uint8_t *data, uint16_t addr, uint8_t number)
{
	if (ext_eeprom_wait() != TWI_OK)	{
		return TWI_ERR;
	}

	//rtc_disable_avr_interrupt();
	uint8_t ret;
	ret = address_write_prefix(addr);
	RETURN_IF_ERROR(ret);
	uint8_t i;
	for (i = 0; i < number; i++)	{
		ret = twi_send_byte(data[i]);
		RETURN_IF_ERROR(ret);
	}
	twi_stop();
	RETURN_MACRO(ret);
}

/*
 * public functions
 */

/**
 * ext_eeprom_write_block - write n bytes to eeprom
 * @data:	pointer to data buffer with data to be written
 * @addr: 	address of first byte written in eeprom 
 * @number:	number of bytes to be written
 *
 * 		If number + addr is bigger than 64kByte ->
 * 		functions starts to write at beginning of eeprom
 *
 * 		Returns TWI_OK on success and TWI_ERR on failure
 */
uint8_t ext_eeprom_write_block(const uint8_t *data, uint16_t addr, uint16_t number)
{

	/*
	 * Note:
	 * eeprom can only write bytes inside one physical page at once. Therefore
	 * this function splits the write progress if necessary into different
	 * page-write-sequences (each time when ext_eeprom_write_page is called).
	 *
	 * Pages in eeprom 24AA512 are 128 bytes large. Beginnings of pages are in
	 * adresses n * 128 (n is natural number including 0). End addresses of 
	 * pages are at n * 128 + 127 (n is natural number including 0).
	 */

	uint16_t current_addr = addr;
	uint16_t current_number = 0;
	uint8_t to_write;
	uint8_t ret;
	while (current_number < number)	{
		to_write = EEPROM_PAGE_SIZE - (addr % EEPROM_PAGE_SIZE);
		if (((uint32_t)current_addr + to_write) > ((uint32_t)addr + number))	{
			to_write = ((uint32_t)addr + number) - current_addr;
			/* if not a whole page is to write any more -> make to_write smaller */
		}
		ret = ext_eeprom_write_page(data + current_number, current_addr, to_write);
		if (ret != TWI_OK)	{
			return ret;
		}
		current_number += to_write;
		current_addr = current_addr + to_write;
	}
	return ret;
}

/**
 * ext_eeprom_read_block - read n bytes from eeprom
 * @data:	pointer to data buffer where the bytes will be stored
 * @addr: 	address of first byte to read
 * @number: number of bytes to read
 *
 * 		If number + addr is bigger than 64kByte ->
 * 		Functions starts to read at beginning of eeprom
 *
 * 		Returns TWI_OK on success and TWI_ERR on failuer
 */
uint8_t ext_eeprom_read_block(uint8_t *data, uint16_t addr, uint16_t number)
{
	if (ext_eeprom_wait() != TWI_OK)	{
		return TWI_ERR;
	}

	//rtc_disable_avr_interrupt();
	uint8_t ret;
	ret = address_read_prefix(addr);
	RETURN_IF_ERROR(ret);
	uint8_t i;
	for (i = 0; i < (number - 1); i++)	{
		ret = twi_read_byte(TWI_ACK, &data[i]);
		RETURN_IF_ERROR(ret);
	}
	ret = twi_read_byte(TWI_NACK, &data[number - 1]);
	RETURN_IF_ERROR(ret);
	twi_stop();
	RETURN_MACRO(ret);
}

/**
 * ext_eeprom_write_byte - write a byte in external eeprom
 * addr: addr of byte to write
 * byte: value to write into this addr
 *
 * 		Returns TWI_OK on succes, TWI_ERR otherwise
 */
uint8_t ext_eeprom_write_byte(uint16_t addr, uint8_t byte)
{
	return ext_eeprom_write_block(&byte, addr, 1);
}

/**
 * ext_eeprom_write_word - write two bytes to external eeprom
 * addr: address of lsb of word to write
 * word: two bytes to be written in little endian (lsb to lowest address)
 *
 * 		Returns TWI_OK on success, TWI_ERR otherwise
 */
uint8_t ext_eeprom_write_word(uint16_t addr, uint16_t word)
{
	return ext_eeprom_write_block((uint8_t *)&word, addr, 2);
}

/**
 * ext_eeprom_write_word - write four bytes to external eeprom
 * addr: address of lsb of dword to write
 * dword: four bytes to be written in little endian (lsb to lowest address)
 *
 * 		Returns TWI_OK on success, TWI_ERR otherwise
 */
uint8_t ext_eeprom_write_dword(uint16_t addr, uint32_t dword)
{
	return ext_eeprom_write_block((uint8_t *)&dword, addr, 4);
}

/**
 * ext_eeprom_read_byte - read a byte from external eeprom
 * addr: address of byte to read
 * byte: pointer where byte will be stored
 *
 * 		Returns TWI_OK on success, TWI_ERR otherwise
 */
uint8_t ext_eeprom_read_byte(uint16_t addr, uint8_t *byte)
{
	return ext_eeprom_read_block(byte, addr, 1);
}

/**
 * ext_eeprom_read_word - read two bytes from external eeprom
 * addr: address of first byte to read
 * word: pointer to two byte variable where the two bytes will get stored
 *
 * 		Two bytes are read in little endian.
 *
 * 		Returns TWI_OK on success, TWI_ERR otherwise
 */
uint8_t ext_eeprom_read_word(uint16_t addr, uint16_t *word)
{
	return ext_eeprom_read_block((uint8_t *)word, addr, 2);
}

/**
 * ext_eeprom_read_dword - read four bytes from external eeprom
 * addr: address of first byte to read
 * dword: pointer to four byte variable where the two bytes will get stored
 *
 * 		Four bytes are read in little endian.
 *
 * 		Returns TWI_OK on success, TWI_ERR otherwise
 */
uint8_t ext_eeprom_read_dword(uint16_t addr, uint32_t *dword)
{
	return ext_eeprom_read_block((uint8_t *)dword, addr, 4);
}

/**
 * ext_eeprom_is_ready - checks if eeprom is ready to send or receive new data
 *
 * 		During write progress eeprom does not respond to request. This
 * 		functions returns whether eeprom is responding or not.
 *
 * 		Returns TRUE if eeprom is ready, FALSE otherwise
 */
uint8_t ext_eeprom_is_ready(void)
{
	uint8_t temp;
	uint8_t ret;

	//rtc_disable_avr_interrupt();

	ret = address_read_prefix(0x00);
	RETURN_IF_ERROR(ret);

	ret = twi_read_byte(TWI_NACK, &temp);
	RETURN_IF_ERROR(ret);
		
	twi_stop();

	RETURN_MACRO(ret);
}



