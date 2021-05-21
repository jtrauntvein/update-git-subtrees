/* Cora.Device.FileLister.h

   Copyright (C) 2002, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 April 2002
   Last Change: Saturday 21 April 2018
   Last Commit: $Date: 2018-04-21 13:02:22 -0600 (Sat, 21 Apr 2018) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_FileLister_h
#define Cora_Device_FileLister_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"
#include "Csi.MaxMin.h"
#include <deque>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class FileLister;
      //@endgroup


      namespace FileListerHelpers
      {
         /**
          * Defines the name and attributes of a file on the datalogger.
          */
         class file_type
         {
         private:
            /**
             * Specifies the name and location of the file.
             */
            StrAsc name;

            // @group: attributes each of the following members can reported as
            // attributes to the file.  The attr_xxx_exists flag will indicate
            // whether a particular attribute was read for that file.

            /**
             * Set to true if the file is a program that is marked to be the current datalogger
             * program.
             */
            bool attr_run_now;
            bool attr_run_now_exists;

            /**
             * Set to true if the file is a program that is marked to be the program that the
             * datalogger will run on power up.
             */
            bool attr_run_on_power_up;
            bool attr_run_on_power_up_exists;

            /**
             * Specifies the size of the file on the datalogger.
             */
            int8 attr_file_size;
            bool attr_file_size_exists;

            /**
             * Specifies the formatted time when the file was created or last written on the
             * datalogger.
             */
            StrAsc attr_last_update;
            bool attr_last_update_exists;

            /**
             * Set to true if the file is marked as read-only.
             */
            bool attr_read_only;
            bool attr_read_only_exists;

            /**
             * Set to true if the file is a program that would be running but is in a paused state.
             */
            bool attr_paused;
            bool attr_paused_exists;

            friend class Cora::Device::FileLister;
            
         public:
            /**
             * Default constructor
             */
            file_type();

            /**
             * Copy constructor
             *
             * @param other specifies the file description to copy.
             */
            file_type(file_type const &other);

            /**
             * Copy operator
             *
             * @param other Specifies the file dscription to copy.
             *
             * @return Returns a reference to this description.
             */
            file_type &operator =(file_type const &other)
            {
               name = other.name;
               attr_run_now = other.attr_run_now;
               attr_run_now_exists = other.attr_run_now_exists;
               attr_run_on_power_up = other.attr_run_on_power_up;
               attr_run_on_power_up_exists = other.attr_run_on_power_up_exists;
               attr_file_size = other.attr_file_size;
               attr_file_size_exists = other.attr_file_size_exists;
               attr_last_update = other.attr_last_update;
               attr_last_update_exists = other.attr_last_update_exists;
               attr_read_only = other.attr_read_only;
               attr_read_only_exists = other.attr_read_only_exists;
               attr_paused = other.attr_paused;
               attr_paused_exists = other.attr_paused_exists;
               return *this;
            }

            /**
             * @return Returns true if the file name is the same as that in the other.
             *
             * @param otheer Specifies the description to compare.
             */
            bool operator ==(file_type const &other) const
            { return compare(other) == 0; }

            /**
             * @return Returns true if the file name is less than the name of the other.
             *
             * @param other Specifies the file to compare.
             */
            bool operator <(file_type const &other) const
            { return compare(other) < 0; }

            /**
             * @return Returns true oif the file name is less than or equal to the other.
             */
            bool operator <=(file_type const &other) const
            { return compare(other) <= 0; }

            /**
             * @return Returns true if the file name is greater than the other.
             */
            bool operator >(file_type const &other) const
            { return compare(other) > 0; }

            /**
             * @return Returns true if the file name is greater than or equal to the other.
             */
            bool operator >=(file_type const &other) const
            { return compare(other) >= 0; }

            /**
             * @return Returns a negative value if this file name is less than the other, 0 if the
             * names are equal, or a positive value if this name is less than the other.
             */
            int compare(file_type const &other) const
            { return name.compare(other.name, false); }

            /**
             * @return Returns the file name.
             */
            StrAsc const &get_name() const
            { return name; }

            /**
             * @param value Specifies the file name.
             */
            void set_name(StrAsc const &value)
            { name = value; }

            /**
             * @return Returns the name of the device on which the file is stored.
             */
            StrAsc get_device() const
            {
               StrAsc rtn;
               name.sub(rtn, 0, name.find(":"));
               return rtn;
            }

            /**
             * @return Returns only the name of the file without the device on which it is stored.
             */
            StrAsc get_file_name() const
            {
               StrAsc rtn;
               size_t sep_pos(name.find(":"));
               name.sub(rtn, sep_pos + 1, name.length());
               return rtn;
            }

            // @group: attributes access methods
            // each of these methods is defined such that the actual value is passed as a reference
            // in the parameter and the return value will indicate whether the attribute is
            // supported.

            /**
             * @return Returns true if the run now attribute exists.
             *
             * @param rtn Specifies the reference that will be reflect the value of run now,
             */
            bool get_attr_run_now(bool &rtn) const
            { rtn = attr_run_now;  return attr_run_now_exists; }

            /**
             * @return Returns a pair of values.  The first of this pair will specify the attribute
             * value while the second will be true if the attribute was reported.
             */
            std::pair<bool, bool> get_attr_run_now() const
            { return std::make_pair(attr_run_now, attr_run_now_exists); }

            /**
             * @param value Set to true if the run now attribute is set.
             */
            void set_attr_run_now(bool value)
            {
               attr_run_now = value;
               attr_run_now_exists = true;
            }

            /**
             * @return Returns true if the attribute was reported.
             *
             * @param rtn Specifies the reference that will be set.
             */
            bool get_attr_run_on_power_up(bool &rtn) const
            { rtn = attr_run_on_power_up;  return attr_run_on_power_up_exists; }

            /**
             * @return Returns a pair of values.  The first will specify the attribute while the
             * second will be true if the attribute was reported.
             */
            std::pair<bool, bool> get_attr_run_on_power_up() const
            { return std::make_pair(attr_run_on_power_up, attr_run_on_power_up_exists); }

            /**
             * @param value Specifies the value fo the run on power up attribute.
             */
            void set_attr_run_on_power_up(bool value)
            {
               attr_run_on_power_up = value;
               attr_run_on_power_up_exists = true;
            }

            /**
             * @return Returns true if the file size attribute was reported.
             *
             * @param file_size Specifies the reference that will be written.
             */
            bool get_attr_file_size(int8 &file_size) const
            {
               file_size = attr_file_size;
               return attr_file_size_exists;
            }
            bool get_attr_file_size(uint4 &file_size) const
            {
               file_size = static_cast<uint4>(Csi::csimin(attr_file_size, static_cast<int8>(0xFFFFFFFF)));
               return attr_file_size_exists;
            }

            /**
             * @return Returns a pair of values that will report the file size and whether the
             * attribute was reported.
             */
            std::pair<int8, bool> get_attr_file_size() const
            { return std::make_pair(attr_file_size, attr_file_size_exists); }

            /**
             * @param value Specifies the file size.
             */
            void set_attr_file_size(int8 value)
            {
               attr_file_size = value;
               attr_file_size_exists = true;
            }

            /**
             * @return Returns true if the last update attribute was reported.
             *
             * @param last_update Specifies the reference that will be changed.
             */
            bool get_attr_last_update(StrAsc &last_update) const
            { last_update = attr_last_update; return attr_last_update_exists; }

            /**
             * @return Returns a pair of values that identify the last changed attribute and whether
             * the attribute was reported.
             */
            std::pair<StrAsc, bool> get_attr_last_update() const
            { return std::make_pair(attr_last_update, attr_last_update_exists); }

            /**
             * @param value Specifies the value of the last update attribute.
             */
            void set_attr_last_update(StrAsc const &value)
            {
               attr_last_update = value;
               attr_last_update_exists = true;
            }

            /**
             * @return Returns true if the read only attribute was reported.
             *
             * @param read_only Specifies the reference that will be changed.
             */
            bool get_attr_read_only(bool &read_only) const
            { read_only = attr_read_only; return attr_read_only_exists; }

            /**
             * @return Returns a pair of values that identify the value of the attribute and whether
             * the attribute was reported.
             */
            std::pair<bool, bool> get_attr_read_only() const
            { return std::make_pair(attr_read_only, attr_read_only_exists); }

            /**
             * @param value Specifies the value of the read only attribute.
             */
            void set_attr_read_only(bool value)
            {
               attr_read_only = value;
               attr_read_only_exists = true;
            }
            
            /**
             * @return Returns true if the paused attribute was reported.
             *
             * @param paused Specifies the reference that will be changed.
             */
            bool get_attr_paused(bool &paused) const
            { paused = attr_paused; return attr_paused_exists; }

            /**
             * @return Returns a pair of values that will report the attribute and whether the
             * attribute was reported.
             */
            std::pair<bool, bool> get_attr_paused() const
            { return std::make_pair(attr_paused, attr_paused_exists); }

            /**
             * @param value Specifies the value of the paused attribute.
             */
            void set_attr_paused(bool value)
            {
               attr_paused = value;
               attr_paused_exists = true;
            }

            // @endgroup:
         };
      };
      

      /**
       * Defines the interface that must be implemented by an application object that uses the
       * FileLister component type.
       */
      class FileListerClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction has been completed.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param outcome Specifies the outcome of the transaction.
          *
          * @param files Specifies the collection of files that were reported. This list will be
          * empty if the transaction failed.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_failure = 3,
            outcome_invalid_device_name = 4,
            outcome_blocked_by_server = 5,
            outcome_unsupported = 6,
            outcome_blocked_by_logger = 7,
            outcome_comm_disabled = 8,
            outcome_comm_failed = 9, 
         };
         typedef FileListerHelpers::file_type file_type;
         typedef std::deque<file_type> file_list_type;
         virtual void on_complete(
            FileLister *lister,
            outcome_type outcome,
            file_list_type const &files) = 0;
      };


      /**
       * Defines a component that can be used to get a list of files from a LoggerNet device.  In
       * order to use this component, the application must provide an object that inherits from
       * class FileListerClient.  It must then construct an instance of this class, set its
       * properties including the device name and, optionally, the filter expression, and then call
       * one of the two versions of start().  When the server transaction is complete, the client's
       * on_complete() method will be called to report the results.
       */
      class FileLister:
         public DeviceBase,
         public Csi::EventReceiver
      {
      public:
         /**
          * Constructor
          */
         FileLister();

         /**
          * Destructor
          */
         virtual ~FileLister();

         /**
          * @return Returns the pattern to be used to filter the files list.
          */
         StrAsc const &get_pattern() const
         { return pattern; }

         /**
          * @param pattern_ Specifies the pattern that will be used to filter the files list.
          */
         void set_pattern(StrAsc const &pattern_);

         /**
          * Called to start the server trasnaction.
          *
          * @param client_ Specifies the application object that will receive the completion
          * notification.
          *
          * @param router Specifies a messaging router that has just been created but not used.
          *
          * @param other_component Specifies a component that already has an active connection to
          * the LoggerNet server that this component can share.
          */
         typedef FileListerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         /**
          * Called to release any resources and return this component to a standby state.
          */
         virtual void finish();

         /**
          * Writes a text description of the outcome to the specified stream.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outcome Specifies the outcome code to describe.
          */
         static void describe_outcome(
            std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overloads the base class version to start the transaction.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle a failure notification.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Overloads the base class version to handle a failure of the server connection.
          */
         virtual void on_devicebase_session_failure();

         /**
          * Overloads the base class version to handle asynchronous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Overloads the base clas version to handle an incoming message from the server.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg); 
         
      private:
         /**
          * Specifies the application object that will receive notification when the server
          * transaction is completed.
          */
         client_type *client;

         /**
          * Specifies the current state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         /**
          * Specifies the pattern that will be used to filter the file list.
          */
         StrAsc pattern;
      };
   };
};


#endif
