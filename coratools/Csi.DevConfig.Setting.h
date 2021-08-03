/* Csi.DevConfig.Setting.h

   Copyright (C) 2003, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 December 2003
   Last Change: Tuesday 20 April 2021
   Last Commit: $Date: 2021-04-20 12:42:24 -0600 (Tue, 20 Apr 2021) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_Setting_h
#define Csi_DevConfig_Setting_h

#include "Csi.DevConfig.SettingDesc.h"
#include "Csi.Html.Tag.h"
#include "Csi.Json.h"
#include <deque>


namespace Csi
{
   namespace DevConfig
   {
      /**
       * Defines an object that represents a setting as read from the device.  It acts as a
       * container of setting component objects and provides methods for streaming the setting to
       * and from message objects.
       */
      class Setting
      {
      protected:
         /**
          * Reference to the description for this setting.
          */
         SharedPtr<SettingDesc> desc;

         /**
          * Set to true if this setting is read-only.  This can either come from the description or
         it can be specified at run-time by the device.
         */
         bool read_only;

         /**
          * Set to true if the value(s) for this setting has been changed.
          */
         bool has_changed;
         
         /**
          * Specifies the collection of components associated with this setting.
          */
      public:
         typedef SharedPtr<SettingComp::CompBase> value_type;
         typedef std::deque<value_type> components_type;
      protected:
         components_type components;

         /**
          * Specifies the device major version.
          */
         byte major_version;

         /**
          * Specifies the device minor version.
          */
         byte minor_version;

      public:
         /**
          * Constructor
          *
          * @param desc Specifies the description for this setting.
          */
         Setting(SharedPtr<SettingDesc> &desc);

         /**
          * Copy constructor
          *
          * @param other Specifies the setting to copy.
          */
         Setting(Setting &other);

         /**
          * Copy component seubset constructor.
          *
          * @param other Specifies the setting to copy.
          *
          * @param level Specifies the repeat level in the other setting to copy.
          */
         Setting(Setting &other, uint4 level);

         /**
          * Destructor
          */
         virtual ~Setting();

         /**
          * @return Returns the description.
          */
         SharedPtr<SettingDesc> &get_desc()
         { return desc; }

         /**
          * @return Returns the name from the description.
          */
         StrAsc const &get_name() const
         { return desc->get_name(); }

         /**
          * @return Returns the setting identifier from the description.
          */
         uint2 get_identifier() const
         { return desc->get_identifier(); }

         /**
          * @return Returns the maximum  repeat count from the description.
          */
         uint4 get_repeat_count() const
         { return desc->get_repeat_count(); }

         /**
          * @return Returns the minimum repeat count from the description.
          */
         uint4 get_min_repeat_count() const
         { return desc->get_min_repeat_count(); }

         /**
          * @return Returns the expected height of the control, in pixels, that will display this
          * setting.
          */
         int4 get_multi_line_height() const
         { return desc->get_multi_line_height(); }

         /**
          * @return Returns true if this setting is marked as read-only.  This can occur because the
          * description specified it as so or because the setting was reported as read-only by the
          * device.
          */
         bool get_read_only() const
         { return read_only; }

         /**
          * @param read_only_ Set to true if this setting must be marked as read-only.
          */
         void set_read_only(bool read_only_);

         /**
          * @return Returns true if this setting should be displayed to the user.
          */
         bool get_displayable() const
         { return desc->get_displayable(); }

         /**
          * @return Returns true if this setting has been identified as one that should
          * automatically refresh.
          */
         bool get_auto_refresh() const
         { return desc->get_auto_refresh(); }

         /**
          * @return Returns true if this setting should be included in the configuration summary.
          */
         bool get_include_after_commit() const
         { return desc->get_include_after_commit(); }

         /**
          * @return Returns the name of the tab  on which this setting will appear.
          */
         StrAsc const &get_tab_name() const
         { return desc->get_tab_name(); }

         /**
          * @return Returns the description ordering factor.
          */
         uint4 get_sort_order() const
         { return desc->get_sort_order(); }

         /**
          * @return Returns true if this setting has been marked as disruptive.
          */
         bool get_disruptive() const
         { return desc->get_disruptive(); }

         /**
          * @return Returns the common name for the setting.
          */
         StrAsc const &get_common_name() const
         { return desc->get_common_name(); }

         /**
          * @return Returns the enable expression for this setting.
          */
         StrAsc const &get_enable_expression() const
         { return desc->get_enable_expression(); }
         
         // @group: components container access

         /**
          * @return Returns the number of components.
          */
         components_type::size_type size() const
         { return components.size(); }

         /**
          * @return Returns true if there are no components.
          */
         bool empty() const
         { return components.empty(); }

         /**
          * @return Returns a reference to the first component.
          */
         value_type &front()
         { return components.front(); }

         /**
          * @return Returns a reference to the last component.
          */
         value_type &back()
         { return components.back(); }

         /**
          * @return Returns an iterator to the first component.
          */
         typedef components_type::iterator iterator;
         iterator begin()
         { return components.begin(); }

         /**
          * @return Returns an iterator beyond the last component.
          */
         iterator end()
         { return components.end(); }

         /**
          * @return Returns the reference to the component at the specified subscript.
          *
          * @param subscript Specifies the subscript.
          *
          * @throws std::invalid_argument Thrown if the subscript does not reference a valid
          * component position.
          */
         value_type &operator [](components_type::size_type subscript)
         { return components.at(subscript); }

         /**
          * @return Returns the component at the specified position.
          *
          * @param subscript Specifies the subscript for the component.
          */
         value_type &at(components_type::size_type subscript)
         { return components.at(subscript); }
         value_type const &at(components_type::size_type subscript) const
         { return components.at(subscript); }

         /**
          * @return Returns an iterator to the first instance of the component that has the
          * specified id (offset within the repeat level) and repeat level.  If no such component
          * exists, end() will be returned.
          *
          * @param comp_id Specifies the zero based component offset.
          *
          * @param repeat_count=0 Specifies the repeat level.
          */
         iterator find_comp(
            uint4 comp_id,
            uint4 repeat_count = 0);

         /**'
          * @return Returns an iterator to the setting component that has the specified common name
          * in its description within the given  repeat level.
          *
          * @param comp_name Specifies the component's common name.
          * @param repeat_count Specifies the repeat level for the component.
          */
         iterator find_comp_common(
            StrAsc const &common_name, uint4 repeat_count = 0);
         
         /**
          * @return Returns true if this setting or any of its components have been changed.
          */
         bool get_has_changed() const;

         /**
          * Clears the changed flag for this setting and for all components.
          */
         void clear_has_changed();

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
          * @param major_version_ Specifies the device major version.
          *
          * @param minor_version_ Specifies the device minor version.
          */
         void set_version(byte major_version_, byte minor_version_);

         /**
          * Reads the component values from the input message assuming that the entire setting is
          * present in the message body.  If the setting is retrieved in multiple chunks, the
          * application must first assemble those chunks before calling this method.
          *
          * @param in Specifies the message body to read.
          *
          * @throws std::exception Thrown if the message is malformed or does not fit this setting.
          */
         void read(SharedPtr<Message> &in);

         /**
          * Writes the component values to the message stream.  The entire setting will be written
          * regardless of size.  Fragmentation and tranmission of the setting after this is the
          * responsibility of the application.
          *
          * @param out Specifies the message to write.
          */
         void write(SharedPtr<Message> &out);

         /**
          * Reads the component values from the specified JSON array structure.
          *
          * @param comps_json Specifies the JSON array that holds component values.
          */
         void read(Json::Array &comps_json);

         /**
          * Reads the setting component values from the provided stream or string object.
          *
          * @param in Specifies the input stream.
          *
          * @param translate Set to true if the component values are expected to reflect
          * "user-friendly" format.
          */
         void read_formatted(
            std::istream &in,
            bool translate = false);
         void read_formatted(
            StrAsc const &in,
            bool translate = false);

         /**
          * Writes the setting component values to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param translate=false Set to true if component values should be written with
          * user-friendly values (interpretation will depend upon component type).
          */
         void write_formatted(
            std::ostream &out,
            bool translate = false);

         /**
          * Reads the value of a specified component from the given input stream.
          *
          * @param in  Specifies the input stream.
          *
          * @param comp_id=0 Specifies the zero based component offset.
          *
          * @param translated=true Set to true if the component value is expected to be
          * user-friendly (interpretation depends on the component type).
          *
          * @param repeat_count=0 Specifies the repeat level.
          */
         void read_comp_by_id(
            std::istream &in,
            uint4 comp_id = 0,
            bool translated = true,
            uint4 repeat_count = 0);

         /**
          * Writes the value of a specified component to the given output stream.
          *
          * @param out Specififies the output stream.
          *
          * @param comp_id=0 Specifies the zero-based component offset.
          *
          * @param translated=true Set to true if component values should be formatted in a
          * "user-friendly" format (this is interpreted by individual component types).
          *
          * @param repeat_count=0 Specifies the zero-based repeat level.
          */
         void write_comp_by_id(
            std::ostream &out,
            uint4 comp_id = 0,
            bool translated = true,
            uint4 repeat_count = 0);

         /**
          * Reads the value of a specified component from the given input stream.
          *
          * @param in Specifies the input stream.
          *
          * @param comp_num=0 Specifies the zero based component index (regardless of repeat
          * level).
          *
          * @param translated=true Set to true if the value is expected to be formatted in a
          * "user-friendly" manner (this is interpreted by the individual component types).
          */
         void read_comp(
            std::istream &in,
            uint4 comp_num = 0,
            bool translated = true);
         void read_comp(StrAsc const &in, uint4 comp_num, bool translated = true);
         
         /**
          * Writes the value of a specified component to the given output stream.
          *
          * @param out Specifies the output stream.
          *
          * @param comp_num=0 Specifies the zero based component index.
          *
          * @param translate=true Set to true if "user-friendly" values are to be written.
          */
         void write_comp(
            std::ostream &out,
            uint4 comp_num = 0,
            bool translated = true);

         /**
          * Removes all components for this setting.
          */
         void clear_components();

         /**
          * @return Returns a copy of this setting object with new components that are copies of
          * this setting's components.
          */
         Setting *clone();

         /**
          * Copies the values of the components in the specified setting into this setting's
          * components.  Verifies that both settings share the same description handle.
          *
          * @param other Specifies the source for component values.
          */
         void copy(Setting *other);

         /**
          * @return Returns the text for the description of a specified component identified by
          * either name or ID.
          *
          * @param comp_name="" Specifies the name of the component.  If empty, the description for
          * the first component will be returned.
          *
          * @param id Specifies the zero based identifier for the component.
          */
         StrAsc const &get_description_text(StrAsc const &comp_name = "")
         { return desc->get_description_text(comp_name); }
         StrAsc const &get_description_text(uint4 id)
         { return desc->get_description_text(id); }

         /**
          * @return Returns true if the description text represents a file path.
          *
          * @param comp_name="" Specifies the name of the component.  If empty, the first component
          * will be evaluated.
          *
          * @param id Specifies the component ID.
          */
         bool description_text_is_file(StrAsc const &comp_name = "")
         { return desc->description_text_is_file(comp_name); }
         bool description_text_is_file(uint4 id)
         { return desc->description_text_is_file(id); }

         /**
          * Reverts the setting to the "factory default" values stored in component descriptions.
          * If there is no default specified, the setting will remain untouched.
          *
          * @return Returns true if the setting component values were changed.
          */
         bool revert_to_default();

         // @group: component set/get scalar values

         /**
          * @return Returns the value of the specified component as an unsigned one byte integer.
          *
          * @param comp_num Specifies the index of the component.
          * @param comp_common_name Specifies the common name for the component
          * @param repeat_count Specifies the repeat level,
          */
         byte get_comp_byte(uint4 comp_num = 0)
         { return components.at(comp_num)->get_val_byte(); }
         byte get_comp_byte(StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            return (*ci)->get_val_byte();
         }

         /**
          * Sets the value of the specified component as a one byte unsigned integer.
          *
          * @param comp_num=0 Specifies the index of the component.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Specifies the repeat level.
          */
         void set_comp_byte(byte val, uint4 comp_num = 0)
         { components.at(comp_num)->set_val_byte(val); }
         void set_comp_byte(byte val, StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            (*ci)->set_val_byte(val);
         }

         /**
          * @return Returns the value of the specified component as a two byte signed integer.
          *
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         int2 get_comp_int2(uint4 comp_num = 0)
         { return components.at(comp_num)->get_val_int2(); }
         int2 get_comp_int2(StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            return (*ci)->get_val_int2();
         }

         /**
          * @param val Specifies the value of the component as a two byte signed integer.
          *
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         void set_comp_int2(int2 val, uint4 comp_num = 0)
         { components.at(comp_num)->set_val_int2(val); }
         void set_comp_int2(int2 val, StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            (*ci)->set_val_int2(val);
         }

         /**
          * @return Returns the specified component value as a two byte unsigned integer.
          *
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         uint2 get_comp_uint2(uint4 comp_num = 0)
         { return components.at(comp_num)->get_val_uint2(); }
         uint2 get_comp_uint2(StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            return (*ci)->get_val_uint2();
         }

         /**
          * @param val  Specifies the component value as a two byte unsigned integer.
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         void set_comp_uint2(uint2 val, uint4 comp_num = 0)
         { components.at(comp_num)->set_val_uint2(val); }
         void set_comp_uint2(uint2 val, StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            (*ci)->set_val_uint2(val);
         }       

         /**
          * @return Returns the specified component value as a four byte signed integer.
          *
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         int4 get_comp_int4(uint4 comp_num = 0)
         { return components.at(comp_num)->get_val_int4(); }
         int4 get_comp_int4(StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            return (*ci)->get_val_int4();
         }

         /**
          * @param val Specifies the specified component value as a four byte signed integer.
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         void set_comp_int4(int4 val, uint4 comp_num = 0)
         { components.at(comp_num)->set_val_int4(val); }
         void set_comp_int4(int4 val, StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            (*ci)->set_val_int4(val);
         }

         /**
          * @return Returns the specified component value as a four byte unsigned integer.
          *
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         uint4 get_comp_uint4(uint4 comp_num = 0)
         { return components.at(comp_num)->get_val_uint4(); }
         uint4 get_comp_uint4(StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            return (*ci)->get_val_uint4();
         }

         /**
          * @param val Specifies the specified component value as a four byte unsigned integer.
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         void set_comp_uint4(uint4 val, uint4 comp_num = 0)
         { components.at(comp_num)->set_val_uint4(val); }
         void set_comp_uint4(uint4 val, StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            (*ci)->set_val_uint4(val);
         }

         /**
          * @return Returns the specified component value as a float.
          *
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         float get_comp_float(uint4 comp_num = 0)
         { return components.at(comp_num)->get_val_float(); }
         float get_comp_float(StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            return (*ci)->get_val_float();
         }

         /**
          * @param val Specifies the specified component value as a float.
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         void set_comp_float(float val, uint4 comp_num = 0)
         { components.at(comp_num)->set_val_float(val); }
         void set_comp_float(float val, StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            (*ci)->set_val_float(val);
         }

         /**
          * @return Returns the specified component value as a double.
          *
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         double get_comp_double(uint4 comp_num = 0)
         { return components.at(comp_num)->get_val_double(); }
         double get_comp_double(StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            return (*ci)->get_val_double();
         }

         /**
          * @param val  Specifies the specified component value as a double. 
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         void set_comp_double(double val, uint4 comp_num = 0)
         { components.at(comp_num)->set_val_double(val); }
         void set_comp_double(double val, StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            (*ci)->set_val_double(val);
         }

         /**
          * @return Returns the specified component as a UTF-8 encoded string.
          *
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         StrAsc get_comp_str(uint4 comp_num = 0)
         { return components.at(comp_num)->get_val_str(); }
         StrAsc get_comp_str(StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            return (*ci)->get_val_str();
         }

         /**
          * @param val Sets the specified component value as a UTF-8 encoded string.
          * @param check Set to true if the string value must be checked before being applied to the component.
          * @param comp_num=0 Specifies the component index.
          * @param comp_common_name Specifies the common name of the component.
          * @param repeat_count Optionally specifies the repeat level.
          */
         void set_comp_str(StrAsc const &val, uint4 comp_num = 0, bool check = false)
         { components.at(comp_num)->set_val_str(val, check); }
         void set_comp_str(StrAsc const &val, bool check, StrAsc const &comp_common_name, uint4 repeat_count = 0)
         {
            auto ci(find_comp_common(comp_common_name, repeat_count));
            if(ci == components.end())
               throw std::invalid_argument("component not found");
            (*ci)->set_val_str(val, check);
         }
         
         // @endgroup:

         /**
          * Removes all components at the specified repeat level.
          *
          * @param level Specifies the repeat level to remove.
          */
         void remove_level(uint4 level);

         /**
          * Adds a new level of components at the specified index.
          *
          * @param level=0xffffffff Specifies the index where the new components will be added.  If
          * 0xffffffff, the new level will be appended.
          */
         void add_level(uint4 level = 0xFFFFFFFF);

         /**
          * Copies components from the other setting into the specified repeat level for this
          * setting.   Ensures that both settings reference the same description object.
          *
          * @param other Specifies the source setting.
          *
          * @param level Specifies the repeat level on which to insert the new components.
          *
          * @param source_level=0 Specifies the level to copy from the source setting.
          */
         void copy_level(Setting *other, uint4 level, uint4 source_level = 0);

         /**
          * @return Returns true if one or more of the components in this setting have "protected"
          * (generally password) values.
          */
         bool has_protected_value() const;

         /**
          * Formats this setting's value(s) as HTML within the specified tag.
          *
          * @param tag Specifies the tag to which the HTML element(s) for this setting will be
          * formatted.
          */
         void format_html(Csi::Html::Tag &tag);
      };
   };
};


#endif
