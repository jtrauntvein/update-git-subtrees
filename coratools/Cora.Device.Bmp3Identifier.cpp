/* Cora.Device.Bmp3Identifier.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 July 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.Bmp3Identifier.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // identifier
            ////////////////////////////////////////////////////////////
            typedef Bmp3Identifier identifier_type;
            identifier_type *identifier;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef identifier_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // bmp_version
            ////////////////////////////////////////////////////////////
            byte bmp_version;

            ////////////////////////////////////////////////////////////
            // model_no
            ////////////////////////////////////////////////////////////
            uint4 model_no;

            ////////////////////////////////////////////////////////////
            // serial_no
            ////////////////////////////////////////////////////////////
            uint4 serial_no;

            ////////////////////////////////////////////////////////////
            // station_name
            ////////////////////////////////////////////////////////////
            StrAsc station_name;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               identifier_type *identifier,
               client_type *client,
               outcome_type outcome,
               byte bmp_version = 0,
               uint4 model_no = 0,
               uint4 serial_no = 0,
               StrAsc const &station_name = "")
            {
               try
               {
                  (new event_complete(
                     identifier,
                     client,
                     outcome,
                     bmp_version,
                     model_no,
                     serial_no,
                     station_name))->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               identifier_type *identifier_,
               client_type *client_,
               outcome_type outcome_,
               byte bmp_version_,
               uint4 model_no_,
               uint4 serial_no_,
               StrAsc const &station_name_):
               Event(event_id,identifier_),
               identifier(identifier_),
               client(client_),
               outcome(outcome_),
               bmp_version(bmp_version_),
               model_no(model_no_),
               serial_no(serial_no_),
               station_name(station_name_)
            { } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::Bmp3Idenfifier::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class Bmp3Identifier definitions
      ////////////////////////////////////////////////////////////
      Bmp3Identifier::Bmp3Identifier():
         client(0),
         state(state_standby)
      { }
      
         
      Bmp3Identifier::~Bmp3Identifier()
      { finish(); }

      
      void Bmp3Identifier::start(
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
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start
      
      
      void Bmp3Identifier::start(
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
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void Bmp3Identifier::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish

      
      void Bmp3Identifier::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(client_type::is_valid_instance(client) && client == event->client)
               client->on_complete(
                  this,
                  event->outcome,
                  event->bmp_version,
                  event->model_no,
                  event->serial_no,
                  event->station_name);
         }
      } // receive

      
      void Bmp3Identifier::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::identify_ack)
            {
               uint4 tran_no;
               uint4 server_outcome;
               byte bmp_version = 0;
               uint4 model_no = 0;
               uint4 serial_no = 0;
               StrAsc station_name;
               client_type::outcome_type outcome;

               msg->readUInt4(tran_no);
               msg->readUInt4(server_outcome);
               if(server_outcome == 1)
               {
                  msg->readByte(bmp_version);
                  msg->readUInt4(model_no);
                  msg->readUInt4(serial_no);
                  msg->readStr(station_name);
               }
               switch(server_outcome)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 2:
                  outcome = client_type::outcome_communication_failed;
                  break;

               case 3:
                  outcome = client_type::outcome_communication_disabled;
                  break;

               case 5:
                  outcome = client_type::outcome_logger_security_blocked;
                  break;

               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(
                  this,
                  client,
                  outcome,
                  bmp_version,
                  model_no,
                  serial_no,
                  station_name);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void Bmp3Identifier::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::identify_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addStr(station_name);
         router->sendMessage(&cmd);
         state = state_active; 
      } // on_devicebase_ready

      
      void Bmp3Identifier::on_devicebase_failure(devicebase_failure_type failure)
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
            outcome = client_type::outcome_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure

      
      void Bmp3Identifier::on_devicebase_session_failure()
      { on_devicebase_failure(devicebase_failure_session); }
   };
};



