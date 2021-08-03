/* Csi.CronPredictor.cpp

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 10 November 2012
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.CronPredictor.h"


namespace Csi
{
   namespace CronPredictorHelpers
   {
      ////////////////////////////////////////////////////////////
      // class Rotor definitions
      ////////////////////////////////////////////////////////////
      Rotor::Rotor(
         allowed_type const &allowed, uint4 min, uint4 max):
         allowed_values(allowed.begin(), allowed.end()),
         selected_pos(0)
      {
         if(allowed.empty())
         {
            for(uint4 i = min; i <= max; ++i)
               allowed_values.push_back(i);
         }
         values_type::iterator vi = allowed_values.begin();
      } // constructor


      bool Rotor::set(uint4 value)
      {
         values_type::iterator vi(allowed_values.begin());
         bool rtn(false);
         while(vi != allowed_values.end() && *vi < value)
            ++vi;
         if(vi == allowed_values.end())
         {
            vi = allowed_values.begin();
            rtn = true;
         }
         selected_pos = (uint4)std::distance(allowed_values.begin(), vi);
         return rtn;
      } // set


      bool Rotor::increment()
      {
         bool rtn(false);
         if(++selected_pos >= allowed_values.size())
         {
            rtn = true;
            reset();
         }
         return rtn;
      } // increment


      ////////////////////////////////////////////////////////////
      // class DayRotor definitions
      ////////////////////////////////////////////////////////////
      DayRotor::DayRotor(
         allowed_type const &month_allowed,
         allowed_type const &week_allowed,
         SharedPtr<Rotor> year_rotor_,
         SharedPtr<Rotor> month_rotor_):
         year_rotor(year_rotor_),
         month_rotor(month_rotor_),
         current_year(0),
         current_month(0),
         month_days(month_allowed.begin(), month_allowed.end()),
         week_days(week_allowed.begin(), week_allowed.end())
      {
         if(month_allowed.empty())
         {
            for(uint4 i = 1; i <= 31; ++i)
               month_days.push_back(i);
         }
         if(week_allowed.empty())
         {
            for(uint4 i = 1; i <= 7; ++i)
               week_days.push_back(i);
         }
      } // constructor


      void DayRotor::fill_allowed()
      {
         if(current_year != year_rotor->get_value() ||
            current_month != month_rotor->get_value())
         {
            LgrDate temp;
            current_year = year_rotor->get_value();
            current_month = month_rotor->get_value();
            allowed_values.clear();
            temp.setDate(current_year, current_month, 1);
            while(temp.month() == current_month)
            {
               values_type::iterator mi(
                  std::find(month_days.begin(), month_days.end(), temp.day()));
               values_type::iterator wi(
                  std::find(week_days.begin(), week_days.end(), temp.dayOfWeek()));
               if(mi != month_days.end() && wi != week_days.end())
                  allowed_values.push_back(temp.day());
               temp += LgrDate::nsecPerDay;
               if(mi == month_days.end() &&
                  wi != week_days.end() &&
                  month_days.back() == 32 &&
                  temp.month() != current_month)
               {
                  LgrDate last_day(temp - LgrDate::nsecPerDay);
                  allowed_values.push_back(last_day.day());
               }
            }
         }
      } // fill_allowed
   };


   ////////////////////////////////////////////////////////////
   // class CronPredictor definitions
   ////////////////////////////////////////////////////////////
   CronPredictor::CronPredictor(
      allowed_type const &months,
      allowed_type const &days,
      allowed_type const &week_days,
      allowed_type const &hours,
      allowed_type const &minutes)
   {
      year_rotor.bind(new CronPredictorHelpers::YearRotor);
      month_rotor.bind(new rotor_type(months, 1, 12));
      day_rotor.bind(
         new CronPredictorHelpers::DayRotor(
            days, week_days, year_rotor, month_rotor));
      hour_rotor.bind(new rotor_type(hours, 0, 23));
      minute_rotor.bind(new rotor_type(minutes, 0, 59));
   } // constructor


   CronPredictor::~CronPredictor()
   {
      year_rotor.clear();
      month_rotor.clear();
      day_rotor.clear();
      hour_rotor.clear();
      minute_rotor.clear();
   } // destructor


   LgrDate CronPredictor::predict(LgrDate const &current)
   {
      // before we apply the current values, we will reset all of the rotors
      month_rotor->reset();
      day_rotor->reset();
      hour_rotor->reset();
      minute_rotor->reset();
      
      // we will apply the current information to each of the rotors in turn until we come
      // across a rotor that was reset.
      LgrDate rtn(current);
      year_rotor->set(current.year());
      if(month_rotor->set(current.month()))
      {
         day_rotor->reset();
         hour_rotor->reset();
         minute_rotor->reset();
         year_rotor->increment();
      }
      rtn.setDate(year_rotor->get_value(), month_rotor->get_value(), day_rotor->get_value());
      rtn.setTime(hour_rotor->get_value(), minute_rotor->get_value());
      if(rtn < current)
      {
         if(day_rotor->set(current.day()))
         {
            if(month_rotor->increment())
               year_rotor->increment();
            hour_rotor->reset();
            minute_rotor->reset();
         }
         rtn.setDate(year_rotor->get_value(), month_rotor->get_value(), day_rotor->get_value());
         rtn.setTime(hour_rotor->get_value(), minute_rotor->get_value());
         if(rtn < current)
         {
            if(hour_rotor->set(current.hour()))
            {
               if(day_rotor->increment())
               {
                  if(month_rotor->increment())
                     year_rotor->increment();
               }
               minute_rotor->reset();
            }
            rtn.setDate(
               year_rotor->get_value(), month_rotor->get_value(), day_rotor->get_value());
            rtn.setTime(hour_rotor->get_value(), minute_rotor->get_value());
            if(rtn < current)
            {
               if(minute_rotor->set(current.minute()))
               {
                  if(hour_rotor->increment())
                  {
                     if(day_rotor->increment())
                     {
                        if(month_rotor->increment())
                           year_rotor->increment();
                     }
                  }
               }
               rtn.setDate(
                  year_rotor->get_value(),
                  month_rotor->get_value(),
                  day_rotor->get_value());
               rtn.setTime(hour_rotor->get_value(), minute_rotor->get_value());
               if(rtn < current)
               {
                  if(minute_rotor->increment())
                  {
                     if(hour_rotor->increment())
                     {
                        if(day_rotor->increment())
                        {
                           if(month_rotor->increment())
                              year_rotor->increment();
                        }
                     }
                  }
                  rtn.setDate(
                     year_rotor->get_value(),
                     month_rotor->get_value(),
                     day_rotor->get_value());
                  rtn.setTime(hour_rotor->get_value(), minute_rotor->get_value());
               }
            }
         }
      }
      return rtn;
   } // predict
};

