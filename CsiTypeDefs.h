/* CsiTypeDefs.h

   Copyright (C) 1997, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 24 April 1997
   Last Change: Tuesday 14 March 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC) 
   Committed by: $Author: tmecham $
*/

#ifndef CsiTypeDefs_h
#define CsiTypeDefs_h

#ifdef _WIN32

typedef unsigned char Byte;
typedef unsigned char byte;
typedef unsigned char UInt1;
typedef UInt1 uint1;
typedef unsigned short UInt2;
typedef UInt2 uint2;
typedef unsigned long UInt4;
typedef UInt4 uint4;
typedef char Int1;
typedef Int1 int1; 
typedef short Int2;
typedef Int2 int2;
typedef long Int4;
typedef Int4 int4;

/* These types specific to the Microsoft compiler */
typedef __int64 Int8;
typedef unsigned __int64 UInt8;
typedef Int8 int8;
typedef UInt8 uint8;

#define Int8_Max   0x7FFFFFFFFFFFFFFF
#define Int8_Min   0x0000000000000000

#else

#include <stdint.h>
typedef uint8_t byte;
typedef uint8_t Byte;
typedef uint8_t UInt1;
typedef uint8_t uint1;
typedef int8_t Int1;
typedef int8_t int1;
typedef int16_t Int2;
typedef int16_t int2;
typedef uint16_t UInt2;
typedef uint16_t uint2;
typedef int32_t Int4;
typedef int32_t int4;
typedef uint32_t UInt4;
typedef uint32_t uint4;
typedef int64_t Int8;
typedef int64_t int8;
typedef uint64_t UInt8;
typedef uint64_t uint8;

#define Int8_Max   0x7fffffffffffffffLL
#define Int8_Min   0x0000000000000000LL

#endif


/* define the maximum and minimum values unsigned types */
#define UInt1_Max  0xFF
#define UInt1_Min  0x00
#define UInt2_Max  0xFFFF
#define UInt2_Min  0x0000
#define UInt4_Max  0xFFFFFFFF
#define UInt4_Min  0x00000000
#define UInt31_Max 0x7FFFFFFF
#define UInt31_Min 0x00000000

#endif
