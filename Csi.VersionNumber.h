/* Csi.VersionNumber.h

   Copyright (C) 2000, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Friday 12 September 2014
   Last Commit: $Date: 2017-08-25 13:10:19 -0600 (Fri, 25 Aug 2017) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_VersionNumber_h
#define Csi_VersionNumber_h

#include "CsiTypeDefs.h"
#include "StrAsc.h"
#include <vector>
#include <stdexcept>
#include <iosfwd>


namespace Csi
{
   /**
    * Represents a software version number as a collection of integers and provides the means of
    * parsing and formatting a version number in various text formats including digits separated by
    * commas, spaces, and/or periods.
    */
   class VersionNumber: public std::vector<uint4>
   {
   public:
      /**
       * Default constructor creates an empty set of versions.
       */
      VersionNumber();

      /**
       * Parse a version string from a single bytes string.
       *
       * @param version_string Specifies a single byte string that conforms to the following syntax:
       *
       * version_str := [ version { punct version }].
       * version     := integer.
       * punct       := space | "." | "," | "-" | ":" | ";".
       */
      VersionNumber(char const *version_string);

      /**
       * Parse a version string from a wide terminated string.
       *
       * @param version_string Specifies a wide nul terminated string that conforms to the following
       * syntax:
       *
       * version_str := [ version { punct version }].
       * version     := integer.
       * punct       := space | "." | "," | "-" | ":" | ";".
       */
      VersionNumber(wchar_t const *version_string);

      /**
       * Destructor
       */
      ~VersionNumber();

      /**
       * Copy operator
       *
       * @param version_string  A single byte nul terminated string in the format specified for the
       * string constructors.
       */
      VersionNumber &operator =(char const *version_string);

      /**
       * Copy operator
       *
       * @param version_string A wide nul terminared sgtring in the format specified for string
       * based constructors.
       */
      VersionNumber &operator =(wchar_t const *version_string);

      /**
       * Formats the version number to the specified stream.
       *
       * @param out  Specifies the stream to which the version will be formatted.
       *
       * @param pad_inner  Set to true if versions after the first should be formatted with two
       * decimal places (2.02).
       */
      void format(std::ostream &out, bool pad_inner = false) const;
      void format(std::wostream &out, bool pad_inner = false) const;

      /**
       * @return Formats to a UTF-8 string and returns it.
       *
       * @param pad_inner Set to true if inner version numbers should be padded with a leading zero
       * for single digits.
       */
      StrAsc to_utf8(bool pad_inner = false) const;

      /**
       * @return Returns the portion of the original string that does not conform to a version
       * number syntax.
       */
      StrAsc const &get_remnant() const
      { return remnant; }
      
   private:
      /**
       * Stores the portion of the string that did not conform to the version string syntax.
       */
      StrAsc remnant;
   };
};


inline std::ostream &operator <<(std::ostream &out, Csi::VersionNumber const &v)
{
   v.format(out, false);
   return out;
}


inline std::wostream &operator <<(std::wostream &out, Csi::VersionNumber const &v)
{
   v.format(out, false);
   return out;
}


#endif
