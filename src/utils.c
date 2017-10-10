/**
 *  utils.c - miscellaneous functions
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

#include "main.h"
#include "utils.h"

/**
 * data_cmp - compares two data arrays
 * data1:	pointer to first array
 * data2:	pointer to second array
 * length: 	length of array
 *
 * 		Returns TRUE if data equals, FALSE if data is not equal
 */
uint8_t data_cmp(const uint8_t *data1, const uint8_t *data2, uint8_t length)
{
	uint8_t i;
	for (i = 0; i < length; i++)	{
		if (data1[i] != data2[i])	{
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * data_cpy - copy a data array
 * dest:	pointer to destination array
 * src: 	pointer to source array
 * length: 	number of bytes to copy
 */
void data_cpy(uint8_t *dest, uint8_t *src, uint8_t length)
{
	uint8_t i;
	for (i = 0; i < length; i++)	{
		dest[i] = src[i];
	}
}
