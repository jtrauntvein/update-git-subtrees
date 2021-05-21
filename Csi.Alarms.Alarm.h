/* Csi.Alarms.Alarm.h

   Copyright (C) 2012, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 24 September 2012
   Last Change: Tuesday 27 November 2018
   Last Commit: $Date: 2018-11-29 13:29:49 -0600 (Thu, 29 Nov 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_Alarm_h
#define Csi_Alarms_Alarm_h

#include "Csi.Alarms.Condition.h"
#include "Csi.Alarms.ActionBase.h"
#include "Cora.DataSources.SinkBase.h"
#include "Csi.Expression.TokenFactory.h"
#include "OneShot.h"
#include "Csi.Json.h"


namespace Csi
{
   namespace Alarms
   {
      /**
       * Defines a client interface that receives notifications whenever an alarm has the potential
       * for changing state.
       */
      class Alarm;
      class AlarmClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the alarm when it has undergone a state change.
          *
          * @param sender Specifies the alarm that has changed.
          */
         virtual void on_alarm_change(Alarm *sender) = 0;

         /**
          * Called to replace the sequence starting at the specified position of the specified
          * format template.
          *
          * @param out Specifies the stream to which data will be written.
          *
          * @param format Specifies the format string to be expanded.
          *
          * @param pos Specifies the current position in the format string.
          *
          * @return Returns the number of bytes consumed from the format string.
          */
         virtual size_t expand_format(std::ostream &out, StrAsc const &format, size_t pos)
         { return 0; }

         /**
          * Called when the value of the source expression has changed but before any condition
          * expressions are evaluated.  This can give the application to perform units conversions
          * or other adjustments before the tests are made.
          *
          * @param sender Specifies the alarm sending this notification.
          *
          * @param last_value Specifies the last value that resulted from the last source expression
          * evaluation. 
          */
         typedef Condition::operand_handle last_value_handle;
         virtual void on_last_value_changed(Alarm *sender, last_value_handle &last_value)
         { }

         /**
          * Called by the alarm to format the last value for the alarm.
          *
          * @return Returns true if this method actually formatted the value.  If false, the alarm
          * will format the value itself.
          *
          * @param out Specifies the stream to which the value will be formatted.
          *
          * @param last_value Specifies the value to be formatted
          */
         virtual bool format_last_value(std::ostream &out, last_value_handle &last_value)
         { return false; }

         /**
          * Called by the alarm to expand the units of the alarm value.
          *
          * @param out Specifies the stream to which the units will be written.
          */
         virtual void format_last_value_units(std::ostream &out)
         { }
      };

      
      /**
       * Defines an object that looks for one or more data related conditions to be asserted and
       * manages the actions and notifications for those conditions.
       */
      class Manager;
      class Alarm:
         public Cora::DataSources::SinkBase,
         public EventReceiver
      {
      public:
         /**
          * Constructor
          *
          * @param manager_ Specifies the object that manages all alarms in the application.
          */
         Alarm(Manager *manager_);

         /**
          * Destructor
          */
         virtual ~Alarm();

         /**
          * @return Returns the one shot timer used by this alarm.
          */
         Csi::SharedPtr<OneShot> &get_timer();

         /**
          * Specifies the names of elements and attributes inside of the alarm configuration
          * structure.
          */
         static StrUni const alarm_name_name;
         static StrUni const id_name;
         static StrUni const conditions_name;
         static StrUni const condition_name;
         static StrUni const source_expression_name;
         static StrUni const latched_name;
         
         /**
          * Reads the configuration for this alarm from the specified XML structure.
          *
          * @param elem Specifies the XML structure that describes this alarm.
          */
         void read(Xml::Element &elem);

         /**
          * Writes the configuration for this alarm to the specified XML structure.
          *
          * @param elem Specifies the XML element that will be configured.
          */
         void write(Xml::Element &elem);

         /**
          * Overloads the base class version to handle the notification that a data request has been
          * started for this alarm.
          */
         typedef Cora::DataSources::Manager sources_type;
         virtual void on_sink_ready(
            sources_type *sources,
            request_handle &request,
            record_handle &record);
         
         /**
          * Overloads the base class version to handle the notification that a data request
          * associated with this alarm has failed.
          */
         virtual void on_sink_failure(
            sources_type *sources,
            request_handle &request,
            sink_failure_type failure);

         /**
          * Overloads the base class version to handle incoming records for data requests
          * associated with this alarm.
          */
         virtual void on_sink_records(
            sources_type *sources,
            requests_type &requests,
            records_type const &records);

         /**
          * Sets up this alarm to ignore the next record sent for the specified data source URI.
          */
         virtual void ignore_next_record(StrUni const &uri);

         /**
          * Process the content of a recently received record.
          */
         virtual void process_record(record_handle const &record);

         /**
          * @return Returns the last value of the alarm source expression.
          */
         typedef Condition::operand_handle operand_handle;
         operand_handle &get_last_value()
         { return last_value; }

         /**
          * @return Returns the state of this alarm with respect to triggers and acknowledgement.
          */
         enum state_type
         {
            state_off,
            state_on,
            state_acked
         };
         state_type get_state() const;

         /**
          * @return Returns true if there are one or more conditions associated with this alarm that
          * have a true test value.
          */
         bool has_on_condition() const;

         /**
          * @return Returns a string that describes the last error event associated with this alarm.
          */
         StrAsc const &get_last_error() const
         { return last_error; }

         /**
          * @return Returns the assigned title for this alarm.
          */
         StrUni const &get_name() const
         { return name; }

         /**
          * @return Returns the unique identifier for this alarm.
          */
         StrUni const get_id() const
         { return id; }

         /**
          * Changes the state of a triggered alarm to acknowledged with the specified comments being
          * logged.
          */
         void acknowledge(StrUni const &comments);

         /**
          * @return Returns the expression token factory.
          */
         SharedPtr<Expression::TokenFactory> &get_token_factory();

         /**
          * Places this alarm in a state where it is sending out data rquests and looking for
          * incoming data.
          */
         void start();

         /**
          * Stops all requests for this alarm.
          */
         void stop();

         /**
          * Sets the application object that will receive alarm state change notifications.
          */
         void set_client(AlarmClient *client_)
         { client = client_; }

         /**
          * @return Returns the application object that will receive alarm state notifications.
          */
         AlarmClient *get_client()
         { return client; }

         /**
          * @return Returns the object that manages all alarms for the application.
          */
         Manager *get_manager()
         { return manager; }

         /**
          * @return Returns the source expression with its values filled in.
          */
         StrUni annotate_source_expression();

         /**
          * @return Returns true if this alarm has been configured to latch (report a triggered
          * state until acknowledged by the client event if the original trigger condition is no
          * longer valid.
          */
         bool get_latched()
         { return latch_alarms; }

         /**
          * @return Returns the number of actions that are still pending for this alarm.  Note that
          * actions may still be pending even if the alarm is no longer in a triggered state.
          */
         uint4 get_pending_actions();

         /**
          * @return Returns the last error encountered for an action or an empty string if the last
          * action succeeded.
          */
         StrAsc const &get_last_action_error() const
         { return last_action_error; }

         /**
          * Adds an action to be executed for this alarm.
          */
         typedef SharedPtr<ActionBase> action_handle;
         void add_action(action_handle action);
         
         /**
          * Called when the current action has been completed.
          */
         void on_action_complete(ActionBase *action);

         /**
          * @param enabled Set to true if alarm actions are enabled.
          */
         void enable_actions(bool enabled);

         /**
          * Overloads the base class version to handle asynchronous events.
          */
         virtual void receive(SharedPtr<Event> &ev);

         /**
          * Adds a log message associated with this alarm.
          */
         void add_log(Xml::Element &elem);

         /**
          * Formats the current value of the source expression to the specified output stream.
          */
         void format_value(std::ostream &out);

         /**
          * Formats the units of the source expression as given by the alarm client (if any).
          */
         void format_value_units(std::ostream &out);

         /**
          * Formats the time stamp for the last triggered value for this alarm.
          */
         void format_value_time(std::ostream &out);

         /**
          * Formats the type of the current value for the source expression to the specified stream.
          */
         void format_value_type(std::ostream &out);

         /**
          * @return Returns the name for the current triggered condition or an empty string if this
          * alarm is not triggered.
          */
         StrUni get_triggered_condition_name();

         /**
          * @return Returns the triggered condition or a null pointer if this alarm is not
          * triggered.
          */
         typedef Csi::SharedPtr<Condition> value_type;
         value_type &get_triggered_condition()
         { return triggered_condition; }

         /**
          * Called to format this alarm into a JSON structure
          */
         void format_json(Csi::Json::Object &message);
         
      private:
         /**
          * Handles the transistion of this alarm from a non-triggered to a triggered state.
          */
         void on_alarm_on();

         /**
          * Handles the transistion of this alarm to a non-triggered state.
          */
         void on_alarm_off();
         
      private:
         /**
          * Specifies the object that manages all alarms for the application.
          */
         Manager *manager;

         /**
          * Specifies the collection of triggering conditions for this alarm.
          */
         typedef std::list<value_type> conditions_type;
         conditions_type conditions;

         /**
          * Specifies the title for this alarm.
          */
         StrUni name;

         /**
          * Specifies the unique identifier for this alarm .
          */
         StrUni id;

         /**
          * Specifies the source expression for this alarm.
          */
         StrUni source_expression_str;
         Csi::SharedPtr<Expression::ExpressionHandler> source_expression;

         /**
          * Set to true if this alarm is latchable.
          */
         bool latch_alarms;

         /**
          * Specifies the last value evaluated for the source expression.
          */
         Condition::operand_handle last_value;

         /**
          * Specifies the error encountered for alarm data source requests.
          */
         StrAsc last_error;

         /**
          * Specifies the collection of data source requests for the source expression.
          */
         typedef Expression::TokenFactory::requests_type requests_type;
         requests_type requests;

         /**
          * Set to true if the data source expression references a table.
          */
         bool for_table;

         /**
          * Specifies the condition (if any) that is currently triggered.
          */
         value_type triggered_condition;

         /**
          * Set to true if this alarm has been acknowledged.
          */
         bool acknowledged;

         /**
          * Specifies the application object that will receive alarm state notifications.
          */
         AlarmClient *client;

         /**
          * Specifies a string that describes the the error for the last action executed.  If the
          * last action succeeded, the string will be empty.
          */
         StrAsc last_action_error;

         /**
          * Specifies the data source requests for the next records that should be ignored.
          */
         requests_type ignore_requests;

         /**
          * Set to true if actions for this alarm are allowed.
          */
         bool actions_enabled;
      };
   };
};


#endif
