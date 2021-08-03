/* Cora.DataSources.VirtualSource.cpp

   Copyright (C) 2011, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 September 2011
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.VirtualSource.h"
#include "Cora.DataSources.SourceTokenFactory.h"
#include "Cora.DataSources.Manager.h"
#include "Cora.DataSources.TableFieldUri.h"
#include "Cora.Device.Defs.h"
#include "Csi.Expression.ExpressionHandler.h"
#include "Csi.MaxMin.h"
#include "Csi.FileSystemObject.h"
#include "Csi.fstream.h"


namespace Cora
{
   namespace DataSources
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class my_field_symbol
         ////////////////////////////////////////////////////////////
         class my_field_symbol: public SymbolBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef VirtualSource::field_handle field_handle;
            my_field_symbol(
               VirtualSource *source, field_handle &field_, SymbolBase *parent):
               SymbolBase(source, field_->get_name(), parent),
               field(field_)
            { }

            ////////////////////////////////////////////////////////////
            // get_symbol_type
            ////////////////////////////////////////////////////////////
            virtual symbol_type_code get_symbol_type() const
            { return type_scalar; }

            ////////////////////////////////////////////////////////////
            // has_data_type
            ////////////////////////////////////////////////////////////
            virtual bool has_data_type() const
            { return true; }

            ////////////////////////////////////////////////////////////
            // get_data_type
            ////////////////////////////////////////////////////////////
            virtual CsiDbTypeCode get_data_type() const
            { return field->get_data_type(); }

            ////////////////////////////////////////////////////////////
            // is_read_only
            ////////////////////////////////////////////////////////////
            virtual bool is_read_only() const
            { return false; }

            ////////////////////////////////////////////////////////////
            // can_expand
            ////////////////////////////////////////////////////////////
            virtual bool can_expand() const
            { return false; }

         private:
            ////////////////////////////////////////////////////////////
            // field
            ////////////////////////////////////////////////////////////
            field_handle field;
         };

         
         ////////////////////////////////////////////////////////////
         // class my_table_symbol
         ////////////////////////////////////////////////////////////
         class my_table_symbol: public SymbolBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            my_table_symbol(VirtualSource *source, SymbolBase *parent):
               SymbolBase(source, "values", parent)
            {
               for(VirtualSource::iterator fi = source->begin(); fi != source->end(); ++fi)
               {
                  VirtualSource::value_type &field(*fi);
                  push_back(new my_field_symbol(source, field, this));
               }
            }

            ////////////////////////////////////////////////////////////
            // get_symbol_type
            ////////////////////////////////////////////////////////////
            virtual symbol_type_code get_symbol_type() const
            { return type_table; }

            ////////////////////////////////////////////////////////////
            // can_expand
            ////////////////////////////////////////////////////////////
            virtual bool can_expand() const
            { return true; }
         };

         
         ////////////////////////////////////////////////////////////
         // class my_source_symbol
         ////////////////////////////////////////////////////////////
         class my_source_symbol: public SymbolBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            my_source_symbol(VirtualSource *source):
               SymbolBase(source, source->get_name())
            { push_back(new my_table_symbol(source, this)); }

            ////////////////////////////////////////////////////////////
            // get_symbol_type
            ////////////////////////////////////////////////////////////
            virtual symbol_type_code get_symbol_type() const
            { return type_virtual_source; }

            ////////////////////////////////////////////////////////////
            // can_expand
            ////////////////////////////////////////////////////////////
            virtual bool can_expand() const
            { return true; }

            ////////////////////////////////////////////////////////////
            // is_connected
            ////////////////////////////////////////////////////////////
            virtual bool is_connected() const
            { return source->is_connected(); }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class VirtualSource definitions
      ////////////////////////////////////////////////////////////
      StrUni const VirtualSource::refresh_interval_name(L"refresh-interval");
      StrUni const VirtualSource::refresh_base_name(L"refresh-base");
      StrUni const VirtualSource::fields_name(L"fields");
      StrUni const VirtualSource::field_name(L"field");
      StrUni const VirtualSource::field_name_name(L"name");
      StrUni const VirtualSource::field_value_name(L"value");

      
      VirtualSource::VirtualSource(
         StrUni const &name,
         StrAsc const &constants_dir,
         StrAsc const &constants_prefix):
         SourceBase(name),
         refresh_id(0),
         refresh_interval(5 * Csi::LgrDate::msecPerMin),
         fields_changed(false)
      {
         Csi::OStrAscStream temp;
         temp << constants_dir;
         if(constants_dir.last() != Csi::FileSystemObject::dir_separator())
            temp << Csi::FileSystemObject::dir_separator();
         temp << constants_prefix << name << ".xml";
         fields_file_name = temp.str();
         token_factory.bind(new Csi::Expression::TokenFactory);
      } // constructor


      VirtualSource::~VirtualSource()
      {
         if(source_symbol != 0)
         {
            source_symbol->set_source(0);
            source_symbol.clear();
         }
         token_factory.clear();
      } // destructor


      void VirtualSource::connect()
      {
         // we need to initialise the record object by first creating a description
         Broker::Record::desc_handle desc(new Broker::RecordDesc(get_name().c_str(), L"values"));
         Broker::ValueFactory value_factory;
         manager->report_source_connecting(this);
         for(iterator fi = fields.begin(); fi != fields.end(); ++fi)
            (*fi)->describe(*desc);
         record.bind(new Broker::Record(desc, value_factory));
         for(iterator fi = fields.begin(); fi != fields.end(); ++fi)
            (*fi)->update(*record);
         
         // start the refresh schedule
         last_fields_date = 0;
         fields_changed = false;
         scheduler.bind(new Scheduler(get_manager()->get_timer()));
         refresh_id = scheduler->start(this, refresh_base, refresh_interval);
         manager->report_source_connect(this);
      } // connect


      void VirtualSource::disconnect()
      {
         bool was_connected(is_connected());
         scheduler.clear();
         requests.clear();
         refresh_id = 0;
         if(fields_changed)
            save_fields();
         if(was_connected)
            manager->report_source_disconnect(this, ManagerClient::disconnect_by_application); 
      } // disconnect


      bool VirtualSource::is_connected() const
      { return scheduler != 0; }


      void VirtualSource::get_properties(Csi::Xml::Element &props)
      {
         Csi::Xml::Element::value_type fields_xml(props.add_element(fields_name));
         props.set_attr_uint4(refresh_interval, refresh_interval_name);
         props.set_attr_lgrdate(refresh_base, refresh_base_name);
         for(fields_type::iterator fi = fields.begin(); fi != fields.end(); ++fi)
         {
            Csi::Xml::Element::value_type field_xml(fields_xml->add_element(field_name));
            field_handle &field(*fi);
            field_xml->set_attr_wstr(field->get_name(), field_name_name);
            field_xml->set_attr_wstr(field->get_expression_str(), field_value_name);
         }
      } // get_properties


      void VirtualSource::set_properties(Csi::Xml::Element &props)
      {
         using namespace Csi::Xml;
         Element::value_type fields_xml(props.find_elem(fields_name));
         StrAsc name;
         StrAsc value;
         field_handle field;
         bool was_connected(is_connected());

         disconnect();
         if(props.has_attribute(refresh_interval_name))
         {
            refresh_interval = props.get_attr_uint4(refresh_interval_name);
            if(refresh_interval < 100)
               refresh_interval = 100;
         }
         if(props.has_attribute(refresh_base_name))
            refresh_base = props.get_attr_lgrdate(refresh_base_name);
         fields.clear();
         for(Element::iterator fi = fields_xml->begin(); fi != fields_xml->end(); ++fi)
         {
            Element::value_type &field_xml(*fi);
            name = field_xml->get_attr_str(field_name_name);
            value = field_xml->get_attr_str(field_value_name);
            field.bind(new field_type(this, name, value));
            fields.push_back(field);
         }
         if(was_connected)
            connect();
      } // set_properties


      void VirtualSource::add_request(
         request_handle &request, bool more_to_follow)
      {
         if(SinkBase::is_valid_instance(request->get_sink()))
         {
            // we will add the request and set its appropriate state
            Csi::PolySharedPtr<WartBase, TableFieldUri> wart(new TableFieldUri(request->get_uri()));
            if(request->get_start_option() != Request::start_date_query)
               requests.push_back(request);
            request->set_state(this, Request::state_started);
            request->set_wart(wart.get_handle());
            
            // report that the request is ready
            request->set_value_indices(*record, wart->get_column_name());
            request->set_state(this, Request::state_started);
            request->get_sink()->on_sink_ready(get_manager(), request, record);
            
            // report the records for this request.  If the request is for a date range, we will report
            // only one record with its time stamp set to the beginning of that range.  If we don't do
            // this, RTMC's report toolbar will filter out our records.
            SinkBase::records_type records;
            SinkBase::requests_type requests;
            records.push_back(record);
            requests.push_back(request);
            if(request->get_start_option() == Request::start_date_query)
               record->set_stamp(request->get_start_time());
            request->set_expect_more_data(false);
            SinkBase::report_sink_records(get_manager(), requests, records);
         }
      } // add_request


      void VirtualSource::remove_request(request_handle &request)
      {
         requests_type::iterator ri(std::find(requests.begin(), requests.end(), request));
         if(ri != requests.end())
            requests.erase(ri);
      } // remove_request


      void VirtualSource::remove_all_requests()
      {
         requests.clear();
      } // remove_all_requests


      void VirtualSource::save_fields()
      {
         if(!fields_changed)
            return;
         try
         {
            Csi::check_file_name_path(fields_file_name.c_str());
            Csi::ofstream output(fields_file_name.c_str(), std::ios::binary);
            if(output)
            {
               // format and close the output
               using namespace Csi::Xml;
               Element fields_xml(fields_name);
               for(iterator fi = fields.begin(); fi != fields.end(); ++fi)
               {
                  value_type &field(*fi);
                  Element::value_type field_xml(fields_xml.add_element(field_name));
                  field_xml->set_attr_wstr(field->get_name(), field_name_name);
                  field_xml->set_attr_wstr(field->get_value()->get_val_str(), field_value_name);
               }
               fields_xml.output(output, true);
               output.close();

               // since we have just changed the fields file, we will not need to refresh from it.
               // We will prevent that by setting out own time equal to the file's last changed time
               Csi::FileSystemObject fields_file_info(fields_file_name.c_str());
               last_fields_date = fields_file_info.get_last_write_date();
               fields_changed = false;
            }
         }
         catch(std::exception &)
         { }
      } // save_fields


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate field_has_name
         ////////////////////////////////////////////////////////////
         struct field_has_name
         {
            StrUni const &name;
            field_has_name(StrUni const &name_):
               name(name_)
            { }

            bool operator ()(VirtualSource::value_type const &field) const
            { return field->get_name() == name; }
         };
      };

      
      void VirtualSource::check_fields()
      {
         try
         {
            // we will first need to check to see if the date on the fields file has changed
            Csi::FileSystemObject fields_file_info(fields_file_name.c_str());
            if(fields_file_info.get_is_valid() && fields_file_info.get_last_write_date() != last_fields_date)
            {
               // we must now open and parse the file
               Csi::ifstream input(fields_file_name.c_str(), std::ios::binary);
               if(input)
               {
                  using namespace Csi::Xml;
                  Csi::Xml::Element fields_xml(fields_name);
                  fields_xml.input(input);
                  for(Element::iterator fxi = fields_xml.begin(); fxi != fields_xml.end(); ++fxi)
                  {
                     Element::value_type &field_xml(*fxi);
                     StrAsc field_name(field_xml->get_attr_str(field_name_name));
                     StrAsc field_value(field_xml->get_attr_str(field_value_name));
                     iterator fi(std::find_if(begin(), end(), field_has_name(field_name)));
                     if(fi != end())
                     {
                        value_type &field(*fi);
                        if(field->set_value(ValueSetter(field_value)) == SinkBase::set_outcome_succeeded)
                           field->update(*record);
                     }
                  }
                  last_fields_date = fields_file_info.get_last_write_date();
               }
            }
         }
         catch(std::exception &)
         { }
      } // check_fields
      

      VirtualSource::symbol_handle VirtualSource::get_source_symbol()
      {
         if(source_symbol == 0)
            source_symbol.bind(new my_source_symbol(this));
         return source_symbol;
      } // get_source_symbol


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class value_setter_type
         ////////////////////////////////////////////////////////////
         class value_setter_type: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // sink
            ////////////////////////////////////////////////////////////
            SinkBase *sink;
            
            ////////////////////////////////////////////////////////////
            // uri
            ////////////////////////////////////////////////////////////
            StrUni const uri;

            ////////////////////////////////////////////////////////////
            // value
            ////////////////////////////////////////////////////////////
            ValueSetter const value;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               VirtualSource *source,
               SinkBase *sink,
               StrUni const &uri,
               ValueSetter const &setter)
            {
               value_setter_type *ev(new value_setter_type(source, sink, uri, setter));
               ev->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            value_setter_type(
               VirtualSource *source,
               SinkBase *sink_,
               StrUni const &uri_,
               ValueSetter const &value_):
               Event(event_id, source),
               sink(sink_),
               uri(uri_),
               value(value_)
            { }
         };


         uint4 const value_setter_type::event_id(
            Csi::Event::registerType("Cora::DataSources::VirtualSource::value_setter_type"));
      };

      
      bool VirtualSource::start_set_value(
         SinkBase *sink,
         StrUni const &uri,
         ValueSetter const &value)
      {
         value_setter_type::cpost(this, sink, uri, value);
         return true;
      } // start_set_value


      void VirtualSource::breakdown_uri(symbols_type &symbols, StrUni const &uri_)
      {
         TableFieldUri uri(uri_);
         symbols.clear();
         if(uri.get_source_name().length() > 0)
         {
            symbols.push_back(symbol_type(uri.get_source_name(), SymbolBase::type_virtual_source));
            if(uri.get_table_name().length() > 0)
            {
               symbols.push_back(symbol_type(uri.get_table_name(), SymbolBase::type_table));
               if(uri.get_column_name().length() > 0)
                  symbols.push_back(symbol_type(uri.get_column_name(), SymbolBase::type_scalar));
            }
         }
      } // breakdown_uri


      Csi::Expression::TokenFactory *VirtualSource::get_token_factory()
      {
         if(token_factory == 0)
            token_factory.bind(new Csi::Expression::TokenFactory());
         return token_factory.get_rep();
      } // get_token_factory


      void VirtualSource::onScheduledEvent(uint4 id)
      {
         if(id == refresh_id)
         {
            // we may need to update the fields file
            save_fields();
            check_fields();
            send_record(
               scheduler->nextTime(refresh_id) - refresh_interval*Csi::LgrDate::nsecPerMSec);
         }
      } // onScheduledEvent
      

      void VirtualSource::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == value_setter_type::event_id)
         {
            value_setter_type *setter(static_cast<value_setter_type *>(ev.get_rep()));
            if(SinkBase::is_valid_instance(setter->sink))
            {
               try
               {
                  TableFieldUri uri(setter->uri);
                  if(uri.get_table_name() == L"values")
                  {
                     iterator fi(std::find_if(fields.begin(), fields.end(), field_has_name(uri.get_column_name())));
                     if(fi != fields.end())
                     {
                        value_type field(*fi);
                        SinkBase::set_outcome_type outcome = field->set_value(setter->value);
                        if(outcome == SinkBase::set_outcome_succeeded)
                        {
                           fields_changed = true;
                           field->update(*record);
                           send_record(Csi::LgrDate::system());
                           save_fields();
                        }
                        setter->sink->on_set_complete(get_manager(), setter->uri, outcome);
                     }
                     else
                     {
                        setter->sink->on_set_complete(
                           get_manager(), setter->uri, SinkBase::set_outcome_invalid_column_name);
                     }     
                  }
                  else
                  {
                     setter->sink->on_set_complete(
                        get_manager(), setter->uri, SinkBase::set_outcome_invalid_table_name);
                  }
               }
               catch(std::exception &)
               {
                  setter->sink->on_set_complete(
                     get_manager(), setter->uri, SinkBase::set_outcome_unknown);
               }
            }
         }
      } // receive


      void VirtualSource::send_record(Csi::LgrDate const &stamp)
      {
         SinkBase::records_type records;
         records.push_back(record);
         record->set_record_no(record->get_record_no() + 1);
         record->set_stamp(stamp);
         SinkBase::report_sink_records(get_manager(), requests, records); 
      } // send_record
      
      
      namespace VirtualSourceHelpers
      {
         ////////////////////////////////////////////////////////////
         // class field_type definitions
         ////////////////////////////////////////////////////////////
         field_type::field_type(
            VirtualSource *source_,
            StrUni const &name_,
            StrUni const &expression_str_):
            name(name_),
            expression_str(expression_str_),
            source(source_)
         {
            value.bind(new Csi::Expression::Constant());
            try
            {
               Csi::Expression::ExpressionHandler expression(source->get_token_factory());
               expression.tokenize(expression_str.c_str());
               value = expression.eval();
               if(value->is_variable())
                  value->set_val(std::numeric_limits<double>::quiet_NaN(), Csi::LgrDate::system());
            }
            catch(std::exception &)
            {
               value->set_val(std::numeric_limits<double>::quiet_NaN(), Csi::LgrDate::system());
            }
         } // constructor


         field_type::~field_type()
         {
            value.clear();
         } // destructor


         void field_type::update(Broker::Record &record)
         {
            // look up the record value if needed
            if(record_value == 0)
            {
               Broker::Record::iterator ri = record.find_formatted(name);
               if(ri == record.end())
                  throw std::invalid_argument("invalid field name");
               record_value = *ri;
            }

            // we will need to update the value's storage pointer
            if(record_value->get_type() == CsiIeee8 || record_value->get_type() == CsiIeee8Lsf)
            {
               double *val = static_cast<double *>(record_value->get_storage());
               *val = value->get_val();
            }
            else if(record_value->get_type() == CsiInt8 || record_value->get_type() == CsiInt8Lsf)
            {
               int8 *val = static_cast<int8 *>(record_value->get_storage());
               *val = value->get_val_int();
            }
            else if(record_value->get_type() == CsiLgrDate || record_value->get_type() == CsiLgrDateLsf)
            {
               int8 *val = static_cast<int8 *>(record_value->get_storage());
               *val = value->get_val_date();
            }
            else if(record_value->get_type() == CsiAscii)
            {
               char *val = static_cast<char *>(record_value->get_storage());
               StrAsc str_val(value->get_val_str());
               record_value->set_to_null();
               memset(val, 0, record_value->get_pointer_len());
               memcpy(val, str_val.c_str(), Csi::csimin<uint4>((uint4)record_value->get_pointer_len(), (uint4)str_val.length()));
            }
            else
               throw std::invalid_argument("unsupported value type");
         } // update


         void field_type::describe(Broker::RecordDesc &record_desc)
         {
            // we need to determine the type and number of array elements that should be added
            CsiDbTypeCode value_type(get_data_type());
            uint4 repeat_count = 1;
            if(value->get_value().type == Csi::Expression::value_string)
            {
               repeat_count = (uint4)value->get_value().vstring.length() + 1;
               if(repeat_count < 51)
                  repeat_count = 51;
            }

            // we can now added the requisite number of values
            for(uint4 i = 0; i < repeat_count; ++i)
            {
               Broker::RecordDesc::value_type value_desc(new Broker::ValueDesc);
               value_desc->name = name.c_str();
               value_desc->data_type = value_type;
               value_desc->modifying_cmd = Cora::Device::Messages::variable_set_cmd;
               if(value_type == CsiAscii)
                  value_desc->array_address.push_back(i + 1);
               record_desc.values.push_back(value_desc);
            }
         } // describe


         CsiDbTypeCode field_type::get_data_type() const
         {
            CsiDbTypeCode rtn;
            switch(value->get_value().type)
            {
            case Csi::Expression::value_double:
               if(Csi::is_big_endian())
                  rtn = CsiIeee8;
               else
                  rtn = CsiIeee8Lsf;
               break;
               
            case Csi::Expression::value_int:
               if(Csi::is_big_endian())
                  rtn = CsiInt8;
               else
                  rtn = CsiInt8Lsf;
               break;
               
            case Csi::Expression::value_string:
               rtn = CsiAscii;
               break;
               
            case Csi::Expression::value_date:
               if(Csi::is_big_endian())
                  rtn = CsiLgrDate;
               else
                  rtn = CsiLgrDateLsf;
               break;
               
            default:
               throw std::invalid_argument("invalid value type");
               break;
            }
            return rtn;
         } // get_data_type


         SinkBase::set_outcome_type field_type::set_value(ValueSetter const &setter)
         {
            SinkBase::set_outcome_type rtn(SinkBase::set_outcome_succeeded);
            switch(setter.value_type)
            {
            case ValueSetter::value_type_bool:
               value->set_val(static_cast<int8>(setter.value_variant.v_bool ? -1 : 0), Csi::LgrDate::system());
               break;
               
            case ValueSetter::value_type_float:
               value->set_val(setter.value_variant.v_float, Csi::LgrDate::system());
               break;
               
            case ValueSetter::value_type_uint4:
               value->set_val(static_cast<int8>(setter.value_variant.v_uint4), Csi::LgrDate::system());
               break;
               
            case ValueSetter::value_type_int4:
               value->set_val(static_cast<int8>(setter.value_variant.v_int4), Csi::LgrDate::system());
               break;
               
            case ValueSetter::value_type_uint2:
               value->set_val(static_cast<int8>(setter.value_variant.v_uint2), Csi::LgrDate::system());
               break;
               
            case ValueSetter::value_type_int2:
               value->set_val(static_cast<int8>(setter.value_variant.v_int2), Csi::LgrDate::system());
               break;
               
            case ValueSetter::value_type_uint1:
               value->set_val(static_cast<int8>(setter.value_variant.v_uint1), Csi::LgrDate::system());
               break;
               
            case ValueSetter::value_type_int1:
               value->set_val(static_cast<int8>(setter.value_variant.v_int1), Csi::LgrDate::system());
               break;

            case ValueSetter::value_type_string:
               value->set_val(setter.value_string, Csi::LgrDate::system());
               break;

            default:
               rtn = SinkBase::set_outcome_invalid_data_type;
               break;
            }
            return rtn;
         } // set_value
      };
   };
};


