/* Csi.Graphics.NestedRect.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 12 January 2015
   Last Change: Tuesday 24 March 2015
   Last Commit: $Date: 2015-03-27 12:07:40 -0600 (Fri, 27 Mar 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Graphics_NestedRect_h
#define Csi_Graphics_NestedRect_h

#include "Csi.Graphics.Rect.h"
#include "Csi.LightSharedPtr.h"
#include <deque>


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines a class that is responsible for calculating the composite layout
       * for a set of constituent sub-rectangles.
       */
      class NestedRect
      {
      public:
         typedef LightSharedPtr<NestedRect> child_handle;
                  
      private:
         /**
          * Specifies the rectangle for this area.
          */
         Rect rect;
         
         /**
          * Specifies the parent for this area.
          */
         NestedRect *parent;
         
         /**
          * set to true if this area is considered stale.
          */
         bool stale;
         
         /**
          * Specifies the children associated with this area.
          */
         typedef std::deque<child_handle> children_type;
         children_type children;
         
      public:
         /**
          * Construct an empty area.
          */
         NestedRect():
            stale(false),
            parent(0)
         { }
         
         /**
          * Construct with the specified width and height.
          *
          * @param width Specifies the width for this rectangle.
          *
          * @param height Specifies the height for this rectangle.
          */
         NestedRect(double width, double height):
            rect(0, 0, width, height),
            stale(false),
            parent(0)
         { }
         
         /**
          * Construct with the a rectangle for this area.
          *
          * @param rect_  Specifies the origin and size of this rectangle.
          */
         NestedRect(Rect const &rect_):
            rect(rect_),
            stale(false),
            parent(0)
         { }

         /**
          * Destructor
          */
         virtual ~NestedRect()
         {
            while(!children.empty())
            {
               child_handle &child(children.back());
               child->parent = 0;
               children.pop_back();
            }
         }
         
         /**
          * Adds the specified child to the set managed by this area.
          *
          * @param child Specifies the child to add.
          */
         void add_child(Rect const &rect)
         { add_child(new NestedRect(rect)); }
         void add_child(child_handle child)
         {
            children.push_back(child);
            child->parent = this;
            mark_stale();
         }
         
         /**
          * Marks the layout for this area as stale.
          */
         void mark_stale()
         {
            NestedRect *pai(parent);
            stale = true;
            while(pai)
            {
               pai->stale = true;
               pai = pai->parent;
            }
         }
         
         
         /**
          * Generates the layout for this area based upon the children.
          */
         void layout()
         {
            if(!children.empty())
            {
               Rect child_rect;
               for(children_type::iterator ci = children.begin(); ci != children.end(); ++ci)
               {
                  child_handle &child(*ci);
                  child_rect = child->get_rect(false).calc_union(child_rect);
                  if(rect.width < child_rect.width)
                     rect.width = child_rect.width;
                  if(rect.height < child_rect.height)
                     rect.height = child_rect.height;
               }
            }
            stale = false;
         }
         
         /**
          * @return Returns the rectsngle for this area.
          *
          * @param use_parent Set to true if the area should be adjusted for the
          * parent coordinates.
          */
         Rect get_rect(bool use_parent = true)
         {
            Rect rtn;
            NestedRect *pai(parent);
            if(stale)
               layout();
            rtn = rect;
            while(use_parent && pai)
            {
               rtn.set_left(rtn.get_left() + pai->get_left(false));
               rtn.set_top(rtn.get_top() + pai->get_top(false));
               pai = pai->parent;
            }
            return rtn;
         }
         
         /**
          * @return Returns the coordinate for the left had side of this area.
          *
          * @param use_parent Set to true if the coordinate should be adjusted to
          * fit the parent's coordinate system.
          */
         double get_left(bool use_parent = true)
         {
            Rect rtn(get_rect(use_parent));
            return rtn.get_left();
         }
         
         /**
          * Sets the left side of this layout.
          *
          * @param x Specifies the x coordinate for the left side.
          */
         void set_left(double x)
         {
            rect.set_left(x);
            if(parent)
            parent->mark_stale();
         }
         
         /**
          * @return Returns the coordinate of the right hand side of this area.
          *
          * @param use_parent Set to true if the return value should be adjusted
          * to account for the parent coordinate system.
          */
         double get_right(bool use_parent = true)
         {
            Rect rtn(get_rect(use_parent));
            return rtn.get_right();
         }
         
         /**
          * Specifies the coordinate for the right side of this area.
          *
          * @param x Specifies the coordinate.
          */
         void set_right(double x)
         {
            if(stale)
               layout();
            rect.set_right(x);
            if(parent)
               parent->mark_stale();
         }
         
         
         /**
          * @return Returns the coordinate for the top of this area.
          *
          * @param use_parent Set to true if the return value should be adjusted
          * to account for the parent's coordinate system.
          */
         double get_top(bool use_parent = true)
         {
            Rect rtn(get_rect(use_parent));
            return rtn.get_top();
         }
         
         
         /**
          * Sets the top coordinate of this area.
          *
          * @param y Specifies the y coordinate for the top.
          */
         void set_top(double y)
         {
            rect.origin.y = y;
            if(parent)
               parent->mark_stale();
         }
         
         /**
          * @return Returns the coordinate for the bottom of this layout area.
          *
          * @param use_parent Set to true if the return value should be adjusted
          * to account for the parent's coordinate system.
          */
         double get_bottom(bool use_parent = true)
         { 
            Rect rect(get_rect(use_parent));
            return rect.get_bottom();
         }
         
         /**
          * Sets the bottom coordinate for this layout.
          *
          * @param y Specifies the new bottom coordinate.
          */
         void set_bottom(double y)
         {
            if(stale)
               layout();
            rect.set_bottom(y);
            if(parent)
               parent->mark_stale();
         }

         /**
          * @return Returns the width of this area.
          */
         double get_width()
         {
            if(stale)
               layout();
            return rect.width;
         }

         /**
          * Sets the width of this area.
          *
          * @param width Specifies the new width.
          */
         void set_width(double width)
         {
            rect.width = width;
            if(parent)
               parent->mark_stale();
         }
      
         /**
          * Sets the height for this area.
          *
          * @param height Specifies the new height for this area.
          */
         void set_height(double height)
         {
            rect.height = height;
            if(parent)
               parent->mark_stale();
         }

         /**
          * @return Returns the height of this area.
          */
         double get_height()
         {
            if(stale)
               layout();
            return rect.height;
         }

         /**
          * @return Returns the centre coordinates of this area.
          *
          * @param use_parent Set to true if the return value should be adjusted
          * for the parent's coordinate system.
          */
         Point get_centre(bool use_parent = true)
         {
            Rect rtn(get_rect(use_parent));
            return rtn.get_centre();
         }

         /**
          * Sets the centre of this rectangle around the point specified by cx
          * and cy.
          *
          * @param cx Specifies the new centre of the rectangle on the x axis.
          *
          * @param cy Specifies the new centre of the rectangle in the y axis.
          */
         void centre(double cx, double cy)
         {
            if(stale)
               layout();
            rect.set_centre(Point(cx, cy));
            if(parent)
               parent->mark_stale();
         }

         /**
          * Centres the rectangle horizontally around the coordinate specified by
          * cx.
          *
          * @param cx Specifies the new x centre coordinate.
          */
         void centre_x(double cx)
         {
            if(stale)
               layout();
            rect.centre_x(cx);
            if(parent)
               parent->mark_stale();
         }

         /**
          * Centres the rectangle vertically around the coordinate specified by
          * cy.
          *
          * @param cy Specifies the new y centre coordinate.
          */
         void centre_y(double cy)
         {
            if(stale)
               layout();
            rect.centre_y(cy);
            if(parent)
               parent->mark_stale();
         }

         /**
          * Shifts the rectangle from its current position by the offsets
          * specified by dx and dy.
          *
          * @param dx Specifies the amount of distance in the x axis.
          *
          * @param dy Specifies the amount of distance in the y axis.
          */
         void shift(double dx, double dy)
         {
            rect.origin.x += dx;
            rect.origin.y += dy;
            mark_stale();
         }

         /**
          * Moves the rectangle to the coordinates specified by x and y.
          *
          * @param x Specifies the new left side coordinate.
          *
          * @param y Specifis the new top coordinate.
          */
         void move(double x, double y)
         {
            rect.move(Point(x, y));
            mark_stale();
         }

         /**
          * @return Returns the point translated to the parent's coordinate
          * systems.
          *
          * @param x Specifies the x coordinate of the point to translate.
          *
          * @param y Specifies the y coordinate of the point to translate.
          */
         Point translate_to_point(double x, double y)
         {
            Point rtn(x, y);
            NestedRect *pai(parent);
            while(pai)
            {
               rtn.x += pai->rect.origin.x;
               rtn.y += pai->rect.origin.y;
               pai = pai->parent;
            }
            return rtn;
         }

         /**
          * @return Returns the point specified by x and y in the parent's
          * coordinate system to a point in this area's coordinate system.
          *
          * @param x Specifies the x coordinate in the parent's coordinate
          * system.
          *
          * @param y Specifies the y coordinate in the parent's coordinate
          * system.
          */
         Point translate_from_point(double x, double y)
         {
            Point rtn(x, y);
            NestedRect *pai = parent;
            while(pai)
            {
               rtn.x -= pai->get_left(false);
               rtn.y -= pai->get_top(false);
               pai = pai->parent;
            }
            return rtn;
         }

         /**
          * Stacks all of the child areas of this area vertically with the
          * specified margin between them.
          *
          * @param margin Specifies the space between stacked areas.
          */
         void stack_vertical(double margin = 0)
         {
            double offset(0);
            for(children_type::iterator ci = children.begin();
                ci != children.end();
                ++ci)
            {
               child_handle &child(*ci);
               child->move(child->get_left(false), offset);
               offset += child->get_height() + margin;
            }
            layout();
         }

         /**
          * Stacks all of the child areas of this area horizontally with the
          * specified margin in between.
          *
          * @param margin Specifies the horizontal margin between child areas.
          */
         void stack_horizontal(double margin = 0)
         {
            double offset(0);
            for(children_type::iterator ci = children.begin();
                ci != children.end();
                ++ci)
            {
               child_handle &child(*ci);
               child->move(offset, child->get_top(false));
               offset += child->get_width() + margin;
            }
            layout();
         }

         /**
          * Stacks the child areas in a grid pattern with one dimension capped by
          * the fixed parameter and the other dimension allowed to grow.
          *
          * @param fixed Specifies the size of the fixed dimension.
          *
          * @param is_vertical Set to true if the fixed dimension is vertical.
          */
         void stack_grid(double fixed, bool is_vertical);

         /**
          * Shifts the children by the spaces specified by dx and dy.
          *
          * @param dx Specifies the amount to shift the children in the x
          * coordinate.
          *
          * @param dy Specifies the amount to shift the children in the y
          * coordinate.
          */
         void shift_children(double dx, double dy)
         {
            for(children_type::iterator ci = children.begin();
                ci != children.end();
                ++ci)
            {
               child_handle &child(*ci);
               child->shift(dx, dy);
            }
         }

         /**
          * @return Returns true if the specified point is within the rectangle
          * for this area
          *
          * @param point Specifies the point to evaluate.
          *
          * @param use_parent Set to true if the point should be evaluated in the
          * parent's coordinate system.
          */
         bool within(Point const &point, bool use_parent = true)
         {
            Rect temp(get_rect(use_parent));
            return temp.within(point);
         }

         // @group: methods and declarations to act as a container of other nested rectangles.

         /**
          * @return Returns the first iterator to the children of this rectangle.
          */
         typedef children_type::iterator iterator;
         typedef children_type::const_iterator const_iterator;
         iterator begin()
         { return children.begin(); }
         const_iterator begin() const
         { return children.begin(); }

         /**
          * @return Returns the end iterator to the children of this rectangle.
          */
         iterator end()
         { return children.end(); }
         const_iterator end() const
         { return children.end(); }

         /**
          * @return Returns true if there are no children associated with this rectangle.
          */
         bool empty() const
         { return children.empty(); }

         /**
          * @return Returns the number of children associated with this rectangle.
          */
         typedef children_type::size_type size_type;
         size_type size() const
         { return children.size(); }

         /**
          * Removes all children from this rectangle.
          */
         void clear()
         { children.clear(); }

         /**
          * @return Returns a reference to the first child.
          */
         typedef children_type::value_type value_type;
         value_type &front()
         { return children.front(); }
         value_type const &front() const
         { return children.front(); }

         /**
          * @return Returns a reference to the last child.
          */
         value_type &back()
         { return children.back(); }
         value_type const &back() const
         { return children.back(); }

         // @endgroup:
      };
   };
};


#endif
