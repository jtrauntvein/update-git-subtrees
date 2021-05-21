/* Csi.Graphics.ShapeBase.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 11 February 2015
   Last Change: Monday 28 September 2015
   Last Commit: $Date: 2018-09-15 01:06:23 -0600 (Sat, 15 Sep 2018) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_ShapeBase_h
#define Csi_Graphics_ShapeBase_h
#include "Csi.Graphics.Driver.h"


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines a base class for objects that represents shapes that can be drawn to a graphics
       * driver.
       */
      class ShapeBase
      {
      public:
         /**
          * Destructor
          */
         virtual ~ShapeBase()
         { }

         /**
          * @return Returns a rectangle that specifies the size of this shape
          * when drawn at the specified scale factor.
          *
          * @param scale Specifies the scale for the shape.  A value of one
          * indicates that the shape is drawn at full scale while a value
          * greater than one will magnify the shape and a value less than one
          * will reduce the size of the shape.
          */
         virtual Rect get_size(double scale = 1) = 0;

         /**
          * Draws this shape to the specified driver and at the specified
          * scale.
          *
          * @param driver Specifies the driver to which the shape will be
          * drawn.
          *
          * @param centre Specifies the centre point of the symbol.
          *
          * @param colour Specifies the colour of the shape.
          *
          * @param scale Specifies the scale for the shape.  A value of one
          * indicates that the shape is drawn at full scale while a value
          * greater than one will magnify the shape and a value less than one
          * will reduce the size of the shape.
          */
         virtual void draw(
            Driver &driver,
            Point const &centre,
            Colour const &colour,
            double scale = 1)
         {
            Driver::pen_handle   pen(driver.make_pen(PenInfo().set_colour(colour)));
            Driver::brush_handle brush(driver.make_brush(BrushInfo().set_colour(colour)));
            draw(driver, centre, brush, pen, scale);
         }

         virtual void draw(
            Driver &driver,
            Point const &centre,
            Driver::brush_handle const &brush,
            Driver::pen_handle   const &pen,
            double scale = 1) = 0;

         /**
          * Enumerate the type of shapes that are supported.
          */
         enum symbol_type
         {
            symbol_rectangle = 0,
            symbol_circle = 1,
            symbol_up_triangle = 2,
            symbol_down_triangle = 3,
            symbol_cross = 4,
            symbol_diagonal_cross = 5,
            symbol_star = 6,
            symbol_diamond = 7,
            symbol_small_dot = 8,
            symbol_nothing = 9,
            symbol_left_triangle = 10,
            symbol_right_triangle = 11,
            symbols_count
         };

         /**
          * @return Must be overloaded to return the symbol type code.
          */
         virtual symbol_type get_symbol_type() const = 0;

         /**
          * Responsible for generating a shape object based upon the specified
          * symbol type.
          *
          * @return Returns an allocated shape object or null if the symbol
          * type is not supported.
          *
          * @param symbol Specifies the symbol type to create.
          */
         static ShapeBase *make_shape(symbol_type symbol);
      };
   };
};


#endif
