/* Cora.Broker.ColumnArrayAddress.h

   Copyright (C) 2000, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 24 May 2000
   Last Change: Thursday 12 May 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef Cora_Broker_ColumnArrayAddress_h
#define Cora_Broker_ColumnArrayAddress_h

#include "Cora.Broker.ColumnDesc.h"
#include "Csi.SharedPtr.h"
#include <stdexcept>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class ColumnArrayAddress
      //
      // Describes the address of a single element within a column
      // described by a ColumnDesc object. Provides methods and overloaded
      // operators that allow the application to manipulate the address
      // safely. These methods will use exceptions to report when a
      // manipulation would cause the address to become invalid.
      //
      // The methods and operators that have a potential to modify the
      // state of the class will perform a check to make sure that the new
      // state would be considered valid before committing the
      // changes. Because of this, an object of this class will not be able
      // to exist in a state that would be considered invalid.
      ////////////////////////////////////////////////////////////
      class ColumnArrayAddress
      {
      public:
         typedef Csi::SharedPtr<ColumnDesc> column_handle;

      private:
         ////////////////////////////////////////////////////////////
         // column
         //
         // Contains a reference to the column description
         ////////////////////////////////////////////////////////////
         column_handle column;
      
         ////////////////////////////////////////////////////////////
         // linear_address
         //
         // The linear address of the element that this object is
         // describing
         ////////////////////////////////////////////////////////////
         uint4 linear_address;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // Sets the subscripts so that the address represents the first
         // array element
         ////////////////////////////////////////////////////////////
         ColumnArrayAddress(column_handle &column_);

         ////////////////////////////////////////////////////////////
         // string constructor
         //
         // Attempts to read the subscripts string as a list of signed
         // integers separated by one or more whitespace characters. This
         // string is expected to be null terminated. The number of
         // integers in the string must match the number of dimensions
         // specified in the column description. The value of each
         // subscript must be in the range of the column description
         // dimension corresponding with its position in the string.
         ////////////////////////////////////////////////////////////
         ColumnArrayAddress(column_handle &column, char const *subscripts_);
         ColumnArrayAddress(column_handle &column, wchar_t const *subscripts_);

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         ColumnArrayAddress(ColumnArrayAddress const &other);

         ////////////////////////////////////////////////////////////
         // copy operator
         ////////////////////////////////////////////////////////////
         ColumnArrayAddress &operator =(ColumnArrayAddress const &other);

         ////////////////////////////////////////////////////////////
         // string assignment operator
         //
         // Attempts to read the subscripts string as a list of signed
         // integers separated by one or more whitespace characters. This
         // string is expected to be null terminated. The number of
         // integers in the string must match the number of dimensions
         // specified in the column description. The value of each
         // subscript must be in the range of the column description
         // dimension corresponding with its position in the string.
         ////////////////////////////////////////////////////////////
         ColumnArrayAddress &operator =(char const *subscripts_);
         ColumnArrayAddress &operator =(wchar_t const *subscripts_);

         ////////////////////////////////////////////////////////////
         // vector assignment operator
         // 
         // Attempts to initialise the array address using a vector of
         // integers. The size of this vector must match the number of
         // columns specified by the column definition. The value of each
         // subscript must be in the range of the column description
         // dimension specified by its position.
         ////////////////////////////////////////////////////////////
         typedef std::vector<uint4> subscripts_type;
         ColumnArrayAddress &operator =(subscripts_type const &subscripts_);

         ////////////////////////////////////////////////////////////
         // next_address
         //
         // Sets the subscript to the next element of the array using
         // row-major order (most significant subscript is first). Returns
         // false if the resulting address would be considered
         // invalid. Note that the change will only be made if the
         // resulting address would be considered valid.
         ////////////////////////////////////////////////////////////
         bool next_address();

         ////////////////////////////////////////////////////////////
         // increment operator
         //
         // Overloads the prefix and postfix increment operators to perform
         // the same task as next_address() only it will throw a
         // std::out_of_range if the resulting address would be
         // invalid. Note that the decrement operator is not defined but
         // could be added if needed.
         ////////////////////////////////////////////////////////////
         ColumnArrayAddress &operator ++() /* prefix */
         { if(!next_address()) throw std::out_of_range("Increment out of range"); }
      
         ColumnArrayAddress operator ++(int) /* postfix */
         {
            ColumnArrayAddress rtn(*this);
            if(!next_address()) throw std::out_of_range("Increment out of range");
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // is_valid_address
         //
         // Returns true if the specified array address is valid (refers to
         // a valid column element)
         ////////////////////////////////////////////////////////////
         bool is_valid_address(uint4 linear_address) const;

         ////////// format_name
         ////////////////////////////////////////////////////////////
         // format_name
         //
         // Formats the name to the stream object. This method is simlar to
         // format_name only it adds parameters that allow strings to be
         // optionally inserted before and after the subscripts list.
         //
         // The specified subscript prefix and postfix parameters will be
         // inserted into the stream only if with_subscripts is true and
         // the following conditions exist:
         //
         //   subscript_list_prefix: Inserted into the stream immediately
         //                          following the name and before any
         //                          subscript is inserted.
         //
         //   subscript_prefix: Inserted into the stream before any but the
         //                     first subscript
         //
         //   subscript_postfix: Inserted into the stream after any but the
         //                      last subscript
         //
         //   subscript_list_postfix: Inserted into the stream after the
         //                           last subscript has been inserted.
         ////////////////////////////////////////////////////////////
         void format_name(
            std::ostream &out,
            bool with_subscripts = true,
            char const *subscript_list_prefix = "_",
            char const *subscript_prefix = "",
            char const *subscript_postfix = "",
            char const *subscript_list_postfix = "");
         void format_name(
            std::wostream &out,
            bool with_subscripts = true,
            wchar_t const *subscript_list_prefix = L"_",
            wchar_t const *subscript_prefix = L"",
            wchar_t const *subscript_postfix = L"",
            wchar_t const *subscript_list_postfix = L"");

         ////////////////////////////////////////////////////////////
         // get_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_name() const { return column->get_name(); }

         ////////////////////////////////////////////////////////////
         // get_subscripts
         ////////////////////////////////////////////////////////////
         void get_subscripts(subscripts_type &subscripts) const;

         ////////////////////////////////////////////////////////////
         // is_scalar
         ////////////////////////////////////////////////////////////
         bool is_scalar() const
         { return column->get_dimensions().empty(); }

         ////////////////////////////////////////////////////////////
         // comparison operators
         //
         // Defines operators that compare one address against another. For efficiency's sake, no
         // checking to make sure they have the same column definition will be done.
         ////////////////////////////////////////////////////////////
         bool operator ==(ColumnArrayAddress const &other) const
         { return linear_address == other.linear_address; }
         bool operator !=(ColumnArrayAddress const &other) const
         { return linear_address != other.linear_address; }
         bool operator <(ColumnArrayAddress const &other) const
         { return linear_address < other.linear_address; }
         bool operator <=(ColumnArrayAddress const &other) const
         { return linear_address <= other.linear_address; }
         bool operator >(ColumnArrayAddress const &other) const
         { return linear_address > other.linear_address; }
         bool operator >=(ColumnArrayAddress const &other) const
         { return linear_address >= other.linear_address; }
      };
   };
};

#endif
