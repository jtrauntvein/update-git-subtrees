/* $HeadURL: svn://engsoft/cora/coratools/Cora.DataSources.KonectSource.cpp $

Copyright (C) 2014, 2016 Campbell Scientific, Inc.
Started On: Tuesday, April 22, 2014

Started By: Tyler Mecham
$LastChangedBy: jon $
$LastChangedDate: 2016-08-27 09:43:34 -0600 (Sat, 27 Aug 2016) $
$LastChangedRevision: 28683 $
*/


#pragma hdrstop               // stop creation of precompiled header

#include "Cora.DataSources.KonectSource.h"
#include "Cora.DataSources.Manager.h"
#include "Cora.DataSources.SymbolBrowser.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.ByteQueueStream.h"
#include "Csi.Json.h"
#include "Csi.ByteOrder.h"
#include <set>
#include <iterator>
#include "Csi.LoginManager.h"
#include "Cora.DataSources.TableFieldUri.h"
#include "Csi.StrUniStream.h"
#include "Csi.Digest.h"
#include "Csi.Hmac.h"
#include "Csi.Base64.h"


namespace Cora
{
   namespace DataSources
   {
      namespace KonectSourceHelpers
      {
         ////////////////////////////////////////////////////////////
         // function json_type_to_csi
         //
         // Converts the type CsiJson type string specified to a CSI data type.
         ////////////////////////////////////////////////////////////
         CsiDbTypeCode json_type_to_csi(StrAsc const &json_type)
         {
            CsiDbTypeCode rtn(CsiUnknown);
            if(json_type == "dt_CsiIeee4")
               rtn = CsiIeee4;
            else if(json_type == "dt_CsiUInt1")
               rtn = CsiUInt1;
            else if(json_type == "dt_CsiUInt2")
               rtn = CsiUInt2;
            else if(json_type == "dt_CsiUInt4")
               rtn = CsiUInt4;
            else if(json_type == "dt_CsiInt1")
               rtn = CsiInt1;
            else if(json_type == "dt_CsiInt2")
               rtn = CsiInt2;
            else if(json_type == "dt_CsiInt4")
               rtn = CsiInt4;
            else if(json_type == "dt_CsiInt8")
               rtn = CsiInt8;
            else if(json_type == "dt_CsiFs2")
               rtn = CsiFs2;
            else if(json_type == "dt_CsiFs3")
               rtn = CsiFs3;
            else if(json_type == "dt_CsiFs4")
               rtn = CsiFs4;
            else if(json_type == "dt_CsiFsf")
               rtn = CsiFsf;
            else if(json_type == "dt_CsiFp4")
               rtn = CsiFp4;
            else if(json_type == "dt_CsiIeee4")
               rtn = CsiIeee4;
            else if(json_type == "dt_CsiIeee8")
               rtn = CsiIeee8;
            else if(json_type == "dt_CsiBool")
               rtn = CsiBool;
            else if(json_type == "dt_CsiBool8")
               rtn = CsiBool8;
            else if(json_type == "dt_CsiSec")
               rtn = CsiSec;
            else if(json_type == "dt_CsiUSec")
               rtn = CsiUSec;
            else if(json_type == "dt_CsiNSec")
               rtn = CsiNSec;
            else if(json_type == "dt_CsiAscii")
               rtn = CsiAscii;
            else if(json_type == "dt_CsiAsciiZ")
               rtn = CsiAsciiZ;
            else if(json_type == "dt_CsiInt4Lsf")
               rtn = CsiInt4Lsf;
            else if(json_type == "dt_CsiUInt2Lsf")
               rtn = CsiUInt2Lsf;
            else if(json_type == "dt_CsiUInt4Lsf")
               rtn = CsiUInt4Lsf;
            else if(json_type == "dt_CsiNSecLsf")
               rtn = CsiNSecLsf;
            else if(json_type == "dt_CsiIeee4Lsf")
               rtn = CsiIeee4Lsf;
            else if(json_type == "dt_CsiIeee8Lsf")
               rtn = CsiIeee8Lsf;
            else if(json_type == "dt_CsiInt8Lsf")
               rtn = CsiInt8Lsf;
            else if(json_type == "dt_CsiBool2")
               rtn = CsiBool2;
            else if(json_type == "dt_CsiBool4")
               rtn = CsiBool4;
            else if(json_type == "dt_CsiInt2Lsf")
               rtn = CsiInt2Lsf;
            else if(json_type == "dt_CsiLgrDate")
               rtn = CsiLgrDate;
            else if(json_type == "dt_CsiLgrDateLsf")
               rtn = CsiLgrDateLsf;
            return rtn;
         } // json_type_to_csi


         ////////////////////////////////////////////////////////////
         // class KonectWart
         ////////////////////////////////////////////////////////////
         class KonectWart: public WartBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // station_name
            ////////////////////////////////////////////////////////////
            StrUni const station_name;

            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrUni const table_name;

            ////////////////////////////////////////////////////////////
            // column_name
            ////////////////////////////////////////////////////////////
            StrUni const column_name;

            ////////////////////////////////////////////////////////////
            // wants_column_name
            ////////////////////////////////////////////////////////////
            bool wants_column_name;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            KonectWart(
               StrUni const &station_name_,
               StrUni const &table_name_,
               StrUni const &column_name_):
               station_name(station_name_),
               column_name(column_name_),
               table_name(table_name_),
               wants_column_name(column_name_.length() > 0)
            { }
         };

         ////////////////////////////////////////////////////////////
         // class Uri
         ////////////////////////////////////////////////////////////
         class Uri
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            Uri(StrUni const &uri)
            {
               parse(uri);
            }

            ////////////////////////////////////////////////////////////
            // parse
            ////////////////////////////////////////////////////////////
            void parse(StrUni const &uri);

            ////////////////////////////////////////////////////////////
            // get_source_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_source_name() const
            {
               return source_name;
            }

            ////////////////////////////////////////////////////////////
            // get_broker_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_broker_name() const
            {
               return broker_name;
            }

            ////////////////////////////////////////////////////////////
            // get_table_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_table_name() const
            {
               return table_name;
            }

            ////////////////////////////////////////////////////////////
            // get_column_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_column_name() const
            {
               return column_name;
            }

         private:
            ////////////////////////////////////////////////////////////
            // source_name
            ////////////////////////////////////////////////////////////
            StrUni source_name;

            ////////////////////////////////////////////////////////////
            // broker_name
            ////////////////////////////////////////////////////////////
            StrUni broker_name;

            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrUni table_name;

            ////////////////////////////////////////////////////////////
            // column_name
            ////////////////////////////////////////////////////////////
            StrUni column_name;
         };


         void Uri::parse(StrUni const &uri)
         {
            size_t source_end_pos = uri.rfind(L":");
            size_t start_pos = 0;

            if(uri.first() == '\"')
               start_pos = 1;
            source_name.cut(0);
            broker_name.cut(0);
            table_name.cut(0);
            column_name.cut(0);
            uri.sub(source_name, start_pos, source_end_pos - start_pos);
            if(source_name.last() == '\"')
               source_name.cut(source_name.length() - 1);
            if(source_end_pos < uri.length())
            {
               // we need take the "remnant" of the URI which is everything left after the source
               // name is cut out and trailing quotes are eliminated
               StrUni remnant;
               uri.sub(remnant, source_end_pos + 1, uri.length());
               if(remnant.last() == '\"')
                  remnant.cut(remnant.length() - 1);

               // because lgrnet station names can contain periods, we need to perform right to left
               // parsing on the URI.  We will do this breaking the remnant down into a collection
               // of tokens
               typedef std::list<StrUni> tokens_type;
               tokens_type tokens;
               StrUni token;

               while(remnant.length() > 0)
               {
                  size_t token_end_pos = remnant.rfind(L".");
                  if(token_end_pos < remnant.length())
                  {
                     remnant.sub(token, token_end_pos + 1, remnant.length());
                     tokens.push_front(token);
                     remnant.cut(token_end_pos);
                  }
                  else
                  {
                     tokens.push_front(remnant);
                     remnant.cut(0);
                  }
               }

               // before assigning meanings to the tokens, we need to combine any tokens that may
               // have been "commented"
               tokens_type::iterator ti = tokens.begin();
               while(ti != tokens.end())
               {
                  if(ti->last() == '\\')
                  {
                     tokens_type::iterator ti_next = ti;
                     if(++ti_next != tokens.end())
                     {
                        ti->cut(ti->length() - 1);
                        ti->append('.');
                        ti->append(*ti_next);
                        tokens.erase(ti_next);
                     }
                     else
                        ++ti;
                  }
                  else
                     ++ti;
               }

               // The URI for LgrNet should have, at most, three levels: station name, table name,
               // and column name.  We will form the broker name by appending all front tokens to
               // the broker name separated by periods.
               if(!tokens.empty())
               {
                  broker_name = tokens.front();
                  tokens.pop_front();
               }
               if(!tokens.empty())
               {
                  table_name = tokens.front();
                  tokens.pop_front();
               }
               for(tokens_type::iterator ti = tokens.begin(); ti != tokens.end(); ++ti)
               {
                  if(ti != tokens.begin())
                     column_name.append('.');
                  column_name.append(*ti);
               }
            }
         } // parse


         ////////////////////////////////////////////////////////////
         // class Cursor
         ////////////////////////////////////////////////////////////
         class Cursor: public Csi::HttpClient::RequestClient
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef KonectSource::request_handle request_handle;
            Cursor(KonectSource *source_, request_handle &request);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~Cursor();

            ////////////////////////////////////////////////////////////
            // add_request
            ////////////////////////////////////////////////////////////
            bool add_request(request_handle &request);

            ////////////////////////////////////////////////////////////
            // remove_request
            ////////////////////////////////////////////////////////////
            bool remove_request(request_handle &request);

            ////////////////////////////////////////////////////////////
            // has_no_requests
            ////////////////////////////////////////////////////////////
            bool has_no_requests();

            ////////////////////////////////////////////////////////////
            // poll
            ////////////////////////////////////////////////////////////
            void poll();

            ////////////////////////////////////////////////////////////
            // on_failure
            ////////////////////////////////////////////////////////////
            typedef Csi::HttpClient::Request http_request_type;
            virtual bool on_failure(http_request_type *request);

            ////////////////////////////////////////////////////////////
            // on_response_complete
            ////////////////////////////////////////////////////////////
            virtual bool on_response_complete(http_request_type *request);

            ////////////////////////////////////////////////////////////
            // get_previously_polled
            ////////////////////////////////////////////////////////////
            bool get_previously_polled() const
            {
               return previously_polled;
            }

            ////////////////////////////////////////////////////////////
            // on_failure
            ////////////////////////////////////////////////////////////
            void on_failure(SinkBase::sink_failure_type failure);

         private:
            ////////////////////////////////////////////////////////////
            // source
            ////////////////////////////////////////////////////////////
            KonectSource *source;

            ////////////////////////////////////////////////////////////
            // requests
            ////////////////////////////////////////////////////////////
            typedef std::list<request_handle> requests_type;
            requests_type requests;

            ////////////////////////////////////////////////////////////
            // previously_polled
            ////////////////////////////////////////////////////////////
            bool previously_polled;

            ////////////////////////////////////////////////////////////
            // record_desc
            ////////////////////////////////////////////////////////////
            typedef Broker::RecordDesc record_desc_type;
            Csi::SharedPtr<record_desc_type> record_desc;

            ////////////////////////////////////////////////////////////
            // cache
            ////////////////////////////////////////////////////////////
            typedef Csi::SharedPtr<Broker::Record> record_handle;
            typedef std::deque<record_handle> cache_type;
            cache_type cache;

            ////////////////////////////////////////////////////////////
            // last_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate last_time;

            ////////////////////////////////////////////////////////////
            // http_request
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<http_request_type> http_request;

            ////////////////////////////////////////////////////////////
            // table_guid
            ////////////////////////////////////////////////////////////
            StrUni table_guid;
         };


         AuthorisationOAuth::AuthorisationOAuth(
            StrAsc const &access_token,
            StrAsc const &signing_key_):
            signing_key(signing_key_)
         {
            Csi::OStrAscStream temp;
            temp.imbue(Csi::StringLoader::make_locale(0));
            temp << Csi::LgrDate::gmt().to_time_t();
            tokens.push_back(token_type("oauth_consumer_key", KonectSource::consumer_key));
            tokens.push_back(token_type("oauth_nonce", Csi::make_guid()));
            tokens.push_back(token_type("oauth_signature_method", "HMAC-SHA1"));
            tokens.push_back(token_type("oauth_timestamp", temp.str()));
            tokens.push_back(token_type("oauth_token", access_token));
            tokens.push_back(token_type("oauth_version", "1.0"));
         } // constructor


         void AuthorisationOAuth::write_authorisation(
            Csi::HttpClient::Request *request, std::ostream &out)
         {
            // we need to calculate the signature based on the URL and the tokens.
            Csi::OStrAscStream signature_base;
            Csi::OStrAscStream oauth_str;
            switch(request->get_method())
            {
            case Csi::HttpClient::Request::method_post:
               signature_base << "POST&";
               break;
               
            case Csi::HttpClient::Request::method_put:
               signature_base << "PUT&";
               break;
               
            case Csi::HttpClient::Request::method_get:
               signature_base << "GET&";
               break;
            }
            request->get_uri().format(signature_base);
            for(tokens_type::iterator ti = tokens.begin(); ti != tokens.end(); ++ti)
            {
               if(ti != tokens.begin())
                  oauth_str << "&";
               oauth_str << ti->first << "=" << ti->second;
            }
            Csi::Uri::encode(signature_base, oauth_str.str());
            
            // we need to calculate the signature as an hmac
            byte digest[Csi::Sha1Digest::digest_size];
            Csi::OStrAscStream oauth_sig;
            Csi::csi_hmac_sha1(signature_base.str(), signing_key, digest);
            Csi::Base64::encode(oauth_sig, digest, sizeof(digest));
            
            // we can now format the authorisation field
            out << "Authorization: OAuth ";
            for(tokens_type::iterator ti = tokens.begin(); ti != tokens.end(); ++ti)
            {
               if(ti != tokens.begin())
                  out << ", ";
               out << ti->first << "=\"" << ti->second << "\"";
            }
            out << ", " << "oauth_signature=\"" << oauth_sig.str() << "\"";
            out << "\r\n";
         } // write_authorisation            
      };


      //Assigned by KonectGDS for RTMC
      StrAsc const KonectSource::consumer_key("Axj1eh804mjTOCFyzJvwU2wGNBcMz8TM1xHCmXvUg");
      StrAsc const KonectSource::consumer_secret("R8KhastbrYDWDZ59TQlRKx3yqn8jNZkpksd6n96QeY");

      ////////////////////////////////////////////////////////////
      // class KonectSource definitions
      ////////////////////////////////////////////////////////////
      KonectSource::KonectSource(StrUni const &source_name):
         SourceBase(source_name)
      { }


      KonectSource::~KonectSource()
      { } // destructor


      void KonectSource::set_manager(Manager *manager)
      {
         SourceBase::set_manager(manager);
         if(manager)
         {
            timer = manager->get_timer();
            scheduler.bind(new Scheduler(timer));
         }
      } // set_manager


      void KonectSource::connect()
      {
         was_connected = true;
      } // connect


      void KonectSource::disconnect()
      {
         //bool report_disconnect(connect_active);
         //connect_active = was_connected = false;
         //connection.clear();
         //if(report_disconnect)
         //   manager->report_source_disconnect(this, ManagerClient::disconnect_by_application);
      } // disconnect


      bool KonectSource::is_connected() const
      {
         return connect_active;
      }


      StrUni const KonectSource::konect_pin_name(L"konect-pin");
      StrUni const KonectSource::oauth_access_token_name(L"oauth-access-token");
      StrUni const KonectSource::oauth_access_token_secret_name(L"oauth-access-token-secret");
      StrUni const KonectSource::poll_schedule_base_name(L"poll-base");
      StrUni const KonectSource::poll_schedule_interval_name(L"poll-interval");


      void KonectSource::get_properties(Csi::Xml::Element &prop_xml)
      {
         prop_xml.set_attr_str(konect_pin, konect_pin_name);
         prop_xml.set_attr_str(oauth_access_token, oauth_access_token_name);
         prop_xml.set_attr_str(oauth_access_token_secret, oauth_access_token_secret_name);
         prop_xml.set_attr_lgrdate(poll_schedule_base, poll_schedule_base_name);
         prop_xml.set_attr_uint4(poll_schedule_interval, poll_schedule_interval_name);
         Csi::PolySharedPtr<SymbolBase, KonectSourceSymbol> konect_symbol(get_source_symbol());
         konect_symbol->get_guid_properties(prop_xml);
      } // get_properties


      void KonectSource::set_properties(Csi::Xml::Element &prop_xml)
      {
         // we will set the optional properties to their default values
         bool reconnect = was_connected;

         disconnect();
         konect_pin = oauth_access_token = oauth_access_token_secret = "";
         poll_schedule_base = 0;
         poll_schedule_interval = 300000;

         // we can now apply the attributes
         if(prop_xml.has_attribute(konect_pin_name))
            konect_pin = prop_xml.get_attr_str(konect_pin_name);
         if(prop_xml.has_attribute(oauth_access_token_name))
         {
            oauth_access_token = prop_xml.get_attr_str(oauth_access_token_name);
            oauth_access_token_secret = prop_xml.get_attr_str(oauth_access_token_secret_name);
         }
         if(prop_xml.has_attribute(poll_schedule_base_name))
            poll_schedule_base = prop_xml.get_attr_lgrdate(poll_schedule_base_name);
         if(prop_xml.has_attribute(poll_schedule_interval_name))
            poll_schedule_interval = prop_xml.get_attr_uint4(poll_schedule_interval_name);

         Csi::PolySharedPtr<SymbolBase, KonectSourceSymbol> konect_symbol(get_source_symbol());
         konect_symbol->set_guid_properties(prop_xml);

         if(reconnect)
            connect();
      } // set_properties


      void KonectSource::start()
      {
         uint4 interval(poll_schedule_interval);
         SourceBase::start();
         if(interval < 100)
            interval = 100;
         poll_schedule_id = scheduler->start(this, poll_schedule_base, interval);
      } // start


      void KonectSource::add_request(
         request_handle &request, bool more_to_follow)
      {
         if(SinkBase::is_valid_instance(request->get_sink()))
         {
            // we need to search for a cursor that is compatible with this request.
            cursor_handle cursor;
            KonectSourceHelpers::Uri uri(request->get_uri());
            Csi::PolySharedPtr<WartBase, KonectSourceHelpers::KonectWart> wart;
            wart.bind(
               new KonectSourceHelpers::KonectWart(
               uri.get_broker_name().c_str(),
               uri.get_table_name().c_str(),
               uri.get_column_name().c_str()));
            request->set_wart(wart.get_handle());
            for(cursors_type::iterator ci = cursors.begin();
                cursor == 0 && ci != cursors.end();
                ++ci)
            {
               cursor_handle &candidate(*ci);
               if(candidate->add_request(request))
                  cursor = candidate;
            }
            if(cursor == 0)
            {
               cursor.bind(new cursor_type(this, request));
               cursors.push_back(cursor);
               if(!more_to_follow)
                  cursor->poll();
            }
         }
      } // add_request


      void KonectSource::remove_request(request_handle &request)
      {
         for(cursors_type::iterator ci = cursors.begin(); ci != cursors.end(); ++ci)
         {
            cursor_handle &cursor(*ci);
            if(cursor->remove_request(request))
            {
               if(cursor->has_no_requests())
                  cursors.erase(ci);
               break;
            }
         }
      } // remove_request


      void KonectSource::remove_all_requests()
      {
         cursors.clear();
      }


      void KonectSource::activate_requests()
      {
         for(cursors_type::iterator ci = cursors.begin(); ci != cursors.end(); ++ci)
         {
            cursor_handle &cursor(*ci);
            if(!cursor->get_previously_polled())
               cursor->poll();
         }
      } // activate_requests


      KonectSource::symbol_handle KonectSource::get_source_symbol()
      {
         if(symbol == 0)
         {
            symbol.bind(new KonectSourceSymbol(this));
         }
         return symbol.get_handle();
      } // get_source_symbol


      void KonectSource::breakdown_uri(symbols_type &symbols, StrUni const &uri_)
      {
         try
         {
            KonectSourceHelpers::Uri uri(uri_);
            symbols.clear();
            if(uri.get_source_name().length() > 0)
            {
               symbols.push_back(symbol_type(uri.get_source_name(), SymbolBase::type_konect_source));
               if(uri.get_broker_name().length() > 0)
               {
                  if(uri.get_broker_name() == L"__statistics__")
                     symbols.push_back(symbol_type(uri.get_broker_name(), SymbolBase::type_statistics_broker));
                  else
                     symbols.push_back(symbol_type(uri.get_broker_name(), SymbolBase::type_station));
                  if(uri.get_table_name().length() != 0)
                  {
                     symbols.push_back(symbol_type(uri.get_table_name(), SymbolBase::type_table));
                     if(uri.get_column_name().length() > 0)
                        symbols.push_back(symbol_type(uri.get_column_name(), SymbolBase::type_scalar));
                  }
               }
            }
         }
         catch(std::exception &)
         {
            symbols.clear();
         }
      } // breakdown_uri


      bool KonectSource::on_failure(http_request *request)
      {
         return false;
      } // on_failure


      bool KonectSource::on_response_complete(http_request *request)
      {
         bool rtn = true;
         return rtn;
      } // on_response_complete


      void KonectSource::onOneShotFired(uint4 id)
      {
         if(id == retry_id)
         {
            retry_id = 0;
            connect();
         }
      } // onOneShotFired


      void KonectSource::onScheduledEvent(uint4 id)
      {
         if(id == poll_schedule_id && poll_schedule_enabled)
         {
         }
      } // onScheduledEvent


      StrAsc const &KonectSource::get_oauth_access_token()
      {
         if(decrypted_oauth_access_token.length() == 0)
         {
            Csi::LoginManager::decrypt_password(decrypted_oauth_access_token, oauth_access_token);
         }
         return decrypted_oauth_access_token;
      }


      StrAsc const &KonectSource::get_oauth_access_token_secret()
      {
         if(decrypted_oauth_access_token_secret.length() == 0)
         {
            Csi::LoginManager::decrypt_password(decrypted_oauth_access_token_secret, oauth_access_token_secret);
         }
         return decrypted_oauth_access_token_secret;
      }


      StrAsc const &KonectSource::get_signing_key()
      {
         if(signing_key.length() == 0)
         {
            Csi::OStrAscStream enc_consumer_secret;
            Csi::Uri::encode(enc_consumer_secret, consumer_secret);
            signing_key.append(enc_consumer_secret.str());
            
            signing_key.append("&");
            
            StrAsc decrypted_secret;
            Csi::LoginManager::decrypt_password(decrypted_secret, oauth_access_token_secret);
            Csi::OStrAscStream encoded_access_token_secret;
            Csi::Uri::encode(encoded_access_token_secret, decrypted_secret);
            signing_key.append(encoded_access_token_secret.str());
         }
         return signing_key;
      }


      void KonectSource::remove_cursor(cursor_type *cursor)
      {
         cursors_type::iterator ci(
            std::find_if(cursors.begin(), cursors.end(), Csi::HasSharedPtr<cursor_type>(cursor)));
         if(ci != cursors.end())
            cursors.erase(ci);
      } // remove_cursor


      namespace KonectSourceHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Cursor definitions
         ////////////////////////////////////////////////////////////
         Cursor::Cursor(KonectSource *source_, request_handle &request):
            source(source_),
            previously_polled(false)
         {
            requests.push_back(request);
         } // constructor


         Cursor::~Cursor()
         {
            requests.clear();
         } // destructor


         bool Cursor::add_request(request_handle &request)
         {
            // we will allow the combination of like requests provided that this cursor has not yet
            // been polled.
            bool rtn(false);
            if(!requests.empty() && record_desc == 0)
            {
               try
               {
                  request_handle &first(requests.front());
                  Csi::PolySharedPtr<WartBase, KonectWart> first_wart(first->get_wart());
                  Csi::PolySharedPtr<WartBase, KonectWart> request_wart(request->get_wart());
                  if(first_wart->table_name == request_wart->table_name &&
                     first->is_compatible(*request))
                  {
                     requests.push_back(request);
                     rtn = true;
                  }
               }
               catch(std::exception &)
               {
                  rtn = false;
               }
            }
            return rtn;
         } // add_request


         bool Cursor::remove_request(request_handle &request)
         {
            bool rtn(false);
            requests_type::iterator ri(
               std::find(requests.begin(), requests.end(), request));
            if(ri != requests.end())
            {
               rtn = true;
               requests.erase(ri);
               if(requests.empty() && http_request != 0)
               {
                  source->get_connection()->remove_request(http_request.get_rep());
                  http_request.clear();
               }
            }
            return rtn;
         } // remove_request


         bool Cursor::has_no_requests()
         {
            return requests.empty();
         }


         void Cursor::poll()
         {
            if(table_guid.length() == 0)
            {
            Csi::PolySharedPtr<SymbolBase, KonectSourceSymbol> konect_symbol(source->get_source_symbol());
               if(konect_symbol.get_rep())
               {
                  if(!requests.empty())
                  {
                     request_handle &request(requests.front());
                     Csi::PolySharedPtr<WartBase, KonectWart> wart(request->get_wart());
                     StrUni table_key;
                     table_key.append(wart->station_name);
                     table_key.append(L".");
                     table_key.append(wart->table_name);
                     konect_symbol->find_guid(table_key, table_guid);
                  }
               }
            }

            if(!requests.empty() && http_request == 0 && source->get_connection() != 0)
            {
               // we need to generate the parameters for the web query 
               bool new_poll(!previously_polled);
               StrAsc query_mode;
               StrAsc query_p1;
               StrAsc query_p2;
               request_handle first(requests.front());

               previously_polled = true;
               switch(first->get_start_option())
               {
                  case Request::start_at_record:
                     //query_mode = "since-record";
                     //if(new_poll)
                     //   temp << first->get_record_no();
                     //else
                     //   temp << last_record_no;
                     //query_p1 = temp.str();
                     break;

                  case Request::start_at_time:
                     //if(new_poll)
                     //{
                     //   query_mode = "since-time";
                     //   format_logger_time(temp, first->get_start_time());
                     //   query_p1 = temp.str();
                     //}
                     //else
                     //{
                     //   query_mode = "since-record";
                     //   temp << last_record_no;
                     //   query_p1 = temp.str();
                     //}
                     break;

                  case Request::start_after_newest:
                  case Request::start_at_newest:
                     //if(first->get_order_option() == Request::order_real_time || new_poll)
                     //{
                     //   query_mode = "most-recent";
                     //   query_p1 = "1";
                     //}
                     //else
                     //{
                     //   query_mode = "since-record";
                     //   temp << last_record_no;
                     //   query_p1 = temp.str();
                     //}
                     break;

                  case Request::start_relative_to_newest:
                     //if(new_poll)
                     //{
                     //   query_mode = "backfill";
                     //   temp << (-first->get_backfill_interval() / Csi::LgrDate::nsecPerSec);
                     //   query_p1 = temp.str();
                     //}
                     //else
                     //{
                     //   query_mode = "since-record";
                     //   temp << last_record_no;
                     //   query_p1 = temp.str();
                     //}
                     break;

                  case Request::start_at_offset_from_newest:
                     //if(new_poll)
                     //{
                     //   query_mode = "most-recent";
                     //   temp << first->get_start_record_offset();
                     //   query_p1 = temp.str();
                     //}
                     //else
                     //{
                     //   query_mode = "since-record";
                     //   temp << last_record_no;
                     //   query_p1 = temp.str();
                     //}
                     break;

                  case Request::start_date_query:
                     //if(new_poll)
                     //{
                     //   query_mode = "date-range";
                     //   format_logger_time(temp, first->get_start_time());
                     //   query_p1 = temp.str();
                     //   temp.str("");
                     //   format_logger_time(temp, first->get_end_time());
                     //   query_p2 = temp.str();
                     //}
                     //else
                     //{
                     //   if(last_time + 1 < first->get_end_time())
                     //   {
                     //      query_mode = "date-range";
                     //      format_logger_time(temp, last_time + 1);
                     //      query_p1 = temp.str();
                     //      temp.str("");
                     //      format_logger_time(temp, first->get_end_time());
                     //      query_p2 = temp.str();
                     //   }
                     //   else
                     //      return;
                     //}
                     break;
               }

               // now that we have determined the parameters and mode for the query, we can launch
               // the query.
               StrAsc data_uri;
               data_uri.append("https://apipreview.konectgds.com/v1/tables/");
               data_uri.append(table_guid.to_utf8());
               data_uri.append("/tabledata");

               Csi::Uri uri(data_uri);
               Csi::PolySharedPtr<WartBase, KonectWart> wart(first->get_wart());

               Csi::OStrAscStream temp;
               temp.imbue(std::locale::classic());
               temp << 1;
               query_p1 = temp.str();

               uri.insert("last", query_p1);
               uri.insert("id", table_guid.to_utf8());

               http_request.bind(new http_request_type(this, uri));
               http_request->set_method(Csi::HttpClient::Request::method_get);
               http_request->set_authorisation(
                  new KonectSourceHelpers::AuthorisationOAuth(
                     source->get_oauth_access_token(), source->get_signing_key()));
               source->get_connection()->add_request(http_request);
            }
         } // poll


         bool Cursor::on_failure(http_request_type *request)
         {
            on_failure(SinkBase::sink_failure_connection_failed);
            return false;
         } // on_failure


         bool Cursor::on_response_complete(http_request_type *request)
         {
            int failure(-1);
            bool more_data(false);
            try
            {
               if(request->get_response_code() == 200)
               {
                  // we now need to parse the response as a JSON structure
                  Csi::IByteQueueStream input(&request->get_receive_buff());
                  Csi::Json::Object response;
                  response.parse(input);
#ifdef _DEBUG
                  Csi::OStrAscStream temp;
                  response.format(temp);
                  OutputDebugStringA("\r\n\r\n");
                  OutputDebugStringA(temp.str().c_str());
                  OutputDebugStringA("\r\n\r\n");
#endif //_DEBUG

                  Csi::SharedPtr<Broker::ValueFactory> value_factory(source->get_manager()->get_value_factory());
                  Csi::Json::ArrayHandle items(response["Items"]);
                  cache_type records;
                  Csi::Json::Array::iterator item_it = items->begin();
                  while(item_it != items->end())
                  {
                     if(record_desc == 0)
                     {
                        //We need to create the record description since we don't have one yet
                        if(!requests.empty())
                        {
                           try
                           {
                              request_handle &first(requests.front());
                              Csi::PolySharedPtr<WartBase, KonectWart> wart(first->get_wart());
                              record_desc.bind(
                                 new Broker::RecordDesc(
                                 wart->station_name,
                                 wart->table_name));
                              Csi::Json::ObjectHandle item(*item_it);
                              Csi::Json::ArrayHandle data((*item)["data"]);
                              Csi::Json::Array::iterator data_it = data->begin();
                              while(data_it != data->end())
                              {
                                 Csi::Json::ObjectHandle field_data(*data_it);
                                 Csi::Json::ObjectHandle tablefield((*field_data)["tablefield"]);
                                 Csi::Json::String fieldname = tablefield->get_property_str("fieldname");
                                 Csi::Json::String fieldtype = tablefield->get_property_str("fieldtype");
                                 Broker::ValueName value_name(fieldname.get_value());
                                 Broker::RecordDesc::value_type value_desc(new Broker::ValueDesc);
                                 value_desc->name = value_name.get_column_name();
                                 std::copy(
                                    value_name.begin(),
                                    value_name.end(),
                                    std::back_inserter(value_desc->array_address));
                                 value_desc->data_type = json_type_to_csi(fieldtype.get_value());
                                 value_desc->modifying_cmd = 0;
                                 record_desc->values.push_back(value_desc);
                                 ++data_it;
                              }
                           }
                           catch(std::exception &)
                           { }
                        }

                        record_handle record;
                        record.bind(new Broker::Record(record_desc, *value_factory));
                        cache.push_back(record);
                        for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                        {
                           request_handle &request(*ri);
                           Csi::PolySharedPtr<WartBase, KonectWart> uri(request->get_wart());
                           request->set_value_indices(*record, uri->column_name);
                           request->set_state(source, Request::state_started);
                           request->get_sink()->on_sink_ready(source->get_manager(), request, record);
                           if(!Cursor::is_valid_instance(this))
                              return false;
                        }
                     }

                     Csi::Json::ObjectHandle item(*item_it);
                     Csi::Json::String ts = item->get_property_str("timestamp");
                     Csi::LgrDate time(Csi::LgrDate::fromStr(ts.get_value().c_str()));
                     if(time != last_time)
                     {
                        record_handle record;
                        if(!cache.empty())
                        {
                           record = cache.front();
                           cache.pop_front();
                        }
                        else
                        {
                           record.bind(new Broker::Record(record_desc, *value_factory));
                        }

                        record->set_stamp(time);
                        Broker::Record::iterator vi(record->begin());
                        Csi::Json::ArrayHandle data((*item)["data"]);
                        Csi::Json::Array::iterator data_it = data->begin();
                        while(data_it != data->end() && vi != record->end())
                        {
                           Broker::Record::value_type &value(*vi);
                           Csi::Json::ObjectHandle field_data(*data_it);
                           Csi::Json::ObjectHandle tablefield((*field_data)["tablefield"]);
                           value->read_json((*field_data)["value"].get_rep());
                           ++data_it;
                           ++vi;
                        }
                        if(vi != record->end() || data_it != data->end())
                           throw SinkBase::sink_failure_invalid_column_name;
                        records.push_back(record);
                        last_time = time;
                     }
                     ++item_it;
                  }
                  if(!records.empty())
                  {
                     std::copy(records.begin(), records.end(), std::back_inserter(cache));
                     SinkBase::report_sink_records(
                        source->get_manager(), requests, records);
                  }
               }
               else
               {
                  switch(request->get_response_code())
                  {
                     case 401:
                        failure = SinkBase::sink_failure_invalid_logon;
                        break;

                     default:
                        failure = SinkBase::sink_failure_unknown;
                        break;
                  }
               }
            }
            catch(SinkBase::sink_failure_type code)
            {
               failure = code;
            }
            catch(std::exception &)
            {
               failure = SinkBase::sink_failure_unknown;
            }

            // we will need to report any failure that occurred
            if(Cursor::is_valid_instance(this))
            {
               // if the datalogger reported that there is more data that will satisfy the
               // conditions, we will immediately launch another query.
               http_request.clear();
               if(failure >= SinkBase::sink_failure_unknown)
                  on_failure(static_cast<SinkBase::sink_failure_type>(failure));
               else if(more_data)
                  poll();
            }
            return true;
         } // on_response_complete


         void Cursor::on_failure(SinkBase::sink_failure_type failure)
         {
            KonectSource *source(this->source);
            requests_type requests(this->requests);

            this->requests.clear();
            while(!requests.empty())
            {
               request_handle request(requests.front());
               requests.pop_front();
               if(SinkBase::is_valid_instance(request->get_sink()))
                  request->get_sink()->on_sink_failure(source->get_manager(), request, failure);
            }
            source->remove_cursor(this);
         } // on_failure
      };
   };
};
