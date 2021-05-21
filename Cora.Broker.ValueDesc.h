/* Cora.Broker.ValueDesc.h

   Copyright (C) 1998, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 24 August 1999
   Last Change: Wednesday 16 May 2018
   Last Commit: $Date: 2018-05-16 16:20:52 -0600 (Wed, 16 May 2018) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_ValueDesc_h
#define Cora_Broker_ValueDesc_h

#include <vector>
#include "CsiTypes.h"
#include "StrUni.h"
#include "Csi.Messaging.Message.h"


namespace Cora
{
   namespace Broker
   {
      /**
       * Defines an object that represents the description of a value within a record.
       */
      class ValueDesc
      {
      public:
         /**
          * Default constructor
          */
         ValueDesc();

         /**
          * Copy constructor
          *
          * @param other Specifies the description to copy.
          */
         ValueDesc(ValueDesc const &other);

         /**
          * Constructor
          *
          * @param name_ Specifies the name for this value.
          *
          * @param data_type_ Specifies the data type for this value.
          */
         ValueDesc(StrUni const &name_, CsiDbTypeCode datatype_);
      
         /**
          * Destructor
          */
         ~ValueDesc();

         /**
          * Copy operator
          */
         ValueDesc &operator =(ValueDesc const &other);

         /**
          * Reads the members for this description from the specified message object.
          *
          * @param in Specifies the message to read.
          *
          * @param read_value_descriptions Set to true if the description field should be expected
          * in the message.
          *
          * @return Returns true if the value description was read of false if an error occurred.
          */
         bool read(
            Csi::Messaging::Message &in,
            bool read_value_descriptions = false);
         
         /**
          * @return Returns true if the value associated with this description should be merged with
          * the previous value (this applies to string data types).
          */
         bool should_be_merged() const
         {
            return data_type == CsiAscii &&
               !array_address.empty() &&
               array_address.back() != 1;
         }

         /**
          * @return Returns true if this value is the first of a set of adjacent values that will be
          * merged.
          */
         bool start_of_merge() const
         {
            return data_type == CsiAscii &&
               !array_address.empty() &&
               array_address.back() == 1;
         }

      public:
         /**
          * Specifies the name of this value.
          */
         StrUni name;

         /**
          * Specifies the data type code for this value.
          */
         CsiDbTypeCode data_type;

         /**
          * Specifies the command code for the LoggerNet transaction used to modify this value.  A
          * value of zero implies that the value is read-only.
          */
         uint4 modifying_cmd;

         /**
          * Specifies the units string associated with this value.
          */
         StrUni units;

         /**
          * Specifies the process string associated with this value.
          */
         StrUni process;

         /**
          * Specifies the description string associated with this value.
          */
         StrUni description;

         /**
          * Specifies the subscripts for the array.  If this is empty, the value is considered to be
          * a scalar.
          */
         typedef std::vector<uint4> array_address_type;
         array_address_type array_address;

         // @group: array address container access methods

         /**
          * @return Returns the iterator to the first array subscript.
          */
         typedef array_address_type::iterator iterator;
         typedef array_address_type::const_iterator const_iterator;
         iterator begin() { return array_address.begin(); }
         const_iterator begin() const { return array_address.begin(); }

         /**
          * @return Returns the reverse iterator to the last array subscript.
          */
         typedef array_address_type::reverse_iterator reverse_iterator;
         typedef array_address_type::const_reverse_iterator const_reverse_iterator;
         reverse_iterator rbegin() { return array_address.rbegin(); }
         const_reverse_iterator rbegin() const { return array_address.rbegin(); }
         
         /**
          * @return Returns the iterator beyond the last array subscript.
          */
         iterator end() { return array_address.end(); }
         const_iterator end() const { return array_address.end(); }

         /**
          * @return Returns the reverse iterator beyond the first subscript.
          */
         reverse_iterator rend() { return array_address.rend(); }
         const_reverse_iterator rend() const { return array_address.rend(); }

         /**
          * @returns true if this value is a scalar.
          */
         bool empty() const { return array_address.empty(); }

         /**
          * @return Returns the number of subscripts.
          */
         array_address_type::size_type size() const
         { return array_address.size(); }
         
         // @endgroup:

         /**
          * Formats the name of this value to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param with_subscripts Set to true if the subscripts should be included.
          *
          * @param subscript_List_prefix Specifies the string that will be printed preceding the
          * subscripts.
          *
          * @param subscript_prefix Specifies the string that should precede any subscript.
          *
          * @param subscript_postfix Specifies the string that should be inserted between
          * subscripts.
          *
          * @param subscript_list_postfix Specifies the string that should be printed after the
          * subscript list
          *
          * @param with_last_subscript Set to true if the least significant subscript should be
          * formatted.
          */
         void format_name(
            std::ostream &out,
            bool with_subscripts = true,
            char const *subscript_list_prefix = "(",
            char const *subscript_prefix = "",
            char const *subscript_postfix = ",",
            char const *subscript_list_postfix = ")",
            bool with_last_subscript = true) const;
         void format_name(
            std::wostream &out,
            bool with_subscripts = true,
            wchar_t const *subscript_list_prefix = L"(",
            wchar_t const *subscript_prefix = L"",
            wchar_t const *subscript_postfix = L",",
            wchar_t const *subscript_list_postfix = L")",
            bool with_last_subscript = true) const;
      };
   };
};

#endif
