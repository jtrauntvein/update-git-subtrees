/* Csi.Xml.EventParser.cpp

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 01 March 2016
   Last Change: Wednesday 22 June 2016
   Last Commit: $Date: 2016-06-22 11:34:26 -0600 (Wed, 22 Jun 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Xml.EventParser.h"
#include "Csi.BuffStream.h"
#include "Csi.StringLoader.h"
#include "CsiTypes.h"
#include <map>


namespace Csi
{
   namespace Xml
   {
      namespace EventParserHelpers
      {
         /**
          * Defines an object used for parsing an XML element.
          */
         class ParserContext
         {
         public:
            /**
             * Defines the possible states for the context.
             */
            enum state_type
            {
               state_initial,
               state_utf8_bom_ef,
               state_utf8_bom_bb,
               state_start_tag,
               state_namespaces_begin,
               state_namespaces_elem_name,
               state_namespaces_between_attr,
               state_namespaces_x,
               state_namespaces_m,
               state_namespaces_l,
               state_namespaces_n,
               state_namespaces_s,
               state_namespaces_name,
               state_namespaces_equal,
               state_namespaces_before_quote,
               state_namespaces_single_quote,
               state_namespaces_double_quote,
               state_namespaces_normal_name,
               state_namespaces_normal_equal,
               state_namespaces_normal_single_quote,
               state_namespaces_normal_double_quote,
               state_namespaces_end,
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
               state_cdata_parse,
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

            /**
             * Constructor
             *
             * @param state_ Specifies the initial state for this context.
             */
            ParserContext(state_type state_ = state_read_elem_name):
               state(state_)
            { }
            
         public:
            /**
             * Specifies the name of this element.  This will be set after the parser has reached
             * the end of the element name.  The namespace name (if any) will have been stripped off
             * by the time that this name is reported to the application.
             */
            StrUni element_name;

            /**
             * Specifies the URI for the namespace for the current element name.
             */
            StrUni element_namespace;

            /**
             * Specifies the state of this context.
             */
            state_type state;

            /**
             * Specifies the byte offset where the declaration for this element begins.
             */
            int8 begins_at;

            /**
             * Specifies the name of the namespace currently being parsed.
             */
            StrAsc namespace_name;

            /**
             * Specifies the namespaces that have been encountered while parsing this element.
             */
            typedef std::map<StrUni, StrUni> namespaces_type;
            namespaces_type namespaces;
         };
      };

      
      EventParser::EventParser():
         current_row(0),
         current_col(0),
         next_offset(0)
      { reset(); }


      EventParser::~EventParser()
      { }


      EventParser::parse_outcome_type EventParser::parse(std::istream &input)
      {
         int rtn(0);
         int ch(0);
         elem_name.cut(0);
         elem_namespace.cut(0);
         value.cut(0);
         attr_name.cut(0);
         attr_namespace.cut(0);
         if(contexts.empty())
            contexts.push_back(new context_type(context_type::state_initial));
         while(rtn == 0 && input && ch != EOF)
         {
            context_handle context(contexts.back());
            ch = input.get();
            if(context->state < context_type::state_namespaces_begin || context->state > context_type::state_namespaces_end)
            {
               if(ch == '\n')
               {
                  ++current_row;
                  current_col = 0;
               }
               else
                  ++current_col;
            }
            switch(context->state)
            {
            case context_type::state_initial:
               if(ch == '<')
                  context->state = context_type::state_start_tag;
               else if(!std::isspace(ch))
                  throw ParsingError("invalid state tag", current_row, current_col);
               break;

               // "<xml-element ..." or "<?..." or "</xml-element>"
               //   ^                     ^          ^
            case context_type::state_start_tag:
               if(ch == '?')
                  context->state = context_type::state_ignored;
               else if(ch == '!')
                  context->state = context_type::state_cdata_start_exclaim;
               else if(ch == '/')
               {
                  // we added a context when we came across the tag begin.  Since slash indicates
                  // that the element is ending, we need to go back to the previous context.
                  if(contexts.size() > 1)
                     contexts.pop_back();
                  context = contexts.back();
                  context->state = context_type::state_read_end_name;
                  token.cut(0);
               }
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else if(!std::isspace(ch))
               {
                  context->state = context_type::state_namespaces_elem_name;
                  context->begins_at = static_cast<int8>(input.tellg()) - 1;
               }
               break;

               // <xml-element attrib="xxx">
               //  ^^^^^^^^^^^
            case context_type::state_namespaces_elem_name:
               if(ch == '>')
               {
                  // we have read the the entire tag scanning for namespaces and are now ready to
                  // parse it for real.
                  input.seekg(context->begins_at, std::ios_base::beg);
                  context->state = context_type::state_read_elem_name;
                  token.cut(0);
               }
               else if(std::isspace(ch))
                  context->state = context_type::state_namespaces_between_attr;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               break;

               // <xml-element attr1="xxx" attr2='yyy'>
               //             ^           ^
            case context_type::state_namespaces_between_attr:
               if(ch == 'x')
                  context->state = context_type::state_namespaces_x;
               else if(ch == '/' || ch == '>')
               {
                  // we have now extracted all of the namespaces for this element.  We will now seek
                  // back to the place where the element started and parse the element name and
                  // attributes.
                  input.seekg(context->begins_at, std::ios_base::beg);
                  context->state = context_type::state_read_elem_name;
                  token.cut(0);
               }
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else if(!std::isspace(ch))
                  context->state = context_type::state_namespaces_normal_name;
               break;

               // <xml-element xmlns:ns="http://xxx.yyy.ccc.ddd/blah/blah/">
               //              ^
            case context_type::state_namespaces_x:
               if(ch == 'm')
                  context->state = context_type::state_namespaces_m;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else
               {
                  token = "x";
                  token.append((char)ch);
                  context->state = context_type::state_namespaces_normal_name;
               }
               break;

               // <xml-element xmlns:ns="http://xxx.yyy.ccc.ddd/blah/blah/">
               //               ^
            case context_type::state_namespaces_m:
               if(ch == 'l')
                  context->state = context_type::state_namespaces_l;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else
               {
                  token = "xm";
                  token.append((char)ch);
                  context->state = context_type::state_namespaces_normal_name;
               }
               break;

               // <xml-element xmlns:ns="http://xxx.yyy.ccc.ddd/blah/blah/">
               //                ^
            case context_type::state_namespaces_l:
               if(ch == 'n')
                  context->state = context_type::state_namespaces_n;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else
               {
                  token = "xml";
                  token.append((char)ch);
                  context->state = context_type::state_namespaces_normal_name;
               }
               break;

               // <xml-element xmlns:ns="http://xxx.yyy.ccc.ddd/blah/blah/">
               //                 ^
            case context_type::state_namespaces_n:
               if(ch == 's')
                  context->state = context_type::state_namespaces_s;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else
               {
                  token = "xmln";
                  token.append((char)ch);
                  context->state = context_type::state_namespaces_normal_name;
               }
               break;

               // <xml-element xmlns:ns="http://xxx.yyy.ccc.ddd/blah/blah/">
               //                  ^
            case context_type::state_namespaces_s:
               if(ch == ':' || std::isspace(ch))
               {
                  context->namespace_name.cut(0);
                  context->state = context_type::state_namespaces_name;
               }
               else if(ch == '=')
                  context->state = context_type::state_namespaces_equal;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else
               {
                  token = "xmlns";
                  token.append((char)ch);
                  context->state = context_type::state_namespaces_normal_name;
               }
               break;

               // <ns:xml-element xmlns:ns="http://xxx.yyy.ccc.ddd/blah/blah/">
               //                       ^^
            case context_type::state_namespaces_name:
               if(ch == '=')
                  context->state = context_type::state_namespaces_equal;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else if(!std::isspace(ch))
                  context->namespace_name.append((char)ch);
               break;

               // <ns:xml-element xmlns:ns="http://xxx.yyy.ccc.ddd/blah/blah/">
               //                         ^
            case context_type::state_namespaces_equal:
               if(ch == '\'')
                  context->state = context_type::state_namespaces_single_quote;
               else if(ch == '\"')
                  context->state = context_type::state_namespaces_double_quote;
               else if(std::isspace(ch))
                  context->state = context_type::state_namespaces_before_quote;
               else
                  throw ParsingError("improperly quoted namespace", current_row, current_col);
               break;

               // <ns:xml-element xmlns:ns='http://xxx.yyy.ccc.ddd/blah/blah/'>
               //                           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
               // <ns:xml-element xmlns:ns="http://xxx.yyy.ccc.ddd/blah/blah/">
               //                           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
            case context_type::state_namespaces_double_quote:
            case context_type::state_namespaces_single_quote:
               if((ch == '\'' && context->state == context_type::state_namespaces_single_quote) ||
                  (ch == '\"' && context->state == context_type::state_namespaces_double_quote))
               {
                  StrUni namespace_name(context->namespace_name);
                  StrUni namespace_value(token);
                  context->state = context_type::state_namespaces_between_attr;
                  context->namespaces[namespace_name] = namespace_value;
               }
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else if(ch == '\"' || ch == '\'')
                  throw ParsingError("improperly quoted namespace", current_row, current_col);
               else
                  token.append((char)ch);
               break;

               // <xml-element attr1="xxx" attr2 = "yyy">
               //              ^^^^^       ^^^^^
            case context_type::state_namespaces_normal_name:
               if(ch == '=')
                  context->state = context_type::state_namespaces_normal_equal;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               break;

               // <xml-element attr1="xxx" attr2 = 'yyy'>
               //                    ^            ^
            case context_type::state_namespaces_normal_equal:
               if(ch == '\'')
                  context->state = context_type::state_namespaces_normal_single_quote;
               else if(ch == '\"')
                  context->state = context_type::state_namespaces_normal_double_quote;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else if(!isspace(ch))
                  throw ParsingError("improperly quoted attribute", current_row, current_col);
               break;

               // <xml-element attr2 = 'yyy'>
               //                       ^^^
            case context_type::state_namespaces_normal_single_quote:
               if(ch == '\'')
                  context->state = context_type::state_namespaces_between_attr;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               break;

            case context_type::state_namespaces_normal_double_quote:
               if(ch == '\"')
                  context->state = context_type::state_namespaces_between_attr;
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               break;

            case context_type::state_read_elem_name:
               if(ch == ':')
               {
                  context->namespace_name = token;
                  token.cut(0);
               }
               else if(ch == '/')
               {
                  context->element_namespace = lookup_namespace_uri(context->namespace_name);
                  context->element_name = token;
                  elem_namespace = context->element_namespace;
                  elem_name = context->element_name;
                  rtn = parse_start_of_element;
                  context->state = context_type::state_empty;
               }
               else if(ch == '>')
               {
                  context->element_namespace = lookup_namespace_uri(context->namespace_name);
                  context->element_name = token;
                  token.cut(0);
                  value.cut(0);
                  elem_namespace = context->element_namespace;
                  elem_name = context->element_name;
                  rtn = parse_start_of_element;
                  context->state = context_type::state_read_content;
               }
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else if(!std::isspace(ch))
                  token.append((char)ch);
               else
               {
                  // we have now parsed the element name for this context.  We will need to look up
                  // the namespace URI for the element name.
                  context->element_namespace = lookup_namespace_uri(context->namespace_name);
                  context->element_name = token;
                  context->state = context_type::state_between_attr;
                  elem_namespace = context->element_namespace;
                  elem_name = context->element_name;
                  rtn = parse_start_of_element;
               }
               break;

               // <ns:xml-element ns1:attr="xxx"  ns2:attr="yyy">
               //                ^              ^^
            case context_type::state_between_attr:
               if(ch == '/')
                  context->state = context_type::state_empty;
               else if(ch == '>')
               {
                  token.cut(0);
                  context->state = context_type::state_read_content;
               }
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else if(!std::isspace(ch))
               {
                  context->state = context_type::state_read_attr_name;
                  context->namespace_name.cut(0);
                  token.cut(0);
                  token.append((char)ch);
               }
               break;

               // <ns:xml-element ns1:attr="xxx"  ns2:attr="yyy">
               //                 ^^^^^^^^        ^^^^^^^^
            case context_type::state_read_attr_name:
               if(ch == ':')
               {
                  context->namespace_name = token;
                  token.cut(0);
               }
               else if(ch == '=')
               {
                  if(context->namespace_name != "xmlns" && token != "xmlns")
                  {
                     attr_namespace = lookup_namespace_uri(context->namespace_name);
                     attr_name = token;
                  }
                  else
                  {
                     attr_name.cut(0);
                     attr_namespace.cut(0);
                  }
                  context->state = context_type::state_read_attr_equal;
               }
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else if(!std::isspace(ch))
                  token.append((char)ch);
               break;

               // <ns:xml-element ns1:attr="xxx"  ns2:attr = 'yyy'>
               //                         ^               ^^
            case context_type::state_read_attr_equal:
               token.cut(0);
               if(ch == '\'')
                  context->state = context_type::state_read_attr_single_quote;
               else if(ch == '\"')
                  context->state = context_type::state_read_attr_double_quote;
               else if(std::isspace(ch))
                  context->state = context_type::state_read_attr_quote;
               else
                  throw ParsingError("improperly marked attribute", current_row, current_col);
               break;

               // <ns:xml-element ns1:attr="xxx"  ns2:attr = 'yyy'>
               //                                           ^
            case context_type::state_read_attr_quote:
               if(ch == '\"')
                  context->state = context_type::state_read_attr_double_quote;
               else if(ch == '\'')
                  context->state = context_type::state_read_attr_single_quote;
               else if(!std::isspace(ch))
                  throw ParsingError("improperly marked attribute", current_row, current_col);
               break;

            case context_type::state_read_attr_single_quote:
            case context_type::state_read_attr_double_quote:
               if((ch == '\'' && context->state == context_type::state_read_attr_single_quote) ||
                  (ch == '\"' && context->state == context_type::state_read_attr_double_quote))
               {
                  if(attr_name.length() > 0)
                  {
                     elem_namespace = context->element_namespace;
                     elem_name = context->element_name;
                     input_xml_data(value, token.c_str(), token.length());
                     rtn = parse_attribute_read;
                  }
                  token.cut(0);
                  context->namespace_name.cut(0);
                  context->state = context_type::state_between_attr;
               }
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               else
                  token.append((char)ch);
               break;

               // <ns:xml-element ns1:attr="xxx"  ns2:attr = 'yyy'  />
               //                                                 ^^^
            case context_type::state_read_after_end_name:
            case context_type::state_empty:
               if(ch == '>')
               {
                  // we are at the end of parsing this context.  This will be reported to the
                  // application as either a document complete event or as the completion of an
                  // element.
                  context->state = context_type::state_complete;
                  elem_namespace = context->element_namespace;
                  elem_name = context->element_name;
                  if(contexts.size() == 1)
                     rtn = parse_end_of_document;
                  else
                  {
                     contexts.pop_back();
                     rtn = parse_end_of_element;
                  }
               }
               else if(!std::isspace(ch))
                  throw ParsingError("improperly terminated element", current_row, current_col);
               break;

               // <xml-element attr="xxx">blah blah</xml-element>
               //                         ^^^^^^^^^
            case context_type::state_read_content:
               if(ch == '<')
               {
                  // this could be the state of a new element even if we are entering a cdata block
                  // or a comment.  Because of this, we will push a new context.
                  input_xml_data(value, token.c_str(), token.length());
                  token.cut(0);
                  context.bind(new context_type(context_type::state_start_tag));
                  contexts.push_back(context);
                  token.cut(0);
                  attr_namespace.cut(0);
                  attr_name.cut(0);
               }
               else if(ch != EOF)
               {
                  if(!std::isspace(ch) || token.length() > 0)
                     token.append((char)ch);
               }
               else
                  throw ParsingError("incomplete tag", current_row, current_col);
               break;

               // <xml-element attr="xxx">blah blah</xml-element>
               //                                    ^^^^^^^^^^^
            case context_type::state_read_end_name:
               if(ch == '>' || std::isspace(ch))
               {
                  if(StrUni(token) == context->element_name)
                  {
                     token.cut(0);
                     if(ch != '>')
                        context->state = context_type::state_read_after_end_name;
                     else
                     {
                        elem_namespace = context->element_namespace;
                        elem_name = context->element_name;
                        if(contexts.size() > 1)
                        {
                           contexts.pop_back();
                           rtn = parse_end_of_element;
                        }
                        else
                           rtn = parse_end_of_document;
                     }
                  }
                  else
                     throw ParsingError("wrong element termination name", current_row, current_col);
               }
               else if(ch != EOF)
               {
                  if(ch == ':')
                     token.cut(0);
                  else
                     token.append((char)ch);
               }
               else
                  throw ParsingError("improperly terminated element", current_row, current_col);
               break;
               
               // <xml-element attr="aaa"><![CDATA[Blah Blah]]></xml-element>
               //                          ^
            case context_type::state_cdata_start_exclaim:
               if(ch == '[')
                  context->state = context_type::state_cdata_start_brace1;
               else if(ch == '-')
                  context->state = context_type::state_comment_start1;
               else if(ch != EOF)
                  context->state = context_type::state_ignored;
               else
                  throw ParsingError("improper cdata encoding", current_row, current_col);
               break;
               
               // <!-- Blah Blah -->
               //   ^                        
            case context_type::state_comment_start1:
               if(ch == '-')
                  context->state = context_type::state_comment_start2;
               else if(ch == EOF)
                  throw ParsingError("improper comment encoding", current_row, current_col);
               else
                  context->state = context_type::state_ignored;
               break;
               
               // <!-- Blah Blah -->
               //    ^
            case context_type::state_comment_start2:
               if(ch == '-')
                  context->state = context_type::state_comment_end1;
               else if(ch == EOF)
                  throw ParsingError("improper comment encoding", current_row, current_col);
               break;
               
               // <!-- Blah Blah -->
               //                 ^ 
            case context_type::state_comment_end1:
               if(ch == '-')
                  context->state = context_type::state_ignored;
               else
                  context->state = context_type::state_comment_start2;
               break;
               
            case context_type::state_ignored:
               if(ch == '>')
               {
                  // this represents the end of the context so we will clear off the current context.
                  if(contexts.size() > 1)
                     contexts.pop_back();
                  else
                     context->state = context_type::state_initial;
               }
               else if(ch == EOF)
                  throw ParsingError("incomplete tag", current_row, current_col);
               break;
               
               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                ^
            case context_type::state_cdata_start_brace1:
               if(ch == 'C')
                  context->state = context_type::state_cdata_start_c;
               else if(ch == EOF)
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               else
                  context->state = context_type::state_ignored;
               break;

               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                  ^
            case context_type::state_cdata_start_c:
               if(ch == 'D')
                  context->state = context_type::state_cdata_start_d;
               else if(ch == EOF)
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               else
                  context->state = context_type::state_ignored;
               break;

               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                  ^
            case context_type::state_cdata_start_d:
               if(ch == 'A')
                  context->state = context_type::state_cdata_start_a1;
               else if(ch == EOF)
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               else
                  context->state = context_type::state_ignored;
               break;

               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                   ^
            case context_type::state_cdata_start_a1:
               if(ch == 'T')
                  context->state = context_type::state_cdata_start_t;
               else if(ch == EOF)
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               else
                  context->state = context_type::state_ignored;
               break;

               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                    ^
            case context_type::state_cdata_start_t:
               if(ch == 'A')
                  context->state = context_type::state_cdata_start_a2;
               else if(ch == EOF)
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               else
                  context->state = context_type::state_ignored;
               break;

               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                     ^
            case context_type::state_cdata_start_a2:
               if(ch == '[')
               {
                  token.cut(0);
                  context->state = context_type::state_cdata_parse;
               }
               else if(ch == EOF)
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               else
                  context->state = context_type::state_ignored;
               break;

               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                       ^^^^^^^^^^^
            case context_type::state_cdata_parse:
               if(ch == ']')
                  context->state = context_type::state_cdata_end_brace1;
               else if(ch != EOF)
                  token.append((char)ch);
               else
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               break;

               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                                  ^
            case context_type::state_cdata_end_brace1:
               if(ch == ']')
                  context->state = context_type::state_cdata_end_brace2;
               else if(ch != EOF)
               {
                  context->state = context_type::state_cdata_parse;
                  token.append(']');
                  token.append((char)ch);
               }
               else
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               break;

               // <xml-element><![CDATA[ Blah Blah ]]></xml-element>
               //                                   ^
            case context_type::state_cdata_end_brace2:
               if(ch == '>')
               {
                  if(contexts.size() > 1)
                  {
                     contexts.pop_back();
                     context = contexts.back();
                  }
                  context->state = context_type::state_read_content;
               }
               else if(ch != EOF)
               {
                  context->state = context_type::state_cdata_parse;
                  token.append("]]");
                  token.append((char)ch);
               }
               else
                  throw ParsingError("improperly encoded CDATA", current_row, current_col);
               break;
            }
         }
         if(rtn == 0)
            throw ParsingError("syntax error", current_row, current_col);
         return static_cast<parse_outcome_type>(rtn);
      } // parse


      void EventParser::reset()
      {
         elem_name.cut(0);
         elem_namespace.cut(0);
         attr_name.cut(0);
         attr_namespace.cut(0);
         value.cut(0);
         current_row = current_col = 0;
         next_offset = 0;
         contexts.clear();
      } // reset


      bool EventParser::get_value_bool() const
      {
         bool rtn(false);
         if(value == L"true")
            rtn = true;
         else if(value == L"false")
            rtn = false;
         else
            rtn = get_value_int4() != 0;
         return rtn;
      } // get_value_bool


      int2 EventParser::get_value_int2() const
      {
         int2 rtn(0);
         Csi::IBuffStreamw temp(value.c_str(), value.length());
         temp.imbue(StringLoader::make_locale(0));
         temp >> rtn;
         if(!temp)
            throw std::invalid_argument("value conversion error");
         return rtn;
      } // get_value_int2


      uint2 EventParser::get_value_uint2() const
      {
         uint2 rtn(0);
         Csi::IBuffStreamw temp(value.c_str(), value.length());
         temp.imbue(StringLoader::make_locale(0));
         temp >> rtn;
         if(!temp)
            throw std::invalid_argument("value conversion error");
         return rtn;
      } // get_value_uint2


      int4 EventParser::get_value_int4() const
      {
         int4 rtn(0);
         Csi::IBuffStreamw temp(value.c_str(), value.length());
         temp.imbue(StringLoader::make_locale(0));
         temp >> rtn;
         if(!temp)
            throw std::invalid_argument("value conversion error");
         return rtn;
      } // get_value_int4


      uint4 EventParser::get_value_uint4() const
      {
         uint4 rtn(0);
         Csi::IBuffStreamw temp(value.c_str(), value.length());
         temp.imbue(StringLoader::make_locale(0));
         temp >> rtn;
         if(!temp)
            throw std::invalid_argument("value conversion error");
         return rtn;
      } // get_value_uint4


      int8 EventParser::get_value_int8() const
      {
         int8 rtn(0);
         Csi::IBuffStreamw temp(value.c_str(), value.length());
         temp.imbue(StringLoader::make_locale(0));
         temp >> rtn;
         if(!temp)
            throw std::invalid_argument("value conversion error");
         return rtn;
      } // get_value_int8


      double EventParser::get_value_double() const
      {
         return csiStringToFloat(value.c_str(), StringLoader::make_locale(0), true); 
      } // get_value_double


      LgrDate EventParser::get_value_stamp() const
      {
         return LgrDate::fromStr(value.to_utf8().c_str());
      } // get_value_stamp
      

      StrUni EventParser::lookup_namespace_uri(StrUni const &name)
      {
         // we will scan backward from the last context to the first and attempt to match the
         // namespace that matches the specified name.
         StrUni rtn;
         for(contexts_type::reverse_iterator ci = contexts.rbegin();
             ci != contexts.rend();
             ++ci)
         {
            context_handle &context(*ci);
            context_type::namespaces_type::iterator cni(context->namespaces.find(name));
            if(cni != context->namespaces.end())
            {
               rtn = cni->second;
               break;
            }
         }
         return rtn;
      } // lookup_namespace_uri
   };
};

