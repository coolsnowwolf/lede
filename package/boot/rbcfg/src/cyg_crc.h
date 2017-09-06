//==========================================================================
//
//      crc.h
//
//      Interface for the CRC algorithms.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Andrew Lunn
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Andrew Lunn
// Contributors: Andrew Lunn
// Date:         2002-08-06
// Purpose:
// Description:
//
// This code is part of eCos (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _SERVICES_CRC_CRC_H_
#define _SERVICES_CRC_CRC_H_

#if 0
#include <cyg/infra/cyg_type.h>
#else
#include <stdint.h>
typedef uint32_t cyg_uint32;
typedef uint16_t cyg_uint16;
#endif

#ifndef __externC
# ifdef __cplusplus
#  define __externC extern "C"
# else
#  define __externC extern
# endif
#endif

// Compute a CRC, using the POSIX 1003 definition

__externC cyg_uint32
cyg_posix_crc32(unsigned char *s, int len);

// Gary S. Brown's 32 bit CRC

__externC cyg_uint32
cyg_crc32(unsigned char *s, int len);

// Gary S. Brown's 32 bit CRC, but accumulate the result from a
// previous CRC calculation

__externC cyg_uint32
cyg_crc32_accumulate(cyg_uint32 crc, unsigned char *s, int len);

// Ethernet FCS Algorithm

__externC cyg_uint32
cyg_ether_crc32(unsigned char *s, int len);

// Ethernet FCS algorithm, but accumulate the result from a previous
// CRC calculation.

__externC cyg_uint32
cyg_ether_crc32_accumulate(cyg_uint32 crc, unsigned char *s, int len);

// 16 bit CRC with polynomial x^16+x^12+x^5+1

__externC cyg_uint16
cyg_crc16(unsigned char *s, int len);

#endif // _SERVICES_CRC_CRC_H_



