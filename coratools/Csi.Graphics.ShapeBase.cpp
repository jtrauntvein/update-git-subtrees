/* Csi.Graphics.ShapeBase.cpp

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 11 February 2015
   Last Change: Tuesday 02 June 2015
   Last Commit: $Date: 2018-10-24 20:24:14 -0600 (Wed, 24 Oct 2018) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Graphics.ShapeBase.h"


namespace Csi
{
   namespace Graphics
   {
      namespace
      {
         class ShapeRectangle: public ShapeBase
         {
         public:
            virtual Rect get_size(double scale)
            { return Rect(0, 0, scale * 2, scale * 2); }

            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               rect.set_centre(centre);
               driver.draw_rect(rect, 0, brush);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_rectangle; }
         };

         
         class ShapeCircle: public ShapeBase
         {
         public:
            virtual Rect get_size(double scale)
            { return Rect(0, 0, scale * 2, scale * 2); }

            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               driver.draw_ellipse(centre, scale, scale, 0, brush);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_circle; }
         };


         class ShapeDiamond: public ShapeRectangle
         {
         public:
            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               Driver::points_type points;
               rect.set_centre(centre);
               points.push_back(Point(rect.get_left(), rect.get_centre_y()));
               points.push_back(Point(rect.get_centre_x(), rect.get_top()));
               points.push_back(Point(rect.get_right(), rect.get_centre_y()));
               points.push_back(Point(rect.get_centre_x(), rect.get_bottom()));
               driver.draw_polygon(points, 0, brush);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_diamond; }
         };

         
         class ShapeUpTriangle: public ShapeRectangle
         {
         public:
            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               Driver::points_type points;
               rect.set_centre(centre);
               points.push_back(rect.get_bottom_left());
               points.push_back(Point(rect.get_centre_x(), rect.get_top()));
               points.push_back(rect.get_bottom_right());
               driver.draw_polygon(points, 0, brush);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_up_triangle; }
         };

         
         class ShapeDownTriangle: public ShapeRectangle
         {
         public:
            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               Driver::points_type points;
               rect.set_centre(centre);
               points.push_back(rect.get_top_left());
               points.push_back(Point(rect.get_centre_x(), rect.get_bottom()));
               points.push_back(rect.get_top_right());
               driver.draw_polygon(points, 0, brush);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_down_triangle; }
         };

         
         class ShapeCross: public ShapeBase
         {
         public:
            virtual Rect get_size(double scale)
            { return Rect(0, 0, 2 * scale, 2 * scale); }

            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               rect.set_centre(centre);
               driver.draw_line(
                  Point(rect.get_left(), rect.get_centre_y()),
                  Point(rect.get_right(), rect.get_centre_y()),
                  pen);
               driver.draw_line(
                  Point(rect.get_centre_x(), rect.get_top()),
                  Point(rect.get_centre_x(), rect.get_bottom()),
                  pen);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_cross; }
         };


         class ShapeDiagonalCross: public ShapeBase
         {
         public:
            virtual Rect get_size(double scale)
            { return Rect(0, 0, 2 * scale, 2 * scale); }

            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               rect.set_centre(centre);
               driver.draw_line(rect.get_top_left(), rect.get_bottom_right(), pen);
               driver.draw_line(rect.get_top_right(), rect.get_bottom_left(), pen);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_diagonal_cross; }
         };

         
         class ShapeStar: public ShapeBase
         {
         public:
            virtual Rect get_size(double scale)
            {
               return Rect(0, 0, 2 * scale, 2 * scale);
            }
            
            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               rect.set_centre(centre);
               driver.draw_line(
                  Point(rect.get_centre_x(), rect.get_top()),
                  Point(rect.get_centre_x(), rect.get_bottom()),
                  pen);
               driver.draw_line(
                  Point(rect.get_left(), rect.get_centre_y()),
                  Point(rect.get_right(), rect.get_centre_y()),
                  pen);
               driver.draw_line(rect.get_top_left(), rect.get_bottom_right(), pen);
               driver.draw_line(rect.get_bottom_left(), rect.get_top_right(), pen);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_star; }
         };


         class ShapeSmallDot: public ShapeBase
         {
         public:
            virtual Rect get_size(double scale)
            { return Rect(0, 0, 1, 1); }

            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               driver.draw_ellipse(centre, scale, scale, 0, brush);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_small_dot; }
         };


         class ShapeRightTriangle: public ShapeRectangle
         {
         public:
            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               Driver::points_type points;
               rect.set_centre(centre);
               points.push_back(rect.get_top_left());
               points.push_back(rect.get_bottom_left());
               points.push_back(Point(rect.get_right(), rect.get_centre_y()));
               driver.draw_polygon(points, 0, brush);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_right_triangle; }
         };

         
         class ShapeLeftTriangle: public ShapeRectangle
         {
         public:
            virtual void draw(
               Driver &driver,
               Point const &centre,
               Driver::brush_handle const &brush,
               Driver::pen_handle const &pen,
               double scale)
            {
               Rect rect(get_size(scale));
               Driver::points_type points;
               rect.set_centre(centre);
               points.push_back(rect.get_top_right());
               points.push_back(rect.get_bottom_right());
               points.push_back(Point(rect.get_left(), rect.get_centre_y()));
               driver.draw_polygon(points, 0, brush);
            }

            virtual symbol_type get_symbol_type() const
            { return symbol_left_triangle; }
         };
      };


      ShapeBase *ShapeBase::make_shape(symbol_type symbol)
      {
         ShapeBase *rtn(0);
         switch(symbol)
         {
         case symbol_rectangle:
            rtn = new ShapeRectangle;
            break;

         case symbol_circle:
            rtn = new ShapeCircle;
            break;
            
         case symbol_diamond:
            rtn = new ShapeDiamond;
            break;
            
         case symbol_up_triangle:
            rtn = new ShapeUpTriangle;
            break;
            
         case symbol_down_triangle:
            rtn = new ShapeDownTriangle;
            break;
            
         case symbol_cross:
            rtn = new ShapeCross;
            break;
            
         case symbol_diagonal_cross:
            rtn = new ShapeDiagonalCross;
            break;
            
         case symbol_star:
            rtn = new ShapeStar;
            break;

         case symbol_small_dot:
            rtn = new ShapeSmallDot;
            break;

         case symbol_right_triangle:
            rtn = new ShapeRightTriangle;
            break;

         case symbol_left_triangle:
            rtn = new ShapeLeftTriangle;
            break;
         }
         return rtn;
      } // make_shape
   };
};

