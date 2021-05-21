/* main.cpp

   Copyright (C) 2008, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 06 August 2008
   Last Change: Wednesday 06 February 2019
   Last Commit: $Date: 2019-02-06 12:55:19 -0600 (Wed, 06 Feb 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.Manager.h"
#include "Cora.DataSources.LgrNetSource.h"
#include "Cora.DataSources.DataFileSource.h"
#include "Cora.DataSources.CsiDbSource.h"
#include "Cora.DataSources.HttpSource.h"
#include "Cora.DataSources.VirtualSource.h"
#include "Cora.DataSources.Bmp5Source.h"
#include "Cora.DataSources.SymbolBrowser.h"
#include "Cora.DataSources.SourceTokenFactory.h"
#include "Csi.Alarms.Manager.h"
#include "Csi.CommandLine.h"
#include "Csi.Expression.ExpressionHandler.h"
#include "Csi.Expression.Editor.h"
#include "Csi.OsException.h"
#include "Csi.Win32Dispatch.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include <iostream>
#include <fstream>
using namespace std;
using namespace Cora::DataSources;


////////////////////////////////////////////////////////////
// class MyExpression
////////////////////////////////////////////////////////////
class MyExpression: public Cora::DataSources::SinkBase
{
private:
   ////////////////////////////////////////////////////////////
   // manager
   ////////////////////////////////////////////////////////////
   typedef Csi::SharedPtr<Cora::DataSources::Manager> manager_handle;
   manager_handle manager;

   ////////////////////////////////////////////////////////////
   // expression
   ////////////////////////////////////////////////////////////
   Csi::SharedPtr<Csi::Expression::ExpressionHandler> expression;
   StrUni expression_str;
   
   ////////////////////////////////////////////////////////////
   // requests
   ////////////////////////////////////////////////////////////
   typedef Csi::SharedPtr<Cora::DataSources::Request> request_handle;
   typedef std::list<request_handle> requests_type;
   requests_type requests;

public:
   ////////////////////////////////////////////////////////////
   // constructor
   ////////////////////////////////////////////////////////////
   MyExpression(manager_handle &manager_, StrUni const &expression_):
      manager(manager_),
      expression_str(expression_)
   {
      // we will use this as an opportunity to test the expression editor component.
      Csi::Expression::Editor editor(expression_str);
      for(Csi::Expression::Editor::iterator fi = editor.begin();
          fi != editor.end();
          ++fi)
      {
         (*fi)->write(std::cout);
         std::cout << "\n";
      }
      std::cout << editor.get_body();
      
      // now generate the expression and add its requests
      expression = Cora::DataSources::SourceTokenFactory::make_expression(
         this, expression_str, requests);
      for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
         manager->add_request(*ri);
   }

   ////////////////////////////////////////////////////////////
   // destructor
   ////////////////////////////////////////////////////////////
   virtual ~MyExpression()
   {
      expression.clear();
      requests.clear();
      manager.clear();
   }

   ////////////////////////////////////////////////////////////
   // on_sink_ready
   ////////////////////////////////////////////////////////////
   virtual void on_sink_ready(
      Manager *manager, request_handle &request, record_handle &record)
   {
   }

   ////////////////////////////////////////////////////////////
   // on_sink_failure
   ////////////////////////////////////////////////////////////
   virtual void on_sink_failure(
      Manager *manager, request_handle &request, sink_failure_type failure)
   {
   }
   

   ////////////////////////////////////////////////////////////
   // on_sink_records
   ////////////////////////////////////////////////////////////
   virtual void on_sink_records(
      Manager *manager, requests_type &requests, records_type const &records)
   {
      try
      {
         using Csi::Expression::ExpressionHandler;
         Csi::OStrAscStream temp;
         double float_val;
         
         for(records_type::const_iterator ri = records.begin(); ri != records.end(); ++ri)
         {
            for(requests_type::iterator rqi = requests.begin(); rqi != requests.end(); ++rqi)
            {
               requests_type::value_type &request = *rqi;
               records_type::value_type const &record = *ri;
               Cora::Broker::Record::const_iterator vi = record->begin() + request->get_begin_index();
               Cora::Broker::Record::value_type const &value = *vi;
               ExpressionHandler::iterator ei = expression->find(request->get_uri());

               std::cout << "data recieved for \"" << request->get_uri() << "\"\n";
               record->get_stamp().format(std::cout, "%Y-%m-%d %H:%M:%S%x\n");
               if(ei == expression->end())
                  continue;
               switch(value->get_type())
               {
               case CsiAscii:
               case CsiAsciiZ:
               case CsiNSec:
               case CsiNSecLsf:
               case CsiLgrDate:
               case CsiLgrDateLsf:
                  temp.str("");
                  value->format(temp);
                  ei->second->set_val(temp.str(), record->get_stamp());
                  break;
                  
               default:
                  value->to_float(float_val);
                  ei->second->set_val(float_val, record->get_stamp());
                  break;
               }
            }
            try
            {
               ExpressionHandler::operand_handle op = expression->eval();
               if(op != 0)
               {
                  std::cout << "Record received for: " << expression_str << "\n  ";
                  op->format(std::cout);
                  std::cout << std::endl;
               }
            }
            catch(std::exception &e)
            {
               std::cout << "Expression evaluation failed: " << e.what() << std::endl;
            }
         }
      }
      catch(std::exception &)
      {
      }
   } 
   
};


////////////////////////////////////////////////////////////
// stop_execution
////////////////////////////////////////////////////////////
bool stop_execution;


////////////////////////////////////////////////////////////
// class MySink
////////////////////////////////////////////////////////////
class MySink:
   public Cora::DataSources::SinkBase,
   public Cora::DataSources::ManagerClient,
   public Cora::DataSources::SymbolBrowserClient,
   public Csi::EventReceiver 
{
public:
   ////////////////////////////////////////////////////////////
   // delete_after_data
   ////////////////////////////////////////////////////////////
   bool delete_after_data;

   ////////////////////////////////////////////////////////////
   // range_queries
   ////////////////////////////////////////////////////////////
   typedef std::list<StrAsc> range_queries_type;
   range_queries_type range_queries;

   ////////////////////////////////////////////////////////////
   // constructor
   ////////////////////////////////////////////////////////////
   MySink():
      delete_after_data(false)
   { }
   
   ////////////////////////////////////////////////////////////
   // on_source_added
   ////////////////////////////////////////////////////////////
   virtual void on_source_added(
      Manager *manager, source_handle &source)
   {
      cout << Csi::LgrDate::system() << ": Source added name=" << source->get_name() << endl;
   }

   ////////////////////////////////////////////////////////////
   // on_source_starting
   ////////////////////////////////////////////////////////////
   virtual void on_source_connecting(
      Manager *manager, source_handle &source)
   {
      cout << Csi::LgrDate::system() << ": Source connecting name=" << source->get_name() << endl;
   }

   ////////////////////////////////////////////////////////////
   // on_source_connect
   ////////////////////////////////////////////////////////////
   virtual void on_source_connect(
      Manager *manager, source_handle &source)
   {
      cout << Csi::LgrDate::system() << ": Source connected: name=" << source->get_name() << endl;
      for(range_queries_type::iterator ri = range_queries.begin(); ri != range_queries.end(); ++ri)
         manager->get_table_range(this, *ri);
   }

   ////////////////////////////////////////////////////////////
   // on_source_disconnect
   ////////////////////////////////////////////////////////////
   virtual void on_source_disconnect(
      Manager *manager, source_handle &source, disconnect_reason_type reason)
   {
      cout << Csi::LgrDate::system() << ": Source disconnected: name=" << source->get_name() << " reason=" << reason << endl;
      stop_execution = true;
   } // on_source_stopped

   ////////////////////////////////////////////////////////////
   // on_sink_ready
   ////////////////////////////////////////////////////////////
   virtual void on_sink_ready(
      Manager *manager,
      request_handle &request,
      record_handle &record)
   {
      cout << Csi::LgrDate::system() << ": Sink Ready: "
           << " uri=" << request->get_uri()
           << " station=\"" << record->get_description()->broker_name << "\""
           << " table=\"" << record->get_description()->table_name << "\""
           << endl;
   }
   
   ////////////////////////////////////////////////////////////
   // on_sink_failure
   ////////////////////////////////////////////////////////////
   virtual void on_sink_failure(
      Manager *manager,
      request_handle &request,
      sink_failure_type failure)
   {
      cout << Csi::LgrDate::system() << ": Sink request failed: " 
           << " uri=" << request->get_uri()
           << " failure=" << failure << endl;
   }


   ////////////////////////////////////////////////////////////
   // functor print_request_value
   ////////////////////////////////////////////////////////////
   struct print_request_value
   {
      void operator ()(Cora::Broker::Record::value_type const &value)
      {
         value->format_name_ex(cout, true, "(", "", ", ", ")");
         cout << ": ";
         if(value->quote_when_formatting())
            cout << "\"";
         value->format(cout);
         if(value->quote_when_formatting())
            cout << "\"";
         cout << "\n";
      }
   };

   
   ////////////////////////////////////////////////////////////
   // functor print_request_record
   ////////////////////////////////////////////////////////////
   struct print_request_record
   {
      request_handle const &request;
      print_request_record(request_handle const &request_):
         request(request_)
      { }

      void operator ()(records_type::value_type const &record)
      {
         cout << "record: " << record->get_record_no()
              << " filemark: " << record->get_file_mark_no()
              << " time: " << record->get_stamp() << "\n";
         std::for_each(
            record->begin() + request->get_begin_index(),
            record->begin() + request->get_end_index(),
            print_request_value());
         cout << endl;
      }
   };
   
      
   ////////////////////////////////////////////////////////////
   // on_sink_records
   ////////////////////////////////////////////////////////////
   virtual void on_sink_records(
      Manager *manager,
      requests_type &requests,
      records_type const &records)
   {
      for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
      {
         requests_type::value_type &request = *ri;
         cout << Csi::LgrDate::system() << ": received " << records.size() << " records: "
              << " uri=" << request->get_uri() << endl;
         std::for_each(records.begin(), records.end(), print_request_record(request)); 
      }

      // for testing purposes, let's abort the first set of requests
      if(delete_after_data)
      {
         for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
            manager->remove_request(*ri);
      }
   }


   ////////////////////////////////////////////////////////////
   // on_request_state_change
   ////////////////////////////////////////////////////////////
   virtual void on_request_state_change(Manager *manager, Request *request)
   {
      std::cout << "Request state change: " << request->get_uri() << ", " << request->get_state() << std::endl;
   }

   ////////////////////////////////////////////////////////////
   // on_symbol_added
   ////////////////////////////////////////////////////////////
   virtual void on_symbol_added(
      SymbolBrowser *browser, symbol_handle &symbol)
   {
      using namespace Cora::DataSources;
      if(symbol->get_symbol_type() != SymbolBase::type_array)
      {
         cout << Csi::LgrDate::system() << ": symbol added: \"";
         symbol->format_uri(cout);
         cout << "\"" << endl;
      }
      if(symbol->can_expand() && symbol->empty())
         symbol->start_expansion();
      else
      {
         for(SymbolBase::iterator si = symbol->begin(); si != symbol->end(); ++si)
            on_symbol_added(browser, *si);
      }
   }

   ////////////////////////////////////////////////////////////
   // on_symbol_removed
   ////////////////////////////////////////////////////////////
   virtual void on_symbol_removed(
      SymbolBrowser *browser,
      symbol_handle &symbol,
      remove_reason_type reason)
   {
      cout << Csi::LgrDate::system() << ": symbol removed: \"";
      symbol->format_uri(cout);
      cout << "\" reason=" << reason << endl;
   }

   ////////////////////////////////////////////////////////////
   // on_source_connect_change
   ////////////////////////////////////////////////////////////
   virtual void on_source_connect_change(
      SymbolBrowser *browser,
      symbol_handle &source_symbol)
   {
      cout << Csi::LgrDate::system() << ": symbol \"" << source_symbol->get_name() << "\" ";
      if(source_symbol->is_connected())
         cout << "is connected" << endl;
      else
         cout << "is not connected" << endl;
   }

   ////////////////////////////////////////////////////////////
   // on_symbol_enabled_change
   ////////////////////////////////////////////////////////////
   virtual void on_symbol_enabled_change(
      SymbolBrowser *browser, symbol_handle &symbol)
   {
      cout << Csi::LgrDate::system() << ": Symbol \"";
      symbol->format_uri(cout);
      cout << "\" " << (symbol->is_enabled() ? "is enabled" : "is disabled") << endl; 
   }


   ////////////////////////////////////////////////////////////
   // receive
   ////////////////////////////////////////////////////////////
   virtual void receive(Csi::SharedPtr<Csi::Event> &ev)
   {
      typedef Cora::DataSources::GetTableRangeCompleteEvent range_event_type;
      if(ev->getType() == range_event_type::event_id)
      {
         range_event_type *event = static_cast<range_event_type *>(ev.get_rep());
         cout << "Table range complete: " << event->uri << ", " << event->outcome << endl;
         if(event->outcome == range_event_type::outcome_success)
         {
            cout << "  oldest: " << event->begin_date << endl
                 << "  newest: " << event->end_date << endl;
         }
      }
   }
};


////////////////////////////////////////////////////////////
// class MySupervisor
////////////////////////////////////////////////////////////
class MySupervisor: 
   public Cora::DataSources::ManagerSupervisor
{
private:
   ////////////////////////////////////////////////////////////
   // begin_date
   ////////////////////////////////////////////////////////////
   Csi::LgrDate begin_date;

   ////////////////////////////////////////////////////////////
   // end_date
   ////////////////////////////////////////////////////////////
   Csi::LgrDate end_date;

public:
   ////////////////////////////////////////////////////////////
   // constructor
   ////////////////////////////////////////////////////////////
   MySupervisor(Csi::LgrDate const &begin_date_, Csi::LgrDate const &end_date_):
      begin_date(begin_date_),
      end_date(end_date_)
   { }

   ////////////////////////////////////////////////////////////
   // on_add_request
   ////////////////////////////////////////////////////////////
   virtual void on_add_request(
      Cora::DataSources::Manager *manager, request_handle &request)
   {
      std::cout << "request for " << request->get_uri() << " adding with start option "
                << request->get_start_option() << std::endl;
      request->set_query_times(begin_date, end_date);
   }


   ////////////////////////////////////////////////////////////
   // on_sink_data
   ////////////////////////////////////////////////////////////
   virtual void on_sink_data(
      Cora::DataSources::Manager *manager, request_handle &request, records_type &records)
   { }
};



////////////////////////////////////////////////////////////
// class MyAlarmClient
////////////////////////////////////////////////////////////
class MyAlarmClient: public Csi::Alarms::AlarmClient
{
public:
   ////////////////////////////////////////////////////////////
   // on_alarm_change
   ////////////////////////////////////////////////////////////
   virtual void on_alarm_change(Csi::Alarms::Alarm *alarm)
   {
      using namespace Csi::Alarms;
      uint4 pending_actions(alarm->get_pending_actions());
      
      std::cout << "alarm changed: \"" << alarm->get_name() << "\"" << std::endl;
      if(alarm->get_last_error().length())
         std::cout << "  error: \"" << alarm->get_last_error() << "\"" << std::endl;
      if(alarm->get_last_value() != 0)
      {
         std::cout << "  value: ";
         alarm->get_last_value()->format(std::cout);
         std::cout << std::endl;
      }
      std::cout << "  state: ";
      switch(alarm->get_state())
      {
      case Alarm::state_off:
         std::cout << "off";
         break;

      case Alarm::state_on:
         std::cout << "on";
         //alarm->acknowledge();
         break;

      case Alarm::state_acked:
         std::cout << "acknowledged";
         break;
      }
      std::cout << std::endl;
      if(pending_actions)
         std::cout << "  pending actions: " << pending_actions << std::endl;
      if(alarm->get_last_action_error().length())
      {
         std::cout << "  last action failed: \""
                   << alarm->get_last_action_error() << "\"" << std::endl;
      }
   }
};


////////////////////////////////////////////////////////////
// functor describe_source_name
////////////////////////////////////////////////////////////
struct describe_source_name
{
   void operator ()(Cora::DataSources::CsiDbSource::source_name_type const &name)
   {
      std::cout << "CsiDb Source Name: \"" << name.first << "\" of type " << name.second << "\n";
   }
};


int main(int argc, char const *argv[])
{
   int rtn = 0;
   try
   {
      // check the number of arguments
      if(argc < 2)
         throw std::invalid_argument("invalid number of arguments");
      ifstream input(argv[1]);
      if(!input)
         throw Csi::OsException("unable to open the input file");

      // we need to perform some coratools initialisation
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init;

      // read the input as XML
      using namespace Csi::Xml;
      Element input_xml(L"input");
      input_xml.input(input);
      input.close();

      // we need to see if there is a supervisor section in the document
      Csi::SharedPtr<Manager> manager(new Manager);
      Element::iterator supervisor_it = input_xml.find(L"supervisor");
      if(supervisor_it != input_xml.end())
      {
         Element::value_type &supervisor_xml = *supervisor_it;
         manager->set_supervisor(
            new MySupervisor(
               supervisor_xml->get_attr_lgrdate(L"begin"),
               supervisor_xml->get_attr_lgrdate(L"end"))); 
      }
      
      // we need to set up the sources.  We will read these from the sources section of the input
      // document
      typedef Csi::SharedPtr<MyExpression> expression_handle;
      typedef std::list<expression_handle> expressions_type;
      Element::value_type sources(input_xml.find_elem(L"sources"));
      Csi::SharedPtr<SymbolBrowser> browser(new SymbolBrowser(manager));
      MySink sink;
      expressions_type expressions;
      Element::iterator ranges_it(input_xml.find(L"ranges"));
      
      if(input_xml.has_attribute(L"delete-after-data"))
         sink.delete_after_data = input_xml.get_attr_bool(L"delete-after-data");
      manager->add_client(&sink);
      browser->add_client(&sink);
      if(ranges_it != input_xml.end())
      {
         Element::value_type &ranges = *ranges_it;
         for(Element::iterator ri = ranges->begin(); ri != ranges->end(); ++ri)
         {
            Element::value_type &range(*ri);
            sink.range_queries.push_back(range->get_attr_str(L"uri"));
         }
      }
      for(Element::iterator ei = sources->begin(); ei != sources->end(); ++ei)
      {
         Element::value_type &source_xml = *ei;
         StrAsc source_name = source_xml->get_attr_str(L"name");
         StrAsc source_type = source_xml->get_attr_str(L"type");
         if(source_type == "loggernet")
         {
            Manager::value_type source(new LgrNetSource(source_name));
            Element::value_type settings_xml(source_xml->find_elem(L"settings"));
            source->set_properties(*settings_xml);
            manager->add_source(source);
         }
         else if(source_type == "file" || source_type == "data-file")
         {
            Manager::value_type source(new DataFileSource(source_name));
            Element::value_type settings_xml(source_xml->find_elem(L"settings"));
            source->set_properties(*settings_xml);
            manager->add_source(source);
         }
         else if(source_type == "database" || source_type == "csidb")
         {
            Manager::value_type source(new CsiDbSource(source_name));
            Element::value_type settings_xml(source_xml->find_elem(L"settings"));
            source->set_properties(*settings_xml);
            manager->add_source(source);
         }
         else if(source_type == "http")
         {
            Manager::value_type source(new HttpSource(source_name));
            Element::value_type settings_xml(source_xml->find_elem(L"settings"));
            source->set_properties(*settings_xml);
            manager->add_source(source);
         }
         else if(source_type == "virtual")
         {
            Manager::value_type source(new VirtualSource(source_name, "", "test"));
            Element::value_type settings_xml(source_xml->find_elem(L"settings"));
            source->set_properties(*settings_xml);
            manager->add_source(source);
         }
         else if(source_type == "bmp5")
         {
            Manager::value_type source (new Bmp5Source(source_name));
            Element::value_type settings_xml(source_xml->find_elem(L"settings"));
            source->set_properties(*settings_xml);
            manager->add_source(source);
         }
         else
            throw std::invalid_argument("Invalid source type");
      }

      // we now need to add all of the requests that are described in the input
      Element::value_type requests_xml(input_xml.find_elem(L"requests"));
      for(Element::iterator ri = requests_xml->begin(); ri != requests_xml->end(); ++ri)
      {
         Element::value_type &request_xml = *ri;
         Manager::request_handle request(
            new Cora::DataSources::Request(
               &sink, request_xml->get_attr_str(L"uri")));
         Element::iterator start_it = request_xml->find(L"start");
         Element::iterator order_it = request_xml->find(L"order");

         if(start_it != request_xml->end())
         {
            Element::value_type &start = *start_it;
            StrUni option(start->get_attr_wstr(L"option"));
            if(option == L"at-record")
            {
               request->set_start_at_record(
                  start->get_attr_uint4(L"file-mark"), start->get_attr_uint4(L"record-no"));
            }
            else if(option == L"at-time")
            {
               request->set_start_at_time(start->get_attr_lgrdate(L"time"));
            }
            else if(option == L"at-newest")
            {
               request->set_start_at_newest();
            }
            else if(option == L"after-newest")
            {
               request->set_start_after_newest();
            }
            else if(option == L"relative-to-newest")
            {
               int8 seconds(start->get_attr_int8(L"interval"));
               request->set_start_relative_to_newest(seconds * Csi::LgrDate::nsecPerSec);
            } 
            else if(option == L"at-offset-from-newest")
            {
               request->set_start_at_offset_from_newest(start->get_attr_uint4(L"offset"));
            }
            else if(option == L"query")
            {
               request->set_query_times(start->get_attr_lgrdate(L"begin"), start->get_attr_lgrdate(L"end"));
            }
         }
         if(order_it != request_xml->end())
         {
            Element::value_type order(*order_it);
            StrUni const option(order->get_attr_wstr(L"option"));
            if(option == L"collected")
               request->set_order_option(Request::order_collected);
            else if(option == L"logged")
               request->set_order_option(Request::order_logged_with_holes);
            else if(option == L"logged-without-holes")
               request->set_order_option(Request::order_logged_without_holes);
            else if(option == L"real-time")
               request->set_order_option(Request::order_real_time);
         }
         manager->add_request(request);
      }

      // we will also start any expressions that are specified in the test input
      Element::iterator expressions_it = input_xml.find(L"expressions");
      if(expressions_it != input_xml.end())
      {
         Element::value_type &expressions_xml(*expressions_it);
         for(Element::iterator ei = expressions_xml->begin(); ei != expressions_xml->end(); ++ei)
         {
            Element::value_type &expression = *ei;
            expressions.push_back(new MyExpression(manager, expression->get_cdata_str()));
         }
      }

      // we will test any database directorives that may be present
      Element::iterator db_it = input_xml.find(L"csidb");
      if(db_it != input_xml.end())
      {
         Element::value_type &db(*db_it);
         for(Element::iterator di = db->begin(); di != db->end(); ++di)
         {
            Element::value_type &db_test(*di);
            if(db_test->get_name() == L"list-sources")
            {
               Cora::DataSources::CsiDbSource::source_names_type source_names;
               int source_type = -1;
               if(db_test->has_attribute(L"type"))
                  source_type = db_test->get_attr_int4(L"type");
               Cora::DataSources::CsiDbSource::list_sources(source_names, source_type);
               std::for_each(source_names.begin(), source_names.end(), describe_source_name());
            }
            else if(db_test->get_name() == L"list-databases")
            {
               try
               {
                  Cora::DataSources::CsiDbSource::databases_type databases;
                  Cora::DataSources::CsiDbSource::list_databases(databases, *db_test);
                  std::cout << "Database list\n";
                  while(!databases.empty())
                  {
                     std::cout << "  " << databases.front() << "\n";
                     databases.pop_front();
                  }
               }
               catch(std::exception &e)
               {
                  std::cout << "list databases failed: " << e.what() << std::endl;
               }
            }
         }
      }

      // we need to create an alarm manager
      Csi::SharedPtr<Csi::Alarms::Manager> alarm_manager(new Csi::Alarms::Manager(manager));
      Element::iterator alarms_it(input_xml.find(L"alarms"));
      MyAlarmClient alarm_client;
      Csi::Alarms::Manager::read_errors_type read_errors;
      if(alarms_it != input_xml.end())
         alarm_manager->read(**alarms_it, read_errors);
      for(auto ai = alarm_manager->begin();
          ai != alarm_manager->end();
          ++ai)
      {
         auto &alarm(*ai);
         alarm->set_client(&alarm_client);
      }

      // we need to clone any alarms that are called out in the XML
      Element::iterator clone_alarms_it(input_xml.find(L"clone-alarms"));
      if(clone_alarms_it != input_xml.end())
      {
         Element::value_type &cloned(*clone_alarms_it);
         for(Element::iterator ci = cloned->begin(); ci != cloned->end(); ++ci)
         {
            auto source(alarm_manager->find_alarm((*ci)->get_cdata_wstr()));
            auto clone(alarm_manager->clone_alarm(source.get_rep()));
            clone->set_client(&alarm_client);
            std::cout << "alarm " << source->get_id() << " cloned as " << clone->get_id() << std::endl;
         }
      }
      
      // we will now drive the message loop forever.  Before doing so, we will need to start the
      // sources
      MSG message;
      manager->start_sources();
      alarm_manager->start();
      while(!stop_execution && GetMessage(&message, 0, 0, 0))
      {
         TranslateMessage(&message);
         DispatchMessage(&message);
      }
   }
   catch(std::exception &e)
   {
      cout << "test error: " << e.what() << endl;
      rtn = 1;
   }

   // clear the coratools
   Csi::Event::set_dispatcher(0);
   Csi::MessageWindow::uninitialise();

   getchar();
   return rtn;
} // main
