/* Cora.Device.HoleDeleter.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 31 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.HoleDeleter.h"
#include <assert.h>


namespace Cora
{
   namespace Device
   {
      namespace HoleDeleterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            HoleDeleterClient *client;
            typedef HoleDeleterClient::resp_code_type resp_code_type;
            resp_code_type resp_code;

            static void create_and_post(HoleDeleter *deleter,
                                        HoleDeleterClient *client,
                                        resp_code_type resp_code);

         private:
            event_complete(HoleDeleter *deleter,
                           HoleDeleterClient *client_,
                           resp_code_type resp_code_):
               Event(event_id,deleter),
               client(client_),
               resp_code(resp_code_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::HoleDeleter::event_complete");


         void event_complete::create_and_post(HoleDeleter *deleter,
                                              HoleDeleterClient *client,
                                              resp_code_type resp_code)
         {
            try { (new event_complete(deleter,client,resp_code))->post(); }
            catch(Csi::Event::BadPost &) { }
         }
      };


      ////////////////////////////////////////////////////////////
      // class HoleDeleter definitions
      ////////////////////////////////////////////////////////////
      void HoleDeleter::start(
         HoleDeleterClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(HoleDeleterClient::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      } // start


      void HoleDeleter::start(
         HoleDeleterClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(HoleDeleterClient::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void HoleDeleter::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish

      
      void HoleDeleter::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session,Messages::holes_delete_cmd);
         command.addUInt4(++last_tran_no);
         if(interface_version >= Csi::VersionNumber("1.3.8.7"))
         {
            if(table_name.length() > 0)
            {
               command.addWStr(table_name);
               if(send_newest_record_no)
                  command.addUInt4(newest_record_no);
            }
         }
         else
         {
            if(table_name.length() > 0)
            {
               command.addUInt4(2);
               command.addWStr(table_name);
            }
            else
               command.addUInt4(1);
         }
         state = state_active;
         router->sendMessage(&command);
      } // on_devicebase_ready

      
      void HoleDeleter::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace HoleDeleterHelpers;
         HoleDeleterClient::resp_code_type resp_code;
         switch(failure)
         {
         case devicebase_failure_logon:
            resp_code = HoleDeleterClient::resp_invalid_logon;
            break;
            
         case devicebase_failure_session:
            resp_code = HoleDeleterClient::resp_session_lost;
            break;

         case devicebase_failure_invalid_device_name:
            resp_code = HoleDeleterClient::resp_invalid_device_name;
            break;

         case devicebase_failure_unsupported:
            resp_code = HoleDeleterClient::resp_unsupported;
            break;
            
         case devicebase_failure_security:
            resp_code = HoleDeleterClient::resp_security_blocked;
            break;
            
         default:
            resp_code = HoleDeleterClient::resp_unknown;
            break;
         }
         event_complete::create_and_post(this,client,resp_code);
      } // on_devicebase_failure

      
      void HoleDeleter::on_devicebase_session_failure()
      {
         using namespace HoleDeleterHelpers;
         event_complete::create_and_post(this,client,HoleDeleterClient::resp_session_lost);
      } // on_devicebase_session_failure

      
      void HoleDeleter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace HoleDeleterHelpers;
         event_complete *event = dynamic_cast<event_complete *>(ev.get_rep());
         assert(event != 0);
         finish();
         if(HoleDeleterClient::is_valid_instance(event->client))
            event->client->on_complete(this,event->resp_code); 
      } // receive

      
      void HoleDeleter::onNetMessage(Csi::Messaging::Router *rtr,
                                     Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::holes_delete_ack)
            {
               using namespace HoleDeleterHelpers;
               uint4 tran_no;
               uint4 resp_code;

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               if(resp_code == 1)
                  event_complete::create_and_post(this,client,HoleDeleterClient::resp_succeeded);
               else
                  event_complete::create_and_post(this,client,HoleDeleterClient::resp_unknown);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   };
};
