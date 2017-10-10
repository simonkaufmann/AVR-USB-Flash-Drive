/**
 *  commands.c - functions to parse UFI-commands
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

/*
 * refer to: http://www.usb.org/developers/docs/devclass_docs/usbmass-ufi10.pdf
 * and: http://www.usb.org/developers/docs/devclass_docs/usbmassbulk_10.pdf
 */

#include <avr/io.h>

#include "main.h"
#include "commands.h"
#include "utils.h"
#include "usbdata.h"
#include "ext_eeprom.h"
#include "ram.h"
#include "usbdrv.h"



uint8_t cbw_tag[CBW_TAG_LENGTH];

uint8_t *cbw;
uint8_t csw[13];

static uint16_t count = 0;
uint16_t read = 0; /* read so many blocks of 8 bytes */
uint8_t should_read = FALSE;

uint8_t data_null[8] = {0,0,0,0,0,0,0,0};

uint8_t data_rw[PAGE_SIZE];

uint16_t logical_block;
uint16_t transfer_length;
uint16_t transfer_length_beginning;

/*
 * internal functions
 */

/**
 * create_basic_csw - create csw with no data residue and STATUS CSW_PASSED
 */
void create_basic_csw()
{
		data_cpy(csw, csw_signature, 4);
		data_cpy(csw + CSW_TAG_OFFSET, cbw_tag, CBW_TAG_LENGTH);
		csw[CSW_DATA_RESIDUE_OFFSET] = 0;
		uint8_t i;
		for (i = CSW_DATA_RESIDUE_OFFSET; i < CSW_DATA_RESIDUE_OFFSET + CSW_DATA_RESIDUE_LENGTH; i++)	{
			csw[i] = 0;
		}
		csw[CSW_STATUS_OFFSET] = CSW_PASSED;
}

void execute_cbw(uint8_t *cbw_int)
{
	cbw = cbw_int;

	data_cpy(cbw_tag, cbw + CBW_TAG_OFFSET, CBW_TAG_LENGTH);
	if ((cbw[CBW_FLAGS] & (1 << 7)) > 0)	{
		/* data from device to host */
	} else {
		/* data from host to device */
	}
	uint8_t cblength = cbw[CBW_CBLENGTH_OFFSET] & 0x1f;
	
	uint8_t *cb = cbw + CBW_CB_OFFSET;

	static uint8_t data[36]; /* 36 bytes for inquiry answer */
	
	if (cb[0] == INQUIRY)	{
		data[0] = 0x00; /* peripheral device type: none */  /* this parameters are copied from kingston flash drive */
		data[1] = (1 << 7); /* removable media bit: one */
		data[2] = 00;
		data[3] = 0x02; /* response data format */
		data[4] = 0x1f;
		data[5] = 0;
		data[6] = 0;
		data[7] = 0;
		data_cpy(data + 8, "HeKa", 4);
		data[12] = ' ';
		data[13] = ' ';
		data[14] = ' ';
		data[15] = ' ';
		data_cpy(data + 16, "DIC driv", 8);
		data_cpy(data + 24, (uint8_t *)"e", 1);
		uint8_t i;
		for(i = 25; i < 32; i++)	{
			data[i] = ' ';
		}
		data_cpy(data + 32, (uint8_t *)"0.01", 4);
		usb_send_data(data, 36);
		
		/* csw */
		create_basic_csw();
		usb_send_data(csw, CSW_LENGTH);
	} else if (cb[0] == TEST_UNIT_READY)	{
		/* return a csw -> device is ready*/
		create_basic_csw();
		usb_send_data(csw, CSW_LENGTH);
	} else if (cb[0] == READ_CAPACITY)	{
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
		data[3] = 128; /* 128 logical blocks a 512 byte */
		data[4] = 0;
		data[5] = 0;
		data[6] = 2; 
		data[7] = 0; /* 512 byte blocks */
		usb_send_data(data, 8);
		create_basic_csw();
		usb_send_data(csw, CSW_LENGTH);
	} else if (cb[0] == MODE_SENSE)	{
		//if (cb[2] == 0x3f || cb[2] == 0x08)	{ /* all pages have to be returned */ /* 0x08 for windows! */ /* return always the same data and hope it works! */
			data[0] = 0x35;
			data[1] = 0x00;
			data[2] = 0x00;
			data[3] = 0x00;
			data[4] = 0x05; /* from kingston flash drive */
			data[5] = 0x1e;
			data[6] = 0xf0;
			data[7] = 0x00;
		
			data[8] = 0xff;
			data[9] = 0x20;
			data[10] = 0x00;
			data[11] = 0x1c;
			data[12] = 0xfc;
			uint8_t i;
			for (i = 12; i < 0x23; i++)	{
				data[13] = 0x00;
			}
			usb_send_data(data, 0x23);
			create_basic_csw();
			usb_send_data(csw, CSW_LENGTH);
		//}
	} else if (cb[0] == PREVENT_REMOVAL)	{
		create_basic_csw();
		usb_send_data(csw, CSW_LENGTH);
	} else if (cb[0] == READ)	{
		logical_block = (cb[2] << 24) | (cb[3] << 16) | (cb[4] << 8) | (cb[5]);
		transfer_length = (cb[7] << 8) | cb[8];
		transfer_length_beginning = transfer_length;
		uint16_t i;
		for (i = 0; i < 8; i++)	{
			data[i] = 0;
		}
		count = 0;
		read = BLOCK_SIZE / 8;
		should_read = TRUE;
	} else if (cb[0] == WRITE)	{
		if (is_writing_beginning == FALSE)	{
			set_writing_beginning();
		}
		logical_block = (cb[2] << 24) | (cb[3] << 16) | (cb[4] << 8) | (cb[5]);
		set_writing(BLOCK_SIZE / 8, cb[7] << 8 | cb[8]); /* so many 8 bit blocks */
	} else if (cb[0] == START_STOP_UNIT)	{
		create_basic_csw();
		usb_send_data(csw, CSW_LENGTH);
	} else if (cb[0] == READ_FORMAT_CAPACITIES)	{
		usb_send_data(data_null, 4); /* do not really understand the command, just return any data and hope that host will ask other command for capacity! */
		create_basic_csw();
		usb_send_data(csw, CSW_LENGTH);
	}
}

void execute_read(void)
{
	if (should_read == TRUE)	{
		LED_ON();
		if (count <= read)	{
			uint8_t i = 0;
			static uint8_t data[8];
			for (i = 0; i < 8; i++)	{
				data[i] = ram_read_byte(logical_block * BLOCK_SIZE + count * 8 + i + (transfer_length_beginning - transfer_length) * BLOCK_SIZE);
			}
			usbSetInterrupt(data, 8);
			count++;
		}
		if (count == read)	{
			transfer_length--;
			if (transfer_length == 0)	{
				LED_OFF();
				create_basic_csw();
				usb_send_data(csw, CSW_LENGTH);
				should_read = FALSE;
			} else {
				count = 0;
			}
		}
	}
}

void execute_writing_finished(void)
{
	create_basic_csw();
	usb_send_data(csw, CSW_LENGTH);
}
