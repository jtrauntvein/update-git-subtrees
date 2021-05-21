/* Csi.NestedRect.cpp

   Copyright (C) 2015, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 13 January 2015
   Last Change: Tuesday 13 January 2015
   Last Commit: $Date: 2015-01-13 16:03:09 -0600 (Tue, 13 Jan 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Graphics.NestedRect.h"


namespace Csi
{
   namespace Graphics
   {
      void NestedRect::stack_grid(double fixed, bool is_vertical)
      {
         // we need to get the maximum size of all of the children
         double max_width(0);
         double max_height(0);
         for(children_type::iterator ci = children.begin();
             ci != children.end();
             ++ci)
         {
            child_handle &child(*ci);
            if(child->get_width() > max_width)
               max_width = child->get_width();
            if(child->get_height() > max_height)
               max_height = child->get_height();
         }
         
         // we can now determine how many children can fit in the fixed dimension.
         int fixed_count(0);
         if(is_vertical)
            fixed_count = static_cast<int>(fixed / max_height);
         else
            fixed_count = static_cast<int>(fixed / max_width);
         if(fixed_count < 1)
            fixed_count = 1;
         
         // we can now iterate through the children and assign their positions in the grid.
         for(size_t i = 0; i < children.size(); ++i)
         {
            child_handle &child(children[i]);
            if(is_vertical)
               child->move((i / fixed_count) * max_width, (i % fixed_count) * max_height);
            else
               child->move((i % fixed_count) * max_width, (i / fixed_count) * max_height);
         }
      } // stack_grid
   };
};


