/*-
 * Free/Libre Near Field Communication (NFC) library
 *
 * Libnfc historical contributors:
 * Copyright (C) 2009      Roel Verdult
 * Copyright (C) 2009-2013 Romuald Conty
 * Copyright (C) 2010-2012 Romain Tartière
 * Copyright (C) 2010-2013 Philippe Teuwen
 * Copyright (C) 2012-2013 Ludovic Rousseau
 * See AUTHORS file for a more comprehensive list of contributors.
 * Additional contributors of this file:
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 *
 * @file mirror-subr.h
 * @brief Mirror bytes
 */

#ifndef _LIBNFC_MIRROR_SUBR_H_
#  define _LIBNFC_MIRROR_SUBR_H_

#  include <stdint.h>
uint8_t  mirror(uint8_t bt);
void mirror_bytes(uint8_t *pbts, size_t szLen);
uint32_t mirror32(uint32_t ui32Bits);
uint64_t mirror64(uint64_t ui64Bits);

void iso14443a_crc_append(uint8_t *pbtData, size_t szLen);
uint8_t iso14443a_Par(uint8_t pbtData);
#endif // _LIBNFC_MIRROR_SUBR_H_
