/* Csi.Alarms.Manager.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 24 September 2012
   Last Change: Tuesday 15 November 2016
   Last Commit: $Date: 2016-11-17 14:43:10 -0600 (Thu, 17 Nov 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_Manager_h
#define Csi_Alarms_Manager_h

#include "Csi.Alarms.Alarm.h"
#include "Csi.Alarms.EmailProfile.h"
#include "Csi.Alarms.ActionBase.h"
#include "Csi.Alarms.AlarmLogger.h"
#include "Cora.DataSources.Manager.h"
#include "Csi.Expression.TokenFactory.h"
#include <algorithm>


namespace Csi
{
   namespace Alarms
   {
      /**
       * Defins a functor object that will compare an alarm's name against a provided name.
       */
      struct alarm_has_name
      {
         StrUni const name;
         alarm_has_name(StrUni const &name_):
            name(name_)
         { }
         bool operator ()(SharedPtr<Alarm> const &alarm) const
         { return alarm->get_name() == name || alarm->get_id() == name; }
      };


      /**
       * Defines an object that will manage a collection of alarms for an application.
       */
      class Manager
      {
      public:
         /**
          * Constructor
          *
          * @param sources_ Specifies the data source manager used by alarms to resolve access to
          * data.
          */
         typedef Cora::DataSources::Manager sources_type;
         typedef Csi::SharedPtr<sources_type> sources_handle;
         Manager(sources_handle &sources_);

         /**
          * Destructor
          */
         ~Manager();

         /**
          * @return Returns the one shot timer used for all alarms.
          */
         Csi::SharedPtr<OneShot> &get_timer()
         { return sources->get_timer(); }

         /**
          * Reads the configuration of this manager from the specified XML element.
          *
          * @param elem Specifies the XML element that describes this manager.
          *
          * @param read_errors Specifies a collection of strings that will be added if an error is
          * encountered while reading the XML.
          *
          * @param client Specifies the client that will be registered with all alarms.
          */
         typedef std::deque<StrAsc> read_errors_type;
         void read(
            Xml::Element &elem,
            read_errors_type &read_errors,
            AlarmClient *client = 0);

         /**
          * Writes the configuration of this manager to the specified XML data structure.
          */
         void write(Xml::Element &elem);

         /**
          * @return Returns the token factory used to expressions in alarms.
          */
         Csi::SharedPtr<Expression::TokenFactory> &get_token_factory()
         { return token_factory; }

         /**
          * @return Returns the data source manager.
          */
         sources_handle &get_sources()
         { return sources; }

         /**
          * Starts monitoring for all registered alarms.
          */
         void start();

         /**
          * Stops monitoring for all registered alarms.
          */
         void stop();

         /**
          * Adds an alarm to the set managed.
          *
          * @param alarm Specifies the alarm to add.
          */
         typedef Csi::SharedPtr<Alarm> value_type;
         void add_alarm(value_type alarm);

         /**
          * Creates a second copy of the specified alarm and adds that copy to the set managed.
          */
         value_type clone_alarm(Alarm *alarm);

         /**
          * Removes the specified alarm from the set managed by this manager.
          */
         void remove_alarm(Alarm *alarm);

         // @group: declaration to act as a container of alarms

         /**
          * @return Returns an iterator to the beginning of the list of alarms.
          */
         typedef std::list<value_type> alarms_type;
         typedef alarms_type::iterator iterator;
         typedef alarms_type::const_iterator const_iterator;
         iterator begin()
         { return alarms.begin(); }
         const_iterator begin() const
         { return alarms.begin(); }

         /**
          * @return Returns an iterator past the end of the list of alarms.
          */
         iterator end()
         { return alarms.end(); }
         const_iterator end() const
         { return alarms.end(); }

         /**
          * @return Returns true if there are no alarms.
          */
         bool empty() const
         { return alarms.empty(); }

         /**
          * @return Returns the number of alarms managed.
          */
         typedef alarms_type::size_type size_type;
         size_type size() const
         { return alarms.size(); }

         /**
          * @return Returns an iterator to the alarm with the specified name or end() if there is no
          * such alarm.
          */
         iterator find(StrUni const &name)
         {
            return std::find_if(begin(), end(), alarm_has_name(name));
         }
         const_iterator find(StrUni const &name) const
         {
            return std::find_if(begin(), end(), alarm_has_name(name));
         }

         /**
          * @return Returns a shared pointer to the alarm with the specified name or throws an
          * exception if there is no such alarm.
          */
         value_type &find_alarm(StrUni const &name)
         {
            iterator ai(find(name));
            if(ai == end())
               throw std::invalid_argument("invalid alarm name");
            return *ai;
         }
         value_type const &find_alarm(StrUni const &name) const
         {
            const_iterator ai(find(name));
            if(ai == end())
               throw std::invalid_argument("invalid alarm name");
            return *ai;
         }
         
         // @endgroup:

         /**
          * @return Generates a new email profile object and adds it to the set managed.
          */
         typedef SharedPtr<EmailProfile> profile_handle;
         profile_handle create_profile();

         /**
          * Adds the specified profile.
          */
         void add_profile(profile_handle profile);

         /**
          * @return Returns the profile with a unique id that matches the specified identifier.  If
          * there is no such profile, returns a null pointer.
          */
         profile_handle find_profile_id(StrUni const &id);

         /**
          * @return Returns the first profile that matches the specified name.
          */
         profile_handle find_profile_name(StrUni const &name);

         /**
          * Removes the specified profile.
          */
         void remove_profile(EmailProfile *profile)
         { remove_profile(profile->get_unique_id()); }
         void remove_profile(StrUni const &id);

         /**
          * Adds an action that needs to be executed for an alarm.
          */
         typedef SharedPtr<ActionBase> action_handle;
         void add_action(action_handle action);

         /**
          * Cancels any pending actions for the specified alarm.
          */
         void stop_actions_for_alarm(Alarm *alarm);
         
         /**
          * Called to report that the specified action is complete.
          */
         void on_action_complete(ActionBase *action);

         /**
          * @return Returns the number of pending actions for the specified alarm.
          */
         uint4 pending_actions_for_alarm(Alarm *alarm);

         /**
          * Removes all actions for the specified alarm.
          */
         void remove_actions_for_alarm(Alarm *alarm);

         /**
          * @return Returns the handle associated with the specified action pointer.
          */
         action_handle find_action(ActionBase *action);
         
         /**
          * @return Returns true if execute program actions are allowed.
          */
         bool get_exec_actions_allowed() const
         { return exec_actions_allowed; }

         /**
          * @param allowed Set to true if execute program actions should be allowed.
          */
         void set_exec_actions_allowed(bool allowed)
         { exec_actions_allowed = allowed; }

         /**
          * Adds an alarm log elment.
          */
         void add_log(Xml::Element &elem);

         /**
          * @return Returns the alarm logger.
          */
         SharedPtr<AlarmLogger> get_log()
         { return log; }

         /**
          * Writes the logging parameters for this manager.
          */
         void get_log(Xml::Element &elem);

         /**
          * Sets up the logging parameters for this manager.
          */
         void set_log(Xml::Element &elem);

         /**
          * @return Returns true if this manager has been started.
          */
         bool get_was_started() const
         { return was_started; }

         /**
          * @return Returns the alarm log directory.
          */
         StrAsc const &get_alarm_log_dir() const
         { return alarm_log_dir; }

         /**
            8 Sets the alarm log directory.
         */
         void set_alarm_log_dir(StrAsc const &dir)
         { alarm_log_dir = dir; }

      private:
         /**
          * Starts the process of executing the next action in the queue.
          */
         void do_next_action();
         
      private:
         /**
          * Specifies the data source manager.
          */
         sources_handle sources;

         /**
          * Specifies the collection of alarms.
          */
         alarms_type alarms;

         /**
          * Specifies the factory used to create tokens for the alarm source and condition
          * expressions.
          */
         Csi::SharedPtr<Expression::TokenFactory> token_factory;

         /**
          * Specifies the collection of email profiles for the application.
          */
         typedef std::map<StrUni, profile_handle> profiles_type;
         profiles_type profiles;

         /**
          * Specifies the collection of actions that need to be performed.
          */
         typedef std::list<action_handle> actions_type;
         actions_type actions;

         /**
          * Specifies the action that is currently being pereformed.
          */
         action_handle current_action;

         /**
          * Set to true if we will allow execute program actions.
          */
         bool exec_actions_allowed;

         /**
          * Specifies the object that bales the alarms log.
          */
         Csi::SharedPtr<AlarmLogger> log;

         /**
          * Set to true if this manager has been started.
          */
         bool was_started;

         /**
          * Specifies the directory where the alarms log will be written.
          */
         StrAsc alarm_log_dir;
      };
   };
};


#endif
