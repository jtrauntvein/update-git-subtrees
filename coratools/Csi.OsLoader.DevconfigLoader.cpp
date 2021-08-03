/* Csi.OsLoader.DevconfigLoader.cpp

   Copyright (C) 2008, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 30 April 2008
   Last Change: Tuesday 19 September 2017
   Last Commit: $Date: 2017-09-19 10:49:29 -0600 (Tue, 19 Sep 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsLoader.DevconfigLoader.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include "coratools.strings.h"
#include "boost/format.hpp"


namespace Csi
{
   namespace OsLoader
   {
      using namespace DevconfigLoaderStrings;

      
      namespace DevconfigLoaderHelpers
      {
         /**
          * Defines a driver that implements the I/O in terms of the devconfig session.
          */
         class MyDriver: public Csi::DevConfig::SessionDriverBase
         {
         public:
            /**
             * Constructor
             */
            MyDriver(OsLoaderDriver *os_driver_):
               os_driver(os_driver_)
            { }

            /**
             * Destructor
             */
            virtual ~MyDriver()
            { }

            /**
             * Overloads the base class version as a no-op.
             */
            virtual void start_open(DevConfig::Session *session)
            { }

            /**
             * @return Overloads the base class version to indicate that the driver is always open.
             */
            virtual bool is_open(DevConfig::Session *session)
            { return true; }

            /**
             * Overloads the base class version as a no-op.
             */
            virtual void close(DevConfig::Session *session)
            { }

            /**
             * Overloads the base class version to transmit the specified data.
             */
            virtual void send(DevConfig::Session *session, void const *buff, uint4 buff_len)
            { os_driver->send(buff, buff_len); }
            
         private:
            /**
             * Specifies the operating system loader driver.
             */
            OsLoaderDriver *os_driver;
         };
      };


      DevconfigLoader::DevconfigLoader(timer_handle timer_):
         timer(timer_),
         bytes_sent(0),
         bytes_to_send(0),
         os_image(0),
         state(state_standby)
      { }

      DevconfigLoader::~DevconfigLoader()
      {
         if(os_file != 0)
         {
            os_file.clear();
            os_image = 0;
         }
         session.clear();
         driver.clear();
      } // destructor


      void DevconfigLoader::open_and_validate(char const *os_file_name)
      {
         // we will first attempt to open the file mapping.
         Csi::get_name_from_file_path(this->os_file_name, os_file_name);
         os_file.bind(new Csi::ReadFileMapping(os_file_name));
         bytes_to_send = static_cast<uint4>(os_file->file_size());
         bytes_sent = 0;
         os_image = static_cast<byte const *>(os_file->open_view(0, bytes_to_send));
      } // open_and_validate


      void DevconfigLoader::start_send(
         driver_handle os_driver, EventReceiver *client)
      {
         if(state == state_standby)
         {
            // we need to first create the bridge driver.  Once that is created, we can create the
            // session and then start the transaction
            driver.bind(new DevconfigLoaderHelpers::MyDriver(os_driver.get_rep()));
            session.bind(new Csi::DevConfig::Session(driver.get_handle(), timer));
            OsLoaderBase::start_send(os_driver, client);

            // before sending the file, we will need to make sure that the session is started.  We
            // will do this by sending a reset command to the modem
            message_handle reset_cmd(new Csi::DevConfig::Message);
            reset_cmd->set_message_type(Csi::DevConfig::Messages::control_cmd);
            reset_cmd->addUInt2(0); // security code
            reset_cmd->addByte(Csi::DevConfig::ControlCodes::action_refresh_timer); 
            state = state_reset;
            session->add_transaction(this, reset_cmd, 20, 500);
         }
      } // start_send


      void DevconfigLoader::cancel_send()
      { }


      void DevconfigLoader::on_receive(
         OsLoaderDriver *driver, void const *buff, uint4 buff_len)
      { session->on_driver_data(buff, buff_len); }


      void DevconfigLoader::on_driver_error(
         OsLoaderDriver *driver, char const *error_message)
      {
         FileSender::do_on_complete(outcome_link_failed);
         OsLoaderBase::on_driver_error(driver, error_message);
      } // on_driver_error


      void DevconfigLoader::get_next_fragment(
         StrBin &send_buffer,
         bool &last_fragment,
         uint4 max_len,
         uint4 offset)
      {
         uint4 len = max_len;
         if(offset + max_len > bytes_to_send)
         {
            len = bytes_to_send - offset;
            last_fragment = true;
         }
         send_buffer.setContents(os_image + offset, len);
         bytes_sent = offset;
         if(offset > 0)
         {
            Csi::OStrAscStream message;
            message << boost::format(my_strings[strid_fragment_sent].c_str()) % bytes_sent % bytes_to_send;
            on_status(message.str().c_str(), bytes_sent, bytes_to_send);
         }
      } // get_next_fragment


      void DevconfigLoader::on_complete(
         outcome_type outcome,
         bool will_reset,
         uint2 wait_after,
         StrAsc const &explanation)
      {
         session.clear();
         driver.clear();
         state = state_standby;
         if(outcome == outcome_success)
         {
            OStrAscStream message;
            message << boost::format(my_strings[strid_fragment_sent].c_str()) % bytes_to_send % bytes_to_send;
            on_status(message.str().c_str(), bytes_to_send, bytes_to_send);
            message.str("");
            if(admonition.length() == 0)
               message << boost::format(my_strings[strid_send_complete].c_str()) % wait_after;
            else
               message << admonition;
            OsLoaderBase::on_complete(message.str().c_str(), true, true);
         }
         else
         {
            Csi::OStrAscStream reason;
            Csi::OStrAscStream message;
            describe_outcome(reason, outcome, explanation);
            message << boost::format(my_strings[strid_send_failed].c_str()) % reason.str();
            OsLoaderBase::on_complete(message.str().c_str(), false);
         }
      } // on_complete


      void DevconfigLoader::on_complete(
         message_handle &command, message_handle &response)
      {
         switch(state)
         {
         case state_reset:
            state = state_send_file;
            start(os_file_name, session.get_handle());
            break;
            
         case state_send_file:
            FileSender::on_complete(command, response);
            break;
         }
      } // on_complete


      void DevconfigLoader::on_failure(
         message_handle &command, failure_type failure)
      {
         switch(state)
         {
         case state_reset:
            on_complete(outcome_timed_out, false, 0);
            break;
            
         case state_send_file:
            FileSender::on_failure(command, failure);
            break;
         }
      } // on_failure
   };
};

