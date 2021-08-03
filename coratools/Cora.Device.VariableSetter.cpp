/* Cora.Device.VariableSetter.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 08 June 2000
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.VariableSetter.h"
#include "Cora.Broker.TableDesc.h"
#include "Csi.StringLoader.h"
#include "Csi.BuffStream.h"
#include "coratools.strings.h"
#include <algorithm>
#include <assert.h>
#include <iterator>


namespace Cora
{
   namespace Device
   {
      namespace VariableSetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete declaration and definitions
         ////////////////////////////////////////////////////////////

         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            VariableSetterClient *client;
            VariableSetterClient::outcome_type outcome;
            static void create_and_post(VariableSetter *setter,
                                        VariableSetterClient *client,
                                        VariableSetterClient::outcome_type outcome);

         private:
            event_complete(VariableSetter *setter,
                           VariableSetterClient *client,
                           VariableSetterClient::outcome_type outcome); 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::VariableSetterHelpers::event_complete");


         void event_complete::create_and_post(VariableSetter *setter,
                                              VariableSetterClient *client,
                                              VariableSetterClient::outcome_type outcome)
         {
            try
            {
               event_complete *ev = new event_complete(setter,client,outcome);
               ev->post();
            }
            catch(Csi::Event::BadPost &)
            { }
         } // create_and_post


         event_complete::event_complete(VariableSetter *setter,
                                        VariableSetterClient *client_,
                                        VariableSetterClient::outcome_type outcome_):
            Event(event_id,setter),
            client(client_),
            outcome(outcome_)
         { } 
      };


      ////////////////////////////////////////////////////////////
      // class VariableSetter definitions
      ////////////////////////////////////////////////////////////
      VariableSetter::VariableSetter():
         value_type(value_type_unset),
         state(state_standby),
         locale(Csi::StringLoader::make_locale())
      { }

      
      VariableSetter::~VariableSetter()
      { finish(); }

      
      void VariableSetter::set_table_name(StrUni const &table_name_)
      {
         if(state == state_standby)
            table_name = table_name_;
         else
            throw exc_invalid_state();
      } // set_table_name

      
      void VariableSetter::set_column_name(StrUni const &column_name_)
      {
         if(state == state_standby)
            column_name = column_name_;
         else
            throw exc_invalid_state();
      } // set_column_name

      
      void VariableSetter::set_index(index_type const &index_)
      {
         if(state == state_standby)
            index = index_;
         else
            throw exc_invalid_state();
      } // set_index

      
      void VariableSetter::set_index(char const *index_string)
      {
         if(state == state_standby)
         {
            Csi::IBuffStream input(index_string, strlen(index_string));
            index.clear();
            std::copy(
               std::istream_iterator<int4>(input),
               std::istream_iterator<int4>(),
               std::back_inserter(index));
         }
         else
            throw exc_invalid_state(); 
      } // set_index

      
      void VariableSetter::set_index(wchar_t const *index_string)
      {
         if(state == state_standby)
         {
            Csi::IBuffStreamw input(index_string, wcslen(index_string));
            index.clear();
            std::copy(
               std::istream_iterator<int4, wchar_t>(input),
               std::istream_iterator<int4, wchar_t>(),
               std::back_inserter(index));
         }
         else
            throw exc_invalid_state(); 
      } // set_index

      
      void VariableSetter::set_value_bool(bool value_bool_)
      {
         if(state == state_standby)
         {
            value_type = value_type_bool;
            value_variant.v_bool = value_bool_;
         }
         else
            throw exc_invalid_state();
      } // set_value_bool

      
      void VariableSetter::set_value_float(float value_float_)
      {
         if(state == state_standby)
         {
            value_variant.v_float = value_float_;
            value_type = value_type_float;
         }
         else
            throw exc_invalid_state();
      } // set_value_float


      void VariableSetter::set_value_double(double value_double_)
      {
         if(state == state_standby)
         {
            value_variant.v_double = value_double_;
            value_type = value_type_double;
         }
         else
            throw exc_invalid_state();
      } // set_value_double

      
      void VariableSetter::set_value_uint4(uint4 value)
      {
         if(state == state_standby)
         {
            value_variant.v_uint4 = value;
            value_type = value_type_uint4;
         }
         else
            throw exc_invalid_state();
      } // set_value_uint4


      void VariableSetter::set_value_int4(int4 value)
      {
         if(state == state_standby)
         {
            value_variant.v_int4 = value;
            value_type = value_type_int4;
         }
         else
            throw exc_invalid_state();
      } // set_value_int4


      void VariableSetter::set_value_uint2(uint2 value)
      {
         if(state == state_standby)
         {
            value_variant.v_uint2 = value;
            value_type = value_type_uint2;
         }
         else
            throw exc_invalid_state();
      } // set_value_uint2

      
      void VariableSetter::set_value_int2(int2 value)
      {
         if(state == state_standby)
         {
            value_variant.v_int2 = value;
            value_type = value_type_int2;
         }
         else
            throw exc_invalid_state();
      } // set_value_int2


      void VariableSetter::set_value_uint1(byte value)
      {
         if(state == state_standby)
         {
            value_variant.v_uint1 = value;
            value_type = value_type_uint1;
         }
         else
            throw exc_invalid_state();
      } // set_value_uint1

      
      void VariableSetter::set_value_int1(char value)
      {
         if(state == state_standby)
         {
            value_variant.v_int1 = value;
            value_type = value_type_int1;
         }
         else
            throw exc_invalid_state();
      } // set_value_int1
      

      void VariableSetter::set_value_string(char const *value_string_)
      {
         if(state == state_standby)
         {
            value_type = value_type_string;
            value_string = value_string_;
         }
         else
            throw exc_invalid_state();
      } // set_value_string


      void VariableSetter::set_locale(std::locale const &locale_)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         locale = locale_;
      } // set_locale


      void VariableSetter::set_value_string(wchar_t const *value_string_)
      {
         StrUni temp(value_string_);
         StrAsc temp_multi(temp.to_utf8());
         set_value_string(temp_multi.c_str());
      } // set_value_string

      
      void VariableSetter::start(
         VariableSetterClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(VariableSetterClient::is_valid_instance(client_))
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


      void VariableSetter::start(
         VariableSetterClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(VariableSetterClient::is_valid_instance(client_))
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

      
      void VariableSetter::finish()
      {
         if(broker_session && router.get_rep())
            router->closeSession(broker_session);
         broker_session = 0;
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void VariableSetter::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace VariableSetterStrings;
         switch(outcome)
         {
         case client_type::outcome_succeeded:
            out << my_strings[strid_outcome_succeeded];
            break;

         case client_type::outcome_connection_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_column_read_only:
            out << my_strings[strid_outcome_column_read_only];
            break;
            
         case client_type::outcome_invalid_table_name:
            out << my_strings[strid_outcome_invalid_table_name];
            break;
            
         case client_type::outcome_invalid_column_name:
            out << my_strings[strid_outcome_invalid_column_name];
            break;
            
         case client_type::outcome_invalid_subscript:
            out << my_strings[strid_outcome_invalid_subscript];
            break;
            
         case client_type::outcome_invalid_data_type:
            out << my_strings[strid_outcome_invalid_data_type];
            break;
            
         case client_type::outcome_communication_failed:
            out << my_strings[strid_outcome_communication_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out << my_strings[strid_outcome_communication_disabled];
            break;
            
         case client_type::outcome_logger_security_blocked:
            out << my_strings[strid_outcome_logger_security_blocked];
            break;
            
         case client_type::outcome_unmatched_logger_table_definition:
            out << my_strings[strid_outcome_invalid_table_defs];
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_outcome
      
      
      void VariableSetter::on_devicebase_ready()
      {
         // we need to start by creating an attachment to the data broker so we can get table
         // definitions
         Csi::Messaging::Message attach_command(net_session,LgrNet_OpenActiveDataBrokerSesCmd);

         attach_command.addUInt4(++last_tran_no);
         attach_command.addWStr(get_device_name());
         attach_command.addUInt4(broker_session = router->openSession(this));
         router->sendMessage(&attach_command);
         state = state_local;
      } // on_devicebase_ready

      
      void VariableSetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         VariableSetterClient::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = VariableSetterClient::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = VariableSetterClient::outcome_connection_failed;
            break;

         case devicebase_failure_invalid_device_name:
            outcome = VariableSetterClient::outcome_invalid_device_name;
            break;
         }
         on_complete(outcome);
      } // on_devicebase_failure

      
      void VariableSetter::on_devicebase_session_failure()
      { on_complete(VariableSetterClient::outcome_connection_failed); }

      
      void VariableSetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_local)
         {
            switch(msg->getMsgType())
            {
            case LgrNet_OpenActiveDataBrokerSesAck:
               on_open_broker_ses_ack(msg);
               break;
               
            case Cora::Broker::Messages::table_def_get_ack:
            case Cora::Broker::Messages::extended_table_def_get_ack:
               on_table_def_get_ack(msg);
               break;
               
            case Cora::Device::Messages::variable_set_ack:
               on_set_variable_ack(msg);
               break;

            default:
               DeviceBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void VariableSetter::onNetSesBroken(
         Csi::Messaging::Router *rtr,
         uint4 session_no,
         uint4 reason,
         char const *why)
      {
         if(state == state_local)
         {
            if(session_no == broker_session)
            {
               broker_session = 0;
               if(reason == 1)
                  on_complete(VariableSetterClient::outcome_invalid_device_name);
               else
                  on_complete(VariableSetterClient::outcome_connection_failed);
            }
         }
         else
            DeviceBase::onNetSesBroken(rtr,session_no,reason,why);
      } // onNetSesBroken


      void VariableSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace VariableSetterHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            finish();
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(VariableSetterClient::is_valid_instance(event->client))
               event->client->on_complete(this,event->outcome);
         }
      } // receive


      void VariableSetter::on_complete(VariableSetterClient::outcome_type outcome)
      {
         using namespace VariableSetterHelpers;
         event_complete::create_and_post(this,client,outcome);
      } // on_complete


      void VariableSetter::on_open_broker_ses_ack(Csi::Messaging::Message *msg)
      {
         uint4 tran_no;
         uint4 resp_code;
         msg->readUInt4(tran_no);
         msg->readUInt4(resp_code);
         if(resp_code == 1)
         {
            // choose the command used to request the table definition
            uint4 message_type = Cora::Broker::Messages::table_def_get_cmd;
            if(interface_version >= Csi::VersionNumber("1.3.1"))
               message_type = Cora::Broker::Messages::extended_table_def_get_cmd;
            
            // we can now request the table definition
            Csi::Messaging::Message command(broker_session,message_type);
            command.addUInt4(++last_tran_no);
            command.addWStr(table_name);
            router->sendMessage(&command);
         }
         else if(resp_code == 2)
            on_complete(VariableSetterClient::outcome_invalid_device_name);
         else
            on_complete(VariableSetterClient::outcome_connection_failed);
      } // on_open_broker_ses_ack

      
      void VariableSetter::on_table_def_get_ack(Csi::Messaging::Message *msg)
      {
         uint4 tran_no;
         uint4 resp_code;
         msg->readUInt4(tran_no);
         msg->readUInt4(resp_code);
         if(resp_code == 1)
         {
            Cora::Broker::TableDesc desc(table_name);
            Cora::Broker::TableDesc::column_desc_handle column_desc;
            
            desc.read(msg);
            if(desc.find_column_desc(column_desc,column_name))
            {
               // we will check up front whether the column can be written
               if(column_desc->get_modifying_command() == Messages::variable_set_cmd)
               {
                  try
                  {
                     // prepare the command message
                     Csi::Messaging::Message command(
                        device_session,
                        Cora::Device::Messages::variable_set_cmd);
                     
                     command.addUInt4(++last_tran_no);
                     command.addWStr(table_name);
                     command.addWStr(column_name);
                     command.addUInt4(column_desc->get_data_type());
                     
                     // we are going to allow some fudging on the array index parameter because of
                     // the way that array indexs are reported by the server data advise and data
                     // query transactions. If the column is a scalar and the array index is empty,
                     // we will send the appropriate value from the column description, otherwise,
                     // we will send what is specified and let the server work it out.
                     if(column_desc->is_scalar() && index.empty())
                     {
                        if(msg->getMsgType() == Cora::Broker::Messages::table_def_get_ack)
                        {
                           command.addUInt4(1);
                           command.addUInt4(1);
                        }
                        else
                           command.addUInt4(0);
                     }
                     else
                     {
                        command.addUInt4((uint4)index.size());
                        for(index_type::iterator ii = index.begin(); ii != index.end(); ++ii)
                           command.addInt4(*ii);
                     }
                     write_value(column_desc->get_data_type(),command);
                     router->sendMessage(&command);
                  }
                  catch(std::invalid_argument &)
                  { on_complete(VariableSetterClient::outcome_invalid_data_type); }
               }
               else
                  on_complete(VariableSetterClient::outcome_column_read_only);
            }
            else
               on_complete(VariableSetterClient::outcome_invalid_column_name);
         }
         else
            on_complete(VariableSetterClient::outcome_invalid_table_name);
      } // on_table_def_get_ack


      void VariableSetter::on_set_variable_ack(Csi::Messaging::Message *msg)
      {
         uint4 tran_no;
         uint4 resp_code;
         VariableSetterClient::outcome_type outcome;
         
         msg->readUInt4(tran_no);
         msg->readUInt4(resp_code);
         switch(resp_code)
         {
         case 1:
            outcome = VariableSetterClient::outcome_succeeded;
            break;
            
         case 2:
            outcome = VariableSetterClient::outcome_column_read_only;
            break;
            
         case 3:
            outcome = VariableSetterClient::outcome_invalid_table_name;
            break;
            
         case 4:
            outcome = VariableSetterClient::outcome_invalid_column_name;
            break;
            
         case 5:
         case 10:
            outcome = VariableSetterClient::outcome_invalid_data_type;
            break;
            
         case 6:
            outcome = VariableSetterClient::outcome_communication_failed;
            break;
            
         case 7:
            outcome = VariableSetterClient::outcome_communication_disabled;
            break;
            
         case 8:
            outcome = VariableSetterClient::outcome_logger_security_blocked;
            break;
            
         case 11:
            outcome = VariableSetterClient::outcome_invalid_subscript;
            break;
            
         case 12:
            outcome = VariableSetterClient::outcome_unmatched_logger_table_definition;
            break;

         default:
            outcome = VariableSetterClient::outcome_unknown;
            break;
         }
         on_complete(outcome);
      } // on_set_variable_ack

   
      void VariableSetter::write_value(uint4 data_type, Csi::Messaging::Message &out)
      {
         // if the new value is specified as a string, it needs to be converted. This is our first
         // opportunity to do so.
         if(value_type == value_type_string)
         {
            Csi::IBuffStream input(value_string.c_str(), value_string.length());
            std::numpunct<char> const &punct_facet(std::use_facet<std::numpunct<char> >(locale));
            StrAsc const true_name(punct_facet.truename().c_str());
            StrAsc const false_name(punct_facet.falsename().c_str());
            
            input.imbue(locale);
            switch(data_type)
            {
            case CsiBool:
            case CsiBool2:
            case CsiBool4:
            case CsiBool8: 
               value_type = value_type_bool;
               if(value_string == true_name)
                  value_variant.v_bool = true;
               else if(value_string == false_name)
                  value_variant.v_bool = false;
               else
               {
                  // our customary value for true, -1, may not be accepted by the locale filter.  If
                  // this is the case, we will make a second attempt by converting the value into a
                  // floating point and checking for zero/non-zero. 
                  double temp = csiStringToFloat(value_string.c_str(), input.getloc(), true);
                  if(temp == 0.0)
                     value_variant.v_bool = false;
                  else
                     value_variant.v_bool = true;
               }
               break;
               
            case CsiFp4:
            case CsiIeee4:
            case CsiIeee4Lsf: 
               value_type = value_type_float;
               value_variant.v_float = static_cast<float>(
                  csiStringToFloat(value_string.c_str(), locale, true));
               break;

            case CsiIeee8:
            case CsiIeee8Lsf:
               value_type = value_type_double;
               value_variant.v_double = csiStringToFloat(
                  value_string.c_str(), locale, true);
               break;

            case CsiUInt4:
            case CsiUInt4Lsf:
               value_type = value_type_uint4;
               input >> value_variant.v_uint4;
               if(!input)
                  throw std::invalid_argument("UInt4 conversion failure");
               break;

            case CsiInt4:
            case CsiInt4Lsf:
               value_type = value_type_int4;
               input >> value_variant.v_int4;
               if(!input)
                  throw std::invalid_argument("Integer conversion failure");
               break;

            case CsiUInt2:
            case CsiUInt2Lsf:
               value_type = value_type_uint2;
               input >> value_variant.v_uint2;
               if(!input)
                  throw std::invalid_argument("UInt2 conversion failure");
               break;
               
            case CsiInt2:
            case CsiInt2Lsf:
               value_type = value_type_int2;
               input >> value_variant.v_int2;
               if(!input)
                  throw std::invalid_argument("Int2 conversion failure");
               break;
               
            case CsiUInt1:
               value_type = value_type_uint1;
               input >> value_variant.v_uint2;
               if(!input || value_variant.v_uint2 > 255)
                  throw std::invalid_argument("UInt1 conversion failure");
               value_variant.v_uint1 = static_cast<byte>(value_variant.v_uint2);
               break;

            case CsiInt1:
               value_type = value_type_int1;
               input >> value_variant.v_int2;
               if(!input || value_variant.v_int2 < -128 || value_variant.v_int2 > 127)
                  throw std::invalid_argument("Int1 conversion failure");
               value_variant.v_int1 = static_cast<char>(value_variant.v_int2);
               break;

            case CsiAscii:
               value_type = value_type_string;
               break;
               
            default:
               throw std::invalid_argument("unsupported type");
            }
         }

         // add the new variable value and send the message
         switch(value_type)
         {
         case value_type_bool:
            switch(data_type)
            {
            case CsiBool:
            case CsiBool2:
            case CsiBool4:
            case CsiBool8:
               out.addBool(value_variant.v_bool);
               break;

            case CsiInt1:
            case CsiUInt1:
               out.addByte(value_variant.v_bool ? 1 : 0);
               break;

            case CsiInt2:
            case CsiUInt2:
            case CsiInt2Lsf:
            case CsiUInt2Lsf:
               out.addUInt2(value_variant.v_bool ? 1 : 0);
               break;

            case CsiInt4:
            case CsiUInt4:
            case CsiInt4Lsf:
            case CsiUInt4Lsf:
               out.addUInt4(value_variant.v_bool ? 1 : 0);
               break;

            case CsiFp4:
            case CsiIeee4:
            case CsiIeee4Lsf:
               out.addFloat(value_variant.v_bool ? -1.0f : 0.0f);
               break;

            default:
               throw std::invalid_argument("Invalid type conversion");
               break;
            }
            break;
            
         case value_type_float:
            switch(data_type)
            {
            case CsiIeee4:
            case CsiIeee4Lsf:
            case CsiFp4:
               out.addFloat(value_variant.v_float);
               break;

            case CsiIeee8:
            case CsiIeee8Lsf:
               out.addIeee8(value_variant.v_double);
               break;

            case CsiUInt4:
            case CsiInt4:
            case CsiUInt4Lsf:
            case CsiInt4Lsf:
               out.addUInt4(static_cast<uint4>(value_variant.v_float));
               break;

            case CsiInt2:
            case CsiUInt2:
            case CsiInt2Lsf:
            case CsiUInt2Lsf:
               out.addUInt2(static_cast<uint2>(value_variant.v_float));
               break;

            case CsiInt1:
            case CsiUInt1:
               out.addByte(static_cast<byte>(value_variant.v_float));
               break;

            case CsiBool:
            case CsiBool2:
            case CsiBool4:
            case CsiBool8:
               out.addBool(value_variant.v_float != 0.0);
               break;

            default:
               throw std::invalid_argument("Unsupported type conversion");
               break;
            }
            break;

         case value_type_double:
            switch(data_type)
            {
            case CsiIeee4:
            case CsiIeee4Lsf:
            case CsiFp4:
               out.addFloat(static_cast<float>(value_variant.v_double));
               break;

            case CsiIeee8:
            case CsiIeee8Lsf:
               out.addDouble(value_variant.v_double);
               break;

            case CsiUInt4:
            case CsiUInt4Lsf:
            case CsiInt4:
            case CsiInt4Lsf:
               out.addInt4(static_cast<int4>(value_variant.v_double));
               break;

            case CsiUInt2:
            case CsiUInt2Lsf:
            case CsiInt2:
            case CsiInt2Lsf:
               out.addInt2(static_cast<int2>(value_variant.v_double));
               break;

            case CsiUInt1:
            case CsiInt1:
               out.addByte(static_cast<uint1>(value_variant.v_double));
               break;
            }
            break;
            
         case value_type_uint4:
         case value_type_int4:
            switch(data_type)
            {
            case CsiBool:
            case CsiBool2:
            case CsiBool4:
            case CsiBool8:
               out.addBool(value_variant.v_uint4 ? true : false);
               break;

            case CsiInt1:
            case CsiUInt1:
               out.addByte(static_cast<uint1>(value_variant.v_int4));
               break;

            case CsiInt2:
            case CsiUInt2:
            case CsiInt2Lsf:
            case CsiUInt2Lsf:
               out.addUInt2(static_cast<uint2>(value_variant.v_int2));
               break;
               
            case CsiUInt4:
            case CsiInt4:
            case CsiUInt4Lsf:
            case CsiInt4Lsf:
               out.addUInt4(value_variant.v_uint4);
               break;

            case CsiFp4:
            case CsiIeee4:
            case CsiIeee4Lsf:
               out.addFloat(static_cast<float>(value_variant.v_int4));
               break;

            case CsiIeee8:
            case CsiIeee8Lsf:
               out.addIeee8(static_cast<double>(value_variant.v_int4));
               break;

            default:
               throw std::invalid_argument("conversion not supported");
               break;
            }
            break;
            
         case value_type_uint2:
         case value_type_int2:
            switch(data_type)
            {
            case CsiBool:
            case CsiBool2:
            case CsiBool4:
            case CsiBool8:
               out.addBool(value_variant.v_uint2 ? true : false);
               break;

            case CsiInt1:
            case CsiUInt1:
               out.addByte(static_cast<uint1>(value_variant.v_uint2));
               break;

            case CsiInt2:
            case CsiUInt2:
            case CsiInt2Lsf:
            case CsiUInt2Lsf:
               out.addUInt2(value_variant.v_uint2);
               break;

            case CsiInt4:
            case CsiInt4Lsf:
            case CsiUInt4:
            case CsiUInt4Lsf:
               out.addUInt4(value_variant.v_uint2);
               break;

            case CsiFp4:
            case CsiIeee4:
            case CsiIeee4Lsf:
               out.addFloat(value_variant.v_int2);
               break;

            case CsiIeee8:
            case CsiIeee8Lsf:
               out.addIeee8(value_variant.v_int2);
               break;
               
            default:
               throw std::invalid_argument("conversion not supported");
               break;
            } 
            break;
            
         case value_type_uint1:
         case value_type_int1:
            switch(data_type)
            {
            case CsiBool:
            case CsiBool2:
            case CsiBool4:
            case CsiBool8:
               out.addBool(value_variant.v_uint1 ? true : false);
               break;

            case CsiInt1:
            case CsiUInt1:
               out.addByte(value_variant.v_uint1);
               break;

            case CsiInt2:
            case CsiUInt2:
            case CsiInt2Lsf:
            case CsiUInt2Lsf:
               out.addInt2(value_variant.v_int1);
               break;

            case CsiInt4:
            case CsiUInt4:
            case CsiInt4Lsf:
            case CsiUInt4Lsf:
               out.addInt4(value_variant.v_int1);
               break;

            case CsiFp4:
            case CsiIeee4:
            case CsiIeee4Lsf:
               out.addFloat(value_variant.v_int1);
               break;

            case CsiIeee8:
            case CsiIeee8Lsf:
               out.addIeee8(value_variant.v_int1);
               break;

            default:
               throw std::invalid_argument("conversion not supported");
               break;
            }
            break;

         case value_type_string:
            if(data_type == CsiAscii)
               out.addStr(value_string);
            else
               throw std::invalid_argument("conversion not supported");
            break;

         default:
            throw std::invalid_argument("type not specified");
         }
      } // write_value
   };
};
