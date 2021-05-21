/* Csi.Html.Link.h

   Copyright (C) 2007, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 23 February 2007
   Last Change: Tuesday 20 November 2012
   Last Commit: $Date: 2012-11-20 12:24:32 -0600 (Tue, 20 Nov 2012) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Html_Link_h
#define Csi_Html_Link_h

#include "Csi.Html.Tag.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Link
      ////////////////////////////////////////////////////////////
      class Link: public Tag
      {
      private:
         ////////////////////////////////////////////////////////////
         // href
         ////////////////////////////////////////////////////////////
         StrAsc href;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Link(StrAsc const &href_ = "", StrAsc const &contents = ""):
            href(href_)
         {
            if(contents.length() > 0)
               add_tag(new Text(contents));
         } 


         ////////////////////////////////////////////////////////////
         // before_render
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out)
         {
            out << "<a href=\"" << href << "\">";
            return true;
         }
         virtual bool before_render(std::wostream &out)
         {
            out << L"<a href=\"" << href << L"\">";
            return true;
         }

         ////////////////////////////////////////////////////////////
         // after_render
         ////////////////////////////////////////////////////////////
         virtual void after_render(std::ostream &out)
         { out << "</a>\n"; }
         virtual void after_render(std::wostream &out)
         { out << L"</a>\n"; }

         ////////////////////////////////////////////////////////////
         // get_href
         ////////////////////////////////////////////////////////////
         StrAsc const &get_href() const
         { return href; }

         ////////////////////////////////////////////////////////////
         // set_href
         ////////////////////////////////////////////////////////////
         void set_href(StrAsc const &href_)
         { href = href_; }
      };
   };
};


#endif
