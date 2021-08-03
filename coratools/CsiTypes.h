/* CsiTypes.h

   Copyright (C) 1998, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 21 February 1997
   Last Change: Tuesday 10 July 2012
   Last Commit: $Date: 2012-07-10 16:10:30 -0600 (Tue, 10 Jul 2012) $ 
   Commited by: $Author: jon $
   
*/

#ifndef CsiTypes_h
#define CsiTypes_h

#include "Csi.LgrDate.h"
#include "Csi.StrAscStream.h"
#include "Csi.StrUniStream.h"
#include "Csi.StringLoader.h"


////////////////////////////////////////////////////////////
// enum CsiDbTypeCode
// 
// Codes for all possible types of data that can be stored in CSI dataloggers
// and the CsiLgrNet database
////////////////////////////////////////////////////////////
enum CsiDbTypeCode
{
   CsiUnknown    =  0,
   CsiUInt1      =  1,         // 1 byte unsigned int
   CsiUInt2      =  2,         // 2 byte unsigned int
   CsiUInt4      =  3,         // 4 byte unsigned int
   CsiInt1       =  4,         // 1 byte signed int
   CsiInt2       =  5,         // 2 byte signed int
   CsiInt4       =  6,         // 4 byte signed int
   CsiFs2        =  7,         // 2 byte final storage
   CsiFs3        = 15,         // 3 byte final storage
   CsiFs4        = 26,         // 4 byte final storage
   CsiFsf        = 27,         // allows storage of either CsiFs2 or CsiFs4. Requires 4 bytes
   CsiFp4        =  8,         // 4 byte CSI float
   CsiIeee4      =  9,         // 4 byte IEEE float
   CsiIeee8      = 18,         // 8 byte IEEE float
   CsiBool       = 10,         // 1 byte boolean (0 or 1)
   CsiBool8      = 17,         // 1 byte bit field
   CsiSec        = 12,         // 4 byte sec since 1 Jan 1990
   CsiUSec       = 13,         // 6 byte 10s of usec since 1 Jan 1990
   CsiNSec       = 14,         // 4 byte sec since 1 Jan 1990 + 4 byte nsec
   CsiAscii      = 11,         // fixed length string
   CsiAsciiZ     = 16,         // variable length nul-terminated string
   CsiInt2Lsf    = 19,         // 2 byte signed int (LSB first)
   CsiInt4Lsf    = 20,         // 4 byte signed int (LSB first)
   CsiUInt2Lsf   = 21,         // 2 byte signed int (LSB first)
   CsiUInt4Lsf   = 22,         // 4 byte signed int (LSB first)
   CsiNSecLsf    = 23,         // same as NSec with the components in LSB
   CsiIeee4Lsf   = 24,         // 4 byte IEEE float (LSB first)
   CsiIeee8Lsf   = 25,         // 8 byte IEEE float (LSB first)
   CsiInt8       = 32,         // 8 byte signed integer
   CsiInt8Lsf    = 33,         // 8 byte signed integer (LSB first)
   CsiLgrDate    = 29,         // 8 byte nanoseconds since 1 jan 1990
   CsiLgrDateLsf = 28,         // 8 byte nanoseconds since 1 jan 1990 (LSB first)
   CsiBool2      = 30,         // 2 byte boolean value
   CsiBool4      = 31,         // 4 byte boolean value
};


////////////////////////////////////////////////////////////
// enum CsiLgrTypeCode
////////////////////////////////////////////////////////////
enum CsiLgrTypeCode
{
   LgrUnknown = 0,              // unknown type
   LgrUInt1 = 1,                // 1 byte unsigned int
   LgrUInt2 = 2,                // 2 byte unsigned int
   LgrUInt4 = 3,                // 4 byte unsigned int
   LgrInt1 =  4,                // 1 byte signed int
   LgrInt2 =  5,                // 2 byte signed int
   LgrInt4 =  6,                // 4 byte signed int
   LgrFs2 =   7,                // 2 byte final storage
   LgrFs3 =   15,               // 3 byte final storage
   LgrFs4 =   26,               // 4 byte final storage
   LgrFp4 =   8,                // 4 byte CSI float
   LgrIeee4 = 9,                // 4 byte IEEE float
   LgrIeee8 = 18,               // 8 byte IEEE float
   LgrBool =  10,               // 1 byte boolean (0 or 1)
   LgrBool8 = 17,               // 1 byte bit field
   LgrSec =   12,               // 4 byte sec since 1 Jan 1990
   LgrUSec =  13,               // 6 byte 10s of usec since 1 Jan 1990
   LgrNSec =  14,               // 4 byte sec since 1 Jan 1990 + 4 byte nsec
   LgrAscii = 11,               // fixed length string
   LgrAsciiZ = 16,              // variable length nul-terminated string
   LgrInt2Lsf =  19,            // 2 byte signed int (LSB first)
   LgrInt4Lsf = 20,             // 4 byte signed int (LSB first)
   LgrUInt2Lsf = 21,            // 2 byte signed int (LSB first)
   LgrUInt4Lsf = 22,            // 4 byte signed int (LSB first)
   LgrNSecLsf = 23,             // same as NSec with the components in LSB
   LgrIeee4Lsf = 24,            // 4 byte IEEE float (LSB first)
   LgrIeee8Lsf = 25,            // 8 byte IEEE float (LSB first)
   LgrBool2 = 27,               // 2 byte boolean value
   LgrBool4 = 28,               // 4 byte boolean value
};


////////////////////////////////////////////////////////////
// csi_to_lgr_type
//
// Converts a CsiDbTypeCode to a logger type code
////////////////////////////////////////////////////////////
CsiLgrTypeCode csi_to_lgr_type(CsiDbTypeCode type);


////////////////////////////////////////////////////////////
// lgr_to_csi_type
//
// Converts a CsiLgrTypeCode to a CsiDbTypeCode value.
////////////////////////////////////////////////////////////
CsiDbTypeCode lgr_to_csi_type(CsiLgrTypeCode type);


////////////////////////////////////////////////////////////
// csiTypeLen
//
// Looks up the number of bytes required to store a CSI database type identified by the
// typeId parameter 
////////////////////////////////////////////////////////////
uint4 csiTypeLen(CsiDbTypeCode typeId);


////////////////////////////////////////////////////////////
// csiTypeLenBits
//
// Looks up the number of bits required to store a CSI database type identified
// by the typeId parameter
////////////////////////////////////////////////////////////
uint4 csiTypeLenBits(CsiDbTypeCode type_id);


////////////////////////////////////////////////////////////
// isCsiDbTypeCode
//
// Checks to see if the type parameter specified is a CSI database type code
////////////////////////////////////////////////////////////
bool isCsiDbTypeCode(uint4 type);


////////////////////////////////////////////////////////////
// csiTypeName
//
// Returns the name associated with the type code
////////////////////////////////////////////////////////////
char const *csiTypeName(CsiDbTypeCode typeId);

////////////////////////////////////////////////////////////
// lgrTypeName
////////////////////////////////////////////////////////////
char const *lgrTypeName(CsiLgrTypeCode type_id); 

////////////////////////////////////////////////////////////
// tob_type_string
//
// Returns a TOB type string for the specified type or a null pointer if the
// type is not supported by TOB.
////////////////////////////////////////////////////////////
char const *tob_type_string(CsiDbTypeCode type_id);

////////////////////////////////////////////////////////////
// parse_tob_data_type
//
// Parses a TOB type declaration into a data type code and a string length.  
////////////////////////////////////////////////////////////
void parse_tob_data_type(
   char const *type_name,
   CsiLgrTypeCode &data_type,
   uint4 &string_len);

////////////////////////////////////////////////////////////
// xml_type_string
//
// Returns the XML type string associated with the specified type code. 
////////////////////////////////////////////////////////////
char const *xml_type_string(CsiDbTypeCode type_id);


//// The following functions convert CSI defined data types into data types
// that can be manipulated easily on the computer.

////////////////////////////////////////////////////////////
// csiInt1ToInt1
////////////////////////////////////////////////////////////
int1 csiInt1ToInt1(void const *buff);


////////////////////////////////////////////////////////////
// csiInt2ToInt2
////////////////////////////////////////////////////////////
uint1 csiUInt1ToUInt1(void const *buff);


////////////////////////////////////////////////////////////
// csiInt2ToInt2
////////////////////////////////////////////////////////////
int2 csiInt2ToInt2(void const *buff);


////////////////////////////////////////////////////////////
// csiInt2LsfToInt2
////////////////////////////////////////////////////////////
int2 csiInt2LsfToInt2(void const *buff); 


////////////////////////////////////////////////////////////
// csiUInt2ToUInt2
////////////////////////////////////////////////////////////
uint2 csiUInt2ToUInt2(void const *buff);


////////////////////////////////////////////////////////////
// csiUInt2LsfToUInt2
////////////////////////////////////////////////////////////
uint2 csiUInt2LsfToUInt2(void const *buff); 


////////////////////////////////////////////////////////////
// csiInt4ToInt4
////////////////////////////////////////////////////////////
int4 csiInt4ToInt4(void const *buff);


////////////////////////////////////////////////////////////
// csiInt4LsfToInt4
////////////////////////////////////////////////////////////
int4 csiInt4LsfToInt4(void const *buff); 


////////////////////////////////////////////////////////////
// csiUInt4ToUInt4
////////////////////////////////////////////////////////////
uint4 csiUInt4ToUInt4(void const *buff);


////////////////////////////////////////////////////////////
// csiUInt4LsfToUInt4
////////////////////////////////////////////////////////////
uint4 csiUInt4LsfToUInt4(void const *buff);


////////////////////////////////////////////////////////////
// csiIeee4ToFloat
////////////////////////////////////////////////////////////
float csiIeee4ToFloat(void const *buff);


////////////////////////////////////////////////////////////
// csiIeee4LsfToFloat
////////////////////////////////////////////////////////////
float csiIeee4LsfToFloat(void const *buff);


////////////////////////////////////////////////////////////
// csiIeee8ToDouble
////////////////////////////////////////////////////////////
double csiIeee8ToDouble(void const *buff);


////////////////////////////////////////////////////////////
// csiIeee8LsfToDouble
////////////////////////////////////////////////////////////
double csiIeee8LsfToDouble(void const *buff);


////////////////////////////////////////////////////////////
// csiInt8LsfToInt8
////////////////////////////////////////////////////////////
int8 csiInt8LsfToInt8(void const *buff);


////////////////////////////////////////////////////////////
// csiBoolToBool
////////////////////////////////////////////////////////////
bool csiBoolToBool(void const *buff);


////////////////////////////////////////////////////////////
// csiBool8ToUInt1
////////////////////////////////////////////////////////////
uint1 csiBool8ToUInt1(void const *buff);


////////////////////////////////////////////////////////////
// csiSecToLgrDate
////////////////////////////////////////////////////////////
Csi::LgrDate csiSecToLgrDate(void const *buff);


////////////////////////////////////////////////////////////
// csiUSecToLgrDate
////////////////////////////////////////////////////////////
Csi::LgrDate csiUSecToLgrDate(void const *buff);


////////////////////////////////////////////////////////////
// csiNSecToLgrDate
////////////////////////////////////////////////////////////
Csi::LgrDate csiNSecToLgrDate(void const *buff);


////////////////////////////////////////////////////////////
// csiNSecLsfToLgrDate
////////////////////////////////////////////////////////////
Csi::LgrDate csiNSecLsfToLgrDate(void const *buff);


////////////////////////////////////////////////////////////
// csiFs2ToString
//
// Converts a two byte final storage format number directly into a string. This
// is the preferred method of formatting since it will have the fewest
// conversion artifacts.
////////////////////////////////////////////////////////////
char const *csiFs2ToString(StrAsc &dest, void const *buffer);


////////////////////////////////////////////////////////////
// csiFs2ToFloat
//
// Converts a CSI 2 byte final storage number to Float
////////////////////////////////////////////////////////////
float csiFs2ToFloat(void const *buffer);


////////////////////////////////////////////////////////////
// csiFs2ToDouble
//
// Converts a CSI 2 byte floating point to double
////////////////////////////////////////////////////////////
double csiFs2ToDouble(void const *buff);


////////////////////////////////////////////////////////////
// doubleToCsiFs2
//
// Converts a double to a CSI two byte floating point value.  Assumes that the
// provided buffer is at least two bytes long.
////////////////////////////////////////////////////////////
void doubleToCsiFs2(void *buff, double value);


////////////////////////////////////////////////////////////
// csiFs3ToString
//
// Converts the CSI 3 byte final storage format value to a string.
////////////////////////////////////////////////////////////
char const *csiFs3ToString(StrAsc &dest, void const *buff);


////////////////////////////////////////////////////////////
// csiFs3ToFloat
//
// Converts a CSI 3 byte final storage number to a float (4 byte IEEE)
////////////////////////////////////////////////////////////
float csiFs3ToFloat(void const *buffer);


////////////////////////////////////////////////////////////
// csiFs3ToDouble
//
// Converts a csi 3 byte floating point number to a double value.
////////////////////////////////////////////////////////////
double csiFs3ToDouble(void const *buff);


////////////////////////////////////////////////////////////
// csiFp4ToDouble
//
// Converts the csi 4 byte floating point number to an IEEE 8 byte floating
// point number. The length of the buffer being passed is assumed to be equal
// to or greater than four bytes. Note that the greatest absolute value for the
// csiFP4 value is 6999.0
////////////////////////////////////////////////////////////
double csiFp4ToDouble(const void *buff);


////////////////////////////////////////////////////////////
// doubleToCsiFp4
//
// Converts an IEEE8 floating point number passed as the second parameter to an
// FP4 value passed in the first parameter as a pointer to void. The buffer
// being passed in is assumed to have storage allocated for four or more bytes.
////////////////////////////////////////////////////////////
void doubleToCsiFp4(void *buff, double val);


////////////////////////////////////////////////////////////
// csiFp4ToFloat
//
// Converts the CSI 4 byte floating point number to an IEEE 4 byte floating
// point number. The length of the buffer being passed is assumed to be greater
// than or equal to four bytes.
////////////////////////////////////////////////////////////
float csiFp4ToFloat(void const *buffer);


////////////////////////////////////////////////////////////
// floatToCsiFp4
//
// Converts an IEEE 4 byte floating point number to a CSI 4 byte floating point
// number. The converted value will be written to the supplied buffer the
// length of which is assumed to be greater than or equal to four. This
// function will throw a std::out_of_range exception if the conversion is not
// supported.
////////////////////////////////////////////////////////////
void floatToCsiFp4(void *buffer, float value);


////////////////////////////////////////////////////////////
// csiFp4ToString
//
// Converts a CSI 4 byte floating point number into a string
////////////////////////////////////////////////////////////
char const *csiFp4ToString(StrAsc &dest, void const *buffer);


////////////////////////////////////////////////////////////
// csiFsfType
//
// Returns the type of final storage object there is at the beginning of the
// buff parameter by examining the first two bytes of that object 
////////////////////////////////////////////////////////////
enum FsfType
{
   FsfLoRes = 1,                // low resolution (two bytes)
   FsfHiRes = 2,                // high resolution (four bytes)
   FsfArrayId = 3,              // array ID
   FsfDummy = 4,                // dummy value
   FsfHiResScndWord = 5,        // second word of high resolution
   FsfCantTell = 6,             // not enough bytes to tell
   FsfBeginAscii = 7,           // begin of ASCII data
   FsfEndByteMode = 8,          // end of byte mode data
};
FsfType csiFsfType(void const *buff);


////////////////////////////////////////////////////////////
// csiFsfToFloat
//
// Converts a CSI final storage format object to a float and returns the type
// of object converted in the type parameter. The caller should check the value
// of that type to see if the conversion succeeded.
////////////////////////////////////////////////////////////
float csiFsfToFloat(void const *buffer, uint4 buffer_len, FsfType &type);


////////////////////////////////////////////////////////////
// csiFsfToDouble
//
// Converts a CSI final storage format object to an eight byte IEEE number and
// returns the type of object as a reference parameter. The caller should check
// this type before accepting the return value as it indicates the success of
// the conversion.
////////////////////////////////////////////////////////////
double csiFsfToDouble(void const *buff, uint4 len, FsfType &type);


////////////////////////////////////////////////////////////
// csiFsfToDouble
//
// Converts a csi 2 byte or 4 byte final storage value to a double. Returns the
// number of bytes that were read in order to make the conversion in the
// bytesUsed parameter. Will throw an MsgExcept if the conversion is impossible
////////////////////////////////////////////////////////////
double csiFsfToDouble(void const *buff, uint4 buffLen, uint4 &bytesUsed);


////////////////////////////////////////////////////////////
// csiFsfToFloat
//
// Converts a CSI 2 byte or 4 byte final storage value into an IEEE4
// value. Returns the number of bytes that were read in order to make the
// conversion in the bytes_used parameter. Will throw a MsgExcept object if the
// conversion is not possible
////////////////////////////////////////////////////////////
float csiFsfToFloat(void const *buffer, uint4 buffer_len, uint4 &bytes_used);


////////////////////////////////////////////////////////////
// csiFsfToStr
//
// Converts a csi 2 byte or 4 byte final storage value into a string. Returns
// the number of bytes that were used in the bytesUsed parameter. Will throw an
// MsgExcept if the conversion is impossible.
////////////////////////////////////////////////////////////
char const *csiFsfToStr(
   StrAsc &dest,
   void const *buff,
   uint4 buffLen,
   uint4 &bytesUsed);


////////////////////////////////////////////////////////////
// csiFloatToString
//
// Formats the floating point value according to the following rules:
//
//   - The value will be formatted with the fixed number of decimal places
//     specified in the decimal_places parameter unless doing so causes the
//     value to have more than precision significant digits.  If this is the
//     case, the value will be formatted in exponential notation if its
//     absolute value is less than 10e-4 or if the absolute value is greater
//     than or equal to 10*10^precision.  If the resulting value is not in
//     exponential notation, the value of pad_for_decimal_places is true, and
//     the number of digits following the decimal point is less than
//     decimal_places, zeroes will be padded to the resulting string.
//
//   - All values will be formatted to the specified precision and any zero
//     characters trailing the decimal point will be removed unless
//     pad_for_decimal_places is true.
//
//   - If the bit pattern indicates a value that is not a number, the value
//     will be formatted as "NAN".  All possible NAN values will recognised.
//     This string will appear in quotes if quote_nan is true. 
//
//   - If the bit pattern indicates a value that is either positive or
//     negative infinity, the value will be formatted as "INF" or
//     "-INF". These strings will appear in quotes if the quote_nan parameter
//     is set to true.
//
//   - The above rules for Nan and +- INF will not apply if the
//     specials_as_numbers flag is set.  In this case, the value will be
//     formatted as "7999" (for Nan and +Inf) or "-7999" (for -Inf)
////////////////////////////////////////////////////////////
char const *csiFloatToString(
   StrAsc &dest,
   double value,
   uint4 precision = 7,
   bool quote_nan = false,
   uint4 decimal_places = 7,
   bool pad_for_decimal_places = false,
   bool specials_as_numbers = false);


////////////////////////////////////////////////////////////
// csiFloatToStream
//
// Formats the floating point value according to the same rules used in
// csiFloatToString().  The output is instead written to an ostream object and
// will use the locale information associated with that stream.  The parameters
// have the following meanings:
//
//  dest: specifies the stream to which the value will be formatted
//
//  precision: specifies the maximum number of significant digits that should
//             be used
//
//  quote_nan: Specifies that if a NaN, Inf, or -Inf value needs to be
//             formatted, that it should be formatted in side of quotations.
//
//  pad_for_decimal_places: Can be set to true to indicate that fixed point
//             formatting should be used.
//
//  specials_as_numbers:  Can be set to true to indicate that "special" values
//  (Inf, NaN, & -Nan) should be formatted with numeric values rather than the
//  special strings.
//
//  optimise_without_locale: Can be set to true to force the value to be
//  formatted using sprintf() rather than using c++ stream I/O.  This can give
//  significant advantage when formatting values in a type loop.
//
//  scratch:  Specifies a "scratch" stream object that will be used when
//  pad_for_decimal_places is set to true.  By passing this in the application
//  can avoid the overhead of the creation of a temporary stream object.  
////////////////////////////////////////////////////////////
void csiFloatToStream(
   std::ostream &dest,
   double value,
   uint4 precision = 7,
   bool quote_nan = false,
   uint4 decimal_places = 7,
   bool pad_for_decimal_places = false,
   bool specials_as_numbers = false,
   bool optimise_without_locale = false,
   Csi::OStrAscStream *scratch = 0);
void csiFloatToStream(
   std::wostream &dest,
   double value,
   uint4 precision = 7,
   bool quote_nan = false,
   uint4 decimal_places = 7,
   bool pad_for_decimal_places = false,
   bool specials_as_numbers = false,
   bool optimise_without_locale = false,
   Csi::OStrUniStream *scratch = 0);

////////////////////////////////////////////////////////////
// csiStringToFloat
//
// Converts a string to a floating point value using the same conventions as
// csiFloatToString().  Recognises the special strings for infinite and NAN
// values. 
////////////////////////////////////////////////////////////
double csiStringToFloat(
   char const *s,
   std::locale locale = Csi::StringLoader::make_locale(),
   bool check_results = false);
double csiStringToFloat(
   wchar_t const *s,
   std::locale locale = Csi::StringLoader::make_locale(),
   bool check_results = false);



#endif
