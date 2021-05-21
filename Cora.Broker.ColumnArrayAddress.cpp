/* Cora.Broker.ColumnArrayAddress.cpp

   Copyright (C) 2000, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 24 May 2000
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.ColumnArrayAddress.h"
#include <sstream>
#include <algorithm>
#include <iterator>
#include <assert.h>


namespace Cora
{
   namespace Broker
   {
      namespace ColumnArrayAddressHelpers
      {
         template <class char_type, class stream_type>
         void format_name(
            stream_type &out,
            ColumnArrayAddress *address,
            bool with_subscripts,
            char_type const *subscript_list_prefix,
            char_type const *subscript_prefix,
            char_type const *subscript_postfix,
            char_type const *subscript_list_postfix)
         {
            out << address->get_name();
            if(with_subscripts && !address->is_scalar())
            {
               typedef std::vector<uint4> subscripts_type;
               subscripts_type subscripts;

               address->get_subscripts(subscripts);
               out << subscript_list_prefix;
               for(subscripts_type::const_iterator si = subscripts.begin();
                   si != subscripts.end();
                   ++si)
               {
                  if(si != subscripts.begin())
                     out << subscript_prefix;
                  out << (*si);
                  if(si + 1 != subscripts.end())
                     out << subscript_postfix; 
               }
               out << subscript_list_postfix;
            }
         } // format_name
      };


      ////////////////////////////////////////////////////////////
      // class ColumnArrayAddress definitions
      ////////////////////////////////////////////////////////////
      ColumnArrayAddress::ColumnArrayAddress(column_handle &column_):
         column(column_),
         linear_address(0)
      { linear_address = column->front().start_index; }


      ColumnArrayAddress::ColumnArrayAddress(
         column_handle &column_,
         char const *subscripts_):
         column(column_)
      { operator =(subscripts_); }


      ColumnArrayAddress::ColumnArrayAddress(
         column_handle &column_,
         wchar_t const *subscripts_):
         column(column_)
      { operator =(subscripts_); }


      ColumnArrayAddress::ColumnArrayAddress(ColumnArrayAddress const &other):
         column(other.column),
         linear_address(other.linear_address)
      { }

   
      ColumnArrayAddress &ColumnArrayAddress::operator =(ColumnArrayAddress const &other)
      {
         column = other.column;
         linear_address = other.linear_address;
         return *this;
      } // assignment operator


      ColumnArrayAddress &ColumnArrayAddress::operator =(char const *subscripts_)
      {
         std::istringstream input(subscripts_);
         subscripts_type temp;
         uint4 offset;

         std::copy(
            std::istream_iterator<uint4>(input),
            std::istream_iterator<uint4>(),
            std::back_inserter(temp));
         offset = column->get_dimensions().to_offset(temp.begin(),temp.end());
         if(is_valid_address(offset))
            linear_address = offset;
         else
            throw std::invalid_argument("Invalid array address specified");
         return *this;
      } // assignment operator


      ColumnArrayAddress &ColumnArrayAddress::operator =(wchar_t const *subscripts_)
      {
         std::wistringstream input(subscripts_);
         subscripts_type temp;
         uint4 offset;

         std::copy(
            std::istream_iterator<uint4, wchar_t>(input),
            std::istream_iterator<uint4, wchar_t>(),
            std::back_inserter(temp));
         offset = column->get_dimensions().to_offset(temp.begin(),temp.end()); 
         if(is_valid_address(offset))
            linear_address = offset;
         else
            throw std::invalid_argument("Invalid array address specified");
         return *this; 
      } // assignment operator


      ColumnArrayAddress &ColumnArrayAddress::operator =(subscripts_type const &subscripts)
      {
         uint4 offset = column->get_dimensions().to_offset(
            subscripts.begin(),
            subscripts.end()); 
         if(is_valid_address(offset))
            linear_address = offset;
         else
            throw std::invalid_argument("Invalid subscripts vector");
         return *this;
      } // assignment operator


      bool ColumnArrayAddress::next_address()
      {
         // search for the piece that contains the current index
         bool rtn = false;

         for(ColumnDesc::const_iterator pi = column->begin();
             pi != column->end();
             ++pi)
         {
            ColumnDesc::piece_type const &piece = *pi;
            if(linear_address >= piece.start_index &&
               linear_address < piece.start_index + piece.num_elements)
            {
               // we have now found the piece that contains the current address. We can now test to
               // see if a simple increment will suffice.
               if(linear_address + 1 < piece.start_index + piece.num_elements)
               {
                  ++linear_address;
                  rtn = true;
               }
               // otherwise, we need to see if the next piece exists
               else
               {
                  ++pi;
                  if(pi != column->end())
                  {
                     rtn = true;
                     linear_address = pi->start_index;
                  }
               }
               break;
            }
         }
         return rtn;
      } // next_address


      bool ColumnArrayAddress::is_valid_address(uint4 linear_address) const
      {
         // scan the list of pieces to see if the address is valid
         bool rtn = false;
         for(ColumnDesc::const_iterator pi = column->begin();
             !rtn && pi != column->end();
             ++pi)
         {
            ColumnDesc::piece_type const &piece = *pi;
            if(linear_address >= piece.start_index &&
               linear_address < piece.start_index + piece.num_elements)
               rtn = true;
         }
         return rtn;
      } // is_valid_address


      void ColumnArrayAddress::format_name(
         std::ostream &out,
         bool with_subscripts,
         char const *subscript_list_prefix,
         char const *subscript_prefix,
         char const *subscript_postfix,
         char const *subscript_list_postfix)
      {
         ColumnArrayAddressHelpers::format_name(
            out,
            this,
            with_subscripts,
            subscript_list_prefix,
            subscript_prefix,
            subscript_postfix,
            subscript_list_postfix);
      } // format_name


      void ColumnArrayAddress::format_name(
         std::wostream &out,
         bool with_subscripts,
         wchar_t const *subscript_list_prefix,
         wchar_t const *subscript_prefix,
         wchar_t const *subscript_postfix,
         wchar_t const *subscript_list_postfix)
      {
         ColumnArrayAddressHelpers::format_name(
            out,
            this,
            with_subscripts,
            subscript_list_prefix,
            subscript_prefix,
            subscript_postfix,
            subscript_list_postfix);
      } // format_name


      void ColumnArrayAddress::get_subscripts(subscripts_type &subscripts) const
      {
         subscripts.resize(column->get_dimensions().size());
         column->get_dimensions().to_index(
            subscripts.begin(),
            linear_address);
      } // get_subscripts
   };
};
