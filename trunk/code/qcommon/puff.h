/*
 *  This is a modified version of Mark Adlers work,
 *  see below for the original copyright.
 *  2006 - Joerg Dietrich <dietrich_joerg@gmx.de>
 */

/* puff.h
  Copyright (C) 2002, 2003 Mark Adler, all rights reserved
  version 1.7, 3 Mar 2002

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler    madler@alumni.caltech.edu
 */

#ifndef __PUFF_H
#define __PUFF_H

#ifdef _MSC_VER
  #include <io.h>

  typedef __int64 int64_t;
  typedef __int32 int32_t;
  typedef __int16 int16_t;
//  typedef __int8 int8_t;
  typedef unsigned __int64 uint64_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int8 uint8_t;

  // vsnprintf is ISO/IEC 9899:1999
  // abstracting this to make it portable
  int Q_vsnprintf(char *str, size_t size, const char *format, va_list ap);
#else
  #include <stdint.h>

  #define Q_vsnprintf vsnprintf
#endif	/* for definitions of the <stdint.h> types */

/*
 * See puff.c for purpose and usage.
 */
int32_t puff(uint8_t  *dest,		/* pointer to destination pointer */
             uint32_t *destlen,		/* amount of output space */
             uint8_t  *source,		/* pointer to source data pointer */
             uint32_t *sourcelen);	/* amount of input available */

#endif // __PUFF_H
