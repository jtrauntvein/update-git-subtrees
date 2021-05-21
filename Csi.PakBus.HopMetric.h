/* Csi.PakBus.HopMetric.h

   Copyright (C) 2002, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 29 March 2002
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_PakBus_HopMetric_h
#define Csi_PakBus_HopMetric_h

#include "CsiTypeDefs.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class HopMetric
      //
      // Represents an encoded hop metric that can be converted into a response time. This class
      // overloads operators that allow the class to be freely converted between a coded byte value
      // and this class.  It also provides methods that convert the coded value to and from
      // response times in milli-seconds.
      ////////////////////////////////////////////////////////////
      class HopMetric
      {
      private:
         ////////////////////////////////////////////////////////////
         // coded_value
         ////////////////////////////////////////////////////////////
         byte coded_value;
         
      public:
         ////////////////////////////////////////////////////////////
         // default constructor
         ////////////////////////////////////////////////////////////
         HopMetric(byte coded_value_ = 0):
            coded_value(coded_value_)
         { }

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         HopMetric(HopMetric const &other):
            coded_value(other.coded_value)
         { }

         ////////////////////////////////////////////////////////////
         // copy operator
         ////////////////////////////////////////////////////////////
         HopMetric &operator =(HopMetric const &other)
         { coded_value = other.coded_value; return *this; }
         HopMetric &operator =(byte coded_value_)
         { coded_value = coded_value_; return *this; }

         ////////////////////////////////////////////////////////////
         // cast to coded value
         ////////////////////////////////////////////////////////////
         operator byte() const
         { return coded_value; }

         ////////////////////////////////////////////////////////////
         // get_coded_value
         ////////////////////////////////////////////////////////////
         byte get_coded_value() const
         { return coded_value; }

         ////////////////////////////////////////////////////////////
         // set_response_time_msec
         //
         // Sets the encoded value appropriately based upon the range that the response time
         // occupies. 
         ////////////////////////////////////////////////////////////
         void set_response_time_msec(uint4 response_time_msec); 

         ////////////////////////////////////////////////////////////
         // get_response_time_msec
         //
         // Converts the encoded value to a response time in milli-seconds
         ////////////////////////////////////////////////////////////
         uint4 get_response_time_msec() const;

         //@group comparison operators
         ////////////////////////////////////////////////////////////
         // equality
         ////////////////////////////////////////////////////////////
         bool operator ==(HopMetric const &other) const
         { return coded_value == other.coded_value; }

         ////////////////////////////////////////////////////////////
         // inequality
         ////////////////////////////////////////////////////////////
         bool operator !=(HopMetric const &other) const
         { return coded_value != other.coded_value; }

         ////////////////////////////////////////////////////////////
         // less then
         ////////////////////////////////////////////////////////////
         bool operator <(HopMetric const &other) const
         { return coded_value < other.coded_value; }

         ////////////////////////////////////////////////////////////
         // less than or equal
         ////////////////////////////////////////////////////////////
         bool operator <=(HopMetric const &other) const
         { return coded_value <= other.coded_value; }

         ////////////////////////////////////////////////////////////
         // greater than
         ////////////////////////////////////////////////////////////
         bool operator >(HopMetric const &other) const
         { return coded_value > other.coded_value; }

         ////////////////////////////////////////////////////////////
         // greater than or equal
         ////////////////////////////////////////////////////////////
         bool operator >=(HopMetric const &other) const
         { return coded_value >= other.coded_value; }
         //@endgroup
      };
   };
};


#endif
