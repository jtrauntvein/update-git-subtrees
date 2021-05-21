/* Csi.SoftwareLicense.cpp

   Copyright (C) 2017, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 19 July 2017
   Last Change: Monday 03 August 2020
   Last Commit: $Date: 2020-08-03 14:06:48 -0600 (Mon, 03 Aug 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SoftwareLicense.h"
#include "Csi.BuffStream.h"
#include "Csi.StrAscStream.h"
#include "Csi.RegistryManager.h"
#include "Csi.Utils.h"
#include "Csi.Base32.h"
#include "Csi.Html.Table.h"
#include "Csi.Html.Text.h"
#include "Csi.Html.List.h"
#include "Packet.h"
#include "coratools.strings.h"
#include "boost/format.hpp"
#include <iomanip>
#include <algorithm>


namespace Csi
{
   namespace SoftwareLicense
   {
      namespace
      {
         StrAsc const model_name("model");
         StrAsc const version_name("version");
         StrAsc const serial_no_name("serial_no");
         StrAsc const date_issued_name("date_issued");
         StrAsc const date_active_name("date_active");
         StrAsc const renew_interval_name("renew_interval");
         StrAsc const machine_id_name("machine_id");
         StrAsc const other_licenses_name("other_licenses");
         StrAsc const license_key_private("Iza");
         

         /**
          * Defines the event used to report that certificate renewal has started
          */
         class event_renewal_started: public Event
         {
         public:
            /**
             * Specifies the id for this event type.
             */
            static uint4 const event_id;

            /**
             * posts an instance of this event.
             */
            static void cpost(LicenseClient *client)
            {
               (new event_renewal_started(client))->post();
            }

         private:
            event_renewal_started(LicenseClient *client):
               Event(event_id, client)
            { }
         };


         uint4 const event_renewal_started::event_id(Event::registerType("Csi::SoftwareLicense::event_renewal_started"));


         /**
          * Defines an event type that will be posted when renwal has completed.
          */
         class event_renewal_complete: public Event
         {
         public:
            /**
             * Specifies the identifier for this event.
             */
            static uint4 const event_id;

            /**
             * Specifies the outcome for the operation
             */
            LicenseClientDelegate::renewal_outcome_type outcome;

            /**
             * Posts this event for asynchronous processing.
             */
            static void cpost(LicenseClient *sender, LicenseClientDelegate::renewal_outcome_type outcome)
            {
               (new event_renewal_complete(sender, outcome))->post();
            }

         private:
            event_renewal_complete(LicenseClient *sender, LicenseClientDelegate::renewal_outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const event_renewal_complete::event_id(Event::registerType("Csi::SoftwareLicense::event_renewal_complete"));


         /**
          * Defines the type of event that is thrown when the certificate is changed.
          */
         class event_certificate_changed: public Csi::Event
         {
         public:
            /**
             * Identifies this event type.
             */
            static uint4 const event_id;

            /**
             * creates and posts this event type.
             */
            static void cpost(LicenseClient *sender)
            {
               (new event_certificate_changed(sender))->post();
            }

         private:
            event_certificate_changed(LicenseClient *sender):
               Event(event_id, sender)
            { }
         };


         uint4 const event_certificate_changed::event_id(Event::registerType("Csi::SoftwareLicense::event_certificate_changed"));


         /**
          * Defines an operation that carries on the software license renewal process.
          */
         class RenewalOp: public LicenseClientOpBase
         {
         private:
            /**
             * Specifies the HTTP request used to communicate with the license server.
             */
            SharedPtr<HttpClient::Request> request;

            /**
             * Specifies the connection used to carry the request.
             */
            connection_handle connection;
            
         public:
            /**
             * Constructor
             *
             * @param owner_ Specifies the license client that owns this operation.
             */
            RenewalOp(LicenseClient *owner_):
               LicenseClientOpBase(owner_)
            { }

            /**
             * Destructor
             */
            virtual ~RenewalOp()
            {
               request.clear();
               connection.clear();
            }

            /**
             * Overloads the base class version to start this operation.
             */
            virtual void start(connection_handle connection_)
            {
               try
               {
                  // we need to format the object for the request body.
                  Json::Object signed_request(owner->make_renewal_request(true));
                  request.bind(
                     new HttpClient::Request(
                        this, owner->get_renew_uri(), HttpClient::Request::method_post, false));
                  request->set_content_type("application/json");
                  
                  // we need to now generate the HTTP request.
                  HttpClient::ORequestStream body(request.get_rep());
                  connection = connection_;
                  signed_request.format(body);
                  body.flush();
                  request->add_bytes("", 0, true);
                  connection->add_request(request);
                  event_renewal_started::cpost(owner);
               }
               catch(std::exception &)
               {
                  event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_failure_communication);
                  owner->on_operation_complete(this);
               }
            }

            /**
             * Overloads the base class version to handle a failure of the http request.
             */
            bool on_failure(HttpClient::Request *sender)
            {
               event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_failure_communication);
               owner->on_operation_complete(this);
               return true;
            }

            /**
             * Overloads the base class version the handle the event where the http request has been
             * returned.
             */
            virtual bool on_response_complete(HttpClient::Request *sender)
            {
               try
               {
                  // we have to ensure that the request succeeded
                  if(request->get_response_code() != 200)
                     event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_failure_communication);

                  // now we can attempt to parse the response
                  HttpClient::IRequestStream envelope_str(sender);
                  SignedJson envelope(envelope_str);
                  IBuffStream response_str(envelope.get_content().getContents(), envelope.get_content().length());
                  Json::Object response;
                  int4 reported_outcome;
                  Json::ObjectHandle certificate_json;
                  LicenseClient::certificate_handle certificate;

                  response.parse(response_str);
                  reported_outcome = (int4)response.get_property_number("outcome");
                  switch(reported_outcome)
                  {
                  case 1:
                     certificate.bind(new Certificate);
                     certificate_json = response.get_property("certificate");
                     certificate->read(*certificate_json);
                     event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_success);
                     owner->set_certificate(certificate);
                     break;

                  case 2:
                     event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_failure_unknown_product);
                     break;
                     
                  case 3:
                     owner->set_certificate(0);
                     event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_failure_disabled);
                     break;

                  case 4:
                     owner->set_certificate(0);
                     event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_failure_trial_expired);
                     break;
                     
                  default:
                     event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_failure_unknown);
                     break;
                  }
                  
               }
               catch(std::exception &)
               {
                  event_renewal_complete::cpost(owner, LicenseClientDelegate::renewal_outcome_failure_communication);
               }
               owner->on_operation_complete(this);
               return true;
            }
         };


         /**
          * Defines the type of event that will get posted when a register add-on operation is
          * complete.
          */
         class event_register_addon_complete: public Event
         {
         public:
            /**
             * Identifies this event type.
             */
            static uint4 const event_id;

            /**
             * Specifies the outcome of the operation.
             */
            typedef LicenseClientDelegate::register_addon_outcome_type outcome_type;
            outcome_type const outcome;
            
            /**
             * Creates and posts this event type.
             */
            static void cpost(LicenseClient *sender, outcome_type outcome)
            {
               (new event_register_addon_complete(sender, outcome))->post();
            }

         private:
            event_register_addon_complete(LicenseClient *sender, outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const event_register_addon_complete::event_id(
            Event::registerType("Csi::SoftwareLicense::event_register_addon_complete"));


         /**
          * Defines an operation that will register an add-on to a parent product license.
          */
         class RegisterAddonOp: public LicenseClientOpBase
         {
         private:
            /**
             * Specifies the HTTP request sent for this operation.
             */
            SharedPtr<HttpClient::Request> request;

            /**
             * Specifies the connection used to send the request.
             */
            connection_handle connection;

            /**
             * Specifies the child model.
             */
            StrAsc const child_model;

            /**
             * Specifies the child serial number.
             */
            StrAsc const child_serial_no;
            
         public:
            /**
             * Constructor
             *
             * @param owner_ Specifies the owner for this operation.
             *
             * @param child_model_ Specifies the child model number.
             *
             * @param child_serial_no_ Specifies the child serial number.
             */
            RegisterAddonOp(LicenseClient *owner_, StrAsc const &child_model_, StrAsc const &child_serial_no_):
               LicenseClientOpBase(owner_),
               child_model(child_model_),
               child_serial_no(child_serial_no_)
            { }

            /**
             * Overloads the base class version to start the HTTP request.
             */
            virtual void start(connection_handle connection_)
            {
               try
               {
                  // we need to format the JSON object for the request body.
                  Json::Object addon_request;
                  OStrAscStream formatted_request;
                  Json::Object signed_request;
                  
                  addon_request.set_property_str("parent_model", owner->get_model());
                  addon_request.set_property_str("parent_version", owner->get_version().to_utf8(false));
                  addon_request.set_property_str("parent_serial_no", owner->get_serial_no());
                  addon_request.set_property_str("child_model", child_model);
                  addon_request.set_property_str("child_serial_no", child_serial_no);
                  addon_request.format(formatted_request);
                  SignedJson::generate_envelope(signed_request, formatted_request.c_str(), formatted_request.length());
                  request.bind(new HttpClient::Request(this, owner->get_register_addon_uri(), HttpClient::Request::method_post, false));
                  request->set_content_type("application/json");
                  
                  //  we need to now send the HTTP request
                  HttpClient::ORequestStream body(request.get_rep());
                  connection = connection_;
                  signed_request.format(body);
                  body.flush();
                  request->add_bytes("", 0, true);
                  connection->add_log_comment(formatted_request.str());
                  connection->add_request(request);
               }
               catch(std::exception &)
               {
                  event_register_addon_complete::cpost(owner, LicenseClientDelegate::register_addon_outcome_failure_communication);
                  owner->on_operation_complete(this);
               }
            }


            /**
             * Overloads the failure handler.
             */
            virtual bool on_failure(HttpClient::Request *sender)
            {
               event_register_addon_complete::cpost(owner, LicenseClientDelegate::register_addon_outcome_failure_communication);
               owner->on_operation_complete(this);
               return true;
            }

            /**
             * Overloads the base class version to handle the completion of the request.
             */
            virtual bool on_response_complete(HttpClient::Request *sender)
            {
               LicenseClientDelegate::register_addon_outcome_type outcome(LicenseClientDelegate::register_addon_outcome_failure_unknown);

               try
               {
                  if(request->get_response_code() == 200)
                  {
                     HttpClient::IRequestStream envelope_str(sender);
                     SignedJson envelope(envelope_str);
                     IBuffStream response_str(envelope.get_content().getContents(), envelope.get_content().length());
                     Json::Object response;
                     int4 reported_outcome;

                     response.parse(response_str);
                     reported_outcome = (int4)response.get_property_number("outcome");
                     switch(reported_outcome)
                     {
                     case 1:
                        outcome = LicenseClientDelegate::register_addon_outcome_success;
                        owner->force_renewal();
                        break;

                     case 2:
                        outcome = LicenseClientDelegate::register_addon_outcome_failure_no_parent_product;
                        break;

                     case 3:
                        outcome = LicenseClientDelegate::register_addon_outcome_failure_child_already_added;
                        break;

                     case 4:
                        outcome = LicenseClientDelegate::register_addon_outcome_failure_no_child_product;
                        break;

                     case 5:
                        outcome = LicenseClientDelegate::register_addon_outcome_failure_disabled;
                        break;
                     }
                  }
                  else
                     outcome = LicenseClientDelegate::register_addon_outcome_failure_communication;
               }
               catch(std::exception &)
               {
                  outcome = LicenseClientDelegate::register_addon_outcome_failure_communication;
               }
               event_register_addon_complete::cpost(owner, outcome);
               owner->on_operation_complete(this);
               return true;
            } // on_response_complete
         };


         /**
          * Defines an event that is posted when the check version operation is complete.
          */
         class event_check_version_complete: public Csi::Event
         {
         public:
            /**
             * Specifies the identifier for this event.
             */
            static uint4 const event_id;

            /**
             * Specifies the outcome.
             */
            LicenseClientDelegate::check_version_outcome_type outcome;

            /**
             * Specifies the version that was reported.
             */
            Json::ArrayHandle versions;

            /**
             * Creates and posts an event object.
             */
            static void cpost(
               LicenseClient *client,
               LicenseClientDelegate::check_version_outcome_type outcome,
               Json::ArrayHandle versions = 0)
            {
               (new event_check_version_complete(client, outcome, versions))->post();
            }

         private:
            event_check_version_complete(
               LicenseClient *client,
               LicenseClientDelegate::check_version_outcome_type outcome_,
               Json::ArrayHandle &versions_):
               Event(event_id, client),
               outcome(outcome_),
               versions(versions_)
            { }
         };


         uint4 const event_check_version_complete::event_id(
            Event::registerType("Csi::SoftwareLicense::event_check_version_complete"));


         /**
          * Defines a predicate used to sort versions so that newer versions come first.
          */
         struct version_less
         {
            bool operator ()(Json::Array::value_type const &v1, Json::Array::value_type const &v2)
            {
               Csi::Json::ObjectHandle version1_json(v1);
               Csi::Json::ObjectHandle version2_json(v2);
               StrAsc v1_str(version1_json->get_property_str("version"));
               StrAsc v2_str(version2_json->get_property_str("version"));
               VersionNumber version1(v1_str.c_str());
               VersionNumber version2(v2_str.c_str());
               return version1 > version2;
            }
         };
         

         /**
          * Defines an operation object that will carry out the check version request to the license
          * server.
          */
         class CheckVersionOp: public LicenseClientOpBase
         {
         private:
            /**
             * Specifies the HTTP request.
             */
            SharedPtr<HttpClient::Request> request;

            /**
             * Specifies the connection used to send the request.
             */
            connection_handle connection;

         public:
            /**
             * Constructor
             *
             * @param owner_ Specifies the license client object that owns this operation.
             */
            CheckVersionOp(LicenseClient *owner_):
               LicenseClientOpBase(owner_)
            { }

            /**
             * Destructor
             */
            virtual ~CheckVersionOp()
            { request.clear(); }

            /**
             * Overloads the base class version to start the HTTP request.
             */
            virtual void start(connection_handle connection_)
            {
               try
               {
                  // we need to generate the body and the request.
                  Json::Object check_version_request;
                  OStrAscStream formatted_request;
                  Json::Object signed_request;
                  
                  check_version_request.set_property_str("model", owner->get_model());
                  check_version_request.format(formatted_request);
                  SignedJson::generate_envelope(signed_request, formatted_request.c_str(), formatted_request.length());
                  request.bind(
                     new HttpClient::Request(
                        this, owner->get_check_version_uri(), HttpClient::Request::method_post, false));
                  request->set_content_type("application/json");
                  
                  // we now need to send the request.
                  HttpClient::ORequestStream body(request.get_rep());
                  connection = connection_;
                  signed_request.format(body);
                  request->add_bytes("", 0, true);
                  connection->add_log_comment(formatted_request.str());
                  connection->add_request(request);
               }
               catch(std::exception &)
               {
                  event_check_version_complete::cpost(owner, LicenseClientDelegate::check_version_outcome_failure_communication);
                  owner->on_operation_complete(this);
               }
            }

            /**
             * Overloads the base class version to handle an HTTP client failure report.
             */
            virtual bool on_failure(HttpClient::Request *sender)
            {
               event_check_version_complete::cpost(owner, LicenseClientDelegate::check_version_outcome_failure_communication);
               owner->on_operation_complete(this);
               return true;
            }

            /**
             * Overloads the base class version to handle the completion of the request.
             */
            virtual bool on_response_complete(HttpClient::Request *sender)
            {
               LicenseClientDelegate::check_version_outcome_type outcome(LicenseClientDelegate::check_version_outcome_failure_unknown);
               Json::ArrayHandle versions;
               try
               {
                  if(request->get_response_code() == 200)
                  {
                     HttpClient::IRequestStream envelope_str(sender);
                     SignedJson envelope(envelope_str);
                     IBuffStream response_str(envelope.get_content().getContents(), envelope.get_content().length());
                     Json::Object response;
                     
                     response.parse(response_str);
                     versions = response.get_property("versions");
                     std::sort(versions->begin(), versions->end(), version_less());
                     outcome = LicenseClientDelegate::check_version_outcome_success;
                  }
               }
               catch(std::exception &)
               {
                  outcome = LicenseClientDelegate::check_version_outcome_failure_communication;
               }
               event_check_version_complete::cpost(owner, outcome, versions);
               owner->on_operation_complete(this);
               return true;
            }
         };


         /**
          * Defines an event object that will report when a changelog operation is complete.
          */
         class event_changelog_complete: public Csi::Event
         {
         public:
            /**
             * Specifies the unique identifier for this event type.
             */
            static uint4 const event_id;

            /**
             * Specifies the outcome.
             */
            LicenseClientDelegate::changelog_outcome_type outcome;

            /**
             * Specifies the change log structure.
             */
            Json::ObjectHandle change_log;

            /**
             * Creates and posts this event.
             */
            static void cpost(
               LicenseClient *sender, LicenseClientDelegate::changelog_outcome_type outcome, Json::ObjectHandle change_log = 0)
            {
               (new event_changelog_complete(sender, outcome, change_log))->post();
            }

         private:
            event_changelog_complete(
               LicenseClient *sender, LicenseClientDelegate::changelog_outcome_type outcome_, Json::ObjectHandle &change_log_):
               Event(event_id, sender),
               outcome(outcome_),
               change_log(change_log_)
            { }
         };
         uint4 const event_changelog_complete::event_id(
            Event::registerType("Csi::SoftwareLicense::event_changelog_complete"));


         /**
          * Defines an operation that can be used to retrieve the change log for a given version.
          */
         class ChangeLogOp: public LicenseClientOpBase
         {
         private:
            /**
             * Specifies the HTTP request.
             */
            SharedPtr<HttpClient::Request> request;

            /**
             * Specifies the connection used to send the request.
             */
            connection_handle connection;

            /**
             * Specifies the version to check.
             */
            VersionNumber version;
            
         public:
            /**
             * Constructor
             *
             * @param owner_ Specifies the license client that owns this operation.
             *
             * @param version_ Specifies the version to check.
             */
            ChangeLogOp(LicenseClient *owner_, VersionNumber const &version_):
               LicenseClientOpBase(owner_),
               version(version_)
            { }

            /**
             * Destructor
             */
            virtual ~ChangeLogOp()
            { request.clear(); }

            /**
             * Overloads the base class version to start the web request.
             */
            virtual void start(connection_handle connection_)
            {
               try
               {
                  Uri uri(owner->get_changelog_uri());
                  uri.insert("model", owner->get_model());
                  uri.insert("version", version.to_utf8());
                  request.bind(new HttpClient::Request(this, uri, HttpClient::Request::method_get));
                  connection = connection_;
                  connection->add_request(request);
               }
               catch(std::exception &)
               {
                  event_changelog_complete::cpost(owner, LicenseClientDelegate::changelog_outcome_failure_communication);
                  owner->on_operation_complete(this);
               }
            } // start

            /**
             * Overloads the failure handler.
             */
            virtual bool on_failure(HttpClient::Request *sender)
            {
               LicenseClientDelegate::changelog_outcome_type outcome(LicenseClientDelegate::changelog_outcome_failure_communication);
               if(sender->get_response_code() == 404)
                  outcome = LicenseClientDelegate::changelog_outcome_failure_no_product;
               event_changelog_complete::cpost(owner, outcome);
               owner->on_operation_complete(this);
               return true;
            }

            /**
             * Overloads the base class version to handle the completion of the request.
             */
            virtual bool on_response_complete(HttpClient::Request *sender)
            {
               LicenseClientDelegate::changelog_outcome_type outcome(LicenseClientDelegate::changelog_outcome_failure_unknown);
               Json::ObjectHandle change_log;

               try
               {
                  HttpClient::IRequestStream response_str(sender);
                  change_log.bind(new Json::Object);
                  change_log->parse(response_str);
                  outcome = LicenseClientDelegate::changelog_outcome_success;
               }
               catch(std::exception &)
               {
                  outcome = LicenseClientDelegate::changelog_outcome_failure_communication;
                  change_log.clear();
               }
               event_changelog_complete::cpost(owner, outcome, change_log);
               owner->on_operation_complete(this);
               return true;
            } // on_response_complete
         };
         

         /**
          * Defines an event object that will report that an update operation is complete.
          */
         class event_update_complete: public Csi::Event
         {
         public:
            /**
             * Identifies this event type.
             */
            static uint4 const event_id;

            /**
             * Specifies the outcome code.
             */
            LicenseClientDelegate::update_outcome_type outcome;

            /**
             * creates an posts an event of this type.
             */
            static void cpost(LicenseClient *sender, LicenseClientDelegate::update_outcome_type outcome)
            {
               (new event_update_complete(sender, outcome))->post();
            }

         private:
            event_update_complete(LicenseClient *sender, LicenseClientDelegate::update_outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const event_update_complete::event_id(Event::registerType("Csi::SoftwareLicense::event_update_comnplete"));


         /**
          * Defines an object that carries out the update operation for the license client.
          */
         class UpdateOp: public LicenseClientOpBase
         {
         private:
            /**
             * Specifies the HTTP request.
             */
            SharedPtr<HttpClient::Request> request;

            /**
             * Specifies the HTTP connection used to send the request.
             */
            connection_handle connection;

            /**
             * Specifies the JSON object that contains the model information to send.
             */
            Json::ObjectHandle version;
            
         public:
            /**
             * Constructor
             *
             * @param owner_ Specifies the license client component that owns this operation.
             *
             * @param version_ Specifies the version information for the update to be retrieved.
             */
            UpdateOp(LicenseClient *owner_, Json::ObjectHandle &version_):
               LicenseClientOpBase(owner_),
               version(version_)
            { }

            /**
             * Destructor
             */
            virtual ~UpdateOp()
            {
               request.clear();
               connection.clear();
            }

            /**
             * Overloads the base class version to start the HTTP request.
             */
            virtual void start(connection_handle connection_)
            {
               try
               {
                  Json::Object request_json;
                  OStrAscStream formatted_request;
                  Json::Object signed_request;
                  
                  request_json.set_property_str("model", version->get_property_str("model"));
                  request_json.set_property_str("version", version->get_property_str("version"));
                  request_json.set_property_str("build", version->get_property_str("build"));
                  request_json.set_property_str("serial_no", owner->get_serial_no());
                  request_json.set_property_str("machine_id", owner->get_machine_id());
                  request_json.format(formatted_request);
                  SignedJson::generate_envelope(signed_request, formatted_request.c_str(), formatted_request.length());
                  request.bind(
                     new HttpClient::Request(
                        this, owner->get_update_uri(), HttpClient::Request::method_post, false));
                  request->set_content_type("application/json");
                  
                  // we now need to send the request.
                  HttpClient::ORequestStream body(request.get_rep());
                  connection = connection_;
                  signed_request.format(body);
                  request->add_bytes("", 0, true);
                  connection->add_log_comment(formatted_request.str());
                  connection->add_request(request);
               }
               catch(std::exception &)
               {
                  event_update_complete::cpost(owner, LicenseClientDelegate::update_outcome_failure_communication);
                  owner->on_operation_complete(this);
               }
            }

            /**
             * Overloads the base class version to handle an HTTP client failure.
             */
            virtual bool on_failure(HttpClient::Request *sender)
            {
               LicenseClientDelegate::update_outcome_type outcome(LicenseClientDelegate::update_outcome_failure_communication);
               try
               {
                  if(request->get_response_code() == 403)
                  {
                     HttpClient::IRequestStream response_str(sender);
                     Json::Object response;
                     int4 reported_outcome;
                     
                     response.parse(response_str);
                     reported_outcome = static_cast<int4>(response.get_property_number("outcome"));
                     switch(reported_outcome)
                     {
                     case 1:
                        outcome = LicenseClientDelegate::update_outcome_failure_no_product;
                        break;

                     case 2:
                        outcome = LicenseClientDelegate::update_outcome_failure_license_expired;
                        break;

                     case 3:
                        outcome = LicenseClientDelegate::update_outcome_failure_license_disabled;
                        break;

                     case 4:
                        outcome = LicenseClientDelegate::update_outcome_failure_needs_maintenance;
                        break;

                     case 5:
                        outcome = LicenseClientDelegate::update_outcome_failure_maintenance_expired;
                        break;
                     }
                  }
               }
               catch(std::exception &)
               { }
               event_update_complete::cpost(owner, outcome);
               owner->on_operation_complete(this);
               return true;
            }

            /**
             * Overloads the base class version to handle the completion of the request.
             */
            virtual bool on_response_complete(HttpClient::Request *sender)
            {
               LicenseClientDelegate::update_outcome_type outcome(LicenseClientDelegate::update_outcome_failure_unknown);
               try
               {
                  if(request->get_response_code() == 200)
                     outcome = LicenseClientDelegate::update_outcome_success;
               }
               catch(std::exception &)
               { outcome = LicenseClientDelegate::update_outcome_failure_communication; }
               event_update_complete::cpost(owner, outcome);
               owner->on_operation_complete(this);
               return true;
            }

            /**
             * Overloads the base class version to handle fragments of received data.
             */
            virtual void on_response_data(HttpClient::Request *sender)
            {
               if(sender->get_response_code() == 200 && LicenseClientDelegate::is_valid_instance(owner->get_delegate()))
               {
                  if(!owner->get_delegate()->on_update_data(owner, *sender))
                  {
                     event_update_complete::cpost(owner, LicenseClientDelegate::update_outcome_failure_aborted);
                     owner->on_operation_complete(this);
                  }
               }
            }
         };
      };

      
      Certificate::Certificate():
         renew_interval(-1),
         was_signed(false)
      { }


      Certificate::Certificate(Certificate const &other):
         model(other.model),
         version(other.version),
         serial_no(other.serial_no),
         date_issued(other.date_issued),
         date_active(other.date_active),
         renew_interval(other.renew_interval),
         machine_id(other.machine_id),
         was_signed(other.was_signed)
      {
         for(other_licenses_type::const_iterator li = other.other_licenses.begin(); li != other.other_licenses.end(); ++li)
            other_licenses.push_back(new Certificate(**li));
      } // copy constructor


      Certificate &Certificate::operator =(Certificate const &other)
      {
         model = other.model;
         version = other.version;
         serial_no = other.serial_no;
         date_issued = other.date_issued;
         date_active = other.date_active;
         renew_interval = other.renew_interval;
         machine_id = other.machine_id;
         was_signed = other.was_signed;
         other_licenses.clear();
         for(other_licenses_type::const_iterator li = other.other_licenses.begin(); li != other.other_licenses.end(); ++li)
            other_licenses.push_back(new Certificate(**li));
         return *this;
      } // copy operator


      Certificate::~Certificate()
      {
         other_licenses.clear();
      } // destructor


      void Certificate::read(Csi::Json::Object &license_json, bool require_signed)
      {
         if(license_json.has_property("signature") && license_json.has_property("signature_alg"))
         {
            SignedJson signed_cert(license_json);
            Csi::IBuffStream input(signed_cert.get_content().getContents(), signed_cert.get_content().length());
            Json::Object decoded;
            decoded.parse(input);
            read(decoded);
            was_signed = true;
         }
         else
         {
            if(require_signed)
               throw std::invalid_argument("signed certificate is required");
            was_signed = false;
            model = license_json.get_property_str(model_name);
            version = license_json.get_property_str(version_name).c_str();
            serial_no = license_json.get_property_str(serial_no_name);
            date_issued = license_json.get_property_date(date_issued_name);
            date_active = license_json.get_property_date(date_active_name);
            renew_interval = (int8)license_json.get_property_number(renew_interval_name);
            machine_id = license_json.get_property_str(machine_id_name);
            other_licenses.clear();
            if(license_json.has_property(other_licenses_name))
            {
               Json::ArrayHandle children(license_json.get_property(other_licenses_name));
               for(Json::Array::iterator ci = children->begin(); ci != children->end(); ++ci)
               {
                  Json::ObjectHandle child_json(*ci);
                  value_type child(new Certificate);
                  child->read(*child_json);
                  other_licenses.push_back(child);
               }
            }
         }
      } // read


      void Certificate::write(Csi::Json::Object &license_json, bool sign) const
      {
         Csi::OStrAscStream temp;
         temp.imbue(StringLoader::make_locale(0));
         if(sign)
         {
            Json::Object decoded;
            write(decoded, false);
            decoded.format(temp);
            SignedJson::generate_envelope(license_json, temp.c_str(), temp.length());
         }
         else
         {
            Json::ArrayHandle other_licenses_json(new Json::Array);
            license_json.set_property_str(model_name, model);
            temp.str("");
            version.format(temp, false);
            license_json.set_property_str(version_name, temp.str());
            license_json.set_property_str(serial_no_name, serial_no);
            license_json.set_property_date(date_issued_name, date_issued);
            license_json.set_property_date(date_active_name, date_active);
            license_json.set_property_number(renew_interval_name, (double)renew_interval);
            license_json.set_property_str(machine_id_name, machine_id);
            license_json.set_property(other_licenses_name, other_licenses_json.get_handle());
            for(const_iterator li = other_licenses.begin(); li != other_licenses.end(); ++li)
            {
               value_type const &other(*li);
               Json::ObjectHandle other_json(new Json::Object);
               other->write(*other_json, false);
               other_licenses_json->push_back(other_json.get_handle());
            }
         }
      } // write


      bool Certificate::needs_renewal() const
      {
         bool rtn(renew_interval <= 0);
         if(!rtn)
         {
            LgrDate now(LgrDate::gmt());
            if(date_issued + renew_interval * LgrDate::nsecPerSec < now)
               rtn = true;
         }
         return rtn;
      } // needs_renewal


      void Certificate::format_certificate(Html::Tag &tag, license_names_type const &license_names) const
      {
         using namespace Html;
         using namespace SoftwareLicenseStrings;
         PolySharedPtr<Tag, Table> license_info_table(new Table);
         OStrAscStream temp;
         license_names_type::const_iterator model_it(license_names.find(model));
         
         temp.imbue(StringLoader::make_locale());
         tag.add_tag(license_info_table.get_handle());
         license_info_table->add_tag(new Text(my_strings[strid_license_product], "b"));
         license_info_table->add_last_cell_attribute("align", "right");
         if(model_it != license_names.end())
            license_info_table->add_tag(new Text(model_it->second));
         else
            license_info_table->add_tag(new Text(model));

         license_info_table->add_row();
         license_info_table->add_tag(new Text(my_strings[strid_license_version], "b"));
         license_info_table->add_last_cell_attribute("align", "right");
         version.format(temp, true);
         license_info_table->add_tag(new Text(temp.str()));

         license_info_table->add_row();
         license_info_table->add_tag(new Text(my_strings[strid_license_serial_no], "b"));
         license_info_table->add_last_cell_attribute("align", "right");
         license_info_table->add_tag(new Text(serial_no));

         license_info_table->add_row();
         license_info_table->add_tag(new Text(my_strings[strid_license_activation_date], "b"));
         license_info_table->add_last_cell_attribute("align", "right");
         temp.str("");
         date_active.format(temp, "%n");
         license_info_table->add_tag(new Text(temp.str()));

         license_info_table->add_row();
         license_info_table->add_tag(new Text(my_strings[strid_license_renewal_status], "b"));
         license_info_table->add_last_cell_attribute("align", "right");
         if(renew_interval > 0)
         {
            LgrDate now(Csi::LgrDate::system());
            LgrDate next_renewal(get_renew_date());
            if(now < next_renewal)
               license_info_table->add_tag(new Text(my_strings[strid_license_certificate_is_current]));
            else
            {
               Csi::LgrDate final_expire_date(get_expire_date());
               temp.str("");
               final_expire_date.format(temp, my_strings[strid_license_expires_final].c_str());
               license_info_table->add_tag(new Text(temp.str()));
            }
         }
         else
            license_info_table->add_tag(new Text(my_strings[strid_license_renewal_not_required]));

         if(!empty())
         {
            PolySharedPtr<Tag, Table> sub_licenses_table(new Table);
            tag.add_tag(new Text(my_strings[strid_license_sub_licenses], "h3"));
            tag.add_tag(sub_licenses_table.get_handle());
            sub_licenses_table->add_heading(new Text(my_strings[strid_license_product], "b"));
            sub_licenses_table->add_heading(new Text(my_strings[strid_license_serial_no], "b"));
            sub_licenses_table->add_heading(new Text(my_strings[strid_license_activation_date], "b"));
            for(const_iterator sli = begin(); sli != end(); ++sli)
            {
               sub_licenses_table->add_row();
               value_type const &sub_cert(*sli);
               sub_licenses_table->add_row();
               model_it = license_names.find(sub_cert->get_model());
               if(model_it != license_names.end())
                  sub_licenses_table->add_tag(new Text(model_it->second));
               else
                  sub_licenses_table->add_tag(new Text(sub_cert->get_model()));
               sub_licenses_table->add_last_cell_attribute("align", "center");
               sub_licenses_table->add_tag(new Text(sub_cert->get_serial_no()));
               sub_licenses_table->add_last_cell_attribute("align", "right");
               temp.str("");
               sub_cert->get_date_active().format(temp, "%n");
               sub_licenses_table->add_tag(new Text(temp.str()));
               sub_licenses_table->add_last_cell_attribute("align", "right");
            }
         }
      } // format_certificate

      
      bool Certificate::is_valid() const
      {
         bool rtn(renew_interval <= 0);
         if(!rtn)
         {
            LgrDate now(LgrDate::gmt());
            LgrDate expiration(date_issued + renew_interval * LgrDate::nsecPerSec + 30 * LgrDate::nsecPerDay);
            if(expiration > now)
               rtn = true;
         }
         return rtn;
      } // is_valid


      StrAsc lookup_machine_id()
      {
         StrAsc rtn;
         RegistryManager::read_anywhere_string(
            rtn,
            "MachineGuid",
            "SOFTWARE\\Microsoft\\Cryptography",
            HKEY_LOCAL_MACHINE,
            true);
         return rtn;
      } // get_machine_id


      LicenseClient::LicenseClient(
         StrAsc const &product_model,
         StrAsc const &model_,
         VersionNumber version_,
         timer_handle timer_,
         uint4 check_interval_,
         node_server_type server_type_):
         model(model_),
         version(version_),
         timer(timer_),
         state(state_standby),
         delegate(0),
         renew_id(0),
         check_interval(check_interval_),
         is_licensed(true)
      {
         RegistryManager registry(product_model, version.to_utf8());
         StrAsc license_key, license_key_version;

         registry.read_string(license_key, "LicenseKey", HKEY_LOCAL_MACHINE);
         registry.read_string(license_key_version, "LicenseKeyVersion", HKEY_LOCAL_MACHINE);
         if(license_key.length() > 0)
         {
            LicenseKey key(license_key, license_key_version);
            OStrAscStream temp;
            temp.imbue(StringLoader::make_locale(0));
            temp << key.get_serial_no();
            serial_no = temp.str();
            model = key.get_model();
         }
         else
            registry.read_string(serial_no, "SerialNumber", HKEY_LOCAL_MACHINE);
         model.to_upper();
         machine_id = lookup_machine_id();

         StrAsc server_uri = "https://cslicense.campbellsci.com";
         if(server_type_ == node_server_type_local)
            server_uri = "http://localhost:1337";
         else if(server_type_ == node_server_type_staging)
            server_uri = "http://cslicenseserver-cslicenseserver-staging.azurewebsites.net";
 
         renew_uri = server_uri + "/services/renew";
         register_addon_uri = server_uri + "/services/addon";
         check_version_uri = server_uri + "/services/check";
         update_uri = server_uri + "/services/update";
         changelog_uri = server_uri + "/services/changelog";

         if(timer == 0)
            timer.bind(new OneShot);
      } // constructor


      LicenseClient::LicenseClient(
         StrAsc const &model_,
         StrAsc const &serial_no_,
         VersionNumber version_,
         StrAsc const &machine_id_,
         timer_handle timer_,
         node_server_type server_type):
         model(model_),
         serial_no(serial_no_),
         machine_id(machine_id_),
         version(version_),
         timer(timer_),
         check_interval(0),
         renew_id(0),
         state(state_standby),
         delegate(0),
         is_licensed(false)
      {
         // we need to ensure that the serial number and machine ID values are somewhat valid.
         model.to_upper();
         if(serial_no.length() == 0)
            serial_no = "0";
         if(machine_id.length() == 0)
            machine_id = model + "-" + serial_no;
         
         // we need to set up the various uri addresses based upon the server selection.
         StrAsc server_uri("https://cslicense.campbellsci.com");
         if(server_type == node_server_type_local)
            server_uri = "http://localhost:1337";
         else if(server_type == node_server_type_staging)
            server_uri = "http://cslicenseserver-cslicenseserver-staging.azurewebsites.net";
         renew_uri = server_uri + "/services/renew";
         register_addon_uri = server_uri + "/services/addon";
         check_version_uri = server_uri + "/services/check";
         update_uri = server_uri + "/services/update";
         changelog_uri = server_uri + "/services/changelog";

         // make sure that the timer is created.
         if(timer == 0)
            timer.bind(new OneShot);
      } // constructor


      LicenseClient::~LicenseClient()
      {
         state = state_standby;
         delegate = 0;
         if(timer != 0 && renew_id)
            timer->disarm(renew_id);
         timer.clear();
         connection.clear();
         current_op.clear();
         operations.clear();
      } // destructor


      void LicenseClient::start(LicenseClientDelegate *delegate_)
      {
         if(state != state_standby)
            throw std::invalid_argument("license client has already been started");
         if(!LicenseClientDelegate::is_valid_instance(delegate_))
            throw std::invalid_argument("invalid license client delegate");
         delegate = delegate_;
         state = state_idle;
         if(check_interval)
            renew_id = timer->arm(this, 100);
      } // start


      void LicenseClient::stop()
      {
         delegate = 0;
         state = state_standby;
         if(renew_id != 0)
            timer->disarm(renew_id);
         connection.clear();
         current_op.clear();
         operations.clear();
      } // stop


      void LicenseClient::set_certificate(certificate_handle certificate_)
      {
         if(certificate_ != 0)
         {
            if(certificate_->get_model() != model)
               throw std::invalid_argument("invalid certificate model");
            if(certificate_->get_machine_id() != machine_id)
               throw std::invalid_argument("invalid certificate machine ID"); 
         }
         certificate = certificate_;
         event_certificate_changed::cpost(this);
      } // set_certificate


      void LicenseClient::set_watcher(SharedPtr<HttpClient::ConnectionWatcher> watcher_)
      {
         watcher = watcher_;
         if(connection != 0)
            connection->set_watcher(watcher_);
      } // set_watcher

      
      void LicenseClient::onOneShotFired(uint4 id)
      {
         if(id == renew_id)
         {
            // we need to decide whether the certificate must be renewed
            bool needs_renew(certificate == 0 || certificate->needs_renewal());
            if(needs_renew && state == state_idle)
               force_renewal();
            if(check_interval)
               renew_id = timer->arm(this, check_interval);
         }
      } // onOneShotFired


      void LicenseClient::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_renewal_started::event_id)
         {
            if(LicenseClientDelegate::is_valid_instance(delegate))
               delegate->on_starting_renewal(this);
            else
               stop();
         }
         else if(ev->getType() == event_renewal_complete::event_id)
         {
            event_renewal_complete *event(static_cast<event_renewal_complete *>(ev.get_rep()));
            if(LicenseClientDelegate::is_valid_instance(delegate))
               delegate->on_renewal_complete(this, event->outcome);
            else
               stop();
         }
         else if(ev->getType() == event_certificate_changed::event_id)
         {
            if(LicenseClientDelegate::is_valid_instance(delegate))
               delegate->on_certificate_changed(this);
         }
         else if(ev->getType() == event_register_addon_complete::event_id)
         {
            event_register_addon_complete *event(static_cast<event_register_addon_complete *>(ev.get_rep()));
            if(LicenseClientDelegate::is_valid_instance(delegate))
               delegate->on_register_addon_complete(this, event->outcome);
            else
               stop();
         }
         else if(ev->getType() == event_check_version_complete::event_id)
         {
            event_check_version_complete *event(static_cast<event_check_version_complete *>(ev.get_rep()));
            if(LicenseClientDelegate::is_valid_instance(delegate))
               delegate->on_check_version_complete(this, event->outcome, event->versions);
            else
               stop();
         }
         else if(ev->getType() == event_update_complete::event_id)
         {
            event_update_complete *event(static_cast<event_update_complete *>(ev.get_rep()));
            if(LicenseClientDelegate::is_valid_instance(delegate))
               delegate->on_update_complete(this, event->outcome);
            else
               stop();
         }
         else if(ev->getType() == event_changelog_complete::event_id)
         {
            event_changelog_complete *event(static_cast<event_changelog_complete *>(ev.get_rep()));
            if(LicenseClientDelegate::is_valid_instance(delegate))
               delegate->on_changelog_complete(this, event->outcome, event->change_log);
            else
               stop();
         }
      } // receive


      void LicenseClient::format_renewal_outcome(
         std::ostream &out, LicenseClientDelegate::renewal_outcome_type outcome)
      {
         using namespace SoftwareLicenseStrings;
         switch(outcome)
         {
         default:
            out << my_strings[strid_renewal_outcome_failure_unknown];
            break;

         case LicenseClientDelegate::renewal_outcome_success:
            out << my_strings[strid_renewal_outcome_success];
            break;

         case LicenseClientDelegate::renewal_outcome_failure_unknown_product:
            out << my_strings[strid_renewal_outcome_failure_unknown_product];
            break;

         case LicenseClientDelegate::renewal_outcome_failure_disabled:
            out << my_strings[strid_renewal_outcome_failure_disabled];
            break;

         case LicenseClientDelegate::renewal_outcome_failure_trial_expired:
            out << my_strings[strid_renewal_outcome_failure_trial_expired];
            break;

         case LicenseClientDelegate::renewal_outcome_failure_communication:
            out << my_strings[strid_renewal_outcome_failure_communication];
            break;
         }
      } // format_renewal_outcome


      void LicenseClient::force_renewal()
      {
         if(state != state_standby)
         {
            operations.push_back(new RenewalOp(this));
            start_next_operation();
         }
      } // force_renewal


      void LicenseClient::register_addon(StrAsc const &child_model, StrAsc const &child_serial_no)
      {
         if(state != state_standby)
         {
            operations.push_back(new RegisterAddonOp(this, child_model, child_serial_no));
            start_next_operation();
         }
      } // register_addon


      void LicenseClient::format_register_addon_outcome(
         std::ostream &out, LicenseClientDelegate::register_addon_outcome_type outcome)
      {
         using namespace SoftwareLicenseStrings;
         switch(outcome)
         {
         case LicenseClientDelegate::register_addon_outcome_failure_unknown:
         default:
            out << my_strings[strid_register_addon_outcome_failure_unknown];
            break;
            
         case LicenseClientDelegate::register_addon_outcome_success:
            out << my_strings[strid_register_addon_outcome_success];
            break;
            
         case LicenseClientDelegate::register_addon_outcome_failure_no_parent_product:
            out << my_strings[strid_register_addon_outcome_failure_no_parent_product];
            break;
            
         case LicenseClientDelegate::register_addon_outcome_failure_child_already_added:
            out << my_strings[strid_register_addon_outcome_failure_child_already_added];
            break;
            
         case LicenseClientDelegate::register_addon_outcome_failure_no_child_product:
            out << my_strings[strid_register_addon_outcome_failure_no_child_product];
            break;
            
         case LicenseClientDelegate::register_addon_outcome_failure_disabled:
            out << my_strings[strid_register_addon_outcome_failure_disabled];
            break;
            
         case LicenseClientDelegate::register_addon_outcome_failure_communication:
            out << my_strings[strid_register_addon_outcome_failure_communication];
            break;
         }
      } // format_register_addon_outcome


      void LicenseClient::check_version()
      {
         if(state != state_standby)
         {
            operations.push_back(new CheckVersionOp(this));
            start_next_operation();
         }
      } // check_updates


      void LicenseClient::format_check_version_outcome(
         std::ostream &out, LicenseClientDelegate::check_version_outcome_type outcome)
      {
         using namespace SoftwareLicenseStrings;
         switch(outcome)
         {
         case LicenseClientDelegate::check_version_outcome_failure_unknown:
         default:
            out << my_strings[strid_check_version_outcome_failure_unknown];
            break;

         case LicenseClientDelegate::check_version_outcome_success:
            out << my_strings[strid_check_version_outcome_success];
            break;

         case LicenseClientDelegate::check_version_outcome_failure_communication:
            out << my_strings[strid_check_version_outcome_failure_communication];
            break;
         }
      } // format_check_version_outcome


      void LicenseClient::get_changelog(VersionNumber const &version)
      {
         if(state != state_standby)
         {
            operations.push_back(new ChangeLogOp(this, version));
            start_next_operation();
         }
      } // get_changelog


      void LicenseClient::format_changelog_outcome(std::ostream &out, LicenseClientDelegate::changelog_outcome_type outcome)
      {
         using namespace SoftwareLicenseStrings;
         switch(outcome)
         {
         case LicenseClientDelegate::changelog_outcome_failure_unknown:
         default:
            out << my_strings[strid_changelog_outcome_failure_unknown];
            break;

         case LicenseClientDelegate::changelog_outcome_success:
            out << my_strings[strid_changelog_outcome_success];
            break;

         case LicenseClientDelegate::changelog_outcome_failure_communication:
            out << my_strings[strid_changelog_outcome_failure_communication];
            break;

         case LicenseClientDelegate::changelog_outcome_failure_no_product:
            out << my_strings[strid_changelog_outcome_failure_no_product];
            break;
         }
      } // format_changelog_outcome


      void LicenseClient::format_changelog(
         Html::Tag &tag,
         Json::Object &change_log,
         std::deque<StrAsc> include_codes)
      {
         using namespace Html;
         using namespace SoftwareLicenseStrings;
         VersionNumber version(change_log.get_property_str("version").c_str());
         OStrAscStream temp, temp2;
         Json::ArrayHandle builds_json(change_log.get_property("builds"));
         
         temp.imbue(StringLoader::make_locale());
         temp2.imbue(StringLoader::make_locale());
         temp << boost::format(my_strings[strid_changelog_version].c_str()) % version.to_utf8(true);
         tag.add_tag(new Text(temp.str(), "h2"));
         for(Json::Array::iterator bi = builds_json->begin(); bi != builds_json->end(); ++bi)
         {
            Json::ObjectHandle build_json(*bi);
            Json::ArrayHandle changes_json(build_json->get_property("changes"));
            LgrDate build_date(LgrDate::fromStr(build_json->get_property_str("date").c_str()));
            VersionNumber build_version(build_json->get_property_str("build").c_str());
            PolySharedPtr<Tag, Html::List> changes_html(new Html::List);

            build_version.insert(build_version.begin(), version.begin(), version.end());
            temp2.str("");
            build_date.format(temp2, "%n");
            temp.str("");
            temp << boost::format(my_strings[strid_changelog_build].c_str()) %
               build_version.to_utf8(true) %
               temp2.str();
            tag.add_tag(new Html::Text(temp.str(), "h3"));
            tag.add_tag(changes_html.get_handle());
            for(Json::Array::iterator ci = changes_json->begin(); ci != changes_json->end(); ++ci)
            {
               Json::ObjectHandle change_json(*ci);
               StrAsc severity(change_json->get_property_str("severity"));
               if(include_codes.empty() || std::find(include_codes.begin(), include_codes.end(), severity) != include_codes.end())
               {
                  temp.str("");
                  temp << boost::format(my_strings[strid_changelog_change].c_str()) % severity % change_json->get_property_str("message");
                  changes_html->add_tag(new Text(temp.str()));
               }
            }
         }
      } // format_changelog


      void LicenseClient::update(Csi::Json::ObjectHandle &version)
      {
         if(state != state_standby)
         {
            operations.push_back(new UpdateOp(this, version));
            start_next_operation();
         }
      } // update


      void LicenseClient::format_update_outcome(
         std::ostream &out, LicenseClientDelegate::update_outcome_type outcome)
      {
         using namespace SoftwareLicenseStrings;
         switch(outcome)
         {
         case LicenseClientDelegate::update_outcome_failure_unknown:
         default:
            out << my_strings[strid_update_outcome_failure_unknown];
            break;

         case LicenseClientDelegate::update_outcome_success:
            out << my_strings[strid_update_outcome_success];
            break;

         case LicenseClientDelegate::update_outcome_failure_no_product:
            out << my_strings[strid_update_outcome_failure_no_product];
            break;

         case LicenseClientDelegate::update_outcome_failure_license_expired:
            out << my_strings[strid_update_outcome_failure_license_expired];
            break;

         case LicenseClientDelegate::update_outcome_failure_license_disabled:
            out << my_strings[strid_update_outcome_failure_license_disabled];
            break;

         case LicenseClientDelegate::update_outcome_failure_needs_maintenance:
            out << my_strings[strid_update_outcome_failure_needs_maintenance];
            break;

         case LicenseClientDelegate::update_outcome_failure_maintenance_expired:
            out << my_strings[strid_update_outcome_failure_maintenance_expired];
            break;

         case LicenseClientDelegate::update_outcome_failure_aborted:
            out << my_strings[strid_update_outcome_failure_aborted];
            break;

         case LicenseClientDelegate::update_outcome_failure_communication:
            out << my_strings[strid_update_outcome_failure_communication];
            break;
         }
      } // format_update_outcome
      

      void LicenseClient::on_operation_complete(LicenseClientOpBase *operation)
      {
         if(current_op == operation)
         {
            current_op.clear();
            start_next_operation();
         }
         else
         {
            operations_type::iterator oi(
               std::find_if(operations.begin(), operations.end(), HasSharedPtr<LicenseClientOpBase>(operation)));
            if(oi != operations.end())
               operations.erase(oi);
         }
      } // on_operation_complete


      Json::Object LicenseClient::make_renewal_request(bool sign)
      {
         Json::Object request;
         request.set_property_str("model", model);
         request.set_property_str("version", version.to_utf8());
         request.set_property_str("serial_no", serial_no);
         request.set_property_str("machine_id", machine_id);
         
         if(sign)
         {
            Json::Object signed_request;
            OStrAscStream formatted_request;
            request.format(formatted_request);
            SignedJson::generate_envelope(signed_request, formatted_request.c_str(), formatted_request.length());
            return signed_request;
         }
         else
            return request;
      } // make_renewal_request

      
      void LicenseClient::start_next_operation()
      {
         if(current_op == 0 && !operations.empty())
         {
            // allocate the connection if needed and create the request object.
            if(connection == 0)
            {
               connection.bind(new HttpClient::Connection(timer));
               if(watcher != 0)
                  connection->set_watcher(watcher);
            }

            // set the current operation and start it with the connection
            current_op = operations.front();
            operations.pop_front();
            current_op->start(connection);
         }
      } // start_next_operation

      
      LicenseKey::LicenseKey(StrAsc const &license_key, StrAsc const &version, bool disable_validation)
      {
         // we'll begin by breaking the string into fragments
         using namespace SoftwareLicenseStrings;
         size_t first_dash_pos(license_key.find("-"));
         size_t second_dash_pos;
         StrAsc sig_str;
         StrAsc rest;
         
         if(first_dash_pos >= license_key.length())
            throw std::invalid_argument(my_strings[strid_license_key_malformed].c_str());
         second_dash_pos = license_key.find("-", first_dash_pos + 1);
         if(second_dash_pos >= license_key.length())
            throw std::invalid_argument(my_strings[strid_license_key_malformed].c_str());
         license_key.sub(model, 0, first_dash_pos);
         license_key.sub(sig_str, first_dash_pos + 1, second_dash_pos - first_dash_pos - 1);
         license_key.sub(rest, second_dash_pos + 1, license_key.length());
         model.to_upper();
         sig_str.to_upper();
         rest.to_upper();
         
         // we now need to validate the signature with that extracted from the key.
         IBuffStream temp(sig_str.c_str(), sig_str.length());
         uint2 reported_sig;
         uint2 my_sig;
         
         temp.imbue(StringLoader::make_locale(0));
         temp >> std::hex >> reported_sig;
         if(!temp)
            throw std::invalid_argument(my_strings[strid_license_key_bad_sig].c_str());
         my_sig = calcSigFor(model.c_str(), model.length());
         my_sig = calcSigFor(rest.c_str(), rest.length(), my_sig);
         my_sig = calcSigFor(version.c_str(), version.length(), my_sig);
         my_sig = calcSigFor(license_key_private.c_str(), license_key_private.length(), my_sig);
         if(!disable_validation && my_sig != reported_sig)
            throw std::invalid_argument(my_strings[strid_license_key_bad_sig].c_str());

         // we can now decode the serial number and generation date.
         StrBin rest_decoded;
         Base32::decode(rest_decoded, rest.c_str(), rest.length());
         Packet packet(rest_decoded.getContents(), (uint4)rest_decoded.length(), 0, false);
         int4 seconds;
         uint2 msec;

         serial_no = packet.readUInt4(!is_big_endian());
         seconds = packet.readInt4(!is_big_endian());
         msec = packet.readUInt2(!is_big_endian());
         time_stamp = seconds * LgrDate::nsecPerSec + msec * LgrDate::nsecPerMSec;
      } // parsing consructor


      StrAsc LicenseKey::generate_key(StrAsc const &version)
      {
         OStrAscStream rtn;
         Packet packet(0);
         uint2 sig;
         StrAsc encoded;
         
         packet.addUInt4(serial_no, !is_big_endian());
         packet.addInt4(time_stamp.get_sec(), !is_big_endian());
         packet.addUInt2(static_cast<uint2>(time_stamp.nsec() / LgrDate::nsecPerMSec));
         Base32::encode(encoded, packet.getMsg(), packet.length());
         sig = calcSigFor(model.c_str(), model.length());
         sig = calcSigFor(encoded.c_str(), encoded.length(), sig);
         if(version.length())
            sig = calcSigFor(version.c_str(), version.length(), sig);
         sig = calcSigFor(license_key_private.c_str(), license_key_private.length(), sig);
         rtn << model << "-" << std::hex << std::setw(4) << std::setfill('0')
             << std::uppercase << sig << '-' << encoded;
         return rtn.str();
      } // generate_key
   };
};

