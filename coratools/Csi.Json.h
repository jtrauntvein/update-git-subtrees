/* Csi.Json.h

   Copyright (C) 2010, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 25 March 2010
   Last Change: Thursday 13 August 2020
   Last Commit: $Date: 2020-08-13 10:27:58 -0600 (Thu, 13 Aug 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Json_h
#define Csi_Json_h

#include "StrAsc.h"
#include "StrUni.h"
#include "CsiTypeDefs.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.LgrDate.h"
#include <deque>


namespace Csi
{
   namespace Json
   {
      /**
       * Defines an enumeration for identifiers for types of various values that can be stored as
       * properties in a JSON object.
       */
      enum ValueType
      {
         value_null,
         value_object,
         value_array,
         value_string,
         value_number,
         value_bool
      };

      
      /**
       * Defines a manipulator that foirmats a string specified in tge constructor to a stream.
       */
      struct format_string
      {
         /**
          * Specifies the unicode content of the string.
          */
         StrUni content;

         /**
          * Constructs this from a multi-byte or utf-8 string.
          *
          * @param s Specifies the string to set.
          *
          * @param from_utf8 Set to true if the source string is expected to be utf-8 encoded.  Set
          * to false if it is multi-byte encoded with the current locale.
          */
         format_string(StrAsc const &s, bool from_utf8 = true);

         /**
          * Constructs this from a unicode string.
          *
          * @param s Specifies the string to copy.
          */
         format_string(StrUni const &s);

         /**
          * Constructs from a raw character buffer.
          *
          * @param s Specifies the raw character buffer.  This value is expected to be utf-8 encoded
          * and null terminated.
          *
          * @param max_len Specifies the maximum length for the copied string.  Thge actual length
          * may be less if the array is nul terminated.
          */
         format_string(char const *s, size_t max_len)
         {
            size_t len(max_len);
            for(uint4 i = 0; i < max_len; ++i)
            {
               if(s[i] == 0)
               {
                  len = i;
                  break;
               }
            }
            content.reserve(len);
            content.append_utf8(s, len);
         }

         /**
          * Output operator overloads.
          */
         void operator ()(std::ostream &out) const;
         void operator ()(std::wostream &out) const;
      };
      std::ostream &operator <<(std::ostream &out, format_string const &s);
      std::wostream &operator <<(std::wostream &out, format_string const &s);


      /**
       * Defines a class of exception that reports a problem parsing a JSON string.
       */
      class parse_exception: public std::exception
      {
      private:
         /**
          * Specifies the error message.
          */
         StrAsc message;
         
      public:
         /**
          * Default constructor
          */
         parse_exception():
            message("JSON parsing exception")
         { }

         /**
          * Constructor
          *
          * @param s Specifies the message.
          *
          * @param row_no Reference to the row number in the source where the error was found.
          *
          * @param column_no Reference to the column number in the source where the error was found.
          */
         parse_exception(StrAsc const &s, int *row_no = 0, int *column_no = 0);

         /**
          * Destructor
          */
         virtual ~parse_exception() throw ()
         { }
         
         /**
          * @return Returns the formatted message.
          */
         virtual char const *what() const throw ()
         { return message.c_str(); }
      };


      /**
       * Defines a base class for any type of object that can appear within a JSON structure.
       */
      class ValueBase
      {
      public:
         /**
          * Destructor
          */
         virtual ~ValueBase()
         { }

         /**
          * @return Must be overloaded to return a copy of this object that has the same type and content.
          */
         virtual ValueBase *clone() const = 0;

         /**
          * @return Must be overloaded to return the type code for this object.
          */
         virtual ValueType get_type() const = 0;

         /**
          * Must be overloaded to format this object to the specified stream.
          *
          * @param out Specifies the stream to which this object will be formatted.
          *
          * @param do_indent Set to true if new lines in the stream should be indented.
          *
          * @param indent_level Specifies the indentation level of this object in the overall
          * structure.
          */
         virtual void format(
            std::ostream &out, bool do_indent = true, int indent_level = 0) const = 0;

         /**
          * Must be overloaded to parse the specific type of object as it would appear in JSON
          * format.
          *
          * @return Returns the character that caused parsing to stop.
          *
          * @param in Specifies the input stream.
          *
          * @param start_char Specifies the starting character.
          *
          * @param row_no Reference to an integer that returns the last row processed by this
          * method if not null.
          *
          * @param column_no Refernce to an integer that returns the last column processed by this
          * method if not null.
          *
          * @throw Throws an object of type Csi::Json::parse_exception if parsing fails.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0) = 0;
      };
      typedef SharedPtr<ValueBase> ValueHandle;


      /**
       * Defines an object that represents a string in a JSON structure.
       */
      class String: public ValueBase
      {
      public:
         /**
          * Copy constructor
          *
          * @param value_ Specifies the string to copy.  Can be one of an StrAsc, StrUni, or
          * nul-terminated "C" string.
          */
         String(StrAsc const &value_ = ""):
            value(value_)
         { }
         String(StrUni const &value_):
            value(value_.to_utf8())
         { }
         String(char const *value_):
            value(value_)
         { }

         /**
          * Overloads the copy operator≥
          */
         String &operator =(StrAsc const &value_)
         {
            value = value_;
            return *this;
         }
         String &operator =(StrUni const &value_)
         {
            value = value_.to_utf8();
            return *this;
         }

         /**
          * Destructor
          */
         virtual ~String()
         { }

         /**
          * @return Returns the string value encoded as utf-8.
          */
         StrAsc const &get_value() const
         { return value; }

         /**
          * @return Overloads the base class to return the type code.
          */
         virtual ValueType get_type() const override
         { return value_string; }

         /**
          * @return Overloads the base class to copy this string.
          */
         virtual String *clone() const override
         { return new String(value); }

         /**
          * Overloads the base class version to format the string to the specified stream.
          */
         virtual void format(
            std::ostream &out, bool do_indent = true, int indent_level = 0) const override;

         /**
          * Overloads the base class to parse the string.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0) override;

      private:
         /**
          * Specifies the value for this object.
          */
         StrAsc value;
      };
      typedef PolySharedPtr<ValueBase, String> StringHandle;
      

      /**
       * Defines an object that represents a numeric property in a JSON structure.
       */
      class Number: public ValueBase
      {
      public:
         /**
          * Constructor
          *
          * @param value_ Specifies the value to be assigned to this object.
          */
         Number(double value_ = 0.0):
            value(value_)
         { }

         /**
          * Overloads the copy operator for a double.
          */
         Number &operator =(double value_)
         {
            value = value_;
            return *this;
         }

         /**
          * @return Overloads the base class to return a copy of this property.
          */
         virtual Number *clone() const override
         { return new Number(value); }

         /**
          * Overloads the base class version to format this value in JSON format.
          */
         virtual void format(
            std::ostream &out, bool do_indent = true, int indent_level = 0) const override;

         /**
          * Overloads the  base class version to parse a numeric property.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0) override;

         /**
          * @return Overloads the base class version to return the type code.
          */
         virtual ValueType get_type() const override
         { return value_number; }

         /**
          * @return Returns the value as a double.
          */
         double get_value() const
         { return value; }

         /**
          * @return Returns the value cast as an eight byte integer.
          */
         int8 get_value_int8() const
         { return static_cast<int8>(value); }

         /**
          * @return Returns the value cast as a four byte unsigned integer.
          */
         uint4 get_value_uint4() const
         { return static_cast<uint4>(value); }

         /**
          * @return Returns the value cast as a four byte signed integer.
          */
         int4 get_value_int4() const
         { return static_cast<int4>(value); }

         /**
          * @return Returns the value cast as a two byte unsigned integer.
          */
         uint2 get_value_uint2() const
         { return static_cast<uint2>(value); }

         /**
          * @return Returns the value cast as a two byte signed integer.
          */
         int2 get_value_int2() const
         { return static_cast<int2>(value); }

         /**
          * @param value_ Specifies the value as a double.
          */
         void set_value(double value_)
         { value = value_; }

      private:
         /**
          * Specifies the value for this object.
          */
         double value;
      };
      typedef PolySharedPtr<ValueBase, Number> NumberHandle;


      /**
       * Defines an object that represents a boolean property.
       */
      class Boolean: public ValueBase
      {
      public:
         /**
          * @param value_ Specifies the value for this object.
          */
         Boolean(bool value_ = true):
            value(value_)
         { }

         /**
          * Copy operator
          */
         Boolean &operator =(bool value_)
         {
            value = value_;
            return *this;
         }

         /**
          * @return Overloads the base class to copy this property.
          */
         virtual Boolean *clone() const override
         { return new Boolean(value); }
         
         /**
          * @return Overloads the base class to return the type code.
          */
         virtual ValueType get_type() const override
         { return value_bool; }

         /**
          * Overloads the base class version to write the JSON encoded value.
          */
         virtual void format(
            std::ostream &out, bool do_indent, int indent_level) const override;

         /**
          * Overloads the base class version to parse a JSON encoded value from the given stream.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0) override;

         /**
          * @return Returns the value.
          */
         bool get_value() const
         { return value; }
         
      private:
         /**
          * Specifies the value for this property.
          */
         bool value;
      };
      typedef PolySharedPtr<ValueBase, Boolean> BooleanHandle;


      /**
       * Defines an object that represents a date property that can be output to a JSON stream.
       * Since JSON does not define a date class, this class can only be used for output purposes.
       */
      class Date: public ValueBase
      {
      public:
         /**
          * @param date_ Specifies the value for this property.
          */
         Date(Csi::LgrDate const &date_ = 0):
            date(date_)
         { }

         /**
          * Copy operator.
          */
         Date &operator =(LgrDate const &date_)
         {
            date = date_;
            return *this;
         }

         /**
          * @return Overloads the base class to copy this property.
          */
         virtual Date *clone() const override
         { return new Date(date); }

         /**
          * @return Overloads the base class version to indicate that this property is represented
          * as a string.
          */
         virtual ValueType get_type() const override
         { return value_string; }

         /**
          * Overloads the base class version to format this property in JSON format.  In this case,
          * the date will be formatted as %Y-%m-%dT%H:%M:%S%x.
          */
         virtual void format(
            std::ostream &out, bool do_indent, int indent_level) const override;

         /**
          * Overloads the base class version to parse this value from a JSON format.  Since a date
          * cannot be identified in a JSON structure, this method is a no-op.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0) override
         { return start_char; }
         
         /**
          * @return Returns the value for this property.
          */
         LgrDate const &get_value() const
         { return date; }

      private:
         /**
          * Specifies the value for this property.
          */
         LgrDate date;
      };
      typedef PolySharedPtr<ValueBase, Date> DateHandle;


      /**
       * Defines a value type that represents the contents of a file that will be base64 encoded in
       * a string.  This will not appear in parsed input but can be used to increase the efficiency
       * for formatting JSON.
       */
      class BlobFile: public ValueBase
      {
      public:
         /**
          * Constructor
          *
          * @param file_name Specifies the name of the file to be read.
          */
         BlobFile(StrAsc const &file_name_):
            file_name(file_name_)
         { }

         /**
          * Destructor
          */
         virtual ~BlobFile()
         { }

         /**
          * Overloaded to return a copy.
          */
         virtual ValueBase *clone() const
         { return new BlobFile(file_name); }

         /**
          * @return Overloaded to return the type of this token.
          */
         virtual ValueType get_type() const
         { return value_string; }

         /**
          * Overloads the base class version to output the file as a base64 encoded string.
          */
         virtual void format(
            std::ostream &out, bool do_indent = true, int indent_level = 0) const;

         /**
          * Overloads the parse algorithm as a no-op.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0)
         { return start_char; }

      private:
         /**
          * Specifies the name of the file to be output.
          */
         StrAsc const file_name;
      };
      typedef Csi::PolySharedPtr<ValueBase, BlobFile> BlobFileHandle;


      /**
       * Defines an object that represents a null property.
       */
      class Null: public ValueBase
      {
      public:
         /**
          * @return Overloads the base class version to copy this property.
          */
         virtual Null *clone() const override
         { return new Null; }

         /**
          * @return Overloads the base class version to return the type code.
          */ 
         virtual ValueType get_type() const override
         { return value_null; }

         /**
          * Overloads the base class version to format this property as JSON.
          */
         virtual void format(
            std::ostream &out, bool do_indent, int indent_level) const override;

         /**
          * Overloads the base class version to parse this property from the JSON formatted stream.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0) override; 
      };
      typedef PolySharedPtr<ValueBase, Null> NullHandle;


      /**
       * Defines an object that represents a property that acts as a dictionary for name/value
       * pairs.
       */
      class Object: public ValueBase
      {
      public:
         /**
          * Constructir'
          */
         Object()
         { }

         /**
          * Destructor
          */
         virtual ~Object()
         { values.clear(); }

         /**
          * @return Overloads the base class version to indicate a JSON object property.
          */
         virtual ValueType get_type() const override
         { return value_object; }

         /**
          * @return Overloads the base class version to return a deep copy of this property.
          */
         virtual Object *clone() const override;

         /**
          * Overloads the base class version to format the object to the specified stream in JSON
          * format.
          */
         virtual void format(
            std::ostream &out, bool do_indent = true, int indent_level = 0) const override;

         /**
          * Overloads the base class version to parse the object from the JSON formatted stream.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0) override;
         
         /**
          * @return Returns an iterator to the first name/value pair.
          */
         typedef Csi::SharedPtr<ValueBase> value_handle;
         typedef std::pair<StrAsc, value_handle> value_type;
         typedef std::deque<value_type> values_type;
         typedef values_type::iterator iterator;
         typedef values_type::const_iterator const_iterator;
         iterator begin()
         { return values.begin(); }
         const_iterator begin() const
         { return values.begin(); }

         /**
          * @return Returns the iterator referencing beyond the last name/value pair.
          */
         iterator end()
         { return values.end(); }
         const_iterator end() const
         { return values.end(); }

         /**
          * @return Returns the number of name/values stored in this object.
          */
         typedef values_type::size_type size_type;
         size_type size() const
         { return values.size(); }

         /**
          * @return Returns true if there are no name/value pairs stored.
          */
         bool empty() const
         { return values.empty(); }

         /**
          * Removes all name/value pairs.
          */
         void clear()
         { values.clear();
         }

         /**
          * Adds a new name/value pair to the end of the container.
          *
          * @param value Specifies the name and value to be stored.
          */
         void push_back(value_type const &value)
         { values.push_back(value); }

         /**
          * Adds a new name/value pair at the front of the container.
          */
         void push_front(value_type const &value)
         { values.push_front(value); } 
         
         /**
          * @return Returns the name/value pair at the specified index.
          *
          * @param idx Specifies the index.
          */
         value_type &operator [](uint4 idx)
         { return values[idx]; }
         value_type const &operator [](uint4 idx) const
         { return values[idx]; }
         value_type &at(uint4 idx)
         { return values.at(idx); }
         value_type const &at(uint4 idx) const
         { return values.at(idx); }

         /**
          * @return Returns the iterator for the first name/value pair that matches the specified
          * name.  Returns end() if there is no such pair.
          *
          * @param name Specifies the search string.
          */
         iterator find(StrAsc const &name);
         const_iterator find(StrAsc const &name) const;

         /**
          * @return Overloads the index operator to search for a name/value pair that matches the
          * specified name.  Returns an empty handle if the search fails.
          *
          * @param name Specifies the search string.
          */
         value_handle &operator [](StrAsc const &name);
         value_handle const &operator [](StrAsc const &name) const;

         /**
          * @return Returns true if this object contains the specified property.
          *
          * @param name Specifies the name of the property to find.
          */
         bool has_property(StrAsc const &name) const
         { return find(name) != end(); }

         /**
          * @return Returns the type code for the property in this object associated with the
          * specified name.  Returns value_null if the property does not exist.
          *
          * @param name Specifies the name of the property to look up.
          */
         ValueType get_property_type(StrAsc const &name) const
         {
            ValueType rtn(value_null);
            const_iterator pi(find(name));
            if(pi != end())
               rtn = pi->second->get_type();
            return rtn;
         }
         
         /**
          * Sets or creates a property with the specified name and value.
          *
          * @return Returns the iterator to the name/value pair that was set.
          *
          * @param name Specifies the property name.
          *
          * @param value Specifies the property value.
          *
          * @param check_existing Set to true if the container should be searched for the specified
          * name before adding a new name/value pair.
          */
         iterator set_property(StrAsc const &name, value_handle value, bool check_existing = true);

         /**
          * Deletes the property associated with the specified name.
          *
          * @param name Specifies the name of the property to delete.
          */
         void remove_property(StrAsc const &name);

         /**
          * @return Returns the property associated with the specified name.
          *
          * @param name Specifies the property name.
          */
         value_handle &get_property(StrAsc const &name)
         { return operator[](name); }

         /**
          * @return Returns the string value for the specified property.
          */
         StrAsc get_property_str(StrAsc const &name);
         
         /**
          * Sets the property associated with the given name as a string.
          *
          * @param name The name of the property to set.
          *
          * @param value The string value for the property.
          *
          * @param check_existing Set to true if the list should be searched before creating a new
          * property.
          */
         void set_property_str(StrAsc const &name, StrAsc const &value, bool check_existing = true)
         {
            set_property(name, new String(value), check_existing);
         }

         /**
          * @return Returns the property value for the given name as a double.
          *
          * @param name Specifies the search string.
          */
         double get_property_number(StrAsc const &name);

         /**
          * @return Returns the value of a numeric property cast as an eight byte signed integer.
          */
         int8 get_property_int8(StrAsc const &name)
         { return static_cast<int8>(get_property_number(name)); }

         /**
          * @return Returns the value of a numeric property cast as an unsigned four byte integer.
          */
         uint4 get_property_uint4(StrAsc const &name)
         { return static_cast<uint4>(get_property_number(name)); }

         /**
          * @return Returns the value of the numeric property cast as a signed four byte integer.
          */
         int4 get_property_int4(StrAsc const &name)
         { return static_cast<int4>(get_property_number(name)); }

         /**
          * @return Returns the value of the numeric property cast as an unsigned two byte integer.
          */
         uint2 get_property_uint2(StrAsc const &name)
         { return static_cast<uint2>(get_property_number(name)); }

         /**
          * @return Returns the value of the numeric property cast as a signed two byte integer.
          */
         int2 get_property_int2(StrAsc const &name)
         { return static_cast<int2>(get_property_number(name)); }

         /**
          * Sets the property associated with the given name as a double.
          *
          * @param name Specifies the property name.
          *
          * @param value Specifies the property value.
          *
          * @param check_existing Set to true if the list should be searched before creating a new property.
          */
         void set_property_number(StrAsc const &name, double value, bool check_existing = true)
         {
            set_property(name, new Number(value), check_existing);
         }

         /**
          * @return Returns the specified property evaluated as a boolean.
          *
          * @param name Specifies the name of the property.
          */
         bool get_property_bool(StrAsc const &name)
         {
            iterator pi(find(name));
            bool rtn(false);

            if(pi != end())
            {
               if(pi->second->get_type() == value_bool)
               {
                  BooleanHandle val(pi->second);
                  rtn = val->get_value();
               }
               else if(pi->second->get_type() == value_number)
               {
                  NumberHandle val(pi->second);
                  rtn = val->get_value() != 0;
               }
               else if(pi->second->get_type() == value_string)
               {
                  StringHandle val(pi->second);
                  rtn = val->get_value() == "true" || val->get_value() == "1";
               }
            }
            return rtn;
         }

         /**
          * Sets the named property to a boolean value.
          *
          * @param name Specifies the name of the propery
          *
          * @param value Specifies the boolean value.
          *
          * @param check_first Set to true if the property list should be first searched.
          */
         void set_property_bool(StrAsc const &name, bool value, bool check_first = true)
         {
            set_property(name, new Boolean(value), check_first);
         }

         /**
          * @return Returns the named property evaluated as a date.
          */
         LgrDate get_property_date(StrAsc const &name);

         /**
          * Sets the named property as a date.
          */
         void set_property_date(StrAsc const &name, Csi::LgrDate const &value, bool check_first = true)
         {
            set_property(name, new Date(value), check_first);
         }
         
      private:
         /**
          * Specifies the collection of properties.
          */
         values_type values;
      };
      typedef PolySharedPtr<ValueBase, Object> ObjectHandle;


      /**
       * Defines a value that acts as an array of other values.
       */
      class Array: public ValueBase
      {
      public:
         /**
          * Constructor
          */
         Array()
         { }

         /**
          * Destructor
          */
         virtual ~Array()
         { values.clear(); }

         /**
          * @return Overloads the base class version to indicate that this is an array.
          */
         virtual ValueType get_type() const
         { return value_array; }

         /**
          * @return Overloads the base class version to generate a copy of this array.
          */
         virtual Array *clone() const;

         /**
          * Overloads the base class version to format this value to the specified stream.
          */
         virtual void format(
            std::ostream &out, bool do_indent = true, int indent_level = 0) const;

         /**
          * Overloads the base class version to parse the content from the specified stream.
          */
         virtual char parse(std::istream &in, char start_char = 0, int *row_no = 0, int *column_no = 0);

         // @group: definitions to act as a container

         /**
          * @return Returns the iterator at the start of the array,
          */
         typedef Csi::SharedPtr<ValueBase> value_type;
         typedef std::deque<value_type> values_type;
         typedef values_type::iterator iterator;
         typedef values_type::const_iterator const_iterator;
         iterator begin()
         { return values.begin(); }
         const_iterator begin() const
         { return values.begin(); }

         /**
          * @return Returns the iterator beyond the end of the array.
          */
         iterator end()
         { return values.end(); }
         const_iterator end() const
         { return values.end(); }

         /**
          * @return Returns the number of values in the array.
          */
         typedef values_type::size_type size_type;
         size_type size() const
         { return values.size(); }

         /**
          * @return Returns true if there are no values in the array.
          */
         bool empty() const
         { return values.empty(); }

         /**
          * Removes all values from the array.
          */
         void clear()
         { values.clear(); }

         /**
          * Appends the specified value to the end of the array.
          *
          * @param value Specifies the value to be added.
          */
         void push_back(value_type value)
         { values.push_back(value); }
         void push_back(StrAsc const &value)
         { values.push_back(value_type(new String(value))); }

         /**
          * Removes the last item in the array.
          */
         void pop_back()
         {
            if(!values.empty())
               values.pop_back();
         }

         /**
          * @return Returns a reference to the first value in the array.
          */
         value_type &front()
         { return values.front(); }
         value_type const &front() const
         { return values.front(); }

         /**
          * @return Returns a reference to the value at the end of the array.
          */
         value_type back()
         { return values.back(); }
         value_type const &back() const
         { return values.back(); }

         /**
          * Pushes the specified value to the front of the array.
          *
          * @param value Specifies the value to be added.
          */
         void push_front(value_type value)
         { values.push_front(value); }
         void push_front(StrAsc const &value)
         { values.push_front(value_type(new String(value))); }

         /**
          * Removes the value at the start of the array.
          */
         void pop_front()
         {
            if(!values.empty())
               values.pop_front();
         }
         
         /**
          * Removes the value identified by the specified iterator.
          *
          * @param id Specifies the iterator.
          */
         void erase(iterator i)
         { values.erase(i); }

         /**
          * @return Overloads the subscript operator to return the value at the specified index.
          *
          * @param idx Specifies the index.
          */
         value_type &operator [](uint4 idx)
         { return values.at(idx); }
         value_type const &operator [](uint4 idx) const
         { return values.at(idx); }
         value_type &at(uint4 idx)
         { return values.at(idx); }
         value_type const &at(uint4 idx) const
         { return values.at(idx); }
         
         // @endgroup

      private:
         /**
          * Specifies the values associated with this array.
          */
         values_type values;
      };
      typedef PolySharedPtr<ValueBase, Array> ArrayHandle;

   };
};


#endif
