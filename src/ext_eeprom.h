/*
 *  ext_eeprom.h - definitions for library for eeprom chip 24AA256
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

uint8_t ext_eeprom_write_byte(uint16_t addr, uint8_t byte);
uint8_t ext_eeprom_write_word(uint16_t addr, uint16_t word);
uint8_t ext_eeprom_write_dword(uint16_t addr, uint32_t dword);
uint8_t ext_eeprom_write_block(const uint8_t *data, uint16_t addr, uint16_t number);

uint8_t ext_eeprom_read_byte(uint16_t addr, uint8_t *byte);
uint8_t ext_eeprom_read_word(uint16_t addr, uint16_t *word);
uint8_t ext_eeprom_read_dword(uint16_t addr, uint32_t *dword);
uint8_t ext_eeprom_read_block(uint8_t *data, uint16_t addr, uint16_t number);

uint8_t ext_eeprom_is_ready(void);


