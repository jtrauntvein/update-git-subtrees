/* Csi.DevConfig.SettingDesc.h

   Copyright (C) 2003, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 16 December 2003
   Last Change: Friday 06 October 2017
   Last Commit: $Date: 2017-10-06 17:09:16 -0600 (Fri, 06 Oct 2017) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingDesc_h
#define Csi_DevConfig_SettingDesc_h

#include "Csi.SharedPtr.h"
#include "Csi.DevConfig.SettingComp.Factory.h"
#include "Csi.Xml.Element.h"
#include <deque>


namespace Csi
{
   namespace DevConfig
   {
      /**
       * Defines an object that describes a DevConfig setting.
       */
      class SettingDesc
      {
      private:
         /**
          * Specifies the name assigned to this setting.
          */
         StrAsc name;

         /**
          * Specifies a unique identifier for this setting within the setting catalogue.
          */
         uint2 identifier;

         /**
          * Set to true if this setting should not be changed by the user.
          */
         bool read_only;

         /**
          * Specifies the maximum number of times that the components for this setting should be
          * allowed to repeat.  A value of one (the default) will indicate that the components
          * should only appear once.
          */
         uint4 repeat_count;

         /**
          * Specifies the text for the default value for this setting.  This
          * member will be an empty string if there is no default value
          * specified.  This member will really only be of use when the
          * DevConfig emulation layer is used on top of string based settings.
          */
         StrAsc default_value;

         /**
          * Set to true if the setting should not be generated when reading string based setting
          * responses.
          */
         bool ignore_not_present;

         /**
          * Controls whether this setting should be displayed in the settings
          * editor.  Some devices, like the CS150, define settings that are
          * meant for other forms of consumption (calibration, display, etc)
          * and would clutter the settings editor.
          */
         bool displayable;

         /**
          * Specifies whether this setting should be polled by the settings
          * editor panel when it performs an auto-refresh operation.
          */
         bool auto_refresh;

         /**
          * Specifies the set of component descriptions for this setting.
          */
      public:
         typedef SharedPtr<SettingComp::DescBase> value_type;
         typedef std::deque<value_type> components_type;
      private:
         components_type components;

         /**
          * Controls whether associated settings will be included in a
          * configuration summary following a commit operation.
          */
         bool include_after_commit;

         /**
          * Specifies the sub-tab of the settings editor on which this setting should appear.
          */
         StrAsc tab_name;

         /**
          * Specifies the order in which this setting should appear.
          */
         uint4 sort_order;

         /**
          * Specifies the minimum number of times that the setting components must repeat.
          */
         uint4 min_repeat_count;

         /**
          * Set to true if this setting should be ignored when reading a configuration summary.
          */
         bool ignore_summary;

         /**
          * Set to true if changing this setting is going to force the device to reboot.
          */
         bool disruptive;

         /**
          * Specifies a common identifier for this setting.  This is used to identify values that
          * are common between device types.
          */
         StrAsc common_name;

         /**
          * Specifies the height in pixels that should be applied to controls associated with this
          * setting.
          */
         int4 multi_line_height;

         /**
          * Specifies the enable expression for the setting as a whole.  This
          * is similar to the enable expression used for individual components
          * but this expression will be used when the repeat count is greater
          * than one.
          */
         StrAsc enable_expression;

         /**
          * Set to true if this setting is only should be considered read/write
          * only if the devconfig command line enables factory mode.
          */
         bool factory_write;

         /**
          * Set to true if the user should be allowed to add or remove repeat levels for this
          * setting.
          */
         bool fixed_repeat;

         /**
          * Specifies how repeated levels of this setting should be presented to the user.
          */
      public:
         enum repeat_presentation_type
         {
            repeat_present_normal,
            repeat_present_grid
         };
      private:
         repeat_presentation_type repeat_presentation;

         /**
          * Set to true if the setting should only be shown in factory mode.
          */
         bool factory_only;
         
      public:
         /**
          * Default constructor.
          */
         SettingDesc():
            identifier(0),
            repeat_count(1),
            ignore_not_present(false),
            displayable(true),
            auto_refresh(false),
            include_after_commit(true),
            sort_order(0),
            min_repeat_count(0),
            ignore_summary(false),
            disruptive(false),
            multi_line_height(-1),
            factory_write(false),
            fixed_repeat(false),
            repeat_presentation(repeat_present_normal),
            factory_only(false)
         { }

         /**
          * Copy constructor.
          */
         SettingDesc(SettingDesc const &other):
            name(other.name),
            components(other.components),
            identifier(other.identifier),
            repeat_count(other.repeat_count),
            ignore_not_present(other.ignore_not_present),
            default_value(other.default_value),
            read_only(other.read_only),
            displayable(other.displayable),
            auto_refresh(other.auto_refresh),
            include_after_commit(other.include_after_commit),
            tab_name(other.tab_name),
            sort_order(other.sort_order),
            min_repeat_count(other.min_repeat_count),
            ignore_summary(other.ignore_summary),
            disruptive(other.disruptive),
            common_name(other.common_name),
            multi_line_height(other.multi_line_height),
            enable_expression(other.enable_expression),
            factory_write(other.factory_write),
            fixed_repeat(other.fixed_repeat),
            repeat_presentation(other.repeat_presentation),
            factory_only(other.factory_only)
         { }

         /**
          * Copy operator.
          */
         SettingDesc &operator =(SettingDesc const &other)
         {
            name = other.name;
            components = other.components;
            identifier = other.identifier;
            ignore_not_present = other.ignore_not_present;
            default_value = other.default_value;
            repeat_count = other.repeat_count;
            read_only = other.read_only;
            displayable = other.displayable;
            auto_refresh = other.auto_refresh;
            include_after_commit = other.include_after_commit;
            tab_name = other.tab_name;
            sort_order = other.sort_order;
            min_repeat_count = other.min_repeat_count;
            ignore_summary = other.ignore_summary;
            disruptive = other.disruptive;
            common_name = other.common_name;
            multi_line_height = other.multi_line_height;
            enable_expression = other.enable_expression;
            factory_write = other.factory_write;
            fixed_repeat = other.fixed_repeat;
            repeat_presentation = other.repeat_presentation;
            factory_only = other.factory_only;
            return *this;
         }

         /**
          * Initialise this object from the provided XML content.
          *
          * @param xml_data Specifies the XML structure that describes this setting.
          *
          * @param library_dir Specifies the path to the device description library.
          */
         void init_from_xml(
            Csi::Xml::Element &xml_data,
            StrAsc const &library_dir);

         /**
          * @return Returns the name of this setting.
          */
         StrAsc const &get_name() const
         { return name; }

         /**
          * @return returns the unique identifier for this setting.
          */
         uint2 get_identifier() const
         { return identifier; }

         /**
          * @return Returns true if the user should not be allowed to change this setting.
          */
         bool get_read_only() const
         { return read_only; }

         /**
          * @param read_only_ Set to true of the user should not be allowed to change this setting.
          */
         void set_read_only(bool read_only_)
         { read_only = read_only_; }

         /**
          * @return Returns the maximum number of times that the components for this setting should
          * be allowed to repeat.
          */
         uint4 get_repeat_count() const
         { return repeat_count; }

         /**
          * @return Returns the minimum number of times that the components for this setting should
          * repeat.
          */
         uint4 get_min_repeat_count() const
         { return min_repeat_count; }

         /**
          * @param the_factory_ Specifies the component factory used by all settings.
          */
         static void set_factory(SharedPtr<SettingComp::Factory> the_factory_);

         /**
          * @return Returns the component factory that should be used by all settings.
          */
         static SharedPtr<SettingComp::Factory> get_factory();

         /**
          * @return Returns the description text for the specified component.
          *
          * @param component_name Specifies the name of the component to look up.  If empty, the
          * default value, the help for the first component will be returned.
          *
          * @param comp Specifies the index for the component.
          */
         StrAsc const &get_description_text(StrAsc const &component_name = "");
         StrAsc const &get_description_text(uint4 comp);

         /**
          * @return Returns true if the description text is a reference to a file.
          *
          * @param component_name Specifies the name of the component to look up.  If empty, the
          * default value, the help for the first component will be returned.
          *
          * @param comp Specifies the index for the component.
          */
         bool description_text_is_file(StrAsc const &component_name = "");
         bool description_text_is_file(uint4 comp);

         /**
          * @return Returns the formattedf default value for this setting.
          */
         StrAsc const &get_default_value() const
         { return default_value; }

         /**
          * @return Returns true if this setting should not be generated when reading settings from
          * the string based response.
          */
         bool get_ignore_not_present() const
         { return ignore_not_present; }

         /**
          * @return Returns true if this setting should be shown in the settings editor.
          */
         bool get_displayable() const
         { return displayable; }

         /**
          * @return Returns true if this setting should be automatically polled.
          */
         bool get_auto_refresh() const
         { return auto_refresh; }

         /**
          * @return Returns true if this setting should be included in the configuration summary
          * generated following a commit.
          */
         bool get_include_after_commit() const
         { return include_after_commit; }

         /**
          * @return Returns the sub-panel tab name on which this setting should be included.
          */
         StrAsc const &get_tab_name() const
         { return tab_name; }

         /**
          * @return Returns the ordering for this setting.
          */
         uint4 get_sort_order() const
         { return sort_order; }

         /**
          * @return Returns true if this setting should not be initialised from a configuration
          * summary.
          */
         bool get_ignore_summary() const
         { return ignore_summary; }

         /**
          * @return Returns true if the changing of this setting would cause the device to reboot or
          * other significant changes following a commit.
          */
         bool get_disruptive() const
         { return disruptive; }

         /**
          * @return Returns the common name for this setting.
          */
         StrAsc const &get_common_name() const
         { return common_name; }

         /**
          * @return Returns the minimum height for controls associated with this setting.
          */
         int4 get_multi_line_height() const
         { return multi_line_height; }

         /**
          * @return Returns the enable expression for this setting.
          */
         StrAsc const &get_enable_expression() const
         { return enable_expression; }

         /**
          * @return Returns true if this setting should be read/write only if
          * the factory option is enabled.
          */
         bool get_factory_write() const
         { return factory_write; }

         /**
          * @return Returns true if the user should be prevented from adding or removing repeat
          * levels.
          */
         bool get_fixed_repeat() const
         { return fixed_repeat && repeat_count > 1; }

         /**
          * @return Returns the code that specifies how this setting should be presented to the user
          * when it has a repeat count greater than one.
          */
         repeat_presentation_type get_repeat_presentation() const
         { return repeat_presentation; }

         /**
          * @return Returns true if the setting should only be shown when in factory mode.
          */
         bool get_factory_only() const
         { return factory_only; }

         //@group container access methods
         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         components_type::size_type size() const
         { return components.size(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return components.empty(); }

         ////////////////////////////////////////////////////////////
         // front
         ////////////////////////////////////////////////////////////
         value_type const &front() const
         { return components.front(); }

         ////////////////////////////////////////////////////////////
         // back
         ////////////////////////////////////////////////////////////
         value_type const &back() const
         { return components.back(); }

         ////////////////////////////////////////////////////////////
         // begin (const)
         ////////////////////////////////////////////////////////////
         typedef components_type::const_iterator const_iterator;
         const_iterator begin() const
         { return components.begin(); }

         ////////////////////////////////////////////////////////////
         // end (const)
         ////////////////////////////////////////////////////////////
         const_iterator end() const
         { return components.end(); }

         ////////////////////////////////////////////////////////////
         // begin (non-const)
         ////////////////////////////////////////////////////////////
         typedef components_type::iterator iterator;
         iterator begin()
         { return components.begin(); }

         ////////////////////////////////////////////////////////////
         // end (non-const)
         ////////////////////////////////////////////////////////////
         iterator end()
         { return components.end(); }

         ////////////////////////////////////////////////////////////
         // subscript operator
         ////////////////////////////////////////////////////////////
         value_type operator[](components_type::size_type pos)
         {
            components_type::size_type i(0);
            value_type rtn;
            for(iterator ci = begin(); rtn == 0 && ci != end(); ++ci)
            {
               if(i++ == pos)
                  rtn = *ci;
            }
            return rtn;
         }
         //@endgroup
      };
   };
};


#endif
