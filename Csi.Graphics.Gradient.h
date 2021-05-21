/* Csi.Graphics.Gradient.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 10 February 2015
   Last Change: Thursday 17 December 2015
   Last Commit: $Date: 2016-02-11 15:35:50 -0600 (Thu, 11 Feb 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Graphics_Gradient_h
#define Csi_Graphics_Gradient_h

#include "Csi.Graphics.Colour.h"
#include "Csi.Graphics.Rect.h"
#include "Csi.Xml.Element.h"
#include "Csi.Utils.h"
#include <deque>


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines an object that describes the parameters for a gradient.  These
       * parameters include a collection of colours and a direction.
       */
      class Gradient
      {
      public:
         /**
          * Enumerates the possible directions for this gradient
          */
         enum direction_type
         {
            direction_vertical,
            direction_horizontal,
            direction_radial,
            direction_diagonal_up,
            direction_diagonal_down
         };

         /**
          * Defines the definition of a stop.
          */
         typedef std::pair<Colour, double> value_type;
         typedef std::deque<value_type> stops_type;

      private:
         /**
          * Specifies the direction used for this gradient.
          */
         direction_type direction;

         /**
          * Specifies the stops used for this gradient.
          */
         stops_type stops;

         /**
          * Specifies the offset point for a radial gradient.
          */
         Point radial_offset;

      public:
         /**
          * Default constructor.
          */
         Gradient():
            direction(direction_vertical)
         { }

         /**
          * Construct with a colour.
          *
          * @param first Specifies the first colour for the gradient.
          */
         Gradient(Colour const &first):
            direction(direction_vertical)
         {
            push_back(first);
         }

         /**
          * Construct with a start end end colour.
          *
          * @param first Specifies the start coloour for the gradient.
          *
          * @param last Specifies the end colour for the gradient.
          */
         Gradient(Colour const &first, Colour const &last):
            direction(direction_vertical)
         {
            stops.push_back(value_type(first, 0));
            stops.push_back(value_type(last, 0));
            set_stops_even();
         }

         /**
          * Copy constructor.
          *
          * @param other Specifies the gradient to copy.
          */
         Gradient(Gradient const &other):
            direction(other.direction),
            stops(other.stops)
         { }

         /**
          * Copy operator
          *
          * @param other Specifies the gradient to copy.
          *
          * @return Returns a reference to this object.
          */
         Gradient &operator =(Gradient const &other)
         {
            direction = other.direction;
            stops = other.stops;
            return *this;
         }

         /**
          * @return Returns the direction for this gradient.
          */
         direction_type get_direction() const
         { return direction; }

         /**
          * @param value Specifies the value for the direction.
          */
         Gradient &set_direction(direction_type value)
         {
            direction = value;
            return *this;
         }

         /**
          * @return Returns the first iterator in the sequence of colours.
          */
         typedef stops_type::iterator iterator;
         typedef stops_type::const_iterator const_iterator;
         iterator begin()
         { return stops.begin(); }
         const_iterator begin() const
         { return stops.begin(); }

         /**
          * @return Returns the last iterator in the sequence of colours.
          */
         iterator end()
         { return stops.end(); }
         const_iterator end() const
         { return stops.end(); }

         /**
          * @return Returns the reverse begin iterator.
          */
         typedef stops_type::reverse_iterator reverse_iterator;
         typedef stops_type::const_reverse_iterator const_reverse_iterator;
         reverse_iterator rbegin()
         { return stops.rbegin(); }
         const_reverse_iterator rbegin() const
         { return stops.rbegin(); }

         /**
          * @return Returns the reverse end iterator.
          */
         reverse_iterator rend()
         { return stops.rend(); }
         const_reverse_iterator rend() const
         { return stops.rend(); }

         /**
          * @return Returns true if there are no stops.
          */
         bool empty() const
         { return stops.empty(); }

         /**
          * Clears the stops for this gradient.
          */
         Gradient &clear()
         {
            stops.clear();
            return *this;
         }

         /**
          * @return Returns the number of stops.
          */
         typedef stops_type::size_type size_type;
         size_type size() const
         { return stops.size(); }

         /**
          * Adds the specified colour to the end of the sequence for this gradient.
          *
          * @param colour Specifies the colour to add.
          *
          * @param position Specifies a position of this colour stop.  If this value is less than
          * zero or greater than one, the colour will not be shown but will influence the colours
          * shown in the gradient.  If this value is specified as NaN (the default), all of the
          * colours in this gradient will be evenly distributed.
          */
         Gradient &push_back(
            Colour const &colour, double position = std::numeric_limits<double>::quiet_NaN())
         {
            stops.push_back(value_type(colour, position));
            if(!is_finite(position))
               set_stops_even();
            return *this;
         }

         /**
          * Adds the specified colour to the beginning of the sequence for this gradient.
          *
          * @param colour Specifies the colour to add.
          *
          * @param position Specifies a position of this colour stop.  If this value is less than
          * zero or greater than one, the colour will not be shown but will influence the colours
          * shown in the gradient.  If this value is specified as NaN (the default), all of the
          * colours in this gradient will be evenly distributed.
          */
         Gradient &push_front(
            Colour const &colour, double position = std::numeric_limits<double>::quiet_NaN())
         {
            stops.push_front(value_type(colour, position));
            if(!is_finite(position))
               set_stops_even();
            return *this;
         }

         /**
          * Sets the positions of all the stops so that they are spaced by an even intervals and all
          * colours will be visible.
          */
         void set_stops_even()
         {
            if(!stops.empty())
            {
               double interval = 1.0;
               if(stops.size() > 1)
                  interval = 1.0 / (stops.size() - 1);
               for(stops_type::size_type i = 0; i < stops.size(); ++i)
                  stops[i].second = interval * i;
            }
         }

         /**
          * @return Returns the first colour in the sequence for this gradient.
          */
         Colour &front()
         { return stops.front().first; }
         Colour const &front() const
         { return stops.front().first; }

         /**
          * @return Returns the last colour in the sequence for this gradient.
          */
         Colour &back()
         { return stops.back().first; }
         Colour const &back() const
         { return stops.back().first; }

         /**
          * @return Returns the first colour if present or returns a
          * transparent colour if there are no stops defined for this
          * gradient.
          */
         Colour first_colour() const
         {
            Colour rtn(transparent_colour());
            if(!stops.empty())
               rtn = stops.front().first;
            return rtn;
         }

         /**
          * @return Returns the last colour if present or returns a transparent
          * colour if this gradient is empty.
          */
         Colour last_colour() const
         {
            Colour rtn(transparent_colour());
            if(!stops.empty())
               rtn = stops.back().first;
            return rtn;
         }

         /**
          * @return Returns the position of the offset for a radial gradient.
          */
         Point const &get_radial_offset() const
         { return radial_offset; }

         /**
          * @param offset Specifies the offset relative to the centre of the radial gradient.  This
          * parameter will be ignored when the direction is not set to direction_radial.
          */
         Gradient &set_radial_offset(Point const &offset)
         {
            radial_offset = offset;
            return *this;
         }

         /**
          * Reads the gradient properties from the specified XML element.
          *
          * @param elem Specifies the XML element that should have the gradient properties.
          */
         void read(Xml::Element &elem)
         {
            using namespace Xml;
            Element::value_type stops_xml(elem.find_elem(L"stops"));
            direction = static_cast<direction_type>(elem.get_attr_uint4(L"direction"));
            if(elem.has_attribute(L"offset-x"))
            {
               radial_offset.x = elem.get_attr_double(L"offset-x");
               radial_offset.y = elem.get_attr_double(L"offset-y");
            }
            stops.clear();
            for(Element::iterator ci = stops_xml->begin(); ci != stops_xml->end(); ++ci)
            {
               Element::value_type colour_xml(*ci);
               stops.push_back(
                  value_type(
                     colour_xml->get_attr_colour(L"rgba"), colour_xml->get_attr_double(L"pos")));
            }
         }  

         /**
          * Writes the properties of this gradient to the specified XML element.
          *
          * @param elem Specifies the element to which this gradient will be described.
          */
         void write(Xml::Element &elem) const
         {
            Xml::Element::value_type stops_xml(elem.add_element(L"stops"));
            elem.set_attr_uint4(direction, L"direction");
            elem.set_attr_double(radial_offset.x, L"offset-x");
            elem.set_attr_double(radial_offset.y, L"offset-y");
            for(stops_type::const_iterator ci = stops.begin(); ci != stops.end(); ++ci)
            {
               Xml::Element::value_type colour_xml(stops_xml->add_element(L"colour"));
               colour_xml->set_attr_colour(ci->first, L"rgba");
               colour_xml->set_attr_double(ci->second, L"pos");
            }
         }

         /**
          * @return Overloads the comparison operator to return true if this gradient is equal to
          * the other gradient in every respect.
          */
         bool operator ==(Gradient const &other) const
         {
            bool rtn(direction == other.direction);
            if(rtn == true)
            {
               rtn = (radial_offset == other.radial_offset && stops.size() == other.stops.size());
               for(size_type i = 0; !rtn && i < stops.size(); ++i)
               {
                  value_type const &my_stop(stops[i]);
                  value_type const &other_stop(other.stops[i]);
                  rtn = (my_stop.first == other_stop.first && my_stop.second == other_stop.second);
               }
            }
            return rtn;
         }
      };
   };
};


#endif
