/**
 *  utils.h - miscellenaeous functions
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

#ifndef UTILS_H
#define UTILS_H

uint8_t data_cmp(const uint8_t *data1, const uint8_t *data2, uint8_t length);
void data_cpy(uint8_t *dest, uint8_t *src, uint8_t length);

#endif
