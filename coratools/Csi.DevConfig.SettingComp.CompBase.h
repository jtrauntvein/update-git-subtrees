/* Csi.DevConfig.SettingComp.CompBase.h

   Copyright (C) 2003, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 17 December 2003
   Last Change: Thursday 01 March 2018
   Last Commit: $Date: 2018-03-01 13:53:28 -0600 (Thu, 01 Mar 2018) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_CompBase_h
#define Csi_DevConfig_SettingComp_CompBase_h

#include <iostream>
#include "Csi.SharedPtr.h"
#include "Csi.DevConfig.Message.h"
#include "Csi.Expression.ExpressionHandler.h"
#include "Csi.Json.h"
#include <list>


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         //@group class forward declarations
         class DescBase;
         class EditorBase;
         //@endgroup


         /**
          * Defines an exception that is thrown when a conversion to a given format is not
          * supported.
          */
         class InvalidComponentConversion: public std::exception
         {
         public:
            /**
             * Constructor
             */
            InvalidComponentConversion()
            { }
            
            /**
             * @return Overloads the base class version to report the error.
             */
            virtual char const *what() const throw ()
            { return "Unsupported setting component conversion"; }
         };
         

         /**
          * Defines a base class for all objects that will represent a component object for a
          * devconfig setting.
          */
         class CompBase
         {
         protected:
            /**
             * Specifies the description for this component.
             */
            SharedPtr<DescBase> desc;

            /**
             * Set to true if this component has been changed by the application.
             */
            bool has_changed;

            /**
             * Set to true if this component is considered read-only.
             */
            bool read_only;

            /**
             * Specifies the number of times that this component has repeated within the setting.
             */
            uint4 repeat_count;

            /**
             * Specifies the device major version.
             */
            byte major_version;

            /**
             * Specifies the device minor version.
             */
            byte minor_version;

            /**
             * Specifies the collection of validation rules that are applied to this component.
             */
         public:
            typedef SharedPtr<Expression::ExpressionHandler> expression_handle;
            typedef std::pair<expression_handle, StrAsc> validate_rule;
            typedef std::list<validate_rule> validate_rules_type;
         protected:
            validate_rules_type validate_rules;
            
         public:
            /**
             * Constructor
             *
             * @param desc_ Specifies the description for this component.
             */
            CompBase(SharedPtr<DescBase> &desc_);

            /**
             * Destructor
             */
            virtual ~CompBase();

            /**
             * @return Returns the name of this component from the description.
             */
            StrAsc const &get_name() const;

            /**
             * @return Returns the description text for this component.
             */
            StrAsc const &get_desc_text() const;

            /**
             * @return Returns true if the description text is a file reference.
             */
            bool get_desc_text_is_file() const;

            /**
             * @return Returns the string that should precede this component when being formatted to
             * a stream.
             */
            StrAsc const &get_format_prefix() const;

            /**
             * @return Returns the string that should follow this component when being formatted to
             * a stream.
             */
            StrAsc const &get_format_postfix() const;

            /**
             * @return Returns true if the component is read-only.
             */
            bool get_read_only() const;

            /**
             * @param read_only_ set to true if the component is read-only.
             */
            void set_read_only(bool read_only_)
            { read_only = read_only_; }
            
            /**
             * @return Returns a reference to the description object.
             */
            SharedPtr<DescBase> get_desc() const;

            /**
             * @return Returns true if this component has been changed by the application.
             */
            bool get_has_changed() const
            { return has_changed; }

            /**
             * @param has_changed_ Set to true if this component has been changed by the
             * application.
             */
            void set_has_changed(bool has_changed_)
            { has_changed = has_changed_; }

            /**
             * @return Returns the repeat level for this component.
             */
            uint4 get_repeat_count() const
            { return repeat_count; }

            /**
             * @param repeat_count_ Sets the repeat level for this component.
             */
            void set_repeat_count(uint4 repeat_count_)
            { repeat_count = repeat_count_; }

            /**
             * Configures the editor associated with this component.
             *
             * @param editor Specifies the editor created for this component.
             */
            void configure_editor(EditorBase *editor);

            /**
             * Reads the content of this component from the specified message.
             *
             * @param message Specifies the message to read.
             */
            virtual void read(SharedPtr<Message> &message) = 0;

            /**
             * Writes this component to the specified message.
             *
             * @param message Specifies the message to be written.
             */
            virtual void write(SharedPtr<Message> &message) = 0;
            
            /**
             * Formats the component to the specified stream.
             *
             8 @param out Specifies the stream to write.
             *
             * @param translate Set to true if the component value should be formatted according to
             * current locale and with user friendly values.
             */
            virtual void output(std::ostream &out, bool translate) = 0;
            virtual void output(std::wostream &out, bool translate) = 0;

            /**
             * Reads the component from the specified stream.
             *
             * @param in Specifies the stream to read.
             *
             * @param translate Set to true if current locale should be used or whether user
             * friendly values are expected.
             */
            virtual void input(std::istream &in, bool translate) = 0;

            /**
             * Adds the value for this component to the specified JSON array.  This may be a no-op
             * for some component types such as bitfields that share a common buffer value.
             *
             * @param components_json Specifies the array that should be written.
             */
            virtual void write(Json::Array &components_json) = 0;

            /**
             * Loads the value of this component from the given JSON array iterator.
             *
             * @return Returns the next iterator that will be available to read (generally the
             * current plus one).
             *
             * @param current Specifies the current position within the array of component values.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current) = 0;
            
            /**
             * Copies the content of the specified component to this component.
             *
             * @param other Specifies the component to be copied.
             */
            void copy(CompBase *other)
            {
               if(other->desc == desc)
               {
                  do_copy(other);
                  has_changed = true;
               }
               else
                  throw std::invalid_argument("Cannot copy unrelated component types");
            }

            /**
             * @return Returns true if this component has min and max values.
             */
            bool has_maxima() const;

            /**
             * Formats the maximum value to the specified stream.
             *
             * @param out Specifies the stream to write.
             */
            void format_max(std::ostream &out) const;

            /**
             * Formats the minimum value to the stream.
             *
             * @param out Specifies the stream to write.
             */
            void format_min(std::ostream &out) const;

            /**
             * @return Returns the maximum number of significant digits.
             */
            int get_significant_digits() const;

            /**
             * @return Returns a code that identifies the type of this component.
             */
            uint4 get_component_type() const;

            /**
             * @return Returns the source for this component's enable expression.
             */
            StrAsc const &get_enable_expr() const;

            /**
             * @return Returns the device major version.
             */
            byte get_major_version() const
            { return major_version; }

            /**
             * @return Returns the device minor version.
             */
            byte get_minor_version() const
            { return minor_version; }

            /**
             * Sets the major and minor versions.
             */
            void set_version(byte major_version_, byte minor_version_)
            {
               major_version = major_version_;
               minor_version = minor_version_;
            }

            /**
             * @return Returns tru if this component has a value that must be protected from
             * exposure.
             */
            bool has_protected_value() const;

            /**
             * @return Returns the collection of sources and messages for validation rules.
             */
            typedef std::pair<StrAsc, StrAsc> rule_source_type;
            typedef std::list<rule_source_type> rule_sources_type;
            rule_sources_type const &get_rule_sources() const;

            /**
             * @return Returns the collection of validation rules.
             */
            validate_rules_type &get_validate_rules()
            { return validate_rules; }
               
            // @group: value setters and getters

            // The following methods can be overloaded by derived classes to
            // provide access to the internal values of the component.   This
            // will allow the application a means of accessing the values
            // without performing upcasts or having to use an intermediate
            // string representation.

            /**
             * @return Returns the value for this setting cast as a single byte unsigned integer.
             *
             * @throw InvalidComponentConversion when the conversion is not supported.
             */
            virtual byte get_val_byte()
            { throw InvalidComponentConversion(); }

            /**
             * @param val Sets the value for this setting cast as a single byte unsigned integer.
             *
             * @throw InvalidComponentConversion when the conversion is not supported.
             */
            virtual void set_val_byte(byte val)
            { throw InvalidComponentConversion(); }

            /**
             * @return Returns the component value as a two byte signed integer.
             *
             * @throw InvalidComponentConversion when a conversion is not supported.
             */
            virtual int2 get_val_int2()
            { throw InvalidComponentConversion(); }

            /**
             * @param val Specifies the value for this component as a two byte signed integer.
             */
            virtual void set_val_int2(int2 val)
            { throw InvalidComponentConversion(); }

            /**
             * @return Returns the component value as a two byte unsigned integer.
             *
             * @throw InvalidComponentConversion when a conversion is not supported.
             */
            virtual uint2 get_val_uint2()
            { throw InvalidComponentConversion(); }

            /**
             * @param val Specifies the value for this component as a two byte unsigned integer.
             */
            virtual void set_val_uint2(uint2 val)
            { throw InvalidComponentConversion(); }

            /**
             * @return Returns the component value as a four byte signed integer.
             *
             * @throw InvalidComponentConversion when a conversion is not supported.
             */
            virtual int4 get_val_int4()
            { throw InvalidComponentConversion(); }

            /**
             * @param val Specifies the value for this component as a four byte signed integer.
             */
            virtual void set_val_int4(int4 val)
            { throw InvalidComponentConversion(); }

            /**
             * @return Returns the component value as a four byte unsigned integer.
             *
             * @throw InvalidComponentConversion when a conversion is not supported.
             */
            virtual uint4 get_val_uint4()
            { throw InvalidComponentConversion(); }

            /**
             * @param val Specifies the value for this component as a four byte unsigned integer.
             */
            virtual void set_val_uint4(uint4 val)
            { throw InvalidComponentConversion(); }

            /**
             * @return Returns the component value as an eight byte unsigned integer.
             *
             * @throw InvalidComponentConversion when a conversion is not supported.
             */
            virtual int8 get_val_int8()
            { return get_val_int4(); }

            /**
             * @param val Specifies the value for this component as an eight byte signed integer.
             */
            virtual void set_val_int8(int8 val)
            { throw InvalidComponentConversion(); }

            /**
             * @return Returns the component value as a four byte floating point.
             */
            virtual float get_val_float()
            { throw InvalidComponentConversion(); }

            /**
             * @param val Specifies the value for the component as a four byte floating point.
             */
            virtual void set_val_float(float val)
            { throw InvalidComponentConversion(); }

            /**
             * @return Returns the value for the component as an eight byte floating point.
             */
            virtual double get_val_double()
            { throw InvalidComponentConversion(); }

            /**
             * @param val Specifies the value of the component as an eight byte floating point.
             */
            virtual void set_val_double(double val)
            { throw InvalidComponentConversion(); }

            /**
             * @return Returns the untranslated value of the component as a string.
             */
            virtual StrAsc get_val_str();

            /**
             * @param val Specifies the untranslated value of the component.
             *
             * @param apply_checks Set to true if validation rules are to be applied.
             */
            virtual void set_val_str(StrAsc const &val, bool apply_checks = false);

            // @endgroup
            
         protected:
            /**
             * Copies the value from the other component to the value for this component.
             *
             * @param other Specifies the component to copy.  In order for this to succeed, both
             * components must be constructed using the same description.
             */
            virtual void do_copy(CompBase *other) = 0;
         };
      };
   };
};


#endif
