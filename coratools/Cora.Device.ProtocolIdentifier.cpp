/* Cora.Device.ProtocolIdentifier.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 24 July 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ProtocolIdentifier.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // identifier
            ////////////////////////////////////////////////////////////
            typedef ProtocolIdentifier identifier_type;
            identifier_type *identifier;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef identifier_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            void do_notify()
            {
               if(client_type::is_valid_instance(client))
                  notify();
            }

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               identifier_type *identifier_,
               client_type *client_):
               Event(event_id,identifier_),
               identifier(identifier_),
               client(client_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // event_status_notification
         ////////////////////////////////////////////////////////////
         class event_status_notification: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // status
            ////////////////////////////////////////////////////////////
            typedef client_type::status_type status_type;
            status_type status;
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_status_notification(identifier,status); }

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               identifier_type *identifier,
               client_type *client,
               status_type status)
            {
               try{(new event_status_notification(identifier,client,status))->post();}
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_status_notification(
               identifier_type *identifier,
               client_type *client,
               status_type status_):
               event_base(event_id,identifier,client),
               status(status_)
            { } 
         };


         uint4 const event_status_notification::event_id =
         Csi::Event::registerType("Cora::Device::ProtocolIdentifier::status_notification");


         ////////////////////////////////////////////////////////////
         // event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // protocol
            ////////////////////////////////////////////////////////////
            typedef client_type::protocol_type protocol_type;
            protocol_type protocol;

            ////////////////////////////////////////////////////////////
            // model_no
            ////////////////////////////////////////////////////////////
            typedef client_type::model_no_type model_no_type;
            model_no_type model_no;

            ////////////////////////////////////////////////////////////
            // pakbus_address
            ////////////////////////////////////////////////////////////
            uint2 pakbus_address;
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               client->on_complete(
                  identifier,
                  outcome,
                  protocol,
                  model_no,
                  pakbus_address);
            } // notify

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               identifier_type *identifier,
               client_type *client,
               outcome_type outcome,
               protocol_type protocol = client_type::protocol_unknown,
               model_no_type model_no = LgrNetId,
               uint2 pakbus_address = 0)
            {
               try
               {
                  (new event_complete(
                     identifier,
                     client,
                     outcome,
                     protocol,
                     model_no,
                     pakbus_address))->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               identifier_type *identifier,
               client_type *client,
               outcome_type outcome_,
               protocol_type protocol_,
               model_no_type model_no_,
               uint2 pakbus_address_):
               event_base(event_id,identifier,client),
               outcome(outcome_),
               protocol(protocol_),
               model_no(model_no_),
               pakbus_address(pakbus_address_)
            { } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::ProtocolIdentifier::complete");
      };


      ////////////////////////////////////////////////////////////
      // class ProtocolIdentifier definitions
      ////////////////////////////////////////////////////////////
      void ProtocolIdentifier::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void ProtocolIdentifier::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void ProtocolIdentifier::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish

      
      void ProtocolIdentifier::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::identify_logger_protocol_start_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4(max_baud_rate);
         if(get_interface_version() >= Csi::VersionNumber("1.3.4.31"))
            cmd.addBool(true);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void ProtocolIdentifier::on_devicebase_failure(
         devicebase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;   
         }
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure

      
      void ProtocolIdentifier::on_devicebase_session_failure()
      { on_devicebase_failure(devicebase_failure_session); }

      
      void ProtocolIdentifier::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::identify_logger_protocol_status_not)
            {
               uint4 tran_no;
               uint4 status;
               msg->readUInt4(tran_no);
               msg->readUInt4(status);
               event_status_notification::cpost(
                  this,
                  client,
                  static_cast<client_type::status_type>(status));
            }
            else if(msg->getMsgType() == Messages::identify_logger_protocol_stopped_not)
            {
               uint4 tran_no;
               uint4 server_outcome;
               uint4 protocol = client_type::protocol_unknown;
               uint4 model_no = LgrNetId;
               uint2 pakbus_address = 0;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(server_outcome);
               switch(server_outcome)
               {
               case 1:
                  msg->readUInt4(protocol);
                  if(get_interface_version() >= Csi::VersionNumber("1.3.4.31"))
                     msg->readUInt2(pakbus_address);
                  outcome = client_type::outcome_protocol_identified;
                  break;

               case 2:
                  msg->readUInt4(protocol);
                  if(get_interface_version() >= Csi::VersionNumber("1.3.4.31"))
                     msg->readUInt2(pakbus_address);
                  msg->readUInt4(model_no);
                  outcome = client_type::outcome_protocol_and_model_no_identified;
                  break;

               case 3:
                  outcome = client_type::outcome_communication_disabled;
                  break;

               case 4:
                  outcome = client_type::outcome_no_protocol_identified;
                  break;

               case 7:
                  outcome = client_type::outcome_link_failed;
                  break;

               default:
                  outcome = client_type::outcome_unknown;
                  break; 
               }
               event_complete::cpost(
                  this,
                  client,
                  outcome,
                  static_cast<client_type::protocol_type>(protocol),
                  static_cast<client_type::model_no_type>(model_no),
                  pakbus_address);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage
      
      
      void ProtocolIdentifier::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            if(event->getType() == event_complete::event_id)
               finish();
            event->do_notify();
         }
      } // receive 
   };
};



