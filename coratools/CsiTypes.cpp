/* CsiTypes.cpp

   This module implements functions that convert CSI defined types into standard
   values (integers, time stamps, and doubles)


   Copyright (C) 1998, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 23 April 1997
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "CsiTypes.h"
#include "MsgExcept.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include "Csi.BuffStream.h"
#include "Csi.StringLoader.h"
#include <stdio.h>
#include <math.h>
#include <limits>
#include <algorithm>
#include <locale>
#include <iomanip>


struct CsiDataTypeInfo
{
   uint4 type;
   uint4 len_bytes;
   uint4 len_bits;
   char const *name;
   char const *tob1_name;
   char const *xml_name;
   uint4 logger_type;
};

// this structure maps supported datalogger data types to their associated
// length.  The information this table was constructed from was derived from
// the document at http://eng/SoftwareNotes/CommProtocols/BMP3/types.htm. Note
// the ASCII data types map to one character in length. This is because all 
// strings are represented as arrays. The one therefore refers to the length 
// of a single character in the string.
static const CsiDataTypeInfo CsiTypes[] =
{  //  type        len_bytes  len_bits   name     tob1_name   xml_name              logger_type
   { CsiUInt1,      1,           8,    "CsiUInt1", "BYTE",    "xsd:unsignedByte",   LgrUInt1 },
   { CsiUInt2,      2,           16,   "CsiUInt2", "USHORT",  "xsd:unsignedShort",  LgrUInt2 },
   { CsiUInt4,      4,           32,   "CsiUInt4", "ULONG",   "xsd:unsignedInt",    LgrUInt4 },
   { CsiInt1,       1,           8,    "CsiInt1",  "INT1",    "xsd:byte",           LgrInt1 },
   { CsiInt2,       2,           16,   "CsiInt2",  "SHORT",   "xsd:short",          LgrInt2 },
   { CsiInt4,       4,           32,   "CsiInt4",  "LONG",    "xsd:int",            LgrInt4},
   { CsiInt8,       8,           64,   "CsiInt8",  "LONG",    "xsd:long",           LgrUnknown},
   { CsiFs2,        2,           16,   "CsiFs2",   "FP2",     "xsd:float",          LgrFs2},
   { CsiFs3,        3,           24,   "CsiFs3",   "IEEE4",   "xsd:float",          LgrFs3},
   { CsiFs4,        4,           32,   "CsiFs4",   "IEEE4",   "xsd:float",          LgrFs4},
   { CsiFsf,        4,           32,   "CsiFsf",   "IEEE4",   "xsd:float",          LgrUnknown},
   { CsiFp4,        4,           32,   "CsiFp4",   "IEEE4",   "xsd:float",          LgrFp4},
   { CsiIeee4,      4,           32,   "CsiIeee4", "IEEE4",   "xsd:float",          LgrIeee4},
   { CsiIeee8,      8,           64,   "CsiIeee8", "IEEE8",   "xsd:double",         LgrIeee8},
   { CsiBool,       1,           8,    "CsiBool",  "BOOL",    "xsd:boolean",        LgrBool},
   { CsiBool2,      2,           16,   "CsiBool2", "BOOL",    "xsd:boolean",        LgrBool2},
   { CsiBool4,      4,           32,   "CsiBool4", "BOOL",    "xsd:boolean",        LgrBool4},
   { CsiBool8,      1,           1,    "CsiBool8", "BOOL8",    "xsd:boolean",        LgrBool8},
   { CsiSec,        4,           32,   "CsiSec",   "SecNano", "xsd:dateTime",       LgrSec},
   { CsiUSec,       6,           48,   "CsiUSec",  "SecNano", "xsd:dateTime",       LgrUSec},
   { CsiNSec,       8,           64,   "CsiNSec",  "SecNano", "xsd:dateTime",       LgrNSec},
   { CsiAscii,      1,           8,    "CsiAscii", "ASCII",   "xsd:string",         LgrAscii},
   { CsiAsciiZ,     1,           8,    "CsiAsciiZ", 0,        "xsd:string",         LgrAsciiZ},
   { CsiInt2Lsf,    2,           16,   "CsiInt2Lsf", "SHORT", "xsd:short",          LgrInt2Lsf},
   { CsiInt4Lsf,    4,           32,   "CsiInt4Lsf", "LONG",  "xsd:int",            LgrInt4Lsf},
   { CsiUInt2Lsf,   2,           16,   "CsiUInt2Lsf","USHORT","xsd:unsignedShort",  LgrUInt2Lsf},
   { CsiUInt4Lsf,   4,           32,   "CsiUInt4Lsf", "ULONG","xsd:unsignedInt",    LgrUInt4Lsf},
   { CsiNSecLsf,    8,           64,   "CsiNSecLsf",  "SecNano","xsd:dateTime",     LgrNSecLsf},
   { CsiIeee4Lsf,   4,           32,   "CsiIeee4Lsf", "IEEE4","xsd:float",          LgrIeee4Lsf},
   { CsiIeee8Lsf,   8,           64,   "CsiIeee8Lsf", "IEEE8","xsd:double",         LgrIeee8Lsf},
   { CsiInt8Lsf,    8,           64,   "CsiInt8Lsf",  "LONG", "xsd:long",           LgrUnknown },
   { CsiLgrDate,    8,           64,   "CsiLgrDate",  "SecNano", "xsd:dateTime",    LgrUnknown },
   { CsiLgrDateLsf, 8,           64,   "CsiLgrDateLsf","SecNano","xsd:dateTime",    LgrUnknown }, 
   { 0,             0,           0,    "",            0,       0,                   0 }
};


CsiLgrTypeCode csi_to_lgr_type(CsiDbTypeCode type)
{
   CsiLgrTypeCode rtn = LgrUnknown;
   for(uint4 i = 0; CsiTypes[i].type != 0; ++i)
   {
      if(CsiTypes[i].type == type)
      {
         rtn = static_cast<CsiLgrTypeCode>(CsiTypes[i].logger_type);
         break;
      }
   }
   return rtn;
} // csi_to_lgr_type


CsiDbTypeCode lgr_to_csi_type(CsiLgrTypeCode type)
{
   CsiDbTypeCode rtn = CsiUnknown;
   for(uint4 i = 0; CsiTypes[i].type != 0; ++i)
   {
      if(CsiTypes[i].logger_type == type)
      {
         rtn = static_cast<CsiDbTypeCode>(CsiTypes[i].type);
         break;
      }
   }
   return rtn;
} // lgr_to_csi_type 


uint4 csiTypeLen(CsiDbTypeCode typeId)
{
   uint4 rtn = 0;

   for(uint4 i = 0; CsiTypes[i].type != 0; i++)
      if(CsiTypes[i].type == uint4(typeId))
      {
         rtn = CsiTypes[i].len_bytes;
         break; 
      }
   return rtn;
} // csiTypeLen


uint4 csiTypeLenBits(CsiDbTypeCode type_id)
{
   uint4 rtn = 0;
   for(uint4 i = 0; CsiTypes[i].type != 0; i++)
      if(CsiTypes[i].type == uint4(type_id))
      {
         rtn = CsiTypes[i].len_bits;
         break;
      }
   return rtn;
} // csiTypeLenBits


bool isCsiDbTypeCode(uint4 type)
{
   bool rtn = false;
   for(uint4 i = 0; !rtn && CsiTypes[i].type != 0; i++)
      if(CsiTypes[i].type == type)
         rtn = true;
   return rtn;
} // isCsiDbTypeCode


char const *csiTypeName(CsiDbTypeCode type)
{
   char const *rtn = "unknown";
   for(uint4 i = 0; CsiTypes[i].type != 0; i++)
   {
      if(CsiTypes[i].type == static_cast<uint4>(type))
      {
         rtn = CsiTypes[i].name;
         break;
      }
   }
   return rtn;
} // csiTypeName


char const *lgrTypeName(CsiLgrTypeCode type)
{
   char const *rtn = "unknown";
   for(uint4 i = 0; CsiTypes[i].type != 0; i++)
   {
      if(CsiTypes[i].logger_type == static_cast<uint4>(type))
      {
         rtn = CsiTypes[i].name;
         break;
      }
   }
   return rtn;
} // lgrTypeName


char const *tob_type_string(CsiDbTypeCode type)
{
   char const *rtn = 0;
   for(uint4 i = 0; rtn == 0 && CsiTypes[i].type != 0; ++i)
   {
      if(CsiTypes[i].type == static_cast<uint4>(type))
      {
         rtn = CsiTypes[i].tob1_name;
         break;
      }
   }
   return rtn;
} // tob_type_string


void parse_tob_data_type(
   char const *type_name_,
   CsiLgrTypeCode &data_type,
   uint4 &string_len)
{
   struct type_names_map_type
   {
      char const *name;
      CsiLgrTypeCode type;
   } type_names_map[] = {
      { "IEEE4",  LgrIeee4Lsf },
      { "IEEE4L", LgrIeee4Lsf },
      { "IEEE4B", LgrIeee4 },
      { "FP2",    LgrFs2 },
      { "SHORT",  LgrInt2Lsf },
      { "USHORT", LgrUInt2Lsf },
      { "INT2",   LgrInt2 },
      { "UINT2",  LgrUInt2 },
      { "ULONG",  LgrUInt4Lsf },
      { "LONG",   LgrInt4Lsf },
      { "UINT4",  LgrUInt4 },
      { "INT4",   LgrInt4 },
      { "SecNano", LgrNSecLsf },
      { "NSec",   LgrNSec },
      { "ASCII",  LgrAscii },
      { "BOOL4",  LgrBool4 },
      { "BOOL2",  LgrBool2 },
      { "BOOL",   LgrBool },
      { "BYTE",   LgrUInt1 },
      { "INT1",   LgrInt1 },
      { "BOOL8",  LgrBool8 },
      { "IEEE8",  LgrIeee8Lsf },
      { "IEEE8L", LgrIeee8Lsf },
      { "IEEE8B", LgrIeee8 },
      { 0,        LgrUnknown }
   }; 
         
   // if the name has a length specified, we need to extract that and then modify the type
   // string so that it can be compared against the names in the type names map.  We also
   // need to trim off any trailing white space from the string. 
   StrAsc name(type_name_);
   size_t paren_pos = name.find("(");
   size_t space_pos;

   string_len = 0;
   if(paren_pos < name.length())
   {
      string_len = strtoul(name.c_str() + paren_pos + 1,0,10);
      name.cut(paren_pos);
   }
   space_pos = name.find(" ");
   if(space_pos < name.length())
      name.cut(space_pos);
   
   // we will now loop through the known type names in order to determine the type code for
   // this field. 
   data_type = LgrUnknown;
   for(int i = 0; type_names_map[i].name != 0; ++i)
   {
      if(name == type_names_map[i].name)
      {
         data_type = type_names_map[i].type;
         break;
      }
   }
   if(data_type == LgrUnknown)
      throw std::invalid_argument("Unknown data type specified");
} // parse_tob_data_type


char const *xml_type_string(CsiDbTypeCode type)
{
   char const *rtn = 0;
   for(uint4 i = 0; rtn == 0 && CsiTypes[i].type != 0; ++i)
   {
      if(CsiTypes[i].type == static_cast<uint4>(type))
      {
         rtn = CsiTypes[i].xml_name;
         break;
      }
   }
   return rtn;
} // xml_type_string


Int1 csiInt1ToInt1(void const *buff)
{
   Int1 const *rtn = (Int1 const *)buff;
   return *rtn;
} // csiInt1ToInt1


UInt1 csiUInt1ToUInt1(void const *buff)
{
   UInt1 const *rtn = (UInt1 const *)buff;
   return *rtn;
} // csiUint1toUint1


Int2 csiInt2ToInt2(void const *buff)
{
   return Int2(csiUInt2ToUInt2(buff));
} // csiInt2ToInt2


Int2 csiInt2LsfToInt2(void const *buff)
{
   Int2 const *rtn = (Int2 const *)buff;
   return *rtn;
} // csiInt2LsfToInt2


UInt2 csiUInt2ToUInt2(void const *buff)
{
   UInt2 rtn = 0;
   UInt1 const *ch = (UInt1 *)buff;
   rtn = ch[0]; rtn <<= 8;
   rtn += ch[1];
   return rtn;
} // csiUInt2ToInt2


UInt2 csiUInt2LsfToUInt2(void const *buff)
{
   UInt2 const *rtn = (UInt2 const *)buff;
   return *rtn;
} // csiUInt2LsfToUInt2


Int4 csiInt4ToInt4(void const *buff)
{
   return Int4(csiUInt4ToUInt4(buff));
} // csiInt4ToInt4


Int4 csiInt4LsfToInt4(void const *buff)
{
   Int4 const *rtn = (Int4 const *)buff;
   return *rtn;
} // csiInt4LsfToInt4


UInt4 csiUInt4ToUInt4(void const *buff)
{
   UInt4 rtn = 0;
   UInt1 const *ch = (UInt1 *)buff;
   for(int i = 0; i < 4; i++)
   {
      rtn <<= 8;
      rtn += ch[i]; 
   }
   return rtn;
} // csiUInt4ToInt4


UInt4 csiUInt4LsfToUInt4(void const *buff)
{
   UInt4 const *rtn = (UInt4 const *)buff;
   return *rtn;
} // csiUInt4LsfToUInt4


float csiIeee4ToFloat(void const *buff)
{
   UInt1 temp[4];
   UInt1 const *ch = (UInt1 const *)buff;
   for(int i = 0; i < 4; i++)
      temp[4 - i] = ch[i];
   float *rtn = (float *)temp;
   return *rtn;
} // csiIeee4ToFloat


float csiIeee4LsfToFloat(void const *buff)
{
   float const *rtn = (float const *)buff;
   return *rtn;
} // csiIeee4LsfToFloat


double csiIeee8ToDouble(void const *buff)
{
   UInt1 temp[8];
   UInt1 const *ch = (UInt1 const *)buff;
   for(int i = 0; i < 8; i++)
      temp[7 - i] = ch[i];
   return double(*temp);
} // csiIeee8ToDouble


int8 csiInt8LsfToInt8(void const *buff)
{
   int8 const *rtn = static_cast<int8 const *>(buff);
   return *rtn;
} // csiInt8ToDouble


double csiIeee8LsfToDouble(void const *buff)
{
   double const *rtn = (double const *)buff;
   return *rtn;
} // csiIeee8LsfToDouble


bool csiBoolToBool(void const *buff)
{
   UInt1 ch = csiUInt1ToUInt1(buff);
   return (ch == 0) ? false : true;
} // csiBoolToBool


UInt1 csiBool8ToUInt1(void const *buff)
{
   return csiUInt1ToUInt1(buff);
} // csiBool8ToUInt1


Csi::LgrDate csiSecToLgrDate(void const *buff)
{
   UInt4 secs = csiUInt4ToUInt4(buff);
   Csi::LgrDate rtn(Csi::LgrDate::nsecPerSec*secs);
   return rtn;
} // csiSecToLgrDate


Csi::LgrDate csiUSecToLgrDate(void const *buff)
{
   // read the six byte stamp
   int8 usecs = 0;
   Byte const *temp = (Byte const *)buff;

   for(int i = 0; i < 6; i++)
   {
      usecs <<= 8;
      usecs += temp[i];
   }

   // convert the offset from micro-seconds to nano-seconds
   usecs *= 10000;
   return Csi::LgrDate(usecs);
} // csiUSecToLgrDate


Csi::LgrDate csiNSecToLgrDate(void const *buff)
{
   // read the seconds and nanoseconds components
   byte const *temp = static_cast<byte const *>(buff);
   int4 secs = csiInt4ToInt4(temp);
   uint4 nsecs = csiUInt4ToUInt4(temp + 4);
   Csi::LgrDate rtn(Csi::LgrDate::nsecPerSec*secs);

   rtn += Csi::LgrDate(nsecs);
   return rtn;
} // csiNSecToLgrDate


Csi::LgrDate csiNSecLsfToLgrDate(void const *buff)
{
   // read the seconds and nanoseconds components
   byte const *temp = static_cast<byte const *>(buff);
   int4 secs = csiInt4LsfToInt4(temp);
   uint4 nsecs = csiUInt4LsfToUInt4(temp + 4);
   Csi::LgrDate rtn(Csi::LgrDate::nsecPerSec*secs);

   rtn += Csi::LgrDate(nsecs);
   return rtn;
} // csiNSecLsfToLgrDate


#pragma warning(disable: 4996)


char const *csiFs2ToString(StrAsc &dest, void const *buffer)
{
   // A value of 0x9ffe explicitly encodes NaN
   byte const *b = static_cast<byte const *>(buffer);
   if(b[0] == 0x9f && b[1] == 0xfe)
      dest = "NaN";
   else if (b[0] == 0x9f && b[1] == 0xff)
      dest = "-INF"; 
   else if (b[0] == 0x1f && b[1] == 0xff)
      dest = "+INF";
   else
   {
      uint4 byte1 = (b[0] & 0x1f);
      uint4 byte2 = b[1];
      uint4 mantissa = (byte1 << 8) + byte2;
      bool is_negative = (b[0] & 0x80) != 0;
      uint4 decimal_locator = static_cast<uint4>(b[0] & 0x60) >> 5;
      char temp[10];
      
      // format the fields as a string
      sprintf(temp,"%u",mantissa);
      dest = temp;
      while(dest.length() < decimal_locator)
         dest.insert("0",0);
      if(decimal_locator > 0)
         dest.insert(".",dest.length() - decimal_locator);
      if(is_negative && mantissa != 0)
         dest.insert("-",0);
   }
   return dest.c_str();
} // csiFs2ToString


float csiFs2ToFloat(void const *buff_)
{
   // we are going to cast the binary value into a two byte integer so that it is convenient to pick
   // out patterns and pick off parts of the structure.
   byte const *buff = static_cast<byte const *>(buff_);
   uint2 fs_word = (uint2(buff[0]) << 8) + buff[1];

   // we can now pick off the components of the FS2 structure
   static uint2 const pos_infinity = 0x1fff;
   static uint2 const neg_infinity = 0x9fff;
   static uint2 const not_a_number = 0x9ffe;
   bool is_negative = ((fs_word & 0x8000) != 0);
   uint2 mantissa = fs_word & 0x1FFF;
   uint2 exponent = (fs_word & 0x6000) >> 13;
   double rtn;

   if(fs_word == pos_infinity)
      rtn = std::numeric_limits<float>::infinity();
   else if(fs_word == neg_infinity)
      rtn = -1.0f * std::numeric_limits<float>::infinity();
   else if(fs_word == not_a_number)
      rtn = std::numeric_limits<float>::quiet_NaN();
   else
   {
      rtn = static_cast<float>(mantissa);

      for(uint2 i = 0; mantissa != 0 && i < exponent; ++i)
         rtn /= 10.0f;
      if(is_negative && mantissa != 0)
         rtn *= -1.0f;
   }
   return static_cast<float>(rtn);
} // csiFs2ToFloat


double csiFs2ToDouble(void const *buff)
{ return csiFs2ToFloat(buff); }


void doubleToCsiFs2(void *buff_, double value)
{
   byte *buff = static_cast<byte *>(buff_);
   if(Csi::is_finite(value))
   {
      // we need to determine the sign, exponent, and mantissa (in base 10).  These values will
      // depend on the magnitude of the value
      bool is_negative = false;
      uint2 mantissa = 0;
      uint2 exponent = 0;
      
      if(value < 0)
      {
         is_negative = true;
         value = -value;
      }
      if(value < 800)
      {
         if(value < 8)
         {
            exponent = 0x6000;
            mantissa = static_cast<uint2>(value * 1000 + 0.5); 
         }
         else if(value < 80)
         {
            exponent = 0x4000;
            mantissa = static_cast<uint2>(value * 100 + 0.5);
         }
         else
         {
            exponent = 0x2000;
            mantissa = static_cast<uint2>(value * 10 + 0.5);
         }
      }
      else if(value < 7999)
      {
         mantissa = static_cast<uint2>(value + 0.5);
         exponent = 0;
      }
      else
         mantissa = 7999;
      mantissa |= exponent;
      if(is_negative)
         mantissa |= 0x8000;
      buff[0] = static_cast<byte>((mantissa & 0xff00) >> 8);
      buff[1] = static_cast<byte>(mantissa & 0x00ff);
   }
   else
   {
      if(Csi::is_signalling_nan(value))
      {
         buff[0] = 0x9f;
         buff[1] = 0xfe;
      }
      else if(value > 0)
      {
         buff[0] = 0x1f;
         buff[1] = 0xff;
      }
      else
      {
         buff[0] = 0x9f;
         buff[1] = 0xff;
      }
   }
} // doubleToCsiFs2


double csiFs3ToDouble(void const *buff)
{
   StrAsc temp;
   return atof(csiFs3ToString(temp,buff));
} // csiFs3ToDouble


float csiFs3ToFloat(void const *buff)
{ return static_cast<float>(csiFs3ToDouble(buff)); }


char const *csiFs3ToString(StrAsc &dest, void const *buff)
{
   // extract the field from the pattern
   byte const *b = static_cast<byte const *>(buff);
   bool is_negative = (b[0] & 0x80) != 0;
   uint4 decimal_place = (b[0] & 0x70) >> 4;
   uint4 mantissa;
   uint4 b0 = b[0] & 0x0f;
   uint4 b1 = b[1];
   uint4 b2 = b[2];

   mantissa = (b0 << 16) + (b1 << 8) + b2;

   // Convert the base mantissa into a string
   char temp[10];

   sprintf(temp,"%u",mantissa);
   dest = temp;

   // now assign the ddecimal place
   while(dest.length() < decimal_place)
      dest.insert("0",0);
   if(decimal_place > 0)
      dest.insert(".",dest.length() - decimal_place);

   // assign the sign notation
   if(is_negative)
      dest.insert("-",0);
   return dest.c_str();
} // csiFs3ToString


float csiFp4ToFloat(void const *buff)
{
   uint4 value = *(reinterpret_cast<uint4 const *>(buff));
   union 
   {
      float float_value;
      uint4 uint4_value;
   } rtn;

   rtn.float_value = 0.0F;
   if(value != 0)
   {
      // before converting, we might need to reverse the byte order of the value
      if(!Csi::is_big_endian())
      {
         byte *temp = reinterpret_cast<byte *>(&value);
         std::swap(temp[0],temp[3]);
         std::swap(temp[1],temp[2]);
      }
      if(value == 0x7FFFFFFF)
         rtn.uint4_value = 0x7F800000; // positive infinity
      else if(value == 0xFFFFFFFF)
         rtn.uint4_value = 0xFF800000; // negative infinity
      else
      {
         uint4 exponent = ((value >> 1) & 0x3F800000) + 0x1F000000;
         rtn.uint4_value = (exponent | (value & 0x807FFFFF));
      }
   }
   return rtn.float_value;
} // csiFp4ToDouble


char const *csiFp4ToString(StrAsc &dest, void const *buffer)
{ return csiFloatToString(dest,csiFp4ToFloat(buffer),7); }


double csiFp4ToDouble(const void *buff)
{ return static_cast<double>(csiFp4ToFloat(buff)); }


void floatToCsiFp4(void *buffer, float value)
{
   uint4 value_bytes = *reinterpret_cast<uint4 *>(&value);
   uint4 return_value_bytes = 0;

   if(value_bytes != 0)
   {
      // IEEE4 has an extra bit of range on its exponent that is not present in CsiFp4. Because of
      // this, not all valid IEEE4 values can be applied to CsiFp4. We need to check the range of
      // the supplied exponent to determine if the conversion is valid.
      uint4 exponent = (value_bytes & 0x7F800000) >> 23;
      static const uint4 ieee4_bias = 127;
      static const uint4 exponent_max = ieee4_bias + 64;
      static const uint4 exponent_min = ieee4_bias - 64;
      if(exponent > exponent_max || exponent < exponent_min)
         throw std::out_of_range("conversion to CsiFp4 invalid");
      
      // we need to adjust the exponent so that the most signicant bit of the CSI mantissa can be
      // set.
      exponent = ((value_bytes << 1) - 0x3E000000) & 0x7F000000;

      // now put the return value together. We do this by masking off the sign bit, the IEEE
      // mantissa and oring them with the CSI exponent and setting the most significant mantissa bit
      // to one.
      return_value_bytes = (exponent | (value_bytes & 0x80FFFFFF) | 0x00800000);

      // if we are running on a machine with little endian ordering (intel machines), we need to
      // reverse the byte order of the returned value
      if(!Csi::is_big_endian())
      {
         byte *temp = reinterpret_cast<byte *>(&return_value_bytes);
         std::swap(temp[0],temp[3]);
         std::swap(temp[1],temp[2]);
      }
   }
   memcpy(buffer,&return_value_bytes,sizeof(return_value_bytes));
} // ieee4toCsiFp4


void doubleToCsiFp4(void *buff, double val)
{ floatToCsiFp4(buff,static_cast<float>(val)); }


static uint2 fsWord(void const *buff)
{
   Byte const *cBuff = (Byte const *)buff;
   uint2 rtn = cBuff[0];
   
   rtn <<= 8;
   rtn += cBuff[1];
   return rtn;
} // fsWord


FsfType csiFsfType(void const *buff)
{
   // get the first word
   static uint2 const id_mask = 0xFC00;
   static uint2 const dummy_mask = 0x7F00;
   static uint2 const low_res_mask = 0x1C00;
   static uint2 const begin_ascii_mask = 0x7D00;
   static uint2 const end_byte_mode_mask = 0xFF00;
   static uint2 const high_res_second_mask = 0x3C00;
   uint2 w1 = fsWord(buff);
   FsfType rtn;

   // see what type the bit pattern indicates
   if((w1&id_mask) == id_mask)
      rtn = FsfArrayId;
   else if((w1&0xFF00) == dummy_mask)
      rtn = FsfDummy;
   else if((w1&begin_ascii_mask) == begin_ascii_mask)
      rtn = FsfBeginAscii;
   else if((w1&end_byte_mode_mask) == end_byte_mode_mask)
      rtn = FsfEndByteMode;
   else if((w1&low_res_mask) != low_res_mask)
      rtn = FsfLoRes;
   else if((w1&low_res_mask) == low_res_mask)
   {
      if((w1&high_res_second_mask) == high_res_second_mask)
         rtn = FsfHiResScndWord;
      else
         rtn = FsfHiRes;
   }
   else
      rtn = FsfCantTell;
   return rtn;
} // csiFsfType


double csiFsfToDouble(void const *buff, uint4 len, FsfType &type)
{ return csiFsfToFloat(buff,len,type); }


float csiFsfToFloat(void const *buff, uint4 len, FsfType &type)
{
   // check to make sure there is enough to work with
   if(len < 2)
   {
      type = FsfCantTell;
      return 0.0;
   }

   // check on the type of number
   type = csiFsfType(buff);
   
   // form the mantissa
   uint4 mant;
   bool isNegative = false;
   uint4 decPlace;
   bool adjust = false;
   uint2 w1 = fsWord(buff);
   uint2 w2;
   
   switch(type)
   {
   case FsfLoRes:
      mant = w1&0x1FFF;
      isNegative = (w1&0x8000) != 0;
      decPlace = (w1&0x6000) >> 13;
      adjust = true;
      break;

   case FsfHiRes:
      if(len < 4)
      {
         type = FsfCantTell;
         return 0.0;
      } 
      w2 = fsWord(((Byte const *)buff) + 2);
      mant = ((w1&0x00FF)|(w2&0x0100)) << 8;
      mant |= (w2&0x00FF);
      isNegative = (w1&0x4000) != 0;
      decPlace = (w1&0x0300) >> 7;
      if((w1&0x8000) > 0)
         decPlace++;
      adjust = true;
      break;

   case FsfArrayId:
      mant = (w1&0x03FF);
      break;

   default:
      mant = 0;
      break;
   }

   // convert the integer mantissa into a floating point number and then apply
   // the adjustment
   float rtn = static_cast<float>(mant);

   for(uint4 i = 0; adjust && i < decPlace; i++)
      rtn /= 10;
   if(adjust && isNegative)
      rtn *= -1.0;
   return rtn;
} // csiFsfToDouble


double csiFsfToDouble(void const *buff, uint4 buffLen, uint4 &bytesUsed)
{ return csiFsfToFloat(buff,buffLen,bytesUsed); }


float csiFsfToFloat(void const *buff, uint4 buffLen, uint4 &bytesUsed)
{
   StrAsc temp;
   csiFsfToStr(temp,buff,buffLen,bytesUsed);
   return static_cast<float>(atof(temp.c_str()));
} // CsiFsfToDouble


char const *csiFsfToStr(
   StrAsc &dest,
   void const *buff,
   uint4 buffLen,
   uint4 &bytesUsed)
{
   // make sure that there are at least two bytes available
   if(buffLen < 2)
      throw MsgExcept("Not enough characters for FSF conversion");

   // what kind of value are we dealing with?
   long val;
   bool isNegative;
   UInt4 decPlace;
   bool adjust = false;
   unsigned short w1 = fsWord(buff);
   const uint2 lowResMask = 0x1C00;
   const uint2 highResMask = 0x3C00;
   const uint2 idMask = 0xFC00;
   const uint2 dummyMask = 0x7F00;

   if((w1&lowResMask) != lowResMask)
   {
      bytesUsed = 2;
      val = w1&0x1FFF;
      isNegative = (w1&0x8000) != 0;
      decPlace = (w1&0x6000) >> 13;
      adjust = true;
   } // low resolution value
   else if((w1&highResMask) != highResMask)
   {
      // make sure that there are another 2 bytes
      if(buffLen < 4)
         throw MsgExcept("Not enough characters for FSF conversion");
      unsigned short w2 = fsWord(((char const *)buff) + 2);

      bytesUsed = 4;
      val = (w1&0x00FF)|(w2&0x0100);
      val = (val << 8)|(w2&0x00FF);
      isNegative = (w1&0x4000) != 0;
      decPlace = (w1&0x0300) >> 7;
      if((w1&0x8000) > 0)
         decPlace++;
      adjust = true;
   } // high resolution value
   else if((w1&idMask) == idMask)
   {
      val = (w1&0x03FF);
      char temp[10];
      sprintf(temp,"%u",static_cast<uint4>(val));
      dest = temp;
      bytesUsed = 2;
   } // array ID
   else if((w1&0xFF00) == dummyMask)
   {
      bytesUsed = 2;
      dest = "";
   } // dummy value
   else
      throw MsgExcept("Need first two bytes of a four byte value");

   // now place the sign and the decimal point in the string
   if(adjust == true)
   {
      // convert the integer value
      char temp[10];
      sprintf(temp,"%u",val);
      dest = temp;
        
      // we need to pad the beginning of the string with zeroes if the 
      // decimal point is greater than the length of the string
      while(dest.length() < decPlace)
         dest.insert("0",0);
        
      // place the decimal point where designated
      if(decPlace > 0)
         dest.insert(".",dest.length() - decPlace);
        
      // place the sign of the number
      if(isNegative)
         dest.insert("-",0);
   } // format the string
   return dest.c_str();
} // csiFsfToStr


char const *csiFloatToString(
   StrAsc &dest,
   double value,
   uint4 precision,
   bool quote_nan,
   uint4 decimal_places,
   bool pad_for_decimal_places,
   bool specials_as_numbers)
{
   Csi::OStrAscStream temp;
   csiFloatToStream(
      temp,
      value,
      precision,
      quote_nan,
      decimal_places,
      pad_for_decimal_places,
      specials_as_numbers,
      true);
   dest = temp.str();
   return dest.c_str();
} // csiFloatToString


namespace
{
   template<class stream, class scratch_type>
   void csiFloatToStreamImpl(
      stream &out,
      double value,
      uint4 precision,
      bool quote_nan,
      uint4 decimal_places,
      bool pad_for_decimal_places,
      bool specials_as_numbers,
      bool optimise_without_locale,
      scratch_type *scratch_)
   {
#ifndef _WIN32
      uint8 const neg_nan_min = 0xFFF0000000000001LL;
      uint8 const neg_nan_max = 0xFFFFFFFFFFFFFFFFLL;
      uint8 const pos_nan_min = 0x7FF0000000000001LL;
      uint8 const pos_nan_max = 0x7FFFFFFFFFFFFFFFLL;
      uint8 const pos_inf_val = 0x7FF0000000000000LL;
      uint8 const neg_inf_val = 0xFFF0000000000000LL;
#else
      uint8 const neg_nan_min = 0xFFF0000000000001;
      uint8 const neg_nan_max = 0xFFFFFFFFFFFFFFFF;
      uint8 const pos_nan_min = 0x7FF0000000000001;
      uint8 const pos_nan_max = 0x7FFFFFFFFFFFFFFF;
      uint8 const pos_inf_val = 0x7FF0000000000000;
      uint8 const neg_inf_val = 0xFFF0000000000000;
#endif
      union
      {
         double fval;
         uint8 ival;
      } uv;
      
      uv.fval = value;
      if((uv.ival >= neg_nan_min && uv.ival <= neg_nan_max) ||
         (uv.ival >= pos_nan_min && uv.ival <= pos_nan_max))
      {
         if(!specials_as_numbers)
         {
            if(quote_nan)
               out << "\"";
            out << "NAN";
            if(quote_nan)
               out <<  "\"";
         }
         else
            out << "-7999";
      }
      else if(uv.ival == pos_inf_val)
      {
         if(!specials_as_numbers)
         {
            if(quote_nan)
               out << "\"";
            out << "INF";
            if(quote_nan)
               out << "\"";
         }
         else
            out << "7999";
      }
      else if(uv.ival == neg_inf_val)
      {
         if(!specials_as_numbers)
         {
            if(quote_nan)
               out << "\"";
            out << "-INF";
            if(quote_nan)
               out << "\"";
         }
         else
            out << "-7999";
      }
      else
      {
         if(pad_for_decimal_places)
         {
            // make sure the scratch stream is created
            scratch_type *scratch = scratch_;
            if(scratch == 0)
            {
               scratch = new scratch_type;
               scratch->imbue(out.getloc());
            }
            scratch->str("");
            scratch->clear();
            
            // we also need to use a number punctuation facet
            typedef std::numpunct<char> punct_type;
            punct_type const &punct = std::use_facet<punct_type>(out.getloc());
            
            // we will format for the total number of decimal places and then evaluate whether that
            // violates the total number of allowed significant digits
            uint4 total_sig;
            typename scratch_type::string_type &dest = scratch->str();
            
            if(decimal_places > 0)
            {
               (*scratch) << std::fixed
                          << std::showpoint
                          << std::setprecision(decimal_places)
                          << value;
            }
            else
            {
               (*scratch) << std::fixed
                          << std::noshowpoint
                          << std::setprecision(0)
                          << value;
            }
            total_sig = (uint4)dest.length();
            
            // we need to calculate the total number of significant digits in the formatted string.  To
            // do this, we will use the length of the string and then subtract from that for the sign
            // and any thousands separators.
            if(dest.first() == '-')
               --total_sig;
            for(size_t i = 0; i < dest.length(); ++i)
            {
               if(dest[i] == punct.thousands_sep() || dest[i] == punct.decimal_point())
                  --total_sig;
            }
            
            // if there are too many significant digits, we will reformat the value using scientific notation
            if(total_sig > precision + 1)
            {
               dest.cut(0);
               (*scratch) << std::scientific
                          << std::noshowpoint
                          << std::setprecision(precision)
                          << value;
            }
            if(dest.length() > 0 && dest.first() == '-')
            {
               bool neg_zero = true;
               for(size_t i = 1; neg_zero && i < dest.length(); ++i)
                  if(dest[i] != '0' &&
                     dest[i] != punct.decimal_point() &&
                     dest[i] != punct.thousands_sep())
                     neg_zero = false;
               if(neg_zero)
                  dest.cut(0, 1); 
            }
            out << dest;
            if(scratch != scratch_)
               delete scratch;
         }
         else
         {
            if(optimise_without_locale)
            {
               char temp[52];
               sprintf(temp,"%.*G",precision,value);
               out << temp;
            }
            else
            {
               out.unsetf(std::ios::fixed);
               out.unsetf(std::ios::scientific);
               out << std::setprecision(precision) << value;
            }
         }
      }
   } // csiFloatToStreamImpl
};


void csiFloatToStream(
   std::ostream &out,
   double value,
   uint4 precision,
   bool quote_nan,
   uint4 decimal_places,
   bool pad_for_decimal_places,
   bool specials_as_numbers,
   bool optimise_without_locale,
   Csi::OStrAscStream *scratch)
{
   csiFloatToStreamImpl(
      out,
      value,
      precision,
      quote_nan,
      decimal_places,
      pad_for_decimal_places,
      specials_as_numbers,
      optimise_without_locale,
      scratch);
} // csiFloatToStream


void csiFloatToStream(
   std::wostream &out,
   double value,
   uint4 precision,
   bool quote_nan,
   uint4 decimal_places,
   bool pad_for_decimal_places,
   bool specials_as_numbers,
   bool optimise_without_locale,
   Csi::OStrUniStream *scratch)
{
   csiFloatToStreamImpl(
      out,
      value,
      precision,
      quote_nan,
      decimal_places,
      pad_for_decimal_places,
      specials_as_numbers,
      optimise_without_locale,
      scratch);
} // csiFloatToStream


double csiStringToFloat(
   char const *s, std::locale locale, bool check_results)
{
   double rtn = 0;
   size_t s_len = strlen(s);
   if(s_len > 0)
   {
      // we want to try to make the conversion immediately and only look for the other string values if
      // the conversion fails.
      Csi::IBuffStream temp(s, s_len);

      temp.imbue(locale);
      temp >> rtn;
      if(!temp)
      {
         StrAsc val_str(s);
      
         if(val_str.find("-inf") < val_str.length())
            rtn = -1.0 * std::numeric_limits<double>::infinity();
         else if(val_str.find("inf") < val_str.length())
            rtn = std::numeric_limits<double>::infinity();
         else if(!check_results || val_str.find("nan") < val_str.length())
            rtn = std::numeric_limits<double>::quiet_NaN();
         else
            throw std::invalid_argument("invalid floating point conversion");
      }
   }
   return rtn;
} // csiStringToFloat


double csiStringToFloat(
   wchar_t const *s, std::locale locale, bool check_results)
{
   double rtn = 0;
   size_t s_len = wcslen(s);
   if(s_len > 0)
   {
      // we want to try to make the conversion immediately and only look for the other string values if
      // the conversion fails.
      Csi::IBuffStreamw temp(s, s_len);

      temp.imbue(locale);
      temp >> rtn;
      if(!temp)
      {
         StrUni val_str(s);
      
         if(val_str.find(L"-inf") < val_str.length())
            rtn = -1.0 * std::numeric_limits<double>::infinity();
         else if(val_str.find(L"inf") < val_str.length())
            rtn = std::numeric_limits<double>::infinity();
         else if(!check_results || val_str.find(L"nan") < val_str.length())
            rtn = std::numeric_limits<double>::quiet_NaN();
         else
            throw std::invalid_argument("invalid floating point conversion");
      }
   }
   return rtn;
} // csiStringToFloat
