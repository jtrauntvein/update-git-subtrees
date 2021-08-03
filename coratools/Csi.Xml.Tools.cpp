/* Csi.Xml.Tools.cpp

   Copyright (C) 2008, 2008 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 10 March 2008
   Last Change: Monday 17 March 2008
   Last Commit: $Date: 2008-03-18 11:26:49 -0600 (Tue, 18 Mar 2008) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Xml.Tools.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace Xml
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate template_exists
         ////////////////////////////////////////////////////////////
         StrUni const template_name_name(L"name");
         StrUni const template_element_name(L"element");
         StrUni const template_attr_name(L"attribute");
         StrUni const template_unique_name(L"unique");
         struct template_matches
         {
            StrUni const &type_name;
            StrUni const &value;
            template_matches(
               StrUni const &type_name_,
               StrUni const &value_):
               type_name(type_name_),
               value(value_)
            { }
            bool operator ()(Element::value_type &template_elem)
            {
               bool rtn = false;
               if(template_elem->get_name() == type_name &&
                  template_elem->get_attr_wstr(template_name_name) == value)
                  rtn = true;
               return rtn;
            }
         };


         ////////////////////////////////////////////////////////////
         // find_template
         ////////////////////////////////////////////////////////////
         Element::value_type find_template(
            Element &template_xml,
            StrUni const &name,
            StrUni const &type_name)
         {
            Element::iterator ei = std::find_if(
               template_xml.begin(), template_xml.end(), template_matches(type_name, name));
            Element::value_type rtn;
            
            if(ei != template_xml.end())
               rtn = *ei;
            else
            {
               rtn = template_xml.add_element(type_name);
               rtn->set_attr_wstr(name, template_name_name);
            }
            return rtn;
         }
      };

      
      bool enum_context_addresses(
         EnumContextClient &client,
         Element &doc,
         Element &templ,
         StrAsc const &base_address)
      {
         Element::value_type doc_template(find_template(templ, doc.get_name(), template_element_name));
         bool proceed;
         OStrAscStream address;

         if(base_address.length() == 0)
            address << "/" << doc.get_name();
         else
            address << base_address;
         proceed = client.on_element(doc, *doc_template, address.str());
         for(Element::attr_iterator ai = doc.attr_begin();
             proceed && ai != doc.attr_end();
             ++ai)
         {
            Element::value_type attr_template(
               find_template(*doc_template, ai->first, template_attr_name));
            address.str("");
            if(base_address.length() == 0)
               address << '/' << doc.get_name() << "[" << ai->first << "]";
            else
               address << base_address << "[" << ai->first << "]";
            proceed = client.on_attribute(doc, *attr_template, address.str(), ai->first, ai->second);
         }

         // we now need to process all of the child elements
         size_t element_count = 0;
         StrUni unique_attr_name;
         for(Element::iterator ei = doc.begin();
             proceed && ei != doc.end();
             ++ei, ++element_count)
         {
            Element::value_type &child = *ei;
            Element::value_type child_template(
               find_template(*doc_template, child->get_name(), template_element_name));
            unique_attr_name = child_template->get_attr_wstr(template_unique_name);
            address.str("");
            if(base_address.length() == 0)
               address << "/" << doc.get_name() << '/' << child->get_name();
            else
               address << base_address << "/" << child->get_name();
            if(unique_attr_name.length() > 0)
               address << "(" << child->get_attr_wstr(unique_attr_name) << ")";
            else
               address << "(" << element_count << ")";
            proceed = enum_context_addresses(client, *child, *doc_template, address.str());
         }
         return proceed;
      } // enum_context_addresses


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate element_matches_key
         ////////////////////////////////////////////////////////////
         struct element_matches_key
         {
            StrUni const &elem_name;
            StrUni const &key_name;
            StrUni const &key_value;
            uint4 num_val;
            uint4 count;
            bool use_count;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            element_matches_key(
               StrUni const &elem_name_,
               StrUni const &key_name_,
               StrUni const &key_value_):
               elem_name(elem_name_),
               key_name(key_name_),
               key_value(key_value_),
               num_val(0),
               count(0),
               use_count(0)
            {
               if(key_name.length() == 0)
               {
                  use_count = true;
                  num_val = wcstoul(key_value.c_str(), 0, 10);
               }
            }

            ////////////////////////////////////////////////////////////
            // function call operator
            ////////////////////////////////////////////////////////////
            bool operator ()(Element::value_type &element)
            {
               bool rtn = false;
               if(element->get_name() == elem_name)
               {
                  if(use_count)
                  {
                     if(count == num_val)
                        rtn = true;
                     else
                        ++count;
                  }
                  else
                  {
                     Element::attr_const_iterator ai = element->find_attr(key_name);
                     if(ai != element->attr_end() && ai->second == key_value)
                        rtn = true;
                  }
               }
               return rtn;
            }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class AddressableItem definitions
      ////////////////////////////////////////////////////////////
      bool AddressableItem::find_item(
         Element &doc,
         Element &doc_template,
         StrAsc const &address,
         bool create_if_needed)
      {
         StrAsc element_name;
         StrUni key_name;
         StrUni key_value;
         bool at_root = true;
         StrUni element_name_uni;
         Element *element_template;
         
         element = &doc;
         is_valid = true;
         for(size_t pos = 0; is_valid && pos <= address.length(); ++pos)
         {
            if(pos < address.length() && address[pos] != '/')
               element_name.append(address[pos]);
            else if(element_name.length())
            {
               // the element needs to broken down into its components
               size_t lparen_pos = element_name.find("(");
               size_t rparen_pos = element_name.find(")", lparen_pos);
               size_t lbrace_pos = element_name.find("[");
               size_t rbrace_pos = element_name.find("]", lbrace_pos);
               if(lparen_pos < element_name.length())
               {
                  if(rparen_pos >= element_name.length())
                     throw std::invalid_argument("invalid address syntax");
                  key_value = element_name.c_str() + lparen_pos + 1;
                  key_value.cut(rparen_pos - lparen_pos - 1);
               }
               else
                  key_value.cut(0);
               if(lbrace_pos < element_name.length())
               {
                  if(rbrace_pos >= element_name.length())
                     throw std::invalid_argument("invalid address syntax");
                  attr_name = element_name.c_str() + lbrace_pos + 1;
                  attr_name.cut(rbrace_pos - lbrace_pos - 1);
               }
               else
                  attr_name.cut(0);
               element_name.cut(lparen_pos);
               element_name.cut(lbrace_pos);

               // if this is the root element, the name parsed must metch the name of the root
               // element
               element_name_uni = element_name.c_str();
               element_name.cut(0);
               if(at_root)
               {
                  if(element->get_name() != element_name_uni)
                     throw std::invalid_argument("incompatible root element");
                  element_template = find_template(
                     doc_template, element_name_uni, template_element_name).get_rep();
                  at_root = false;
               }
               else
               {
                  // locate the template that describes this element
                  element_template = find_template(
                     *element_template, element_name_uni, template_element_name).get_rep();
                  key_name = element_template->get_attr_wstr(template_unique_name);

                  // we now need to resolve the element ID
                  element_matches_key predicate(element_name_uni, key_name, key_value);
                  Element::iterator ei = std::find_if(element->begin(), element->end(), predicate);
                  
                  if(ei != element->end())
                     element = ei->get_rep();
                  else if(create_if_needed)
                  {
                     // the element described by the address does not exist and we are allowed to
                     // add it.  If we are identifying the element by position, we may need to "pad"
                     // other elements first so that the new element is in the right position.
                     while(predicate.use_count && predicate.count <= predicate.num_val)
                     {
                        element->add_element(element_name_uni);
                        ++predicate.count;
                     }
                     if(!predicate.use_count)
                     {
                        element = element->add_element(element_name_uni).get_rep();
                        element->set_attr_wstr(key_value, key_name);
                     }
                     else
                        element = element->back().get_rep();
                  }
                  else
                     is_valid = false; 
               } 
            }  
         }
         return is_valid;
      } // find_item
   };
};

