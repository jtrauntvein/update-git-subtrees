/* Csi.Graphics.Brush.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 06 May 2015
   Last Change: Wednesday 14 October 2015
   Last Commit: $Date: 2018-06-18 12:47:54 -0600 (Mon, 18 Jun 2018) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_Brush_h
#define Csi_Graphics_Brush_h

#include "Csi.Graphics.Gradient.h"
#include "Csi.LightSharedPtr.h"
#include "Csi.Xml.Element.h"


namespace Csi
{
   namespace Graphics
   {
      class BrushInfo
      {
      public:
         /**
          * Defines an object that represents the information needed to construct a brush.
          */
         BrushInfo() :
            visible(true)
         {  }

         /**
          * Construct with a solid background colour.
          *
          * @param colour Specifies the fill colour.
          */
         BrushInfo(Colour const &colour):
            fill(colour),
            visible(true)
         { }

         /**
          * Construct with a gradient.
          *
          * @param gradient Specifies the gradient to use.
          */
         BrushInfo(Gradient const &gradient):
            fill(gradient),
            visible(true)
         { }
         
         /**
          * @return Returns the gradient used to fill regions painted with this brush.
          */
         Gradient const &get_fill() const
         { return fill; }
         
         /**
          * @param value Specifies the gradient used to fill regions painted with
          * this brush.  If the gradient is empty, the region will be
          * transparent.  If the gradient has only one colour, the region will be
          * solid.
          */
         BrushInfo &set_fill(Gradient const &value)
         {
            fill = value;
            return *this;
         }

         /**
          * @param colour Specifies that the fill should be a solid colour.
          */
         BrushInfo &set_colour(Colour const &colour)
         {
            fill.clear();
            fill.push_back(colour);
            return *this;
         }

         /**
          * @return Returns tru if this brush is visible.
          */
         bool get_visible() const
         { return visible; }

         /**
          * @param value Set to true if the brush should be visible.  
          */
         BrushInfo &set_visible(bool value)
         {
            visible = value;
            return *this;
         }

         /**
          * Removes all colours from the gradient.
          */
         void clear()
         { fill.clear(); }

         /**
          * Adds the specified colour stop to the fill gradient.
          *
          * @param colour Specifies the coliyr to add.
          *
          * @param position Specifies the stop position as a ratio between 0 and 1.
          */
         void push_back(Colour const &colour, double position = std::numeric_limits<double>::quiet_NaN())
         {
            fill.push_back(colour, position);
         }

         /**
          * @return Returns the first iterator to the set of stops in the fill gradient.
          */
         typedef Gradient::iterator iterator;
         typedef Gradient::const_iterator const_iterator;
         iterator begin()
         { return fill.begin(); }
         const_iterator begin() const
         { return fill.begin(); }

         /**
          * @return Returns the iterator beyond the last fill stop.
          */
         iterator end()
         { return fill.end(); }
         const_iterator end() const
         { return fill.end(); }

         /**
          * @return Returns true if the fill gradient is empty.
          */
         bool empty() const
         { return fill.empty(); }

         /**
          * @return Returns the number of stops in the fill gradient.
          */
         typedef Gradient::size_type size_type;
         size_type size() const
         {
            return fill.size();
         }

         /**
          * @return Returns the fill gradient's direction code.
          */
         typedef Gradient::direction_type direction_type;
         direction_type get_direction() const
         { return fill.get_direction(); }

         /**
          * @param value Sets the fill direction code.
          */
         BrushInfo &set_direction(direction_type value)
         {
            fill.set_direction(value);
            return *this;
         }

         /**
          * @return Returns the fill gradient's radial offset.
          */
         Point const &get_radial_offset() const
         { return fill.get_radial_offset(); }

         /**
          * @param value Specifies the radial offset for the fill gradient.
         */
         BrushInfo &set_radial_offset(Point const &value)
         {
            fill.set_radial_offset(value);
            return *this;
         }
         
         /**
          * Writes the information about the brush to the specified XML structure.
          *
          * @param elem Specifies the destination XML structure.
          */
         void write(Xml::Element &elem) const
         {
            Xml::Element::value_type fill_xml(elem.add_element(L"fill"));
            fill.write(*fill_xml);
            elem.set_attr_bool(visible, L"visible");
         }

         /**
          * Reads the information about the brush from the specified XML structure.
          *
          * @param elem Specifies the source XML.
          */
         void read(Xml::Element &elem)
         {
            Xml::Element::value_type fill_xml(elem.find_elem(L"fill"));
            fill.read(*fill_xml);
            if(elem.has_attribute(L"visible"))
               visible = elem.get_attr_bool(L"visible");
            else
               visible = true;
         } // read
         
      protected:
         /**
          * Specifies the gradient used for filling on this brush.
          */
         Gradient fill;

         /**
          * Specifies whether anything drawn with this brush should be visible.
          */
         bool visible;
      };


      /**
       * Defines a class that represents a brush resource.
       */
      class Brush
      {
      public:
         /**
          * Constructor
          *
          * @param desc_ Specifies the brush description.
          */
         Brush(BrushInfo const &desc_):
            desc(desc_)
         { }

         /**
          * Destructor
          */
         virtual ~Brush()
         { }

         /**
          * @return Returns the description for this brush
          */
         BrushInfo const &get_desc() const
         { return desc; }

         /**
          * @return Returns true if this brush is visible.
          */
         bool get_visible() const
         { return desc.get_visible(); }

      protected:
         /**
          * Specifies the description that was used to create this brush.
          */
         BrushInfo desc;
      };


      typedef LightSharedPtr<Brush> brush_handle;
   };
};


#endif
