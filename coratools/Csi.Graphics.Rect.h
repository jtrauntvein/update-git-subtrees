/* Csi.Graphics.Rect.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 12 January 2015
   Last Change: Wednesday 23 September 2015
   Last Commit: $Date: 2015-09-23 14:57:28 -0600 (Wed, 23 Sep 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Graphics_Rect_h
#define Csi_Graphics_Rect_h

#include "Csi.MaxMin.h"
#include "Csi.FloatUtils.h"
#include <math.h>


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines a geometric point.
       */
      struct Point
      {
         /**
          * Specifies the x coordinate for this point.
          */
         double x;
         
         /**
          * Specifies the y coordinate for this point.
          */
         double y;
         
         /**
          * Default constructor
          */
         Point():
            x(0),
            y(0)
         { }
         
         /**
          * Construct from coordinates.
          */
         Point(double x_, double y_):
            x(x_),
            y(y_)
         { }
         
         /**
          * Copy constructor
          */
         Point(Point const &other):
            x(other.x),
            y(other.y)
         { }
         
         /**
          * Copy operator
          */
         Point &operator =(Point const &other)
         {
            x = other.x;
            y = other.y;
            return *this;
         }

         /**
          * @return Returns the distance between this point and the other specified point.
          *
          * @param other Specifies the other point.
          */
         double distance(Point const &other) const
         {
            double delta_x(x - other.x);
            double delta_y(y - other.y);
            return sqrt(delta_x * delta_x + delta_y * delta_y);
         }

         /**
          * @return Returns 0 if this point is equal to the other point, -1 if this point is less
          * than the other point, and 1 if this point is greater than the other point.
          */
         int compare(Point const &other) const
         {
            int rtn(0);
            if(x < other.x)
               rtn = -1;
            else if(x > other.x)
               rtn = 1;
            else
            {
               if(y < other.y)
                  rtn = -1;
               else if(y > other.y)
                  rtn = 1;
            }
            return rtn;
         }

         /**
          * @return Returns true if this point is equal to the other.
          */
         bool operator ==(Point const &other) const
         { return compare(other) == 0; }

         /**
          * @return Returns true if this point is less than the other.
          */
         bool operator <(Point const &other) const
         { return compare(other) < 0; }

         /**
          * @return Returns true if this point is less than or equal to the other.
          */
         bool operator <=(Point const &other) const
         { return compare(other) <= 0; }

         /**
          * @return Returns true if this point is greater than the other.
          */
         bool operator >(Point const &other) const
         { return compare(other) > 0; }

         /**
          * @return Returns true if this point is greater than or equal to the other.
          */
         bool operator >=(Point const &other) const
         { return compare(other) >= 0; }
      };
      
      
      /**
       * Defines a class that represents a rectangular area with an origin, width, and height.
       */
      class Rect
      {
      public:
         /**
          * Specifies the origin of this rectangle
          */
         Point origin;
         
         /**
          * Specifies the width of this rectangle
          */
         double width;
         
         /**
          * Specifies the height for this rectangle
          */
         double height;
         
         /**
          * Constructor with complete specs for the rectangle.
          *
          * @param x Specifies the x coordinate of the origin.
          *
          * @param y Specifis the y coordinate of the origin.
          *
          * @param width_ Specifies the width of this area.
          *
          * @param height_ Specifies the height of this area.
          */
         Rect(double x = 0, double y = 0, double width_ = 0, double height_ = 0):
            origin(x, y),
            width(width_),
            height(height_)
         { }

         /**
          * Construct a rectangle from two corner points.
          *
          * @param p1 Specifies one corner point.
          *
          * @param p2 Specifies the second corner point.
          */
         Rect(Point const &p1, Point const &p2):
            width(0),
            height(0)
         {
            if(p2.x >= p1.x && p2.y >= p1.y)
            {
               origin = p1;
               width = p2.x - p1.x;
               height = p2.y - p1.y;
            }
            else if(p2.x >= p1.x && p2.y < p1.y)
            {
               origin = Point(p1.x, p2.y);
               width = p2.x - p1.x;
               height = p1.y - p2.y;
            }
            else if(p2.x < p1.x && p2.y >= p1.y)
            {
               origin = Point(p2.x, p1.y);
               width = p1.x - p2.x;
               height = p2.y - p1.y;
            }
            else
            {
               origin = p2;
               width = p1.x - p2.x;
               height = p1.y - p2.y;
            }
         }
         
         /**
          * Copy contructor
          */
         Rect(Rect const &other):
            origin(other.origin),
            width(other.width),
            height(other.height)
         { }
         
         /**
          * Copy operator
          */
         Rect &operator =(Rect const &other)
         {
            origin = other.origin;
            width = other.width;
            height = other.height;
            return *this;
         }

         /**
          * Construct the rectangle with the bounds of all of the points.
          *
          * @param begin Specifies the first point in the set.
          *
          * @param end Specifies the iterator beyond the last point in the set.
          */
         template <class iterator>
         Rect(iterator begin, iterator end):
            width(0),
            height(0)
         {
            for(iterator pi = begin; pi != end; ++pi)
            {
               if(pi == begin)
                  origin = *pi;
               else
               {
                  if(pi->x < origin.x)
                  {
                     width += origin.x - pi->x;
                     origin.x = pi->x;
                  }
                  if(pi->x > get_right())
                     width += pi->x - get_right();
                  if(pi->y < origin.y)
                  {
                     height += origin.y - pi->y;
                     origin.y = pi->y;
                  }
                  if(pi->y > origin.y)
                     height += get_bottom() - pi->y;
               }
            }
         }
         
         
         /**
          * Sets the four corners of this rectangle.
          *
          * @param left Specifies the new left coordinate
          *
          * @param top Specifies the new top coordinate
          *
          * @param right Specifies the new right coordinate.
          *
          * @param bottom Specifies the new bottom coordinate.
          */
         void set_corners(double left, double top, double right, double bottom)
         {
            origin.x = left;
            origin.y = top;
            width = right - left;
            height = bottom - top;
         }
         
         /**
          * @return Returns true if the specified point lies within the bounds of this rectangle.
          *
          * @param point Specifies the point to test.
          */
         bool within(Point const &point)
         {
            bool rtn(
               point.x >= origin.x &&
               point.y >= origin.y &&
               point.x <= origin.x + width &&
               point.y <= origin.y + height);
            return rtn;
         }
         
         /**
          * @return Returns the coordinate for the left hand side of this rectangle.
          */
         double get_left() const
         { return origin.x; }
         
         /**
          * Sets the coordinate for the left hand side of this rectangle.
          *
          * @param x Specifies the new x coordinate for the left hand side.
          */
         void set_left(double x)
         { origin.x = x; }
         
         /**
          * @return Returns the coordinate for the top of this rectangle.
          */
         double get_top() const
         { return origin.y; }
         
         /**
          * Sets the coordinate for the top of this rectanngle.
          *
          * @param y Specifies the new y coordinate for the top.
          */
         void set_top(double y)
         { origin.y = y; }
         
         /**
          * @return Returns the coordinate for the right hand side of this rectangle.
          */
         double get_right() const
         { return origin.x + width; }
         
         /**
          * Sets the coordinate for the right hand side of this rectangle without
          * affecting the width.
          *
          * @param x Specifies the new x coordinate for the right hand side.
          */
         void set_right(double x)
         { origin.x = x - width; }
         
         /**
          * @return Returns the coordinate for the bottom of this rectangle.
          */
         double get_bottom() const
         { return origin.y + height; }
         
         /**
          * Sets the coordinate for the bottom side of this rectangle without
          * affecting the height.
          *
          * @param y Specifies the new y coordinate for the bottom edge.
          */
         void set_bottom(double y)
         { origin.y = y - height; }
         
         /**
          * @return Returns the centre point of this rectangle.
          */
         Point get_centre() const
         { return Point(origin.x + width / 2, origin.y + height / 2); }

         /**
          * @return Returns the centre x coordinate.
          */
         double get_centre_x() const
         { return origin.x + width / 2; }

         /**
          * @return Returns the centre y coordinate.
          */
         double get_centre_y() const
         { return origin.y + height / 2; }
         
         /**
          * Sets the centre point of this rectangle.
          *
          * @param centre_ Specifies the new centre for this rectangle.
          */
         void set_centre(Point const centre)
         {
            origin.x = centre.x - width / 2;
            origin.y = centre.y - height / 2;
         }
         
         /**
          * Centres the rectangle horizontally around the coordinate specified by cx.
          *
          * @param cx Specifies the new x centre coordinate.
          */
         void centre_x(double cx)
         {
            origin.x = cx - width / 2;
         }
         
         /**
          * Centres the rectangle vertically around the coordinate specified by cy.
          *
          * @param cy Specifies tyhe new centr y coordinate.
          */
         void centre_y(double cy)
         {
            origin.y = cy - height / 2;
         }
         
         /**
          * Moves the left and top side of the rectangle to the coordinates specified by point.
          *
          * @param point Specifies the new origin.
          */
         void move(Point const &point)
         {
            origin.x = point.x;
            origin.y = point.y;
         }
         
         /**
          * @return Returns the rectangle that is a union with this and the
          * specified rectangles.
          *
          * @param other Specifies the other rectangle.
          */
         Rect calc_union(Rect const &other)
         {
            Rect rtn;
            rtn.set_corners(
               csimin(get_left(), other.get_left()),
               csimin(get_top(), other.get_top()),
               csimax(get_right(), other.get_right()),
               csimax(get_bottom(), other.get_bottom()));
            return rtn;
         }
         
         /**
          * @return Returns the rectangle that is formed by the intersection of
          * this rectangle and the other specified rectangle.
          *
          * @param other  Specifies the other rectangle.
          */
         Rect calc_intersect(Rect const &other) const
         {
            double x2(get_right()), y2(get_bottom());
            double x1(get_left()), y1(get_top());
            Rect rtn;
            
            if(get_left() < other.get_left())
               x1 = other.get_left();
            if(y1 < other.get_left())
               y1 = other.get_top();
            if(x2 > other.get_right())
               x2 = other.get_right();
            if(y2 > other.get_bottom())
               y2 = other.get_bottom();
            rtn.set_corners(x1, y1, x2, y2);
            if(width <= 0 || height <= 0)
               rtn.width = rtn.height = 0;
            return rtn;
         }

         /**
          * @return Returns true if this area of this rectangle has a non-zero
          * intersection with the other specified rectangle.
          *
          * @param other Specifies the rectangle that will be determined
          * whether it intersects with this rectangle.
          */
         bool intersects(Rect const &other) const
         {
            Rect intersection(calc_intersect(other));
            return intersection.width > 0 && intersection.height > 0;
         }

         /**
          * @return Returns the bottom left corner of the rectangle.
          */
         Point get_bottom_left() const
         { return Point(origin.x, origin.y + height); }

         /**
          * @return Returns the top left corner of the rectangle.
          */
         Point get_top_left() const
         { return origin; }

         /**
          * @return Returns the top right corner of the rectangle.
          */
         Point get_top_right() const
         { return Point(origin.x + width, origin.y); }

         /**
          * @return Returns the bottom right corner of the rectangle.
          */
         Point get_bottom_right() const
         { return Point(origin.x + width, origin.y + height); }

         /**
          * Rotates this retangle around its centre point by the specified angle.
          *
          * @param angle Specifies the angle in degrees at which the rectangle should be rotated.  
          */
         void rotate(double angle)
         {
            // zero degree rotation or 180 degree rotation will not affect the rectangle.  If the
            // rotation is purely vertical, we have only to swap the width and the height.
            if(angle == 90 || angle == 270)
            {
               Point centre(get_centre());
               double temp(width);
               width = height;
               height = temp;
               set_centre(centre);
            }
            else if(angle != 0 && angle != 180)
            {
               double theta(degrees_to_radians(angle));
               Point centre(get_centre());
               double old_width(width);
               double old_height(height);
               width = fabs(old_width * cos(theta)) + fabs(old_height * cos(pi() / 2 - theta));
               height = fabs(old_width * sin(theta)) + fabs(old_height * sin(pi() / 2 - theta));
               set_centre(centre);
            }
         }
      };
   };
};


#endif
