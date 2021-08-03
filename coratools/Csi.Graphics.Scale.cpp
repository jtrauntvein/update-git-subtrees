/* Csi.Graphics.Scale.cpp

   Copyright (C) 2015, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 14 January 2015
   Last Change: Friday 29 January 2016
   Last Commit: $Date: 2020-07-14 15:42:11 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Graphics.Scale.h"
#include "Csi.StrUniStream.h"
#include "Csi.StringLoader.h"
#include "Csi.BuffStream.h"
#include "CsiTypes.h"
#include "boost/format.hpp"
#include <regex>
#include <math.h>


namespace Csi
{
   namespace Graphics
   {
      namespace
      {
         /**
          * Defines a function that will clear out the specified decimal position and round if
          * needed.
          *
          * @param s Specifies a string formatted as a decimal.
          *
          * @param pos Specifies the position to consider
          */
         void clear_with_carry(StrAsc &s, size_t pos)
         {
            if(pos < s.length() && pos > 0)
            {
               size_t previous_pos(pos - 1);
               wchar_t decimal_point(L'.');
               
               if(previous_pos > 0 && s[previous_pos] == decimal_point)
                  --previous_pos;
               s[pos] = '0';
               if(previous_pos >= 0)
               {
                  if(s[previous_pos] != '9')
                     s[previous_pos] = s[previous_pos] + 1;
                  else
                     clear_with_carry(s, previous_pos);
               }
               else
                  s[0] = L'1';
            }
            else
            {
               if(pos < s.length())
                  s[0] = '0';
               s.insert("1", 0);
            }
         }


         /**
          * @return Returns the position of the least significant digit in the specified formatted
          * decimal string.  This is defined as the first non-zero digit (ignoring the decimal
          * point) from the right hand side of the string.  Returns a value greater than or equal to
          * the string length if no such digit could be found.
          *
          * @param s Specifies the string to evaluate.
          */
         size_t last_decimal_pos(StrAsc const &s)
         {
            size_t rtn(s.length() - 1);
            while(rtn > 0 && rtn < s.length())
            {
               if(s[rtn] >= '1' && s[rtn] <= '9')
                  break;
               else
                  --rtn;
            }
            if(rtn < s.length() && (s[rtn] < '1' || s[rtn] > '9'))
               rtn = s.length();
            return rtn;
         }


         int8 const month_interval    (LgrDate::msecPerWeek * (int8) 4);
         int8 const quarter_interval  (LgrDate::msecPerWeek * (int8)13);
         int8 const half_year_interval(LgrDate::msecPerWeek * (int8)26);
         int8 const year_interval     (LgrDate::msecPerWeek * (int8)52);
      };

      
      Scale::Scale(driver_handle driver_):
         size(-1),
         scale(-1),
         driver(driver_),
         auto_interval(false)
      { set_default_properties(); }


      Scale::~Scale()
      { }


      StrAsc const &Scale::format_label(value_handle value, bool for_mark)
      {
         scratch.imbue(StringLoader::make_locale());
         scratch.str("");
         if(!for_mark || logarithmic)
         {
            if(!time_domain)
            {
               StrAsc format("%.7g");
               if(fixed_decimals)
               {
                  scratch << "%." << decimal_places << "f";
                  format = scratch.str();
                  scratch.str("");
               }
               if(logarithmic)
                  scratch << boost::format(format.c_str()) % pow(log_base, value->get_val());
               else
                  scratch << boost::format(format.c_str()) % value->get_val();
            }
            else
            {
               LgrDate stamp(value->get_val_int() * LgrDate::nsecPerMSec);
               stamp.format(scratch, time_format);
            }
         }
         else
         {
            if(!time_domain)
               csiFloatToStream(scratch, value->get_val(), 7);
            else
            {
               LgrDate stamp(value->get_val_int() * LgrDate::nsecPerMSec);
               stamp.format(scratch, "%Y-%m-%d %H:%M:%S%x");
            }
         }
         return scratch.str();
      }


      void Scale::set_default_properties()
      {
         logarithmic = false;
         log_base = 10;
         fixed_decimals = false;
         decimal_places = 7;
         rotation = 0;
         time_format = "%Y-%m-%d %H:%M:%S%x";
         inverted = false;
         font = driver->make_font(driver->default_font_desc().set_point_size(10));
         time_domain = !vertical;
      } // set_default_properties


      void Scale::write(Xml::Element &elem)
      {
         Xml::Element::value_type font_xml(elem.add_element(L"font"));
         elem.set_attr_bool(logarithmic, L"logarithmic");
         elem.set_attr_double(log_base, L"log-base");
         elem.set_attr_bool(fixed_decimals, L"fixed-decimals");
         elem.set_attr_int4(decimal_places, L"decimal-places");
         font->get_desc().write(*font_xml);
         elem.set_attr_double(rotation, L"rotation");
         elem.set_attr_bool(time_domain, L"time-domain");
         elem.set_attr_bool(inverted, L"inverted");
      } // write


      void Scale::read(Xml::Element &elem)
      {
         Xml::Element::value_type font_xml(elem.find_elem(L"font"));
         FontInfo font_desc;
         logarithmic = elem.get_attr_bool(L"logarithmic");
         log_base = elem.get_attr_double(L"log-base");
         fixed_decimals = elem.get_attr_bool(L"fixed-decimals");
         decimal_places = elem.get_attr_int4(L"decimal-places");
         rotation = elem.get_attr_double(L"rotation");
         if(elem.has_attribute(L"inverted"))
            inverted = elem.get_attr_bool(L"inverted");
         font_desc.read(*font_xml);
         font = driver->make_font(font_desc);
      } // read
      

      double Scale::calculate_scale(value_handle &max_value_, value_handle &min_value_)
      {
         // we need to determine the range for the min and max values.
         double local_min(min_value_->get_val());
         double local_max(max_value_->get_val());
         double range(fabs(local_max - local_min));
         one_tick_only = false;
         if(!logarithmic && range < 1e-38)
         {
            local_max = local_min + 0.5;
            local_min = local_max - 1.0;
            range = 1.0;
            one_tick_only = true;
         }

         // if this scale is logarithmic, we will need to calculate the log of the local max and min
         // values.
         else if(logarithmic)
         {
            if(local_min <= 1)
               local_min = 1;
            if(local_max < local_min + 1)
               local_max = local_min + 1;

            local_max = log(local_max) / log(log_base);
            local_min = log(local_min) / log(log_base);
            range = fabs(local_max - local_min);

            if (range < 1)
            {
               local_max = local_min + 1;
               range = 1;
            }
         }

         scratch.imbue(StringLoader::make_locale(0));

         // we can now format the scale and the interval and then use this to calculate the scale
         // and interval.

         StrUni format_min, format_max;
         if (!logarithmic && !auto_interval && (label_interval != 0) && (label_interval->get_val() > 0))
         {
            double txt_min_ = local_min;
            double txt_max_ = local_max;
            if (txt_min_ <= 0)
               txt_min_ = txt_min_ + fmod(fabs(txt_min_), label_interval->get_val());
            else
               txt_min_ = txt_min_ - fmod(fabs(txt_min_), label_interval->get_val());

            if (txt_max_ <= 0)
               txt_max_ = txt_max_ + fmod(fabs(txt_max_), label_interval->get_val());
            else
               txt_max_ = txt_max_ - fmod(fabs(txt_max_), label_interval->get_val());

            format_min = format_label(new Expression::Operand(txt_min_, min_value_->get_timestamp()));
            format_max = format_label(new Expression::Operand(txt_max_, max_value_->get_timestamp()));
         }
         else
         {
            format_min = format_label(new Expression::Operand(local_min, min_value_->get_timestamp()));
            format_max = format_label(new Expression::Operand(local_max, max_value_->get_timestamp()));
         }

         Rect min_rect(driver->measure_text(format_min, font, rotation));
         Rect max_rect(driver->measure_text(format_max, font, rotation));
         double label_req(csimax(min_rect.width, max_rect.width) + 3);
         if(vertical)
            label_req = csimax(min_rect.height, max_rect.height) + 3;
         scale = size / range;

         min_value.bind(new Expression::Operand(local_min, min_value_->get_timestamp()));
         max_value.bind(new Expression::Operand(local_max, max_value_->get_timestamp()));

         if (logarithmic)
         {
            label_interval.bind(new Expression::Operand(1, 0));
         }
         else
         {
            if (auto_interval || (label_interval == 0) || (label_interval->get_val() <= 0))
            {
               if(!time_domain)
                  label_interval = scalar_optimum_step_size(range, size, label_req);
               else
                  label_interval = time_optimum_step_size(range, size, label_req);
            }
            else
            {
               double multiple = 2.5;

               value_handle label_interval_tight = label_interval;
               while ((range / label_interval_tight->get_val()) * label_req >= size && multiple >= 1)
               {
                  value_handle label_interval_auto_calculated;
                  if (time_domain)
                     label_interval_auto_calculated = time_optimum_step_size(range, size * multiple, label_req);
                  else
                     label_interval_auto_calculated = scalar_optimum_step_size(range, size * multiple, label_req);

                  if (label_interval_tight->get_val() < label_interval_auto_calculated->get_val())
                     label_interval_tight = label_interval_auto_calculated;
                  //else if (label_interval->get_val() >= label_interval_tight->get_val())
                  //   break;
                  multiple -= 0.5;
               }

               if (label_interval->get_val() < label_interval_tight->get_val())
                  label_interval = label_interval_tight;
            }
         }
         return scale;
      } // calculate_scale


      Scale::labels_type &Scale::generate_labels(value_handle max_value_, value_handle min_value_)
      {
         // we need to calculate the position of the first tick
         double first_tick(min_value_->get_val());
         if (logarithmic)
            first_tick = ceil(log(first_tick) / log(log_base));
         calculate_scale(max_value_, min_value_);

         if(!one_tick_only)
         {
            if (/*auto_interval && */(label_interval->get_val() > 0))
            {
               if(first_tick <= 0)
                  first_tick = first_tick + fmod(fabs(first_tick), label_interval->get_val());
               else
                  first_tick = first_tick - fmod(fabs(first_tick), label_interval->get_val());

               if (time_domain && label_interval->get_val() >= year_interval)
               {
                  LgrDate first_tick_date(static_cast<int8>(first_tick) * LgrDate::nsecPerMSec);
                  int4 year;
                  uint4 month, day;
                  first_tick_date.toDate(year, month, day);
                  first_tick_date.setDate(year, 1, 1);
                  first_tick = static_cast<double>(first_tick_date.get_nanoSec() / LgrDate::nsecPerMSec);
               }

               else if (time_domain && label_interval->get_val() >= half_year_interval)
               {
                  LgrDate first_tick_date(static_cast<int8>(first_tick) * LgrDate::nsecPerMSec);
                  int4 year;
                  uint4 month, day;
                  first_tick_date.toDate(year, month, day);
                  month = ((month-1) / 6) * 6 + 1;
                  first_tick_date.setDate(year, month, 1);
                  first_tick = static_cast<double>(first_tick_date.get_nanoSec() / LgrDate::nsecPerMSec);
               }
               else if (time_domain && label_interval->get_val() >= quarter_interval)
               {
                  LgrDate first_tick_date(static_cast<int8>(first_tick) * LgrDate::nsecPerMSec);
                  int4 year;
                  uint4 month, day;
                  first_tick_date.toDate(year, month, day);
                  month = ((month-1) / 3) * 3 + 1;
                  first_tick_date.setDate(year, month, 1);
                  first_tick = static_cast<double>(first_tick_date.get_nanoSec() / LgrDate::nsecPerMSec);
               }
               else if (time_domain && label_interval->get_val() >= 2* month_interval)
               {
                  LgrDate first_tick_date(static_cast<int8>(first_tick) * LgrDate::nsecPerMSec);
                  int4 year;
                  uint4 month, day;
                  first_tick_date.toDate(year, month, day);
                  month = ((month-1) / 2) * 2 + 1;
                  first_tick_date.setDate(year, month, 1);
                  first_tick = static_cast<double>(first_tick_date.get_nanoSec() / LgrDate::nsecPerMSec);
               }

               else if (time_domain && label_interval->get_val() >= month_interval)
               {
                  LgrDate first_tick_date(static_cast<int8>(first_tick) * LgrDate::nsecPerMSec);
                  int4 year;
                  uint4 month, day;
                  first_tick_date.toDate(year, month, day);
                  first_tick_date.setDate(year, month, 1);
                  first_tick = static_cast<double>(first_tick_date.get_nanoSec() / LgrDate::nsecPerMSec);
               }

               if (label_interval->get_val() > (max_value_->get_val() - min_value_->get_val()))
               {
                  if (first_tick < min_value_->get_val() || first_tick > max_value_->get_val())
                  {
                     first_tick = min_value->get_val() + (max_value->get_val() - min_value->get_val()) / 2;
                  }
               }
            }
         }
         else
         {
            first_tick = min_value->get_val() + (max_value->get_val() - min_value->get_val()) / 2;
         }

         // we can now generate the labels along with their positions, and values.
         uint4 tick_count(0);
         double tick(first_tick);
         labels.clear();
         scratch.imbue(StringLoader::make_locale());
         double label_interval_ = label_interval->get_val();
         double range_ = max_value->get_val() - min_value->get_val();
         if (range_ / label_interval_ > (size + 10)) {
            label_interval_ = range_ / (size + 10);
         }

         while(tick <= max_value->get_val() && scale > 0 && label_interval_ > 0)
         {
            double tick_value(logarithmic ? pow(log_base, tick) : tick);
            if(tick_value >= min_value->get_val())
            {
               ScaleLabel::rect_handle label_rect;
               value_handle value(new Expression::Operand(tick_value, 0));
               value_handle value_label(new Expression::Operand(tick, 0));
               label_handle label;
               scratch.str("");
               format_label(value_label);
               if(scratch.length() > 0)
               {
                  label_rect.bind(new NestedRect(driver->measure_text(scratch.str(), font, rotation)));
                  label.bind(
                     new ScaleLabel(
                        StrUni(scratch.str()), value, value_to_pos(value), label_rect));
                  label_rect->move(0, 0);
                  if(vertical)
                     label_rect->centre_y(label->position);
                  else
                     label_rect->centre_x(label->position);
                  labels.push_back(label);
               }
               else
                  break;
            }
            tick = next_tick(tick, label_interval_);
            tick_count++;
         }
         return labels;
      } // generate_labels


      void Scale::generate_minor_ticks(
         minor_ticks_type &minor_ticks,
         double min_width,
         int4 count)
      {
         // we will only generate minor ticks if there is more than one label.
         minor_ticks.clear();
         if(count <= 0)
            count = 0x7ffffffe;

         if(!labels.empty() && !one_tick_only)
         {
            // we need to calculate the interval between minor ticks
            double tick_interval_(label_interval->get_val() / (count + 1));
            double tick_width(tick_interval_ * scale);
            double space(label_interval->get_val() * scale);

            if(tick_width < min_width || !is_finite(tick_interval_))
            {
               double intervals_count(space / min_width);
               if(intervals_count > 9)
                  min_width = space / 9;

               if (time_domain)
               {
                  tick_interval_ = time_optimum_step_size(label_interval->get_val(), space, min_width)->get_val();
                  count = 64; // just to make sure we do not get in a very long loop
               }
               else
               {
                  count = 64;
                  tick_interval_ = label_interval->get_val() / 64.0;
                  while (count * min_width >= space)
                  {
                     tick_interval_ *= 2.0;
                     count /= 2;
                  }
               }
            }

            // we can now generate the minor ticks for the interval between each label.
            double scale_min(min_value->get_val());
            double scale_max(max_value->get_val());
            if(logarithmic)
            {
               scale_min = pow(log_base, scale_min);
               scale_max = pow(log_base, scale_max);
            }

            // We need to deal with integers to avoid overwriting major tick mark lines.  Multiply up until
            // we get an integer number, but put a limit on it, to deal with irrational numbers.
            if (floor(tick_interval_) == tick_interval_)
               generate_minor_ticks_double(minor_ticks, min_width, tick_interval_, scale_min, scale_max, count);
            else
               generate_minor_ticks_uint8(minor_ticks, min_width, tick_interval_, scale_min, scale_max, count);
         }
      } // generate_minor_ticks



      void Scale::generate_minor_ticks_double(
         minor_ticks_type &minor_ticks,
         double min_width,
         double tick_interval,
         double scale_min,
         double scale_max,
         int4 count)
      {
         for (size_t i = 0; i <= labels.size(); ++i)
         {
            label_handle label;
            double label_min;
            double label_max;
            double tick(0);
            uint4 ticks_count(0);
            double first_tick(0);

            if (i < labels.size())
            {
               label = labels[i];
               label_max = label->value->get_val();
               if (!logarithmic)
                  label_min = label->value->get_val() - label_interval->get_val();
               else
                  label_min = pow(log_base, (log(label_max) / log(log_base)) - 1);
            }
            else
            {
               label = labels.back();
               label_min = label->value->get_val();
               if (!logarithmic)
                  label_max = label_min + label_interval->get_val();
               else
                  label_max = pow(log_base, (log(label_min) / log(log_base)) + 1);
            }

            if (logarithmic)
            {
               int4 count = static_cast<int4>(log_base);
               tick_interval = fabs(label_max - label_min) / count;
               if (tick_interval < 1)
                  return;
            }
            first_tick = label_min;
            //if (auto_interval)
            //{
            //   if (label_min <= 0)
            //      first_tick = label_min + fabs(label_min % tick_interval);
            //   else
            //      first_tick = label_min - fabs(label_min % tick_interval);
            //}
            //else
            //{
            //   first_tick = label_min;
            //}
            tick = first_tick;
            while (tick < label_max && tick < scale_max && ticks_count <= (uint4)count)
            {
               if (tick > scale_min && tick > label_min)
               {
                  minor_ticks.push_back((double)tick);
               }
               tick = next_tick(tick, tick_interval);
               ticks_count++;
            }
         }
      } // generate_minor_ticks_double

      void Scale::generate_minor_ticks_uint8(
         minor_ticks_type &minor_ticks,
         double min_width,
         double tick_interval_,
         double scale_min,
         double scale_max,
         int4 count)
      {
         // We need to deal with integers to avoid overwriting major tick mark lines.  Multiply up until
         // we get an integer number, but put a limit on it, to deal with irrational numbers.
         int integer_multipler = 1;
         while ((floor(tick_interval_) != tick_interval_) && integer_multipler < 10000)
         {
            tick_interval_ *= 10;
            integer_multipler *= 10;
         }
         if (fabs(tick_interval_) < 1)
            return;

         uint8 tick_interval = tick_interval_ > 0 ? (uint8)(tick_interval_ + 0.5) : (uint8)(tick_interval_ - 0.5);



         for (size_t i = 0; i <= labels.size(); ++i)
         {
            label_handle label;
            double label_min_;
            double label_max_;
            int8 tick(0);
            uint4 ticks_count(0);
            int8 first_tick(0);

            if (i < labels.size())
            {
               label = labels[i];
               label_max_ = label->value->get_val();
               if (!logarithmic)
                  label_min_ = label->value->get_val() - label_interval->get_val();
               else
                  label_min_ = pow(log_base, (log(label_max_) / log(log_base)) - 1);
            }
            else
            {
               label = labels.back();
               label_min_ = label->value->get_val();
               if (!logarithmic)
                  label_max_ = label_min_ + label_interval->get_val();
               else
                  label_max_ = pow(log_base, (log(label_min_) / log(log_base)) + 1);
            }

            int8 label_min = label_min_ > 0 ? (int8)(label_min_ * integer_multipler + 0.5) : (int8)(label_min_ * integer_multipler - 0.5);
            int8 label_max = label_max_ > 0 ? (int8)(label_max_ * integer_multipler + 0.5) : (int8)(label_max_ * integer_multipler - 0.5);

            if (logarithmic)
            {
               if (count > log_base)
                  count = static_cast<int4>(log_base);
               tick_interval_ = fabs(label_max_ - label_min_) / count;
               tick_interval = tick_interval_ > 0 ? (int8)(tick_interval_ * integer_multipler + 0.5) : (int8)(tick_interval_ * integer_multipler - 0.5);
               if (tick_interval < 1)
                  return;
            }
            first_tick = label_min;
            //if (auto_interval)
            //{
            //   if (label_min <= 0)
            //      first_tick = label_min + fabs(label_min % tick_interval);
            //   else
            //      first_tick = label_min - fabs(label_min % tick_interval);
            //}
            //else
            //{
            //   first_tick = label_min;
            //}
            tick = first_tick;
            while (tick < label_max && tick < scale_max * integer_multipler && ticks_count <= (uint4)count)
            {
               if (tick > scale_min* integer_multipler&& tick > label_min)
               {
                  minor_ticks.push_back((double)tick / integer_multipler);
               }
               tick = (int8)next_tick((double)tick, (double)tick_interval);
               ticks_count++;            
            }
         }
      } // generate_minor_ticks



      double Scale::value_to_pos(value_handle &value_)
      {
         double rtn(0);
         double value(value_->get_val());
         if(logarithmic)
            value = log(value) / log(log_base);
         if(vertical)
         {
            if(inverted)
               rtn = scale * (value - min_value->get_val());
            else
               rtn = scale * (max_value->get_val() - value);
         }
         else
         {
            if(inverted)
               rtn = scale * (max_value->get_val() - value);
            else
               rtn = scale * (value - min_value->get_val());
         }
         return rtn;
      } // value_to_pos


      Scale::value_handle Scale::pos_to_value(double pos)
      {
         double rtn(0);
         if(vertical)
         {
            if(inverted)
               rtn = pos / scale + min_value->get_val();
            else
               rtn = max_value->get_val() - pos / scale;
         }
         else
         {
            if(inverted)
               rtn = max_value->get_val() - pos / scale;
            else
               rtn = pos / scale + min_value->get_val();
         }
         if(logarithmic)
            rtn = pow(log_base, rtn);
         return new Expression::Operand(rtn, 0);
      } // pos_to_value

      double Scale::offset_to_size(double value)
      {
         double rtn(0);
         if(logarithmic)
            value = log(value) / log(log_base);
         rtn = scale * value;
         return rtn;
      } // offset_to_size


      double Scale::offset_weight(value_handle &value, double offset)
      {
         double rtn(0);
         double pos(value_to_pos(value));
         if(vertical)
         {
            if(inverted)
               rtn = fabs(pos_to_value(pos + offset)->get_val() - value->get_val());
            else
               rtn = fabs(pos_to_value(pos - offset)->get_val() - value->get_val());
         }
         else
         {
            if(inverted)
               rtn = fabs(pos_to_value(pos - offset)->get_val() - value->get_val());
            else
               rtn = fabs(pos_to_value(pos + offset)->get_val() - value->get_val());
         }
         return rtn;
      } // offset_weight


      double Scale::next_tick(double tick_prev, double interval)
      {
         double rtn(tick_prev + interval);
         if(time_domain && interval >= month_interval)
         {
            LgrDate rtn_date(static_cast<int8>(rtn) * LgrDate::nsecPerMSec);
            int4 year;
            uint4 month, day;
            rtn_date.toDate(year, month, day);
            if (day > 10)
            {
               month++;
               while (month > 12)
               {
                  month = month - 12;
                  year++;
               }
            }
            day = 1;
            rtn_date.setDate(year, month, day);
            rtn = static_cast<double>(rtn_date.get_nanoSec() / LgrDate::nsecPerMSec);
         }
         else if (!time_domain)
         {
            if (fabs(rtn) < 1e-16)
               rtn = 0;
         }
         return rtn;
      } // next_tick
      

      Scale::value_handle Scale::scalar_optimum_step_size(
         double range, double space, double required)
      {
         // we need to make our first estimate of the step size and format it as a string. 
         double max_steps(space / required);
         double step_size(range / max_steps);
         StrAsc step_str;
         scratch.str("");
         scratch << boost::format("%.7f") % step_size;
         step_str = scratch.str();

         // we can now use a regular expression to look for the least significant digit in the label
         // string. 
         size_t last_pos(last_decimal_pos(step_str));
         while(last_pos < step_str.length())
         {
            StrAsc temp(step_str.c_str(), last_pos);
            size_t previous_pos(last_decimal_pos(temp));
            if(previous_pos < temp.length())
            {
               clear_with_carry(step_str, last_pos);
               last_pos = last_decimal_pos(step_str);
            }
            else
            {
               switch(step_str[last_pos])
               {
               case '1':
               case '2':
               case '5': // these are the digits we want
                  break;

               case '3':
               case '4': // round up
                  step_str[last_pos] = '5';
                  break;

               case '6':
               case '7':
               case '8':
               case '9':
                  clear_with_carry(step_str, last_pos);
                  break;

               case '0':
                  step_str[last_pos] = '1';
                  break;
               }

               // we need to convert the step string back to floating point
               Csi::IBuffStream input(step_str.c_str(), step_str.length());
               input.imbue(StringLoader::make_locale(0));
               last_pos = step_str.length();
               input >> step_size;
            }
         }
         if(step_size <= 1e-38)
            step_size = 1;
         return new Expression::Operand(step_size, 0);
      } // scalar_optimum_step_size


      Scale::value_handle Scale::time_optimum_step_size(
         double range, double space, double label_req)
      {
         // These are in milliseconds
         static int8 const time_intervals[] =
         {
                1,
                2,
                5,
               10,
               20,
               50,
              100,
              200,
              500,
             1000,
             2000,
             5000,
            10000,
            15000,
            30000,  // 30 seconds
                1 * LgrDate::msecPerMin,
                2 * LgrDate::msecPerMin,
                5 * LgrDate::msecPerMin,
               10 * LgrDate::msecPerMin,
               15 * LgrDate::msecPerMin,
               30 * LgrDate::msecPerMin,
                1 * LgrDate::msecPerHour,
                2 * LgrDate::msecPerHour,
                6 * LgrDate::msecPerHour,
               12 * LgrDate::msecPerHour,
                1 * LgrDate::msecPerDay,
                2 * LgrDate::msecPerDay,
                3 * LgrDate::msecPerDay,
                1 * LgrDate::msecPerWeek,
                2 * LgrDate::msecPerWeek,
                1 * month_interval,
                2 * month_interval,
                1 * quarter_interval,   // 3 * month_interval
                1 * half_year_interval, // 6 months
                1 * year_interval,
                2 * year_interval,
                3 * year_interval,
                5 * year_interval,
               10 * year_interval,
               20 * year_interval,
               50 * year_interval,
          0
         };
         int8 step_size;
         if(is_finite(range))
         {
            double max_steps(space / label_req);
            int i = 0;

            step_size = static_cast<int8>(range / max_steps);
            while(time_intervals[i] != 0 && step_size > time_intervals[i])
               ++i;
            if(time_intervals[i] != 0)
               step_size = time_intervals[i];
            else
               return scalar_optimum_step_size(range, space, label_req);
         }
         else
            step_size = 100;

         // we need to allocate a value for the return
         value_handle rtn(new Expression::Operand);
         rtn->set_val(step_size, 0);
         return rtn;
      } // time_optimum_step_size
   };
};

