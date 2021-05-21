/* Cora.Sec2.AccountChanger.h

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 30 December 2002
   Last Change: Sunday 06 October 2019
   Last Commit: $Date: 2019-10-07 06:58:27 -0600 (Mon, 07 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Sec2_AccountChanger_h
#define Cora_Sec2_AccountChanger_h

#include "Cora.Sec2.Sec2Base.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>
#include <algorithm>


namespace Cora
{
   namespace Sec2
   {
      class AccountChanger;


      /**
       * Defines the interface that the application must implement in order to use the
       * AccountChnager component type.
       */
      class AccountChangerClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the LoggerNet transaction has been completed.
          *
          * @param sender Specifies the component that has invoked this method.
          *
          * @param outcome Specifies a numeric code that identifies the outcome of the LoggerNet
          * transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_connection_failed = 2,
            outcome_invalid_logon = 3,
            outcome_insufficient_access = 4,
            outcome_unsupported = 5,
            outcome_locked = 6,
            outcome_invalid_account_name = 7
         };
         virtual void on_complete(AccountChanger *adder, outcome_type outcome) = 0;
      };


      /**
       * Defines a component type that can be used to change the properties of an existing LoggerNet
       * security account.  In order to use this component, the application must provide an object
       * that implements the AccountChangerClient interface.  It should then create an instance of
       * this class, set the appropriate properties such as account name, password, access level,
       * and device exceptions, and then call one of the two versions of start().  When the
       * LoggerNet transaction has been completed, the component will call the client's
       * on_complete() method.
       */
      class AccountChanger:
         public Sec2Base,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the name of the LoggerNet security account that should be changed.
          */
         StrUni account_name;

         /**
          * Specifies the new password for the account.
          */
         StrUni account_password;

         /**
          * Specifies the assigned access level for the account.
          */
         AccessLevels::AccessLevelType access_level;

         /**
          * Specifies the list of device names for which the account will be assigned operator
          * access level
          */
      public:
         typedef std::list<StrUni> device_additions_type;
      private:
         device_additions_type device_additions;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the application object that will receive completion notification.
          */
      public:
         typedef AccountChangerClient client_type;
      private:
         client_type *client;
         
      public:
         /**
          * Constructor
          */
         AccountChanger();

         /**
          * Destructor
          */
         virtual ~AccountChanger();

         /**
          * @param account_name_ Specifies the name of the security account to be changed.
          */
         void set_account_name(StrUni const &account_name_)
         {
            if(state == state_standby)
               account_name = account_name_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the name of the security account to be changed.
          */
         StrUni const &get_account_name() const
         { return account_name; }

         /**
          * @param account_password_ Specifies the new password to be assigned to the account.
          */
         void set_account_password(StrUni const &account_password_)
         {
            if(state == state_standby)
               account_password = account_password_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the new password to be assigned to the account.
          */
         StrUni const &get_account_password() const
         { return account_password; }

         /**
          * @param access_level_ Specifies the access level that should be assigned to the account.
          */
         void set_access_level(AccessLevels::AccessLevelType access_level_)
         {
            if(state == state_standby)
               access_level = access_level_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the access level to be assigned to the account.
          */
         AccessLevels::AccessLevelType get_access_level() const
         { return access_level; }

         /**
          * Removes all names for the list of devices for which this account has at least operator
          * access.
          */
         void clear_device_additions()
         {
            if(state == state_standby)
               device_additions.clear();
            else
               throw exc_invalid_state();
         }

         /**
          * @param device_name Specifies the name of a device for which the account will have
          * operator access.
          */
         void add_device_addition(StrUni const &device_name)
         {
            if(state == state_standby)
               device_additions.push_back(device_name);
            else
               throw exc_invalid_state();
         }

         /**
          * @param begin Specifies the begin iterator for a container of strings that specifies
          * names for devics for which the account will have operator permissions.
          *
          * @param end Specifies the end iterator for the container for device name strings.
          *
          * @param clear_first Set to true if the existing list of devices should be cleared before
          * assigning the new list.
          */
         template <class iter>
         void set_device_additions(iter begin, iter end, bool clear_first = true)
         {
            if(state == state_standby)
            {
               if(clear_first)
                  device_additions.clear();
               std::copy(begin,end,std::back_inserter(device_additions));
            }
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returnd the number of devices for which the account will be assigned operator
          * access.
          */
         device_additions_type::size_type size() const
         { return device_additions.size(); }

         /**
          * @return Returns true if there are no devices for which the account will be assigned
          * operator access
          */
         bool empty() const
         { return device_additions.empty(); }

         /**
          * @return Returns the begin iterator for the list of device names for which the account
          * will be assigned operator access.
          */
         typedef device_additions_type::iterator iterator;
         typedef device_additions_type::const_iterator const_iterator;
         iterator begin()
         { return device_additions.begin(); }
         const_iterator begin() const
         { return device_additions.begin(); }

         /**
          * @return Returns the end iterator for the list of device nwmes for which the account will
          * be assigned operator access.
          */
         iterator end()
         { return device_additions.end(); }
         const_iterator end() const
         { return device_additions.end(); }
         
         /**
          * Starts the LoggerNet transaction.
          *
          * @param client_ Specifies the application object that will receive completion
          * notification.
          *
          * @param router Specifies the newly created router object that has not been previously
          * connected to the LoggerNet server.
          *
          * @param other_client Specifies another component that has an active connection to the
          * LoggerNet server that can be shared.
          */
         void start(client_type *client_, router_handle &router);
         void start(client_type *client_, ClientBase *other_client);

         /**
          * Called to restore this component to a standby state.
          */
         virtual void finish();

         /**
          * Writes a description of the specifies failure code to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome code.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overloads the base class version to handle incoming asynchronous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev); 

      protected:
         /**
          * Overloads the base class version to start the server transaction.
          */
         virtual void on_sec2base_ready();

         /**
          * Overloads the base class version to handle failure reports.
          */
         virtual void on_sec2base_failure(sec2base_failure_type failure);

         /**
          * Overloads the base class version to handle incoming messages from the server.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

      };
   };
};


#endif
