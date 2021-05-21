/* Csi.Graphics.GraphAxis.cpp

   Copyright (C) 2015, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 23 January 2015
   Last Change: Saturday 21 May 2016
   Last Commit: $Date: 2020-07-14 15:42:11 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#define NOMINMAX
#include "Csi.Graphics.GraphAxis.h"
#include "Csi.Graphics.Graph.h"
#include <numeric>


namespace Csi
{
   namespace Graphics
   {
      namespace
      {
         double const small_margin(2);
      };

      
      GraphAxis::GraphAxis(Graph *graph_, axis_type_code axis_type_, bool domain_axis_):
         graph(graph_),
         axis_type(axis_type_),
         driver(graph_->driver),
         domain_axis(domain_axis_)
      {
         scale.bind(new Scale(driver));
         scale->vertical = (axis_type == axis_right || axis_type == axis_left);
         set_default_properties();
      } // constructor


      GraphAxis::~GraphAxis()
      { }

      GraphAxis &GraphAxis::set_caption(StrUni const &caption_)
      {
         caption = caption_;
         graph->set_positions_invalid();
         return *this;
      }
      
      GraphAxis &GraphAxis::set_auto_bounds(bool auto_min_, bool auto_max_)
      {
         auto_min = auto_min_;
         auto_max = auto_max_;
         graph->set_positions_invalid();
         return *this;
      }

      GraphAxis &GraphAxis::set_max_value(double value)
      {
         if(is_finite(value))
         {
            max_value.bind(new Expression::Operand(value, 0));
            auto_max = false;
         }
         else
         {
            max_value.clear();
            auto_max = true;
         }
         graph->set_positions_invalid();
         return *this;
      }

      GraphAxis &GraphAxis::set_min_value(double value)
      {
         if(is_finite(value))
         {
            min_value.bind(new Expression::Operand(value, 0));
            auto_min = false;
         }
         else
         {
            min_value.clear();
            auto_min = true;
         }
         graph->set_positions_invalid();
         return *this;
      }

      GraphAxis &GraphAxis::set_min_offset(double offset)
      {
         min_offset = offset;
         //auto_min = false;
         graph->set_positions_invalid();
         return *this;
      }

      GraphAxis &GraphAxis::set_max_offset(double offset)
      {
         max_offset = offset;
         graph->set_positions_invalid();
         return *this;
      }
      
      GraphAxis &GraphAxis::set_minor_tick_count(int4 value)
      {
         minor_tick_count = value;
         graph->set_positions_invalid();
         return *this;
      }
      
      GraphAxis &GraphAxis::set_labels_size(double size)
      {
         labels_size = size;
         graph->set_positions_invalid();
         return *this;
      }

      GraphAxis &GraphAxis::set_auto_label(bool value)
      {
         auto_label = value;
         scale->auto_interval = value;
         graph->set_positions_invalid();
         return *this;
      } // set_auto_label


      GraphAxis &GraphAxis::set_fixed_num_labels(bool value)
      {
         use_fixed_num_steps = value;
         if (value)
         {
            auto_label = false;
            scale->auto_interval = false;
         }
         graph->set_positions_invalid();
         return *this;
      }

      GraphAxis &GraphAxis::set_num_fixed_steps(int value)
      {
         num_fixed_steps = value;
         graph->set_positions_invalid();
         return *this;
      }
      
      GraphAxis &GraphAxis::set_increment(double value)
      {
         increment = value;
         auto_label = false;
         scale->auto_interval = false;
         use_fixed_num_steps = false;
         graph->set_positions_invalid();
         return *this;
      } // set_increment


      GraphAxis &GraphAxis::set_increment_units(increment_units_type value)
      {
         increment_units = value;
         auto_label = false;
         scale->auto_interval  = false;
         graph->set_positions_invalid();
         return *this;
      } // set_increment_units


      GraphAxis &GraphAxis::set_increment_with_units(double value, int value_units)
      {
         switch(value_units)
         {
         case increment_seconds:
            increment = value * LgrDate::msecPerSec;
            increment_units = increment_seconds;
            break;
            
         case increment_minutes:
            increment = value * LgrDate::msecPerMin;
            increment_units = increment_minutes;
            break;
            
         case increment_hours:
            increment = value * LgrDate::msecPerHour;
            increment_units = increment_hours;
            break;
            
         case increment_days:
            increment = value * LgrDate::msecPerDay;
            increment_units = increment_days;
            break;
            
         case increment_weeks:
            increment = value * LgrDate::msecPerWeek;
            increment_units = increment_weeks;
            break;
         }
         auto_label = false;
         scale->auto_interval = false;
         graph->set_positions_invalid();
         return *this;
      } // set_increment_with_units

      
      double GraphAxis::get_increment_with_units() const
      {
         double rtn(increment);
         if(get_time_domain())
         {
            switch(increment_units)
            {
            case increment_seconds:
               rtn /= LgrDate::msecPerSec;
               break;

            case increment_minutes:
               rtn /= LgrDate::msecPerMin;
               break;

            case increment_hours:
               rtn /= LgrDate::msecPerHour;
               break;

            case increment_days:
               rtn /= LgrDate::msecPerDay;
               break;

            case increment_weeks:
               rtn /= LgrDate::msecPerWeek;
               break;
            }
         }
         return rtn;
      } // get_increment_with_units


      GraphAxis &GraphAxis::set_log_base(double value)
      {
         scale->log_base = value;
         if(scale->log_base < 1)
            scale->log_base = 10;
         graph->set_positions_invalid();
         return *this;
      } // set_log_base
      
      
      GraphAxis &GraphAxis::set_inverted(bool value)
      {
         scale->inverted = value;
         graph->set_positions_invalid();
         return *this;
      } // set_inverted


      GraphAxis &GraphAxis::set_fixed_decimals(bool value)
      {
         scale->fixed_decimals = value;
         graph->set_positions_invalid();
         return *this;
      } // set_fixed_decimals


      GraphAxis &GraphAxis::set_decimal_places(int value)
      {
         scale->decimal_places = value;
         graph->set_positions_invalid();
         return *this;
      } // set_decimal_places


      GraphAxis &GraphAxis::set_auto_time(bool value)
      {
         auto_time = value;
         graph->set_positions_invalid();
         return *this;
      } // set_auto_time


      GraphAxis &GraphAxis::set_axis_type(axis_type_code value)
      {
         axis_type = value;
         scale->vertical = (axis_type == axis_left || axis_type == axis_right);
         return *this;
      } // set_axis_type


      StrUni GraphAxis::get_axis_type_title() const
      {
         StrUni rtn;
         switch(axis_type)
         {
         case axis_bottom:
            rtn = L"Bottom";
            break;

         case axis_top:
            rtn = L"Top";
            break;

         case axis_left:
            rtn = L"Left";
            break;

         case axis_right:
            rtn = L"Right";
            break;
         }
         return rtn;
      } // get_axis_type_title
      

      void GraphAxis::set_default_properties()
      {
         caption.cut(0);
         caption_angle = 0;
         caption_visible = true;
         auto_min = true;
         auto_max = true;
         min_offset = max_offset = 5;
         increment = -1;
         increment_units = increment_seconds;
         minor_tick_count = 1;
         labels_visible = true;
         labels_colour = black_colour();
         labels_size = 0;
         auto_label = true;
         scale->auto_interval = true;
         use_fixed_num_steps = false;
         num_fixed_steps = 5;
         auto_time = true;
         major_tick_length = 4;
         minor_tick_length = 2;

         min_value.bind(new Expression::Operand(0, 0));
         max_value.bind(new Expression::Operand(100, 0));
         axis_pen = driver->make_pen(PenInfo().set_width(1));
         major_ticks_pen = driver->make_pen(PenInfo().set_width(2).set_colour(dark_gray_colour()));
         minor_ticks_pen = driver->make_pen(PenInfo().set_width(1).set_colour(dark_gray_colour()));
         major_grid_pen = driver->make_pen(
            PenInfo().set_width(1).set_line_type(line_small_dots).set_colour(dark_gray_colour().set_alpha(0x7b)));
         caption_font = driver->make_font(driver->default_font_desc().set_point_size(10));
         caption_colour = black_colour();
         minor_grid_pen = driver->make_pen(
            PenInfo().set_width(1).set_line_type(line_small_dots).set_colour(light_gray_colour().set_alpha(0x7b)).set_visible(false));
         highlights.clear();
         scale->set_default_properties();
      } // set_default_properties


      void GraphAxis::write(Xml::Element &elem)
      {
         Xml::Element::value_type scale_xml(elem.add_element(L"scale"));
         Xml::Element::value_type caption_xml(elem.add_element(L"caption"));
         Xml::Element::value_type caption_font_xml(caption_xml->add_element(L"font"));
         Xml::Element::value_type major_grid_xml(elem.add_element(L"major"));
         Xml::Element::value_type minor_grid_xml(elem.add_element(L"minor"));
         Xml::Element::value_type highlights_xml(elem.add_element(L"highlights"));
         
         scale->write(*scale_xml);
         elem.set_attr_str(time_format, L"time-format");
         caption_font->get_desc().write(*caption_font_xml);
         caption_xml->set_attr_wstr(caption, L"caption");
         caption_xml->set_attr_colour(caption_colour, L"colour");
         caption_xml->set_attr_double(caption_angle, L"angle");
         elem.set_attr_bool(auto_min, L"auto-min");
         elem.set_attr_bool(auto_max, L"auto-max");
         elem.set_attr_bool(auto_time, L"auto-time");
         elem.set_attr_double(increment, L"increment");
         elem.set_attr_int4(increment_units, L"increment-units");
         elem.set_attr_bool(auto_label, L"auto-label");
         elem.set_attr_bool(use_fixed_num_steps, L"fixed-num-steps");
         elem.set_attr_int4(num_fixed_steps, L"num-fixed-steps");
         elem.set_attr_int4(minor_tick_count, L"num-minor-fixed-steps");
         elem.set_attr_colour(labels_colour, L"labels-colour");
         elem.set_attr_bool(labels_visible, L"labels-visible");
         if(min_value != 0)
            elem.set_attr_double(min_value->get_val(), L"min-value");
         else
            elem.set_attr_double(std::numeric_limits<double>::quiet_NaN(), L"min-value");
         if(max_value != 0)
            elem.set_attr_double(max_value->get_val(), L"max-value");
         else
            elem.set_attr_double(std::numeric_limits<double>::quiet_NaN(), L"min-value");
         elem.set_attr_double(min_offset, L"min-offset");
         elem.set_attr_double(max_offset, L"max-offset");
         if(major_grid_pen != 0)
         {
            Xml::Element::value_type pen_xml(major_grid_xml->add_element(L"grid"));
            major_grid_pen->get_desc().write(*pen_xml);
         }
         if(major_ticks_pen != 0)
         {
            Xml::Element::value_type pen_xml(major_grid_xml->add_element(L"ticks"));
            major_ticks_pen->get_desc().write(*pen_xml);
            pen_xml->set_attr_int4(major_tick_length, L"tick_length");
         }
         if(minor_grid_pen != 0)
         {
            Xml::Element::value_type pen_xml(minor_grid_xml->add_element(L"grid"));
            minor_grid_pen->get_desc().write(*pen_xml);
         }
         if(minor_ticks_pen != 0)
         {
            Xml::Element::value_type pen_xml(minor_grid_xml->add_element(L"ticks"));
            minor_ticks_pen->get_desc().write(*pen_xml);
            pen_xml->set_attr_int4(minor_tick_length, L"tick_length");
         }
         if(axis_pen != 0)
         {
            Xml::Element::value_type pen_xml(elem.add_element(L"axis-pen"));
            axis_pen->get_desc().write(*pen_xml);
         }
         for(highlights_type::iterator hi = highlights.begin(); hi != highlights.end(); ++hi)
         {
            GraphAxisHighlight const &highlight(*hi);
            Xml::Element::value_type highlight_xml(highlights_xml->add_element(L"highlight"));
            Xml::Element::value_type highlight_brush(highlight_xml->add_element(L"brush"));
            highlight_xml->set_attr_double(highlight.start->get_val(), L"start");
            highlight_xml->set_attr_double(highlight.stop->get_val(), L"stop");
            highlight.brush->get_desc().write(*highlight_brush);
         }
      } // write


      void GraphAxis::read(Xml::Element &elem)
      {
         Xml::Element::value_type scale_xml(elem.find_elem(L"scale"));
         Xml::Element::value_type caption_xml(elem.find_elem(L"caption"));
         Xml::Element::value_type caption_font_xml(caption_xml->find_elem(L"font"));
         FontInfo caption_font_desc;
         Xml::Element::value_type major_grid_xml(elem.find_elem(L"major"));
         Xml::Element::value_type minor_grid_xml(elem.find_elem(L"minor"));
         Xml::Element::value_type axis_pen_xml(elem.find_elem(L"axis-pen"));
         Xml::Element::value_type highlights_xml(elem.find_elem(L"highlights"));

         scale->read(*scale_xml);
         time_format = elem.get_attr_str(L"time-format");
         caption_font_desc.read(*caption_font_xml);
         caption_font = driver->make_font(caption_font_desc);
         caption = caption_xml->get_attr_str(L"caption");
         caption_colour = caption_xml->get_attr_colour(L"colour");
         caption_angle = caption_xml->get_attr_double(L"angle");
         auto_min = elem.get_attr_bool(L"auto-min");
         auto_max = elem.get_attr_bool(L"auto-max");
         auto_time = elem.get_attr_bool(L"auto-time");
         increment = elem.get_attr_double(L"increment");
         if(elem.has_attribute(L"increment-units"))
            increment_units = static_cast<increment_units_type>(elem.get_attr_int4(L"increment-units"));
         auto_label = elem.get_attr_bool(L"auto-label");
         scale->auto_interval = auto_label;
         try {use_fixed_num_steps = elem.get_attr_bool(L"fixed-num-steps");} catch(std::exception &) {}
         try {num_fixed_steps = elem.get_attr_int4(L"num-fixed-steps");} catch(std::exception &) {}
         try {minor_tick_count = elem.get_attr_int4(L"num-minor-fixed-steps");} catch(std::exception &) {}


         min_value.bind(new Expression::Operand(elem.get_attr_double(L"min-value"), 0));
         max_value.bind(new Expression::Operand(elem.get_attr_double(L"max-value"), 0));
         min_offset = elem.get_attr_double(L"min-offset");
         max_offset = elem.get_attr_double(L"max-offset");
         if(elem.has_attribute(L"labels-colour"))
            labels_colour = elem.get_attr_colour(L"labels-colour");
         if(elem.has_attribute(L"labels-visible"))
            labels_visible = elem.get_attr_bool(L"labels-visible");
         major_grid_pen.clear();
         major_ticks_pen.clear();
         for(Xml::Element::iterator mgi = major_grid_xml->begin();
             mgi != major_grid_xml->end();
             ++mgi)
         {
            Xml::Element::value_type &child(*mgi);
            if(child->get_name() == L"grid")
            {
               PenInfo info;
               info.read(*child);
               major_grid_pen = driver->make_pen(info);
            }
            else if(child->get_name() == L"ticks")
            {
               PenInfo new_color;
               new_color.read(*child);
               PenInfo major_ticks_desc;
               if (major_ticks_pen != 0) 
                  major_ticks_desc= major_ticks_pen->get_desc();
               major_ticks_desc.set_colour(new_color.get_colour());
               major_ticks_desc.set_width(new_color.get_width());
               major_ticks_pen = driver->make_pen(major_ticks_desc);

               try {
                  major_tick_length = child->get_attr_int4(L"tick_length");
               }
               catch (...)
               { }
            }
         }
         minor_grid_pen.clear();
         minor_ticks_pen.clear();
         for(Xml::Element::iterator mgi = minor_grid_xml->begin();
             mgi != minor_grid_xml->end();
             ++mgi)
         {
            Xml::Element::value_type &child(*mgi);
            if(child->get_name() == L"grid")
            {
               PenInfo info;
               info.read(*child);
               minor_grid_pen = driver->make_pen(info);
            }
            else if(child->get_name() == L"ticks")
            {
               PenInfo new_color;
               new_color.read(*child);
               PenInfo minor_ticks_desc;
               if (minor_ticks_pen != 0) 
                  minor_ticks_desc = major_ticks_pen->get_desc();
               minor_ticks_desc.set_colour(new_color.get_colour());
               minor_ticks_desc.set_width(new_color.get_width());
               minor_ticks_pen = driver->make_pen(minor_ticks_desc);

               try {
                  minor_tick_length = child->get_attr_int4(L"tick_length");
               }
               catch (...)
               { }

            }
         }

         axis_pen.clear();
         PenInfo new_color;
         new_color.read(*axis_pen_xml);
         PenInfo axis_pen_desc;
         if (axis_pen != 0)
            axis_pen_desc = axis_pen->get_desc();
         axis_pen_desc.set_colour(new_color.get_colour());
         axis_pen_desc.set_visible(new_color.get_visible());
         axis_pen = driver->make_pen(axis_pen_desc);

         highlights.clear();
         for(Xml::Element::iterator hi = highlights_xml->begin();
             hi != highlights_xml->end();
             ++hi)
         {
            Xml::Element::value_type &highlight_xml(*hi);
            GraphAxisHighlight highlight;
            Xml::Element::value_type brush_xml(highlight_xml->find_elem(L"brush"));
            BrushInfo brush_info;
            brush_info.read(*brush_xml);
            highlight.brush = driver->make_brush(brush_info);
            highlight.start.bind(new Expression::Operand(highlight_xml->get_attr_double(L"start"), 0));
            highlight.stop.bind(new Expression::Operand(highlight_xml->get_attr_double(L"stop"), 0));
         }
      } // read
      
      
      GraphAxis::layout_handle &GraphAxis::generate_axis_rect(double space)
      {
         axis_rect.bind(new NestedRect);
         scale->labels.clear();
         minor_ticks.clear();
         caption_rect.clear();
         if(!traces.empty())
         {
            // we need to lay out the title rect.
            if(caption_visible && caption_font != 0 && caption.length() > 0)
               caption_rect.bind(new NestedRect(driver->measure_text(caption, caption_font, caption_angle)));

            // we need to evaluate the min and max values for this axis.
            double local_max(-std::numeric_limits<double>::infinity());
            double local_min(std::numeric_limits<double>::infinity());
            uint4 valid_count(0);
            if(!auto_min)
            {
               if(scale->time_domain && auto_time)
                  local_min = static_cast<double>(graph->get_newest_time() - graph->get_display_width());
               else
                  local_min = min_value->get_val();
            }
            if(!auto_max)
            {
               if (scale->time_domain && auto_time)
                  local_max = static_cast<double>(graph->get_newest_time());
               else
                  local_max = max_value->get_val();
            }

            if (!auto_min && !auto_max)
               valid_count = 1;

            for(iterator ti = traces.begin(); (auto_min || auto_max) && ti != traces.end(); ++ti)
            {
               trace_handle &trace(*ti);
               if(trace->get_visible())
               {
                  GraphTrace::bounds_type bounds(trace->get_bounds(domain_axis));
                  if(is_finite(bounds.first) && is_finite(bounds.second))
                  {
                     if(auto_min && bounds.first < local_min)
                        local_min = bounds.first;
                     if(auto_max && bounds.second > local_max)
                        local_max = bounds.second;
                     ++valid_count;
                  }
               }
            }
            if(valid_count == 0)
            {
               if(scale->time_domain)
               {
                  local_max = static_cast<double>(LgrDate::system().get_nanoSec() / LgrDate::nsecPerMSec);
                  local_min = local_max - LgrDate::msecPerHour;
               }
               else
               {
                  if (!is_finite(local_min) && !is_finite(local_max))
                  {
                     local_max = 0.5;
                     local_min = -0.5;
                  }
                  else if (!is_finite(local_max))
                  {
                     local_max = max_value->get_val();
                     //local_max = local_min + 1;
                  }
                  else if (!is_finite(local_min))
                  {
                     local_min = min_value->get_val();
                     //local_min = local_max - 1;
                  }
               }
            }

            // we need to apply sanity checking for the local max and min values
            if(local_max < local_min)
               std::swap(local_max, local_min);
            if(scale->time_domain && fabs(local_max - local_min) < 10)
               local_max = local_min + 10;
            if(!scale->time_domain && local_max == local_min)
               local_max = local_min + 0.01;

            // we need to generate the rectangle for the axis scale.
            if(scale->vertical)
            {
               double scale_rect_width(1);
               if(axis_pen->get_visible())
                  scale_rect_width += axis_pen->get_width();
               if(major_ticks_pen->get_visible() && major_tick_length > 0)
                  scale_rect_width += major_tick_length;
               else if (minor_ticks_pen->get_visible() && minor_tick_length > 0)
                  scale_rect_width += minor_tick_length;
               scale_rect.bind(new NestedRect(scale_rect_width, space));
            }
            else
            {
               double scale_rect_height(1);
               if(axis_pen->get_visible())
                  scale_rect_height += axis_pen->get_width();
               if(major_ticks_pen->get_visible())
                  scale_rect_height += major_tick_length;
               else if(minor_ticks_pen->get_visible())
                  scale_rect_height += minor_tick_length;
               scale_rect.bind(new NestedRect(space, scale_rect_height));
            }

            // if the time format string is not specified, we will set the scale's format
            // automatically.
            if(scale->time_domain && time_format.length() == 0)
            {
               double range(local_max - local_min);
               if(range > LgrDate::msecPerDay)
                  scale->time_format = "%m-%d %H:%M";
               else if(range > LgrDate::msecPerHour)
                  scale->time_format = "%H:%M";
               else if(range > LgrDate::msecPerMin)
                  scale->time_format = "%H:%M:%S";
               else
                  scale->time_format = "%H:%M:%S%x";
            }
            else
               scale->time_format = time_format;

            // if the label interval is specified manually, we will need to manually set it for the
            // scale.
            if (!auto_label && !use_fixed_num_steps && increment > 0)
            {
               scale->label_interval.bind(new Expression::Operand(increment, 0));
            }
            else if (!auto_label && use_fixed_num_steps && num_fixed_steps > 0)
            {
               double range(local_max - local_min);
               double step_size(range / (num_fixed_steps-1));
               if(step_size <= 1e-38)
                  step_size = 1;

               scale->label_interval.bind(new Expression::Operand(step_size, 0));
            }

            // we need to apply max and min offsets
            scale->size = space;

            double min_offset_with_bars = min_offset;
            double max_offset_with_bars = max_offset;

            // Add in room for the bar widths, so that the bars do not get chopped off. 
            // Note we will not do this if we are zooming or scrolling (i.e., if state_backup 
            // is non-null).  This is so that scrolling won't result in smaller and smaller spans.
            if (domain_axis && graph->state_backup == nullptr)
            {
               double bar_width = graph->get_bar_width();
               double bar_count = graph->get_bar_count();

               if (bar_count > 0 && bar_width > 0)
               {
                  // If there is space between the bars (bar_width_ratio < 1), then
                  // leave the same space at the edges as there are between the bar groups.
                  if (graph->bar_width_ratio > 0 && graph->bar_width_ratio < 1)
                     bar_width /= graph->bar_width_ratio;

                  // Make room for the bar
                  min_offset_with_bars += (bar_width) / 2.f;
                  max_offset_with_bars += (bar_width) / 2.f;
                  
                  // Make room for the borders drawn around the bars and the axis lines
                  min_offset_with_bars += 4;
                  max_offset_with_bars += 4;
               }
            }
            
            if(max_offset_with_bars >= 0 || min_offset_with_bars >= 0)
            {
               value_handle temp_min(new Expression::Operand(local_min));
               value_handle temp_max(new Expression::Operand(local_max));
               scale->calculate_scale(temp_max, temp_min);
               if(max_offset_with_bars >= 0)
                  local_max += scale->offset_weight(temp_max, max_offset_with_bars);
               if(min_offset_with_bars >= 0)
               {
                  if (!get_logarithmic())
                     local_min -= scale->offset_weight(temp_min, min_offset_with_bars);
                  else
                  {
                     double orig_local_min = local_min;
                     local_min -= scale->offset_weight(temp_min, -min_offset_with_bars);
                     if (local_min < 1)
                     {
                        local_min = 1;
                        if (orig_local_min > local_min)
                           min_offset_with_bars -= scale->offset_to_size(orig_local_min - local_min);
                        scale->size -= min_offset_with_bars;
                     }
                  }
               }
            }

            // we can finally generate the labels
            labels_rect.bind(new NestedRect);
            scale->generate_labels(
               new Expression::Operand(local_max, 0),
               new Expression::Operand(local_min));
            for(Scale::iterator li = scale->begin(); li != scale->end(); ++li)
            {
               Scale::value_type &label(*li);
               labels_rect->add_child(label->rect);
            }

            // Now, after the offsets are calculated in, recalculate the bar width.
            // Our labels are little closer together now, and we want to make
            // sure that 100% will not overlap with each other.
            if (domain_axis && graph->state_backup == nullptr)
            {
               graph->calculate_bar_width();
            }


            // we also need to generate the minor ticks
            bool minor_ticks_visible(
               (minor_grid_pen != 0 && minor_grid_pen->get_visible()) ||
               (minor_ticks_pen != 0 && minor_ticks_pen->get_visible()));
            if (!auto_label && minor_tick_count == 0)
               minor_ticks_visible = false;
            if(minor_ticks_visible)
            {
               double minor_tick_width(0);
               if(minor_ticks_pen != 0 && minor_ticks_pen->get_visible())
                  minor_tick_width = minor_ticks_pen->get_width();
               if(minor_grid_pen != 0 && minor_grid_pen->get_visible() && minor_grid_pen->get_width() > minor_tick_width)
                  minor_tick_width = minor_grid_pen->get_width();
               if(minor_tick_width > 0)
               {
                  int4 tick_count(minor_tick_count);
                  if(auto_label)
                     tick_count = - 1; // this will force it to auto calculate the best interval 
                  scale->generate_minor_ticks(minor_ticks, minor_tick_width * 2, tick_count);
               }
            }

            // with everything generated, we need to position the component rectangles within the
            // axis rectangle.
            if(axis_type == axis_bottom)
            {
               axis_rect->add_child(scale_rect);
               if(labels_visible)
               {
                  axis_rect->add_child(new NestedRect(small_margin, small_margin));
                  axis_rect->add_child(labels_rect);
                  axis_rect->add_child(new NestedRect(small_margin, small_margin));
                  if(labels_rect->get_height() < labels_size)
                  {
                     axis_rect->add_child(
                        Rect(
                           labels_rect->get_left(false), labels_rect->get_top(false),
                           labels_rect->get_width(),
                           labels_size - labels_rect->get_height()));
                  }
               }
               if(caption_rect != 0)
               {
                  caption_rect->centre_x(space / 2);
                  axis_rect->add_child(caption_rect);
               }
               axis_rect->stack_vertical();
            }
            else if(axis_type == axis_top)
            {
               if(caption_rect != 0)
               {
                  caption_rect->centre_x(space / 2);
                  axis_rect->add_child(caption_rect);
               }
               if(labels_visible)
               {
                  if(labels_rect->get_height() < labels_size)
                  {
                     axis_rect->add_child(
                        Rect(
                           labels_rect->get_left(false), labels_rect->get_top(false),
                           labels_rect->get_width(), labels_size - labels_rect->get_height()));
                  }
                  axis_rect->add_child(new NestedRect(small_margin, small_margin));
                  axis_rect->add_child(labels_rect);
                  axis_rect->add_child(new NestedRect(small_margin, small_margin));
               }
               axis_rect->add_child(scale_rect);
               axis_rect->stack_vertical();
            }
            else if(axis_type == axis_left)
            {
               if(caption_rect != 0)
               {
                  caption_rect->centre_y(space / 2);
                  axis_rect->add_child(caption_rect);
               }
               if(labels_visible)
               {
                  if(labels_rect->get_width() < labels_size)
                  {
                     axis_rect->add_child(
                        Rect(
                           labels_rect->get_left(false), labels_rect->get_top(false),
                           labels_size - labels_rect->get_width(), labels_rect->get_height()));
                  }
                  axis_rect->add_child(new NestedRect(small_margin, small_margin));
                  axis_rect->add_child(labels_rect);
               }
               axis_rect->add_child(new NestedRect(small_margin, small_margin));
               axis_rect->add_child(scale_rect);
               axis_rect->stack_horizontal();
            }
            else if(axis_type == axis_right)
            {
               axis_rect->add_child(scale_rect);
               if(labels_visible)
               {
                  axis_rect->add_child(new NestedRect(small_margin, small_margin));
                  axis_rect->add_child(labels_rect);
                  axis_rect->add_child(new NestedRect(small_margin, small_margin));
                  if(labels_rect->get_width() < labels_size)
                  {
                     axis_rect->add_child(
                        Rect(
                           labels_rect->get_left(false), labels_rect->get_top(false),
                           labels_size - labels_rect->get_width(), labels_rect->get_height()));
                  }
                  if(caption_rect != 0)
                  {
                     caption_rect->centre_y(space / 2);
                     axis_rect->add_child(caption_rect);
                  }
                  axis_rect->stack_horizontal();
               }
            }
         }
         return axis_rect;
      } // generate_axis_rect


      double GraphAxis::value_to_pos(value_handle value)
      {
         double rtn(0);
         if(scale_rect != 0)
         {
            double pos(scale->value_to_pos(value));
            switch(axis_type)
            {
            case axis_bottom:
            case axis_top:
               rtn = scale_rect->translate_to_point(pos, 0).x;
               break;
               
            case axis_left:
            case axis_right:
               rtn = scale_rect->translate_to_point(0, pos).y;
               break;
            }
         }
         return rtn;
      } // value_to_pos


      GraphAxis::value_handle GraphAxis::pos_to_value(double pos)
      {
         value_handle rtn(new Expression::Operand);
         if(scale_rect != 0)
         {
            double point(0);
            switch(axis_type)
            {
            case axis_bottom:
            case axis_top:
               point = scale_rect->translate_from_point(pos, 0).x;
               break;

            case axis_left:
            case axis_right:
               point = scale_rect->translate_from_point(0, pos).y;
               break;
            }
            rtn = scale->pos_to_value(point);
         }
         return rtn;
      } // pos_to_value


      namespace
      {
         struct highlight_has_owner
         {
            void const *owner;
            highlight_has_owner(void const *owner_):
               owner(owner_)
            { }

            bool operator ()(GraphAxisHighlight const &highlight) const
            { return highlight.owner == owner; }
         };
      };
      

      GraphAxis &GraphAxis::remove_highlight(void const *owner)
      {
         highlights_type::iterator new_end(
            std::remove_if(highlights.begin(), highlights.end(), highlight_has_owner(owner)));
         if(new_end != highlights.end())
            highlights.erase(new_end, highlights.end());
         return *this;
      } // remove_highlight

      
      void GraphAxis::draw_highlights()
      {
         Rect plot_rect(graph->get_plot_rect()->get_rect(true));
         driver->set_clipping_region(plot_rect);
         for(highlights_type::const_iterator hi = highlights.begin();
             hi != highlights.end();
             ++hi)
         {
            GraphAxisHighlight const &highlight(*hi);
            double start_pos(value_to_pos(highlight.start));
            double stop_pos(value_to_pos(highlight.stop));
            if(is_finite(start_pos) && is_finite(stop_pos))
            {
               if(start_pos > stop_pos)
                  std::swap(start_pos, stop_pos);
               if(axis_type == axis_bottom || axis_type == axis_top)
               {
                  Rect rect(
                     start_pos, plot_rect.get_top(),
                     stop_pos - start_pos, plot_rect.height);
                  if(rect.width < 1)
                     rect.width = 1;
                  if(plot_rect.intersects(rect))
                     driver->draw_rect(rect, 0, highlight.brush);
               }
               else
               {
                  Rect rect(
                     plot_rect.get_left(), start_pos,
                     plot_rect.width, stop_pos - start_pos);
                  if(rect.height < 1)
                     rect.height = 1;
                  if(plot_rect.intersects(rect))
                     driver->draw_rect(rect, 0, highlight.brush);
               }
            }
         }
         driver->remove_clipping_region();
      } // draw_highlights
      

      void GraphAxis::draw_caption()
      {
         if(caption_font != 0 && caption_rect != 0 && caption_visible)
         {
            driver->draw_text(
               caption,
               caption_rect->get_rect(true),
               caption_font,
               caption_colour,
               transparent_colour(),
               caption_angle);
         }
      } // draw_caption

      
      void GraphAxis::draw_axis()
      {
         if(axis_pen != nullptr && axis_pen->get_visible() && !traces.empty())
         {
            double x1, y1, x2, y2;
            Graph::rect_handle plot_rect(graph->get_plot_rect());
            switch(axis_type)
            {
            case axis_bottom:
               y1 = y2 = plot_rect->get_bottom(true);
               x1 = plot_rect->get_left(true);
               x2 = plot_rect->get_right(true);
               break;

            case axis_top:
               y1 = y2 = plot_rect->get_top(true);
               x1 = plot_rect->get_left(true);
               x2 = plot_rect->get_right(true);
               break;

            case axis_left:
               x1 = x2 = plot_rect->get_left(true);
               y1 = plot_rect->get_top(true);
               y2 = plot_rect->get_bottom(true);
               break;

            case axis_right:
               x1 = x2 = plot_rect->get_right(true);
               y1 = plot_rect->get_top(true);
               y2 = plot_rect->get_bottom(true);
               break;
            }
            driver->draw_line(Point(x1, y1), Point(x2, y2), axis_pen);
         }
      } // draw_axis


      void GraphAxis::draw_labels()
      {
         if(labels_visible && labels_rect != 0)
         {
            for(Scale::iterator li = scale->begin(); li != scale->end(); ++li)
            {
               Scale::value_type &label(*li);
               driver->draw_text(
                  label->label,
                  label->rect->get_rect(true),
                  scale->font,
                  labels_colour,
                  transparent_colour(),
                  scale->rotation);
            }
         }
      } // draw_labels


      void GraphAxis::draw_major_ticks()
      {
         if(major_ticks_pen != 0 && major_ticks_pen->get_visible() && scale_rect != 0 && major_tick_length > 0)
         {
            for(Scale::iterator li = scale->begin(); li != scale->end(); ++li)
            {
               Scale::value_type &label(*li);
               double pos(value_to_pos(label->value));
               double x1, y1, x2, y2;
               switch(axis_type)
               {
               case axis_bottom:
               case axis_top:
                  x1 = x2 = pos;
                  y1 = scale_rect->get_top(true);
                  y2 = scale_rect->get_bottom(true);
                  break;

               case axis_left:
               case axis_right:
                  y1 = y2 = pos;
                  x1 = scale_rect->get_left(true);
                  x2 = scale_rect->get_right(true);
                  break;
               }
               driver->draw_line(Point(x1, y1), Point(x2, y2), major_ticks_pen);
            }
         }
      } // draw_major_ticks


      void GraphAxis::draw_minor_ticks()
      {
         if(minor_ticks_pen != 0 && minor_ticks_pen->get_visible() && minor_tick_length > 0)
         {
            for(minor_ticks_type::iterator ti = minor_ticks.begin(); ti != minor_ticks.end(); ++ti)
            {
               double tick(*ti);
               double pos(value_to_pos(new Expression::Operand(tick, 0)));
               double x1, y1, x2, y2;
               switch(axis_type)
               {
               case axis_bottom:
                  x1 = x2 = pos;
                  y1 = scale_rect->get_top(true);
                  y2 = scale_rect->get_top(true) + minor_tick_length;
                  break;

               case axis_top:
                  x1 = x2 = pos;
                  y1 = scale_rect->get_bottom(true);
                  y2 = scale_rect->get_bottom(true) - minor_tick_length;
                  break;

               case axis_left:
                  y1 = y2 = pos;
                  x1 = scale_rect->get_right(true);
                  x2 = scale_rect->get_right(true) - minor_tick_length;
                  break;

               case axis_right:
                  y1 = y2 = pos;
                  x1 = scale_rect->get_left(true);
                  x2 = scale_rect->get_left(true) + minor_tick_length;
                  break;
               }
               driver->draw_line(Point(x1, y1), Point(x2, y2), minor_ticks_pen);
            }
         }
      } // draw_minor_ticks


      void GraphAxis::draw_major_grid()
      {
         if(major_grid_pen != 0 && major_grid_pen->get_visible())
         {
            Graph::rect_handle plot_rect(graph->get_plot_rect());
            for(Scale::iterator li = scale->begin(); li != scale->end(); ++li)
            {
               Scale::value_type &label(*li);
               double pos(value_to_pos(label->value));
               double x1, y1, x2, y2;
               switch(axis_type)
               {
               case axis_bottom:
               case axis_top:
                  x1 = x2 = pos;
                  y1 = plot_rect->get_top(true);
                  y2 = plot_rect->get_bottom(true);
                  break;

               case axis_left:
               case axis_right:
                  y1 = y2 = pos;
                  x1 = plot_rect->get_left(true);
                  x2 = plot_rect->get_right(true);
                  break;
               }
               driver->draw_line(Point(x1, y1), Point(x2, y2), major_grid_pen);
            }
         }
      } // draw_major_grid


      void GraphAxis::draw_minor_grid()
      {
         if(minor_grid_pen != 0 && minor_grid_pen->get_visible())
         {
            Graph::rect_handle plot_rect(graph->get_plot_rect());
            for(minor_ticks_type::iterator ti = minor_ticks.begin(); ti != minor_ticks.end(); ++ti)
            {
               double tick(*ti);
               double pos(value_to_pos(new Expression::Operand(tick, 0)));
               double x1, y1, x2, y2;
               switch(axis_type)
               {
               case axis_bottom:
               case axis_top:
                  x1 = x2 = pos;
                  y1 = plot_rect->get_top(true);
                  y2 = plot_rect->get_bottom(true);
                  break;

               case axis_left:
               case axis_right:
                  y1 = y2 = pos;
                  x1 = plot_rect->get_left(true);
                  x2 = plot_rect->get_right(true);
                  break;
               }
               driver->draw_line(Point(x1, y1), Point(x2, y2), minor_grid_pen);
            }
         }
      } // draw_minor_grid


      void GraphAxis::draw(bool draw_grid)
      {
         draw_caption();
         draw_axis();
         draw_labels();
         draw_minor_ticks();
         draw_major_ticks();
         if(draw_grid)
         {
            draw_major_grid();
            draw_minor_grid();
         }
      } // draw
   };
};

