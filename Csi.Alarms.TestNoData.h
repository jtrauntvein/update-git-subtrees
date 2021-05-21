/* Csi.Alarms.TestNoData.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 24 September 2012
   Last Change: Thursday 17 November 2016
   Last Commit: $Date: 2016-11-17 14:43:10 -0600 (Thu, 17 Nov 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_TestNoData_h
#define Csi_Alarms_TestNoData_h

#include "Csi.Alarms.TestBase.h"
#include "Csi.Alarms.Alarm.h"
#include "OneShot.h"


namespace Csi
{
   namespace Alarms
   {
      /**
       * Defines a test for a condition that determines whether a specified amount of time has
       * elapsed since receiving new record data from the source expression.
       */
      class TestNoData: public TestBase, public OneShotClient
      {
      private:
         /**
          * Specifies the one shot timer that will be used to firing the event after the interval
          * has elapsed.
          */
         typedef Csi::SharedPtr<OneShot> timer_handle;
         timer_handle timer;

         /**
          * Identifies the timer used to track the watch dog interval.
          */
         uint4 no_data_id;

         /**
          * Specifies the interval in milli-seconds
          */
         uint4 interval;

         /**
          * Specifies the alarm that owns this test.
          */
         Alarm *alarm;

         /**
          * Set to true if the alarm has been started.
          */
         bool started;
         
      public:
         /**
          * Constructor
          *
          * @param alarm_ Specifies the alarm that owns this test.
          *
          * @param timer_ Specifies the one shot timer that will be used to trigger an elapsed
          * interval.
          */
         TestNoData(Alarm *alarm_, timer_handle &timer_):
            alarm(alarm_),
            timer(timer_),
            no_data_id(0),
            interval(60000),
            started(false)
         { }

         /**
          * Destructor
          */
         virtual ~TestNoData()
         {
            if(timer != 0 && no_data_id != 0)
            {
               timer->disarm(no_data_id);
               no_data_id = 0;
            }
            timer.clear();
         }

         /**
          * Overloads the base class version to start the interval timer.
          */
         virtual void on_started()
         {
            started = true;
            no_data_id = timer->arm(this, interval);
         }

         /**
          * Overloads the base class version to stop the interval timer.
          */
         virtual void on_stopped()
         {
            started = false;
            if(no_data_id != 0)
               timer->disarm(no_data_id);
         }

         
         /**
          * Handles the event where a new record has been received.
          */
         virtual bool on_record(record_handle const &record)
         {
            bool rtn(true);
            if(record != 0)
            {
               if(no_data_id != 0)
                  timer->reset(no_data_id);
               else
                  no_data_id = timer->arm(this, interval);
               rtn = false;
            }
            return rtn;
         }

         /**
          * Overloaded to ignore reports of new values.
          */
         virtual bool on_value(operand_handle &value)
         { return false; }

         /**
          * Overloaded to return true if the test has been started and the interval timer has fired.
          */
         virtual bool is_triggered()
         { return no_data_id == 0 && started; }
         virtual bool has_on_condition() const
         { return no_data_id == 0 && started; }

         /**
          * Specifies the names of elements and attributes specific to this test.
          */
         static StrUni const interval_name;
         
         /**
          * Overloaded to read the configuration for this test from the specified XML element.
          */
         virtual void read(Xml::Element &e)
         {
            interval = e.get_attr_uint4(interval_name);
            if(interval == 0)
               throw std::invalid_argument("invalid no data interval specified");
            if(no_data_id != 0)
               timer->disarm(no_data_id);
            no_data_id = timer->arm(this, interval);
         }

         /**
          * Overloaded to write the configuration of this test to the specified stream.
          */
         virtual void write(Xml::Element &e)
         {
            e.set_attr_wstr(L"no-data", L"type");
            e.set_attr_uint4(interval, interval_name);
         }

         /**
          * Overloaded to format the entrance condition for this test.
          */
         virtual void format_entrance(std::ostream &out)
         { out << "no data received"; }

         /**
          * Overloaded to format the exit condition for this test.
          */
         virtual void format_exit(std::ostream &out)
         { out << "data received"; }

         /**
          * Overloads the base class version to handle timer events.
          */
         virtual void onOneShotFired(uint4 id)
         {
            if(id == no_data_id)
            {
               record_handle nothing;
               no_data_id = 0;
               alarm->process_record(nothing);
            }
         }
      };
   };
};


#endif
