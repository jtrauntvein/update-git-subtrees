/* Csi.Win32.LgrDateEx.h

   Copyright (C) 2000, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 12 October 2000
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef Csi_Win32_LgrDateEx_h
#define Csi_Win32_LgrDateEx_h

#include "Csi.LgrDate.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class LgrDateEx
      //
      // Extends class LgrDate with methods that provide conversion to and from Variant date/times  and
      // other Microsoft OS specific time structures.
      //
      // This class is deprecated as the variant functions are now being provided by the
      // Csi::LgrDate class. 
      ////////////////////////////////////////////////////////////
      class LgrDateEx: public LgrDate
      {
      public:
         ////////////////////////////////////////////////////////////
         // default constructor
         //////////////////////////////////////////////////////////// 
         LgrDateEx()
         { }

         ////////////////////////////////////////////////////////////
         // construct from raw nano seconds
         //////////////////////////////////////////////////////////// 
         LgrDateEx(int8 nanoseconds_since_1990):
            LgrDate(nanoseconds_since_1990)
         { }

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         LgrDateEx(LgrDateEx const &other):
            LgrDate(other)
         { }

         ////////////////////////////////////////////////////////////
         // LgrDate copy constructor
         //
         // Allows this class to be created using a LgrDate instance
         //////////////////////////////////////////////////////////// 
         LgrDateEx(LgrDate const &other):
            LgrDate(other)
         { }

         ////////////////////////////////////////////////////////////
         // constructor from variant
         //////////////////////////////////////////////////////////// 
         LgrDateEx(double variant_time)
         { operator =(from_variant(variant_time)); }

         ////////////////////////////////////////////////////////////
         // copy operator
         //
         // Copies from a LgrDate object. Since LgrDateEx objects are convertible to LgrDate, this
         // constructor will work for all LgrDateEx objects as well.
         //////////////////////////////////////////////////////////// 
         LgrDateEx &operator =(LgrDate const &other)
         {
            LgrDate::operator =(other);
            return *this;
         }

         ////////////////////////////////////////////////////////////
         // assignment to variant
         //////////////////////////////////////////////////////////// 
         LgrDateEx &operator =(double variant_time)
         {
            operator =(from_variant(variant_time));
            return *this;
         }
      };
   };
};

#endif
