/* Cora.Broker.Value.h

   Copyright (C) 1998, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 24 August 1999
   Last Change: Thursday 06 February 2020
   Last Commit: $Date: 2020-02-06 19:41:32 -0600 (Thu, 06 Feb 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_Value_h
#define Cora_Broker_Value_h

#include <iostream>
#include "Csi.SharedPtr.h"
#include "CsiTypes.h"
#include "Cora.Broker.ValueDesc.h"
#include "Cora.Broker.CustomCsvOptions.h"
#include "Csi.Messaging.Message.h"
#include "Csi.MsgExcept.h"
#include "Csi.Json.h"


// @group: class forward declarations
namespace Csi
{
   namespace Json
   {
      class ValueBase;
   };
}; 
// @endgroup:

namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class Value
      //
      // Base class for all objects that represent scalar data values from a
      // data set resulting from the data broker data advise or data broker
      // data query transactions.
      ////////////////////////////////////////////////////////////
      class Value
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<ValueDesc> desc_handle;
         Value(desc_handle &description_):
            description(description_),
            combined_with_adjacent_values(false),
            storage(0)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Value()
         { }

         ////////////////////////////////////////////////////////////
         // read_text
         //
         // Reads the value from the specified text string.
         ////////////////////////////////////////////////////////////
         virtual void read_text(char const *text)
         { throw Csi::MsgExcept("Value->text conversion not implemented"); }
         
         ////////////////////////////////////////////////////////////
         // get_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_name() const
         { return description->name; }

         ////////////////////////////////////////////////////////////
         // get_description
         ////////////////////////////////////////////////////////////
         desc_handle &get_description()
         { return description; }
         desc_handle const &get_description() const
         { return description; }

         ////////////////////////////////////////////////////////////
         // get_description_str
         ////////////////////////////////////////////////////////////
         StrUni const &get_description_str() const
         { return description->description; }

         ////////////////////////////////////////////////////////////
         // format
         //
         // Formats the value in the output stream object 
         // The optional overload_stream_attribs parameter lets you
         // control whether or not you want the stream attributes to
         // be changed.  The optional time_format parameter lets you
         // specify how you want the time value to be formatted
         ////////////////////////////////////////////////////////////
         virtual void format(
            std::ostream &out,
            bool overload_stream_attribs = false,
            char const *time_format = 0,
            CustomCsvOptions const *custom_options = 0,
            bool specials_as_numbers = false,
            bool optimise_without_locale = false,
            bool translate_strings = false) const = 0;
         virtual void format(
            std::wostream &out,
            bool overload_stream_attribs = false,
            wchar_t const *time_format = 0,
            CustomCsvOptions const *options = 0,
            bool specials_as_numbers = false,
            bool optimise_without_locale = false,
            bool translate_strings = false) const = 0;

         ////////////////////////////////////////////////////////////
         // format_json
         ////////////////////////////////////////////////////////////
         virtual void format_json(std::ostream &out)
         { format(out, false, "%Y-%m-%dT%H:%M:%S%x"); }

         ////////////////////////////////////////////////////////////
         // read_json
         ////////////////////////////////////////////////////////////
         virtual void read_json(Csi::Json::ValueBase *json)
         { throw Csi::MsgExcept("Cannot convert a JSON value to a record value."); }

         /**
          * @return Generates a JSON value type to represent this value.  If not overloaded, will
          * format the value as a string.
          */
         virtual Csi::Json::ValueHandle write_json();
         
         ////////////////////////////////////////////////////////////
         // format_name
         //
         // Formats the name to the stream object. Includes the subscripts
         // provided the with_subscripts parameter is set to true
         ////////////////////////////////////////////////////////////
         void format_name(
            std::ostream &out,
            bool with_subscripts = true,
            char const *subscript_prefix = "_",
            char const *subscript_postfix = "");
         void format_name(
            std::wostream &out,
            bool with_subscripts = true,
            wchar_t const *subscript_prefix = L"_",
            wchar_t const *subscript_postfix = L"");

         ////////////////////////////////////////////////////////////
         // format_name_ex
         //
         // Formats the name to the stream object. This method is simlar to
         // format_name only it adds parameters that allow strings to be
         // optionally inserted before and after the subscripts list.
         //
         // The specified subscript prefix and postfix parameters will be
         // inserted into the stream only if with_subscripts is true and the
         // following conditions exist:
         //
         //   subscript_list_prefix: Inserted into the stream immediately
         //                          following the name and before any
         //                          subscript is inserted. 
         //
         //   subscript_prefix:      Inserted into the stream before any
         //                          but the first subscript
         //
         //   subscript_postfix: Inserted into the stream after any but the
         //                      last subscript
         //
         //   subscript_list_postfix: Inserted into the stream after the last
         //                           subscript has been inserted.
         ////////////////////////////////////////////////////////////
         void format_name_ex(
            std::ostream &out,
            bool with_subscripts = true,
            char const *subscript_list_prefix = "(",
            char const *subscript_prefix = ",",
            char const *subscript_postfix = "",
            char const *subscript_list_postfix = ")") const;
         void format_name_ex(
            std::wostream &out,
            bool with_subscripts = true,
            wchar_t const *subscript_list_prefix = L"(",
            wchar_t const *subscript_prefix = L",",
            wchar_t const *subscript_postfix = L"",
            wchar_t const *subscript_list_postfix = L")") const;

         ////////////////////////////////////////////////////////////
         // format_ldep_type
         //
         // Writes the type string associated with LDEP to the supplied stream
         // object
         ////////////////////////////////////////////////////////////
         virtual void format_ldep_type(std::ostream &out) const = 0;
         virtual void format_ldep_type(std::wostream &out) const = 0;

         ////////////////////////////////////////////////////////////
         // get_pointer
         //
         // Returns a pointer to the value's storage
         ////////////////////////////////////////////////////////////
         void *get_pointer()
         { return storage; }
         void const *get_pointer() const
         { return storage; }

         ////////////////////////////////////////////////////////////
         // get_pointer_len
         //
         // Returns the amount of storage in bytes required for this value
         ////////////////////////////////////////////////////////////
         virtual uint4 get_pointer_len() const = 0;

         ////////////////////////////////////////////////////////////
         // get_type
         //
         // Returns the type associated with the value description
         ////////////////////////////////////////////////////////////
         CsiDbTypeCode get_type() const;

         ////////////////////////////////////////////////////////////
         // get_process_string
         //
         // returns the process string
         ////////////////////////////////////////////////////////////
         StrUni const &get_process_string() const;

         ////////////////////////////////////////////////////////////
         // get_units_string
         //
         // returns the units string
         ////////////////////////////////////////////////////////////
         StrUni const &get_units_string() const;

         ////////////////////////////////////////////////////////////
         // empty
         //
         // Returns the whether or not the array address is empty thus whether
         // its a scalar or not
         ////////////////////////////////////////////////////////////
         bool empty()
         { return description->empty(); }

         ////////////////////////////////////////////////////////////
         // clone
         //
         // Creates another instance of this value of the same type.
         // Ordinarily, values will get their storage pointers from the records
         // to which they belong.  If a value is cloned, however, and the
         // clone_storage parameter is true, a new storage pointer will be
         // allocated and the owns_storage flag will be set.  This will allow
         // the cloned value to live without the record.
         ////////////////////////////////////////////////////////////
         virtual Value *clone() = 0;

         ////////////////////////////////////////////////////////////
         // combine_with_adjacent_values
         //
         // Called to check if this value should be merged with a subsequent
         // value.  The description for the other value is given in the
         // other_desc parameter.  This capability is needed in order to allow
         // the treatment of ASCII character arrays as full fledged strings.
         // This default implementation will return false indicating that the
         // value cannot be merged.
         ////////////////////////////////////////////////////////////
         virtual bool combine_with_adjacent_values(desc_handle &other_desc)
         { return false; }

         ////////////////////////////////////////////////////////////
         // to_float
         //
         // Can be overloaded to convert the value to a floating point number.
         // Returns false if the value cannot be converted (the default
         // implementation)
         ////////////////////////////////////////////////////////////
         virtual bool to_float(double &dest) const
         { dest = 0.0; return false; }

         ////////////////////////////////////////////////////////////
         // write_tob1
         //
         // Appends the binary representation of this value to the specified
         // buffer.
         ////////////////////////////////////////////////////////////
         virtual void write_tob1(StrBin &buffer)
         { buffer.append(storage,get_pointer_len()); }

         ////////////////////////////////////////////////////////////
         // quote_when_formatting
         //
         // Returns true if this value type should be quoted when formatted as
         // text
         ////////////////////////////////////////////////////////////
         virtual bool quote_when_formatting(
            CustomCsvOptions const *options = 0) const
         { return false; }

         ////////////////////////////////////////////////////////////
         // get_read_only
         ////////////////////////////////////////////////////////////
         bool get_read_only() const
         { return description->modifying_cmd == 0; }

         ////////////////////////////////////////////////////////////
         // set_storage
         ////////////////////////////////////////////////////////////
         uint4 set_storage(void *storage_)
         {
            storage = storage_;
            return get_pointer_len();
         }

         ////////////////////////////////////////////////////////////
         // is_tob1_native
         //
         // Returns true if this value type is "native tob1".
         ////////////////////////////////////////////////////////////
         virtual bool is_tob1_native() const
         { return false; }
         
         ////////////////////////////////////////////////////////////
         // set_to_null
         //
         // Provides a means of setting the value to its "null" value.  What
         // this means is going to depend heavily on the type of data.
         ////////////////////////////////////////////////////////////
         virtual void set_to_null() = 0;

         ////////////////////////////////////////////////////////////
         // get_combined_with_adjacent_values
         ////////////////////////////////////////////////////////////
         bool get_combined_with_adjacent_values() const
         { return combined_with_adjacent_values; }

         ////////////////////////////////////////////////////////////
         // get_storage
         ////////////////////////////////////////////////////////////
         void const *get_storage() const
         { return storage; }
         void *get_storage()
         { return storage; }
         
      protected:
         ////////////////////////////////////////////////////////////
         // description
         //
         // Handle to the value description object that was used to create this
         // object
         ////////////////////////////////////////////////////////////
         desc_handle description;

         ////////////////////////////////////////////////////////////
         // combined_with_adjacent_values
         //
         // Should be set to true if a derived class allows this value to be
         // combined.  This will affect the way that format_name_xxx() methods
         // will work.
         ////////////////////////////////////////////////////////////
         bool combined_with_adjacent_values;

         ////////////////////////////////////////////////////////////
         // storage
         ////////////////////////////////////////////////////////////
         void *storage;
      };
   };
};
      
#endif
