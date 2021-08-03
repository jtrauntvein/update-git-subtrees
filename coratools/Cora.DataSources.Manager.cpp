/* Cora.DataSources.Manager.cpp

   Copyright (C) 2008, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 01 August 2008
   Last Change: Tuesday 30 April 2013
   Last Commit: $Date: 2018-07-09 10:01:56 -0600 (Mon, 09 Jul 2018) $
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.Manager.h"
#include <algorithm>


namespace Cora
{
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class GetTableRangeCompleteEvent definitions
      ////////////////////////////////////////////////////////////
      uint4 const GetTableRangeCompleteEvent::event_id(
         Csi::Event::registerType("Cora::DataSources::GetTableRangeCompleteEvent"));

      
      ////////////////////////////////////////////////////////////
      // class Manager definitions
      ////////////////////////////////////////////////////////////
      Manager::Manager()
      {
         timer.bind(new OneShot);
         value_factory.bind(new Cora::Broker::ValueFactory);
      } // constructor


      Manager::~Manager()
      {
         clients.clear();
         value_factory.clear();
      }


      void Manager::add_client(client_type *client)
      {
         if(std::find(clients.begin(), clients.end(), client) == clients.end())
            clients.push_back(client);
      } // add_client


      void Manager::remove_client(client_type *client)
      {
         clients_type::iterator ci = std::find(
            clients.begin(), clients.end(), client);
         if(ci != clients.end())
            clients.erase(ci);
      } // remove_client


      void Manager::set_supervisor(supervisor_handle supervisor_)
      {
         if(requests.empty())
            supervisor = supervisor_;
         else
            throw Csi::MsgExcept("attempt to add a supervisor when requests are pending.");
      } // set_supervisor


      Manager::iterator Manager::find(SourceBase *source)
      {
         return std::find_if(
            sources.begin(), sources.end(), Csi::HasSharedPtr<SourceBase>(source));
      }


      Manager::const_iterator Manager::find(SourceBase *source) const
      {
         return std::find_if(
            sources.begin(), sources.end(), Csi::HasSharedPtr<SourceBase>(source));
      }


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate source_has_name
         ////////////////////////////////////////////////////////////
         struct source_has_name
         {
            StrUni const &name;
            source_has_name(StrUni const &name_):
               name(name_)
            { }

            bool operator ()(Manager::source_handle const &source) const
            { return source->get_name() == name; }
         };


         ////////////////////////////////////////////////////////////
         // functor do_start_source
         ////////////////////////////////////////////////////////////
         struct do_start_source
         {
            void operator ()(Manager::value_type &source)
            { source->start(); } 
         };


         ///////////////////////////////////////////////////////////
         // do_add_source
         ///////////////////////////////////////////////////////////
         struct do_add_source
         {
            SourceBase *source;

            do_add_source(SourceBase *source_):
               source(source_)
            { }

            void operator()(Manager::request_handle &request)
            {
               if(request->get_source() == 0 && 
                  request->get_source_name() == source->get_name())
                  request->set_source(source);
            }
         };


         ///////////////////////////////////////////////////////////
         // do_remove_source
         ///////////////////////////////////////////////////////////
         struct do_remove_source
         {
            SourceBase *source;

            do_remove_source(SourceBase *source_):
               source(source_)
            { }

            void operator()(Manager::request_handle &request)
            {
               if(request->get_source() == source)
                  request->set_source(0);
            }
         };


         ////////////////////////////////////////////////////////////
         // functor do_stop_source
         ////////////////////////////////////////////////////////////
         struct do_stop_source
         {
            void operator ()(Manager::value_type &source)
            { source->stop(); }
         };


         ////////////////////////////////////////////////////////////
         // class event_source_connecting
         ////////////////////////////////////////////////////////////
         class event_source_connecting: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // source
            ////////////////////////////////////////////////////////////
            Manager::value_type source;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(Manager *manager, Manager::value_type &source)
            {
               event_source_connecting *ev = new event_source_connecting(manager, source);
               ev->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_source_connecting(Manager *manager, Manager::value_type &source_):
               Event(event_id, manager),
               source(source_)
            { }
         };


         uint4 const event_source_connecting::event_id = Csi::Event::registerType(
            "Cora::DataSources::Manager::event_source_connecting");
         

         ////////////////////////////////////////////////////////////
         // functor do_report_source_connecting
         ////////////////////////////////////////////////////////////
         struct do_report_source_connecting
         {
            Manager::value_type &source;
            do_report_source_connecting(Manager::value_type &source_):
               source(source_)
            { }

            void operator ()(ManagerClient *client)
            {
               if(ManagerClient::is_valid_instance(client))
                  client->on_source_connecting(source->get_manager(), source);
            }
         }; 


         ////////////////////////////////////////////////////////////
         // class event_source_connect
         ////////////////////////////////////////////////////////////
         class event_source_connect: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // source
            ////////////////////////////////////////////////////////////
            Manager::value_type source;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(Manager *manager, Manager::value_type &source)
            {
               event_source_connect *ev = new event_source_connect(manager, source);
               ev->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_source_connect(Manager *manager, Manager::value_type &source_):
               Event(event_id, manager),
               source(source_)
            { }
         };


         uint4 const event_source_connect::event_id = Csi::Event::registerType(
            "Cora::DataSources::Manager::event_source_connect"); 

         
         ////////////////////////////////////////////////////////////
         // functor do_report_source_connect
         ////////////////////////////////////////////////////////////
         struct do_report_source_connect
         {
            Manager::value_type &source;
            do_report_source_connect(Manager::value_type &source_):
               source(source_)
            { }

            void operator ()(ManagerClient *client)
            {
               if(ManagerClient::is_valid_instance(client))
                  client->on_source_connect(source->get_manager(), source);
            }
         };


         ////////////////////////////////////////////////////////////
         // class event_source_disconnect
         ////////////////////////////////////////////////////////////
         class event_source_disconnect: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // source
            ////////////////////////////////////////////////////////////
            Manager::value_type source;

            ////////////////////////////////////////////////////////////
            // reason
            ////////////////////////////////////////////////////////////
            ManagerClient::disconnect_reason_type const reason;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               Manager *manager,
               Manager::value_type &source,
               ManagerClient::disconnect_reason_type reason)
            {
               event_source_disconnect *ev = new event_source_disconnect(manager, source, reason);
               ev->post();
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_source_disconnect(
               Manager *manager,
               Manager::value_type &source_,
               ManagerClient::disconnect_reason_type reason_):
               Event(event_id, manager),
               source(source_),
               reason(reason_)
            { }
         };


         uint4 const event_source_disconnect::event_id = Csi::Event::registerType(
            "Cora::DataSources::Manager::event_source_disconnect");

         
         ////////////////////////////////////////////////////////////
         // functor do_report_source_disconnect
         ////////////////////////////////////////////////////////////
         struct do_report_source_disconnect
         {
            Manager::value_type &source;
            ManagerClient::disconnect_reason_type reason;
            do_report_source_disconnect(
               Manager::value_type &source_,
               ManagerClient::disconnect_reason_type reason_):
               source(source_),
               reason(reason_)
            { }

            void operator ()(ManagerClient *client)
            {
               if(ManagerClient::is_valid_instance(client))
                  client->on_source_disconnect(source->get_manager(), source, reason);
            }
         };


         ////////////////////////////////////////////////////////////
         // functor do_report_source_added
         ////////////////////////////////////////////////////////////
         struct do_report_source_added
         {
            Manager::value_type &source;
            do_report_source_added(Manager::value_type &source_):
               source(source_)
            { }

            void operator ()(ManagerClient *client)
            {
               if(ManagerClient::is_valid_instance(client))
                  client->on_source_added(source->get_manager(), source);
            }
         };


         ////////////////////////////////////////////////////////////
         // functor do_on_source_removed
         ////////////////////////////////////////////////////////////
         struct do_report_source_removed
         {
            Manager::value_type &source;
            do_report_source_removed(Manager::value_type &source_):
               source(source_)
            { }

            void operator ()(ManagerClient *client)
            {
               if(ManagerClient::is_valid_instance(client))
                  client->on_source_removed(source->get_manager(), source);
            }
         };


         ////////////////////////////////////////////////////////////
         // struct do_report_source_log
         ////////////////////////////////////////////////////////////
         struct do_report_source_log
         {
            Manager::value_type source;
            StrAsc const message;
            do_report_source_log(Manager::value_type &source_, StrAsc const &message_):
               source(source_),
               message(message_)
            { }

            void operator ()(ManagerClient *client)
            {
               if(client)
                  client->on_source_log(source->get_manager(), source, message); 
            }
         };
      };


      Manager::iterator Manager::find(StrUni const &name)
      {
         return std::find_if(
            sources.begin(), sources.end(), source_has_name(name));
      } // find


      Manager::const_iterator Manager::find(StrUni const &name) const
      {
         return std::find_if(
            sources.begin(), sources.end(), source_has_name(name));
      } // find
      

      Manager::source_handle Manager::find_source(StrUni const &name)
      {
         StrUni source_name;
         size_t start_pos = 0;
         size_t end_pos;
         if(name.first() == '\"')
            start_pos = 1;
         end_pos = name.find(L"\"", start_pos);
         name.sub(source_name, start_pos, end_pos - start_pos);

         sources_type::iterator si = std::find_if(
            sources.begin(), sources.end(), source_has_name(source_name));
         source_handle rtn;
         if(si != sources.end())
            rtn = *si;
         return rtn;
      } // find_source


      StrUni Manager::parse_source_name(StrUni const &uri)
      {
         StrUni rtn;
         size_t start_pos = 0;
         size_t end_pos;
         
         if(uri.first() == '\"')
            start_pos = 1;
         end_pos = uri.rfind(L":");
         uri.sub(rtn, start_pos, end_pos - start_pos);
         return rtn;
      } // parse_source_name


      void Manager::add_source(source_handle source, bool start_now)
      {
         clients_type::iterator ci = clients.begin();
         source->set_manager(this);
         sources.push_back(source);
         std::for_each(clients.begin(), clients.end(), do_report_source_added(source));
         std::for_each(requests.begin(), requests.end(), do_add_source(source.get_rep()));
         source->connect();
         if(start_now)
            source->start();
      } // add_source


      void Manager::remove_source(SourceBase *source_)
      {
         sources_type::iterator si = find(source_);
         if(si != sources.end())
         {
            clients_type::iterator ci = clients.begin();
            source_handle source(*si);
            sources.erase(si);
            source->disconnect();
            std::for_each(clients.begin(), clients.end(), do_report_source_removed(source));
            std::for_each(requests.begin(), requests.end(), do_remove_source(source.get_rep()));
            source->set_manager(0);
         }
      } // remove_source


      void Manager::remove_all_sources()
      {
         while(!sources.empty())
            remove_source(sources.begin()->get_rep());
      } // remove_all_sources


      void Manager::start_sources()
      {
         std::for_each(sources.begin(), sources.end(), do_start_source());
      } // start_sources


      void Manager::stop_sources()
      {
         std::for_each(sources.begin(), sources.end(), do_stop_source());
      } // stop_sources


      void Manager::report_source_connecting(SourceBase *source)
      { 
         sources_type::iterator si = std::find(sources.begin(), sources.end(), source);
         if(si != sources.end())
            event_source_connecting::cpost(this, *si);
      } // report_source_starting


      void Manager::report_source_connect(SourceBase *source)
      {
         sources_type::iterator si = std::find(sources.begin(), sources.end(), source);
         if(si != sources.end())
            event_source_connect::cpost(this, *si);
      } // report_source_started


      void Manager::report_source_disconnect(SourceBase *source, ManagerClient::disconnect_reason_type reason)
      {
         sources_type::iterator si = std::find(sources.begin(), sources.end(), source);
         if(si != sources.end())
            event_source_disconnect::cpost(this, *si, reason);
      } // report_source_stopped


      void Manager::report_source_log(SourceBase *source, StrAsc const &message)
      {
         if(source)
         {
            sources_type::iterator si = std::find(sources.begin(), sources.end(), source);
            if(si != sources.end())
            {
               std::for_each(
                  clients.begin(), clients.end(), do_report_source_log(*si, message));
            }
         }
      } // report_source_log
      

      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_add_request
         ////////////////////////////////////////////////////////////
         class event_add_request: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // request
            ////////////////////////////////////////////////////////////
            typedef Csi::SharedPtr<Request> request_handle;
            request_handle request;

            ////////////////////////////////////////////////////////////
            // more_to_follow
            ////////////////////////////////////////////////////////////
            bool more_to_follow;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               Manager *manager, request_handle &request, bool more_to_follow)
            {
               event_add_request *event(
                  new event_add_request(manager, request, more_to_follow));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_add_request(
               Manager *manager, request_handle &request_, bool more_to_follow_):
               Event(event_id, manager),
               request(request_),
               more_to_follow(more_to_follow_)
            { }
         };


         uint4 const event_add_request::event_id(
            Csi::Event::registerType("Cora::DataSources::Manager::event_add_request"));


         ////////////////////////////////////////////////////////////
         // class event_remove_request
         ////////////////////////////////////////////////////////////
         class event_remove_request: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // request
            ////////////////////////////////////////////////////////////
            typedef Csi::SharedPtr<Request> request_handle;
            request_handle request;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(Manager *manager, request_handle &request)
            {
               event_remove_request *event(new event_remove_request(manager, request));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_remove_request(Manager *manager, request_handle &request_):
               Event(event_id, manager),
               request(request_)
            { }
         };


         uint4 const event_remove_request::event_id(
            Csi::Event::registerType("Cora::DataSources::Manager::event_remove_request"));


         ////////////////////////////////////////////////////////////
         // class event_activate_requests
         ////////////////////////////////////////////////////////////
         class event_activate_requests: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(Manager *manager)
            {
               event_activate_requests *event(new event_activate_requests(manager));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_activate_requests(Manager *manager):
               Event(event_id, manager)
            { }
         };


         uint4 const event_activate_requests::event_id(
            Csi::Event::registerType("Cora::DataSources::Manager::event_activate_requests"));
      };

      
      void Manager::add_request(request_handle request, bool more_to_follow)
      {
         if(supervisor != 0)
            supervisor->on_add_request(this, request); 
         event_add_request::cpost(this, request, more_to_follow);
      } // add_request


      void Manager::activate_requests()
      { event_activate_requests::cpost(this); }
         

      void Manager::remove_request(request_handle &request)
      {
         request->set_state(request->get_source(), Request::state_remove_pending);
         event_remove_request::cpost(this, request);
      } // remove_request


      void Manager::remove_requests(SinkBase *sink)
      {
         for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
         {
            request_handle request = *ri;
            if(request->get_sink() == sink)
               remove_request(request);
         }
      } // remove_requests


      void Manager::remove_all_requests()
      {
         requests.clear();
         for(sources_type::iterator si = sources.begin(); si != sources.end(); ++si)
         {
            sources_type::value_type &source(*si);
            source->remove_all_requests();
         }
      } // remove_all_requests
      

      StrUni Manager::get_statistic_uri(StrUni const &station_uri, StrUni const &statistic_name)
      {
         source_handle source(find_source_uri(station_uri));
         Csi::OStrAscStream rtn;

         if(source != 0)
            source->get_statistic_uri(rtn, station_uri, statistic_name);
         return rtn.str();
      } // get_statistic_uri


      StrUni Manager::get_statistic_station(StrUni const &statistic_uri)
      {
         source_handle source(find_source_uri(statistic_uri));
         Csi::OStrAscStream rtn;

         if(source != 0)
            source->get_statistic_station(rtn, statistic_uri);
         return rtn.str();
      } // get_statistic_station


      bool Manager::start_set_value(
         SinkBase *sink,
         StrUni const &uri,
         ValueSetter const &value)
      {
         source_handle source(find_source_uri(uri));
         bool rtn(false);

         if(source != 0)
            rtn = source->start_set_value(sink, uri, value);
         return rtn;
      } // start_set_value


      void Manager::breakdown_uri(
         symbols_type &symbols, StrUni const &uri, bool quote_embedded_periods)
      {
         source_handle source(find_source_uri(uri));
         symbols.clear();
         if(source != 0)
            source->breakdown_uri(symbols, uri);
         if(quote_embedded_periods)
         {
            for(symbols_type::iterator si = symbols.begin(); si != symbols.end(); ++si)
               si->first.replace(L".", L"\\.");
         }
      } // breakdown_uri
      
         
      StrUni Manager::make_table_uri(StrUni const &uri)
      {
         StrUni rtn;
         SourceBase::symbols_type symbols;
         int last_symbol_type = -1;
         StrUni temp;
         
         breakdown_uri(symbols, uri);
         for(SourceBase::symbols_type::iterator si = symbols.begin();
             si != symbols.end() && last_symbol_type != SymbolBase::type_table;
             ++si)
         {
            temp = si->first;
            temp.replace(L".", L"\\.");
            rtn.append(temp);
            if(si == symbols.begin())
               rtn.append(':');
            last_symbol_type = si->second;
            if(last_symbol_type != SymbolBase::type_table && si != symbols.begin())
               rtn.append('.');
         }
         if(last_symbol_type != SymbolBase::type_table)
            rtn.cut(0);
         return rtn;
      } // make_table_uri
      

      bool Manager::source_is_connected(StrUni const &uri)
      {
         bool rtn(false);
         source_handle source(find_source_uri(uri));
         if(source != 0)
            rtn = source->is_connected();
         return rtn;
      } // source_is_connected

      
      void Manager::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         uint4 const type = ev->getType();
         clients_type clients(this->clients);
         if(type == event_source_connecting::event_id)
         {
            event_source_connecting *event = static_cast<event_source_connecting *>(ev.get_rep());
            std::for_each(clients.begin(), clients.end(), do_report_source_connecting(event->source));
         }
         else if(type == event_source_connect::event_id)
         {
            event_source_connect *event = static_cast<event_source_connect *>(ev.get_rep());
            std::for_each(clients.begin(), clients.end(), do_report_source_connect(event->source));
         }
         else if(type == event_source_disconnect::event_id)
         {
            event_source_disconnect *event = static_cast<event_source_disconnect *>(ev.get_rep());
            std::for_each(clients.begin(), clients.end(), do_report_source_disconnect(event->source, event->reason));
         }
         else if(type == event_add_request::event_id)
         {
            event_add_request *event(static_cast<event_add_request *>(ev.get_rep()));
            request_handle &request(event->request);
            if(SinkBase::is_valid_instance(request->get_sink()))
            {
               source_handle source(find_source_uri(request->get_uri()));
               requests.push_back(request);
               if(source != 0)
               {
                  request->set_state(source.get_rep(), Request::state_pending);
                  if(source->is_started())
                  {
                     try
                     {
                        source->add_request(request, event->more_to_follow);
                     }
                     catch(std::exception &e)
                     { trace(e.what()); }
                  }
               }
               else
               {
                  request->set_state(source.get_rep(), Request::state_error);
                  request->get_sink()->on_sink_failure(
                     this, request, SinkBase::sink_failure_invalid_source);
               }
            }
         }
         else if(type == event_remove_request::event_id)
         {
            event_remove_request *event(static_cast<event_remove_request *>(ev.get_rep()));
            request_handle &request(event->request);
            requests_type::iterator ri = std::find(requests.begin(), requests.end(), request);
            if(ri != requests.end())
            {
               if(request->get_source() != 0)
                  request->get_source()->remove_request(request);
               requests.erase(ri);
            } 
         }
         else if(type == event_activate_requests::event_id)
         {
            for(sources_type::iterator si = sources.begin(); si != sources.end(); ++si)
            {
               sources_type::value_type &source = *si;
               source->activate_requests();
            }
         }
      } // receive


      bool Manager::is_classic_inlocs(StrUni const &uri)
      {
         bool rtn = false;
         source_handle source(find_source_uri(uri));
         if(source != 0)
            rtn = source->is_classic_inlocs(uri);
         return rtn;
      } // is_classic_inlocs
   };
};
