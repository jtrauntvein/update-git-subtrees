/* Csi.Xml.Tools.h

   Copyright (C) 2008, 2008 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 10 March 2008
   Last Change: Wednesday 12 March 2008
   Last Commit: $Date: 2008-03-18 11:26:49 -0600 (Tue, 18 Mar 2008) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Xml_Tools_h
#define Csi_Xml_Tools_h

#include "Csi.Xml.Element.h"


namespace Csi
{
   namespace Xml
   {
      ////////////////////////////////////////////////////////////
      // class EnumContextClient
      //
      // Defines the interface for a class that enumerates all of the
      // addressable elements in an XML document.  Objects of this class are
      // expected when enum_context_addresses() is invoked.
      ////////////////////////////////////////////////////////////
      class EnumContextClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_element
         //
         // Called when enum_context_addresses() starts processing a new
         // element.  If the client returns false, the enumeration will be
         // aborted.
         ////////////////////////////////////////////////////////////
         virtual bool on_element(
            Element &element,
            Element &template_element,
            StrAsc const &address)
         { return true; }

         ////////////////////////////////////////////////////////////
         // on_attribute
         //
         // Called when enum_context_addresses() processes an attribute of the
         // specified element.  A return value of false will cause the
         // enumeration to be aborted.
         ////////////////////////////////////////////////////////////
         virtual bool on_attribute(
            Element &element,
            Element &template_element,
            StrAsc const &address,
            StrUni const &attribute_name,
            StrUni const &attribute_value)
         { return true; }
      };


      ////////////////////////////////////////////////////////////
      // enum_context_addresses
      //
      // Generates a list of all of the context addresses present in the source
      // document.  The client object's on_element and on_attribute() methods
      // will be invoked for each addressable element or attribute in the
      // document.  If either of these calls returns false, the enumeration
      // will be aborted.   The return value will be true to indicate that all
      // elements were processed or false to indicate that the client aborted
      // processing. 
      ////////////////////////////////////////////////////////////
      bool enum_context_addresses(
         EnumContextClient &client,
         Element &doc,
         Element &templ,
         StrAsc const &base_address = "");


      ////////////////////////////////////////////////////////////
      // class AddressableItem
      //
      // Defines an object that represents an "addressable" item in an XML
      // structure.  This item can be either the CDATA associated with an
      // element, the element itself, or an attribute of that element.
      ////////////////////////////////////////////////////////////
      class AddressableItem
      {
      private:
         ////////////////////////////////////////////////////////////
         // element
         ////////////////////////////////////////////////////////////
         Element *element;

         ////////////////////////////////////////////////////////////
         // attr_name
         ////////////////////////////////////////////////////////////
         StrUni attr_name;

         ////////////////////////////////////////////////////////////
         // is_valid
         ////////////////////////////////////////////////////////////
         bool is_valid;

      public:
         ////////////////////////////////////////////////////////////
         // default constructor
         ////////////////////////////////////////////////////////////
         AddressableItem():
            element(0),
            is_valid(false)
         { }

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         AddressableItem(AddressableItem const &other):
            element(other.element),
            attr_name(other.attr_name),
            is_valid(other.is_valid)
         { }

         ////////////////////////////////////////////////////////////
         // copy operator
         ////////////////////////////////////////////////////////////
         AddressableItem &operator =(AddressableItem const &other)
         {
            element = other.element;
            attr_name = other.attr_name;
            is_valid = other.is_valid;
            return *this;
         }

         ////////////////////////////////////////////////////////////
         // find_item
         //
         // Attempts to locate the item in the XML structure given by doc and
         // that is specified by address and template.  The create_if_needed
         // flag and default_value will control whether the element will be
         // generated if it cannot be found in the document.  The return value
         // will be true if the element could be found or created.
         ////////////////////////////////////////////////////////////
         bool find_item(
            Element &doc,
            Element &doc_template,
            StrAsc const &address,
            bool create_if_needed = true);

         ////////////////////////////////////////////////////////////
         // get_wstr
         ////////////////////////////////////////////////////////////
         StrUni get_wstr()
         {
            StrUni rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_wstr(attr_name);
            else
               rtn = element->get_cdata_wstr();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_str
         ////////////////////////////////////////////////////////////
         StrAsc get_str()
         {
            StrAsc rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_str(attr_name);
            else
               rtn = element->get_cdata_str();
            return rtn;
         } 

         ////////////////////////////////////////////////////////////
         // get_bool
         ////////////////////////////////////////////////////////////
         bool get_bool()
         {
            bool rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_bool(attr_name);
            else
               rtn = element->get_cdata_bool();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_uint1
         ////////////////////////////////////////////////////////////
         byte get_uint1()
         {
            byte rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_uint1(attr_name);
            else
               rtn = element->get_cdata_uint1();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_uint2
         ////////////////////////////////////////////////////////////
         uint2 get_uint2()
         {
            uint2 rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_uint2(attr_name);
            else
               rtn = element->get_cdata_uint2();
            return rtn;
         }
         
         ////////////////////////////////////////////////////////////
         // get_uint4
         ////////////////////////////////////////////////////////////
         uint4 get_uint4()
         {
            uint4 rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_uint4(attr_name);
            else
               rtn = element->get_cdata_uint4();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_int4
         ////////////////////////////////////////////////////////////
         int4 get_int4()
         {
            int4 rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_int4(attr_name);
            else
               rtn = element->get_cdata_int4();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_int8
         ////////////////////////////////////////////////////////////
         int8 get_int8()
         {
            int8 rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_int8(attr_name);
            else
               rtn = element->get_cdata_int8();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_float
         ////////////////////////////////////////////////////////////
         float get_float()
         {
            float rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_float(attr_name);
            else
               rtn = element->get_cdata_float();
         }

         ////////////////////////////////////////////////////////////
         // get_double
         ////////////////////////////////////////////////////////////
         double get_double()
         {
            double rtn;
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               rtn = element->get_attr_double(attr_name);
            else
               rtn = element->get_cdata_double();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // set_wstr
         ////////////////////////////////////////////////////////////
         void set_wstr(StrUni const &val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_wstr(val, attr_name);
            else
               element->set_cdata_wstr(val);
         }

         ////////////////////////////////////////////////////////////
         // set_str
         ////////////////////////////////////////////////////////////
         void set_str(StrAsc const &val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_str(val, attr_name);
            else
               element->set_cdata_str(val);
         }

         ////////////////////////////////////////////////////////////
         // set_uint1
         ////////////////////////////////////////////////////////////
         void set_uint1(uint1 val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_uint1(val, attr_name);
            else
               element->set_cdata_uint1(val);
         }

         ////////////////////////////////////////////////////////////
         // set_uint2
         ////////////////////////////////////////////////////////////
         void set_uint2(uint2 val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_uint2(val, attr_name);
            else
               element->set_cdata_uint2(val);
         }

         ////////////////////////////////////////////////////////////
         // set_uint4
         ////////////////////////////////////////////////////////////
         void set_uint4(uint4 val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_uint4(val, attr_name);
            else
               element->set_cdata_uint4(val);
         }

         ////////////////////////////////////////////////////////////
         // set_int2
         ////////////////////////////////////////////////////////////
         void set_int2(int2 val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_int2(val, attr_name);
            else
               element->set_cdata_int2(val);
         }

         ////////////////////////////////////////////////////////////
         // set_int4
         ////////////////////////////////////////////////////////////
         void set_int4(int4 val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_int4(val, attr_name);
            else
               element->set_cdata_int4(val);
         }

         ////////////////////////////////////////////////////////////
         // set_int8
         ////////////////////////////////////////////////////////////
         void set_int8(int8 val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_int8(val, attr_name);
            else
               element->set_cdata_int8(val);
         }

         ////////////////////////////////////////////////////////////
         // set_float
         ////////////////////////////////////////////////////////////
         void set_float(float val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_float(val, attr_name);
            else
               element->set_cdata_float(val);
         }

         ////////////////////////////////////////////////////////////
         // set_double
         ////////////////////////////////////////////////////////////
         void set_double(double val)
         {
            if(!is_valid)
               throw std::invalid_argument("Invalid item address");
            if(attr_name.length())
               element->set_attr_double(val, attr_name);
            else
               element->set_cdata_double(val);
         }
      };
   };
};


#endif
