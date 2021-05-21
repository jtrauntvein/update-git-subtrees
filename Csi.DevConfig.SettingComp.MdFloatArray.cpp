/* Csi.DevConfig.SettingComp.MdFloatArray.cpp

   Copyright (C) 2011, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 21 December 2011
   Last Change: Thursday 01 March 2018
   Last Commit: $Date: 2018-03-01 14:40:39 -0600 (Thu, 01 Mar 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.MdFloatArray.h"
#include "CsiTypes.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         CompBase *MdFloatArrayDesc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         {
            return new MdFloatArray(desc);
         }


         MdFloatArray::MdFloatArray(SharedPtr<DescBase> &desc):
            CompBase(desc)
         { }


         MdFloatArray::~MdFloatArray()
         { }


         void MdFloatArray::read(SharedPtr<Message> &message)
         {
            // we need to read in the list of dimensions for this array
            uint4 dim = message->readUInt4();
            dimensions.clear();
            values.clear();
            while(dim != 0)
            {
               dimensions.add_dimension(dim);
               dim = message->readUInt4();
            }

            // we now need to read in the set of values
            uint4 values_count = dimensions.array_size();
            if(values_count > 0)
            {
               values.reserve(values_count);
               for(uint4 i = 0; i < values_count; ++i)
                  values.push_back(message->readIeee4());
            }
         } // read


         void MdFloatArray::write(SharedPtr<Message> &message)
         {
            for(ArrayDimensions::const_iterator di = dimensions.begin(); di != dimensions.end(); ++di)
               message->addUInt4(*di);
            message->addUInt4(0);
            for(iterator vi = begin(); vi != end(); ++vi)
               message->addIeee4(*vi); 
         } // write


         void MdFloatArray::write(Json::Array &json)
         {
            Json::ObjectHandle comp_json(new Json::Object);
            Json::ArrayHandle dimensions_json(new Json::Array);
            Json::ArrayHandle values_json(new Json::Array);

            comp_json->set_property("dimensions", dimensions_json.get_handle());
            comp_json->set_property("values", values_json.get_handle());
            json.push_back(comp_json.get_handle());
            for(ArrayDimensions::const_iterator ai = dimensions.begin();
                ai != dimensions.end();
                ++ai)
               dimensions_json->push_back(new Json::Number(*ai));
            for(values_type::iterator vi = values.begin();
                vi != values.end();
                ++vi)
               values_json->push_back(new Json::Number(*vi));
         } // write


         Json::Array::iterator MdFloatArray::read(Json::Array::iterator current)
         {
            Json::ObjectHandle comp_json(*current);
            Json::ArrayHandle dimensions_json(comp_json->get_property("dimensions"));
            Json::ArrayHandle values_json(comp_json->get_property("values"));
            dimensions.clear();
            for(Json::Array::iterator jdi = dimensions_json->begin(); jdi != dimensions_json->end(); ++jdi)
            {
               Json::NumberHandle dim_json(*jdi);
               dimensions.add_dimension((uint4)dim_json->get_value());
            }
            values.clear();
            for(Json::Array::iterator jvi = values_json->begin(); jvi != values_json->end(); ++jvi)
            {
               Json::NumberHandle value_json(*jvi);
               values.push_back((float)value_json->get_value());
            }
            return ++current;
         } // read


         void MdFloatArray::output(std::ostream &out, bool translate)
         {
            std::vector<uint4> subscripts(dimensions.begin(), dimensions.end());
            uint4 start_vector_count = (uint4)subscripts.size();
            
            for(iterator vi = begin(); vi != end(); ++vi)
            {
               // we need to prefix the value with a space or with start vector marks
               if(start_vector_count == 0)
                  out << " ";
               else
               {
                  if(vi != begin())
                     out << "\r\n";
                  while(start_vector_count > 0)
                  {
                     out << '[';
                     --start_vector_count;
                  }
               }
               csiFloatToStream(out, *vi);

               // we now need to adjust our subscripts array
               for(uint4 i = (uint4)subscripts.size(); i > 0; --i)
               {
                  uint4 &this_subscript = subscripts[i - 1];
                  if(--this_subscript == 0)
                  {
                     out << ']';
                     ++start_vector_count;
                     this_subscript = dimensions[i - 1];
                  }
                  else
                     break;
               }
            }
         }


         void MdFloatArray::output(std::wostream &out, bool translate)
         {
            std::vector<uint4> subscripts(dimensions.begin(), dimensions.end());
            uint4 start_vector_count = (uint4)subscripts.size();
            
            for(iterator vi = begin(); vi != end(); ++vi)
            {
               // we need to prefix the value with a space or with start vector marks
               if(start_vector_count == 0)
                  out << L" ";
               else
               {
                  if(vi != begin())
                     out << L"\r\n";
                  while(start_vector_count > 0)
                  {
                     out << L'[';
                     --start_vector_count;
                  }
               }
               csiFloatToStream(out, *vi);

               // we now need to adjust our subscripts array
               for(uint4 i = (uint4)subscripts.size(); i > 0; --i)
               {
                  uint4 &this_subscript = subscripts[i - 1];
                  if(--this_subscript == 0)
                  {
                     out << L']';
                     ++start_vector_count;
                     this_subscript = dimensions[i - 1];
                  }
                  else
                     break;
               }
            }
         }


         void MdFloatArray::input(std::istream &in, bool translate)
         {
            std::vector<uint4> new_dimensions;
            std::vector<float> new_values;
            StrAsc temp;
            enum state_type
            {
               state_before_array,
               state_before_value,
               state_in_value,
               state_after_array,
               state_complete
            } state = state_before_array;
            char ch;
            uint4 indent_level = 0;
            bool dimension_is_new = false;
            std::locale locale = std::locale::classic();
            bool skip_next = false;
            
            while(state != state_complete)
            {
               if(!skip_next && !in.get(ch))
                  throw std::invalid_argument("array syntax error");
               skip_next = false;
               if(state == state_before_array)
               {
                  if(ch == '[')
                  {
                     if(new_dimensions.size() < indent_level + 1)
                     {
                        new_dimensions.push_back(0);
                        dimension_is_new = true;
                     }
                     else
                        ++(new_dimensions[indent_level - 1]);
                     ++indent_level;
                     state = state_before_value;
                  }
                  else if(!isspace(ch))
                     throw std::invalid_argument("array syntax error");
               }
               else if(state == state_in_value)
               {
                  if(isspace(ch) || ch == ']')
                  {
                     new_values.push_back(static_cast<float>(csiStringToFloat(temp.c_str(), locale, true)));
                     if(indent_level != new_dimensions.size())
                        throw std::invalid_argument("dimensions conflict");
                     if(dimension_is_new)
                        ++(new_dimensions[indent_level - 1]);
                     skip_next = true;
                     state = state_before_value;
                  }
                  else
                     temp.append(ch);
               }
               else if(state == state_before_value)
               {
                  if(ch == '[')
                  {
                     if(new_dimensions[indent_level - 1] == 0)
                        new_dimensions[indent_level - 1] = 1;
                     state = state_before_array;
                     skip_next = true;
                  }
                  else if(ch == ']')
                  {
                     if(--indent_level == 0)
                        state = state_complete;
                     else 
                        state = state_after_array;
                     dimension_is_new = false;
                  }
                  else if(!isspace(ch))
                  {
                     temp.cut(0);
                     temp.append(ch);
                     state = state_in_value;
                  }
               }
               else if(state == state_after_array)
               {
                  if(ch == ']')
                  {
                     if(--indent_level == 0)
                        state = state_complete;
                  }
                  else if(ch == '[')
                  {
                     state = state_before_array;
                     skip_next = true;
                  }
                  else if(!isspace(ch))
                     throw std::invalid_argument("array syntax error");
               }
            }

            // now that we are done, we need to verify that the number of values parsed coincides
            // with the number of values given by the array dimensions.
            ArrayDimensions test_dims(new_dimensions.begin(), new_dimensions.end());
            if(test_dims.array_size() != new_values.size())
               throw std::invalid_argument("array dimensions conflict");
            dimensions = test_dims;
            values = new_values;
            set_has_changed(true);
         } // input


         MdFloatArray::iterator MdFloatArray::value_at_address(address_type const &address)
         {
            iterator rtn = values.end();
            uint4 offset = dimensions.to_offset(address.begin(), address.end());
            if(offset <= dimensions.array_size())
               rtn = values.begin() + offset - 1;
            return rtn;
         }


         MdFloatArray::const_iterator MdFloatArray::value_at_address(address_type const &address) const
         {
            const_iterator rtn = values.end();
            uint4 offset = dimensions.to_offset(address.begin(), address.end());
            if(offset <= dimensions.array_size())
               rtn = values.begin() + offset - 1; 
            return rtn;
         }

         
         void MdFloatArray::do_copy(CompBase *other_)
         {
            MdFloatArray *other = static_cast<MdFloatArray *>(other_);
            dimensions = other->dimensions;
            values = other->values;
         }
      };
   };
}
