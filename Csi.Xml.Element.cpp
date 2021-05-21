/* Csi.Xml.Element.cpp

   Copyright (C) 2006, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 19 May 2006
   Last Change: Monday 10 June 2019
   Last Commit: $Date: 2019-06-10 15:29:07 -0600 (Mon, 10 Jun 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Xml.Element.h"
#include "Csi.StrAscStream.h"
#include "Csi.Base64.h"
#include "Csi.Utils.h"
#include "Csi.LoginManager.h"
#include "CsiTypes.h"
#include <algorithm>
#include <iomanip>


namespace Csi
{
   namespace Xml
   {
      std::locale Element::my_locale = std::locale::classic();


      Element::~Element()
      {
         while(!elements.empty())
         {
            value_type &child = elements.front();
            child->parent = 0;
            elements.pop_front();
         }
      } // destructor
      

      void Element::add_attribute(
         StrUni const &attr_name,
         StrUni const &attr_value)
      {
         assert(attr_name.length() > 0);
         attributes[attr_name] = attr_value;
      }

      
      StrUni &Element::get_attribute(StrUni const &attr_name, StrUni const &namespace_uri)
      {
         // we need to determine the decoration, if any for the attribute name.  Because this is a
         // high volume function, we want to avoid allocating a temporary string unless it is
         // needed.  It costs little to create an empty temporary so we will do that and use a
         // pointer reference the "real" decorated name
         StrUni const *decorated_name = &attr_name;
         StrUni temp;
         
         if(namespace_uri.length() > 0)
         {
            get_decorated_identifier(temp, attr_name, namespace_uri);
            decorated_name = &temp;
         }

         // we can now search for the specified attribute
         attributes_type::iterator ai = attributes.find(*decorated_name);
         
         if(ai == attributes.end())
         {
            ai = attributes.insert(
               attributes_type::value_type(
                  *decorated_name,
                  L"")).first;
         } 
         return ai->second;
      } // get_attribute


      bool Element::has_attribute(StrUni const &attr_name, StrUni const &namespace_uri)
      {
         StrUni temp;
         StrUni const *decorated_name = &attr_name;
         if(namespace_uri.length() > 0)
         {
            get_decorated_identifier(temp, attr_name, namespace_uri);
            decorated_name = &temp;
         }
         return attributes.find(*decorated_name) != attributes.end(); 
      } // has_attribute


      void Element::remove_attribute(StrUni const &attr_name, StrUni const &namespace_uri)
      {
         StrUni temp;
         StrUni const *decorated_name(&attr_name);
         attributes_type::iterator ai;
         
         if(namespace_uri.length() > 0)
         {
            get_decorated_identifier(temp, attr_name, namespace_uri);
            decorated_name = &temp;
         }
         ai = attributes.find(*decorated_name);
         if(ai != attributes.end())
            attributes.erase(ai);
      } // remove_attribute
      

      StrUni Element::get_identifier_namespace(StrUni const &identifier) const
      {
         // our first task is to determine the name of the namespace identifier for the specified
         // identifier.  We will start assuming a "default" namespace.  We will then look for a more
         // specific identifier if a colon is found in the given name.
         StrUni rtn;
         StrUni namespace_id(L"xmlns");
         size_t colon_pos = identifier.find(L":");

         if(colon_pos < identifier.length())
         {
            namespace_id.append(L":");
            namespace_id.append(identifier.c_str(), colon_pos);
         }

         // we now need to search our own attributes or the parent's attributes to find the
         // namespace identifier.  This search must continue up to the root until a match is found
         Element const *element = this;
         while(rtn.length() == 0 && element != 0)
         {
            attr_const_iterator ai = element->attributes.find(namespace_id);
            if(ai != element->attributes.end())
               rtn = ai->second;
            else
               element = element->parent;
         }
         return rtn;
      } // get_identifier_namespace


      namespace
      {
         struct attr_has_value
         {
            StrUni const &value;
            attr_has_value(StrUni const &value_):
               value(value_)
            { }

            bool operator ()(Element::attr_value_type const &attr) const
            { return attr.second == value; }
         };
      };

      
      void Element::get_decorated_identifier(
         StrUni &decorated_name,
         StrUni const &undecorated_name,
         StrUni const &namespace_uri) const
      {
         decorated_name.cut(0);
         if(namespace_uri.length() > 0)
         {
            Element const *element = this;
            bool found_def = false;

            while(element != 0 && !found_def)
            {
               attr_const_iterator ai = std::find_if(
                  element->attr_begin(),
                  element->attr_end(),
                  attr_has_value(namespace_uri));
               if(ai != element->attr_end())
               {
                  StrUni const &def = ai->first;
                  size_t def_colon_pos = def.find(L":");
                  if(def_colon_pos < def.length())
                  {
                     def.sub(decorated_name, def_colon_pos + 1, def.length());
                     decorated_name.append(L':');
                  }
                  found_def = true;
               }
               else
                  element = element->parent;
            }
         }

         // we can now decorate the tag.
         size_t undecorated_colon_pos = undecorated_name.find(L":");
         if(undecorated_colon_pos >= undecorated_name.length())
            decorated_name.append(undecorated_name);
         else
         {
            StrUni temp;
            undecorated_name.sub(temp, undecorated_colon_pos + 1, undecorated_name.length());
            decorated_name.append(temp);
         }
      } // get_decorated_identifier


      namespace
      {
         class elem_has_id
         {
         public:
            StrUni name;
            int pos;

         public:
            elem_has_id(StrUni const &name_, int pos_):
               name(name_),
               pos(pos_)
            { }

            bool operator ()(SharedPtr<Element> &e)
            {
               bool rtn = false;
               if(e->get_name() == name)
               {
                  if(pos-- == 0)
                     rtn = true;
               }
               return rtn;
            }
         };
      };


      bool Element::operator ==(Element const &other) const
      {
         // we will consider two elements to be equal if they share the same name, have the same
         // number and values of attributes, have the same number of sub-elements, and each
         // sub-element in one element is equal to the sub-element of the other element in the same
         // position.  Note that this definitions becomes recursive.
         bool rtn =
            (name == other.name) &&
            (attributes.size() == other.attributes.size()) &&
            (elements.size() == other.elements.size());
         attributes_type::const_iterator ai = attributes.begin();
         attributes_type::const_iterator oai = other.attributes.begin();
         while(rtn && ai != attributes.end() && oai != other.attributes.end())
         {
            if(oai != other.attributes.end())
               rtn = (ai->second == oai->second);
            else
               rtn = false;
            ++oai;
            ++ai;
         }
         if(rtn && elements.empty())
            rtn = (cdata == other.cdata);
         elements_type::const_iterator ei = elements.begin();
         elements_type::const_iterator oei = other.elements.begin();
         while(rtn && ei != elements.end() && oei != other.elements.end())
         {
            element_type const &child = *ei++;
            element_type const &other_child = *oei++;
            rtn = (*child == *other_child);
         }
         return rtn;
      } // equality operator
      

      Element::attr_iterator Element::find_attr(
         StrUni const &attr_name, StrUni const &namespace_uri)
      {
         StrUni temp;
         StrUni const *decorated_name = &attr_name;
         if(namespace_uri.length() > 0)
         {
            get_decorated_identifier(temp, attr_name, namespace_uri);
            decorated_name = &temp;
         }
         return attributes.find(*decorated_name);
      } // find_attr


      Element::attr_const_iterator Element::find_attr(
         StrUni const &attr_name, StrUni const &namespace_uri) const
      {
         StrUni temp;
         StrUni const *decorated_name = &attr_name;
         if(namespace_uri.length() > 0)
         {
            get_decorated_identifier(temp, attr_name, namespace_uri);
            decorated_name = &temp;
         }
         return attributes.find(*decorated_name);
      } // find_attr

      
      Element::iterator Element::find(
         StrUni const &name, int pos, StrUni const &namespace_uri)
      {
         iterator rtn = elements.end();
         StrUni temp;
         int count = 0;
         for(iterator ei = elements.begin(); ei != elements.end() && rtn == elements.end(); ++ei)
         {
            value_type &child = *ei;
            StrUni const *decorated_name = &name;
            if(namespace_uri.length() > 0)
            {
               child->get_decorated_identifier(temp, name, namespace_uri);
               decorated_name = &temp; 
            }
            if(child->name == *decorated_name && ++count > pos)
               rtn = ei;
         }
         return rtn;
      } // find


      Element::value_type Element::find_elem(
         StrUni const &name, int pos, bool create_if_needed, StrUni const &namespace_uri)
      {
         int count = 0;
         StrUni temp;
         value_type rtn;
         for(iterator ei = elements.begin();
             rtn == 0 && count <= pos && ei != elements.end();
             ++ei)
         {
            value_type &child = *ei;
            StrUni const *decorated_name = &name;
            if(namespace_uri.length() > 0)
            {
               child->get_decorated_identifier(temp, name, namespace_uri);
               decorated_name = &temp;
            }
            if(child->name == *decorated_name)
            {
               if(count >= pos)
                  rtn = child;
               else
                  ++count; 
            }
         }
         if(rtn == 0 && create_if_needed)
         {
            StrUni const *decorated_name = &name;
            if(namespace_uri.length() > 0)
            {
               get_decorated_identifier(temp, name, namespace_uri);
               decorated_name = &temp;
            }
            while(count <= pos)
            {
               rtn = add_element(*decorated_name);
               ++count;
            }
         }
         else if(rtn == 0)
            throw std::invalid_argument("Specified element does not exist");
         return rtn;
      } // find_elem

      
      void Element::output(
         std::ostream &out,
         bool on_own_lines,
         int indent_level,
         bool dos_line_ends,
         bool output_header_first)
      {
         if(output_header_first)
         {
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
            if(dos_line_ends)
               out << "\r\n";
            else
               out << "\n";
         }
         for(int i = 0; on_own_lines && i < indent_level; ++i)
            out << "  ";
         out << "<" << name;
         for(attributes_type::iterator ai = attributes.begin();
             ai != attributes.end();
             ++ai)
         {
            if(ai->first.length() > 0)
            {
               out << " " << ai->first << "=\"";
               output_xml_data(out,ai->second);
               out << "\"";
            }
         }
         if(elements.empty() && cdata.length() == 0)
            out << " />";
         else
            out << ">";
         for(elements_type::iterator ei = elements.begin();
             ei != elements.end();
             ++ei)
         {
            if(on_own_lines)
            {
               if(dos_line_ends)
                  out << "\r\n";
               else
                  out << "\n";
            }
            (*ei)->output(out, on_own_lines, indent_level + 1, dos_line_ends, false);
         }
         if(cdata.length() != 0 && elements.empty())
         {
            if(!cdata_tagged)
               output_xml_data(out,cdata);
            else
               out << "<![CDATA[" << cdata << "]]>";
         }
         if(!elements.empty() || cdata.length() != 0)
         {
            if(on_own_lines && cdata.length() == 0)
            {
               if(dos_line_ends)
                  out << "\r\n";
               else
                  out << "\n";
               for(int i = 0; i < indent_level; ++i)
                  out << "  ";
            }
            out << "</" << name << ">";
         }
      } // output


      namespace
      {
         enum state_type
         {
            state_initial,
            state_utf8_bom_ef,
            state_utf8_bom_bb,
            state_start_tag,
            state_cdata_start_exclaim,
            state_comment_start1,
            state_comment_start2,
            state_comment_end1,
            state_cdata_start_brace1,
            state_cdata_start_c,
            state_cdata_start_d,
            state_cdata_start_a1,
            state_cdata_start_t,
            state_cdata_start_a2,
            state_parse_cdata,
            state_cdata_end_brace1,
            state_cdata_end_brace2,
            state_ignored,
            state_read_elem_name,
            state_between_attr,
            state_read_attr_name,
            state_read_attr_equal,
            state_read_attr_quote,
            state_read_attr_double_quote,
            state_read_attr_single_quote,
            state_read_content,
            state_empty,
            state_read_end_name,
            state_read_after_end_name,
            state_complete
         };
         struct element_state_type
         {
            element_state_type(
               Element *element_,
               state_type state_ = state_read_elem_name):
               element(element_),
               state(state_),
               start_pos(0)
            { }

            element_state_type(element_state_type const &other):
               state(other.state),
               element(other.element),
               start_pos(other.start_pos)
            { }

            element_state_type &operator =(element_state_type const &other)
            {
               state = other.state;
               element = other.element;
               start_pos = other.start_pos;
               return *this;
            }
            
            state_type state;
            Element *element;
            int8 start_pos;
         };
      };
      

      void Element::input(std::istream &in)
      {
         // we will use a stack structure to parse the recursive elements.  We'll begin by putting
         // our own pointer at the base of the list.  As recursive elements are found, we will push
         // these to the back of the list and pop them off when their end is found.
         typedef std::list<element_state_type> element_states_type;
         element_states_type element_states;
         char ch;
         size_t row_no = 0, column_no = 0;
         StrAsc temp;
         StrUni attr_name;
         int8 current_pos;
         
         name.cut(0);
         cdata.cut(0);
         elements.clear();
         attributes.clear();
         element_states.push_back(element_state_type(this,state_initial));
         current_pos = in.tellg();
         while(!element_states.empty() && in.get(ch))
         {
            // we will keep track of the row and column in the input in case of a syntax failure.
            current_pos = in.tellg(); --current_pos;
            if(ch == '\n')
            {
               ++row_no;
               column_no = 0;
            }
            else
               ++column_no;

            // we will work with the most recent tag state
            element_state_type *current = &element_states.back();
            switch(current->state)
            {
            case state_initial:
               if(ch == '<')
               {
                  current->state = state_start_tag;
                  current->start_pos = current_pos;
               }
               else if(ch == 0xEF && row_no == 0 && column_no == 1)
                  current->state = state_utf8_bom_ef;
               else if(!isspace(ch))
                  throw ParsingError("Invalid character",row_no,column_no);
               break;

            case state_utf8_bom_ef:
               if(ch == 0xBB)
                  current->state = state_utf8_bom_bb;
               else
                  throw ParsingError("Invalid character in byte order mark", row_no, column_no);
               break;
               
            case state_utf8_bom_bb:
               if(ch == 0xBF)
                  current->state = state_initial;
               else
                  throw ParsingError("Invalid character in byte order mark", row_no, column_no);
               break;
               
            case state_start_tag:
               if(ch == '?')
                  current->state = state_ignored;
               else if(ch == '!')
                  current->state = state_cdata_start_exclaim;
               else if(ch == '/')
               {
                  current->state = state_read_end_name;
                  temp.cut(0);
               }
               else if(!isspace(ch))
               {
                  if(name.length() != 0)
                  {
                     current->element->get_cdata_mod().cut(0);
                     element_states.push_back(
                        element_state_type(
                           current->element->add_element(
                              new Element(L"")).get_rep()));
                     current->state = state_read_content;
                     element_states.back().element->set_begin_offset(current->start_pos);
                     current = &element_states.back();
                  }
                  temp.cut(0);
                  if(ch != '<')
                     temp.append(&ch,1);
                  current->state = state_read_elem_name;
               }
               break;

            case state_cdata_start_exclaim:
               if(ch == '[')
                  current->state = state_cdata_start_brace1;
               else if(ch == '-')
                  current->state = state_comment_start1;
               else
                  current->state = state_ignored;
               break;
               
            case state_cdata_start_brace1:
               if(ch == 'C')
                  current->state = state_cdata_start_c;
               else
                  current->state = state_ignored;
               break;
               
            case state_cdata_start_c:
               if(ch == 'D')
                  current->state = state_cdata_start_d;
               else
                  current->state = state_ignored;
               break;
               
            case state_cdata_start_d:
               if(ch == 'A')
                  current->state = state_cdata_start_a1;
               else
                  current->state = state_ignored;
               break;
               
            case state_cdata_start_a1:
               if(ch == 'T')
                  current->state = state_cdata_start_t;
               else
                  current->state = state_ignored;
               break;
               
            case state_cdata_start_t:
               if(ch == 'A')
                  current->state = state_cdata_start_a2;
               else
                  current->state = state_ignored;
               break;
               
            case state_cdata_start_a2:
               if(ch == '[')
               {
                  temp.cut(0);
                  current->state = state_parse_cdata;
               }
               else
                  current->state = state_ignored;
               break;
               
            case state_parse_cdata:
               if(ch != ']')
                  temp.append(&ch,1);
               else
                  current->state = state_cdata_end_brace1;
               break;
               
            case state_cdata_end_brace1:
               if(ch == ']')
                  current->state = state_cdata_end_brace2;
               else
               {
                  current->state = state_parse_cdata;
                  temp.append(']');
                  temp.append(ch);
               }
               break;
               
            case state_cdata_end_brace2:
               if(ch == '>')
               {
                  current->state = state_read_content;
                  current->element->get_cdata_mod().append_mb(temp.c_str());
                  current->element->set_cdata_tagged(true);
               }
               else
               {
                  current->state = state_parse_cdata;
                  temp.append("]]"); 
               }
               break;

            case state_comment_start1:
               if(ch == '-')
                  current->state = state_comment_start2;
               else
                  current->state = state_ignored;
               break;
               
            case state_comment_start2:
               if(ch == '-')
                  current->state = state_comment_end1;
               break;
               
            case state_comment_end1:
               if(ch == '-')
                  current->state = state_ignored;
               else
                  current->state = state_comment_start2;
               break;
               
            case state_ignored:
               if(ch == '>')
                  current->state = state_initial;
               break;
               
            case state_read_elem_name:
               if(ch == '>')
               {
                  current->element->set_name(temp.c_str());
                  temp.cut(0);
                  current->state = state_read_content;
               }
               else if(ch == '/')
               {
                  current->state = state_empty;
                  current->element->set_name(temp.c_str());
               }
               else if(!isspace(ch))
                  temp.append(&ch,1);
               else
               {
                  current->element->set_name(temp.c_str());
                  current->state = state_between_attr;
               }
               break;
               
            case state_between_attr:
               if(ch == '/')
                  current->state = state_empty;
               else if(ch == '>')
               {
                  temp.cut(0);
                  current->state = state_read_content;
               }
               else if(!isspace(ch))
               {
                  temp.cut(0);
                  temp.append(&ch,1);
                  current->state = state_read_attr_name;
               }
               break;
               
            case state_read_attr_name:
               if(isspace(ch))
               {
                  attr_name = temp.c_str();
                  temp.cut(0);
                  current->state = state_read_attr_equal;
               }
               if(ch == '=')
               {
                  attr_name = temp.c_str();
                  temp.cut(0);
                  current->state = state_read_attr_quote;
               }
               else
                  temp.append(&ch,1);
               break;

            case state_read_attr_equal:
               if(ch == '=')
                  current->state = state_read_attr_quote;
               else if(!isspace(ch))
                  throw ParsingError("No equal sign for an attribute",row_no,column_no);
               break;
                  
            case state_read_attr_quote:
               if(ch == '\"')
                  current->state = state_read_attr_double_quote;
               else if(ch == '\'')
                  current->state = state_read_attr_single_quote;
               else if(!isspace(ch))
                  throw ParsingError("Attribute value improperly quoted",row_no,column_no);
               break;
               
            case state_read_attr_double_quote:
               if(ch != '\"')
                  temp.append(&ch,1);
               else
               {
                  try
                  {
                     StrUni attr_value;
                     input_xml_data(attr_value,temp.c_str(),temp.length());
                     current->element->add_attribute(attr_name, attr_value);
                     current->state = state_between_attr;
                  }
                  catch(std::exception &e)
                  { throw ParsingError(e.what(), row_no, column_no); }
               }
               break;
               
            case state_read_attr_single_quote:
               if(ch != '\'')
                  temp.append(&ch,1);
               else
               {
                  try
                  {
                     StrUni attr_value;
                     input_xml_data(attr_value,temp.c_str(),temp.length());
                     current->element->add_attribute(attr_name, attr_value);
                     current->state = state_between_attr;
                  }
                  catch(std::exception &e)
                  { throw ParsingError(e.what(), row_no, column_no); }
               }
               break;
               
            case state_read_content:
               if(ch != '<')
                  temp.append(&ch,1);
               else
               {
                  try
                  {
                     current->element->set_cdata(L"");
                     input_xml_data(
                        current->element->get_cdata_mod(),
                        temp.c_str(),
                        temp.length());
                     temp.cut(0);
                     current->state = state_start_tag;
                     current->start_pos = current_pos;
                  }
                  catch(std::exception &e)
                  { throw ParsingError(e.what(), row_no, column_no); }
               }
               break;

            case state_read_end_name:
               if(ch == '>' || isspace(ch))
               {
                  if(current->element->get_name() == StrUni(temp.c_str()))
                  {
                     if(ch == '>')
                     {
                        current->element->set_end_offset(current_pos);
                        element_states.pop_back();
                     }
                     else
                        current->state = state_read_after_end_name;
                  }
                  else
                     throw ParsingError("End tag mismatch",row_no,column_no);
               }
               else
                  temp.append(&ch,1);
               break;

            case state_read_after_end_name:
            case state_empty:
               if(ch == '>')
               {
                  current->element->set_end_offset(current_pos);
                  element_states.pop_back();
               }
               else if(!isspace(ch))
                  throw ParsingError("Invalid content in end tag",row_no,column_no);
               break;
            }
         }
         if(!element_states.empty())
         {
            if(element_states.back().element == this)
               throw ParsingError("Element never started",row_no,column_no);
            else
               throw ParsingError("Element started but not stopped",row_no,column_no);
         }
      } // input


      namespace
      {
         ////////////////////////////////////////////////////////////
         // template extract_cdata_value
         ////////////////////////////////////////////////////////////
         template <class T>
         T extract_cdata_value(Element *element)
         {
            IElementStream s(element);
            T rtn;

            s >> rtn;
            if(!s)
               throw Csi::MsgExcept("XML cdata extraction error");
            return rtn;
         }

         template <>
         int1 extract_cdata_value<int1>(Element *element)
         { return static_cast<int1>(extract_cdata_value<int2>(element)); }

         template <>
         uint1 extract_cdata_value<uint1>(Element *element)
         { return static_cast<uint1>(extract_cdata_value<uint2>(element)); }

         template <>
         bool extract_cdata_value<bool>(Element *element)
         {
            bool rtn;
            if(element->get_cdata() == L"true" || element->get_cdata() == L"1")
               rtn = true;
            else if(element->get_cdata() == L"false" || element->get_cdata() == L"0")
               rtn = false;
            else
               throw std::invalid_argument("Cannot read cdata bool");
            return rtn;
         }
               
            

         ////////////////////////////////////////////////////////////
         // template insert_cdata_value
         ////////////////////////////////////////////////////////////
         template <class T>
         void insert_cdata_value(T const &v, Element *element)
         {
            OElementStream s(element);
            s.clear();
            s << v;
         }


         template <>
         void insert_cdata_value<int1>(int1 const &v, Element *element)
         { insert_cdata_value<int2>(v,element); }


         template <>
         void insert_cdata_value<uint1>(uint1 const &v, Element *element)
         { insert_cdata_value<uint2>(v,element); }

         template <>
         void insert_cdata_value<bool>(bool const &v, Element *element)
         { element->set_cdata(v ? L"true" : L"false"); }


         ////////////////////////////////////////////////////////////
         // template extract_attr_value
         ////////////////////////////////////////////////////////////
         template <class T>
         T extract_attr_value(
            Element *element,
            StrUni const &attr_name,
            StrUni const &namespace_uri)
         {
            T rtn;
            IAttributeStream s(element, attr_name, namespace_uri);
            s >> rtn;
            if(!s)
            {
               OStrAscStream msg;
               msg << "Failed to get attribute \""
                   << attr_name << "\" from element \""
                   << element->get_name() << "\"";
               throw Csi::MsgExcept(msg.str().c_str());
            }
            return rtn; 
         }


         template <>
         int1 extract_attr_value<int1>(
            Element *element,
            StrUni const &attr_name,
            StrUni const &namespace_uri)
         {
            return static_cast<int1>(
               extract_attr_value<int2>(
                  element, attr_name, namespace_uri));
         }


         template <>
         uint1 extract_attr_value<uint1>(
            Element *element,
            StrUni const &attr_name,
            StrUni const &namespace_uri)
         {
            return static_cast<uint1>(
               extract_attr_value<uint2>(
                  element, attr_name, namespace_uri));
         }

         template <>
         bool extract_attr_value<bool>(
            Element *element,
            StrUni const &attr_name,
            StrUni const &namespace_uri)
         {
            bool rtn;
            StrUni const &attr_val = element->get_attribute(attr_name, namespace_uri);
            if(attr_val == L"true" || attr_val == L"1")
               rtn = true;
            else if(attr_val == L"false" || attr_val == L"0")
               rtn = false;
            else
               throw std::invalid_argument("Unable to interpret attribute as boolean");
            return rtn;
         }


         template <>
         float extract_attr_value<float>(
            Element *element,
            StrUni const &attr_name,
            StrUni const &namespace_uri)
         {
            StrUni const &attr_val(element->get_attribute(attr_name, namespace_uri));
            float rtn(
               static_cast<float>(
                  csiStringToFloat(attr_val.c_str(), Element::my_locale, true)));
            return rtn;
         }


         template <>
         double extract_attr_value<double>(
            Element *element,
            StrUni const &attr_name,
            StrUni const &namespace_uri)
         {
            StrUni const &attr_val(element->get_attribute(attr_name, namespace_uri));
            double rtn(
               csiStringToFloat(
                  attr_val.c_str(), Element::my_locale, true));
            return rtn;
         }


         ////////////////////////////////////////////////////////////
         // template insert_attr_value
         ////////////////////////////////////////////////////////////
         template <class T>
         void insert_attr_value(T const &v, Element *element, StrUni const &attr_name)
         {
            OAttributeStream s(element, attr_name, L"");
            assert(attr_name.length() > 0);
            s.clear();
            s << v;
         }


         template <>
         void insert_attr_value<int1>(int1 const &v, Element *element, StrUni const &attr_name)
         { insert_attr_value<int2>(v,element,attr_name); }


         template <>
         void insert_attr_value<uint1>(uint1 const &v, Element *element, StrUni const &attr_name)
         { insert_attr_value<uint2>(v,element,attr_name); }


         template <>
         void insert_attr_value<bool>(bool const &v, Element *element, StrUni const &attr_name)
         { element->add_attribute(attr_name,v ? L"true" : L"false"); }

         template <>
         void insert_attr_value<StrAsc>(StrAsc const &v, Element *e, StrUni const &attr_name)
         { e->add_attribute(attr_name, StrUni(v)); }

         template <>
         void insert_attr_value<float>(float const &v, Element *e, StrUni const &attr_name)
         {
            OAttributeStream out(e, attr_name);
            out.clear();
            csiFloatToStream(out, v);
         }

         template <>
         void insert_attr_value<double>(double const &v, Element *e, StrUni const &attr_name)
         {
            OAttributeStream out(e, attr_name);
            out.clear();
            csiFloatToStream(out, v, 15);
         }
      };


      void Element::set_cdata_bool(bool val)
      { insert_cdata_value<bool>(val,this); }

      
      void Element::set_cdata_uint1(uint1 val)
      { insert_cdata_value<uint1>(val,this); }

      
      void Element::set_cdata_int1(int1 val)
      { insert_cdata_value<int1>(val,this); }

      
      void Element::set_cdata_uint2(uint2 val)
      { insert_cdata_value<uint2>(val,this); }

      
      void Element::set_cdata_int2(int2 val)
      { insert_cdata_value<int2>(val,this); }

      
      void Element::set_cdata_uint4(uint4 val)
      { insert_cdata_value<uint4>(val,this); }

      
      void Element::set_cdata_int4(int4 val)
      { insert_cdata_value<int4>(val,this); }

      
      void Element::set_cdata_int8(int8 val)
      { insert_cdata_value<int8>(val,this); }


      bool Element::get_cdata_bool()
      { return extract_cdata_value<bool>(this); }


      void Element::set_cdata_float(float val)
      { insert_cdata_value<float>(val,this); }

      
      float Element::get_cdata_float()
      { return extract_cdata_value<float>(this); }


      void Element::set_cdata_double(double val)
      { insert_cdata_value(val,this); }


      double Element::get_cdata_double()
      { return extract_cdata_value<double>(this); }
      
      
      uint1 Element::get_cdata_uint1()
      { return extract_cdata_value<uint1>(this); }

      
      int1 Element::get_cdata_int1()
      { return extract_cdata_value<int1>(this); }

      
      uint2 Element::get_cdata_uint2()
      { return extract_cdata_value<uint2>(this); }

      
      int2 Element::get_cdata_int2()
      { return extract_cdata_value<int2>(this); }

      
      uint4 Element::get_cdata_uint4()
      { return extract_cdata_value<uint4>(this); }

      
      int4 Element::get_cdata_int4()
      { return extract_cdata_value<int4>(this); }

      
      int8 Element::get_cdata_int8()
      { return extract_cdata_value<int8>(this); }


      void Element::set_cdata_binary(StrBin const &val)
      {
         OElementStream s(this);
         s.clear();
         for(uint4 i = 0; i < val.length(); ++i)
            s << std::hex << std::setw(2) << std::setfill(L'0')
              << static_cast<uint4>(val[i]);
      } // set_cdata_binary

      
      void Element::set_cdata_lgrdate(LgrDate const &value)
      {
         OElementStream temp(this);
         temp.clear();
         value.format(temp, L"%Y-%m-%dT%H:%M:%S%x");
      }


      void Element::set_cdata_colour(Graphics::Colour const &value)
      {
         OElementStream temp(this);
         temp.clear();
         temp << value;
      }


      LgrDate Element::get_cdata_lgrdate()
      {
         StrAsc temp(cdata.to_utf8());
         return LgrDate::fromStr(temp.c_str());
      }


      Graphics::Colour Element::get_cdata_colour()
      { return Graphics::Colour(cdata.c_str()); }


      StrAsc Element::get_cdata_password() const
      {
         StrAsc rtn;
         LoginManager::decrypt_password(rtn, get_cdata_str());
         return rtn;
      } // get_cdata_password


      void Element::set_cdata_password(StrUni const &password)
      {
         StrAsc temp;
         LoginManager::encrypt_password(temp, password);
         cdata = temp;
      } // set_cdata_password


      void Element::get_cdata_binary(StrBin &val)
      {
         val.cut(0);
         for(uint4 i = 0; i < cdata.length(); i += 2)
         {
            wchar_t temp[3];
            if(!isxdigit(cdata[i]) || i + 1 >= cdata.length() || !isxdigit(cdata[i + 1]))
               throw MsgExcept("Invalid binary content");
            temp[0] = cdata[i];
            temp[1] = cdata[i + 1];
            temp[2] = 0;
            val.append(
               static_cast<byte>(
                  wcstoul(temp,0,16))); 
         }
      } // get_cdata_binary


      void Element::get_cdata_base64(StrBin &buff)
      {
         buff.cut(0);
         Base64::decode(buff, cdata.c_str(), cdata.length());
      } // get_cdata_base64


      void Element::set_cdata_base64(void const *buff, size_t buff_len)
      {
         OElementStream temp(this);
         temp.clear();
         Base64::encode(temp, buff, buff_len);
      } // set_cdata_base64


      void Element::set_attr_str(
         StrAsc const &val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<StrAsc>(val,this,attr_name); }


      void Element::set_attr_password(
         StrUni const &val, StrUni const &attr_name, StrUni const &namespace_uri)
      {
         StrAsc temp;
         LoginManager::encrypt_password(temp, val);
         set_attr_str(temp, attr_name, namespace_uri);
      } // set_attr_password

      
      void Element::set_attr_bool(
         bool val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<bool>(val,this,attr_name); }
      
      
      void Element::set_attr_uint1(
         uint1 val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<uint1>(val,this,attr_name); }

      
      void Element::set_attr_int1(
         int1 val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<int1>(val,this,attr_name); }

      
      void Element::set_attr_uint2(
         uint2 val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<uint2>(val,this,attr_name); }

      
      void Element::set_attr_int2(
         int2 val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<int2>(val,this,attr_name); }

      
      void Element::set_attr_uint4(
         uint4 val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<uint4>(val,this,attr_name); }

      
      void Element::set_attr_int4(
         int4 val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<int4>(val,this,attr_name); }

      
      void Element::set_attr_int8(
         int8 val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<int8>(val,this,attr_name); }

      
      void Element::set_attr_float(
         float val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<float>(val,this,attr_name); }


      void Element::set_attr_double(
         double val, StrUni const &attr_name, StrUni const &namespace_uri)
      { insert_attr_value<double>(val,this,attr_name); }


      void Element::set_attr_lgrdate(
         LgrDate const &date, StrUni const &attr_name, StrUni const &namespace_uri)
      {
         Csi::OStrAscStream temp;
         date.format(temp, "%Y-%m-%dT%H:%M:%S%x");
         set_attr_str(temp.str(), attr_name, namespace_uri);
      } // set_attr_lgrdate


      void Element::set_attr_colour(
         Graphics::Colour const &colour, StrUni const &attr_name, StrUni const &namespace_uri)
      {
         OAttributeStream output(this, attr_name, namespace_uri);
         output << colour;
      } // set_attr_colour


      StrAsc Element::get_attr_password(
         StrUni const &name, StrUni const &namespace_uri)
      {
         StrAsc temp(get_attr_str(name, namespace_uri));
         StrAsc rtn;
         Csi::LoginManager::decrypt_password(rtn, temp);
         return rtn;
      } // get_attr_password


      bool Element::get_attr_bool(
         StrUni const &name, StrUni const &namespace_uri)
      { return extract_attr_value<bool>(this, name, namespace_uri); }

      
      int1 Element::get_attr_int1(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<int1>(this, attr_name, namespace_uri); }

      
      uint1 Element::get_attr_uint1(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<uint1>(this, attr_name, namespace_uri); }

      
      int2 Element::get_attr_int2(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<int2>(this, attr_name, namespace_uri); }

      
      uint2 Element::get_attr_uint2(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<uint2>(this, attr_name, namespace_uri); }

      
      int4 Element::get_attr_int4(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<int4>(this, attr_name, namespace_uri); }

      
      uint4 Element::get_attr_uint4(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<uint4>(this, attr_name, namespace_uri); }

      
      int8 Element::get_attr_int8(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<int8>(this, attr_name, namespace_uri); }

      
      float Element::get_attr_float(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<float>(this, attr_name, namespace_uri); }

      double Element::get_attr_double(
         StrUni const &attr_name, StrUni const &namespace_uri)
      { return extract_attr_value<double>(this, attr_name, namespace_uri); }


      LgrDate Element::get_attr_lgrdate(
         StrUni const &attr_name, StrUni const &namespace_uri)
      {
         StrAsc temp = get_attr_str(attr_name, namespace_uri);
         return LgrDate::fromStr(temp.c_str());
      }


      Graphics::Colour Element::get_attr_colour(
         StrUni const &attr_name, StrUni const &namespace_uri)
      {
         StrUni temp(get_attr_wstr(attr_name, namespace_uri));
         return Graphics::Colour(temp.c_str());
      }
   };
};
