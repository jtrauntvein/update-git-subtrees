/* Csi.Graphics.Pen.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 23 January 2015
   Last Change: Wednesday 30 December 2015
   Last Commit: $Date: 2015-12-30 17:10:20 -0600 (Wed, 30 Dec 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Graphics_Pen_h
#define Csi_Graphics_Pen_h

#include "Csi.Graphics.Colour.h"
#include "Csi.Xml.Element.h"


namespace Csi
{
   namespace Graphics
   {
      /**
       * Specifies the line types that are supported.
       */
      enum LineType
      {
         line_solid = 0,
         line_dash = 1,
         line_dot = 2,
         line_dash_dot = 3,
         line_dash_dot_dot = 4,
         line_clear = 5,
         line_ignore = 6,
         line_small_dots = 7,
         line_styles_count
      };

      
      /**
       * Defines an object that describes the parameters for a pen object.
       */
      class PenInfo
      {
      public:
         enum cap_style_type
         {
            cap_round,
            cap_projecting,
            cap_butt
         };

         enum join_style_type
         {
            join_bevel,
            join_round,
            join_mitre
         };

         /**
          * Default Constructor
          */
         PenInfo():
            visible(true),
            line_type(line_solid),
            width(1),
            cap_style(cap_butt),
            join_style(join_round),
            colour(black_colour())   
         { }

         /**
          * Copy constructor
          */
         PenInfo(PenInfo const &other):
            visible(other.visible),
            line_type(other.line_type),
            colour(other.colour),
            width(other.width)
         { }

         /**
          * Copy operator
          */
         PenInfo &operator =(PenInfo const &other)
         {
            visible = other.visible;
            line_type = other.line_type;
            colour = other.colour;
            width = other.width;
            return *this;
         }

         /**
          * @return Returns true if lines drawn with this pen will be visible.
          */
         bool get_visible() const
         { return visible && line_type != line_clear && line_type != line_ignore; }

         /**
          * Sets whether lines drawn with the pen will be visible.
          *
          * @param visible_ Set to true if lines drawn with the pen will be visible.
          */
         PenInfo &set_visible(bool visible_)
         {
            visible = visible_;
            return *this;
         }

         /**
          * @return Returns the line type code.
          */
         LineType get_line_type() const
         { return line_type; }
         
         /**
          * Sets the line type.
          *
          * @param line_type_ Specifies the line type.
          */
         PenInfo &set_line_type(LineType line_type_)
         {
            line_type = line_type_;
            return *this;
         }

         /**
          * @return Returns the colour of this pen.
          */
         Colour const &get_colour() const
         { return colour; }

         /**
          * Sets the pen colour.
          *
          * @param colour_ Specifies the colour for this pen.
          */
         PenInfo &set_colour(Colour const &colour_)
         {
            colour = colour_;
            return *this;
         }

         /**
          * @return Returns the line width.
          */
         double get_width() const
         { return width; }

         /**
          * Sets the line width
          *
          * @param width_ Specifies the new line width.
          */
         PenInfo &set_width(double width_)
         {
            width = width_;
            return *this;
         }

         /**
          * @return Returns the cap style for this pen.
          */
         cap_style_type get_cap_style() const
         { return cap_style; }

         /**
          * @param style Specifies the cap style.
          */
         PenInfo &set_cap_style(cap_style_type style)
         {
            cap_style = style;
            return *this;
         }

         /**
          * @return Returns the join style.
          */
         join_style_type get_join_style() const
         { return join_style; }

         /**
          * @param style Specifies the join style for this pen.
          */
         PenInfo &set_join_style(join_style_type style)
         {
            join_style = style;
            return *this;
         }

         /**
          * Writes the pen information to the specified XML object.
          *
          * @param elem Specifies the XML structure to which the pen information will be written.
          */
         void write(Xml::Element &elem) const
         {
            elem.set_attr_bool(visible, L"visible");
            elem.set_attr_uint4(line_type, L"line-type");
            elem.set_attr_colour(colour, L"colour");
            elem.set_attr_double(width, L"width");
            elem.set_attr_uint4(cap_style, L"cap-style");
            elem.set_attr_uint4(join_style, L"join-style");
         }

         /**
          * Reads the pen information from the specified XML object.
          *
          * @param elem Specifies the XML that should contain the information for this pen.
          */
         void read(Xml::Element &elem)
         {
            visible = elem.get_attr_bool(L"visible");
            line_type = static_cast<LineType>(elem.get_attr_uint4(L"line-type"));
            colour = elem.get_attr_colour(L"colour");
            width = elem.get_attr_double(L"width");
            cap_style = static_cast<cap_style_type>(elem.get_attr_uint4(L"cap-style"));
            join_style = static_cast<join_style_type>(elem.get_attr_uint4(L"join-style"));
         }
         
      private:
         /**
          * Specifies whether this pen is visible.
          */
         bool visible;

         /**
          * Specifies the type of line that will be drawn
          */
         LineType line_type;

         /**
          * Specifies the colour of the line.
          */
         Colour colour;

         /**
          * Specifies the line width.
          */
         double width;

         /**
          * Specifies the cap style
          */
         cap_style_type cap_style;

         /**
          * Specifies the join style.
          */
         join_style_type join_style;
      };


      /**
       * Defines an object that represents a created pen resource.  Specific
       * implementations will define subclasses that will manage the specific
       * resources.
       */
      class Pen
      {
      public:
         /**
          * Constructor
          *
          * @param desc_ Specifies the description for this pen.
          */
         Pen(PenInfo const &desc_):
            desc(desc_)
         { }
         
         /**
          * virtual destructor
          */
         virtual ~Pen()
         { }

         /**
          * @return Returns the description for this pen.
          */
         PenInfo const &get_desc() const
         { return desc; }

         /**
          * @return Returns true if this pen is visible.
          */
         bool get_visible() const
         { return desc.get_visible(); }

         /**
          * @return Returns the width of this pen.
          */
         double get_width() const
         { return desc.get_width(); }

         /**
          * @return Returns the colour for this pen.
          */
         Colour const &get_colour() const
         { return  desc.get_colour(); }

      protected:
         /**
          * Specifies the description for this pen.
          */
         PenInfo desc;
      };
   };
};


#endif
