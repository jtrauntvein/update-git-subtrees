/* Csi.DevConfig.Setting.cpp

   Copyright (C) 2003, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 December 2003
   Last Change: Monday 19 April 2021
   Last Commit: $Date: 2021-04-20 12:42:24 -0600 (Tue, 20 Apr 2021) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.Setting.h"
#include "Csi.MsgExcept.h"
#include "Csi.BuffStream.h"
#include "Csi.StrAscStream.h"
#include "Csi.Html.Table.h"
#include "Csi.Html.Text.h"
#include "coratools.strings.h"
#include "boost/format.hpp"
#include <algorithm>


namespace Csi
{
   namespace DevConfig
   {
      Setting::Setting(SharedPtr<SettingDesc> &desc_):
         desc(desc_),
         read_only(desc_->get_read_only()),
         major_version(0xFF),
         minor_version(0xFF)
      {
         // for each element of the description, we need to create a component to represent it.
         if(desc->get_repeat_count() == 1)
         {
            value_type previous_component;
            for(SettingDesc::iterator di = desc->begin();
                di != desc->end();
                ++di)
            {
               SettingDesc::value_type &comp_desc = *di;
               components.push_back(
                  comp_desc->make_component(comp_desc,previous_component));
               previous_component = components.back();
               previous_component->set_version(major_version, minor_version);
               previous_component->set_read_only(read_only);
            }
         }
      } // constructor

      Setting::Setting(Setting &other):
         desc(other.desc),
         read_only(other.read_only),
         major_version(other.major_version),
         minor_version(other.minor_version)
      {
         value_type previous;
         for(iterator ci = other.begin(); ci != other.end(); ++ci)
         {
            value_type &comp(*ci);
            SharedPtr<SettingComp::DescBase> comp_desc(comp->get_desc());
            value_type copy(
               comp->get_desc()->make_component(comp_desc, previous));
            copy->copy(comp.get_rep());
            previous = copy;
            components.push_back(copy);
            if(components.size() % desc->size() == 0)
               previous.clear();
         }
      } // copy constructor

      Setting::Setting(Setting &other, uint4 level):
         desc(other.desc),
         read_only(other.read_only),
         major_version(other.major_version),
         minor_version(other.minor_version)
      {
         value_type previous;
         if(level * desc->size() < other.size())
         {
            iterator copy_begin(other.begin() + (level * desc->size()));
            iterator copy_end(copy_begin + desc->size());
            for(iterator ci = copy_begin; ci != copy_end; ++ci)
            {
               value_type &comp(*ci);
               SharedPtr<SettingComp::DescBase> comp_desc(comp->get_desc());
               value_type copy(
                  comp->get_desc()->make_component(comp_desc, previous));
               copy->copy(comp.get_rep());
               components.push_back(copy);
               previous = copy;
               if(components.size() % desc->size() == 0)
                  previous.clear();
            }
         }
      } // copy level constructor

      Setting::~Setting()
      { components.clear(); }

      void Setting::set_read_only(bool read_only_)
      {
         read_only = read_only_;
         for(components_type::iterator ci = components.begin();
             ci != components.end();
             ++ci)
            (*ci)->set_read_only(read_only);
      } // set_read_only
      
      Setting::iterator Setting::find_comp(
         uint4 comp_id, uint4 repeat_level)
      {
         // we must first attempt to locate the specified component
         components_type::iterator rtn = components.begin();
         bool found_comp = false;
         uint4 prev_repeat_count = 0;
         uint4 current_id = 0;
         
         while(!found_comp && rtn != components.end())
         {
            // the ID repeats each time the repeat count changes
            value_type &comp = *rtn;
            if(prev_repeat_count != comp->get_repeat_count())
            {
               current_id = 0;
               prev_repeat_count = comp->get_repeat_count();
            }
            if(current_id == comp_id && comp->get_repeat_count() == repeat_level)
               found_comp = true;
            else
            {
               ++current_id;
               ++rtn;
            }
         }
         
         if(rtn == components.end() && comp_id < desc->size())
         {
            // since this level does not exist, we will create a new tier of components and then
            // return the component associated with that level.
            value_type previous_comp;
            components_type::size_type old_size = components.size();
            uint4 i = 0;
            
            components.resize(old_size + desc->size());
            rtn = components.end() - (desc->size() + comp_id);
            for(SettingDesc::iterator di = desc->begin();
                di != desc->end();
                ++di)
            {
               SettingDesc::value_type &comp_desc = *di;
               value_type comp(
                  comp_desc->make_component(
                     comp_desc,
                     previous_comp));
               comp->set_repeat_count(repeat_level);
               comp->set_read_only(desc->get_read_only());
               components[i++ + old_size] = comp;
               comp->set_version(major_version, minor_version);
               previous_comp = comp;
            }
         }
         return rtn;
      } // find_comp

      Setting::iterator Setting::find_comp_common(
         StrAsc const &comp_name, uint4 repeat_count)
      {
         // we need to search for the first component that is at the given repeat level
         auto rtn(
            std::find_if(
               components.begin(),
               components.end(),
               [repeat_count, comp_name](components_type::value_type &comp) {
                  return comp->get_repeat_count() == repeat_count &&
                     comp->get_desc()->get_common_name() == comp_name; }));
         return rtn;
      } // find_comp_common

      bool Setting::get_has_changed() const
      {
         bool rtn = has_changed && !read_only;
         for(components_type::const_iterator ci = components.begin();
             !read_only && !rtn && ci != components.end();
             ++ci)
         {
            value_type const &comp = *ci;
            rtn = comp->get_has_changed();
         }
         return rtn;
      } // has_changed

      void Setting::clear_has_changed()
      {
         has_changed = false;
         for(components_type::iterator ci = components.begin();
             ci != components.end();
             ++ci)
            (*ci)->set_has_changed(false);
      } // clear_has_changed

      void Setting::set_version(byte major_version_, byte minor_version_)
      {
         major_version = major_version_;
         minor_version = minor_version_;
         for(iterator ci = begin(); ci != end(); ++ci)
         {
            value_type &component(*ci);
            component->set_version(major_version, minor_version);
         }
      } // set_version
      
      void Setting::read(SharedPtr<Message> &in)
      {
         // make a local backup of the components before clearing the settings
         components_type backup(components);
         components_type this_level;
         if(desc->get_repeat_count() == 1)
            this_level = components; 
         components.clear();

         // we now need to start creating all of the components
         try
         {
            uint4 repeat_count = 0;
            
            while(in->whatsLeft())
            {
               // we need to create all of the components for this iteration of the setting. This
               // all has to be done at once before the read attempt takes place so that components
               // like BitField that need to be aware of subsequent components are properly
               // initialised.
               value_type previous_comp;

               if(this_level.empty())
               {
                  for(SettingDesc::iterator di = desc->begin();
                      di != desc->end();
                      ++di)
                  {
                     SettingDesc::value_type &comp_desc = *di;
                     value_type comp(
                        comp_desc->make_component(
                           comp_desc,
                           previous_comp));
                     comp->set_repeat_count(repeat_count);
                     comp->set_read_only(desc->get_read_only());
                     comp->set_version(major_version, minor_version);
                     this_level.push_back(comp);
                     previous_comp = comp;
                  }
               }
               
               // we now need to read the components that were created above and append them to
               // those managed by this object.
               while(!this_level.empty())
               {
                  value_type comp(this_level.front());
                  this_level.pop_front();
                  comp->read(in);
                  comp->set_has_changed(false); 
                  components.push_back(comp);
               }

               // the repeat count needs to be incremented if there is still input available from
               // the message.
               if(desc->get_repeat_count() > 1 && in->whatsLeft() > 0)
               {
                  if(++repeat_count >= desc->get_repeat_count())
                  {
                     using namespace SettingStrings;
                     OStrAscStream msg;
                     msg << boost::format(my_strings[strid_too_many_values].c_str()) % get_name();
                     throw MsgExcept(msg.str().c_str());
                  }
               }
               else
                  break;
            }
         }
         catch(std::exception &)
         {
            components = backup;
            throw;
         }
      } // read

      void Setting::read(Csi::Json::Array &comps_json)
      {
         // make a local backup of the components before clearing them.
         components_type backup(components);
         components_type this_level;
         if(desc->get_repeat_count() == 1)
            this_level = components;
         components.clear();

         // we now need to start creating all of the components.
         try
         {
            uint4 repeat_count(0);
            Json::Array::iterator ji(comps_json.begin());
            while(ji != comps_json.end())
            {
               // we need to create all of the components for this iteration of the setting.  This
               // is done all at once so that components such as BitField need not be aware of
               // whether subsequent values are initialised.
               value_type previous_comp;
               if(this_level.empty())
               {
                  for(SettingDesc::iterator di = desc->begin(); di != desc->end(); ++di)
                  {
                     SettingDesc::value_type &comp_desc(*di);
                     value_type comp(comp_desc->make_component(comp_desc, previous_comp));
                     comp->set_repeat_count(repeat_count);
                     comp->set_read_only(desc->get_read_only());
                     comp->set_version(major_version, minor_version);
                     this_level.push_back(comp);
                     previous_comp = comp;
                  }
               }

               // we now need to read the components that were created for this level above and
               // append them to those managed by this setting.
               while(!this_level.empty() && ji != comps_json.end())
               {
                  value_type comp(this_level.front());
                  this_level.pop_front();
                  ji = comp->read(ji);
                  comp->set_has_changed(true);
                  components.push_back(comp);
               }

               // the repeat count needs to be incremented if there are still input values that have
               // not been read.
               if(desc->get_repeat_count() > 1 && ji != comps_json.end())
               {
                  if(++repeat_count >= desc->get_repeat_count())
                  {
                     using namespace SettingStrings;
                     OStrAscStream message;
                     message << boost::format(my_strings[strid_too_many_values].c_str()) % get_name();
                     throw MsgExcept(message.c_str());
                  }
               }
               else
                  break;
            }
         }
         catch(std::exception &)
         {
            components = backup;
            throw;
         }
      } // read

      void Setting::write(SharedPtr<Message> &out)
      {
         for(iterator ci = components.begin(); ci != components.end(); ++ci)
         {
            value_type &comp = *ci;
            comp->write(out);
            comp->set_has_changed(false);
         }
      } // write

      void Setting::read_formatted(
         std::istream &in,
         bool translate)
      {
         // make a backup copy of the components before clearing them.  This can be used to restore
         // the previous components if these cannot be read.
         using namespace SettingStrings;
         components_type backup(components);
         components.clear();

         try
         {
            uint4 repeat_count = 0;
            SettingDesc::iterator di = desc->begin();
            StrAsc token;
            bool read_finished = false;
            bool first_in_series = true;
            
            while(in.good() && !read_finished)
            {
               // as is done in the read message method, we need to allocate all of the components
               // for this repetition at once so that related components can be properly
               // initialised.  These will be stored in a temporary container.
               components_type this_level;
               value_type previous_comp;

               first_in_series = true;
               for(SettingDesc::iterator di = desc->begin(); di != desc->end(); ++di)
               {
                  SettingDesc::value_type &comp_desc = *di;
                  value_type comp(
                     comp_desc->make_component(
                        comp_desc,
                        previous_comp));
                  comp->set_repeat_count(repeat_count);
                  comp->set_read_only(desc->get_read_only());
                  comp->set_version(major_version, minor_version);
                  this_level.push_back(comp);
                  previous_comp = comp;
               }
               
               // now that the components for this level are created, we need to attempt to parse
               // them out of the stream
               while(in.good() && !this_level.empty() && !read_finished)
               {
                  // pop the first component off of the list
                  value_type comp(this_level.front());
                  this_level.pop_front();

                  // get the values of the description, prefix and postfix
                  StrAsc prefix = comp->get_format_prefix();
                  StrAsc postfix = comp->get_format_postfix();
                  prefix.reverse();

                  // read past the prefix
                  char ch;
                  bool in_prefix = false;
                  
                  while(prefix.length() != 0 && in.get(ch))
                  {
                     if(ch == prefix.last())
                     {
                        in_prefix = true;
                        prefix.cut(prefix.length() - 1); 
                     }
                     else if(in_prefix || !isspace(ch))
                     {
                        OStrAscStream msg;
                        msg << boost::format(my_strings[strid_invalid_char_prefix].c_str()) %
                           ch % comp->get_name();
                        throw MsgExcept(msg.str().c_str());
                     }
                  }
                  if(prefix.length() != 0)
                  {
                     if(!first_in_series)
                     {
                        OStrAscStream msg;
                        msg << boost::format(my_strings[strid_prefix_not_found].c_str()) %
                           comp->get_format_prefix() % comp->get_name();
                        throw MsgExcept(msg.str().c_str());
                     }
                     else
                        break;
                  }
                  if(desc->size() > 1 && postfix.length() == 0)
                     postfix = "\n";
               
                  // now fill the token until the postfix is located
                  bool found_postfix = postfix.length() == 0;
                  
                  token.cut(0); 
                  while(!found_postfix && in.get(ch))
                  {
                     token.append(ch);
                     if(postfix.length() && token.find(postfix.c_str()) < token.length())
                     {
                        token.cut(token.length() - postfix.length());
                        found_postfix = true;
                     } 
                  }

                  // we want to be able to ignore the postfix for the last component
                  if(!found_postfix && in.eof() && this_level.empty())
                     found_postfix = true;
                  if(postfix.length() && !found_postfix)
                  {
                     if(!first_in_series && !in_prefix)
                     {
                        OStrAscStream msg;
                        msg << boost::format(my_strings[strid_postfix_not_found].c_str()) %
                           postfix % comp->get_name();
                        throw MsgExcept(msg.str().c_str());
                     }
                     else
                        break;
                  }

                  // now that the the token is separated from the prefix and postfix, we can
                  // construct an in-memory stream to read the component value.
                  try
                  {
                     if(postfix.length() != 0)
                     {
                        IBuffStream value_stream(token.c_str(), token.length());
                        comp->input(value_stream, translate);
                     }
                     else
                        comp->input(in,translate);
                     comp->set_repeat_count(repeat_count);
                     components.push_back(comp);
                     first_in_series = false;
                  }
                  catch(std::exception &)
                  {
                     if(components.empty() && token.length() == 0)
                        read_finished = true;
                     else
                        throw;
                  }

                  // the repeat count needs to be incremented if there is more to read on the
                  // stream.
                  if(desc->get_repeat_count() > 1 &&
                     !read_finished &&
                     this_level.empty() &&
                     in.peek() != std::char_traits<char>::eof())
                  {
                     if(++repeat_count >= desc->get_repeat_count())
                     {
                        OStrAscStream msg;
                        msg << boost::format(my_strings[strid_too_many_values].c_str()) % get_name();
                        throw MsgExcept(msg.str().c_str());
                     }
                  }
                  else if(this_level.empty())
                     read_finished = true;
               }
               has_changed = true;
            }
         }
         catch(std::exception &)
         {
            components = backup;
            throw;
         }
      } // read_formatted

      void Setting::read_formatted(
         StrAsc const &in,
         bool translate)
      {
         IBuffStream temp(in.c_str(),in.length());
         read_formatted(temp,translate);
      } // read_formatted

      void Setting::write_formatted(
         std::ostream &out,
         bool translated)
      {
         std::locale old_locale = out.imbue(std::locale::classic());
         value_type previous_comp;
         for(iterator ci = components.begin(); ci != components.end(); ++ci)
         {
            value_type &comp = *ci;
            StrAsc const &prefix = comp->get_format_prefix();
            StrAsc postfix = comp->get_format_postfix();
            bool started_new_repeat(false);
            
            if(desc->size() > 1 && postfix.length() == 0)
               postfix = "\n";
            if(previous_comp != 0 &&
               previous_comp->get_repeat_count() != comp->get_repeat_count())
            {
               started_new_repeat = true;
               if(translated)
                  out << "\n";
               else 
                  out << " ";
            }
            if(prefix.length() > 0)
               out << prefix;
            else if(ci != components.begin() && !started_new_repeat)
               out << " ";
            comp->output(out,translated);
            if(postfix.length())
               out << postfix;
            previous_comp = comp;
         }
         out.imbue(old_locale);
      } // write_formatted
      
      void Setting::read_comp_by_id(
         std::istream &in,
         uint4 comp_id,
         bool translated,
         uint4 repeat_level)
      {
         using namespace SettingStrings;
         iterator ci = find_comp(comp_id,repeat_level);
         if(ci != components.end())
            (*ci)->input(in,translated);
         else
            throw MsgExcept(my_strings[strid_invalid_component_name].c_str());
      } // read_comp_by_name

      void Setting::read_comp(
         std::istream &in,
         uint4 comp_no,
         bool translated)
      {
         value_type &comp = components.at(comp_no);
         comp->input(in,translated);
      } // read_comp

      void Setting::read_comp(StrAsc const &in, uint4 comp_no, bool translated)
      {
         Csi::IBuffStream temp(in.c_str(), in.length());
         read_comp(temp, comp_no, translated);
      } // read_comp

      void Setting::write_comp_by_id(
         std::ostream &out,
         uint4 comp_id,
         bool translated,
         uint4 repeat_level)
      {
         std::locale old_locale = out.imbue(std::locale::classic());
         try
         {
            using namespace SettingStrings;
            iterator ci = find_comp(comp_id,repeat_level);
            if(ci != components.end())
               (*ci)->output(out,translated);
            else
               throw MsgExcept(my_strings[strid_cannot_copy_unrelated].c_str());
            out.imbue(old_locale);
         }
         catch(std::exception &)
         {
            out.imbue(old_locale);
            throw;
         }
      } // write_comp_by_name

      void Setting::write_comp(
         std::ostream &out,
         uint4 comp_no,
         bool translated)
      {
         std::locale old_locale = out.imbue(std::locale::classic());
         try
         {
            value_type &comp = components.at(comp_no);
            comp->output(out,translated);
            out.imbue(old_locale);
         }
         catch(std::exception &)
         {
            out.imbue(old_locale);
            throw;
         }
      } // write_comp

      void Setting::clear_components()
      {
         if(desc->get_repeat_count() > 1)
         {
            has_changed = true;
            components.clear();
         }
      } // clear_components
      
      Setting *Setting::clone()
      {
         Setting *rtn = new Setting(desc);
         rtn->copy(this);
         return rtn;
      } // clone

      void Setting::copy(Setting *other)
      {
         if(desc == other->desc)
         {
            components_type::iterator oci = other->components.begin();
            uint4 repeat_level(0);
            
            components.clear(); 
            while(oci != other->components.end())
            {
               // all of the components for this iteration need to be created before the copy takes
               // place.
               components_type this_level;
               value_type previous_component;
               value_type other_comp = *oci;
               
               for(SettingDesc::iterator di = desc->begin();
                   di != desc->end();
                   ++di)
               {
                  SettingDesc::value_type &comp_desc = *di;
                  value_type this_comp(
                     comp_desc->make_component(
                        comp_desc,
                        previous_component));
                  this_comp->set_repeat_count(other_comp->get_repeat_count());
                  this_comp->set_read_only(other_comp->get_read_only());
                  this_comp->set_version(major_version, minor_version);
                  this_comp->set_repeat_count(repeat_level);
                  this_level.push_back(this_comp);
                  previous_component = this_comp;
               }
               ++repeat_level;

               // now we can copy the other components
               while(oci != other->components.end() && !this_level.empty())
               {
                  value_type this_comp(this_level.front());
                  this_level.pop_front();
                  other_comp = *oci;
                  this_comp->copy(other_comp.get_rep());
                  components.push_back(this_comp);
                  ++oci;
               }
            }
            has_changed = true;
         }
         else
            throw MsgExcept("Cannot copy unrelated settings");
      } // copy

      bool Setting::revert_to_default()
      {
         components_type old_comps(components);
         bool rtn = true;
         try
         {
            StrAsc const &default_value = desc->get_default_value();
            if(default_value.length())
            {
               IBuffStream temp(default_value.c_str(), default_value.length());
               read_formatted(temp,false);
            }
            else
               rtn = false;
         }
         catch(std::exception &)
         {
            components = old_comps;
            rtn = false;
         }
         return rtn;
      } // revert_to_default
      
      void Setting::remove_level(uint4 level)
      {
         uint4 start(level * (uint4)desc->size());
         if(start < components.size())
         {
            iterator start_it(begin() + start);
            iterator end_it(begin() + start + desc->size());
            components.erase(start_it, end_it);
         }
      } // remove_level

      void Setting::add_level(uint4 level)
      {
         if(components.size() / desc->size() < desc->get_repeat_count())
         {
            value_type previous;
            uint4 pos(0);
            for(SettingDesc::iterator di = desc->begin(); di != desc->end(); ++di)
            {
               SettingDesc::value_type comp_desc(*di);
               value_type comp(comp_desc->make_component(comp_desc, previous));
               if(comp_desc->get_default_value().length() > 0)
                  comp->set_val_str(comp_desc->get_default_value(), false);
               previous = comp;
               if(level == 0xFFFFFFFF || level * desc->size() >= size())
                  components.push_back(comp);
               else
               {
                  iterator insert_it(begin() + level * desc->size() + pos);
                  components.insert(insert_it, comp);
               }
               ++pos;
            }
         }
      } // add_level
      
      void Setting::copy_level(Setting *other, uint4 level, uint4 source_level)
      {
         // we need to ensure that the level exists.
         if(other->desc != desc)
            throw std::invalid_argument("setting descriptions don't match");
         if(source_level * desc->size() > other->size())
            throw std::invalid_argument("invalid source level specified");
         while(size() / desc->size() < level)
            add_level();

         // we can now copy the components from the source setting to this setting.
         iterator source_begin(other->begin() + source_level * desc->size());
         iterator source_end(other->end());
         iterator dest(begin() + level * desc->size());
         while(source_begin != source_end)
         {
            // we may need to allocate another level if the dest is at the end
            if(dest == end())
            {
               add_level();
               dest = end() - desc->size();
            }

            // we can now copy the components
            value_type &source_comp(*source_begin++);
            value_type &dest_comp(*dest++);
            dest_comp->copy(source_comp.get_rep());
         }
      } // copy_level

      bool Setting::has_protected_value() const
      {
         bool rtn(false);
         for(components_type::const_iterator ci = components.begin();
             !rtn && ci != components.end();
             ++ci)
         {
            value_type const &comp(*ci);
            rtn = comp->has_protected_value();
         }
         return rtn;
      } // has_protected_value

      void Setting::format_html(Csi::Html::Tag &tag)
      {
         using namespace SettingStrings;
         using namespace Csi::Html;
         typedef Csi::PolySharedPtr<Tag, Table> table_tag;
         Csi::OStrAscStream temp;

         temp.imbue(StringLoader::make_locale());
         if(desc->get_repeat_count() > 1 || desc->size() > 1)
         {
            table_tag components_table(new Table);
            uint4 last_repeat_count(UInt4_Max);
            
            for(SettingDesc::iterator di = desc->begin(); di != desc->end(); ++di)
            {
               SettingDesc::value_type &comp_desc(*di);
               if(comp_desc->present_in_summary())
                  components_table->add_tag(new Text(comp_desc->get_name(), "b"));
            }
            for(iterator ci = begin(); ci != end(); ++ci)
            {
               value_type &component(*ci);
               if(component->get_desc()->present_in_summary())
               {
                  temp.str("");
                  if(last_repeat_count != component->get_repeat_count())
                     components_table->add_row();
                  last_repeat_count = component->get_repeat_count();
                  if(!component->has_protected_value())
                     component->output(temp, true);
                  else
                     temp << my_strings[strid_protected_value];
                  temp.str().replace("&", "&amp;");
                  temp.str().replace("<", "&lt;");
                  temp.str().replace(">", "&gt;");
                  components_table->add_tag(new Text(temp.str(), "pre"));
               }
            }
            tag.add_tag(components_table.get_handle());
         }
         else if(!empty())
         {
            value_type &component(front());
            if(component->get_desc()->present_in_summary())
            {
               temp.str("");
               if(!component->has_protected_value())
                  component->output(temp, true);
               else
                  temp << my_strings[strid_protected_value];
               temp.str().replace("&", "&amp;");
               temp.str().replace("<", "&lt;");
               temp.str().replace(">", "&gt;");
               tag.add_tag(new Text(temp.str(), "pre"));
            }
         }
      } // format_html
   };
};

