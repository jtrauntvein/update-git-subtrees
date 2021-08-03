/* Csi.DevConfig.SettingComp.DescBase.h

   Copyright (C) 2003, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 17 December 2003
   Last Change: Monday 02 December 2019
   Last Commit: $Date: 2021-04-20 12:42:24 -0600 (Tue, 20 Apr 2021) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_DescBase_h
#define Csi_DevConfig_SettingComp_DescBase_h

#include "StrAsc.h"
#include "CsiTypeDefs.h"
#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.Xml.Element.h"
#include "Csi.Json.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines an object that represents the application's ability to edit a setting component.
          */
         class EditorBase
         {
         public:
            /**
             * Destructor
             */
            virtual ~EditorBase()
            { }
         };
         
         
         /**
          * Defines a base class for all setting component description objects.  This includes
          * management of common attributes for all component types.
          */
         class DescBase
         {
         protected:
            /**
             * Specifies the name of the component.
             */
            StrAsc name;

            /**
             * Specifies the help for this component.
             */
            StrAsc description;

            /**
             * Specifies the original partial URL for the description if the reference is a file.
             */
            StrAsc description_url;

            /**
             * Specifies the prefix that should be presented before this component when the setting
             * is being formatted as text.
             */
            StrAsc format_prefix;

            /**
             * Specifies the string that should follow this component value when the setting is
             * being formatted as text.
             */
            StrAsc format_postfix;

            /**
             * Set to true if the description string is a file reference.
             */
            bool description_is_file;

            /**
             * Identifies the type of this component.
             */
            uint4 const component_type;

            /**
             * Specifies the source for the expression that will be evaluated in order for the
             * settings editor control(s) to be enabled.
             */
            StrAsc enable_expr;

            /**
             * Set to true if this component cannot be changed.
             */
            bool read_only;

            /**
             * Specifies the collection of rules that can be used to validate this component's value
             * in the context of other setting/component values.
             */
         public:
            typedef std::pair<StrAsc, StrAsc> rule_type;
            typedef std::list<rule_type> rules_type;
         protected:
            rules_type rules;

            /**
             * Specifies the string that can be used to set this component to a default value.
             */
            StrAsc default_value;

            /**
             * Specifies the value that will be written when the component will be written to a
             * configuration summary.
             */
            StrAsc summary_value;
            bool has_summary_value;

            /**
             * Specifies an identifier that can be used to distinguish this component from other
             * components in the same setting.  This is similar to the common-name that can be
             * assigned to a setting but the context is more constricted.  Unlike the name, this
             * value cannot be changed through translation so it can be counted on uniquely identify
             * the component within a repeat level.
             */
            StrAsc common_name;
            
         public:
            /**
             * Constructor
             *
             * @param component_type_ Specifies the type code for this component.
             */
            DescBase(uint4 component_type_):
               description_is_file(false),
               component_type(component_type_),
               read_only(false),
               has_summary_value(false)
            { }

            /**
             * Destructor
             */
            virtual ~DescBase()
            { }

            /**
             * @return Returns the name of this component.
             */
            StrAsc const &get_name() const
            { return name; }

            /**
             * @return Returns this component's common name.
             */
            StrAsc const &get_common_name() const
            { return  common_name; }

            /**
             * @return Returns the description string (either the text or file reference).
             */
            StrAsc const &get_description() const
            { return description; }

            /**
             * @return Returns the original description URL that was given in the XML.
             */
            StrAsc const &get_description_url() const
            { return description_url; }

            /**
             * @return Returns true if the description is a file reference.
             */
            bool get_description_is_file() const
            { return description_is_file; }

            /**
             * @return Returns the string that should precede this component's value when its
             * setting is being formatted as text.
             */
            StrAsc const &get_format_prefix() const
            { return format_prefix; }

            /**
             * @param format_prefix_ Specifies the string that should precede the component's value
             * when the setting is being formatted as text.
             */
            void set_format_prefix(StrAsc const &format_prefix_)
            { format_prefix = format_prefix_; }

            /**
             * @return Returns the string that should follow the component value when the setting is
             * being formatted as text.
             */
            StrAsc const &get_format_postfix() const
            { return format_postfix; }

            /**
             * @param format_postfix_ Specifies the string that should follow the component's value
             * when the setting is formatted as text.
             */
            void set_format_postfix(StrAsc const &format_postfix_)
            { format_postfix = format_postfix_; }

            /**
             * @return Returns the source string for the enable expression.
             */
            StrAsc const &get_enable_expr() const
            { return enable_expr; }

            /**
             * @return Returns a component object that matches the type of this description.
             *
             * @param previous_component Specifies the component that was allocated for the setting
             * before this call was made.  Will be null if this is the first component created.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous_component) = 0;

            /**
             * Reads this component's description from the given XML structure.
             *
             * @param xml_data Specifies the XML element that should describe this component.
             * @param library_dir Specifies the base directory for the library.
             */
            virtual void init_from_xml(
               Csi::Xml::Element &xml_data, StrAsc const &library_dir);

            /**
             * Can be overloaded to configure the specified component editor.
             */
            virtual void configure_editor(EditorBase *editor)
            { }

            /**
             * Can be overloaded to indicate that this component has min and max values.
             */
            virtual bool has_maxima() const
            { return false; }

            /**
             * Formats the max value to the given stream.
             */
            virtual void format_max(std::ostream &out) const
            { }

            /**
             * Formats the min value to the given stream.
             */
            virtual void format_min(std::ostream &out) const
            { }

            /**
             * @return Returns a numeric identifier for this component type.
             */
            uint4 get_component_type() const
            { return component_type; }

            /**
             * Can be overloaded to return the maximum number of significant digits that should be
             * used when the component value is being formatted as text.
             */
            virtual int get_significant_digits() const
            { return 0; }

            /**
             * @return Can be overloaded to specify that this component should be protected by not
             * exposing its value to the user.
             */
            virtual bool has_protected_value() const
            { return false; }

            /**
             * @return Returns true if this setting component cannot be changed.
             */
            bool get_read_only() const
            { return read_only; }

            /**
             * @return Returns the set of validation rules for this component.
             */
            rules_type const &get_rules() const
            { return rules; }

            /**
             * @return Returns the default value from the description.
             */
            StrAsc const &get_default_value() const
            { return default_value; }

            /**
             * @return Returns true if this description has a summary value.
             */
            bool get_has_summary_value() const
            { return has_summary_value; }

            /**
             * @return Returns the summary value that was specified in the component description.
             */
            StrAsc const &get_summary_value() const
            { return summary_value; }

            /**
             * @return Returns true if this component type should be presented in a configuration
             * summary.
             */
            virtual bool present_in_summary() const
            { return true; }

            /**
             * Writes the description parameters to the specified JSON object.  If this version is
             * overloaded, thge overloaded version must call this version as well.
             */
            virtual void describe_json(Csi::Json::Object &desc_json);
         };
      };
   };
};


#endif
