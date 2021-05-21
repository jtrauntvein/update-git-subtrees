/* Csi.Xml.EventParser.h

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 01 March 2016
   Last Change: Thursday 03 March 2016
   Last Commit: $Date: 2016-03-05 12:22:47 -0600 (Sat, 05 Mar 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Xml_EventParser_h
#define Csi_Xml_EventParser_h

#include "Csi.Xml.Utils.h"
#include "Csi.LightSharedPtr.h"
#include "CsiTypeDefs.h"
#include "Csi.LgrDate.h"
#include <deque>


namespace Csi
{
   namespace Xml
   {
      namespace EventParserHelpers
      {
         class ParserContext;
      };

      
      /**
       * Defines a component that can be used to parse an XML document from an input stream in
       * pieces.  The application can use this component by creating an instance of this class as
       * well as a std::istream instance.  It can then call the parse() method which will return
       * each time that an element begins, an attribute is found, or the end of an element is
       * encountered.  
       */
      class EventParser
      {
      public:
         /**
          * Default Constructor
          */
         EventParser();

         /**
          * Destructor
          */
         virtual ~EventParser();

         /**
          * Defines the values that can be returned from the parse() method.
          */
         enum parse_outcome_type
         {
            parse_start_of_element = 1,
            parse_attribute_read = 2,
            parse_end_of_element = 3,
            parse_end_of_document = 4
         };

         /**
          * Implements the algorithm that will parse the XML document one piece at a time.
          *
          * @return Returns the outcome of this call to parse().
          *
          * @param input Specifies the input stream to be read.
          *
          * @throws ParsingError Will throw an instance of ParsingError if the document contains an
          * error.
          */
         parse_outcome_type parse(std::istream &input);

         /**
          * Clears out any state from previous calls to parse().
          */
         void reset();

         /**
          * @return Returns the element namespace
          */
         StrUni const &get_elem_namespace() const
         { return elem_namespace; }

         /**
          * @return Returns hte element name.
          */
         StrUni const &get_elem_name() const
         { return elem_name; }

         /**
          * @return Returns the attribute namespace.
          */
         StrUni const &get_attr_namespace() const
         { return attr_namespace; }

         /**
          * @return Returns the attribute name.
          */
         StrUni const &get_attr_name() const
         { return attr_name; }

         /**
          * @return Returns the attribute value or element contents
          */
         StrUni const &get_value() const
         { return value; }

         /**
          * @return Returns the value interpreted as a boolean.
          */
         bool get_value_bool() const;

         /**
          * @return Returns the value interpreted as an int2.
          */
         int2 get_value_int2() const;

         /**
          * @return Returns the value interpreted as an uint2.
          */
         uint2 get_value_uint2() const;

         /**
          * @return Returns the value interpreted as an int4
          */
         int4 get_value_int4() const;

         /**
          * @return Returns the value interpreted as a uint4.
          */
         uint4 get_value_uint4() const;

         /**
          * @return Returns the value interpreeted as an int8.
          */
         int8 get_value_int8() const;

         /**
          * @return Returns the value interpreted as a double.
          */
         double get_value_double() const;

         /**
          * @return Returns the value interpreted as  timestamp.
          */
         LgrDate get_value_stamp() const;

      private:
         /**
          * Searches for the specified namespace identifier in the current and previous contexts. 
          *
          * @return Returns the namespace URI for the specified name.
          *
          * @param name Specifies the name to look up.
          */
         StrUni lookup_namespace_uri(StrUni const &name);
         
      private:
         /**
          * Specifies name of the current element.
          */
         StrUni elem_name;

         /**
          * Specifies the namespace for the current element.
          */
         StrUni elem_namespace;

         /**
          * Specifies the name of the current attribute.
          */
         StrUni attr_name;

         /**
          * Specifies the namespace for the current attribute.
          */
         StrUni attr_namespace;

         /**
          * Specifies the value of the current attribute or the content of the current element.
          */
         StrUni value;

         /**
          * Specifies the current row number.
          */
         size_t current_row;

         /**
          * Specifies the current column  number.
          */
         size_t current_col;

         /**
          * Specifies the offset from the beginning of the stream.
          */
         int8 next_offset;

         /**
          * Used to buffer the current token being read.
          */
         StrAsc token;

         /**
          * Specifies the contexts used for parsing XML elements.
          */
         typedef EventParserHelpers::ParserContext context_type;
         typedef LightSharedPtr<context_type> context_handle;
         typedef std::deque<context_handle> contexts_type;
         contexts_type contexts;
      };
   };
};


#endif
