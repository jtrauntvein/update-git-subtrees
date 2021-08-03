/* Cora.Sec2.AccountAdder.h

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 30 December 2002
   Last Change: Monday 04 January 2016
   Last Commit: $Date: 2019-10-29 14:47:04 -0600 (Tue, 29 Oct 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_Sec2_AccountAdder_h
#define Cora_Sec2_AccountAdder_h

#include "Cora.Sec2.Sec2Base.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>
#include <algorithm>


namespace Cora
{
   namespace Sec2
   {
      //@group class forward declarations
      class AccountAdder;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class AccountAdderClient
      ////////////////////////////////////////////////////////////
      class AccountAdderClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_connection_failed = 2,
            outcome_invalid_logon = 3,
            outcome_insufficient_access = 4,
            outcome_unsupported = 5,
            outcome_locked = 6,
            outcome_invalid_account_name = 7,
            outcome_not_admin = 8
         };
         virtual void on_complete(
            AccountAdder *adder,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class AccountAdder
      //
      // Defines a component that can be used to add a security account on a LoggerNet server that
      // supports the Security2 interface.  In order to use this component, an application must
      // provide an object that inherits from class AccountAdderClient (typedefed as client_type
      // within this class definition).  It should create an instance of this class, call
      // set_account_name(), set_account_password(), set_access_level(), and
      // set_device_additions().  The application can also optionally call set_logon_name() and
      // set_logon_password().  Once the component has been set up as described above, the
      // application can call one of the two versions of start() which will start the sequence of
      // events that will lead to carrying out the add account transaction with the server.  When
      // the server transaction is complete, the component will invoke the client's on_complete()
      // method.  The application can cancel this notification (but not necessarily the server
      // transaction) by invoking finish() at any time.
      //
      // Once finish() has been called or the server transaction completed, the component will be
      // placed in a standby state where it can be used again by the application if needed. 
      ////////////////////////////////////////////////////////////
      class AccountAdder:
         public Sec2Base,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // account_name
         ////////////////////////////////////////////////////////////
         StrUni account_name;

         ////////////////////////////////////////////////////////////
         // account_password
         ////////////////////////////////////////////////////////////
         StrUni account_password;

         ////////////////////////////////////////////////////////////
         // access_level
         ////////////////////////////////////////////////////////////
         AccessLevels::AccessLevelType access_level;

         ////////////////////////////////////////////////////////////
         // device_additions
         ////////////////////////////////////////////////////////////
      public:
         typedef std::list<StrUni> device_additions_type;
      private:
         device_additions_type device_additions;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
      public:
         typedef AccountAdderClient client_type;
      private:
         client_type *client;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         AccountAdder();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~AccountAdder();

         ////////////////////////////////////////////////////////////
         // set_account_name
         ////////////////////////////////////////////////////////////
         void set_account_name(StrUni const &account_name_)
         {
            if(state == state_standby)
               account_name = account_name_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_account_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_account_name() const
         { return account_name; }

         ////////////////////////////////////////////////////////////
         // set_account_password
         ////////////////////////////////////////////////////////////
         void set_account_password(StrUni const &account_password_)
         {
            if(state == state_standby)
               account_password = account_password_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_account_password
         ////////////////////////////////////////////////////////////
         StrUni const &get_account_password() const
         { return account_password; }

         ////////////////////////////////////////////////////////////
         // set_access_level
         ////////////////////////////////////////////////////////////
         void set_access_level(AccessLevels::AccessLevelType access_level_)
         {
            if(state == state_standby)
               access_level = access_level_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_access_level
         ////////////////////////////////////////////////////////////
         AccessLevels::AccessLevelType get_access_level() const
         { return access_level; }

         ////////////////////////////////////////////////////////////
         // clear_device_additions
         ////////////////////////////////////////////////////////////
         void clear_device_additions()
         {
            if(state == state_standby)
               device_additions.clear();
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // add_device_addition
         ////////////////////////////////////////////////////////////
         void add_device_addition(StrUni const &device_name)
         {
            if(state == state_standby)
               device_additions.push_back(device_name);
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // set_device_additions
         ////////////////////////////////////////////////////////////
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

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         device_additions_type::size_type size() const
         { return device_additions.size(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return device_additions.empty(); }

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef device_additions_type::iterator iterator;
         typedef device_additions_type::const_iterator const_iterator;
         iterator begin()
         { return device_additions.begin(); }
         const_iterator begin() const
         { return device_additions.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return device_additions.end(); }
         const_iterator end() const
         { return device_additions.end(); }
         
         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_client);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         /**
          * Formats the specified outcome code to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome to format.
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         ////////////////////////////////////////////////////////////
         // on_sec2base_ready
         ////////////////////////////////////////////////////////////
         virtual void on_sec2base_ready();

         ////////////////////////////////////////////////////////////
         // on_sec2base_failure
         ////////////////////////////////////////////////////////////
         virtual void on_sec2base_failure(sec2base_failure_type failure);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev); 
      };
   };
};


#endif
