/* Csi.Graphics.Driver.h

   Copyright (C) 2015, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 16 January 2015
   Last Change: Wednesday 28 March 2018
   Last Commit: $Date: 2019-01-23 13:45:31 -0600 (Wed, 23 Jan 2019) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_Driver_h
#define Csi_Graphics_Driver_h

#include "Csi.Graphics.NestedRect.h"
#include "Csi.Graphics.Font.h"
#include "Csi.Graphics.Pen.h"
#include "Csi.Graphics.Brush.h"
#include "Csi.Graphics.Colour.h"
#include "Csi.Graphics.Gradient.h"


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines an object that provides drawing related services for graphics
       * objects.  This is done through virtual methods that must be overloaded
       * to perform the actual drawing.
       */
      class Driver
      {
      public:
         /**
          * Destructor
          */
         virtual ~Driver()
         { }

         /**
          * Must be overloaded to calculate the space required to draw the
          * specified text at the specified angle.
          *
          * @return Returns a rectangle that represents the area for the text.
          *
          * @param text Specifies the text to be evaluated.
          *
          * @param font Specifies the font that will be used to draw the text.
          *
          * @param angle Specifies the angle of the text in degrees.
          */
         typedef LightSharedPtr<Font> font_handle;
         virtual Rect measure_text(
            StrUni const &text, font_handle &font, double angle = 0) = 0;
         virtual Rect measure_text(
            StrAsc const &text, font_handle &font, double angle = 0)
         { return measure_text(StrUni(text), font, angle); }

         /**
          * @return Returns the default font description for the application.
          */
         virtual FontInfo default_font_desc()
         { return FontInfo(); }
         
         /**
          * Creates a font object using the informationin the provided description.
          *
          * @param desc Specifies a description of the font object.
          *
          * @return Returns a platform specific font handle.
          */
         virtual font_handle make_font(FontInfo const &desc) = 0;

         /**
          * Creates a pen object from the specified description.
          *
          * @param desc Specifies the description for the pen.
          *
          * @return Returns the pen object created from the description.
          */
         typedef LightSharedPtr<Pen> pen_handle;
         virtual pen_handle make_pen(PenInfo const &desc) = 0;

         /**
          * @return Returns a brush object based upon the specified
          * description.
          *
          * @param desc Specifies the description of the brush.
          */
         typedef LightSharedPtr<Brush> brush_handle;
         virtual brush_handle make_brush(BrushInfo const &desc) = 0;

         /**
          * Draws the specified text in the specified rectangle and using the specified font. 
          *
          * @param text Specifies the text to draw.
          *
          * @param rect 
          *
          * @param font Specifies the font used to draw the text.
          *
          * @param foreground Specifies the text foreground colour.
          *
          * @param background Specifies the text background colour.
          *
          * @param rotation Specifies the angle in degrees at which the text
          * should be rotated.  The text should always be rotated around the
          * centre point.  A rotation angle of zero should imply that the text
          * will be drawn parallel to the X axis. The implementation must apply
          * any rotation around the specified centre point. 
          */
         virtual void draw_text(
            StrUni const &text,
            Rect const &rect,
            font_handle &font,
            Colour const &foreground,
            Colour const &background,
            double rotation = 0) = 0;

         /**
          * Draws a line from the specified point to the other specified point
          * using the specified pen.
          *
          * @param p1 Specifies the origin point.
          *
          * @param p2 Specifies the destination point.
          *
          * @param pen Specifies the pen used for drawing.
          */
         virtual void draw_line(
            Point const &p1,
            Point const &p2,
            pen_handle pen) = 0;


         /**
          * Draws a collection of line segments between the points specified in the provided
          * collection.
          *
          * @param points Specifies the collection of points between which line
          * segments should be drawn.
          *
          * @param pen Specifies the pen used for drawing.
          */
         typedef std::deque<Point> points_type;
         virtual void draw_lines(
            points_type const &points,
            pen_handle pen)
         {
            Point last_point;
            for(points_type::const_iterator pi = points.begin();
                pi != points.end();
                ++pi)
            {
               if(pi != points.begin())
                  draw_line(last_point, *pi, pen);
               last_point = *pi;
            }
         }

         /**
          * Draws and fills a polygon with a specified solid colour.
          *
          * @param points Specifies the coordinates for polygon vertices.
          *
          * @param brush Specifies the brush that will be used to fill the
          * background.  if set to null, the background will be transparent.
          *
          * @param pen Specifies how the perimeter will be drawn.  Set to null
          * if no border is to be drawn. 
          */
         virtual void draw_polygon(
            points_type const &points,
            pen_handle pen = 0,
            brush_handle brush = 0) = 0;

         /**
          * Draws and fills a rectangular region with a specified solid colour.
          *
          * @param rect Specifies the coordinate of the rectangular region.
          *
          * @param background Specifies the brush used to fill the
          * background of the rectangle.  If null, the background will be
          * transparent.
          *
          * @param pen Specifies the pen with which the border will be drawn.  Set to null if no
          * border is to be drawn.
          */
         virtual void draw_rect(
            Rect const &rect,
            pen_handle pen,
            brush_handle brush = 0)
         {
            points_type points;
            points.push_back(rect.get_top_left());
            points.push_back(rect.get_top_right());
            points.push_back(rect.get_bottom_right());
            points.push_back(rect.get_bottom_left());
            draw_polygon(points, pen, brush);
         }

         /**
         * Draws and fills a rectangular region with a specified solid colour, using
         * rounded edges, if the graphic driver allows it.  Note that the default
         * simply draws a rectangle, but the some drivers might overwrite this.
         *
         * @param rect Specifies the coordinate of the rectangular region.
         *
         * @param background Specifies the brush used to fill the
         * background of the rectangle.  If null, the background will be
         * transparent.
         *
         * @param pen Specifies the pen with which the border will be drawn.  Set to null if no
         * border is to be drawn.
         */
         virtual void draw_rounded_rect(
            Rect const &rect,
            pen_handle pen,
            brush_handle brush = 0)
         {
            points_type points;
            points.push_back(rect.get_top_left());
            points.push_back(rect.get_top_right());
            points.push_back(rect.get_bottom_right());
            points.push_back(rect.get_bottom_left());
            draw_polygon(points, pen, brush);
         }

         /**
          * Draws and fills an elliptical area with a solid colour.
          *
          * @param centre Specifies the centre of the ellipse.
          *
          * @param width Specifies the width of the elipse.
          *
          * @param height Specifies the height of the elipse.  Set this value equal to the width to
          * draw a circle.
          *
          * @param pen Specifies how the ellipse boundary will be drawn. Set to null if no border is
          * to be drawn.
          *
          * @param brush Specifies the brush used to fill the region inside of
          * the ellipse.  If null, the background will be transparent.
          */
         virtual void draw_ellipse(
            Point const &centre,
            double width,
            double height,
            pen_handle pen,
            brush_handle brush = 0) = 0;

         /**
         * Draw a wind barb at the given location, pointing to the given direction.  
         * Over-ride this routine if you want to implement drawing a wind barb.
         */
         virtual void draw_wind_barb(Point point, double dir, double length, pen_handle)
         {
         }

         /**
          * Sets the clipping region.
          *
          * @param rect Specifies the boundaries of the clipping region.  This will replace any
          * existing clipping region.
          */
         virtual void set_clipping_region(Rect const &rect) = 0;

         /**
          * Removes any associated clipping region.
          */
         virtual void remove_clipping_region() = 0;

      };


   };
};

#endif
