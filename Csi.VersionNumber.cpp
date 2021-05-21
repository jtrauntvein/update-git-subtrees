/* Csi.VersionNumber.cpp

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Friday 25 August 2017
   Last Commit: $Date: 2017-08-25 13:10:19 -0600 (Fri, 25 Aug 2017) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.VersionNumber.h"
#include "StrAsc.h"
#include "StrUni.h"
#include "Csi.StrAscStream.h"
#include <ctype.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>


namespace Csi
{
   namespace VersionNumberHelpers
   {
      ////////// read_version_number
      void read_version_number(
         VersionNumber &version,
         char const *source,
         StrAsc &remnant)
      {
         StrAsc this_number;
         char const *s(source);
         version.clear();
         for(s = source; *s != 0; ++s)
         {
            if(isdigit(*s))
               this_number += *s;
            else if(isspace(*s) || ispunct(*s))
            {
               // we will allow several whitespace characters before, between, and after the
               // string. We will not allow a puncuation character without at least one intervening
               // digit.
               if(this_number.length() > 0)
               {
                  version.push_back(strtoul(this_number.c_str(),0,10));
                  this_number.cut(0);
               }
               else if(ispunct(*s))
                  throw std::invalid_argument("invalid version string");
            }
            else if(!version.empty())
            {
               remnant = s;
               break;
            }
         }
         if(this_number.length() > 0)
            version.push_back(strtoul(this_number.c_str(),0,10));
      } // read_version_number


      void read_version_number(
         VersionNumber &version,
         wchar_t const *source,
         StrAsc &remnant)
      {
         StrUni source_uni(source);
         StrAsc source_mb(source_uni.to_utf8());
         read_version_number(version, source_mb.c_str(), remnant);
      } // read_version_number
   };

   ////////////////////////////////////////////////////////////
   // class VersionNumber definitions
   //////////////////////////////////////////////////////////// 
   VersionNumber::VersionNumber()
   { }


   VersionNumber::VersionNumber(char const *version_string)
   {
      VersionNumberHelpers::read_version_number(
         *this,
         version_string,
         remnant);
   } // constructor


   VersionNumber::VersionNumber(wchar_t const *version_string)
   {
      VersionNumberHelpers::read_version_number(
         *this,
         version_string,
         remnant);
   } // constructor


   VersionNumber::~VersionNumber()
   { }


   VersionNumber &VersionNumber::operator =(char const *version_string)
   {
      VersionNumberHelpers::read_version_number(
         *this,
         version_string,
         remnant);
      return *this; 
   } // copy operator


   VersionNumber &VersionNumber::operator =(wchar_t const *version_string)
   {
      VersionNumberHelpers::read_version_number(
         *this,
         version_string,
         remnant);
      return *this; 
   } // copy operator 


   void VersionNumber::format(std::ostream &out, bool pad_inner) const
   {
      for(const_iterator i = begin(); i != end(); ++i)
      {
         uint4 value = *i;
         if(i == begin())
            out << value;
         else
         {
            out << ".";
            if(pad_inner)
               out << std::setw(2) << std::setfill('0') << std::right << value;
            else
               out << value;
         }
      }
   }


   void VersionNumber::format(std::wostream &out, bool pad_inner) const
   {
      for(const_iterator i = begin(); i != end(); ++i)
      {
         uint4 value = *i;
         if(i == begin())
            out << value;
         else
         {
            out << L".";
            if(pad_inner)
               out << std::setw(2) << std::setfill(L'0') << std::right << value;
            else
               out << value;
         }
      }
   }


   StrAsc VersionNumber::to_utf8(bool pad_inner) const
   {
      OStrAscStream rtn;
      format(rtn, pad_inner);
      return rtn.str();
   } // to_StrAsc
};

