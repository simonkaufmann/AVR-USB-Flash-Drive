/**
 *  commands.h - functions to parse UFI-commands
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

#ifndef COMMANDS_H
#define COMMANDS_H

void execute_commands(void);
void execute_cbw(uint8_t *cbw);
void execute_writing_finished(void);
void create_basic_csw(void);
void execute_read(void);

extern uint8_t data_rw[];
extern uint16_t logical_block;
extern uint8_t csw[];

#define CSW_TAG_OFFSET	4
#define CSW_DATA_RESIDUE_OFFSET	8
#define CSW_DATA_RESIDUE_LENGTH	4
#define CSW_STATUS_OFFSET		12
#define CSW_LENGTH 13

#define CBW_TAG_OFFSET	4
#define CBW_TAG_LENGTH	4
#define CBW_FLAGS		12
#define CBW_CBLENGTH_OFFSET	14
#define CBW_CB_OFFSET	15


#define CSW_PASSED 0x00
#define CSW_FAILED 0x01
#define CSW_PHASE_ERROR 0x02

#define INQUIRY			0x12
#define TEST_UNIT_READY 0x00
#define READ_CAPACITY	0x25
#define MODE_SENSE		0x1a
#define PREVENT_REMOVAL 0x1e
#define READ			0x28
#define WRITE			0x2a
#define START_STOP_UNIT	0x1b
#define READ_FORMAT_CAPACITIES 0x23

#endif
