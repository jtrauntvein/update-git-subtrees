/* Csi.Html.Document.cpp

   Copyright (C) 2000, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 3 May 2000
   Last Change: Friday 27 August 2010
   Last Commit: $Date: 2010-08-27 08:19:29 -0600 (Fri, 27 Aug 2010) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Html.Document.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Document definitions
      ////////////////////////////////////////////////////////////
      Document::Document(char const *title):
         Tag("html",false)
      {
         head_tag = Tag::add_tag(new Tag("head", false));
         value_type title_tag_holder(head_tag->add_tag(new Tag("title", true)));
         title_tag = title_tag_holder->add_tag(new Text(title));
         body_tag = Tag::add_tag(new Tag("body", false));
      } // constructor


      Document::~Document()
      { }
   

      void Document::set_title(char const *title)
      { title_tag->set_content(title); }
      

      Document::tag_handle Document::add_tag(tag_handle tag)
      { 
         body_tag->add_tag(tag);
         return tag;
      } // add_tag


      void Document::set_encoding(StrAsc const &encoding_name)
      {
         if(encoding_tag == 0)
         {
            OStrAscStream content;
            encoding_tag = head_tag->add_tag("meta");
            encoding_tag->add_attribute("http-equiv","Content-Type");
            content << "text/html; charset=" << encoding_name;
            encoding_tag->add_attribute("content",content.str());
         }
      } // set_encoding
   };
};
