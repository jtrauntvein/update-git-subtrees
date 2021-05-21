/* $HeadURL: svn://engsoft/cora/coratools/Cora.DataSources.KonectSymbol.cpp $

Copyright (C) 2014, 2016 Campbell Scientific, Inc.
Started On: Tuesday, April 22, 2014

Started By: Tyler Mecham
$LastChangedBy: jon $
$LastChangedDate: 2016-08-27 09:42:07 -0600 (Sat, 27 Aug 2016) $
$LastChangedRevision: 28682 $
*/


#include "Cora.DataSources.KonectSymbol.h"
#include "Cora.DataSources.TableFieldUri.h"
#include "Cora.DataSources.Manager.h"
#include "Cora.DataSources.SymbolBrowser.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.ByteQueueStream.h"
#include "Csi.Json.h"
#include "Csi.ByteOrder.h"
#include "Csi.Utils.h"
#include "Csi.Digest.h"
#include "Csi.Hmac.h"
#include "Csi.Base64.h"
#include <set>
#include <iterator>

namespace Cora
{
   namespace DataSources
   {
      namespace
      {
      };

      
      StrUni const KonectSourceSymbol::konect_guids_name(L"konect-guids");
      StrUni const KonectSourceSymbol::konect_guid_name(L"konect-guid");
      StrUni const KonectSourceSymbol::key_name(L"key");
      StrUni const KonectSourceSymbol::guid_name(L"guid");

      KonectSourceSymbol::KonectSourceSymbol(KonectSource *source):
         SymbolBase(source, source->get_name()),
         expansion_started(false)
      {
      }


      KonectSourceSymbol::~KonectSourceSymbol()
      {
         connection.clear();
      }


      void KonectSourceSymbol::set_guid_properties(Csi::Xml::Element &prop_xml)
      {
         try
         {
            StrUni key, guid;
            Csi::Xml::Element::value_type guids_elem = prop_xml.find_elem(konect_guids_name);
            Csi::Xml::Element::iterator guid_it = guids_elem->begin();
            while(guid_it != guids_elem->end())
            {
               Csi::Xml::Element::value_type guid_elem = *guid_it;
               if(guid_elem->has_attribute(key_name))
               {
                  key = guid_elem->get_attr_str(key_name);
               }
               if(guid_elem->has_attribute(guid_name))
               {
                  guid = guid_elem->get_attr_str(guid_name);
               }
               guids[key] = guid;
               ++guid_it;
            }
         }
         catch(std::exception &)
         { }
      }


      void KonectSourceSymbol::get_guid_properties(Csi::Xml::Element &prop_xml)
      {
         try
         {
            Csi::SharedPtr<Csi::Xml::Element> guids_elem = prop_xml.add_element(konect_guids_name);
            guids_type::const_iterator it = guids.begin();
            while(it != guids.end())
            {
               Csi::SharedPtr<Csi::Xml::Element> guid_elem = guids_elem->add_element(konect_guid_name);
               guid_elem->add_attribute(key_name, it->first);
               guid_elem->add_attribute(guid_name, it->second);
               ++it;
            }
         }
         catch(std::exception &)
         { }
      }


      bool KonectSourceSymbol::find_guid(StrUni const &key, StrUni &guid)
      {
         bool rtn = false;

         guids_type::iterator guid_it = guids.find(key);
         if(guid_it != guids.end())
         {
            guid = guid_it->second;
            rtn = true;
         }

         //Did we find a cached guid for the key?
         if(!rtn)
         {
            //We need to look up the guid
            Cora::DataSources::Manager::symbols_type uri_symbols;
            source->breakdown_uri(uri_symbols, key);
            Cora::DataSources::Manager::symbols_type::iterator it = uri_symbols.begin();
            while(it != uri_symbols.end())
            {
               if(it->second == Cora::DataSources::SymbolBase::type_station)
               {
                  break;
               }
               ++it;
            }
         }
         return rtn;
      }


      bool KonectSourceSymbol::is_connected() const
      {
         return source->is_connected();
      }


      void KonectSourceSymbol::start_expansion()
      {
         KonectSource *source(static_cast<KonectSource *>(source));

         // we need to form the HTTP request
         Csi::Uri uri("https://apipreview.konectgds.com/v1/stations");
         http_request.bind(new http_request_type(this, uri));
         http_request->set_method(Csi::HttpClient::Request::method_get);
         http_request->set_authorisation(
            new KonectSourceHelpers::AuthorisationOAuth(
               source->get_oauth_access_token(),
               source->get_signing_key()));
         if(connection == 0)
            connection = source->get_connection();
         connection->add_request(http_request);
      }


      void KonectSourceSymbol::refresh()
      {
         children.clear();
         start_expansion();
      }


      bool KonectSourceSymbol::on_failure(http_request_type *request)
      {
         if(http_request == request)
            http_request.clear();
         return true;
      } // on_failure


      bool KonectSourceSymbol::on_response_complete(http_request_type *request)
      {
         try
         {
            if(request->get_response_code() == 200)
            {
               // we will first parse the response as JSON
               Csi::IByteQueueStream input(&request->get_receive_buff());
               Csi::Json::Array response;
               response.parse(input);
               Csi::Json::Array::iterator station_it = response.begin();
               while(station_it != response.end())
               {
                  Csi::Json::ObjectHandle json_field(*station_it);
                  Csi::Json::ObjectHandle station_link((*json_field)["link"]);
                  StrUni station_name = station_link->get_property_str("title");
                  Csi::Uri station_uri(station_link->get_property_str("href"));
                  std::list<StrUni> path_names;
                  station_uri.get_path_names(path_names);
                  StrUni station_guid = path_names.back();

                  //Add the station_guid to the source
                  guids[station_name] = station_guid;
                  
                  value_type station_symbol(
                     new KonectStationSymbol((KonectSource*)source, 
                     this, 
                     station_name, 
                     station_guid));

                  station_symbol->set_browser(browser);
                  children.insert(
                     std::upper_bound(
                     children.begin(),
                     children.end(),
                     station_symbol,
                     symbol_name_less()),
                     station_symbol);
                  browser->send_symbol_added(station_symbol);
                  ++station_it;
               }
            }
         }
         catch(std::exception &e)
         {
            trace(e.what());
         }
         http_request.clear();
         return true;
      } // on_response_complete


      KonectStationSymbol::KonectStationSymbol(KonectSource *source,
                                               KonectSourceSymbol *parent,
                                               StrUni const &title,
                                               StrUni const &guid_):
         SymbolBase(source, title, parent),
         guid(guid_),
         expansion_started(false)
      {
      }


      KonectStationSymbol::~KonectStationSymbol()
      {
      }


      bool KonectStationSymbol::is_connected() const
      {
         return source->is_connected();
      }


      void KonectStationSymbol::start_expansion()
      {
         KonectSource *source(static_cast<KonectSource *>(source));

         // we need to form the HTTP request
         StrAsc table_uri("https://apipreview.konectgds.com/v1/stations/");
         table_uri.append(guid.to_utf8());
         Csi::Uri uri(table_uri);
         http_request.bind(new http_request_type(this, uri));
         http_request->set_method(Csi::HttpClient::Request::method_get);
         http_request->set_authorisation(
            new KonectSourceHelpers::AuthorisationOAuth(
               source->get_oauth_access_token(),
               source->get_signing_key()));
         if(connection == 0)
            connection = source->get_connection();
         connection->add_request(http_request);
      }


      void KonectStationSymbol::refresh()
      {
         children.clear();
         start_expansion();
      }


      bool KonectStationSymbol::on_failure(http_request_type *request)
      {
         if(http_request == request)
            http_request.clear();
         return true;
      } // on_failure


      bool KonectStationSymbol::on_response_complete(http_request_type *request)
      {
         try
         {
            if(request->get_response_code() == 200)
            {
               Csi::ByteQueue &bt = request->get_receive_buff();
               StrAsc contents;
               bt.pop(contents, bt.size());
               connection->add_log_comment(contents);
               Csi::IBuffStream buff_stream(contents.c_str(), contents.length());
               Csi::Json::Object response;
               response.parse(buff_stream);
               Csi::Json::ArrayHandle tables(response["tables"]);
               Csi::Json::Array::iterator table_it = tables->begin();
               while(table_it != tables->end())
               {
                  Csi::Json::ObjectHandle table(*table_it);
                  Csi::Json::ObjectHandle table_link = (*table)["link"];
                  StrAsc href = table_link->get_property_str("href");
                  Csi::Uri table_uri(href);
                  std::list<StrUni> pathes;
                  table_uri.get_path_names(pathes);
                  StrUni table_guid = pathes.back();
                  StrUni table_name = table_link->get_property_str("title");
                  KonectSourceSymbol *source_symbol = static_cast<KonectSourceSymbol*>(parent);
                  StrUni table_key(name);
                  table_key.append(L".");
                  table_key.append(table_name);
                  source_symbol->insert_guid(table_key, table_guid);

                  value_type table_symbol(
                     new KonectTableSymbol((KonectSource*)source,
                     this,
                     table_name,
                     table_guid));
                  table_symbol->set_browser(browser);
                  children.insert(
                     std::upper_bound(
                     children.begin(),
                     children.end(),
                     table_symbol,
                     symbol_name_less()),
                     table_symbol);
                  browser->send_symbol_added(table_symbol);
                  ++table_it;
               }
            }
         }
         catch(std::exception &e)
         {
            trace(e.what());
         }
         http_request.clear();
         return true;
      } // on_response_complete


      KonectTableSymbol::KonectTableSymbol(KonectSource *source,
                                           KonectStationSymbol *parent,
                                           StrUni const &title,
                                           StrUni const &guid_):
                                           SymbolBase(source, title, parent),
                                           guid(guid_),
                                           expansion_started(false)
      {
      }


      KonectTableSymbol::~KonectTableSymbol()
      {
      }


      bool KonectTableSymbol::is_connected() const
      {
         return source->is_connected();
      }


      void KonectTableSymbol::start_expansion()
      {
         KonectSource *source(static_cast<KonectSource *>(source));

         // we need to form the HTTP request
         StrAsc table_uri("https://apipreview.konectgds.com/v1/tables/");
         table_uri.append(guid.to_utf8());
         Csi::Uri uri(table_uri);
         http_request.bind(new http_request_type(this, uri));
         http_request->set_method(Csi::HttpClient::Request::method_get);
         http_request->set_authorisation(
            new KonectSourceHelpers::AuthorisationOAuth(
               source->get_oauth_access_token(),
               source->get_signing_key()));
         if(connection == 0)
            connection = source->get_connection();
         connection->add_request(http_request);
      }


      void KonectTableSymbol::refresh()
      {
         children.clear();
         start_expansion();
      }


      bool KonectTableSymbol::on_failure(http_request_type *request)
      {
         if(http_request == request)
            http_request.clear();
         return true;
      } // on_failure


      bool KonectTableSymbol::on_response_complete(http_request_type *request)
      {
         try
         {
            if(request->get_response_code() == 200)
            {
               Csi::ByteQueue &bt = request->get_receive_buff();
               StrAsc contents;
               bt.pop(contents, bt.size());
               connection->add_log_comment(contents);
               Csi::IBuffStream buff_stream(contents.c_str(), contents.length());
               Csi::Json::Object response;
               response.parse(buff_stream);
               Csi::Json::ArrayHandle fields(response["tablefields"]);
               Csi::Json::Array::iterator field_it = fields->begin();
               while(field_it != fields->end())
               {
                  Csi::Json::ObjectHandle field(*field_it);
                  StrUni fieldname = field->get_property_str("fieldname");
                  StrUni fieldtype = field->get_property_str("fieldtype");

                  value_type symbol(new KonectFieldSymbol(
                     (KonectSource*)source,
                     this,
                     fieldname,
                     fieldtype));
                  ++field_it;
                  symbol->set_browser(browser);
                  children.insert(
                     std::upper_bound(
                     children.begin(),
                     children.end(),
                     symbol,
                     symbol_name_less()),
                     symbol);
                  browser->send_symbol_added(symbol);
               }
            }
         }
         catch(std::exception &e)
         {
            trace(e.what());
         }
         http_request.clear();
         return true;
      } // on_response_complete


      KonectFieldSymbol::KonectFieldSymbol(KonectSource *source,
                                           KonectTableSymbol *parent,
                                           StrUni const &fieldname,
                                           StrUni const &fieldtype):
                                           SymbolBase(source, fieldname, parent),
                                           field_type(fieldtype)
      {
      }


      KonectFieldSymbol::~KonectFieldSymbol()
      {
      }


      bool KonectFieldSymbol::is_connected() const
      {
         return source->is_connected();
      }
   };
};
