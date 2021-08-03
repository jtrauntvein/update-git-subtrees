/* Csi.Alarms.Condition.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 24 September 2012
   Last Change: Wednesday 16 November 2016
   Last Commit: $Date: 2016-11-17 14:43:10 -0600 (Thu, 17 Nov 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_Condition_h
#define Csi_Alarms_Condition_h

#include "Csi.Alarms.TestBase.h"
#include "Csi.Alarms.ActionTemplateBase.h"
#include "Csi.Xml.Element.h"


namespace Csi
{
   namespace Alarms
   {
      class Alarm;
      class Manager;

      /**
       * Defines an object that acts as a condition for an alarm.  This includes any actions
       * associated with the event when this condition is activated.
       */
      class Condition
      {
      public:
         /**
          * Constructor
          *
          * @param alarm_ Specifies the alarm that owns this condition.
          */
         Condition(Alarm *alarm_):
            alarm(alarm_)
         { }

         /**
          * Destructor
          */
         ~Condition()
         { }

         /**
          * Specifies the names of elements and attributes in the Condition XML structure.
          */
         static StrUni const condition_name_name;
         static StrUni const test_name;
         static StrUni const test_type_name;
         static StrUni const test_type_no_data;
         static StrUni const test_type_data;
         static StrUni const actions_name;
         static StrUni const action_name;
         static StrUni const action_type_name;
         
         /**
          * Reads the configuration of this condition from the specified XML element.
          */
         void read(Xml::Element &elem);

         /**
          * Writes the configuration of this condition to the specified XML element.
          */
         void write(Xml::Element &elem);

         /**
          * Called when a new value has been evaluated by the alarm.
          *
          * @return Returns true if the condition is triggered because of the new value.
          *
          * @param value Specifies the value to evaluate.
          */
         typedef TestBase::operand_handle operand_handle;
         bool on_value(operand_handle &value);

         /**
          * Called when a new record has been received by the alarm.
          *
          * @param record Specifies the record that has been received.
          *
          * @return Returns true if this condition will trigger because of this record.
          */
         typedef TestBase::record_handle record_handle;
         bool on_record(record_handle const &record);

         /**
          * @return Returns true if the test for this condition has been triggered.
          */
         bool is_triggered()
         { return test->is_triggered(); }

         /**
          * @return Returns the alarm that owns this condition.
          */
         Alarm *get_alarm()
         { return alarm; }
         Alarm const *get_alarm() const
         { return alarm; }

         /**
          * Called when the alarm has entered a triggered state.
          */
         void on_alarm_on();

         /**
          * Called when the alarm has left a triggered state.
          */
         void on_alarm_off();

         Manager *get_manager();

         /**
          * Formats the description of this condition to the specified output stream.  This method
          * will replace the following sequences within the message.
          *
          * %n: replaced with the name of the alarm.
          * %s: replaced with expanded alarm source expression.
          * %v: replaced by the last value returned from the alarm source expression.
          * %e: replaced by the entrance condition for test.
          * %x: replaced by the exit condition for the test.
          */
         void format_desc(std::ostream &out, StrAsc const &fmt);

         /**
          * @return Returns the test for this condition.
          */
         SharedPtr<TestBase> get_test()
         { return test; }
         SharedPtr<TestBase> const &get_test() const
         { return test; }

         /**
          * Returns the name for this condition.
          */
         StrUni const &get_name() const
         { return name; }

         /**
          * @param name_ Specifies the name for this condition.
          */
         void set_name(StrUni const &name_)
         { name = name_; }

         /**
          * Notifies the test that the alarm has been started.
          */
         void on_started()
         { test->on_started(); }

         /**
          * Notifies the test that the alarm has been stopped.
          */
         void on_stopped()
         { test->on_stopped(); }
         
      private:
         /**
          * Specifies the alarm that ownes this condition.
          */
         Alarm *alarm;
         
         /**
          * Specifies the test for this condition.
          */
         Csi::SharedPtr<TestBase> test;

         /**
          * Specifies the actions to take when this condition is triggered.
          */
         typedef SharedPtr<ActionTemplateBase> action_handle;
         typedef std::list<action_handle> actions_type;
         actions_type actions;

         /**
          * Specifies the name of this condition.
          */
         StrUni name;
      };
   };
};


#endif
