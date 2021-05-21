/* Cora.Setting.h

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Saturday 19 October 2019
   Last Commit: $Date: 2019-10-21 07:50:04 -0600 (Mon, 21 Oct 2019) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Setting_h
#define Cora_Setting_h

#include "CsiTypeDefs.h"
#include "Csi.Json.h"
#include <iosfwd>


namespace Csi
{
   namespace Messaging
   {
      class Message;
   };
};


namespace Cora
{
   /**
    * Defines a base class for all objects that will represent settings for various type of
    * components that deal with LoggerNet server settings.
    */
   class Setting
   {
   public:
      /**
       * Constructor
       *
       * @param identifier_ Specifies the identifier for this setting.
       */
      enum set_outcome_type
      {
         outcome_no_attempt_made = 0,
         outcome_set = 1,
         outcome_read_only = 2,
         outcome_setting_locked = 3,
         outcome_invalid_value = 4,
         outcome_unsupported = 5,
         outcome_network_locked = 6,
      };
      Setting(uint4 identifier_):
         identifier(identifier_),
         set_outcome(outcome_no_attempt_made)
      { }

      /**
       * Destructor
       */
      virtual ~Setting()
      { }

      /**
       * @return Returns the identifier for this setting.
       */
      uint4 get_identifier() const
      { return identifier; }

      /**
       * @return Returns the outcome from the last attempt to set the server setting using this
       * object.
       */
      set_outcome_type get_set_outcome() const
      { return set_outcome; }

      /**
       * @param set_outcome_ Specifies the outcome for attempting to set this setting.
       */
      void set_set_outcome(set_outcome_type set_outcome_)
      { set_outcome = set_outcome_; }

      /**
       * Must be overloaded to format this setting value as text to the specified stream.
       *
       * @param out Specifies the stream to which the setting will be formatted.
       */
      virtual void format(std::ostream &out) const = 0;

      /**
       * Must be overloaded to read this setting value from the text in the specified string.
       *
       * @param source Specifies the string to parse.
       *
       * @return Returns true if the setting was read.
       */
      virtual bool read(char const *source) = 0;

      /**
       * Must be overloaded to read this setting from the specified message object.
       *
       * @param message Specifies the message to read.
       *
       * @return Returns true if the setting was read.
       */
      virtual bool read(Csi::Messaging::Message *message) = 0;

      /**
       * Must be overloaded to write this setting value to the specified message object.
       *
       * @param message Specifies the message to read.
       */
      virtual void write(Csi::Messaging::Message *message) const = 0;

      /**
       * @return Must be overloaded to return this setting formatted as a value that can be included
       * in a JSON object.
       */
      typedef Csi::SharedPtr<Csi::Json::ValueBase> json_value_handle;
      virtual json_value_handle write_json() const = 0;

      /**
       * Reads the value for this setting from the specified value handle.
       *
       * @param value Specifies the value handle to read.
       *
       * @return Must return true if the value was successfully parsed.
       */
      virtual bool read_json(json_value_handle &value) = 0;

      /**
       * Can be overloaded by a derived class to change the formatting behaviour for settings that
       * represent passwords.
       */
      virtual void obscure_passwords()
      { }

      /**
       * Formats the specified set outcome code to the specified stream.
       *
       * @param out Specifies the output stream.
       *
       * @param outcome Specifies the outcome to describe.
       */
      static void format_set_outcome(std::ostream &out, set_outcome_type outcome);

   private:
      /**
       * Specifies the identifier for this setting.
       */
      uint4 identifier;

      /**
       * Specifies the outcome of the last attempt to use this setting object to change the server
       * setting.
       */
      set_outcome_type set_outcome;
   };
};

#endif
