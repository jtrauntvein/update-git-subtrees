/* Cora.CommonSettingTypes.h

   This header defines the "common" setting types that are used by various
   objects in the LgrNet interface.

   Copyright (C) 2000, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Thursday 09 January 2020
   Last Commit: $Date: 2020-02-19 13:39:29 -0600 (Wed, 19 Feb 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_CommonSettingTypes_h
#define Cora_CommonSettingTypes_h

#include "Cora.Setting.h"
#include "StrAsc.h"
#include "StrUni.h"
#include "Csi.LgrDate.h"
#include <list>
#include <map>
#include <set>
#include <iostream>


namespace Cora
{
   /**
    * Defines an object that represents a boolean setting.
    */
   class SettingBool: public Setting
   {
   public:
      SettingBool(uint4 setting_id, bool value_ = false):
         Setting(setting_id),
         value(value_)
      { }
      
      virtual bool read(Csi::Messaging::Message *msg);
      virtual bool read(char const *s);
      virtual void write(Csi::Messaging::Message *out) const;
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         json_value_handle rtn(new Csi::Json::Boolean(value));
         return rtn;
      }

      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::BooleanHandle val(val_);
            value = val->get_value();
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      }
            
      bool get_value() const
      { return value; }
      
      void set_value(bool value_)
      { value = value_; }

   private:
      bool value;
   };


   /**
    * Defines an object that represents a setting that holds a four byte unsigned integer.
    */
   class SettingUInt4: public Setting
   {
   public:
      SettingUInt4(uint4 setting_id, uint4 value_ = 0):
         Setting(setting_id),
         value(value_)
      { }

      virtual bool read(Csi::Messaging::Message *msg);
      
      virtual bool read(char const *s);
      
      virtual void write(Csi::Messaging::Message *out) const;
      
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         json_value_handle rtn(new Csi::Json::Number(value));
         return rtn;
      }

      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::NumberHandle val(val_);
            value = val->get_value_uint4();
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      }
      
      uint4 get_value() const { return value; }
      void set_value(uint4 value_) { value = value_; }

   private:
      uint4 value;
   };


   ////////////////////////////////////////////////////////////
   // class SettingByte
   ////////////////////////////////////////////////////////////
   class SettingByte: public Setting
   {
   public:
      SettingByte(uint4 setting_id, byte value_ = 0):
         Setting(setting_id),
         value(value_)
      { }

      virtual bool read(Csi::Messaging::Message *msg);
      virtual bool read(char const *s);
      virtual void write(Csi::Messaging::Message *out) const;
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const 
      {
         json_value_handle rtn(new Csi::Json::Number(value));
         return rtn;
      }
      virtual bool read_json(json_value_handle &val_) 
      {
         bool rtn(true);
         try
         {
            Csi::Json::NumberHandle val(val_);
            value = static_cast<byte>(val->get_value_int2());
         }
         catch (std::exception&)
         {
            rtn = false;
         }
         return rtn;
      }

      byte get_value() const { return value; }
      void set_value(byte value_) { value = value_; }

   private:
      byte value;
   };


   ////////////////////////////////////////////////////////////
   // class SettingUInt2
   ////////////////////////////////////////////////////////////
   class SettingUInt2: public Setting
   {
   public:
      SettingUInt2(uint4 setting_id, uint2 value_ = 0):
         Setting(setting_id),
         value(value_)
      { }

      virtual bool read(Csi::Messaging::Message *msg);
      virtual bool read(char const *s);
      virtual void write(Csi::Messaging::Message *out) const;
      virtual void format(std::ostream &out) const;
    
      virtual json_value_handle write_json() const
      {
         json_value_handle rtn(new Csi::Json::Number(value));
         return rtn;
      }

      virtual bool read_json(json_value_handle &val_) 
      {
         bool rtn(true);
         try 
         {
            Csi::Json::NumberHandle val(val_);
            value = val->get_value_int2();
         }
         catch (std::exception&)
         {
            return false;
         }
         return rtn;
      }
	  
      uint2 get_value() const { return value; }
      void set_value(uint2 value_) { value = value_; }
	
   private:
      uint2 value;
   };


   ////////////////////////////////////////////////////////////
   // class SettingInt4
   ////////////////////////////////////////////////////////////
   class SettingInt4: public Setting
   {
   public:
      SettingInt4(uint4 setting_id, int4 value_ = 0):
         Setting(setting_id),
         value(value_)
      { }

      virtual bool read(Csi::Messaging::Message *msg);
      virtual bool read(char const *s);
      virtual void write(Csi::Messaging::Message *out) const;
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         json_value_handle rtn(new Csi::Json::Number(value));
         return rtn;
      }
      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::NumberHandle val(val_);
            value = val->get_value_int4();
         }
         catch (std::exception &)
         {
            rtn = false;
         }
         return rtn;
      }

      int4 get_value() const { return value; }
      void set_value(int4 value_) { value = value_; }

   private:
      int4 value;
   };

   ////////////////////////////////////////////////////////////
   // class SettingStrAsc
   ////////////////////////////////////////////////////////////
   class SettingStrAsc: public Setting
   {
   public:
      SettingStrAsc(uint4 setting_id, StrAsc const &value_ = ""):
         Setting(setting_id),
         value(value_)
      { }

      virtual bool read(Csi::Messaging::Message *msg);
      virtual bool read(char const *s);
      virtual void write(Csi::Messaging::Message *out) const;
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         json_value_handle rtn(new Csi::Json::String(value));
         return rtn;
      }
      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::StringHandle val(val_);
            if(validate(val->get_value()))
               value = val->get_value();
            else
               rtn = false;
         }
         catch(std::exception &)
         {
            rtn = false;
         }
         return rtn;
      }

      StrAsc const &get_value() const { return value; }
      void set_value(StrAsc const &value_) { value = value_; }

   protected:
      /**
       * Can be overloaded to validate the specified value.
       */
      virtual bool validate(StrAsc const &val)
      { return true; }
      
   protected:
      StrAsc value;
   };


   ////////////////////////////////////////////////////////////
   // class SettingAsciiPassword
   ////////////////////////////////////////////////////////////
   class SettingAscPassword: public SettingStrAsc
   {
   private:
      ////////////////////////////////////////////////////////////
      // obscure
      ////////////////////////////////////////////////////////////
      bool obscure;
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      SettingAscPassword(
         uint4 setting_id, StrAsc const &value = ""):
         SettingStrAsc(setting_id, value),
         obscure(false)
      { }

      ////////////////////////////////////////////////////////////
      // format
      ////////////////////////////////////////////////////////////
      virtual void format(std::ostream &out) const;

      ////////////////////////////////////////////////////////////
      // obscure_passwords
      ////////////////////////////////////////////////////////////
      virtual void obscure_passwords()
      { obscure = true; }
   };


   ////////////////////////////////////////////////////////////
   // class SettingStrAscList
   ////////////////////////////////////////////////////////////
   class SettingStrAscList: public Setting
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      SettingStrAscList(uint4 setting_id):
         Setting(setting_id)
      { }

      ////////////////////////////////////////////////////////////
      // read (message)
      ////////////////////////////////////////////////////////////
      virtual bool read(Csi::Messaging::Message *message);

      ////////////////////////////////////////////////////////////
      // read (string)
      ////////////////////////////////////////////////////////////
      virtual bool read(char const *s);

      ////////////////////////////////////////////////////////////
      // write
      ////////////////////////////////////////////////////////////
      virtual void write(Csi::Messaging::Message *out) const;

      ////////////////////////////////////////////////////////////
      // format
      ////////////////////////////////////////////////////////////
      virtual void format(std::ostream &out) const;



      // @group: string container definitions

      ////////////////////////////////////////////////////////////
      // begin
      ////////////////////////////////////////////////////////////
      typedef StrAsc value_type;
      typedef std::list<value_type> strings_type;
      typedef strings_type::iterator iterator;
      typedef strings_type::const_iterator const_iterator;
      iterator begin()
      { return strings.begin(); }
      const_iterator begin() const
      { return strings.begin(); }

      ////////////////////////////////////////////////////////////
      // end
      ////////////////////////////////////////////////////////////
      iterator end()
      { return strings.end(); }
      const_iterator end() const
      { return strings.end(); }

      ////////////////////////////////////////////////////////////
      // empty
      ////////////////////////////////////////////////////////////
      bool empty() const
      { return strings.empty(); }

      ////////////////////////////////////////////////////////////
      // size
      ////////////////////////////////////////////////////////////
      typedef strings_type::size_type size_type;
      size_type size() const
      { return strings.size(); }
      
      virtual json_value_handle write_json() const
      {
         Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
         for(const_iterator si = begin(); si != end(); ++si)
            rtn->push_back(*si);
         return rtn.get_handle();
      }

      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::ArrayHandle val(val_);
            strings.clear();
            for (Csi::Json::Array::iterator it = val->begin(); it != val->end(); it++)
            {
               Csi::Json::StringHandle stringVal(*it);
               strings.push_back(stringVal->get_value());
            }

         }
         catch (std::exception&)
         {
            rtn = false;
         }
         return rtn;
      }
      // @endgroup
      
   private:
      ////////////////////////////////////////////////////////////
      // strings
      ////////////////////////////////////////////////////////////
      strings_type strings; 
   };
   


   ////////////////////////////////////////////////////////////
   // class SettingStrUni
   ////////////////////////////////////////////////////////////
   class SettingStrUni: public Setting
   {
   public:
      SettingStrUni(uint4 setting_id, StrUni const &value_ = L""):
         Setting(setting_id),
         value(value_)
      { }

      virtual bool read(Csi::Messaging::Message *msg);
      virtual bool read(char const *s);
      virtual void write(Csi::Messaging::Message *out) const;
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         json_value_handle rtn(new Csi::Json::String(value));
         return rtn;
      }
      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::StringHandle val(val_);
            value = val->get_value();
         }	
         catch(std::exception &)
         {
            rtn = false;
         }
         return rtn;
      }

      StrUni const &get_value() const { return value; }
      void set_value(StrUni const &value_) { value = value_; }
      
   private:
      StrUni value;
   };


   ////////////////////////////////////////////////////////////
   // class SettingNameSet
   ////////////////////////////////////////////////////////////
   class SettingNameSet:
      public Setting,
      public std::list<StrUni>
   {
   public:
      SettingNameSet(uint4 identifier);
      virtual ~SettingNameSet();
      virtual bool read(Csi::Messaging::Message *in);
      virtual bool read(char const *s);
      virtual void write(Csi::Messaging::Message *out) const;
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
         for(const_iterator si = begin(); si != end(); ++si)
            rtn->push_back(new Csi::Json::String(si->to_utf8()));
         return rtn.get_handle();
      }

      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::ArrayHandle val(val_);
            for (Csi::Json::Array::iterator it = val->begin(); it != val->end(); it++)
            {
               Csi::Json::StringHandle Stringval(*it);
               push_back(Stringval->get_value());
            }
         }
         catch (std::exception&)
         {
            rtn = false;
         }
         return rtn;
      }

      bool has_name(StrUni const &name) const;
   };


   ////////////////////////////////////////////////////////////
   // class SettingAsciiNameSet
   ////////////////////////////////////////////////////////////
   class SettingAsciiNameSet:
      public Setting,
      public std::list<StrAsc>
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      SettingAsciiNameSet(uint4 identifier);

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~SettingAsciiNameSet();

      ////////////////////////////////////////////////////////////
      // read (message)
      ////////////////////////////////////////////////////////////
      virtual bool read(Csi::Messaging::Message *in);

      ////////////////////////////////////////////////////////////
      // read (string)
      ////////////////////////////////////////////////////////////
      virtual bool read(char const *s);

      ////////////////////////////////////////////////////////////
      // write (message) 
      ////////////////////////////////////////////////////////////
      virtual void write(Csi::Messaging::Message *out) const;

      ////////////////////////////////////////////////////////////
      // format
      ////////////////////////////////////////////////////////////
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
         for(const_iterator si = begin(); si != end(); ++si)
            rtn->push_back(*si);
         return rtn.get_handle();
      }

      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::ArrayHandle val(val_);
            clear();
            for (Csi::Json::Array::iterator it = val->begin(); it != val->end(); it++)
            {
               Csi::Json::StringHandle stringVal(*it);
               push_back(stringVal->get_value());
            }
         }
         catch (std::exception&)
         {
            rtn = false;
         }
         return rtn;
      }
   };


   ////////////////////////////////////////////////////////////
   // class SettingEnumeration
   //
   // Base class for a setting that represents an enumeration, a uint31 that
   // has a strictly limited set of values. A derived class' constructor is
   // expected to fill in the values that are to be expected before the read or
   // format methods are invoked.
   ////////////////////////////////////////////////////////////
   class SettingEnumeration: public Setting
   {
   protected:
      ////////////////////////////////////////////////////////////
      // value
      //
      // The current value of the setting
      ////////////////////////////////////////////////////////////
      uint4 value;

      ////////////////////////////////////////////////////////////
      // supported_values
      //
      // The list of values that are supported by this setting along with the
      // names associated with those values.
      ////////////////////////////////////////////////////////////
      typedef std::map<uint4, StrAsc> supported_values_type;
      supported_values_type supported_values;

   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      SettingEnumeration(uint4 identifier);

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~SettingEnumeration();

      ////////////////////////////////////////////////////////////
      // read (message)
      ////////////////////////////////////////////////////////////
      virtual bool read(Csi::Messaging::Message *in);

      ////////////////////////////////////////////////////////////
      // read (string)
      ////////////////////////////////////////////////////////////
      virtual bool read(char const *s);

      ////////////////////////////////////////////////////////////
      // write (message)
      ////////////////////////////////////////////////////////////
      virtual void write(Csi::Messaging::Message *out) const;

      ////////////////////////////////////////////////////////////
      // format
      ////////////////////////////////////////////////////////////
      virtual void format(std::ostream &out) const;


      virtual json_value_handle write_json() const
      {
         Csi::Json::ObjectHandle rtn(new Csi::Json::Object);
         rtn->set_property_number("value", value);
         rtn->set_property_str("value_str", supported_values.at(value));
         return rtn.get_handle();
      }

      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            if (val_->get_type() == Csi::Json::value_object)
            {
               Csi::Json::ObjectHandle val(val_);
               set_value(static_cast<uint4>(val->get_property_number("value")));
            }
            else if (val_->get_type() == Csi::Json::value_number)
            {
               Csi::Json::NumberHandle val(val_);
               set_value(val->get_value_uint4());
            }
            else if (val_->get_type() == Csi::Json::value_string)
            {
               Csi::Json::StringHandle val(val_);
               rtn = read(val->get_value().c_str());
            }
            else
            {
               rtn = false;
            }
         }
         catch (std::exception&)
         {
            rtn = false;
         }

         return rtn;
      }

      ////////////////////////////////////////////////////////////
      // get_value
      ////////////////////////////////////////////////////////////
      uint4 get_value() const
      { return value; }

      ////////////////////////////////////////////////////////////
      // set_value
      ////////////////////////////////////////////////////////////
      void set_value(uint4 value_)
      {
         if(supported_values.find(value_) != supported_values.end())
            value = value_;
         else
            throw std::invalid_argument("Invalid enumerated setting value");
      }
   };


   ////////////////////////////////////////////////////////////
   // class TimeSetting
   //
   // Setting that uses a time stamp for its representation.
   ////////////////////////////////////////////////////////////
   class TimeSetting: public Setting
   {
   public:
      ////////////////////////////////////////////////////////////
      // time
      ////////////////////////////////////////////////////////////
      Csi::LgrDate time;
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      TimeSetting(uint4 setting_id): Setting(setting_id)
      { }
      
      ////////////////////////////////////////////////////////////
      // format
      ////////////////////////////////////////////////////////////
      virtual void format(std::ostream &out) const
      { time.format(out, "%Y-%m-%d %H:%M:%S%x"); }
      
      ////////////////////////////////////////////////////////////
      // read
      ////////////////////////////////////////////////////////////
      virtual bool read(char const *s)
      {
         bool rtn = true;
         try
         {
            time = Csi::LgrDate::fromStr(s);
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      }
      
      ////////////////////////////////////////////////////////////
      // read
      ////////////////////////////////////////////////////////////
      virtual bool read(Csi::Messaging::Message *in);
      
      ////////////////////////////////////////////////////////////
      // write
      ////////////////////////////////////////////////////////////
      virtual void write(Csi::Messaging::Message *out) const;

      virtual json_value_handle write_json() const
      {
         json_value_handle rtn(new Csi::Json::Date(time));
         return rtn;
      }

      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::StringHandle val(val_);
            time = Csi::LgrDate::fromStr(val->get_value().c_str());
         }
         catch (std::exception&)
         {
            rtn = false;
         }
         return rtn;
      }


   };


   /**
    * Defines a setting object that represents a set of unsigned 32 bit integers.
    */
   class SettingUInt4Set: public Setting, public std::set<uint4>
   {
   public:
      /**
       * construct with setting ID
       *
       * @param setting_id the identifier for this setting.
       */
      SettingUInt4Set(uint4 setting_id):
         Setting(setting_id)
      { }

      /**
       * destructor
       */
      virtual ~SettingUInt4Set()
      { }

      /**
       * read from string.
       *
       * @param s the nul terminated string to read
       */
      virtual bool read(char const *s);

      /**
       * read from a message
       *
       * @param message the message from which to read.
       */
      virtual bool read(Csi::Messaging::Message *in);

      /**
       * write to message
       *
       * @param message The message to which we will write
       */
      virtual void write(Csi::Messaging::Message *out) const;

      /**
       * format the message to a stream
       *
       * @param out the stream to which we will be formatted.
       */
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
         for(set<uint4>::iterator si = begin(); si != end(); ++si)
            rtn->push_back(new Csi::Json::Number(*si));
         return rtn.get_handle();
      }

      virtual bool read_json(json_value_handle &val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::ArrayHandle val(val_);
            for (Csi::Json::Array::iterator it = val->begin(); it != val->end(); it++)
            {
               Csi::Json::NumberHandle numVal(*it);
               set<uint4>::insert(numVal->get_value_uint4());
            }
         }
         catch (std::exception&)
         {
            rtn = false;
         }
         return rtn;
      }
   };


   ////////////////////////////////////////////////////////////
   // class SettingInt8
   ////////////////////////////////////////////////////////////
   class SettingInt8: public Setting
   {
   private:
      ////////////////////////////////////////////////////////////
      // value
      ////////////////////////////////////////////////////////////
      int8 value;
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      SettingInt8(uint4 setting_id, int8 value_ = 0):
         Setting(setting_id),
         value(value_)
      { }

      ////////////////////////////////////////////////////////////
      // read
      ////////////////////////////////////////////////////////////
      virtual bool read(Csi::Messaging::Message *message);

      ////////////////////////////////////////////////////////////
      // read (string)
      ////////////////////////////////////////////////////////////
      virtual bool read(char const *s);

      ////////////////////////////////////////////////////////////
      // write
      ////////////////////////////////////////////////////////////
      virtual void write(Csi::Messaging::Message *out) const;

      ////////////////////////////////////////////////////////////
      // format
      ////////////////////////////////////////////////////////////
      virtual void format(std::ostream &out) const;

      virtual json_value_handle write_json() const
      {
         json_value_handle rtn(new Csi::Json::Number(static_cast<double>(value)));
         return rtn;
      }

      virtual bool read_json(json_value_handle& val_)
      {
         bool rtn(true);
         try
         {
            Csi::Json::NumberHandle val(val_);
            value = val->get_value_int8();

         }
         catch (std::exception&)
         {
            rtn = false;
         }
         return rtn;
      }

      ////////////////////////////////////////////////////////////
      // get_value
      ////////////////////////////////////////////////////////////
      int8 get_value() const
      { return value; }

      ////////////////////////////////////////////////////////////
      // set_value
      ////////////////////////////////////////////////////////////
      void set_value(int8 value_)
      { value = value_; }
      
   };


   /**
    * Defines a setting that maintains its content as a JSON structure.
    */
   class JsonString: public Setting
   {
   private:
      /**
       * Specifies the JSON structure for this setting.
       */
      Csi::Json::ObjectHandle content;

   public:
      /**
       * Constructor
       *
       * @param identifier_ Specifies the identifier for this setting.
       *
       * @param content_ Specifies the content for this setting.  If null  the content will be
       * initialised as an empty structure.
       */
      JsonString(uint4 identifier_, Csi::Json::ObjectHandle content_ = nullptr):
         Setting(identifier_)
      {
         if(content_ != nullptr)
            content = content;
         else
            content.bind(new Csi::Json::Object);
      }

      /**
       * Destructor
       */
      virtual ~JsonString()
      { content.clear(); }

      /**
       * Overloads the base class version to format the content to the specified stream.
       */
      virtual void format(std::ostream &out) const
      { content->format(out); }

      /**
       * Overloads the base class version to read the setting from the specified string.
       */
      virtual bool read(char const *source);

      /**
       * Overloads the base class version to read the content from the specified message.
       */
      virtual bool read(Csi::Messaging::Message *message);

      /**
       * Overloads the base class version to write the content to the specified message.
       */
      virtual void write(Csi::Messaging::Message *message) const;

      /**
       * Overloads the base class to return the content.
       */
      virtual json_value_handle write_json() const
      {
         Csi::Json::ObjectHandle rtn(content->clone());
         return rtn.get_handle();
      }

      /**
       * Overloads the base class version to read the content from the specified JSON value.
       */
      virtual bool read_json(json_value_handle &value)
      {
         bool rtn(value->get_type() == Csi::Json::value_object);
         if(rtn)
            content = value;
         return rtn;
      }

      /**
       * @return Returns the content for this setting.
       */
      Csi::Json::ObjectHandle &get_content()
      { return content; }
      Csi::Json::ObjectHandle const &get_content() const
      { return content; }

      /**
       * @param content_ Specifies the content.
       */
      void set_content(Csi::Json::ObjectHandle content_)
      {
         if(content != nullptr)
            content = content_;
         else
            content.bind(new Csi::Json::Object);
      }
   };
}; 

#endif
