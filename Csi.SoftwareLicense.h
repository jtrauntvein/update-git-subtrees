/* Csi.SoftwareLicense.h

   Copyright (C) 2017, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 19 July 2017
   Last Change: Monday 03 August 2020
   Last Commit: $Date: 2020-08-03 14:06:48 -0600 (Mon, 03 Aug 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SoftwareLicense_h
#define Csi_SoftwareLicense_h
#include "Csi.SignedJson.h"
#include "Csi.InstanceValidator.h"
#include "Csi.HttpClient.h"
#include "Csi.VersionNumber.h"
#include "Csi.Html.Tag.h"
#include "OneShot.h"


namespace Csi
{
   namespace SoftwareLicense
   {
      // @group class forward declarations
      class LicenseClient;
      // @endgroup:

      
      /**
       * Defines an object that represents the content of a software license certificate.
       */
      class Certificate
      {
      public:
         /**
          * Default Constructor
          */
         Certificate();

         /**
          * Copy constructor
          *
          * @param other Specifies the certficiate to copy.
          */
         Certificate(Certificate const &other);

         /**
          * copy operator
          *
          * @param other Specifies the certificare to copy.
          */
         Certificate &operator =(Certificate const &other);

         /**
          * Destructor
          */
         virtual ~Certificate();

         /**
          * @return Returns the software product model number.
          */
         StrAsc const &get_model() const
         { return model; }

         /**
          * @return Returns the software version.
          */
         VersionNumber const &get_version() const
         { return version; }

         /**
          * @return Returns the product serial number.
          */
         StrAsc const &get_serial_no() const
         { return serial_no; }

         /**
          * @return Returns the UTC time when this certificate was last renewed.
          */
         LgrDate const &get_date_issued() const
         { return date_issued; }

         /**
          * @return Returns the UTC time when the software product license was first activated.
          */
         LgrDate const &get_date_active() const
         { return date_active; }

         /**
          * @return Returns the interval in seconds at which this certificate must be renewed.  A
          * value of less than or equal to zero will indicate that renewal is not required.
          */
         int8 get_renew_interval() const
         { return renew_interval; }

         /**
          * @return Returns the date and time after which the certificate must be renewed.
          */
         LgrDate get_renew_date() const
         { return date_issued + renew_interval * LgrDate::nsecPerSec; }

         /**
          * @return Returns the date and time after which the certificate is considered to be
          * expired (includes the thirty day grace period).
          */
         LgrDate get_expire_date() const
         { return date_issued + renew_interval * LgrDate::nsecPerSec + 30 * LgrDate::nsecPerDay; }

         /**
          * @return Returns the machine identifier for which this certificate was issued.
          */
         StrAsc const &get_machine_id() const
         { return machine_id; }

         /**
          * @return Returns true if this license was signed when it was read.
          */
         bool get_was_signed() const
         { return was_signed; }

         // @group: declarations to act as a collection of certificates.

         /**
          * @return Returns the iterator to the start of the registered sub-product certificates.
          */
         typedef SharedPtr<Certificate> value_type;
         typedef std::deque<value_type> other_licenses_type;
         typedef other_licenses_type::iterator iterator;
         typedef other_licenses_type::const_iterator const_iterator;
         iterator begin()
         { return other_licenses.begin(); }
         const_iterator begin() const
         { return other_licenses.begin(); }

         /**
          * @return Returns the iterator beyond the end of the collection of otther licenses.
          */
         iterator end()
         { return other_licenses.end(); }
         const_iterator end() const
         { return other_licenses.end(); }

         /**
          * @return Returns true if this certificate contains no sub-licenses.
          */
         bool empty() const
         { return other_licenses.empty(); }

         /**
          * @return Returns the reference to the first other license.
          */
         value_type &front()
         { return other_licenses.front(); }
         value_type const &front() const
         { return other_licenses.front(); }

         /**
          * @return Returns the reference to the last other license.
          */
         value_type &back()
         { return other_licenses.back(); }
         value_type const &back() const
         { return other_licenses.back(); }
         
         // @endgroup:

         /**
          * Reads the content of this license from the specified JSON structure.  If the JSON
          * structure is a signed JSON, the signature will first be validated and the content parsed
          * before the structure is read.
          *
          * @param license_json Specifies the JSOn structure for this license or a signed json
          * structure.
          *
          * @param require_signed Set to true if the certificate must be in signed JSON form in
          * order to load.
          */
         void read(Csi::Json::Object &license_json, bool require_signed = false);

         /**
          * Writes the content of this license into the specified JSON structure.
          *
          * @param license_json Specifies the structure to which the license will be written.
          *
          * @param sign Set to true if the content if the signed version of the license is to be
          * written.
          */
         void write(Csi::Json::Object &license_json, bool sign) const;

         /**
          * @return Returns true if this certificate needs renewal
          */
         virtual bool needs_renewal() const;

         /**
          * @return Returns true if this certificate can be considered to be valid.  Note that a
          * certificate can need renewal and still be considered valid if fewer than thirty days
          * have elapsed since the certificate expired.
          */
         virtual bool is_valid() const;

         /**
          * Generates a report on the certificate by adding up to two HTML tables to the provided
          * tag that describe the certificate, its status, and its add-on certificates.
          *
          * @param tag Specifies the tag to which the HTML will be added.
          *
          * @param license_names Specifies a map of license key models and the user friendly model
          * to which these names will be mapped.  If the certificate contains models that are not in
          * this list, those models will appear.
          */
         typedef std::map<StrAsc, StrAsc> license_names_type;
         void format_certificate(Html::Tag &tag, license_names_type const &license_names) const;

      private:
         /**
          * Specifies the product model number.
          */
         StrAsc model;

         /**
          * Specifies the product version.
          */
         VersionNumber version;

         /**
          * Specifies the product serial number.
          */
         StrAsc serial_no;

         /**
          * Specifies the date on which this certificate was issued.
          */
         LgrDate date_issued;

         /**
          * Specifies the date on which the product was first activated.
          */
         LgrDate date_active;

         /**
          * Specifies the interval in seconds at which this certificate should be renewed.
          */
         int8 renew_interval;

         /**
          * Specifies the machine identifier for which the certificate was issued.
          */
         StrAsc machine_id;

         /**
          * Specifies the collection of sub-products that are associated with this license.
          */
         other_licenses_type other_licenses;

         /**
          * Specifies whether this license was signed when it was read.
          */
         bool was_signed;
      };
      


      /**
       * Defines the interface that an application must implement to receive notifications from the
       * license client component.
       */
      class LicenseClientDelegate: public Csi::InstanceValidator
      {
      public:
         /**
          * Notifies the application that the license client is starting the process of renewing the
          * license certificate.
          *
          * @param sender Specifies the license client object.
          */
         virtual void on_starting_renewal(LicenseClient *sender)
         { }

         /**
          * Notifies the application that the license client has completed renewal of the license
          * certificate.
          *
          * @param sender Specifies the license client object.
          *
          * @param outcome Specifies the outcome of the attempt.
          */
         enum renewal_outcome_type
         {
            renewal_outcome_failure_unknown = 0,
            renewal_outcome_success = 1,
            renewal_outcome_failure_unknown_product = 2,
            renewal_outcome_failure_disabled = 3,
            renewal_outcome_failure_trial_expired = 4,
            renewal_outcome_failure_communication
         };
         virtual void on_renewal_complete(LicenseClient *sender, renewal_outcome_type outcome)
         { }

         /**
          * Called when the software license certificate has been changed.
          *
          * @param sender Specifies the client component reporting this event.
          */
         virtual void on_certificate_changed(LicenseClient *sender)
         { }

         /**
          * Called when registration of an add-on has been completed.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param outcome Specifies the outcome of the registration attempt.
          */
         enum register_addon_outcome_type
         {
            register_addon_outcome_failure_unknown = 0,
            register_addon_outcome_success = 1,
            register_addon_outcome_failure_no_parent_product = 2,
            register_addon_outcome_failure_child_already_added = 3,
            register_addon_outcome_failure_no_child_product = 4,
            register_addon_outcome_failure_disabled = 5,
            register_addon_outcome_failure_communication
         };
         virtual void on_register_addon_complete(LicenseClient *sender, register_addon_outcome_type outcome)
         { }

         /**
          * Called when an operation for check for updates has completed.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param outcome Specifies the outcome of the operation.
          *
          * @param versions Specifies the versions of patches that are available.  
          */
         enum check_version_outcome_type
         {
            check_version_outcome_failure_unknown = 0,
            check_version_outcome_success = 1,
            check_version_outcome_failure_communication = 2
         };
         virtual void on_check_version_complete(
            LicenseClient *sender,
            check_version_outcome_type outcome,
            Json::ArrayHandle &versions)
         { }

         /**
          * Called to report that data has been received for an update request.
          *
          * @param sender Specifies the license client that is reporting this event.
          *
          * @param request Specifies the HTTP client component.  This object will contain all of the
          * header parameters that were parsed and will also buffer the received data.
          *
          * @return Returns true if the request should continue.
          */
         virtual bool on_update_data(
            LicenseClient *sender, HttpClient::Request &request)
         { return true; }

         /**
          * Called to report that the update transaction has been completed.
          *
          * @param sender Specifies the license client component reporting this event.
          *
          * @param outcome Specifies the outcome of the transaction.
          */
         enum update_outcome_type
         {
            update_outcome_failure_unknown = 0,
            update_outcome_success = 1,
            update_outcome_failure_no_product = 2,
            update_outcome_failure_license_expired = 3,
            update_outcome_failure_license_disabled = 4,
            update_outcome_failure_needs_maintenance = 5,
            update_outcome_failure_maintenance_expired = 6,
            update_outcome_failure_aborted = 7,
            update_outcome_failure_communication = 8
         };
         virtual void on_update_complete(
            LicenseClient *sender, update_outcome_type outcome)
         { }

         /**
          * Called to report that the change log has been retrieved from the license server.
          *
          * @param sender Specifies the license client reporting this event.
          *
          * @param outcome Specifies the outcome of the operation.
          *
          * @param change_log Specifies the change log JSON object that was received.
          */
         enum changelog_outcome_type
         {
            changelog_outcome_failure_unknown = 0,
            changelog_outcome_success = 1,
            changelog_outcome_failure_no_product = 2,
            changelog_outcome_failure_communication = 3
         };
         virtual void on_changelog_complete(
            LicenseClient *sender, changelog_outcome_type outcome, Json::ObjectHandle &change_log)
         { }
      };


      /**
       * Defines a base class for an operation carried out by a LicenseClient component.
       */
      class LicenseClientOpBase: public HttpClient::RequestClient
      {
      public:
         /**
          * Constructor
          *
          * @param owner_ Specifies the license client object that owns this operation.
          */
         LicenseClientOpBase(LicenseClient *owner_):
            owner(owner_)
         { }

         /**
          * Destructor
          */
         virtual ~LicenseClientOpBase()
         { owner = 0; }

         /**
          * Called by the license client to start the operation.  There will likely be a time delay
          * between the time that the operation is constructed and this method is called because the
          * license client will have to maintain a queue of these operations.
          *
          * @param connection_ Specifies the connection maintained by the owner that will carry out
          * our requests.
          */
         typedef SharedPtr<HttpClient::Connection> connection_handle;
         virtual void start(connection_handle connection_) = 0;
         
      protected:
         /**
          * Specifies the license client that owns this operation.
          */
         LicenseClient *owner;
      };


      /**
       * @return Looks up the machone ID for the host computer.
       */
      StrAsc lookup_machine_id();
      

      /**
       * Defines a component that can be used to communicate with the license server in order to
       * renew or modify software licenses.
       */
      class LicenseClient: public OneShotClient, public Csi::EventReceiver
      {
      public:
         /**
          * Constructor for registered product
          *
          * @param product_model Specifies the software product model name.  This value will be used
          * to locate the registry key for the software product.
          *
          * @param model_ Specifies the license model.  If the license key is found in the registry,
          * the model stored in that key will replace the value specified here.
          *
          * @param version_ Specifies the software version.
          *
          * @param timer Specifies the one shot timer to be used by this component.  If specified as
          * a null pointer (the default value), this component will create its own one shot timer.
          *
          * @param check_interval_ Specifies the interval at which the component will check to see
          * if the license needs to be renewed.  If zero, the license client will not automatically
          * check.
          *
          * @param server_type Specifies the license server instance that will be selected.
          * Defaults to the production server.
          */
         enum node_server_type
         {
            node_server_type_production = 0,
            node_server_type_local = 1,
            node_server_type_staging = 2
         };
         typedef Csi::SharedPtr<OneShot> timer_handle;
         LicenseClient(
            StrAsc const &product_model,
            StrAsc const &model_,
            VersionNumber version_,
            timer_handle timer_ = 0,
            uint4 check_interval_ = 5 * LgrDate::msecPerMin,
            node_server_type server_type = node_server_type_production);

         /**
          * Constructor for unlicensed product.
          *
          * @param model_ Specifies the model for the unlicensed product.
          *
          * @param serial_no_ Specifies the product serial number.
          *
          * @param version_ Specifies the version.
          *
          * @param machine_id_ Specifies the machine ID.
          *
          * @param timer_ Specifies the one shot timer that will be used for client communications.
          *
          * @param server_type Specifies the license server instance that will be selected.
          * Defaults to the production server.
          */
         LicenseClient(
            StrAsc const &model_,
            StrAsc const &serial_no_,
            VersionNumber version_,
            StrAsc const &machine_id_ = lookup_machine_id(),
            timer_handle timer_ = 0,
            node_server_type server_type = node_server_type_production);
         
         /**
          * Destructor
          */
         virtual ~LicenseClient();

         /**
          * Starts the process of maintaining the license certificate.  If there is no associated
          * certificate, it will be renewed immediately.  Otherwise, a timer will be set to renew
          * the certificate.
          *
          * @param delegate_ Specifies the application object that will receive event notifications
          * from this component.
          */
         virtual void start(LicenseClientDelegate *delegate_);

         /**
          * Ceases operations to communicate with the license server and stops any further
          * notifications to the delegate.
          */
         virtual void stop();

         /**
          * @return Returns the software model number.
          */
         StrAsc const &get_model() const
         { return model; }

         /**
          * @return Returns the software version.
          */
         VersionNumber const &get_version() const
         { return version; }

         /**
          * @return Returns the software serial number.
          */
         StrAsc const &get_serial_no() const
         { return serial_no; }

         /**
          * @return Returns the host computer ID.
          */
         StrAsc const &get_machine_id() const
         { return machine_id; }

         /**
          * @return Returns true if this client was created to work with a licensed product.
          */
         bool get_is_licensed() const
         { return is_licensed; }
         
         /**
          * @return Returns the certificate handle
          */
         typedef Csi::SharedPtr<Certificate> certificate_handle;
         certificate_handle &get_certificate()
         { return certificate; }

         /**
          * @param certificate Specifies the certificate for this license.
          */
         void set_certificate(certificate_handle certificate);

         /**
          * @return Returns the timer used by this component.
          */
         timer_handle &get_timer()
         { return timer; }

         /**
          * @param watcher_ Specifies the object that will watch the HTTP connection.
          */
         void set_watcher(SharedPtr<HttpClient::ConnectionWatcher> watcher_);

         /**
          * Adds a comment to the current watcher (if any).
          *
          * @param comment Specifies the comment to add.
          */
         void add_watcher_comment(StrAsc const &comment)
         {
            if(watcher != 0)
               watcher->on_log_comment(connection.get_rep(), comment);
         }
         
         /**
          * Overloads the one shot timer event handler.
          */
         virtual void onOneShotFired(uint4 id);

         /**
          * Handles the asynchronously posted events.
          */
         virtual void receive(SharedPtr<Event> &ev);

         /**
          * Formats a description of the specified renewal outcome code to the specified stream.
          *
          * @param out Specifies the stream to which the outcome will be written.
          *
          * @param outcome Specifies the outcome code.
          */
         static void format_renewal_outcome(
            std::ostream &out, LicenseClientDelegate::renewal_outcome_type outcome);

         /**
          * Can be called by the application to force renewal of the license certificate.  In order
          * for this method to have any effect, the component must be in a started state.
          */
         void force_renewal();

         /**
          * Can be called by the application in order to register an add-on product to the license
          * for this software product.  In order for this method to have any effect, this component
          * must be in a started state.
          *
          * @param child_model Specifies the model of the child product.
          *
          * @param child_serial_no Specifies the serial number of the child product.
          */
         void register_addon(StrAsc const &child_model, StrAsc const &child_serial_no);

         /**
          * Formats a description of the specified register add-on outcome code to the specified
          * stream.
          *
          * @param out Specifies the stream to which the text will be written.
          *
          * @param outcome Specifies the outcome code to format.
          */
         static void format_register_addon_outcome(
            std::ostream &out, LicenseClientDelegate::register_addon_outcome_type outcome);

         /**
          * Can be called by the application to check for an updated version.
          */
         void check_version();

         /**
          * Formats a description of the outcome code for check_updates.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outcome Specifies the outcome code.
          */
         static void format_check_version_outcome(
            std::ostream &out, LicenseClientDelegate::check_version_outcome_type outcome);

         /**
          * Can be called by the application to get the change log for specified version.  When the
          * operation is complete, the delegate will be notified through its on_changelog_complete()
          * method. 
          *
          * @param version Specifies the version of the change log to retrieve.
          */
         void get_changelog(VersionNumber const &version);

         /**
          * Formats the description of the given change log outcome code.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outcome Specifies the outcome to format.
          */
         static void format_changelog_outcome(
            std::ostream &out, LicenseClientDelegate::changelog_outcome_type outcome);

         /**
          * Generates a report on the specified change log structure to the specified HTML element.
          *
          * @param tag Specifies the HTML element to which the report will be formatted.
          *
          * @param change_log Specifies the JSON object that describes the software change log.
          *
          * @param include_codes Specifies the list of severity codes that should be included in the
          * output.   If empty (the default value), all changes will be listed.
          */
         static void format_changelog(
            Html::Tag &tag,
            Json::Object &change_log,
            std::deque<StrAsc> include_codes = std::deque<StrAsc>());
         
         /**
          * Starts an update operation to attempt to retrieve the update for the specified software
          * version.
          *
          * @param version Specifies the JSON object that has the same parameters as were returned
          * from a check_version() operation.
          */
         void update(Json::ObjectHandle &version);

         /**
          * Formats a description of the outcome code for an update() operation.
          *
          * @param out Specifies the stream to which the description will be formatted.
          *
          * @param outcome Specifies the outcome code to format.
          */
         static void format_update_outcome(
            std::ostream &out, LicenseClientDelegate::update_outcome_type outcome);
         
         /**
          * Called by an operation when it has completed its work.
          *
          * @param operation Specifies the operation that has completed its work.
          */
         void on_operation_complete(LicenseClientOpBase *operation);

         /**
          * @return Returns the URI used for renewing license certificates.
          */
         StrAsc const &get_renew_uri() const
         { return renew_uri; }

         /**
          * @return Returns the URI used for register add-on licenses.
          */
         StrAsc const &get_register_addon_uri() const
         { return register_addon_uri; }

         /**
          * @return Returns the URI used to check for new software versions.
          */
         StrAsc const &get_check_version_uri() const
         { return check_version_uri; }

         /**
          * @return Returns the URI used to get an update for new software versions.
          */
         StrAsc const &get_update_uri() const
         { return update_uri; }

         /**
          * @return Returns the URI used for the changelog operation.
          */
         StrAsc const &get_changelog_uri() const
         { return changelog_uri; }

         /**
          * @return Returns a JSON structure that contains a renewal request.
          *
          * @param sign Set to true if the returned request must be signed.
          */
         Json::Object make_renewal_request(bool sign = true);

         /**
          * @return Returns the reference to the delegate.
          */
         LicenseClientDelegate *get_delegate()
         { return delegate; }

      private:
         /**
          * Starts the next operation in the queue assuming that there is one and that there is no
          * currently pending operation.
          */
         void start_next_operation();
         
      private:
         /**
          * Specifies the model number for the software package.
          */
         StrAsc model;

         /**
          * Specifies the version of the software package.
          */
         VersionNumber const version;

         /**
          * Specifies the serial number of the software package.
          */
         StrAsc serial_no;

         /**
          * Specifies the host compute ID.
          */
         StrAsc machine_id;

         /**
          * Set to true if the product is licensed (initialised from a license key).
          */
         bool is_licensed;

         /**
          * Specifies the current license certificate.
          */
         certificate_handle certificate;

         /**
          * Specifies the delegate.
          */
         LicenseClientDelegate *delegate;

         /**
          * Specifies the object that we will use to communicate with the license server.
          */
         Csi::SharedPtr<HttpClient::Connection> connection;

         /**
          * Specifies an application object that monitors the low level between this license client
          * and the license server.
          */
         Csi::SharedPtr<HttpClient::ConnectionWatcher> watcher;

         /**
          * Specifies the queue of pending operations.
          */
         typedef SharedPtr<LicenseClientOpBase> operation_handle;
         typedef std::deque<operation_handle> operations_type;
         operations_type operations;

         /**
          * Specifies the operation that is currently being carried out.
          */
         operation_handle current_op;

         /**
          * Specifies the timer.
          */
         timer_handle timer;

         /**
          * Specifies the current state of this component.
          */
         enum state_type
         {
            state_standby,
            state_renewing,
            state_upgrading,
            state_adding_sublicense,
            state_idle
         } state;

         /**
          * Specifies the timer object used to track whether the certificate should be renewed.
          */
         uint4 renew_id;

         /**
          * Specifies the check interval
          */
         uint4 const check_interval;

         /**
          * Specifies the URI used to renew licenses.
          */
         StrAsc renew_uri;

         /**
          * Specifies the URI used to register add-on licenses.
          */
         StrAsc register_addon_uri;

         /**
          * Specifies the URI used to check for new versions.
          */
         StrAsc check_version_uri;

         /**
          * Specifies the URI used to get updates.
          */
         StrAsc update_uri;

         /**
          * Specifies the URI used for getting a changelog.
          */
         StrAsc changelog_uri;
      };


      /**
       * Defines an object that represents the information associated with a license key.  This
       * object will be able to both encode and decode license keys strings and will also validate
       * the license key as it is decoded.
       */
      class LicenseKey
      {
      public:
         /**
          * Construct from the model, serial number, and date.
          *
          * @param model_ Specifies the model for this key.
          *
          * @param company_code_ Specifies the company code as a single letter.
          *
          * @param serial_no_ Specifies the serial number.
          *
          * @param time_stamp_ Specifies the time of creation.
          */
         LicenseKey(StrAsc const &model_, char company_code_, uint4 serial_no_, LgrDate const &time_stamp_ = LgrDate::gmt()):
            model(model_),
            serial_no((static_cast<uint4>(company_code_) << 24) | serial_no_),
            time_stamp(time_stamp_)
         { }

         /**
          * Construct from a parsed and validated license key.
          *
          * @param license_key Specifies the license key string.
          *
          * @param version Specifies the version used to validate the license key.
          *
          * @param disable_validation Set to true (false by default) if the key should only be interpreted.
          */
         LicenseKey(StrAsc const &license_key, StrAsc const &version, bool disable_validation = false);

         /**
          * Copy constructor
          */
         LicenseKey(LicenseKey const &other):
            model(other.model),
            serial_no(other.serial_no),
            time_stamp(other.time_stamp)
         { }
         
         /**
          * copy operator
          */
         LicenseKey &operator =(LicenseKey const &other)
         {
            model = other.model;
            serial_no = other.serial_no;
            time_stamp = other.time_stamp;
            return *this;
         }

         /**
          * @return Returns the product model.
          */
         StrAsc const &get_model() const
         { return model; }

         /**
          * @return Returns the company code.
          */
         StrAsc get_company_code() const
         {
            StrAsc rtn;
            if((serial_no & 0xff000000) != 0)
               rtn.append(static_cast<char>((serial_no & 0xff000000) >> 24));
            return rtn;
         }

         /**
          * @return Returns the serial number.
          *
          * @param include_company_code Set to true (the default) if the company code should be
          * included in the return value.
          */
         uint4 get_serial_no(bool include_company_code = true) const
         {
            uint4 rtn(serial_no);
            if(!include_company_code)
               rtn &= 0x00ffffff;
            return rtn;
         }

         /**
          * @return Returns the date of generation.
          */
         LgrDate get_time_stamp() const
         { return time_stamp; }

         /**
          * @return Returns a generated license key.
          *
          * @param version Specifies the version used to generate the key signature.
          */
         StrAsc generate_key(StrAsc const &version);
         
      private:
         /**
          * Specifies the model.
          */
         StrAsc model;

         /**
          * Specifies the date of generation.
          */
         LgrDate time_stamp;

         /**
          * Specifies the serial number.
          */
         uint4 serial_no;
      };
   };
};


#endif

