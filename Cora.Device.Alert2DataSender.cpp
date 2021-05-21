/* Cora.Device.Alert2DataSender.cpp

   Copyright (C) 2016, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 14 July 2016
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.Alert2DataSender.h"
#include "Csi.Utils.h"
#include <iostream>


namespace Cora
{
   namespace Device
   {
      namespace
      {
         class event_fragment_sent: public Csi::Event
         {
         public:
            static uint4 const event_id;

            int8 const bytes_sent;
            int8 const total_bytes;

            static void cpost(Alert2DataSender *sender, int8 bytes_sent, int8 total_bytes)
            { (new event_fragment_sent(sender, bytes_sent, total_bytes))->post(); }

         private:
            event_fragment_sent(Alert2DataSender *sender, int8 bytes_sent_, int8 total_bytes_):
               Event(event_id, sender),
               bytes_sent(bytes_sent_),
               total_bytes(total_bytes_)
            { }
         };


         uint4 const event_fragment_sent::event_id(
            Csi::Event::registerType("Cora::Device::Alert2DataSender::event_fragment_sent"));


         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;

            Alert2DataSenderClient::outcome_type const outcome;

            static void cpost(Alert2DataSender *sender, Alert2DataSenderClient::outcome_type outcome)
            { (new event_complete(sender, outcome))->post(); }

         private:
            event_complete(Alert2DataSender *sender, Alert2DataSenderClient::outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Device::Alert2DataSender::event_complete"));
      };


      Alert2DataSenderFileSource::Alert2DataSenderFileSource(StrAsc const &file_name)
      {
         input = Csi::open_file(file_name, "rb");
         if(input == 0)
            throw Csi::OsException("failed to open the alert2 source");
      } // constructor


      Alert2DataSenderFileSource::~Alert2DataSenderFileSource()
      {
         if(input)
            fclose(input);
         input = 0;
      } // destructor


      uint4 Alert2DataSenderFileSource::get_next_fragment(
         Alert2DataSender *sender, void *buff, uint4 buff_len)
      {
         uint4 rtn(0);
         if(input)
            rtn = (uint4)fread(buff, 1, buff_len, input);
         return rtn;
      } // get_next_fragment


      int8 Alert2DataSenderFileSource::get_total_size(Alert2DataSender *sender)
      {
         int8 rtn(0);
         if(input)
            rtn = Csi::long_file_length(input);
         return rtn;
      } // get_total_size


      void Alert2DataSender::on_devicebase_ready()
      {
         state = state_active;
         tran_no = ++last_tran_no;
         bytes_sent = 0;
         send_next_fragment();
      } // on_devicebase_ready


      void Alert2DataSender::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_failure_logon;
            break;

         case devicebase_failure_session:
            outcome = client_type::outcome_failure_session;
            break;

         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_failure_invalid_device_name;
            break;

         case devicebase_failure_unsupported:
            outcome = client_type::outcome_failure_unsupported;
            break;

         case devicebase_failure_security:
            outcome = client_type::outcome_failure_security;
            break;
         }
         event_complete::cpost(this, outcome);
      } // on_devicebase_failure


      void Alert2DataSender::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::send_alert2_data_ack)
            {
               uint4 tran;
               uint4 response;
               message->readUInt4(tran);
               message->readUInt4(response);
               if(response == 1 || response == 3)
               {
                  event_fragment_sent::cpost(this, bytes_sent, source->get_total_size(this));
                  if(response == 1)
                     send_next_fragment();
                  else
                     event_complete::cpost(this, client_type::outcome_success);
               }
               else
                  event_complete::cpost(this, client_type::outcome_failure_unknown);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage


      void Alert2DataSender::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_fragment_sent::event_id)
         {
            event_fragment_sent *event(static_cast<event_fragment_sent *>(ev.get_rep()));
            if(client_type::is_valid_instance(client))
               client->on_fragment_sent(this, event->bytes_sent, event->total_bytes);
            else
               finish();
         }
         else if(ev->getType() == event_complete::event_id)
         {
            event_complete *event(static_cast<event_complete *>(ev.get_rep()));
            client_type *sender(client);
            finish();
            if(client_type::is_valid_instance(sender))
               sender->on_complete(this, event->outcome);
         }
      } // receive


      void Alert2DataSender::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;

         case client_type::outcome_failure_security:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_failure_source:
            out << "message source failure";
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome


      void Alert2DataSender::send_next_fragment()
      {
         try
         {
            uint4 bytes_read(source->get_next_fragment(this, tx_buff, sizeof(tx_buff)));
            Csi::Messaging::Message command(device_session, Messages::send_alert2_data_cmd);
            command.addUInt4(tran_no);
            command.addBytes(tx_buff, bytes_read);
            bytes_sent += bytes_read;
            command.addBool(bytes_sent >= source->get_total_size(this));
            router->sendMessage(&command);
         }
         catch(std::exception &)
         {
            event_complete::cpost(this, client_type::outcome_failure_source);
         }
      } // send_next_fragment
   };
};

