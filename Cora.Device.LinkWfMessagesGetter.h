/* Cora.Device.LinkWfMessagesGetter.h

   Copyright (C) 2005, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 05 January 2005
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_LinkWfMessagesGetter_h
#define Cora_Device_LinkWfMessagesGetter_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"
#include "Csi.LgrDate.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class LinkWfMessagesGetter;
      //@endgroup


      namespace LinkWfMessagesGetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class wf_message
         ////////////////////////////////////////////////////////////
         class wf_message
         {
         public:
            ////////////////////////////////////////////////////////////
            // dev_name
            ////////////////////////////////////////////////////////////
            StrUni dev_name;

            ////////////////////////////////////////////////////////////
            // stamp
            ////////////////////////////////////////////////////////////
            Csi::LgrDate stamp;

            ////////////////////////////////////////////////////////////
            // severity
            ////////////////////////////////////////////////////////////
            SwfCode severity;

            ////////////////////////////////////////////////////////////
            // text
            ////////////////////////////////////////////////////////////
            StrAsc text;

            ////////////////////////////////////////////////////////////
            // default constructor
            ////////////////////////////////////////////////////////////
            wf_message():
               severity(swf_warning)
            { }

            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            wf_message &operator =(wf_message const &other)
            {
               dev_name = other.dev_name;
               stamp = other.stamp;
               severity = other.severity;
               text = other.text; 
               return *this;
            }
            
            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            wf_message(wf_message const &other):
               dev_name(other.dev_name),
               stamp(other.stamp),
               severity(other.severity),
               text(other.text)
            { } 
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class LinkWfMessagesGetterClient
      ////////////////////////////////////////////////////////////
      class LinkWfMessagesGetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has been completed.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_failed = 3,
            outcome_invalid_device_name = 4,
            outcome_unsupported = 5,
            outcome_security_blocked = 6
         };
         typedef LinkWfMessagesGetterHelpers::wf_message message_type;
         typedef std::list<message_type> messages_type;
         virtual void on_complete(
            LinkWfMessagesGetter *getter,
            outcome_type outcome,
            messages_type const &messages) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class LinkWfMessagesGetter
      //
      // Defines a component that can be used to retrieve the queued
      // warning/fault messages for a device and its parents.  In order to use
      // this component, an application must provide an object derived from
      // LinkWfMessagesGetterClient (typedefed as client_type within this
      // class) and must create an instance of this class.  Once the properties
      // have been set including the device name, the application should invoke
      // one of the two start methods.  When the server transaction is
      // complete, this component will inform the application by invoking the
      // client object's on_complete() method.  The queued messages, if any
      // will be passed as a parameter to on_complete().
      ////////////////////////////////////////////////////////////
      class LinkWfMessagesGetter:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         LinkWfMessagesGetterClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LinkWfMessagesGetter():
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LinkWfMessagesGetter()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // start (new router)
         ////////////////////////////////////////////////////////////
         typedef LinkWfMessagesGetterClient client_type;
         void start(
            client_type *client_,
            router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // start (based on other component)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_component);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            DeviceBase::finish();
         }

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);
         
         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }
      };
   };
};


#endif
