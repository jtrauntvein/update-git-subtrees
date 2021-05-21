/* Csi.SmtpSender.h

   Copyright (C) 2012, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 27 September 2012
   Last Change: Friday 31 March 2017
   Last Commit: $Date: 2019-05-10 15:37:56 -0600 (Fri, 10 May 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SmtpSender_h
#define Csi_SmtpSender_h

#include "Csi.SocketTcpSock.h"
#include "Csi.HttpClient.h"
#include "OneShot.h"
#include "Csi.MsgExcept.h"
#include "StrUni.h"
#include "Csi.LgrDate.h"
#include "Csi.Json.h"
#include "Csi.StrAscStream.h"
#include "Csi.Events.h"
#include <deque>


namespace Csi
{
   /**
    * Defines the application call-back interface for the SmtpSender component.  The application
    * must provide an object that implements this interface to the sender when it is started.
    */
   class SmtpSender;
   class SmtpSenderClient: public InstanceValidator
   {
   public:
      /**
       * Called when the email send operation has been completed.
       *
       * @param sender Specifies the component responsible for this call.
       *
       * @param outcome Specifies a code that describes the outcome of the transaction.
       */
      enum outcome_type
      {
         outcome_unknown_failure = 0,
         outcome_success = 1,
         outcome_connection_failed = 2,
         outcome_authorisation_failed = 3,
         outcome_tls_initialise_failed = 4,
         outcome_data_ack_invalid = 5,
         outcome_receipt_to_ack_invalid = 6,
         outcome_password_ack_invalid = 7,
         outcome_user_name_ack_invalid = 8,
         outcome_auth_ack_invalid = 9,
         outcome_starttls_ack_invalid = 10,
         outcome_helo_ack_invalid = 11,
         outcome_timed_out = 12
      };
      virtual void on_complete(
         SmtpSender *sender,
         outcome_type outcome) = 0;

      /**
       * Can be overloaded to handle a block of low level protocol data that has been sent.
       *
       * @param sender Specifies the component responsible for this call.
       *
       * @param buff Specifies the start of the data that has been transmitted.
       *
       * @param buff_len Specifies the amount of data in the buffer.
       */
      virtual void on_bytes_sent(
         SmtpSender *sender, void const *buff, uint4 buff_len)
      { }

      /**
       * Can be overloaded to handle a block of low level protocol data that has been received.
       *
       * @param sender Specifies the component responsible for this call.
       *
       * @param buff Specifies the start of the block of data that has been received.
       *
       * @param buff_len Specifies the number of bytes in the buffer.
       */
      virtual void on_bytes_received(
         SmtpSender *sender, void const *buff, uint4 buff_len)
      { }

      /**
       * Can be overloaded to handle a logged comment from the protocol.
       *
       * @param sender Specifies the component responsible for this call.
       *
       * @param log Specifies the logged comment.
       */
      virtual void on_log(
         SmtpSender *sender, StrAsc const &log)
      { }

      /**
       * Can be invoked by the application to format the outcom code that was passed through a call
       * to on_complete().
       *
       * @param out Specifies the stream that will receive the formatted outcome.
       *
       * @param outcome Specifies the outcome to be described.
       */
      static void format_outcome(std::ostream &out, outcome_type outcome);
   };


   namespace SmtpHelpers
   {
      /**
       * Formats a list of email addresses as a comma separated list.
       *
       * @return Returns the comma separated list of email addresses.
       *
       * @param addresses Specifies the list of addresses to format.
       */
      typedef std::deque<StrUni> addresses_type;
      inline StrUni format_addresses(addresses_type const &addresses)
      {
         StrUni rtn;
         for(addresses_type::const_iterator ai = addresses.begin();
             ai != addresses.end();
             ++ai)
         {
            if(ai != addresses.begin())
               rtn.append(L", ");
            rtn.append(*ai);
         }
         return rtn;
      }

      /**
       * Parses a comma separated list of email addresses into a container.
       *
       * @param addresses Specfies the container to which the parsed addresses will be written,
       *
       * @param s Specifies the comma separated list of addresses to parse.
       */
      void parse_addresses(addresses_type &addresses, StrUni const &s);

      /**
       * Formats a date and time value to a format that can be accepted by an SMTP server.
       *
       * @param out Specifies the stream to which the value will be written.
       *
       * @param d Specifies the time stamp to format.
       */
      void format_smtp_date(std::ostream &out, LgrDate const &d);

      /**
       * Defines an object that represents an attachment for an email.
       */
      struct attachment_type
      {
         /**
          * Specifies the name of the attachment file.
          */
         StrAsc file_name;

         /**
          * Specifies the content for the attachment (empty if the file must be read.
          */
         StrAsc content;

         /**
          * Specifies the mime-type of the attachment.
          */
         StrAsc content_type;

         /**
          * Specifies the value that will be specified for the content disposition.
          */
         StrAsc content_disposition;
      };
   };
   

   /**
    * Defines a component that can be used by an application to send email messages with optional
    * attachments to one or more destination addresses.  This can be done by either communicating
    * directly with an SMTP server or by sending a web request to the CSI email gateway.  In order
    * to use this component, the application must provide an objet that is derived from class
    * SmtpSenderClient.  It should then create an instance of this class, invoke various methods to
    * set required and optional properties, and then call the start() method to initiate the
    * transfer.  When the transfer to the server is complete, the client's on_complete() method will
    * be called and this component will return to a state where it can be configured to send another
    * message.
    */
   class SmtpSender:
      public SocketTcpSock,
      public HttpClient::RequestClient,
      public OneShotClient
#ifdef WIN32
      , public EventReceiver
#endif
   {
   private:
      /**
       * Specifies the application object that will receive completion notification as well as state
       * notifications.
       */
      SmtpSenderClient *client;

      /**
       * Specifies the SMTP server address.  This value will be ignored when use_gateway has been
       * set to true.
       */
      StrUni server_address;

      /**
       * Specifies the account user name for the SMTP server.  This value will be ignored when
       * use_gateway has been set to true.
       */
      StrUni user_name;

      /**
       * Specifies the account password for the SMTP server.  This value will be ignored when the
       * use_gateway property has been set to true.
       */
      StrUni password;

      /**
       * Specifies the collection of recipient addresses to which the message should be sent.
       */
      typedef SmtpHelpers::addresses_type addresses_type;
      addresses_type to_addresses;

      /**
       * Specifies the collection of CC addresses to which the message will be sent.
       */
      addresses_type cc_addresses;

      /**
       * Specifies the collection of BCC addresses to which the message will be sent.
       */
      addresses_type bcc_addresses;

      /**
       * Specifies the source email address for the message.  This value will be ignored when the
       * use_gateway property has been set to true.
       */
      StrUni from_address;

      /**
       * Specifies the content of the message subject line.
       */
      StrUni subject;
      
      /**
       * Specifies the content of the message body.
       */
      StrUni message;

      /**
       * Specifies the collection of file names that should be included as attachements.
       */
      typedef SmtpHelpers::attachment_type attachment_type;
      typedef std::deque<attachment_type> attachments_type;
      attachments_type attachments;

      /**
       * Specifies the current state of this component.
       */
      enum state_type
      {
         state_standby,
         state_connecting,
         state_wait_220,
         state_ehlo_wait_250,
         state_helo_wait_250,
         state_auth_wait_235,
         state_cram_wait_334,
         state_rcpt_to_wait_250,
         state_data_wait_354,
         state_data_wait_250,
         state_quit_wait_221,
         state_login_name_wait_334,
         state_login_password_wait_334,
         state_starttls_wait_220,
         state_send_http
      } state;

      /**
         8 Set to true if this component is using TLS.
      */
      bool using_tls;

      /**
       * Used to measure time sensitive state transistions.
       */
      SharedPtr<OneShot> timer;

      /**
       * Specifies the timer identifier.
       */
      uint4 timeout_id;

      /**
       * Buffers the content of the current response being received.
       */
      StrAsc last_response;

      /**
       * Set to true if the last response buffer should be cleared.
       */
      bool clear_last_response;

      /**
       * Specifies the index of the current to address.
       */
      uint4 to_index;

      /**
       * Specifies the index of the current cc address.
       */
      uint4 cc_index;

      /**
       * Specifies the index of the current bcc address.
       */
      uint4 bcc_index;

      /**
       * Used to buffer the current message so that we can report its length.
       */
      Csi::OStrAscStream message_out;

      /**
       * Set to true to indicate that this component should use the CSI email gateway service.
       */
      bool use_gateway;

      /**
       * Specifies the object that maintains the connection with the email gateway HTTP server.
       */
      SharedPtr<HttpClient::Connection> http_connection;

      /**
       * Specifies the object that will be used to carry out a particular request to the email
       * gateway HTTP server.
       */
      SharedPtr<HttpClient::Request> http_request;

      /**
       * Specifies the JSON document that will be sent to the email gateway server as the request
       * payload.
       */
      Json::ObjectHandle gateway_request_json;
      
   public:
      /**
       * Constructor
       *
       * @param timer_ Specifies the object that will provide timing services for this component.
       * If not specified or null, this component will create its own timer.
       */
      SmtpSender(SharedPtr<OneShot> timer_ = 0):
         state(state_standby),
         client(0),
         using_tls(false),
         timeout_id(0),
         clear_last_response(false),
         use_gateway(false)
      {
         if(timer == 0)
            timer.bind(new OneShot);
      }

      /**
       * Destructor
       */
      virtual ~SmtpSender()
      {
         if(timeout_id != 0)
            timer->disarm(timeout_id);
         timer.clear();
         close();
      }

      // @group: properties

      /**
       * @return Returns the SMTP server address.
       */
      StrUni const &get_server_address() const
      { return server_address; }

      /**
       * @param address Specifies the SMTP server address.
       *
       * @throws std::exception Thrown if the component has already been started.
       */
      void set_server_address(StrUni const &address)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         server_address = address;
      }

      /**
       * @return Returns the SMTP server account user name.
       */
      StrUni const &get_user_name() const
      { return user_name; }

      /**
       * @param name Specifies the SMTP server account user name.
       *
       * @throws std::exception Thrown if the the component has already been started.
       */
      void set_user_name(StrUni const &name)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         user_name = name;
      }

      /**
       * @return Returns the SMTP server account password.
       */
      StrUni const &get_password() const
      { return password; }

      /**
       * @param password_ Specifies the SMTP server account password.
       *
       * @throws std::exception Thrown if the component has already been started.
       */
      void set_password(StrUni const &password_)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         password = password_;
      }

      /**
       * @return Returns the SMTP sender address.
       */
      StrUni const &get_from_address() const
      { return from_address; }

      /**
       * @param address Specifies the SMTP sender address.
       */
      void set_from_address(StrUni const &address)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         from_address = address;
      }

      /**
       * @return Returns the list of recipient addresses formatted as a comma separated list.
       */
      StrUni get_to_addresses() const
      { return SmtpHelpers::format_addresses(to_addresses); }

      /**
       * @param s Specifies a list of recepient addresses formatted as a comma separated list.
       */
      void set_to_addresses(StrUni const &s)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         SmtpHelpers::parse_addresses(to_addresses, s);
      }

      /**
       * @return Returns the CC addresses formatted as a comma separated list.
       */
      StrUni get_cc_addresses() const
      { return SmtpHelpers::format_addresses(cc_addresses); }

      /**
       * @param s Specifies the list of cc address formatted as a comma separated list.
       */
      void set_cc_addresses(StrUni const &s)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         SmtpHelpers::parse_addresses(cc_addresses, s);
      }

      /**
       * @return Returns the list of bcc addresses formatted as a comma separated list.
       */
      StrUni get_bcc_addresses() const
      { return SmtpHelpers::format_addresses(bcc_addresses); }
         
      /**
       * @param s Specifies the list of bcc addresses formatted as a comma separated list.
       */
      void set_bcc_addresses(StrUni const &s)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         SmtpHelpers::parse_addresses(bcc_addresses, s);
      }

      /**
       * @return Returns the email subject line.
       */
      StrUni const &get_subject() const
      { return subject; }
         
      /**
       * @param s Specifies the email subject line.
       */
      void set_subject(StrUni const &s)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         subject = s;
      }

      /**
       * @return Returns the email subject body.
       */
      StrUni const &get_message() const
      { return message; }

      /**
       * @param s Specifies the email message body.
       */
      void set_message(StrUni const &s)
      {
         // we cannot allow the message body to contain an EOM sequence.  We will slightly change it
         // to avoid this.
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         message = s;
         while(message.length() > 0 && iswspace(message.last()))
            message.cut(message.length() - 1);
      }

      /**
       * @return Returns the list of attachment file names.
       */
      attachments_type &get_attachments()
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         return  attachments;
      }

      /**
       * Adds an attachment to the list to be sent with the message.
       *
       * @param file_name Specifies the name and path of the attachment.
       *
       * @param content_type Specifies the content type of the attachment (empty if it should be
       * automatically assigned).
       *
       * @param content Specifies the content for the attachment (empty if the file must be read.).
       *
       * @param content_disposition Specifies the content disposition for this attachment.  If
       * empty, it will default to "attachment".
       */
      void add_attachment(
         StrAsc const &file_name,
         StrAsc const &content_type = "",
         StrAsc const &content = "",
         StrAsc const &content_disposition = "");

      /**
       * Removes any attachments that should be sent.
       */
      void clear_attachments()
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         attachments.clear();
      }

      /**
       * @return Returns true if this component should use the email gateway.
       */
      bool get_use_gateway() const
      { return use_gateway; }

      /**
       * @param value Set to true if the component should use the email gateway.
       */
      void set_use_gateway(bool value)
      {
         if(state != state_standby)
            throw Csi::MsgExcept("cannot change a started component");
         use_gateway = value;
      }
      
      // @endgroup:

      /**
       * @return Returns the product name that will be sent in the SMTP header.
       */
      static StrUni const &get_product_name();

      /**
       * @param name Specifies the product name that will be sent in the SMTP header.
       */
      static void set_product_name(StrUni const &name);

      /**
       * @return Returns the product version that will be sent in the SMTP header.
       */
      static StrUni const &get_product_version();

      /**
       * @param version Specifies the product version that will be sent in the SMTP header.
       */
      static void set_product_version(StrUni const &version);

      /**
       * @param value Specifies the model that will be used to send mails through the CSI email
       * gateway service.
       */
      static void set_gateway_model(StrUni const &value);

      /**
       * @return Returns the model that will be used to send emails through the CSI email gateway
       * service.
       */
      static StrUni const &get_gateway_model();

      /**
       * @param value Specifies the serial number that will be used to send emails through the CSI
       * email gateway service.
       */
      static void set_gateway_serial_no(StrUni const &value);

      /**
       * @return Returns the serial number that will be used to send emails through the CSI  email
       * gateway service.
       */
      static StrUni const &get_gateway_serial_no();
      
      /**
       * Starts the process of sending the email.  If this is called while the component is already
       * active, a std::exception derived object will be thrown.
       *
       * @param client_ Specifies the application object that will receive a completion notification
       * as well as status notifications.
       */
      typedef SmtpSenderClient client_type;
      void start(client_type *client_);

      /**
       * Overloads the base class version to handle the case where the connection to the SMTP server
       * has succeeded.
       */
      virtual void on_connected(SocketAddress const &sonnected_address);

      /**
       * Overloads the base class version to handle received data.
       */
      virtual void on_read();

      /**
       * Overloads the base class version to write data.
       */
      virtual void write(void const *buff, uint4 buff_len);

      /**
       * Overloads the base class version to handle the case where the TLS connection is ready.
       */
      virtual void on_tls_client_ready();

      /**
       * Overloads the base class version to handle the case where the connection has failed.
       */
      virtual void on_socket_error(int err);

      /**
       * Overloads the base class version to handle timer events.
       */
      virtual void onOneShotFired(uint4 id);

      /**
       * Handles an asynchronous event.
       */
      virtual void receive(SharedPtr<Event> &ev);

      /**
       * Formats the message portion to the specified stream.
       */
      void format_message(std::ostream &out);

      /**
       * Formats the mail from address to the specified stream.
       */
      void format_mail_from(std::ostream &out);

      /**
       * Overloads the base class version to handle the notification that the request was connected.
       */
      virtual void on_connected(HttpClient::Request *sender);

      /**
       * Overloads the base class version to handle the notification where the request header has
       * been sent and the request is ready for the body.
       */
      virtual void on_header_sent(HttpClient::Request *sender);
      
      /**
       * Overloads the base class version to handle the notification that the response header has
       * been received.
       */
      virtual void on_response_header(HttpClient::Request *sender);

      /**
       * Overloads the base class version to handle the event where a portion of the response has
       * been received.
       */
      virtual void on_response_data(HttpClient::Request *sender);
      
      /**
       * Overloads the base class to handle a failure from the HTTP client component.
       */
      virtual bool on_failure(HttpClient::Request *sender);

      /**
       * Overloads the base class to handle a notification that the response is complete.
       */
      virtual bool on_response_complete(HttpClient::Request *sender);
      
   private:
      /**
       * Reports completion to the client and resets the state of this component.
       */
      void on_complete(client_type::outcome_type outcome);

      
   };
};


#endif
