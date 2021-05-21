/* Csi.FloatUtils.h

   Copyright (C) 2008, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 28 October 2008
   Last Change: Friday 31 July 2015
   Last Commit: $Date: 2015-07-31 10:54:17 -0600 (Fri, 31 Jul 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_FloatUtils_h
#define Csi_FloatUtils_h

#include "CsiTypeDefs.h"
#include <limits>
#include <math.h>


#ifndef _WIN32
#define double_neg_nan_min 0xFFF0000000000001LL
#define double_neg_nan_max 0xFFFFFFFFFFFFFFFFLL
#define double_pos_nan_min 0x7FF0000000000001LL
#define double_pos_nan_max 0x7FFFFFFFFFFFFFFFLL
#define double_pos_inf_val 0x7FF0000000000000LL
#define double_neg_inf_val 0xFFF0000000000000LL
#else
#define double_neg_nan_min 0xFFF0000000000001
#define double_neg_nan_max 0xFFFFFFFFFFFFFFFF
#define double_pos_nan_min 0x7FF0000000000001
#define double_pos_nan_max 0x7FFFFFFFFFFFFFFF
#define double_pos_inf_val 0x7FF0000000000000
#define double_neg_inf_val 0xFFF0000000000000
#endif
#define float_neg_nan_min 0xFF800001L
#define float_neg_nan_max 0xFFBFFFFFL
#define float_pos_nan_min 0x7F800001L
#define float_pos_nan_max 0x7FBFFFFFL


namespace Csi
{
   /**
    * @return Returns true if the specified value is in the range of NaN values
    * (quiet or signalling).
    *
    * @param value  Specifies the value to evaluate.
    */
   inline bool is_signalling_nan(double value)
   {
      union
      {
         double fval;
         uint8 ival;
      } uv;
      bool rtn = false;

      uv.fval = value;
      if((uv.ival >= double_neg_nan_min && uv.ival <= double_neg_nan_max) ||
         (uv.ival >= double_pos_nan_min && uv.ival <= double_pos_nan_max))
         rtn = true;
      return rtn;
   }

   
   inline bool is_signalling_nan(float value)
   {
      bool rtn = false;
      union
      {
         float fval;
         uint4 ival;
      } uv;
      uv.fval = value;
      if((uv.ival >= float_neg_nan_min && uv.ival <= float_neg_nan_max) ||
         (uv.ival >= float_pos_nan_min && uv.ival <= float_pos_nan_max))
         rtn = true;
      return rtn;
   }


   /**
    * @return Returns true if the specified value is considered finite.  A
    * value of false will be returned for any Inf, -Inf, or NaN value.
    *
    * @param value  Specifies the value to evaluate.
    */
   inline bool is_finite(double value)
   {
      union
      {
         double fval;
         uint8 ival;
      } uv;
      bool rtn = true;
      
      uv.fval = value;
      if((uv.ival >= double_neg_nan_min && uv.ival <= double_neg_nan_max) ||
         (uv.ival >= double_pos_nan_min && uv.ival <= double_pos_nan_max) ||
         (uv.ival == double_pos_inf_val) ||
         (uv.ival == double_neg_inf_val))
         rtn = false;
      return rtn; 
   }


   /**
    * @return Returns true if the specified value represents negative infinity.
    *
    * @param value  Specifies the value to evaluate.
    */
   inline bool is_neg_inf(double value)
   {
      union
      {
         double fval;
         uint8 ival;
      } uv;
      bool rtn(false);
      
      uv.fval = value;
      if(uv.ival == double_neg_inf_val)
         rtn = true;
      return rtn; 
   }


   /**
    * @return Returns true if the specified value represents positive infinity.
    *
    * @param value  Specifies the value to evaluate.
    */
   inline bool is_pos_inf(double value)
   {
      union
      {
         double fval;
         uint8 ival;
      } uv;
      bool rtn(false);
      
      uv.fval = value;
      if(uv.ival == double_pos_inf_val)
         rtn = true;
      return rtn; 
   }


   /**
    * @return Converts the specified set of bytes to an IEEE4 value.
    *
    * @param s_  Specifies the storage to be converted.  This is assumed to be
    * four bytes long.
    *
    * @param reverse  Specifies that the storage is in opposite of the host
    * machine order.
    */
   inline float build_float(void const *s_, bool reverse)
   {
      byte const *s = static_cast<byte const *>(s_);
      union
      {
         uint4 ival;
         float fval;
         byte sval[4];
      } uv;
      if(!reverse)
      {
         uv.sval[0] = s[0];
         uv.sval[1] = s[1];
         uv.sval[2] = s[2];
         uv.sval[3] = s[3];
      }
      else
      {
         uv.sval[0] = s[3];
         uv.sval[1] = s[2];
         uv.sval[2] = s[1];
         uv.sval[3] = s[0];
      }
      if((uv.ival >= float_neg_nan_min && uv.ival <= float_neg_nan_max) ||
         (uv.ival >= float_pos_nan_min && uv.ival <= float_pos_nan_max))
         uv.fval = std::numeric_limits<float>::quiet_NaN();
      return uv.fval;
   } // build_float


   /**
    * @return Returns the double value represented by the specified memory
    * location.
    *
    * @param s_  Specifies the memory storage location for the value.  This is
    * assumed to be eight bytes in length.
    *
    * @param reverse  Set to true if the storage is in the byte order opposite
    * that of the host machine.
    */
   inline double build_double(void const *s_, bool reverse)
   {
      byte const *s = static_cast<byte const *>(s_);
      union
      {
         uint8 ival;
         double fval;
         byte sval[8];
      } uv;
      if(!reverse)
      {
         uv.sval[0] = s[0];
         uv.sval[1] = s[1];
         uv.sval[2] = s[2];
         uv.sval[3] = s[3];
         uv.sval[4] = s[4];
         uv.sval[5] = s[5];
         uv.sval[6] = s[6];
         uv.sval[7] = s[7];
      }
      else
      {
         uv.sval[0] = s[7];
         uv.sval[1] = s[6];
         uv.sval[2] = s[5];
         uv.sval[3] = s[4];
         uv.sval[4] = s[3];
         uv.sval[5] = s[2];
         uv.sval[6] = s[1];
         uv.sval[7] = s[0];
      }
      if((uv.ival >= double_neg_nan_min && uv.ival <= double_neg_nan_max) ||
         (uv.ival >= double_pos_nan_min && uv.ival <= double_pos_nan_max))
         uv.fval = std::numeric_limits<double>::quiet_NaN();
      return uv.fval;
   }


   /**
    * @return Returns the specified value rounded to the specified number of
    * decimal places.
    *
    * @param val  Specifies the value to round.
    *
    * @param deci  Specifies the number of decimal places to round by.
    */
   double round(double val, int deci);


   /**
    * @return Returns 0 if the two values are equal (within the number of
    * decimal places specified by deci), a positive value if the two v1 is
    * greater than v2, and a negative value if v1 is less than v2.
    *
    * @param v1  Specifies the first value to compare.
    *
    * @param v2 Specifies the second value to compare.
    *
    * @param deci  Specifies the number of decimal places to compare for equality.
    *
    * @param incomparable  Returns true if the two values cannot be compared
    * (this would happen if one of the values is NaN).
    */
   int8 compare_doubles(double v1, double v2, int deci, bool &incomparable);

   /**
    * @return Returns the value of pi.
    */
   inline double pi()
   { return 3.14159265359; }
   
   
   /**
    * @return Returns the angle converted from degrees to radians.
    *
    * @param degrees Specifies the angle in degrees.
    */
   inline double degrees_to_radians(double degrees)
   {
      return degrees * pi() / 180;
   }

   /**
    * @return Returns the angle converted from radians to degrees.
    *
    * @param radians Specifies the angle in radians.
    */
   inline double radians_to_degrees(double radians)
   {
      return (radians * 180) / pi();
   }

   /**
    * @return Returns the specified value rounded to the nearest integer.
    *
    * @param value Specifies the floating point value to round.
    */
   inline int round_double(double value)
   {
      double int_part(0);
      double fract_part(modf(value, &int_part));
      int rtn;
      if(int_part >= 0)
         rtn = static_cast<int>(int_part + (fract_part > 0.5 ? 1 : 0));
      else
         rtn = static_cast<int>(int_part - (fabs(fract_part) > 0.5 ? 1 : 0));
      return rtn;
   }
};


#endif
