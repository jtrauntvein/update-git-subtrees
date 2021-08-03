/* Csi.Html.Anchor.h

   Copyright (C) 2007, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 March 2007
   Last Change: Wednesday 21 November 2012
   Last Commit: $Date: 2012-11-26 09:32:28 -0600 (Mon, 26 Nov 2012) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Html_Anchor_h
#define Csi_Html_Anchor_h

#include "Csi.Html.Tag.h"


namespace Csi
{
   namespace Html
   {
      ////////////////////////////////////////////////////////////
      // class Anchor
      ////////////////////////////////////////////////////////////
      class Anchor: public Tag
      {
      private:
         ////////////////////////////////////////////////////////////
         // name
         ////////////////////////////////////////////////////////////
         StrAsc name;

      public:
         ////////////////////////////////////////////////////////////
         // Anchor
         ////////////////////////////////////////////////////////////
         Anchor(StrAsc const &name_ = ""):
            name(name_)
         { }

         ////////////////////////////////////////////////////////////
         // before_render
         ////////////////////////////////////////////////////////////
         virtual bool before_render(std::ostream &out)
         {
            out << "<a name=\"" << name << "\">";
            return true;
         }
         virtual bool before_render(std::wostream &out)
         {
            out << L"<a name=\"" << name << L"\">";
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
         // get_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_name() const
         { return name; }

         ////////////////////////////////////////////////////////////
         // set_name
         ////////////////////////////////////////////////////////////
         void set_name(StrAsc const &name_)
         { name = name_; }
      };
   };
};


#endif
