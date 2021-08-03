/* Csi.Graphics.Graph.cpp

   Copyright (C) 2015, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 22 January 2015
   Last Change: Thursday 28 January 2016
   Last Commit: $Date: 2020-07-14 15:42:11 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#define NOMINMAX
#include "Csi.Graphics.Graph.h"
#include "coratools.strings.h"
#include <algorithm>
#include <iterator>


namespace Csi
{
   namespace Graphics
   {
      namespace
      {
         double const small_margin(2);
         double const legend_margin(10);
      };

      
      Graph::Graph(driver_handle driver_):
         driver(driver_),
         positions_invalid(true),
         layout(new NestedRect),
         mouse_state(mouse_idle)
      {
         bottom_axis.bind(new GraphAxis(this, GraphAxis::axis_bottom, true));
         left_axis.bind(new GraphAxis(this, GraphAxis::axis_left, false));
         right_axis.bind(new GraphAxis(this, GraphAxis::axis_right, false));
         set_default_properties(false);
         bar_count = 0;
         bar_width = 0;
      } // constructor


      Graph::~Graph()
      {
      } // destructor


      Graph &Graph::set_rotation(rotation_type value)
      {
         rotation = value;
         switch(rotation)
         {
         case rotation_none:
            bottom_axis->set_axis_type(GraphAxis::axis_bottom);
            left_axis->set_axis_type(GraphAxis::axis_left);
            right_axis->set_axis_type(GraphAxis::axis_right);
            break;
            
         case rotation_right:
            bottom_axis->set_axis_type(GraphAxis::axis_left);
            left_axis->set_axis_type(GraphAxis::axis_top);
            right_axis->set_axis_type(GraphAxis::axis_bottom);
            break;
            
         case rotation_left:
            bottom_axis->set_axis_type(GraphAxis::axis_right);
            left_axis->set_axis_type(GraphAxis::axis_bottom);
            right_axis->set_axis_type(GraphAxis::axis_top);
            break;
         }
         positions_invalid = true;
         return *this;
      } // set_rotation
      

      void Graph::write(Xml::Element &elem, bool write_traces)
      {
         Xml::Element::value_type title_xml(elem.add_element(L"title"));
         Xml::Element::value_type legend_xml(elem.add_element(L"legend"));
         Xml::Element::value_type background_xml(elem.add_element(L"background"));
         Xml::Element::value_type plot_area_xml(elem.add_element(L"plot-area"));
         Xml::Element::value_type rect_xml(elem.add_element(L"rect"));
         Xml::Element::value_type bottom_axis_xml(elem.add_element(L"bottom-axis"));
         Xml::Element::value_type left_axis_xml(elem.add_element(L"left-axis"));
         Xml::Element::value_type right_axis_xml(elem.add_element(L"right-axis"));

         elem.set_attr_int4(rotation, L"rotation");
         title_xml->set_attr_wstr(title, L"value");
         title_xml->set_attr_colour(title_colour, "colour");
         if(title_font != 0)
         {
            Xml::Element::value_type title_font_xml(title_xml->add_element(L"font"));
            title_font->get_desc().write(*title_font_xml);
         }
         if(legend_font != 0)
         {
            Xml::Element::value_type font_xml(legend_xml->add_element(L"font"));
            legend_font->get_desc().write(*font_xml);
         }
         legend_xml->set_attr_uint4(legend_pos, L"position");
         if(legend_brush != 0)
         {
            Xml::Element::value_type brush_xml(legend_xml->add_element(L"brush"));
            legend_brush->get_desc().write(*brush_xml);
         }
         legend_xml->set_attr_colour(legend_text_colour, L"colour");
         if(legend_pen != 0)
         {
            Xml::Element::value_type pen_xml(legend_xml->add_element(L"pen"));
            legend_pen->get_desc().write(*pen_xml);
         }
         elem.set_attr_int8(display_width, L"display-width");
         elem.set_attr_int8(newest_time, L"newest-time");
         if(background_brush != 0)
         {
            Xml::Element::value_type brush_xml(background_xml->add_element(L"brush"));
            background_brush->get_desc().write(*brush_xml);
         }
         if(background_pen != 0)
         {
            Xml::Element::value_type pen_xml(background_xml->add_element(L"pen"));
            background_pen->get_desc().write(*pen_xml);
         }
         if(plot_area_brush != 0)
         {
            Xml::Element::value_type brush_xml(plot_area_xml->add_element(L"brush"));
            plot_area_brush->get_desc().write(*brush_xml);
         }
         if(plot_area_pen != 0)
         {
            Xml::Element::value_type pen_xml(plot_area_xml->add_element(L"pen"));
            plot_area_pen->get_desc().write(*pen_xml);
         }
         rect_xml->set_attr_double(layout->get_left(), L"left");
         rect_xml->set_attr_double(layout->get_top(), L"top");
         rect_xml->set_attr_double(layout->get_width(), L"width");
         rect_xml->set_attr_double(layout->get_height(), L"height");
         left_axis->write(*left_axis_xml);
         bottom_axis->write(*bottom_axis_xml);
         right_axis->write(*right_axis_xml);
         if(write_traces)
         {
            Xml::Element::value_type traces_xml(elem.add_element(L"traces"));
            for(iterator ti = begin(); ti != end(); ++ti)
            {
               trace_handle &trace(*ti);
               Xml::Element::value_type trace_xml(traces_xml->add_element(L"trace"));
               trace->write(*trace_xml);
            }
         }
         elem.set_attr_double(bar_width_ratio, L"bar-width-ratio");
         elem.set_attr_double(max_bar_width,   L"max-bar-width");
         elem.set_attr_int4(bar_overlap_style, L"bar-overlap-style");
      } // write


      void Graph::read(Xml::Element &elem, bool read_traces)
      {
         using namespace Xml;
         Element::value_type title_xml(elem.find_elem(L"title"));
         Element::value_type legend_xml(elem.find_elem(L"legend"));
         Element::value_type background_xml(elem.find_elem(L"background"));
         Element::value_type plot_area_xml(elem.find_elem(L"plot-area"));
         Element::value_type rect_xml(elem.find_elem(L"rect"));
         Element::value_type bottom_axis_xml(elem.find_elem(L"bottom-axis"));
         Element::value_type left_axis_xml(elem.find_elem(L"left-axis"));
         Element::value_type right_axis_xml(elem.find_elem(L"right-axis"));

         if(elem.has_attribute(L"rotation"))
            set_rotation(static_cast<rotation_type>(elem.get_attr_int4(L"rotation")));
         else
            set_rotation(rotation_none);
         title = title_xml->get_attr_wstr(L"value");
         title_colour = title_xml->get_attr_colour(L"colour");
         title_font.clear();
         try
         {
            Element::value_type title_font_xml(title_xml->find_elem(L"font"));
            FontInfo desc;
            desc.read(*title_font_xml);
            title_font = driver->make_font(desc);
         }
         catch(std::exception &)
         { }
         legend_font.clear();
         legend_pen.clear();
         legend_brush.clear();
         legend_text_colour = legend_xml->get_attr_colour(L"colour");
         legend_pos = static_cast<legend_pos_type>(legend_xml->get_attr_uint4(L"position"));
         for(Element::iterator ei = legend_xml->begin(); ei != legend_xml->end(); ++ei)
         {
            Element::value_type &child(*ei);
            if(child->get_name() == L"font")
            {
               FontInfo info;
               info.read(*child);
               legend_font = driver->make_font(info);
            }
            else if(child->get_name() == L"pen")
            {
               PenInfo info;
               info.read(*child);
               legend_pen = driver->make_pen(info);
            }
            else if(child->get_name() == L"brush")
            {
               BrushInfo info;
               info.read(*child);
               legend_brush = driver->make_brush(info);
            }
         }
         display_width = elem.get_attr_int8(L"display-width");
         newest_time = elem.get_attr_int8(L"newest-time");

         try
         {
            bar_width_ratio = elem.get_attr_double(L"bar-width-ratio");
            max_bar_width = elem.get_attr_double(L"max-bar-width");
            bar_overlap_style = static_cast<bar_overlap_style_type>(elem.get_attr_int4(L"bar-overlap-style"));
         }
         catch(std::exception &)
         { }

         background_brush.clear();
         background_pen.clear();
         for(Element::iterator ei = background_xml->begin(); ei != background_xml->end(); ++ei)
         {
            Element::value_type &child(*ei);
            if(child->get_name() == L"brush")
            {
               BrushInfo info;
               info.read(*child);
               background_brush = driver->make_brush(info);
            }
            else if(child->get_name() == L"pen")
            {
               PenInfo info;
               info.read(*child);
               background_pen = driver->make_pen(info);
            }
         }
         plot_area_brush.clear();
         plot_area_pen.clear();
         for(Element::iterator ei = plot_area_xml->begin(); ei != plot_area_xml->end(); ++ei)
         {
            Element::value_type &child(*ei);
            if(child->get_name() == L"brush")
            {
               BrushInfo info;
               info.read(*child);
               plot_area_brush = driver->make_brush(info);
            }
            else if(child->get_name() == L"pen")
            {
               PenInfo info;
               info.read(*child);
               plot_area_pen = driver->make_pen(info);
            }
         }
         set_rect(
            Rect(
               rect_xml->get_attr_double(L"left"),
               rect_xml->get_attr_double(L"top"),
               rect_xml->get_attr_double(L"width"),
               rect_xml->get_attr_double(L"height")));
         bottom_axis->read(*bottom_axis_xml);
         left_axis->read(*left_axis_xml);
         right_axis->read(*right_axis_xml);
         if(read_traces)
         {
            Element::value_type traces_xml(elem.find_elem(L"traces"));
            traces.clear();
            for(Element::iterator ti = traces_xml->begin(); ti != traces_xml->end(); ++ti)
            {
               Element::value_type &trace_xml(*ti);
               trace_handle trace(make_trace(driver));
               trace->read(*trace_xml);
               push_back(trace);
            }
         }
      } // read
      

      void Graph::push_back(trace_handle trace)
      {
         trace->graph = this;
         if (trace->get_visible())
         {
            bottom_axis->push_back(trace);
            trace->set_domain_axis(bottom_axis.get_rep());

            if(trace->get_vertical_axis() == GraphTrace::range_left)
            {
               left_axis->push_back(trace);
               trace->set_range_axis(left_axis.get_rep());
            }
            else
            {
               right_axis->push_back(trace);
               trace->set_range_axis(right_axis.get_rep());
            }
         }
         traces.push_back(trace);
         positions_invalid = true;
      } // push_back

      void Graph::move_trace(size_t from_index, size_t to_index)
      {
         // we need to rearrange the pen in the list managed by this component.
         trace_handle trace(traces[from_index]);
         traces.erase(traces.begin() + from_index);
         //if (from_index < to_index)
         //   --to_index;
         traces.insert(traces.begin() + to_index, trace);
      } // move_trace

      void Graph::on_trace_axis_changed(GraphTrace *trace_)
      {
         iterator ti(
            std::find_if(
               begin(), end(), HasLightSharedPtr<GraphTrace>(trace_)));
         if(ti != end())
         {
            trace_handle &trace(*ti);
            left_axis->erase(trace);
            right_axis->erase(trace);
            if (trace->get_visible())
            {
               if(trace->get_vertical_axis() == GraphTrace::range_left)
               {
                  left_axis->push_back(trace);
                  trace->set_range_axis(left_axis.get_rep());
               }
               else
               {
                  right_axis->push_back(trace);
                  trace->set_range_axis(right_axis.get_rep());
               }
            }
            positions_invalid  = true;
         }
      } // on_trace_axis_changed


      void Graph::erase(iterator begin, iterator end)
      {
         traces_type affected;
         std::copy(begin, end, std::back_inserter(affected));
         traces.erase(begin, end);
         for(iterator ai = affected.begin(); ai != affected.end(); ++ai)
         {
            trace_handle &trace(*ai);
            trace->set_domain_axis(0);
            trace->set_range_axis(0);
            trace->graph = 0;
            if(trace->get_vertical_axis() == GraphTrace::range_left)
               left_axis->erase(trace);
            else
               right_axis->erase(trace);
            bottom_axis->erase(trace);
         }
         positions_invalid = true;
      } // erase

      
      void Graph::clear()
      {
         traces.clear();
         left_axis->clear();
         right_axis->clear();
         bottom_axis->clear();
      } // clear


      void Graph::calculate_bar_width()
      {
         double all_bar_width = (std::numeric_limits<double>::max());
         bar_count = 0;

         for each (trace_handle trace in traces)
         {
            if (trace->get_trace_type() == GraphTrace::trace_bar && trace->get_visible())
            {
               bool bValid = false;
               double prev_x(std::numeric_limits<double>::quiet_NaN());
               GraphAxis *domain_axis = trace->get_domain_axis();
               for (GraphTrace::iterator pi = trace->begin(); pi != trace->end(); ++pi)
               {
                  GraphPoint const &point(*pi);
                  double x(domain_axis->value_to_pos(point.x));
                  if (is_finite(x) && is_finite(prev_x))
                  { 
                     if (x > prev_x)
                     {
                        all_bar_width = csimin(all_bar_width, x - prev_x);
                     }
                     else if (x < prev_x)
                     {
                        all_bar_width = csimin(all_bar_width, prev_x - x);
                     }
                     bValid = true;
                  }
                  prev_x = x;
               }
               if (bValid)
               {
                  trace->set_bar_position(static_cast<int>(bar_count));
                  bar_count++;
               }
            }
         }

         if (bar_count == 0)
            bar_width = 0;
         else if (bar_overlap_style == bars_overlap)
            bar_width = all_bar_width * (bar_count / (2 * bar_count - 1));
         else
            bar_width = all_bar_width;

         if (max_bar_width > 0)
            bar_width = csimin(bar_width, max_bar_width);

         if (bar_width_ratio > 0 && bar_width_ratio < 1)
            bar_width *= bar_width_ratio;

      }

      void Graph::set_default_properties(bool set_axis_props)
      {
         FontInfo default_font_desc(driver->default_font_desc());
         FontInfo title_font_desc(default_font_desc);
         title_font_desc.set_point_size(title_font_desc.get_point_size() + 2).set_weight(FontInfo::weight_bold);
         title_colour = black_colour();
         positions_invalid = true;
         title_font = driver->make_font(title_font_desc);
         legend_font = driver->make_font(default_font_desc);
         legend_pen = driver->make_pen(PenInfo());
         background_brush = driver->make_brush(BrushInfo().set_colour(milk_white_colour()));
         legend_brush = driver->make_brush(BrushInfo().set_colour(transparent_colour()));
         plot_area_brush = legend_brush;
         legend_text_colour = black_colour();
         legend_pos = legend_right;
         plot_area_pen = driver->make_pen(PenInfo().set_colour(dark_gray_colour()));
         bar_width_ratio = 0.75;
         max_bar_width = 50;
         bar_overlap_style = bars_overlap;
         set_rotation(rotation_none);
         if(set_axis_props)
         {
            bottom_axis->set_default_properties();
            left_axis->set_default_properties();
            right_axis->set_default_properties();
         }
      } // set_default_properties
      

      void Graph::calculate_positions()
      {
         // we will start by removing all children from the layout rectangle.  We can then create
         // the title rect and initialise the plot rectangle.
         layout->clear();
         layout->set_width(window_rect.width);
         layout->set_height(window_rect.height);
         layout->set_left(window_rect.get_left());
         layout->set_top(window_rect.get_top());
         if(title.length() > 0)
         {
            title_rect.bind(
               new NestedRect(
                  driver->measure_text(title, title_font)));
            title_rect->centre_x(layout->get_width() / 2);
            layout->add_child(title_rect);
         }
         else
            title_rect.bind(new NestedRect);
         plot_rect.bind(
            new NestedRect(
               layout->get_width(),
               layout->get_height() - title_rect->get_height() - small_margin * 2));
         plot_rect->set_top(title_rect->get_bottom(false) + small_margin * 2);
         layout->add_child(plot_rect);

         // we need to allocate the space for the legend in the layout.
         if(legend_pos != legend_not_shown && !empty() && legend_font != nullptr)
         {
            // we need to allocate the legend rectangle and add a rectangle for each trace.
            legend_rect.bind(new NestedRect);
            for(iterator ti = begin(); ti != end(); ++ti)
            {
               trace_handle &trace(*ti);
               if (trace->get_visible())
               {
                  trace->make_legend_rect(legend_rect, legend_font);
               }
            }
            switch(legend_pos)
            {
            case legend_top:
               legend_rect->stack_grid(plot_rect->get_width(), false);
               if(legend_rect->get_height() < plot_rect->get_height() / 2)
                  plot_rect->set_height(plot_rect->get_height() - legend_rect->get_height() - legend_margin * 2);
               else
                  plot_rect->set_height(plot_rect->get_height() / 2);
               legend_rect->centre_x(plot_rect->get_centre(false).x);
               legend_rect->set_top(title_rect->get_bottom(false) + small_margin * 2);
               plot_rect->set_top(legend_rect->get_bottom(false) + legend_margin);
               break;
               
            case legend_bottom:
               legend_rect->stack_grid(plot_rect->get_width(), false);
               if(legend_rect->get_height() < plot_rect->get_height() / 2)
                  plot_rect->set_height(plot_rect->get_height() - legend_rect->get_height() - legend_margin * 2);
               else
                  plot_rect->set_height(plot_rect->get_height() / 2);
               legend_rect->centre_x(plot_rect->get_centre(false).x);
               legend_rect->set_top(plot_rect->get_bottom(false) + legend_margin);
               break;
               
            case legend_left:
               legend_rect->stack_grid(plot_rect->get_height(), true);
               if(legend_rect->get_width() < plot_rect->get_width() / 2)
                  plot_rect->set_width(plot_rect->get_width() - legend_rect->get_width() - legend_margin * 2);
               else
                  plot_rect->set_width(plot_rect->get_width() / 2);
               legend_rect->centre_y(plot_rect->get_centre(false).y);
               legend_rect->set_left(10);
               plot_rect->set_left(legend_rect->get_right(false) + legend_margin);
               break;
               
            case legend_right:
               legend_rect->stack_grid(plot_rect->get_height(), true);
               if(legend_rect->get_width() < plot_rect->get_width() / 2)
                  plot_rect->set_width(plot_rect->get_width() - legend_rect->get_width() - legend_margin * 2);
               else
                  plot_rect->set_width(plot_rect->get_width() / 2);
               legend_rect->set_left(plot_rect->get_right(false) + legend_margin);
               legend_rect->centre_y(plot_rect->get_centre(false).y);
               break;
            }
            layout->add_child(legend_rect);
         }
         else
            legend_rect.clear();

         if(rotation == rotation_none)
         {
            // we need to generate the axes rectangles and adjust the width of the left and right
            // axes to account for the space required for 1/2 the width of the domain axis label.
            // This will ensure that the domain axis labels are not truncated.
            rect_handle bottom_axis_rect(
               bottom_axis->generate_axis_rect(plot_rect->get_width()));
            rect_handle left_axis_rect(
               left_axis->generate_axis_rect(plot_rect->get_height() - bottom_axis_rect->get_height()));
            rect_handle right_axis_rect(
               right_axis->generate_axis_rect(plot_rect->get_height() - bottom_axis_rect->get_height()));

            if(!bottom_axis->get_scale()->empty())
            {
               Scale::value_type first_label(bottom_axis->get_scale()->front());
               double half_label_width(first_label->rect->get_width() / 2);
               double left_wprime(plot_rect->get_left(false) + left_axis_rect->get_width());
               double right_wprime(layout->get_right() - plot_rect->get_right(true) + right_axis_rect->get_width());
               if (right_wprime < 0)
                  right_wprime = 0;

               // don't let the legend and bottom axis overlap
               if (legend_pos == legend_right || legend_pos == legend_left)
               {
                  if (legend_rect->get_height() > plot_rect->get_height() - bottom_axis_rect->get_height())
                  {
                     if (legend_pos == legend_right)
                        right_wprime = 0;
                     else if (legend_pos == legend_left)
                        left_wprime = 0;
                  }
               }

               if(half_label_width > left_wprime)
               {
                  left_axis_rect->shift_children(half_label_width - left_wprime, 0);
                  left_axis_rect->add_child(new NestedRect(half_label_width - left_wprime, 2));
               }
               if (half_label_width > right_wprime)
               {
                  //right_axis_rect->shift_children(-(half_label_width - right_wprime), 0);
                  right_axis_rect->add_child(new NestedRect(half_label_width, 2));
               }
            }
            
            // The axes need to adjust the plot layout based upon the space needed for labels.
            // There is a mutual dependency between the width of the bottom axis and the height of
            // the left and right axes that makes this less than simple.
            plot_rect->set_height(plot_rect->get_height() - bottom_axis_rect->get_height());
            plot_rect->set_width(plot_rect->get_width() - left_axis_rect->get_width() - right_axis_rect->get_width());
            plot_rect->move(plot_rect->get_left(false) + left_axis_rect->get_width(), plot_rect->get_top(false));

            // Now, recalculate the bottom axis, based on the bar width and the new width of the 
            // bar, based on positions of the left and right axis
            double orig_bar_width = bar_width;
            calculate_bar_width();
            bottom_axis_rect = bottom_axis->generate_axis_rect(plot_rect->get_width());

            int iterationCount = 0;
            while (fabs(bar_width - orig_bar_width) > 3 && iterationCount++ < 4)
            {
               orig_bar_width = bar_width;
               calculate_bar_width();
               bottom_axis_rect = bottom_axis->generate_axis_rect(plot_rect->get_width());
            }

            bottom_axis_rect->move(plot_rect->get_left(false), plot_rect->get_bottom(false));
            left_axis_rect->set_right(plot_rect->get_left(false));
            left_axis_rect->set_top(plot_rect->get_top(false));
            right_axis_rect->move(plot_rect->get_right(false), plot_rect->get_top(false));

            layout->add_child(bottom_axis_rect);
            layout->add_child(left_axis_rect);
            layout->add_child(right_axis_rect);
         }
         else
         {
            // for the time being, we will not worry about adjusting the domain width.  We will
            // adjust the plot rectangle.
            rect_handle domain_axis_rect(
               bottom_axis->generate_axis_rect(plot_rect->get_height()));
            rect_handle left_axis_rect(
               left_axis->generate_axis_rect(plot_rect->get_width() - domain_axis_rect->get_width()));
            rect_handle right_axis_rect(
               right_axis->generate_axis_rect(plot_rect->get_width() - domain_axis_rect->get_width()));

            // todo?? the non-rotated graphs verifies that the labels do not draw off the screen
            // or ovelap with the legend at this point.


            plot_rect->set_height(plot_rect->get_height() - left_axis_rect->get_height() - right_axis_rect->get_height());
            plot_rect->set_width(plot_rect->get_width() - domain_axis_rect->get_width());
            if(rotation == rotation_left)
               plot_rect->move(plot_rect->get_left(false), plot_rect->get_top(false) + right_axis_rect->get_height());
            else
               plot_rect->move(plot_rect->get_left(false) + domain_axis_rect->get_width(), plot_rect->get_top(false) + left_axis_rect->get_height());

            // Now, recalculate the bottom axis, based on the bar width and the new width of the 
            // bar, based on positions of the left and right axis
            double orig_bar_width = bar_width;
            calculate_bar_width();
            domain_axis_rect = bottom_axis->generate_axis_rect(plot_rect->get_height());

            int iterationCount = 0;
            while (fabs(bar_width - orig_bar_width) > 3 && iterationCount++ < 4)
            {
               orig_bar_width = bar_width;
               calculate_bar_width();
               domain_axis_rect = bottom_axis->generate_axis_rect(plot_rect->get_height());
            }

            domain_axis_rect->set_top(plot_rect->get_top(false));
            left_axis_rect->set_left(plot_rect->get_left(false));
            right_axis_rect->set_left(plot_rect->get_left(false));
            if(rotation == rotation_left)
            {
               domain_axis_rect->set_left(plot_rect->get_right(false));
               left_axis_rect->set_top(plot_rect->get_bottom(false));
               right_axis_rect->set_bottom(plot_rect->get_top(false));
            }
            else
            {
               domain_axis_rect->set_right(plot_rect->get_left(false));
               left_axis_rect->set_bottom(plot_rect->get_top(false));
               right_axis_rect->set_top(plot_rect->get_bottom(false));
            }
            layout->add_child(domain_axis_rect);
            layout->add_child(left_axis_rect);
            layout->add_child(right_axis_rect);
         }

         switch (legend_pos)
         {
         case legend_right:
         case legend_left:
            if (legend_rect->get_height() < plot_rect->get_height())
               legend_rect->centre_y(plot_rect->get_centre(false).y);
            break;
         case legend_top:
         case legend_bottom:
            if (legend_rect->get_width() < plot_rect->get_width())
               legend_rect->centre_x(plot_rect->get_centre(false).x);
            break;
         }

      } // calculate_positions


      void Graph::draw_background()
      {
         driver->draw_rect(
            layout->get_rect(true),
            background_pen,
            background_brush);
      } // draw_background

      
      void Graph::draw_title()
      {
         if(title.length() > 0 && title_font != 0)
         {
            Colour background(transparent_colour());
            driver->draw_text(
               title,
               title_rect->get_rect(true),
               title_font,
               title_colour,
               transparent_colour());
         }
      } // draw_title


      void Graph::draw_legend_background()
      {
         if(legend_rect != 0)
         {
            driver->draw_rect(
               legend_rect->get_rect(true),
               legend_pen,
               legend_brush);
         }
      } // draw_legend_background

      
      void Graph::draw_legend()
      {
         // we will do nothing if the legend rectangle has not been allocated.
         if(legend_rect != 0)
         {
            draw_legend_background();
            for(iterator ti = begin(); ti != end(); ++ti)
            {
               trace_handle &trace(*ti);
               trace->draw_legend(
                  legend_font,
                  legend_text_colour,
                  transparent_colour());
            }
         }
      } // draw_legend


      void Graph::draw_plot_area_background()
      {
         driver->draw_rect(
            plot_rect->get_rect(true),
            plot_area_pen,
            plot_area_brush);
      } // draw_plot_area_background


      void Graph::draw_plot_area()
      {
         draw_plot_area_background();
         bottom_axis->draw();
         left_axis->draw(true);
         right_axis->draw(true);
         driver->set_clipping_region(plot_rect->get_rect(true));
         for(iterator ti = begin(); ti != end(); ++ti)
         {
            trace_handle &trace(*ti);
            if(trace->get_visible())
               trace->draw();
         }
         driver->remove_clipping_region();
      } // draw_plot_area


      void Graph::draw_decorations()
      {
         // draw any marks
         for(iterator ti = begin(); ti != end(); ++ti)
         {
            trace_handle &trace(*ti);
            if(trace->get_visible())
               trace->draw_marks();
         }

         // we need to draw the restore button if the state has been backed up.
         if(state_backup != 0)
         {
            StrUni restore(my_strings[strid_restore]);
            if(restore_font == 0)
            {
               restore_font = driver->make_font(driver->default_font_desc());
               restore_pen = driver->make_pen(
                  PenInfo().set_width(2).set_colour(black_colour()));
               restore_brush = driver->make_brush(
                  BrushInfo().set_colour(milk_white_colour()));
            }
            restore_button_rect = driver->measure_text(restore, restore_font);
            restore_button_rect.width += 6;
            restore_button_rect.height += 2;
            restore_button_rect.move(Point(layout->get_left() + 5, layout->get_top() + 5));
            driver->draw_rect(restore_button_rect, restore_pen, restore_brush);
            driver->draw_text(
               restore,
               restore_button_rect,
               restore_font,
               black_colour(),
               milk_white_colour());
         }

         // draw the zoom rectangle
         if(mouse_state == mouse_zoom_rect)
         {
            // we need to draw the zoom rectangle
            double height(fabs(mouse_current_pos.y - mouse_start_pos.y));
            pen_handle pen(
               driver->make_pen(
                  PenInfo().set_colour(black_colour().set_alpha(127))));
            driver->draw_line(
               mouse_start_pos,
               Point(mouse_current_pos.x, mouse_start_pos.y),
               pen);
            driver->draw_line(
               Point(mouse_current_pos.x, mouse_start_pos.y),
               mouse_current_pos,
               pen);
            driver->draw_line(
               mouse_current_pos,
               Point(mouse_start_pos.x, mouse_current_pos.y),
               pen);
            driver->draw_line(
               Point(mouse_start_pos.x, mouse_current_pos.y),
               mouse_start_pos,
               pen);
         }
      } // draw_decorations


      bool Graph::on_left_mouse_down(Point const &pos)
      {
         bool rtn(false);
         Rect test_rect(plot_rect->get_rect(true));
         if(mouse_state == mouse_idle && state_backup != 0 && restore_button_rect.within(pos))
         {
            mouse_state = mouse_restore_down;
            rtn = true;
         }
         else if(mouse_state == mouse_idle && test_rect.within(pos))
         {
            start_mouse_capture();
            mouse_state = mouse_left_down;
            mouse_start_pos = pos;
            rtn = true;
         }
         return rtn;
      } // on_left_mouse_down


      bool Graph::on_left_mouse_up(Point const &pos)
      {
         bool rtn(false);
         if(mouse_state == mouse_left_down)
         {
            // we will interpret this as a left click event and will search for the nearest data
            // point.  We will do this by searching for the trace point that lies closest to the
            // start position.  If this is not found, we will search with an ever increasing radius.
            int try_count(1);
            GraphPoint closest_mark;
            value_type closest_trace;
            double closest_distance(std::numeric_limits<double>::max());
            for(iterator ti = begin(); ti != end(); ++ti)
               (*ti)->clear_marks(true);
            while(closest_trace == 0 && try_count <= 10)
            {
               for(iterator ti = begin(); ti != end(); ++ti)
               {
                  trace_handle &trace(*ti);
                  if (trace->get_visible() &&
                     trace->get_marks_enabled() && (trace->get_marks_show_x_value() || trace->get_marks_show_y_value()))
                  {
                     double radius(try_count * 5.0);
                     GraphTrace::hit_test_value rcd(trace->hit_test(pos, radius));
                     if (rcd.second <= closest_distance && rcd.second < radius)
                     {
                        closest_trace = trace;
                        closest_mark = rcd.first;
                        closest_distance = rcd.second;
                     }
                  }
               }
               ++try_count;
            }
            if(closest_trace != 0)
               closest_trace->set_current_mark(closest_mark);
            mouse_state = mouse_idle;
            release_mouse_capture();
            do_refresh();
         }
         else if(mouse_state == mouse_zoom_rect)
         {
            // we need set the bounds of the graph to the current zoom rectangle.
            Rect zoom_rect(mouse_start_pos, mouse_current_pos);
            backup_settings();
            if(!bottom_axis->empty())
            {
               GraphAxis::value_handle new_min;
               GraphAxis::value_handle new_max;

               if (rotation == rotation_none)
               {
                  new_min = bottom_axis->pos_to_value(zoom_rect.get_left());
                  new_max = bottom_axis->pos_to_value(zoom_rect.get_right());
               }
               else
               {
                  new_min = bottom_axis->pos_to_value(zoom_rect.get_top());
                  new_max = bottom_axis->pos_to_value(zoom_rect.get_bottom());
               }
               if (bottom_axis->get_inverted())
               {
                  std::swap(new_min, new_max);
               }
               bottom_axis->set_time_format("");
               bottom_axis->set_min_value(new_min);
               bottom_axis->set_max_value(new_max);
               bottom_axis->set_auto_time(false);
               bottom_axis->set_fixed_decimals(false);
               positions_invalid = true;
            }
            if(!left_axis->empty())
            {
               GraphAxis::value_handle new_min;
               GraphAxis::value_handle new_max;

               if (rotation == rotation_none)
               {
                  new_min = left_axis->pos_to_value(zoom_rect.get_bottom());
                  new_max = left_axis->pos_to_value(zoom_rect.get_top());
               }
               else
               {
                  new_min = left_axis->pos_to_value(zoom_rect.get_left());
                  new_max = left_axis->pos_to_value(zoom_rect.get_right());
               }
               if (left_axis->get_inverted())
               {
                  std::swap(new_min, new_max);
               }

               left_axis->set_min_value(new_min);
               left_axis->set_max_value(new_max);
               left_axis->set_fixed_decimals(false);
               positions_invalid = true;
            }
            if(!right_axis->empty())
            {
               GraphAxis::value_handle new_min;
               GraphAxis::value_handle new_max;

               if (rotation == rotation_none)
               {
                  new_min = right_axis->pos_to_value(zoom_rect.get_bottom());
                  new_max = right_axis->pos_to_value(zoom_rect.get_top());
               }
               else
               {
                  new_min = right_axis->pos_to_value(zoom_rect.get_left());
                  new_max = right_axis->pos_to_value(zoom_rect.get_right());
               }
               if (right_axis->get_inverted())
               {
                  std::swap(new_min, new_max);
               }

               right_axis->set_min_value(new_min);
               right_axis->set_max_value(new_max);
               right_axis->set_fixed_decimals(false);
               positions_invalid = true;
            }
            
            // we can now release the mouse and redraw.
            mouse_state = mouse_idle;
            release_mouse_capture();
            do_refresh();
         }
         else if(mouse_state == mouse_restore_down)
         {
            restore_settings();
            mouse_state = mouse_idle;
            do_refresh();
         }
         return rtn;
      } // on_left_mouse_up


      bool Graph::on_right_mouse_down(Point const &pos)
      {
         bool rtn(false);
         if(mouse_state == mouse_idle)
         {
            rtn = true;
            mouse_state = mouse_right_down;
            mouse_start_pos = pos;
            start_mouse_capture();
         }
         return rtn;
      } // on_right_mouse_down


      bool Graph::on_right_mouse_up(Point const &pos)
      {
         bool rtn(false);
         if(mouse_state == mouse_right_down)
         {
            rtn = true;
            mouse_state = mouse_idle;
            release_mouse_capture();
         }
         return rtn;
      } // on_right_mouse_up


      bool Graph::on_mouse_move(Point const &pos)
      {
         bool rtn(false);
         mouse_current_pos = pos;
         if(mouse_state == mouse_left_down)
         {
            // we need to evaluate the distance of this point from the position when the left button
            // was clicked.  If greater than 10 pixels away, we will assume that the user is
            // dragging a zoom rectangle.
            double distance(pos.distance(mouse_start_pos));
            if(distance > 10)
            {
               mouse_state = mouse_zoom_rect;
               do_refresh();
            }
            rtn = true;
         }
         else if(mouse_state == mouse_zoom_rect)
         {
            do_refresh();
            rtn = true;
         }
         else if(mouse_state == mouse_right_down)
         {
            // the user is dragging the mouse with the right button down.  We will use this to pan
            // the graph in the direction of the drag between the current position and the position
            // when the right button was clicked.
            backup_settings();

            // If the axis is rotated, then the bottom axis needs to look a the 
            // difference in the Y mouse position, and the left and right axis needs 
            // to look at differenes in X mouse position.
            Point mouse_start_pos_ = mouse_start_pos;
            Point mouse_current_pos_ = mouse_current_pos;

            if (rotation != rotation_none)
            {
               mouse_start_pos_.x = mouse_start_pos.y;
               mouse_start_pos_.y = mouse_start_pos.x;

               mouse_current_pos_.x = mouse_current_pos.y;
               mouse_current_pos_.y = mouse_current_pos.x;
            }



            if(!bottom_axis->empty())
            {
               GraphAxis::value_handle min_val(bottom_axis->get_scale()->min_value);
               GraphAxis::value_handle max_val(bottom_axis->get_scale()->max_value);
               GraphAxis::value_handle start_x(bottom_axis->pos_to_value(mouse_start_pos_.x));
               GraphAxis::value_handle end_x(bottom_axis->pos_to_value(mouse_current_pos_.x));
               double dx(end_x->get_val() - start_x->get_val());
               min_val->set_val(min_val->get_val() - dx, min_val->get_timestamp());
               max_val->set_val(max_val->get_val() - dx, max_val->get_timestamp());
               bottom_axis->set_min_value(min_val);
               bottom_axis->set_max_value(max_val);
               bottom_axis->set_max_offset(0).set_min_offset(0);
               bottom_axis->set_auto_time(false);
            }
            if(!left_axis->empty())
            {
               GraphAxis::value_handle min_val(left_axis->get_scale()->min_value);
               GraphAxis::value_handle max_val(left_axis->get_scale()->max_value);
               GraphAxis::value_handle start_y(left_axis->pos_to_value(mouse_start_pos_.y));
               GraphAxis::value_handle end_y(left_axis->pos_to_value(mouse_current_pos_.y));
               double dy(end_y->get_val() - start_y->get_val());
               min_val->set_val(min_val->get_val() - dy, min_val->get_timestamp());
               max_val->set_val(max_val->get_val() - dy, max_val->get_timestamp());
               left_axis->set_max_offset(0).set_min_offset(0);
               left_axis->set_min_value(min_val).set_max_value(max_val);
            }
            if(!right_axis->empty())
            {
               GraphAxis::value_handle min_val(right_axis->get_scale()->min_value);
               GraphAxis::value_handle max_val(right_axis->get_scale()->max_value);
               GraphAxis::value_handle start_y(right_axis->pos_to_value(mouse_start_pos_.y));
               GraphAxis::value_handle end_y(right_axis->pos_to_value(mouse_current_pos_.y));
               double dy(end_y->get_val() - start_y->get_val());
               min_val->set_val(min_val->get_val() - dy, min_val->get_timestamp());
               max_val->set_val(max_val->get_val() - dy, max_val->get_timestamp());
               right_axis->set_min_value(min_val).set_max_value(max_val);
               right_axis->set_min_offset(0).set_max_offset(0);
            }
            positions_invalid = true;
            rtn = true;
            mouse_start_pos = pos;
            do_refresh();
         }
         return rtn;
      } // on_mouse_move


      bool Graph::on_mouse_wheel(Point const &pos, double rotation, double wheel_delta)
      {
         bool rtn(false);
         return rtn;
      } // on_mouse_wheel


      void Graph::backup_settings()
      {
         if(state_backup == 0)
         {
            state_backup.bind(new GraphState);
            state_backup->bottom_state = bottom_axis->get_state();
            state_backup->left_state = left_axis->get_state();
            state_backup->right_state = right_axis->get_state();
         }
      } // backup_settings


      void Graph::restore_settings()
      {
         if(state_backup != 0)
         {
            bottom_axis->set_state(*state_backup->bottom_state);
            left_axis->set_state(*state_backup->left_state);
            right_axis->set_state(*state_backup->right_state);
            state_backup.clear();
            positions_invalid = true;
            do_refresh();
         }
      } // restore_settings
   };
};

