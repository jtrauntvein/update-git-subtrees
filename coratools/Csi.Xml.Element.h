/* Csi.Xml.Element.h

   Copyright (C) 2006, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 19 May 2006
   Last Change: Friday 07 June 2019
   Last Commit: $Date: 2019-06-10 15:29:07 -0600 (Mon, 10 Jun 2019) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Xml_Element_h
#define Csi_Xml_Element_h

#include "StrBin.h"
#include "StrUni.h"
#include "Csi.SharedPtr.h"
#include "Csi.MsgExcept.h"
#include "Csi.BuffStream.h"
#include "Csi.LgrDate.h"
#include "CsiTypeDefs.h"
#include "Csi.Graphics.Colour.h"
#include "Csi.Xml.Utils.h"
#include <list>
#include <map>
#include <algorithm>


namespace Csi
{
   namespace Xml
   {
      /**
       * Defines an object that models an element in an XML document.
       */
      class Element
      {
      protected:
         /**
          * Specifies the collection of attributes for this element.
          */
         typedef std::map<StrUni, StrUni> attributes_type;
         attributes_type attributes;

         /**
          * Specifies the collection of child elements to this element.
          */
         typedef SharedPtr<Element> element_type;
         typedef std::list<element_type> elements_type;
         elements_type elements;

         /**
          * Specifies the name of this element.
          */
         StrUni name;

         /**
          * Specifies the non-element content of this element.
          */
         StrUni cdata;

         /**
          * Set to true if the content of this element is to be protected in a CDATA section.
          */
         bool cdata_tagged;

         /**
          * Specifies a reference to the element that includes this element.  If this reference is
          * null, then this element is a root element.
          */
         Element *parent;

         /**
          * Specifies the file offset where the declaration for this element began.
          */
         int8 begin_offset;

         /**
          * Specifies the file offset where the declaration for this element ended.
          */
         int8 end_offset;

      public:
         /**
          * Specifies the locale that will be used by the stream objects that are created in
          * association with content and attributes.
          */
         static std::locale my_locale;

         /**
          * Constructor
          *
          * @param name_ Specifies the name of this element.
          *
          * @param cdata_ Specifies the content for this element.
          */
         Element(
            StrUni const &name_,
            StrUni const &cdata_ = L""):
            name(name_),
            cdata(cdata_),
            cdata_tagged(false),
            parent(0)
         { }

         /**
          * copy constructor
          *
          * @param other Specifies the element to copy.
          *
          * @param deep_copy Set to true if all of all of the children of the "other" element should
          * be created as new element objects.
          */
         Element(Element const &other, bool deep_copy = false):
            name(other.name),
            cdata(other.cdata),
            cdata_tagged(other.cdata_tagged),
            parent(other.parent),
            attributes(other.attributes)
         {
            if(!deep_copy)
               elements = other.elements;
            else
            {
               for(elements_type::const_iterator ei = other.begin();
                   ei != other.end();
                   ++ei)
               {
                  value_type const &elem = *ei;
                  elements.push_back(new Element(*elem, true));
               }
            }
         } // copy constructor
         

         /**
          * Copy operator
          *
          * @param other Specifies the element to be copied.
          */
         Element &operator =(Element const &other)
         {
            name = other.name;
            attributes = other.attributes;
            elements = other.elements;
            cdata = other.cdata;
            cdata_tagged = other.cdata_tagged;
            parent = other.parent;
            return *this;
         }

         /**
          * @return Returns true if this element is functionally equal to the other element.  This
          * would mean that the name, all attributes, and the content is the same.
          */
         bool operator ==(Element const &other) const;

         /**
          * @return Returns true if this element is functionally different from the other element.
          * This would mean that the names, attributes, or contents might be different.
          */
         bool operator !=(Element const &other) const
         { return !operator ==(other); }

         /**
          * Destructor
          */
         virtual ~Element();

         /**
          * @return Returns the name for this element.
          */
         StrUni const &get_name() const
         { return name; }

         /**
          * @param name_ Specifies the name for this element.
          */
         void set_name(StrUni const &name_)
         { name = name_; }

         /**
          * @return Returns the parent to this element.
          */
         Element *get_parent()
         { return parent; }
         Element const *get_parent() const
         { return parent; }

         /**
          * @return Returns the input stream offset where this element began.
          */
         int8 get_begin_offset() const
         { return begin_offset; }

         /**
          * @param value Specifies the offset into the input stream where this element began.
          */
         void set_begin_offset(int8 value)
         { begin_offset = value; }

         /**
          * @return Returns the end offset into the input stream where this element ended.
          */
         int8 get_end_offset() const
         { return end_offset; }

         /**
          * @param value Specifies the end offset into the input stream where this element ended.
          */
         void set_end_offset(int8 value)
         { end_offset = value; }
         
         /**
          * @return Returns the content string for this element.
          */
         StrUni const &get_cdata() const
         { return cdata; }

         /**
          * @return Returns a modifiable reference to the content for this element.
          */
         StrUni &get_cdata_mod()
         { return cdata; }

         /**
          * @param cdata_ Specifies the content for this element.
          */
         void set_cdata(StrUni const &cdata_)
         { cdata = cdata_; }

         /**
          * @return returns true if the content of this element should be marked in a CDATA section.
          */
         bool get_cdata_tagged() const
         { return cdata_tagged; }

         /**
          * @param cdata_tagged_ Set to true if the content of this element should be marked in a
          * CDATA section.
          */
         void set_cdata_tagged(bool cdata_tagged_)
         { cdata_tagged = cdata_tagged_; }

         /**
          * Adds an attribute with a name and value.
          *
          * @param attr_name Specifies the name of the attribute to be added.
          *
          * @param attr_value Specifies the value for the attribute.
          */
         void add_attribute(
            StrUni const &attr_name,
            StrUni const &attr_value);

         /**
          * @return Returns a modiable reference to the value of the attribute that was specified.
          *
          * @param attr_name Specifies the name of the attribute to look up.
          *
          * @param namesoace_uri Specifies the XML namespace of the attribute.
          */
         StrUni &get_attribute(StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns true if this element has an attribute with the specified name.
          *
          * @param attr_name Specifies the name of the attribute.
          *
          8 @param namespace_uri Specifies the XML namespace for the attribute.
         */
         bool has_attribute(StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * Removes the identified attribute.
          *
          * @param attr_name Specifies the name of the attribute.
          *
          * @param namespace_uri Specifies the XML namespace for the attribute.
          */
         void remove_attribute(StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the reference to the element object that was added.
          *
          * @param element Specifies the element reference to add.
          *
          * @param elem_name Specifies the name of the element.
          *
          * @param cdata Specifies the content for the elemment.
          */
         Csi::SharedPtr<Element> add_element(
            Csi::SharedPtr<Element> element)
         {
            elements.push_back(element);
            element->parent = this;
            return element;
         }
         Csi::SharedPtr<Element> add_element(
            StrUni const &elem_name,
            StrUni const &cdata = L"")
         { return add_element(new Element(elem_name,cdata)); }

         /**
          * Resolves the namespace URI for the specified element or attribute name.  This is done by
          * searching the attributes of this element or a child element for a specific namespace
          * attribute (xmlns:xxx="uri") or default namespace attribute (xmlns="uri").
          *
          * @return Returns the contents of the URI or an empty string if no namespace definition
          * was located.
          *
          * @param identifier Specifies the namespace identifier.
          */
         StrUni get_identifier_namespace(StrUni const &identifier) const;

         /**
          * Determines the appropriate namespace decoration fro the specified namespace identifier
          * based upon the provided namespace URI.
          *
          * @param decorated_name Specifies the reference to the string that will return the
          * decoration.
          *
          * @param undecorated_name Specifies the undecorated identifier.
          *
          * @param namespace_uri Specifies the XML namespace.
          */
         void get_decorated_identifier(
            StrUni &decorated_name,
            StrUni const &undecorated_name,
            StrUni const &namespace_uri) const;
         
         // @group: element container methods

         /**
          * @return Returns the iterator that references the start of this element's child list.
          */
         typedef elements_type::value_type value_type;
         typedef elements_type::iterator iterator;
         typedef elements_type::const_iterator const_iterator;
         iterator begin()
         { return elements.begin(); }
         const_iterator begin() const
         { return elements.begin(); }

         /**
          * @return Returns the iterator that references beyond the end of this element's child
          * list.
          */
         iterator end()
         { return elements.end(); }
         const_iterator end() const
         { return elements.end(); }

         /**
          * @return Returns the number of child elements.
          */
         typedef elements_type::size_type size_type;
         size_type size() const
         { return elements.size(); }

         /**
          * @return Returns tru if this element has no children.
          */
         bool empty() const
         { return elements.empty(); }

         /**
          * Removes all child elements.
          */
         void clear()
         { elements.clear(); }

         /**
          * @return Returns the reverse iterator at the end of the child list.
          */
         typedef elements_type::reverse_iterator reverse_iterator;
         typedef elements_type::const_reverse_iterator const_reverse_iterator;
         reverse_iterator rbegin()
         { return elements.rbegin(); }
         const_reverse_iterator rbegin() const
         { return elements.rbegin(); }

         /**
          * @return Returns the reverse iterator beyond the beginning of the child list.
          */
         reverse_iterator rend()
         { return elements.rend(); }
         const_reverse_iterator rend() const
         { return elements.rend(); }

         /**
          * @return Returns a reference to the child element at the beginning of this element's
          * child list.
          */
         value_type &front()
         { return elements.front(); }
         value_type const &front() const
         { return elements.front(); }

         /**
          * @return Returns a reference to the last child element in this element's child list.
          */
         value_type &back()
         { return elements.back(); }
         value_type const &back() const
         { return elements.back(); }

         /**
          * Adds the specified element as a child to this element and at the end of this element's
          * child list.
          */
         void push_back(value_type &value)
         {
            value->parent = this;
            elements.push_back(value);
         }

         /**
          * Adds the specified element as a child at the beginning of this element's child list.
          */
         void push_front(value_type &value)
         {
            value->parent = this;
            elements.push_front(value);
         }

         /**
          * Removes the last element from this element's child list.
          */
         void pop_back()
         {
            if(!elements.empty())
               elements.pop_back();
         }

         /**
          * Removes the first element from this element's child list.
          */
         void pop_front()
         {
            if(!elements.empty())
               elements.pop_front();
         }

         /**
          * Removes the child element(s) at the position indicated by the iterator or range of
          * iterators.
          */
         void erase(iterator ei)
         { elements.erase(ei); }
         void erase(iterator first, iterator last)
         { elements.erase(first, last); }

         /**
          * Adds a child element at the position indicated by the iterator.
          */
         void insert(iterator pos, value_type &e)
         {
            e->parent = this;
            elements.insert(pos, e);
         }
         
         // @endgroup:


         // @group: attributes container methods

         /**
          * @return Returns an iterator at the start of this element's attributes.
          */
         typedef attributes_type::iterator attr_iterator;
         typedef attributes_type::const_iterator attr_const_iterator;
         attr_iterator attr_begin()
         { return attributes.begin(); }
         attr_const_iterator attr_begin() const
         { return attributes.begin(); }

         /**
          * @return Returns an iterator beyond the end of this element's attributes.
          */
         attr_iterator attr_end()
         { return attributes.end(); }
         attr_const_iterator attr_end() const
         { return attributes.end(); }

         /**
          * @return Returns the iterator associated with the specified attribute name.  Will return
          * the value of attr_end() if the attribute cannot be found.
          *
          * @param attr_name Specifies the attribute name.
          *
          * @param namespace_uri Specifies the XML namespace for the attribute.
          */
         attr_iterator find_attr(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");
         attr_const_iterator find_attr(
            StrUni const &attr_name, StrUni const &namespace_uri = L"") const;

         /**
          * @return Returns true if this element has no attributes.
          */
         bool attr_empty() const
         { return attributes.empty(); }

         /**
          * @return Returns the number of attributes for this element.
          */
         typedef attributes_type::size_type attr_size_type;
         attr_size_type attr_size() const
         { return attributes.size(); }

         /**
          * Removes all attributes from this element.
          */
         void attr_clear()
         { attributes.clear(); }

         /**
          * @return Returns a reference to the first attribute.
          */
         typedef attributes_type::value_type attr_value_type;
         attr_value_type &attr_front()
         { return *attributes.begin(); }
         attr_value_type const &attr_front() const
         { return *attributes.begin(); }

         /**
          * @return Returns a refernece to the last attribute.
          */
         attr_value_type &attr_back()
         { return *attributes.rbegin(); }
         attr_value_type const &attr_back() const
         { return *attributes.rbegin(); }

         /**
          * Removes the first attribute.
          */
         void attr_pop_front()
         {
            if(!attributes.empty())
               attributes.erase(attributes.begin());
         }

         /**
          * Removes the last attribute.
          */
         void attr_pop_back()
         {
            if(!attributes.empty())
               attributes.erase(--(attributes.begin()));
         }

         /**
          * Removes the attribute with the specified name or position.
          *
          * @param name Specifies the name of the attribute.
          *
          * @param ai Specifies the position of the attribute.
          */
         void attr_erase(attr_iterator ai)
         { attributes.erase(ai); }
         void attr_erase(StrUni const &attr_name)
         {
            attr_iterator ai(attributes.find(attr_name));
            if(ai != attributes.end())
               attributes.erase(ai);
         }
         
         // @endgroup:

         /**
          * @return Returns the pos_th element given the specified name.  Returns end() if there is
          * no such element.
          *
          * @param name Specifies the name of the element.
          *
          * @param pos Specifies the number of prevous elements to skip.
          *
          * @param namespace_uri Specifies the XML namespace for the element.
          */
         iterator find(
            StrUni const &name, int pos = 0, StrUni const &namespace_uri = L"");

         /**
          * Searches for the pos_th element given the specified name.
          *
          * @return Returns a shared pointer to the element that was located or created.
          *
          * @param pos Specifies the count of preceding elements that should be skipped.
          *
          * @param create_if_needed Set to true if the element and preceding elements are to be
          * created if the specified value cannot be found.  If this value is false and the element
          * is not found, an exception will be thrown,
          *
          * @param namespace_uri Specifies the XML namespace.
          */
         value_type find_elem(
            StrUni const &name,
            int pos = 0,
            bool create_if_needed = false,
            StrUni const &namespace_uri = L"");

         /**
          * Searches for the element that satisfies the specified predicate.
          *
          * @return Returns the child element that matched or a null reference if no match was
          * found.
          *
          * @param pred Specifies the predicate that will be called for each candidate element.
          *
          * @param recursive Set to true if the child elements and their children are to be searched
          * as well.
          */
         template<class pred_type>
         value_type find_elem_if(pred_type pred, bool recursive = true)
         {
            value_type rtn;
            iterator ei = std::find_if(begin(),end(),pred);
            if(ei == end() && recursive)
            {
               for(ei = begin(); rtn == 0 && ei != end(); ++ei)
               {
                  value_type &element = *ei;
                  rtn = element->find_elem_if(pred,recursive);
               }
            }
            else if(ei != end())
               rtn = *ei;
            return rtn;
         }

         /**
          * Formats the element to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param on_own_lines Set to true if the elements are supposed to be printed on their own,
          * indented lines.
          *
          * @param indent_level Specifies the number of indents that should be applied at the root
          * level.
          *
          * @param dos_line_ends Set to true if line endings are supposed to be written using
          * "\r\n".
          *
          * @param output_header_first Set to true if the XML file header should be written first.
          */
         virtual void output(
            std::ostream &out,
            bool on_own_lines = false,
            int indent_level = 0,
            bool dos_line_ends = true,
            bool output_header_first = false);

         /**
          * Parses the XML structure from the input stream.
          *
          * @param in Specifies the input stream.
          *
          * @throw std::exception Throws an exception if there is a syntax error encountered while
          * parsing the stream.
          */
         virtual void input(std::istream &in);

         // @group: cdata access methods

         // the following methods are used to set the cdata values as specific
         // types.  This provides a short cut to having the application declare
         // stream objects to extract or format these types. 

         /**
          * @param s Specifies the content string as a utf-8 string.
          */
         void set_cdata_str(StrAsc const &s)
         { cdata = s; }

         /**
          * @param s Specifies the value that will be encrypted as a password to be stored as the
          * content.
          */
         void set_cdata_password(StrUni const &s);

         /**
          * @param s Specifies the value that will be stored as a wide string (unicode) as the
          * content of this element.
          */
         void set_cdata_wstr(StrUni const &s)
         { cdata = s; }

         /**
          * @param val Specifies the value that will be stored as either "true" or "false" as the
          * content of this element.
          */
         void set_cdata_bool(bool val);

         /**
          * @param val Specifies the value that will be formatted as a single byte unsigned integer
          * as the content of this element.
          */
         void set_cdata_uint1(uint1 val);

         /**
          * @param val Specifies the value that will be formatted as a single byte signed integer as
          * the content of this element.
          */
         void set_cdata_int1(int1 val);

         /**
          * Specifies the value that will be formatted as a two byte unsigned integer as the content
          * of this element.
          */
         void set_cdata_uint2(uint2 val);

         /**
          * @param val Specifies the value that will be formatted as a two byte signed integer as
          * the content of this element.
          */
         void set_cdata_int2(int2 val);

         /**
          * @param val Specifies the value that will be formatted as a four byte unsigned integer as
          * the content of this element.
          */
         void set_cdata_uint4(uint4 val);

         /**
          * @param val Specifies the value that will be formatted as a four byte signed integer as
          * the content of this element.
          */
         void set_cdata_int4(int4 val);

         /**
          * @param val Specifies the value that will be formatted as an eight byte signed integer as
          * the content of this element.
          */
         void set_cdata_int8(int8 val);

         /**
          * @param val Specifis the value that will be formatted as a floating point number as the
          * content of this element.  The value will have a maximum of seven significant digits.
          */
         void set_cdata_float(float val);

         /**
          * @param val Specifies the value that will be formatted as a floating point number as the
          * content of this element.  The value will have a maximum of 15 significant digits.
          */
         void set_cdata_double(double val);

         /**
          * @param value Specifies the value that will be formatted as a time stamp as the content
          * of this element.
          */
         void set_cdata_lgrdate(LgrDate const &value);

         /**
          * Encodes the content of this element as a colour.
          *
          * @param value Specifies the colour to encode.
          */
         void set_cdata_colour(Graphics::Colour const &value);

         /**
          * @return Returns the content of this element as a utf-8 encoded string.
          */
         StrAsc get_cdata_str() const
         { return cdata.to_utf8(); }

         /**
          * @return Returns the content of this element as a decrypted password.
          */
         StrAsc get_cdata_password() const;

         /**
          * @return Returns the content of this element as a wide unicode string.
          */
         StrUni const &get_cdata_wstr() const
         { return cdata; }

         /**
          * @return Returns the content of this element interpreted as a boolean value ("true",
          * "false", "1", or "0").
          */
         bool get_cdata_bool();

         /**
          * @return Returns the content of this element interpreted as a single byte unsigned
          * integer.
          */
         uint1 get_cdata_uint1();

         /**
          * @return Returns the content of this element interpreted as a single byte signed integer.
          */
         int1 get_cdata_int1();

         /**
          * @return Returns the content of this element interpreted as a two byte unsigned integer.
          */
         uint2 get_cdata_uint2();

         /**
          * @return Returns the content of this element interpreted as a two byte signed integer.
          */
         int2 get_cdata_int2();

         /**
          * @return Returns the content of this element interpreted as a four byte unsigned integer.
          */
         uint4 get_cdata_uint4();

         /**
          * @return Returns the content of this element interpreted as a four byte signed integer.
          */
         int4 get_cdata_int4();

         /**
          * @return Returns the content of this element interpreted as an eight byte unsigned
          * integer.
          */
         int8 get_cdata_int8();

         /**
          * @return Returns the content of this element interpreted as a floating point value.
          */
         float get_cdata_float();
         double get_cdata_double();

         /**
          * @param val Specifies the value that will be formatted as a binary hex encoded string in
          * the content for this element.
          */
         void set_cdata_binary(StrBin const &val);

         /**
          * Interprets the element content as binary data encoded as a hex string.
          *
          * @param dest Specifies the buffer to which the binary data will be written.
          */
         void get_cdata_binary(StrBin &dest);

         /**
          * @return Returns the element content interpreted as a time stamp.
          */
         LgrDate get_cdata_lgrdate();

         /**
          * @return Returns the content of this element interpreted as an
          * encoded colour.
          */
         Graphics::Colour get_cdata_colour();

         /**
          * Interprets the element content as binary encoded using base64.
          *
          * @param dest Specifies the buffer to which the decoded data will be written.
          */
         void get_cdata_base64(StrBin &dest);

         /**
          * Writes the content of this element as a base64 encoded binary string.
          *
          * @param buff Specifies the start of the buffer to encode.
          *
          * @param buff_len Specifies the number of bytes to encode.
          */
         void set_cdata_base64(void const *buff, size_t buff_len);
         
         // @endgroup:

         // @group: attribute access methods

         // the following methods allow the application to set and get
         // attribute values as specific types.  These methods provide
         // shortcuts that allow the application to bypass the creation of
         // stream types itself.

         /**
          * @param val Specifies the utf-8 encoded string that will be written to the attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_str(
            StrAsc const &val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @param val Specifies the unicode string that will be encrypted and set to the specified
          * attribute value.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */          
         void set_attr_password(
            StrUni const &val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @param value Specifies the unicode string that will be set top the specified attribute
          * value.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */          
         void set_attr_wstr(
            StrUni const &val, StrUni const &attr_name, StrUni const &namespace_uri = L"")
         { add_attribute(attr_name,val); }

         /**
          * @param val Specifies the boolean value that will be written as the value for the
          * specified attribute ("true" or "false").
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_bool(
            bool val, StrUni const &name, StrUni const &namespace_uri = L"");

         /**
          * @param val Specifies the single byte unsigned integer that will be formatted for the
          * value of the specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_uint1(
            uint1 val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * Specifis the single byte signed integer that will be formatted for the value of the
          * specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_int1(
            int1 val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @param val Specifies the two byte unsigned integer that will be formatted as the value
          * of the specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_uint2(
            uint2 val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @param val Specifies the two byte signed integer that will be formatted as the value of
          * the specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_int2(
            int2 val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @param val Specifies the four byte unsigned integer that will be formatted as the value
          * of the specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_uint4(
            uint4 val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @param val Specifies the four byte signed integer that will be formatted as the value
          * for the specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_int4(
            int4 val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * Specifies the eight byte signed integer that will be formatted as the value of the
          * specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_int8(
            int8 val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @param val Specifies the floating point number that will be formatted as the value of
          * the specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_float(
            float val, StrUni const &attr_name, StrUni const &namespace_uri = L"");
         void set_attr_double(
            double val, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @param date Specifies the time stamp that will be formatted as the value of the
          * specified attribute.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         void set_attr_lgrdate(
            LgrDate const &date, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * Sets the value for the specified attribute as an encoded colour
          * string.
          *
          * @param colour Specifies the colour to write
          *
          * @param attr_name Specifies the name of the attribute.
          *
          * @param namespace_uri Specifies the namespace URI.
          */
         void set_attr_colour(
            Graphics::Colour const &colour, StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the specified attribute interpreted as a utf-8 encoded string.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         StrAsc get_attr_str(
            StrUni const &attr_name, StrUni const &namespace_uri = L"")
         {
            StrAsc rtn;
            rtn = get_attribute(attr_name, namespace_uri).to_utf8();
            return rtn;
         }

         /**
          * @return Returns the specified attribute value interpreted as an encrypted password.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         StrAsc get_attr_password(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the specified attribute value interpreted as a unicode string.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         StrUni const &get_attr_wstr(
            StrUni const &attr_name, StrUni const &namespace_uri = L"")
         { return get_attribute(attr_name, namespace_uri); }

         /**
          * @return Returns the specified attribute value interpreted as a boolean value.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         bool get_attr_bool(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the specified attribute value interpreted as a one byte signed integer.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         int1 get_attr_int1(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the specified attribute value interpreted as a one byte unsigned
          * integer.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         uint1 get_attr_uint1(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the specified attribute value interpreted as a two byte signed integer.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         int2 get_attr_int2(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the specified attribute value interpreted as a two byte unsigned
          * integer.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         uint2 get_attr_uint2(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the specified attribute of interpreted as a four byte signed integer.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         int4 get_attr_int4(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the value of the specified attribute interpreted as a four byte unsigned
          * integer.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         uint4 get_attr_uint4(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the value of the specified attribute interpreted as an eight byte signed
          * integer.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         int8 get_attr_int8(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the value of the specified attribute interpreted as a floating point
          * number.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         float get_attr_float(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");
         double get_attr_double(
            StrUni const &attr_name, StrUni const &namespace_uri = L"");

         /**
          * @return Returns the specified attribute value interpreted as a time stamp.
          *
          * @param attr_name Specifies the name of the attribute to change.
          *
          * @param namespace_uri Specifies the XML namespace of the attribute.
          */
         Csi::LgrDate get_attr_lgrdate(
            StrUni const &attr_name, StrUni const &namespace_uri_name = L"");

         /**
          * @return Returns the attributes specified by attr_name and
          * namespace_uri_name as a decoded colour string.
          *
          * @param attr_name Specifies the name of the attribute to read.
          *
          * @param namespace_uri_name Specifies the namespace for the
          * attribute.
          */
         Csi::Graphics::Colour get_attr_colour(
            StrUni const &attr_name, StrUni const &namespace_uri_name = L"");
         
         // @endgroup:
      };


      /**
       * Defines an object that represents a stream buffer associated with the content of an
       * element.
       */
      class OElementBuff: public std::wstreambuf
      {
      private:
         /**
          * Specifies the element.
          */
         Element *element;

         friend class OElementStream;

      public:
         /**
          * Constructor
          */
         OElementBuff(Element *element_):
            element(element_)
         { }

         /**
          * Overloads the base class version to handle the overflow event.
          */
         virtual int_type overflow(int_type ch)
         {
            element->get_cdata_mod().append(static_cast<wchar_t>(ch));
            return ch;
         }

         /**
          * Overloads the base class version to handle block write.
          */
         virtual std::streamsize xsputn(
            wchar_t const *buff,
            std::streamsize buff_len)
         {
            element->get_cdata_mod().append(buff,static_cast<size_t>(buff_len));
            return buff_len; 
         }

         /**
          * Clears the content of the element.
          */
         void clear()
         { element->get_cdata_mod().cut(0); }
      };


      /**
       * Defines an output stream object associated with an element's content.
       */
      class OElementStream: public std::wostream
      {
      protected:
         /**
          * Specifies the stream buffer.
          */
         OElementBuff buffer;

      public:
         /**
          * Constructor
          */
         OElementStream(Element *element):
            buffer(element),
            std::wostream(&buffer)
         { imbue(Element::my_locale); }

         /**
          * Clears the content of the element.
          */
         void clear()
         { buffer.clear(); }
      };


      /**
       * Defines an input stream assaociated with the content of an element.
       */
      class IElementStream: public IBuffStreamw
      {
      public:
         IElementStream(Element *element):
            IBuffStreamw(
               element->get_cdata().c_str(),
               element->get_cdata().length())
         { imbue(Element::my_locale); }

         IElementStream(SharedPtr<Element> &element):
            IBuffStreamw(
               element->get_cdata().c_str(),
               element->get_cdata().length())
         { imbue(Element::my_locale); }
      };
      

      /**
       * Defines an object that acts as a stream buffer associated with an element attribute.
       */
      class OAttributeBuff: public std::wstreambuf
      {
      private:
         Element *element;
         StrUni *attribute;

      public:
         OAttributeBuff(
            Element *element_,
            StrUni const &attr_name,
            StrUni const &namespace_uri):
            element(element_),
            attribute(&element_->get_attribute(attr_name, namespace_uri))
         { }

         virtual int_type overflow(int_type ch)
         {
            attribute->append(static_cast<wchar_t>(ch));
            return ch;
         }

         virtual std::streamsize xsputn(
            wchar_t const *buff,
            std::streamsize buff_len)
         {
            attribute->append(buff,static_cast<size_t>(buff_len));
            return buff_len;
         }

         void clear()
         { attribute->cut(0); }
      };


      /**
       * Defines an output stream associated with an element attribute.
       */
      class OAttributeStream: public std::wostream
      {
      private:
         OAttributeBuff buffer;
         
      public:
         OAttributeStream(
            Element *element,
            StrUni const &attr_name,
            StrUni const &namespace_uri = L""):
            buffer(element, attr_name, namespace_uri),
            std::wostream(&buffer)
         { imbue(Element::my_locale); }

         OAttributeStream(
            SharedPtr<Element> &element,
            StrUni const &attr_name,
            StrUni const &namespace_uri = L""):
            buffer(element.get_rep(), attr_name, namespace_uri),
            std::wostream(&buffer)
         { imbue(Element::my_locale); }

         void clear()
         { buffer.clear(); }
      };


      /**
       * Defines an input stream associated with an element attribute.
       */
      class IAttributeStream: public IBuffStreamw
      {
      public:
         IAttributeStream(
            Element *element,
            StrUni const &attr_name,
            StrUni const &namespace_uri = L""):
            IBuffStreamw(0, 0)
         {
            StrUni const &attr_value = element->get_attribute(attr_name, namespace_uri);
            buffer.set_buffer(attr_value.c_str(), attr_value.length());
            imbue(Element::my_locale);
         }

         IAttributeStream(
            SharedPtr<Element> &element,
            StrUni const &attr_name,
            StrUni const &namespace_uri = L""):
            IBuffStreamw(0, 0)
         {
            StrUni const &attr_value = element->get_attribute(attr_name, namespace_uri);
            buffer.set_buffer(attr_value.c_str(), attr_value.length());
            imbue(Element::my_locale);
         }
      };
   };
};


#endif
