/* File Name: $RCSfile: Cora.Device.TerminalEmulator.cpp,v $

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written By: Tyler Mecham
   Date Begun: 7/9/2002 12:08:17 PM

   Last Changed By: $Author: jon $
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   File Revision Number: $Revision: 27879 $
*/

#pragma hdrstop

#include "Cora.Device.TerminalEmulator.h"
#include "Cora.Device.Defs.h"

namespace Cora
{
   namespace Device
   {
      namespace TerminalEmulatorHelpers
      {
         ////////////////////////////////////////////////////////////
         // Class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            TerminalEmulator *tran;
            TerminalEmulatorClient *client;

            ////////// constructor
            event_base(uint4 event_id,
                       TerminalEmulator *tran_,
                       TerminalEmulatorClient *client_):
               tran(tran_),
               client(client_),
               Event(event_id,tran_)
            { }

            ////////// notify
            // Called by the enumerator to send the notification to the client 
            virtual void notify() = 0;
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////

         class event_start: public event_base
         {
         public:
            static uint4 const event_id;

            static void create_and_post(TerminalEmulator *tran,
                                        TerminalEmulatorClient *client);

            virtual void notify() { client->on_started(tran); }
            
         private:
            event_start(TerminalEmulator *tran,
                        TerminalEmulatorClient *client):
               event_base(event_id,tran,client)
            { }
         };


         uint4 const event_start::event_id =
         Csi::Event::registerType("Cora::Device::TerminalEmulator::event_start");


         void event_start::create_and_post(TerminalEmulator *tran,
                                        TerminalEmulatorClient *client)
         {
            try { (new event_start(tran,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         }


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////

         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;
            typedef TerminalEmulatorClient::failure_type failure_type;
            failure_type failure;

            static void create_and_post(TerminalEmulator *tran,
                                        TerminalEmulatorClient *client,
                                        failure_type failure);

            virtual void notify() { client->on_failure(tran,failure); }
            
         private:
            event_failure(TerminalEmulator *tran,
                        TerminalEmulatorClient *client,
                          failure_type failure_):
               event_base(event_id,tran,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::TerminalEmulator::event_failure");


         void event_failure::create_and_post(TerminalEmulator *tran,
                                             TerminalEmulatorClient *client,
                                             failure_type failure)
         {
            try { (new event_failure(tran,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         }

      
         ////////////////////////////////////////////////////////////
         // class event_received
         ////////////////////////////////////////////////////////////

         class event_received: public event_base
         {
         public:
            static uint4 const event_id;
            bool more;
            StrBin bytes;

            static void create_and_post(TerminalEmulator *tran,
                                        TerminalEmulatorClient *client,
                                        bool more,
                                        StrBin const &bytes);

            virtual void notify() { client->on_received(tran,more,bytes); }
            
         private:
            event_received(TerminalEmulator *tran,
                        TerminalEmulatorClient *client,
                        bool more_,
                        StrBin const &bytes_):
               event_base(event_id,tran,client),
               more(more_),bytes(bytes_)
            { }
         };


         uint4 const event_received::event_id =
         Csi::Event::registerType("Cora::Device::TerminalEmulator::event_received");


         void event_received::create_and_post(TerminalEmulator *tran,
                                             TerminalEmulatorClient *client,
                                             bool more,
                                             StrBin const &bytes)
         {
            try { (new event_received(tran,client,more,bytes))->post(); }
            catch(Csi::Event::BadPost &) { }
         }
      };

      
      ////////////////////////////////////////////////////////////
      // Class TerminalEmulator
      ////////////////////////////////////////////////////////////
      TerminalEmulator::TerminalEmulator():
         state(state_standby),
         client(0),
         term_emu_tran(0),
         max_baud_rate(0)
      {
      }

      TerminalEmulator::~TerminalEmulator()
      {
         finish();
      }


      void TerminalEmulator::set_max_baud_rate(uint4 max_baud_rate_)
      {
         if(state == state_standby)
            max_baud_rate = max_baud_rate_;
         else
            throw exc_invalid_state();
      } // set_max_baud_rate
      
      
      void TerminalEmulator::start(
         TerminalEmulatorClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            state = state_waiting;
            client = client_;
            DeviceBase::start(router);
         }
         else
            throw exc_invalid_state();
      } // start


      void TerminalEmulator::start(
         TerminalEmulatorClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            state = state_waiting;
            client = client_;
            DeviceBase::start(other_component);
         }
         else
            throw exc_invalid_state();
      } // start

      
      void TerminalEmulator::on_devicebase_ready()
      {
         //We have a device session now, so let's start the terminal emulation tran
         Csi::Messaging::Message start_command(device_session,
                                               Messages::terminal_emu_start_cmd);
         start_command.addUInt4(term_emu_tran = ++last_tran_no);
         if(max_baud_rate != 0)
            start_command.addUInt4(max_baud_rate);
         router->sendMessage(&start_command);
      } // on_devicebase_ready

      
      void TerminalEmulator::on_term_start_ack(Csi::Messaging::Message *msg)
      {
         uint4 tranNo,respCode;
         if( !msg->readUInt4(tranNo) )
         {
            TerminalEmulatorHelpers::event_failure::create_and_post(this,client,TerminalEmulatorClient::failure_unknown);
            return;
         }
         if( !msg->readUInt4(respCode) )
         {
            TerminalEmulatorHelpers::event_failure::create_and_post(this,client,TerminalEmulatorClient::failure_unknown);
            return;
         }

         if( respCode == 1 )
         {
            state = state_active;
            using namespace TerminalEmulatorHelpers;
            event_start::create_and_post(this,client);
         }
         else //An error occured
         {
            TerminalEmulatorClient::failure_type client_failure;
            if( respCode == 2 ) //Device already servicing a different term emu tran
            {
               client_failure = TerminalEmulatorClient::failure_already_servicing_tran;
            }
            else // Comm disabled
            {
               client_failure = TerminalEmulatorClient::failure_comm_disabled;
            }
            TerminalEmulatorHelpers::event_failure::create_and_post(this,client,client_failure);
         }
      }


      void TerminalEmulator::send_bytes(StrBin const &bytes)
      {
         if( state == state_active )
         {
            //We have a device session now, so let's start the terminal emulation tran
            Csi::Messaging::Message send_command(device_session,
                                                  Messages::terminal_emu_send_cmd);
            send_command.addUInt4(term_emu_tran);
            send_command.addBytes(bytes.getContents(), (uint4)bytes.getLen());
            router->sendMessage(&send_command);
         }
         else
            throw exc_invalid_state();
      }
      
      
      void TerminalEmulator::on_term_send_ack(Csi::Messaging::Message *msg)
      {
         //This should never happen, but never say never
         uint4 tranNo,respCode=0;
         msg->readUInt4(tranNo);
         msg->readUInt4(respCode);
         if( respCode != 1 )
            TerminalEmulatorHelpers::event_failure::create_and_post(this,client,TerminalEmulatorClient::failure_send_failed);
      }


      void TerminalEmulator::on_term_received(Csi::Messaging::Message *msg)
      {
         uint4 tranNo;
         bool more;
         StrBin bytes;
         if( msg->readUInt4(tranNo) )
         {
            if( msg->readBool(more) )
            {
               if( msg->readBStr(bytes) )
               {
                  TerminalEmulatorHelpers::event_received::create_and_post(this,client,more,bytes);
                  return;
               }
            }
         }
         TerminalEmulatorHelpers::event_failure::create_and_post(this,client,TerminalEmulatorClient::failure_unknown);
      }

         
      void TerminalEmulator::finish()
      {
         state = state_standby;
         client = 0;
         term_emu_tran = 0;
         DeviceBase::finish();
      } // finish


      void TerminalEmulator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TerminalEmulatorHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());

         if(ev->getType() == event_failure::event_id)
            finish();
         if(event && TerminalEmulatorClient::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      } // receive

      
      void TerminalEmulator::onNetMessage(Csi::Messaging::Router *rtr,
                                          Csi::Messaging::Message *msg)
      {
         if(state != state_standby)
         {
            switch(msg->getMsgType())
            {
               case Messages::terminal_emu_start_ack:
               {
                  on_term_start_ack(msg);
                  break;
               }
               case Messages::terminal_emu_send_ack:
               {
                  on_term_send_ack(msg);
                  break;
               }
               case Messages::terminal_emu_receive_not:
               {
                  on_term_received(msg);
                  break;
               }
               default:
               {
                  DeviceBase::onNetMessage(rtr,msg);
                  break;
               }
            }
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void TerminalEmulator::on_devicebase_failure(devicebase_failure_type failure)
      {
         TerminalEmulatorClient::failure_type client_failure;
         
         switch(failure)
         {
            case devicebase_failure_logon:
               client_failure = TerminalEmulatorClient::failure_invalid_logon;
               break;
            
            case devicebase_failure_session:
               client_failure = TerminalEmulatorClient::failure_connection_failed;
               break;
            
            case devicebase_failure_invalid_device_name:
               client_failure = TerminalEmulatorClient::failure_device_name_invalid;
               break;
            
            default:
               client_failure = TerminalEmulatorClient::failure_unknown;
               break;
         }
         TerminalEmulatorHelpers::event_failure::create_and_post(this,client,client_failure);
      } // on_devicebase_failure

      
      void TerminalEmulator::on_devicebase_session_failure()
      {
         using namespace TerminalEmulatorHelpers;
         event_failure::create_and_post(this,
                                        client,
                                        TerminalEmulatorClient::failure_connection_failed);
      } // on_devicebase_session_failure
   };
};
