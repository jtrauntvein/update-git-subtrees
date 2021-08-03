/* Csi.Graphics.GraphTrace.cpp

   Copyright (C) 2015, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 28 January 2015
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2020-07-14 15:42:11 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#define NOMINMAX
#include "Csi.Graphics.GraphTrace.h"
#include "Csi.Graphics.Graph.h"
#include <algorithm>
#include <cmath>
#include <iterator>


namespace Csi
{
   namespace
   {
      double const min_legend_symbol_width(30);
      double const legend_horizontal_margin(2);
      double const legend_vertical_margin(3);
      double const mark_margin(2);
      double const mark_distance(25);
   };

   
   namespace Graphics
   {
      GraphTrace::GraphTrace(driver_handle driver_):
         graph(0),
         driver(driver_),
         domain_axis(0),
         range_axis(0)
      {
         set_default_properties();
      } // constructor


      void GraphTrace::set_default_properties()
      {
         set_vertical_axis(range_left);
         trace_type = trace_line;
         use_stairs = false;
         visible = true;
         point_colour = red_colour();
         point_pen   = driver->make_pen(PenInfo().set_colour(point_colour));
         point_brush = driver->make_brush(BrushInfo().set_colour(point_colour));
         set_point_size(2);
         time_offset = 0;
         bar_position = 1;
         pen = driver->make_pen(PenInfo().set_colour(red_colour()).set_width(1));
         bar_pen = driver->make_pen(PenInfo());
         bar_down_pen = bar_pen;
         bar_up_brush = driver->make_brush(BrushInfo().set_colour(red_colour()));
         bar_down_brush = bar_up_brush;
         bar_show_two_legend_items = false;
         mark_props.enabled = true;
         mark_props.always_show = false;
         mark_props.decimation = 1;
         mark_props.show_x = mark_props.show_y = true;
         mark_props.time_format = "%Y-%m-%d %H:%M:%S%x";
         mark_props.font = driver->make_font(driver->default_font_desc().set_point_size(8));
         mark_props.transparent_back = false;
         mark_props.colour = red_colour(); // synch this to the pen colour
         mark_props.rounded_back = true;
         colours_controller.clear();

         thresholds.enable_high = false;
         thresholds.enable_low  = false;
         thresholds.high_value = 0;
         thresholds.low_value  = 0;
         thresholds.high_pen = pen;
         thresholds.low_pen  = pen;
         thresholds.high_point_pen    = point_pen;
         thresholds.high_point_brush  = point_brush;
         thresholds.low_point_pen     = point_pen;
         thresholds.low_point_brush   = point_brush;
         thresholds.high_point_colour = red_colour();
         thresholds.low_point_colour  = red_colour();
         //thresholds.high_point_size   = point_size;
         //thresholds.low_point_size    = point_size;
         thresholds.show_high_in_legend = true;
         thresholds.show_low_in_legend  = true;
         thresholds.high_data_exists = false;
         thresholds.low_data_exists  = false;
         thresholds.high_legend_text = L"";
         thresholds.low_legend_text  = L"";

         reference_line = 0;
         reference_dot = 0;

      } // set_default_properties

      
      void GraphTrace::write(Xml::Element &elem)
      {
         elem.set_attr_uint4(vertical_axis, L"vertical-axis");
         elem.set_attr_uint4(trace_type, L"trace-type");
         elem.set_attr_wstr(title, L"title");
         elem.set_attr_bool(use_stairs, L"use-stairs");
         if(point_type != 0)
            elem.set_attr_uint4(point_type->get_symbol_type(), L"symbol-type");
         else
            elem.set_attr_uint4(ShapeBase::symbol_nothing, L"symbol-type");
         elem.set_attr_colour(point_colour, L"point-colour");
         elem.set_attr_double(point_size, L"point-size");
         elem.set_attr_int8(time_offset, L"time-offset");
         elem.set_attr_bool(visible, L"visible");
         if(pen != 0)
         {
            Xml::Element::value_type pen_xml(elem.add_element(L"pen"));
            pen->get_desc().write(*pen_xml);
         }
         if(mark_props.font != 0)
         {
            Xml::Element::value_type font_xml(elem.add_element(L"marks-font"));
            mark_props.font->get_desc().write(*font_xml);
         }
         if(bar_pen != 0)
         {
            Xml::Element::value_type pen_xml(elem.add_element(L"bar-pen"));
            bar_pen->get_desc().write(*pen_xml);
         }
         if(bar_down_pen != 0)
         {
            Xml::Element::value_type pen_xml(elem.add_element(L"bar-down-pen"));
            bar_down_pen->get_desc().write(*pen_xml);
         }
         if(bar_up_brush != 0)
         {
            Xml::Element::value_type brush_xml(elem.add_element(L"bar-up-brush"));
            bar_up_brush->get_desc().write(*brush_xml);
         }
         if(bar_down_brush != 0)
         {
            Xml::Element::value_type brush_xml(elem.add_element(L"bar-down-brush"));
            bar_down_brush->get_desc().write(*brush_xml);
         }
         elem.set_attr_int4(bar_position, L"bar-position");


         Xml::Element::value_type marks_elem = elem.add_element(L"marks");
         marks_elem->add_element(L"enable")->set_cdata_bool(mark_props.enabled);
         marks_elem->add_element(L"always_show")->set_cdata_bool(mark_props.always_show);
         marks_elem->add_element(L"decimation")->set_cdata_int4(mark_props.decimation);
         marks_elem->add_element(L"show_x")->set_cdata_bool(mark_props.show_x);
         marks_elem->add_element(L"show_y")->set_cdata_bool(mark_props.show_y);
         marks_elem->add_element(L"units_x")->set_cdata_wstr(mark_props.x_units);
         marks_elem->add_element(L"units_y")->set_cdata_wstr(mark_props.y_units);
         marks_elem->add_element(L"time_format")->set_cdata_wstr(mark_props.time_format);
         marks_elem->add_element(L"transparent")->set_cdata_bool(mark_props.transparent_back);
         marks_elem->add_element(L"rounded_back")->set_cdata_bool(mark_props.rounded_back);
         marks_elem->set_attr_colour(mark_props.colour, L"colour");


         Xml::Element::value_type thresh_elem = elem.add_element(L"thresholds");

         thresh_elem->add_element(L"enable_high")->set_cdata_bool(thresholds.enable_high);
         thresh_elem->add_element(L"enable_low")->set_cdata_bool(thresholds.enable_low);

         thresh_elem->add_element(L"high_value")->set_cdata_double(thresholds.high_value);
         thresh_elem->add_element(L"low_value")->set_cdata_double(thresholds.low_value);
         {
            Xml::Element::value_type pen_xml(thresh_elem->add_element(L"high_pen"));
            thresholds.high_pen->get_desc().write(*pen_xml);
         }
         {
            Xml::Element::value_type pen_xml(thresh_elem->add_element(L"low_pen"));
            thresholds.low_pen->get_desc().write(*pen_xml);
         }
         thresh_elem->set_attr_colour(thresholds.high_point_colour, L"high_point_colour");
         thresh_elem->set_attr_colour(thresholds.low_point_colour,  L"low_point_colour");

         thresh_elem->set_attr_colour(thresholds.high_age_point_colour, L"high_point_age_colour");
         thresh_elem->set_attr_colour(thresholds.low_age_point_colour,  L"low_point_age_colour");
 
         //thresh_elem->set_attr_double(thresholds.high_point_size,   L"high-point-size");
         //thresh_elem->set_attr_double(thresholds.low_point_size,    L"low-point-size");

         thresh_elem->set_attr_bool(thresholds.show_high_in_legend, L"high-show-in_legend");
         thresh_elem->set_attr_bool(thresholds.show_low_in_legend,  L"low-show-in_legend");
         thresh_elem->set_attr_wstr(thresholds.high_legend_text,    L"high-legend_text");
         thresh_elem->set_attr_wstr(thresholds.low_legend_text,     L"low-legend-text");

         thresh_elem->set_attr_bool(bar_show_two_legend_items,      L"bar-separate-legend");

      } // write


      void GraphTrace::read(Xml::Element &elem)
      {
         set_vertical_axis(static_cast<vertical_axis_type>(elem.get_attr_uint4(L"vertical-axis")));
         set_trace_type(static_cast<trace_type_code>(elem.get_attr_uint4(L"trace-type")));
         title = elem.get_attr_wstr(L"title");
         use_stairs = elem.get_attr_bool(L"use-stairs");
         set_point_type(static_cast<ShapeBase::symbol_type>(elem.get_attr_uint4(L"symbol-type")));
         set_point_colour(elem.get_attr_colour(L"point-colour"));
         set_point_size(elem.get_attr_double(L"point-size"));
         time_offset = elem.get_attr_int8(L"time-offset");
         bar_position = elem.get_attr_int4(L"bar-position");
         if(elem.has_attribute(L"visible"))
            set_visible(elem.get_attr_bool(L"visible"));
         else
            set_visible(true);
         pen.clear();
         mark_props.font.clear();
         bar_pen.clear();
         bar_down_pen.clear();
         bar_up_brush.clear();
         bar_down_brush.clear();

         for(Xml::Element::iterator ci = elem.begin(); ci != elem.end(); ++ci)
         {
            Xml::Element::value_type &child(*ci);
            if(child->get_name() == L"pen")
            {
               PenInfo info;
               info.read(*child);
               pen = driver->make_pen(info);
            }
            else if(child->get_name() == L"marks-font")
            {
               FontInfo info(driver->default_font_desc());
               info.read(*child);
               mark_props.font = driver->make_font(info);
            }
            else if(child->get_name() == L"bar-pen")
            {
               PenInfo info;
               info.read(*child);
               bar_pen = driver->make_pen(info);
               bar_down_pen = bar_pen;
            }
            else if (child->get_name() == L"bar-down-pen")
            {
               PenInfo info;
               info.read(*child);
               bar_down_pen = driver->make_pen(info);
            }
            else if(child->get_name() == L"bar-up-brush")
            {
               BrushInfo info;
               info.read(*child);
               bar_up_brush = driver->make_brush(info);
            }
            else if(child->get_name() == L"bar-down-brush")
            {
               BrushInfo info;
               info.read(*child);
               bar_down_brush = driver->make_brush(info);
            }

            else if (child->get_name() == L"marks")
            {
               try
               {
                  set_marks_enabled(child->find_elem(L"enable")->get_cdata_bool());
                  set_marks_always_show(child->find_elem(L"always_show")->get_cdata_bool());
                  set_marks_decimation(child->find_elem(L"decimation")->get_cdata_int4());
                  set_marks_show_x_value(child->find_elem(L"show_x")->get_cdata_bool());
                  set_marks_show_y_value(child->find_elem(L"show_y")->get_cdata_bool());
                  set_marks_time_format(child->find_elem(L"time_format")->get_cdata_wstr());
                  set_marks_y_units(child->find_elem(L"units_y")->get_cdata_str());
                  set_marks_x_units(child->find_elem(L"units_x")->get_cdata_str());

               }
               catch (std::exception&)
               { }

               try
               {
                  set_marks_transparent_back(child->find_elem(L"transparent")->get_cdata_bool());
                  set_marks_colour(child->get_attr_colour(L"colour"));
                  set_marks_rounded_back(child->find_elem(L"rounded_back")->get_cdata_bool());
               }
               catch (std::exception&)
               { 
                  if (pen != nullptr)
                  {
                     set_marks_colour(pen->get_colour());
                     if (!pen->get_visible())
                        set_marks_colour(point_colour);
                  }
               }
            }

            else if (child->get_name() == L"thresholds")
            {
               try
               {
                  set_enable_high_threshold(child->find_elem(L"enable_high")->get_cdata_bool());
                  set_enable_low_threshold (child->find_elem(L"enable_low")->get_cdata_bool());

                  set_high_threshold(child->find_elem(L"high_value")->get_cdata_double());
                  set_low_threshold (child->find_elem(L"low_value")->get_cdata_double());

                  Xml::Element::value_type high_color = child->find_elem(L"high_pen");
                  PenInfo info;
                  info.read(*high_color);
                  thresholds.high_pen = driver->make_pen(info);

                  Xml::Element::value_type low_color = child->find_elem(L"low_pen");
                  info.read(*low_color);
                  thresholds.low_pen = driver->make_pen(info);

                  set_high_threshold_point_colour(child->get_attr_colour(L"high_point_colour"));
                  set_low_threshold_point_colour(child->get_attr_colour(L"low_point_colour"));

                  //set_high_point_size(child->get_attr_double(L"high-point-size"));
                  //set_low_point_size(child->get_attr_double(L"low-point-size"));

                  set_show_high_in_legend(child->get_attr_bool(L"high-show-in_legend"));
                  set_show_low_in_legend(child->get_attr_bool(L"low-show-in_legend"));
                  set_legend_high_text(child->get_attr_wstr(L"high-legend_text"));
                  set_legend_low_text(child->get_attr_wstr(L"low-legend-text"));

                  set_bar_separate_legend(child->get_attr_bool(L"bar-separate-legend"));

                  set_high_threshold_age_point_colour(child->get_attr_colour(L"high_point_age_colour"));
                  set_low_threshold_age_point_colour(child->get_attr_colour(L"low_point_age_colour"));
               } 
               catch (std::exception&)
               { }
            }
         }
      } // read


      GraphTrace &GraphTrace::set_vertical_axis(vertical_axis_type axis)
      {
         if(axis != vertical_axis)
         {
            vertical_axis = axis;
            if(graph)
               graph->on_trace_axis_changed(this);
         }
         return *this;
      } // set_vertical_axis


      GraphTrace &GraphTrace::set_visible(bool value_)
      {
         visible = value_;
         if(graph)
            graph->on_trace_axis_changed(this);
         return *this;
      }

      GraphTrace::bounds_type GraphTrace::get_bounds(bool for_domain) const
      {
         bounds_type rtn(
            std::make_pair(
               std::numeric_limits<double>::infinity(),
               -std::numeric_limits<double>::infinity()));
         for(const_iterator pi = begin(); visible && pi != end(); ++pi)
         {
            value_type const &point(*pi);
            double x(point.x->get_val());
            double y(point.y->get_val());
            if(is_finite(x) && is_finite(y))
            {
               if(for_domain)
               {
                  rtn.first = csimin(rtn.first, point.x->get_val());
                  rtn.second = csimax(rtn.second, point.x->get_val());
               }
               else
               {
                  rtn.first = csimin(rtn.first, point.y->get_val());
                  rtn.second = csimax(rtn.second, point.y->get_val());
               }
            }
         }
         if(!for_domain)
         {
            if ((trace_type == trace_bar) || (trace_type== trace_area))
            {
               if (rtn.first > 0)
                  rtn.first = 0;
               if (rtn.second < 0)
                  rtn.second = 0;
            }
         }
         return rtn;
      } // get_bounds


      namespace
      {
         struct time_domain_less
         {
            bool operator ()(GraphPoint const &p1, GraphPoint const &p2) const
            {
               return p1.x->get_val_int() < p2.x->get_val_int();
            }
         };
      };


      void GraphTrace::erase(iterator start, iterator end)
      {
         for(iterator pi = start; pi != end; ++pi)
            pi->owner = 0;
         points.erase(start, end);
      } // erase
      

      GraphTrace::value_type GraphTrace::add_point(value_handle x, value_handle y)
      {
         GraphPoint new_point(x, y, this);
         if(time_offset != 0 && domain_axis->get_time_domain())
         {
            new_point.x->set_val(
               x->get_val_int() + time_offset,
               x->get_timestamp() + time_offset * LgrDate::nsecPerMSec);
         }
         if(!points.empty() && domain_axis->get_time_domain())
         {
            // we need to keep the points sorted by their time stamps.
            value_type const &last(back());
            if(last.x->get_val_int() < x->get_val_int())
               points.push_back(GraphPoint(x, y, this));
            else
            {
               iterator insert_pos(
                  std::upper_bound(begin(), end(), new_point, time_domain_less()));
               points.insert(insert_pos, new_point);
            }
         }
         else
            points.push_back(new_point);
         if(graph)
            graph->positions_invalid = true;
         return new_point;
      } // add_point

      GraphTrace::value_type GraphTrace::add_point(value_handle x, value_handle y, value_handle wind_dir)
      {
         GraphPoint new_point(x, y, wind_dir, this);

         if(time_offset != 0 && domain_axis->get_time_domain())
         {
            new_point.x->set_val(
               x->get_val_int() + time_offset,
               x->get_timestamp() + time_offset * LgrDate::nsecPerMSec);
         }

         if(!points.empty() && domain_axis->get_time_domain())
         {
            // we need to keep the points sorted by their time stamps.
            value_type const &last(back());
            if(last.x->get_val_int() < x->get_val_int())
               points.push_back(new_point);
            else
            {
               iterator insert_pos(
                  std::upper_bound(begin(), end(), new_point, time_domain_less()));
               points.insert(insert_pos, new_point);
            }
         }
         else
            points.push_back(new_point);
         
         if (graph)
            graph->positions_invalid = true;
         return new_point;
      } // add_point


      void GraphTrace::remove_older_points(Csi::LgrDate const &cutoff)
      {
         // since the points are already sorted by their time stamp, any points removed will be at
         // the beginning of the container. 
         iterator remove_begin = begin();
         iterator remove_end = begin();
         for(iterator pi = begin(); pi != end(); ++pi)
         {
            if(pi->x->get_timestamp() < cutoff)
               remove_end = pi + 1;
            else
               break;
         }
         if(remove_begin != remove_end)
            points.erase(remove_begin, remove_end);
      } // remove_older_points


      void GraphTrace::remove_older_points(size_t max_num_points)
      {
         if (points.size() > max_num_points)
         {
            iterator remove_begin = begin();
            iterator remove_end = end() - max_num_points;
            points.erase(remove_begin, remove_end);
         }
      }

      GraphTrace::rect_handle GraphTrace::make_legend_rect(StrUni &title, font_handle &legend_font, rect_handle &text_rect, rect_handle &symbol_rect)
      {
         rect_handle rtn;
         rtn.bind(new NestedRect);

         if(title.length() > 0)
         {
            const double legend_point_size = 5;  // Make all legend points the same size

            rect_handle middle_rect(new NestedRect);

            text_rect.bind(new NestedRect(driver->measure_text(title, legend_font)));

            if(point_type != nullptr)
               symbol_rect.bind(new NestedRect(point_type->get_size(legend_point_size)));
            else
               symbol_rect.bind(new NestedRect);

            if(symbol_rect->get_width() < min_legend_symbol_width)
               symbol_rect->set_width(min_legend_symbol_width);

            rtn->add_child(new NestedRect(legend_horizontal_margin, legend_vertical_margin));

            middle_rect->add_child(new NestedRect(legend_horizontal_margin, legend_vertical_margin));
            middle_rect->add_child(symbol_rect);
            middle_rect->add_child(new NestedRect(legend_horizontal_margin, legend_vertical_margin));
            middle_rect->add_child(text_rect);
            middle_rect->add_child(new NestedRect(legend_horizontal_margin, legend_vertical_margin));
            middle_rect->stack_horizontal();
            if(text_rect->get_height() > symbol_rect->get_height())
               symbol_rect->centre_y(text_rect->get_centre().y);
            else
               text_rect->centre_y(symbol_rect->get_centre().y);

            rtn->add_child(middle_rect);
            rtn->add_child(new NestedRect(legend_horizontal_margin, legend_vertical_margin));
            rtn->stack_vertical();
         }
         return rtn;
      } // make_legend_rect

      void GraphTrace::make_legend_rect(rect_handle &legend_rect, font_handle &legend_font)
      {
         rect_handle rect1 = 
            make_legend_rect(title, 
               legend_font, 
               legend_pos.text_rect, 
               legend_pos.symbol_rect);

         if (rect1 != nullptr && rect1->get_width() > 0)
            legend_rect->add_child(rect1);


         bool show_high_in_legend = false;
         bool show_low_in_legend  = false;
         get_show_thresholds_in_legend(show_high_in_legend, show_low_in_legend);

         if (show_high_in_legend)
         {
            rect_handle rect2 = 
               make_legend_rect(thresholds.high_legend_text, 
                  legend_font, 
                  legend_pos.text_high_rect, 
                  legend_pos.symbol_high_rect);

            if (rect2 != nullptr && rect2->get_width() > 0)
               legend_rect->add_child(rect2);
         }


         if (show_low_in_legend)
         {
            rect_handle rect3 = 
               make_legend_rect(thresholds.low_legend_text, legend_font,
                  legend_pos.text_low_rect, legend_pos.symbol_low_rect);
            if (rect3 != nullptr && rect3->get_width() > 0)
               legend_rect->add_child(rect3);
         }
      }


      void GraphTrace::get_show_thresholds_in_legend(bool &show_high, bool &show_low)
      {

         show_high = false;
         show_low = false;;

         switch (trace_type)
         {
         case trace_line:
         case trace_point:
         case trace_wind_dir:
         {
            bool high_data_exists = thresholds.high_data_exists || points.empty();
            bool low_data_exists  = thresholds.low_data_exists || points.empty();

            show_high = (thresholds.enable_high &&
               thresholds.show_high_in_legend &&
               high_data_exists);

            show_low = (thresholds.enable_low &&
               thresholds.show_low_in_legend &&
               low_data_exists);

            break;
         }

         case trace_bar:
         case trace_area:
         {
            show_high = false;
            show_low = bar_show_two_legend_items;
            break;
         }

         default:
         {
            show_high = false;
            show_low = false;
            break;
         }
         }
      }

      void GraphTrace::draw_legend(
         StrUni &title,
         rect_handle &text_rect,
         rect_handle &symbol_rect,
         pen_handle &pen,
         Colour &point_colour,
         pen_handle &bar_pen,
         brush_handle &bar_brush,
         font_handle &legend_font,
         Colour const &foreground,
         Colour const &background)
      {
         Point centre(symbol_rect->get_centre());
         double width = symbol_rect->get_width() * .85f;
         double left = centre.x - width / 2.f;
         double right = centre.x + width / 2.f;

         // Force the legend line width's to to be one pixel wide, and 5 pixels tall
         PenInfo pen_info = pen->get_desc();
         pen_handle pen_legend = driver->make_pen(pen_info.set_width(1));
         const double legend_point_size = 5;

         Point point_left(left, centre.y);
         Point point_right(right, centre.y);


         if (trace_type == trace_reference_line ||
            trace_type == trace_vertical_reference_line)
         {
            driver->draw_line(point_left, point_right, pen_legend);
         }

         else if (trace_type == trace_line)
         {
            driver->draw_line(point_left, point_right, pen_legend);

            if (point_type != nullptr)
               point_type->draw(*driver, centre, point_colour, legend_point_size);
         }

         else if (trace_type == trace_bar || trace_type == trace_area)
         {
            Rect legend_bar_rect = symbol_rect->get_rect();
            legend_bar_rect.set_left(left);
            legend_bar_rect.width = width;
            legend_bar_rect.set_top(legend_bar_rect.get_centre_y() - 2.5);
            legend_bar_rect.height = 5;

            bool one_color = bar_up_brush->get_desc().get_fill().front() == bar_down_brush->get_desc().get_fill().front();
            if (one_color || bar_show_two_legend_items)
            {
               driver->draw_rect(
                  legend_bar_rect,
                  bar_pen,
                  bar_brush);
            }
            else
            {
               legend_bar_rect.set_left(centre.x);
               legend_bar_rect.width = width / 2;

               driver->draw_rect(
                  legend_bar_rect,
                  nullptr,
                  bar_down_brush);

               legend_bar_rect.set_left(left);
               driver->draw_rect(
                  legend_bar_rect,
                  nullptr,
                  bar_up_brush);


               legend_bar_rect.width = width;
               driver->draw_rect(
                  legend_bar_rect,
                  bar_pen,
                  nullptr);

            }
         }
         else if (trace_type == trace_point || trace_type == trace_reference_dot)
         {
            if (point_type != nullptr)
               point_type->draw(*driver, centre, point_colour, legend_point_size);
         }
         else if (trace_type == trace_wind_dir)
         {
            driver->draw_line(point_left, point_right, pen_legend);

            pen_handle pen_arrow = driver->make_pen(PenInfo().set_colour(point_colour));
            driver->draw_line(
               Point(centre.x - 4, centre.y + 4),
               Point(centre.x + 4, centre.y - 4),
               pen_arrow);

            driver->draw_line(
               Point(centre.x + 4, centre.y - 4),
               Point(centre.x - 1, centre.y - 4),
               pen_arrow);

            driver->draw_line(
               Point(centre.x + 4, centre.y - 4),
               Point(centre.x + 4, centre.y + 1),
               pen_arrow);

         }

         driver->draw_text(
            title,
            text_rect->get_rect(true),
            legend_font,
            foreground,
            background);

      } // draw_legend


      void GraphTrace::draw_legend(
         font_handle &legend_font,
         Colour const &foreground,
         Colour const &background)
      {

         if(visible && 
            legend_pos.text_rect != nullptr && 
            legend_pos.symbol_rect != nullptr)
         {
            draw_legend(title,
               legend_pos.text_rect,
               legend_pos.symbol_rect,
               pen,
               point_colour,
               bar_pen,
               bar_up_brush,
               legend_font, foreground, background);


            bool show_high_in_legend = false;
            bool show_low_in_legend  = false;
            get_show_thresholds_in_legend(show_high_in_legend, show_low_in_legend);


            if (show_high_in_legend &&
               legend_pos.text_high_rect != nullptr &&
               legend_pos.symbol_high_rect != nullptr)
            {
               draw_legend(thresholds.high_legend_text,
                  legend_pos.text_high_rect,
                  legend_pos.symbol_high_rect,
                  thresholds.high_pen,
                  thresholds.high_point_colour,
                  bar_pen, bar_up_brush,
                  legend_font, foreground, background);
            }

            if (show_low_in_legend &&
               legend_pos.text_low_rect != nullptr &&
               legend_pos.symbol_low_rect != nullptr)
            {
               draw_legend(thresholds.low_legend_text,
                  legend_pos.text_low_rect,
                  legend_pos.symbol_low_rect,
                  thresholds.low_pen,
                  thresholds.low_point_colour,
                  bar_down_pen, bar_down_brush,
                  legend_font, foreground, background);
            }
         }
      } // draw_legend


      void GraphTrace::draw()
      {
         bool orig_high_data_exists = thresholds.high_data_exists;
         bool orig_low_data_exists = thresholds.low_data_exists;

         thresholds.high_data_exists = false;
         thresholds.low_data_exists = false;

         if(visible)
         {
            get_marks();
            
            if(trace_type == trace_line)
            {
               draw_lines();
               draw_points();
            }
            else if(trace_type == trace_point)
            {
               draw_points();
            }
            else if(trace_type == trace_bar)
            {
               draw_bars();
            }
            else if (trace_type == trace_area)
            {
               draw_areas();
            }
            else if (trace_type == trace_wind_dir)
            {
               draw_lines();
               draw_wind_dirs();
            }
            else if (trace_type == trace_reference_line)
            {
               draw_reference_line();
            }
            else if (trace_type == trace_vertical_reference_line)
            {
               draw_vertical_reference_line();
            }
            else if (trace_type == trace_reference_dot)
            {
               draw_reference_dot();
            }
         }

         if ((thresholds.high_data_exists != orig_high_data_exists) ||
             (thresholds.low_data_exists != orig_low_data_exists))
         {
            graph->positions_invalid = true;
            graph->force_redraw();
         }

      } // draw


      GraphTrace::hit_test_value GraphTrace::hit_test(
         Point const &pos, double radius) const
      {
         // todo!  (next version)
         // if (trace_type == trace_bar)
         // {
         //   <determine if the user has clicked in the area inside the bar>
         //   <this might mean for each point, calculate the bar rectangle, like
         //   <we do when we draw the bar, then determine inf pos falls inside
         //   <the bar rectangle>
         // }
         double prev_distance(std::numeric_limits<double>::max());
         hit_test_value rtn(std::make_pair(GraphPoint(), prev_distance));
         for(const_iterator pi = begin(); visible && pi != end(); ++pi)
         {
            // we need to convert the point to screen coordinates
            GraphPoint const &point(*pi);
            Point point_pos(
               domain_axis->value_to_pos(point.x),
               range_axis->value_to_pos(point.y));
            if(domain_axis->get_vertical())
               std::swap(point_pos.x, point_pos.y);


            // we can now evaluate the distance between this point and the mouse position
            double distance(point_pos.distance(pos));
            if(distance < radius && distance < prev_distance)
            {
               prev_distance = rtn.second = distance;
               rtn.first = point;
            }
         }
         return rtn;
      } // hit_test

      
      StrAsc GraphTrace::format_mark(GraphPoint const &point)
      {
         OStrAscStream rtn;
         StrUni x_format;
         StrUni y_format;

         if (mark_props.show_x)
         {
            if (domain_axis->get_scale()->time_domain)
            {
               LgrDate stamp(point.x->get_val_int() * LgrDate::nsecPerMSec);
               stamp.format(x_format, mark_props.time_format);
            }
            else
            {
               x_format = range_axis->get_scale()->format_label(point.x, true);
               if (mark_props.x_units.length() > 0)
               {
                  x_format += L" ";
                  x_format += mark_props.x_units;
               }
            }
         }

         if (mark_props.show_y)
         {
            if (point.wind_dir == nullptr)
            {
               y_format = range_axis->get_scale()->format_label(point.y, true);
               if (mark_props.y_units.length() > 0)
               {
                  y_format += L" ";
                  y_format += mark_props.y_units;
               }

            }
            else
            {
               y_format = L"(";
               y_format += range_axis->get_scale()->format_label(point.y, true);
               if (mark_props.y_units.length() > 0)
               {
                  y_format += L" ";
                  y_format += mark_props.y_units;
               }
               y_format +=  L", ";
               y_format += range_axis->get_scale()->format_label(point.wind_dir, true);
               y_format += L"°)";
            }
         }

         if (mark_props.show_x && mark_props.show_y)
         {
            rtn << y_format << L" @ " << x_format;
         }
         else if (mark_props.show_x)
         {
            rtn << x_format;
         }
         else if (mark_props.show_y)
         {
            rtn << y_format;
         }
         return rtn.str();
      } // format_mark


      namespace
      {
         struct user_defined_mark
         {
            bool operator ()(GraphTrace::mark_handle const &mark) const
            { return mark->get_user_specified(); }
         };
      };


      /******************************************************************************************************
      * GraphTrace::clear_marks
      *
      *****************************************************************************************************/
      void GraphTrace::clear_marks(bool current_only)
      {
         if(!current_only)
            marks.clear();
         else
         {
            marks_type::iterator ri(std::remove_if(marks.begin(), marks.end(), user_defined_mark()));
            if(ri != marks.end())
               marks.erase(ri, marks.end());
         }
      } // clear_marks

      /******************************************************************************************************
       * GraphTrace::get_marks
       *
       *****************************************************************************************************/
      void GraphTrace::get_marks()
      {
         if (mark_props.always_show)
         {
            clear_marks(false);
            if (visible && mark_props.enabled && (mark_props.show_x || mark_props.show_y))
            {
               GraphTraceMarkDirType preferred_direction = mark_dir_north;
               if (domain_axis->get_axis_type() == GraphAxis::axis_left)
                  preferred_direction = mark_dir_east;
               else if (domain_axis->get_axis_type() == GraphAxis::axis_right)
                  preferred_direction = mark_dir_west;

               Rect plot_rect(graph->get_plot_rect()->get_rect(true));
               GraphAxis::value_handle domain_scale_max(domain_axis->get_scale_max());
               int decimateCount = mark_props.decimation;

               for (iterator pi = begin(); pi != end(); ++pi)
               {
                  value_type const &point(*pi);
                  Point coord(domain_axis->value_to_pos(point.x), range_axis->value_to_pos(point.y));

                  if (domain_axis->get_vertical())
                     std::swap(coord.x, coord.y);

                  if (is_finite(coord.x) && is_finite(coord.y) && plot_rect.within(coord))
                  {
                     decimateCount++;
                     if (decimateCount >= mark_props.decimation)
                     {
                        if (coord.y > plot_rect.get_top() && coord.y < plot_rect.get_bottom())
                           marks.push_back(new mark_type(point, format_mark(point), true, preferred_direction));
                        decimateCount = 0;
                     }
                  }
                  if (point.x->get_val() > domain_scale_max->get_val())
                     break;
               }
            }
         }
      }


      /******************************************************************************************************
      * GraphTrace::draw_lines
      *
      *****************************************************************************************************/
      void GraphTrace::draw_lines()
      {
         if (visible && pen->get_visible() && !empty())
         {
            if (!thresholds.enable_high && !thresholds.enable_low)
            {
               draw_lines_solid();
            }

            else
            {
               draw_lines_with_thresholds();
            }
         }
      }

      /******************************************************************************************************
      * GraphTrace::draw_lines_solid
      *
      *****************************************************************************************************/
      void GraphTrace::draw_lines_solid()
      {
         Driver::points_type drawn;

         Rect plot_rect(graph->get_plot_rect()->get_rect(true));
         Point prev_coord(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
         GraphAxis::value_handle domain_scale_max(get_domain_axis()->get_scale_max());
         pen_handle draw_with_pen = pen;
         bool bValidPrev = false;

         for (const_iterator pi = begin(); pi != end(); ++pi)
         {
            value_type const &point(*pi);
            double value = point.y->get_val();
            Point coord(get_domain_axis()->value_to_pos(point.x), get_range_axis()->value_to_pos(point.y));
         
            if (get_domain_axis()->get_vertical())
               std::swap(coord.x, coord.y);

            if (Csi::is_finite(coord.x) && Csi::is_finite(coord.y))
            {
               if (plot_rect.within(coord) || plot_rect.within(prev_coord))
               {
                  if (drawn.empty() && bValidPrev)
                  {
                     drawn.push_back(prev_coord);
                  }

                  if (get_use_stairs())
                  {
                     Point stepPoint(coord.x, prev_coord.y);
                     drawn.push_back(stepPoint);
                  }
                  drawn.push_back(coord);
               }

               else if (!drawn.empty())
               {
                  driver->draw_lines(drawn, draw_with_pen);
                  drawn.clear();
               }

               prev_coord = coord;
               bValidPrev = true;
            }
            else
            {
               if (!drawn.empty())
               {
                  driver->draw_lines(drawn, draw_with_pen);
                  drawn.clear();
                  bValidPrev = false;
               }
            }

            if (point.x->get_val() > domain_scale_max->get_val())
               break;
         }

         if (!drawn.empty())
            driver->draw_lines(drawn, draw_with_pen);
      }

      /******************************************************************************************************
      * GraphTrace::draw_lines_with_thresholds
      *
      *****************************************************************************************************/
      void GraphTrace::draw_lines_with_thresholds()
      {
         Driver::points_type drawn;

         Rect plot_rect(graph->get_plot_rect()->get_rect(true));
         Point prev_coord(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
         GraphAxis::value_handle domain_scale_max(get_domain_axis()->get_scale_max());

         pen_handle normal_pen = get_pen();

         bool bValidPrev = false;
         multiColored_t myPoint, prevPoint;

         for (const_iterator pi = begin(); pi != end(); ++pi)
         {
            value_type const &point(*pi);
            double value = point.y->get_val();

            Point coord(get_domain_axis()->value_to_pos(point.x), get_range_axis()->value_to_pos(point.y));
            if (get_domain_axis()->get_vertical())
               std::swap(coord.x, coord.y);


            if (Csi::is_finite(coord.x) && Csi::is_finite(coord.y))
            {
               myPoint.coord = coord;
               myPoint.value = value;

               if (plot_rect.within(coord) || plot_rect.within(prev_coord))
               {
                  if (thresholds.enable_high && value > thresholds.high_value)
                  {
                     thresholds.high_data_exists = true;
                     myPoint.pen = thresholds.high_pen;
                  }
                  else if (thresholds.enable_low && value < thresholds.low_value)
                  {
                     thresholds.low_data_exists = true;
                     myPoint.pen = thresholds.low_pen;
                  }
                  else
                  {
                     myPoint.pen = normal_pen;
                  }

                  if (bValidPrev)
                  {
                     if (get_use_stairs())
                     {
                        multiColored_t pointStep = prevPoint;
                        if (get_domain_axis()->get_vertical())
                           pointStep.coord.y = coord.y;
                        else
                           pointStep.coord.x = coord.x;

                        draw_threshold_colored_line(prevPoint, pointStep, drawn);
                        prevPoint = pointStep;
                     }
                     draw_threshold_colored_line(prevPoint, myPoint, drawn);
                  }
               }

               prev_coord = coord;
               prevPoint  = myPoint;
               bValidPrev = true;
            }
            else
            {
               bValidPrev = false;
            }

            if (point.x->get_val() > domain_scale_max->get_val())
               break;
         }

         if (!drawn.empty())
            driver->draw_lines(drawn, myPoint.pen);

      }


      /******************************************************************************************************
      * GraphTrace::draw_threshold_colored_line
      *
      * This determines if the line needs to cross the high or low thresholds (or both).  If they do,
      * then insert multiple points to handle the places where they cross.
      *
      * Input:
      *     prev_point, point:  the two points of our line. The point information includes
      *            information of its coordinates, its value, and its color.
      *
      * Output:
      *     draws the line
      *****************************************************************************************************/
      void GraphTrace::draw_threshold_colored_line(multiColored_t &prev_point, multiColored_t &point, Driver::points_type &drawn)
      {
         if (prev_point.pen == nullptr)
         {
            if (thresholds.enable_high && prev_point.value > thresholds.high_value)
            {
               thresholds.high_data_exists = true;
               prev_point.pen = thresholds.high_pen;
            }
            else if (thresholds.enable_low && prev_point.value < thresholds.low_value)
            {
               thresholds.low_data_exists = true;
               prev_point.pen = thresholds.low_pen;
            }
            else
            {
               prev_point.pen = pen;
            }
            drawn.push_back(prev_point.coord);
         }

         if (point.pen != prev_point.pen)
         {
            multiColored_t betweenPoint = prev_point;
            double *pCoordX = &betweenPoint.coord.x;
            double *pCoordY = &betweenPoint.coord.y;
            bool bVerticalLine = prev_point.coord.x == point.coord.x;
            if (get_domain_axis()->get_vertical())
            {
               pCoordX = &betweenPoint.coord.y;
               pCoordY = &betweenPoint.coord.x;
               bVerticalLine = prev_point.coord.y == point.coord.y;
            }

            double lowPos  = get_range_axis()->value_to_pos(thresholds.low_value);
            double highPos = get_range_axis()->value_to_pos(thresholds.high_value);

            pen_handle normal_pen = get_pen();

            betweenPoint = prev_point;
            if (bVerticalLine)
            {
               if (prev_point.value < point.value)
               {
                  if (thresholds.enable_low && prev_point.value < thresholds.low_value)
                  {
                     *pCoordY = lowPos;
                     drawn.push_back(betweenPoint.coord);
                     driver->draw_lines(drawn, prev_point.pen);
                     drawn.clear();
                     drawn.push_back(betweenPoint.coord);
                     prev_point = betweenPoint;
                  }

                  if (thresholds.enable_high && point.value > thresholds.high_value)
                  {
                     *pCoordY = highPos;
                     betweenPoint.pen = normal_pen;
                     drawn.push_back(betweenPoint.coord);
                     driver->draw_lines(drawn, normal_pen);
                     drawn.clear();
                     drawn.push_back(betweenPoint.coord);
                     prev_point = betweenPoint;

                  }
                  drawn.push_back(point.coord);
               }
               else
               {
                  if (thresholds.enable_high && prev_point.value > thresholds.high_value)
                  {
                     *pCoordY = highPos;
                     drawn.push_back(betweenPoint.coord);
                     driver->draw_lines(drawn, prev_point.pen);
                     drawn.clear();
                     drawn.push_back(betweenPoint.coord);
                     prev_point = betweenPoint;
                  }
                  if (thresholds.enable_low && point.value < thresholds.low_value)
                  {
                     *pCoordY = lowPos;

                     drawn.push_back(betweenPoint.coord);
                     driver->draw_lines(drawn, normal_pen);
                     drawn.clear();
                     drawn.push_back(betweenPoint.coord);
                     prev_point = betweenPoint;
                  }

                  drawn.push_back(point.coord);
               }
            }
            else
            {
               // Calculate the line, y = mx + c;
               double slope = (point.coord.y - prev_point.coord.y) / (point.coord.x - prev_point.coord.x);
               double offset = point.coord.y - slope * point.coord.x;

               if (get_domain_axis()->get_vertical())
               {
                  slope = (point.coord.x - prev_point.coord.x) / (point.coord.y - prev_point.coord.y);
                  offset = point.coord.x - slope * point.coord.y;
               }

               if (prev_point.value < point.value)
               {
                  if (thresholds.enable_low && prev_point.value < thresholds.low_value)
                  {
                     *pCoordY = lowPos;
                     *pCoordX = (lowPos - offset) / slope;

                     drawn.push_back(betweenPoint.coord);
                     driver->draw_lines(drawn, prev_point.pen);
                     drawn.clear();
                     drawn.push_back(betweenPoint.coord);
                     prev_point = betweenPoint;
                  }

                  if (thresholds.enable_high && point.value > thresholds.high_value)
                  {
                     *pCoordY = highPos;
                     *pCoordX = (highPos - offset) / slope;
                     betweenPoint.pen = normal_pen;

                     drawn.push_back(betweenPoint.coord);
                     driver->draw_lines(drawn, normal_pen);
                     drawn.clear();
                     drawn.push_back(betweenPoint.coord);
                     prev_point = betweenPoint;

                     prev_point = betweenPoint;
                  }
                  drawn.push_back(point.coord);
               }
               else
               {
                  if (thresholds.enable_high && prev_point.value > thresholds.high_value)
                  {
                     *pCoordY = highPos;
                     *pCoordX = (highPos - offset) / slope;
                     drawn.push_back(betweenPoint.coord);
                     driver->draw_lines(drawn, prev_point.pen);
                     drawn.clear();
                     drawn.push_back(betweenPoint.coord);
                     prev_point = betweenPoint;
                  }

                  if (thresholds.enable_low && point.value < thresholds.low_value)
                  {
                     *pCoordY = lowPos;
                     *pCoordX = (lowPos - offset) / slope;
                     betweenPoint.pen = normal_pen;
                     drawn.push_back(betweenPoint.coord);
                     driver->draw_lines(drawn, normal_pen);
                     drawn.clear();
                     drawn.push_back(betweenPoint.coord);
                     prev_point = betweenPoint;
                  }
                  drawn.push_back(point.coord);
               }
            }
         }
         else
         {
            drawn.push_back(point.coord);
         }
      }

      /******************************************************************************************************
      * GraphTrace::draw_points
      *
      *****************************************************************************************************/
      void GraphTrace::draw_points()
      {
         if(visible && point_type != nullptr && point_size > 0)
         {
            Rect plot_rect(graph->get_plot_rect()->get_rect(true));
            GraphAxis::value_handle domain_scale_max(domain_axis->get_scale_max());
            
            for(iterator pi = begin(); pi != end(); ++pi)
            {
               value_type const &point(*pi);
               Point coord(domain_axis->value_to_pos(point.x), range_axis->value_to_pos(point.y));

               if(domain_axis->get_vertical())
                  std::swap(coord.x, coord.y);
               
               if(is_finite(coord.x) && is_finite(coord.y) && plot_rect.within(coord))
               {
                  double value = point.y->get_val();

                  Colour start_colour = point_colour;
                  Colour age_colour   = point_colour;

                  if (colours_controller != nullptr)
                  {
                     age_colour = point_colour_gradient;
                  }

                  pen_handle pen     = point_pen;
                  brush_handle brush = point_brush;

                  if (thresholds.enable_high && value > thresholds.high_value)
                  {
                     thresholds.high_data_exists = true;
                     start_colour = thresholds.high_point_colour;
                     age_colour   = thresholds.high_age_point_colour;
                     brush        = thresholds.high_point_brush;
                     pen          = thresholds.high_point_pen;
                  }

                  else if (thresholds.enable_low && value < thresholds.low_value)
                  {
                     thresholds.low_data_exists = true;
                     start_colour = thresholds.low_point_colour;
                     age_colour   = thresholds.low_age_point_colour;
                     brush        = thresholds.low_point_brush;
                     pen          = thresholds.low_point_pen;
                  }

                  if (colours_controller != nullptr)
                  {
                     uint4 pos = (uint4)std::distance(begin(), pi);
                     Colour cor = colours_controller->get_point_colour(this, pos, start_colour, age_colour);
                     point_type->draw(*driver, coord, cor, point_size);
                  }
                  else
                  {
                     point_type->draw(*driver, coord, brush, pen, point_size);
                  }
               }

               if(domain_axis->get_time_domain() && !domain_axis->get_logarithmic() && point.x->get_val() > domain_scale_max->get_val())
                  break;
            }
         }
      } // draw_points


      bool withinDomain(Point const &point, Rect const &rect, bool bDomainHorizontal)
      {
         if (bDomainHorizontal)
         {
            return
               (point.x >= rect.origin.x) &&
               (point.x <= rect.origin.x + rect.width);
         }
         else
         {
            return
               (point.y >= rect.origin.y) &&
               (point.y <= rect.origin.y + rect.height);
         }
      }


      /******************************************************************************************************
      * GraphTrace::draw_areas
      *
      *****************************************************************************************************/
      void GraphTrace::draw_areas()
      {
         // we can now draw all visible bars
         Rect plot_rect(graph->get_plot_rect()->get_rect(true));
         double zero_val = range_axis->get_logarithmic() ? 1 : 0;
         double zero_pos(range_axis->value_to_pos(zero_val));

         Driver::points_type coordinates;
         bool bPrevValid = false;

         // this is dealing with values, not coordinates
         typedef struct {
            double x;
            double y;
         } my_points_t;  

         my_points_t myPoint, prevPoint;
         Point myCoord, prevCoord;
         

         for(const_iterator pi = begin(); pi != end(); ++pi)
         {
            StrUni timestr;
            pi->x->get_timestamp().format(timestr, L"%M:%S");

            myPoint.x = pi->x->get_val();
            myPoint.y = pi->y->get_val();

            myCoord.x = domain_axis->value_to_pos(myPoint.x);
            myCoord.y = range_axis->value_to_pos(myPoint.y);

            bool bDomainHorizontal = !domain_axis->get_vertical();
            if (!bDomainHorizontal)
            {
               std::swap(myCoord.x, myCoord.y);
            }


            if(is_finite(myCoord.x) && is_finite(myCoord.y))
            {
               if (bPrevValid && 
                  (myPoint.x >= prevPoint.x) &&
                  (withinDomain(myCoord, plot_rect, bDomainHorizontal)) || withinDomain(prevCoord, plot_rect, bDomainHorizontal))
               {
                  if ((myPoint.y > 0 && prevPoint.y < 0) ||
                      (myPoint.y < 0 && prevPoint.y > 0))
                  {
                     Point betweenCoord;

                     if (myPoint.x > prevPoint.x)
                     {
                        // y = mx + c;
                        double m = (myPoint.y - prevPoint.y) / (myPoint.x - prevPoint.x);
                        double c = myPoint.y - (m * myPoint.x);
                        double zeroX = -c / m;

                        betweenCoord.x = domain_axis->value_to_pos(zeroX);
                        betweenCoord.y = zero_pos;
                     }
                     else
                     {
                        betweenCoord.x = domain_axis->value_to_pos(prevPoint.x);
                        betweenCoord.y = zero_pos;
                     }

                     if (domain_axis->get_vertical())
                     {
                        std::swap(betweenCoord.x, betweenCoord.y);
                     }

                     draw_area(prevCoord, betweenCoord, prevPoint.y > 0);
                     prevCoord = betweenCoord;
                  }

                  bool bPointingUp = myPoint.y > 0;
                  if (myPoint.y == 0)
                     bPointingUp = prevPoint.y >= 0;

                  draw_area(prevCoord, myCoord, bPointingUp);
               }

               bPrevValid = true;
               prevPoint = myPoint;
               prevCoord = myCoord;
            }
            else
            {
               bPrevValid = false;
            }
         }
      }

      /******************************************************************************************************
      * GraphTrace::draw_area
      *
      *****************************************************************************************************/
      void GraphTrace::draw_area(Point const &point1, Point const &point2, bool pointing_up)
      {
         Driver::points_type coordinates;
         coordinates.clear();
         double zero_val(range_axis->get_logarithmic() ? 1 : 0);
         double zero_pos(range_axis->value_to_pos(zero_val));

         if (range_axis->get_vertical())
         {
            if (point1.x != point2.x)
            {
               if ((point1.y != zero_pos) || (point2.y != zero_pos))
               {
                  coordinates.push_back(Point(point1.x, zero_pos));
                  coordinates.push_back(point1);
                  coordinates.push_back(Point(point2.x + 0.75, point2.y));
                  coordinates.push_back(Point(point2.x + 0.75, zero_pos));
               }
               else
               {
                  coordinates.push_back(Point(point1.x, zero_pos + .5));
                  coordinates.push_back(Point(point1.x, point1.y - 0.5));
                  coordinates.push_back(Point(point2.x + 0.5, point2.y - 0.5));
                  coordinates.push_back(Point(point2.x + 0.5, zero_pos + .5));
               }
            }
         }
         else
         {
            if (point1.y != point2.y)
            {
               if ((point1.x != zero_pos) || (point2.x != zero_pos))
               {
                  coordinates.clear();
                  coordinates.push_back(Point(zero_pos, point1.y));
                  coordinates.push_back(point1);
                  coordinates.push_back(Point(point2.x, point2.y + 0.75));
                  coordinates.push_back(Point(zero_pos, point2.y + 0.75));
               }
               else
               {
                  coordinates.clear();
                  coordinates.push_back(Point(zero_pos - .5, point1.y));
                  coordinates.push_back(Point(point1.x + .5, point1.y));
                  coordinates.push_back(Point(point2.x + .5, point2.y + 0.5));
                  coordinates.push_back(Point(zero_pos - .5, point2.y + 0.5));
               }
            }
         }
         
         if (!coordinates.empty())
            driver->draw_polygon(coordinates, nullptr, pointing_up ? bar_up_brush : bar_down_brush);

         driver->draw_line(point1, point2, pointing_up ? bar_pen : bar_down_pen);
      } // draw_area


      /******************************************************************************************************
      * GraphTrace::draw_bars
      *
      *****************************************************************************************************/
      void GraphTrace::draw_bars()
      {
         // to begin with, we will need to find the minimum domain interval between points.
         double total_bar_width = graph->get_bar_width();
         double bar_count       = graph->get_bar_count();

         // we need to now calculate the width of the bar.  This will depend upon the minimum
         // interval (calculated above), the number of bar traces on the graph, and the bar width
         // ratio.   
         double single_bar_width = (total_bar_width / bar_count);
         double bar_offset       = single_bar_width * bar_position;

         // Be sure to take into accont the borders that will be shared between the bars.
         if (bar_pen->get_visible() && bar_pen->get_width() >= 1)
         {
            single_bar_width += (bar_count-1) / 2;
            bar_offset       -= 1/2 * bar_position;
         }

         // Center the group of bars over the label
         bar_offset -= (total_bar_width / 2.f - single_bar_width / 2.f);

         double bar_width = graph->get_bar_width();
         if (graph->get_bar_overlap_style() == Csi::Graphics::Graph::bars_side_by_side)
         {
            bar_width = single_bar_width;
         }
         
         // we can now draw all visible bars
         Rect plot_rect(graph->get_plot_rect()->get_rect(true));
         double zero_val = range_axis->get_logarithmic() ? 1 : 0;
         double zero_pos(range_axis->value_to_pos(new Expression::Operand(zero_val, 0)));
         for(const_iterator pi = begin(); pi != end(); ++pi)
         {
            double x(domain_axis->value_to_pos(pi->x) + bar_offset);
            double y(range_axis->value_to_pos(pi->y));
            if(is_finite(x) && is_finite(y))
            {
               Rect bar_rect;
               bool pointing_up;
               if(!domain_axis->get_vertical())
               {
                  if(zero_pos > y)
                  {
                     bar_rect = Rect(x - bar_width / 2, y, bar_width, zero_pos - y);
                     pointing_up = true;
                  }
                  else
                  {
                     bar_rect = Rect(x - bar_width / 2, zero_pos, bar_width, y - zero_pos);
                     pointing_up = false;
                  }
               }
               else
               {
                  if(zero_pos > y)
                  {
                     bar_rect = Rect(y, x - bar_width / 2, zero_pos - y, bar_width);
                     pointing_up = false;
                  }
                  else
                  {
                     bar_rect = Rect(zero_pos, x - bar_width / 2, y - zero_pos, bar_width);
                     pointing_up = true;
                  }
               }

               if(range_axis->get_vertical() && bar_rect.height <= 0)
               {
                  bar_rect.height = 1;
                  bar_rect.set_top(bar_rect.get_top() - 0.5);
                  pointing_up = true;
               }
               else if (!range_axis->get_vertical() && bar_rect.width <= 0)
               {
                  bar_rect.width= 1;
                  bar_rect.set_left(bar_rect.get_left() - 0.5);
                  pointing_up = true;
               }

               if(plot_rect.intersects(bar_rect))
                  draw_bar(bar_rect, pointing_up);
            }
         }
      } // draw_bars


      void GraphTrace::draw_bar(Rect const &rect_, bool pointing_up)
      {
         Rect rect(rect_);
         pen_handle outline_pen = pointing_up ? bar_pen : bar_down_pen;
         PenInfo info = outline_pen->get_desc();
         if (info.get_width() > 1)
         {
           info.set_width(1);
           outline_pen = driver->make_pen(info);
         }
         
         if (outline_pen != nullptr)
         {
            if (!outline_pen->get_visible() || outline_pen->get_width() <= 0)
               outline_pen = nullptr;

            // Shorten the width of the bars to make room for the outline pens
            if (outline_pen != nullptr)
            {
               if (!domain_axis->get_vertical())
               {
                  rect.set_left(rect.get_left() + outline_pen->get_width() / 2);
                  rect.width = rect.width - outline_pen->get_width();
               }
               else
               {
                  rect.set_top(rect.get_top() + outline_pen->get_width() / 2);
                  rect.height = rect.height - outline_pen->get_width();
               }
            }
         }

         driver->draw_rect(rect, outline_pen, pointing_up ? bar_up_brush : bar_down_brush);
      }


      /******************************************************************************************************
      * GraphTrace::draw_wind_dirs
      *
      *****************************************************************************************************/
      void GraphTrace::draw_wind_dirs()
      {
         // Draw all visible wind arrows
         Rect plot_rect(graph->get_plot_rect()->get_rect(true));

         GraphAxis::value_handle domain_scale_max(get_domain_axis()->get_scale_max());
         pen_handle myPen;

         for (const_iterator pi = begin(); pi != end(); ++pi)
         {
            value_type const &point(*pi);

            double value = point.y->get_val();

            Point coord(get_domain_axis()->value_to_pos(point.x), get_range_axis()->value_to_pos(point.y));
            double wind_dir = (point.wind_dir != nullptr) ? point.wind_dir->get_val() : 0;
            if (get_domain_axis()->get_vertical())
               std::swap(coord.x, coord.y);

            if (Csi::is_finite(coord.x) && Csi::is_finite(coord.y) && plot_rect.within(coord))
            {
               myPen = point_pen;

               if (thresholds.enable_high && value >= thresholds.high_value)
               {
                  thresholds.high_data_exists = true;
                  myPen = thresholds.high_point_pen;
                  //myPointSize = thresholds.high_point_size;
               }
               else if (thresholds.enable_low && value <= thresholds.low_value)
               {
                  thresholds.low_data_exists = true;
                  myPen = thresholds.low_point_pen;
                  //myPointSize = thresholds.low_point_size;
               }

               driver->draw_wind_barb(coord, wind_dir, point_size, myPen);
            }

            if (point.x->get_val() > domain_scale_max->get_val())
               break;
         }
      }


      /******************************************************************************************************
       * GraphTrace::draw_reference_line
       *
       *****************************************************************************************************/
      void GraphTrace::draw_reference_line()
      {
         Rect plot_rect(graph->get_plot_rect()->get_rect(true));
         double position(range_axis->value_to_pos(reference_line));

         if (range_axis->get_vertical())
         {
            Point point1(plot_rect.get_left(),  position);
            Point point2(plot_rect.get_right(), position);
            driver->draw_line(point1, point2, pen);
         }
         else
         {
            Point point1(position, plot_rect.get_top());
            Point point2(position, plot_rect.get_bottom());
            driver->draw_line(point1, point2, pen);
         }
      }

      void GraphTrace::draw_vertical_reference_line()
      {
         Rect plot_rect(graph->get_plot_rect()->get_rect(true));
         double position(domain_axis->value_to_pos(reference_line));

         if (!range_axis->get_vertical())
         {
            Point point1(plot_rect.get_left(),  position);
            Point point2(plot_rect.get_right(), position);
            driver->draw_line(point1, point2, pen);
         }
         else
         {
            Point point1(position, plot_rect.get_top());
            Point point2(position, plot_rect.get_bottom());
            driver->draw_line(point1, point2, pen);
         }
      }

      void GraphTrace::draw_reference_dot()
      {
         if(visible && point_type != nullptr && point_size > 0)
         {
            Rect plot_rect(graph->get_plot_rect()->get_rect(true));
            double positionDomain(domain_axis->value_to_pos(reference_line));
            double positionRange(range_axis->value_to_pos(reference_dot));
            Point point(positionDomain, positionRange);

            if (!range_axis->get_vertical())
            {
               std::swap(point.x, point.y);
            }
            point_type->draw(*driver, point, point_brush, point_pen, point_size);
         }
      }

      /******************************************************************************************************
      * GraphTrace::draw_marks
      *             draw_mark
      *
      *****************************************************************************************************/
      void GraphTrace::draw_marks()
      {
         for(marks_type::iterator mi = marks.begin(); mi != marks.end(); ++mi)
            draw_mark(**mi);
      } // draw_marks


      void GraphTrace::draw_mark(mark_type const &mark)
      {
         Rect plot_rect(graph->get_plot_rect()->get_rect(true));
         double x(domain_axis->value_to_pos(mark.get_point().x));
         double y(range_axis->value_to_pos(mark.get_point().y));

         if (trace_type == trace_bar)
         {
            // to begin with, we will need to find the minimum domain interval between points.
            double bar_width = graph->get_bar_width();
            double bar_count = graph->get_bar_count();

            // we need to now calculate the width of the bar.  This will depend upon the minimum
            // interval (calculated above), the number of bar traces on the graph, and the bar width
            // ratio.
            double bar_step_size(bar_width / bar_count);
            double bar_offset(bar_step_size * bar_position);
            bar_offset -= (bar_width / 2.f - bar_step_size / 2.f);

            x += bar_offset;
         }

         if(domain_axis->get_vertical())
            std::swap(x, y);
         if(is_finite(x) && is_finite(y) && plot_rect.within(Point(x, y)))
         {
            // we need to format the mark and determine where to place it.
            StrAsc mark_text(mark.get_label());
            Rect mark_rect(driver->measure_text(mark_text, mark_props.font));
            mark_rect.width += mark_margin * 2;
            mark_rect.centre_x(x + ((mark_distance + mark_rect.width / 2) * std::cos(degrees_to_radians(mark.get_preferred_direction()))));
            mark_rect.centre_y(y - ((mark_distance + mark_rect.height / 2) * std::sin(degrees_to_radians(mark.get_preferred_direction()))));

            // Calculate where the mark should be drawn.  Also, while we're at it, calculate where the line pointing to the mark
            // should be drawn. 
            Point pointer_line;
            switch (mark.get_preferred_direction())
            {
            default:
            case mark_dir_north:
            case mark_dir_north_west:
            case mark_dir_north_east:
               if (mark_rect.get_top() < plot_rect.get_top())
               {
                  mark_rect.set_top(y + mark_distance);
                  pointer_line.y = mark_rect.get_top();
               }
               else
                  pointer_line.y = mark_rect.get_bottom();
               if (mark_rect.get_left() < plot_rect.get_left() + mark_margin * 2)
                  mark_rect.set_left(plot_rect.get_left() + mark_margin * 2);
               if (mark_rect.get_right() > plot_rect.get_right() - mark_margin * 2)
                  mark_rect.set_right(plot_rect.get_right() - mark_margin * 2);
               pointer_line.x = mark_rect.get_centre_x();
               break;

            case mark_dir_south:
            case mark_dir_south_west:
            case mark_dir_south_east:
               if (mark_rect.get_bottom() > plot_rect.get_bottom())
               {
                  mark_rect.set_bottom(y - mark_distance);
                  pointer_line.y = mark_rect.get_bottom();
               }
               else
                  pointer_line.y = mark_rect.get_top();

               if (mark_rect.get_left() < plot_rect.get_left() + mark_margin * 2)
                  mark_rect.set_left(plot_rect.get_left() + mark_margin * 2);
               if (mark_rect.get_right() > plot_rect.get_right() - mark_margin * 2)
                  mark_rect.set_right(plot_rect.get_right() - mark_margin * 2);
               pointer_line.x = mark_rect.get_centre_x();
               break;
               
            case mark_dir_west:
               if (mark_rect.get_left() < plot_rect.get_left())
               {
                  mark_rect.set_left(x + mark_distance);
                  pointer_line.x = mark_rect.get_left();
               }
               else
                  pointer_line.x = mark_rect.get_right();

               if (mark_rect.get_top() < plot_rect.get_top() + mark_margin * 2)
                  mark_rect.set_top(plot_rect.get_top() + mark_margin * 2);
               if (mark_rect.get_bottom() > plot_rect.get_bottom() - mark_margin * 2)
                  mark_rect.set_bottom(plot_rect.get_bottom() - mark_margin * 2);
               pointer_line.y = mark_rect.get_centre_y();
               break;

            case mark_dir_east:
               if (mark_rect.get_right() > plot_rect.get_right())
               {
                  mark_rect.set_right(x - mark_distance);
                  pointer_line.x = mark_rect.get_right();
               }
               else
                  pointer_line.x = mark_rect.get_left();
               if (mark_rect.get_top() < plot_rect.get_top() + mark_margin * 2)
                  mark_rect.set_top(plot_rect.get_top() + mark_margin * 2);
               if (mark_rect.get_bottom() > plot_rect.get_bottom() - mark_margin * 2)
                  mark_rect.set_bottom(plot_rect.get_bottom() - mark_margin * 2);
               pointer_line.y = mark_rect.get_centre_y();
               break;
            }

            // we have to decide on the colour to use for the mark background.  After that, we can
            // decide on the colour to use for the mark text.
            
            Colour foreground_colour;
            if(mark_props.transparent_back)
            {
               foreground_colour = mark_props.colour;
            }
            else 
            {
               if(mark_props.colour.to_grayscale() < 0.5)
                  foreground_colour = white_colour();
               else 
                  foreground_colour = black_colour();
            }

            // draw the line that goes from the point to the mark.  Note that if we have a transparent back,
            // then we do not want the line to overwrite the text, so adjust the point.
            pen_handle pen(driver->make_pen(PenInfo().set_colour(mark_props.colour).set_width(2)));
            driver->draw_line(Point(x, y), pointer_line, pen);

            // Draw the background and text
            if (!mark_props.transparent_back)
            {
               brush_handle brush(driver->make_brush(BrushInfo().set_colour(mark_props.colour)));
               if (mark_props.rounded_back)
                  driver->draw_rounded_rect(mark_rect, 0, brush);
               else
                  driver->draw_rect(mark_rect, 0, brush);
            }
            driver->draw_text(mark_text, mark_rect, mark_props.font, foreground_colour, mark_props.colour);
         }
      } // draw_mark
   };
};

