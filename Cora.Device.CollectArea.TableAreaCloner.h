/* Cora.Device.CollectArea.TableAreaCloner.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 April 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectArea_TableAreaCloner_h
#define Cora_Device_CollectArea_TableAreaCloner_h


#include "Cora.Device.CollectAreasEnumerator.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         //@group class forward declarations
         class TableAreaCloner;
         //@endgroup


         ////////////////////////////////////////////////////////////
         // class TableAreaClonerClient
         ////////////////////////////////////////////////////////////
         class TableAreaClonerClient: public Csi::InstanceValidator
         {
         public:
            ////////////////////////////////////////////////////////////
            // on_started
            //
            // Called when the server transaction has been completed and the area created.  
            ////////////////////////////////////////////////////////////
            virtual void on_started(
               TableAreaCloner *cloner,
               StrUni const &new_area_name)
            { }
            
            ////////////////////////////////////////////////////////////
            // on_failure
            //
            // Called when the server transaction has failed or if the device session fails
            // thereafter. 
            ////////////////////////////////////////////////////////////
            enum failure_type
            {
               failure_unknown = 0,
               failure_invalid_logon = 1,
               failure_session_failed = 2,
               failure_security_blocked = 3,
               failure_unsupported = 4,
               failure_invalid_device_name = 5,
               failure_invalid_source_name = 6,
               failure_area_deleted = 7
            };
            virtual void on_failure(
               TableAreaCloner *cloner,
               failure_type failure) = 0;
         };


         ////////////////////////////////////////////////////////////
         // class TableAreaCloner
         //
         // Defines a coratools component that can be used to create a clone of a table
         // based collect area and to maintain its existance if the area was specified as being
         // temporary.
         //
         // In order to use this component, an application must provide an object derived from class
         // TableAreaClonerClient (also known as TableAreaCloner::client_type), create an instance
         // of this class, invoke the appropriate methods to set properties including
         // set_device_name() and set_source_area_name(), and invoke the start() one of the two
         // versions of start().
         //
         // If the area could be created by the server, the component will call the client's
         // on_started() method after receiving the server acknowledgement and then enter a state
         // where it is maintaining the device session for the transaction.  This is needed in order
         // to maintain the presence of a temporary collect area.  The component can be brought out
         // of that state when the application invokes finish() or deletes the component.  The
         // component will also return to a standby state if the server transaction failed, the
         // device is deleted or renamed, or the server is shut down.  In all circumstances except
         // when finish() is called or the component is deleted, the component will invoke the
         // client's on_failure() notification.  
         ////////////////////////////////////////////////////////////
         class TableAreaCloner:
            public DeviceBase,
            public CollectAreasEnumeratorClient,
            public Csi::EventReceiver
         {
         private:
            //@group properties
            ////////////////////////////////////////////////////////////
            // source_area_name
            //
            // Specifies the name of the collect area that will serve as a "source" for the new
            // collect area.  The table definitions and settings for the new area will be copied
            // from the "source" area.  This value must refer to a collect area that already exists
            // for the station.
            ////////////////////////////////////////////////////////////
            StrUni source_area_name;

            ////////////////////////////////////////////////////////////
            // new_area_name
            //
            // Specifies the seed that will be used to generate the new collect
            // area name.  If this value is not specified, the server will
            // generate a name based upon the old area name.  The actual new
            // name used by the server will be returned in the on_started()
            // notification to the client.
            ////////////////////////////////////////////////////////////
            StrUni new_area_name;

            ////////////////////////////////////////////////////////////
            // is_permanent
            //
            // Should be set to true (via set_is_permanent()) if the new area
            // is to outlast the session that created it.
            ////////////////////////////////////////////////////////////
            bool is_permanent;

            ////////////////////////////////////////////////////////////
            // selectors
            ////////////////////////////////////////////////////////////
            typedef std::list<StrUni> selectors_type;
            selectors_type selectors;

            ////////////////////////////////////////////////////////////
            // copy_option
            ////////////////////////////////////////////////////////////
         public:
            enum copy_option_type
            {
               copy_at_record = 1,
               copy_at_time = 2,
               copy_at_newest = 3,
               copy_none = 4,
               copy_time_relative = 5
            };
         private:
            copy_option_type copy_option;

            ////////////////////////////////////////////////////////////
            // file_mark_no
            ////////////////////////////////////////////////////////////
            uint4 file_mark_no;

            ////////////////////////////////////////////////////////////
            // record_no
            ////////////////////////////////////////////////////////////
            uint4 record_no;

            ////////////////////////////////////////////////////////////
            // nsec
            ////////////////////////////////////////////////////////////
            int8 nsec;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            TableAreaCloner();

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~TableAreaCloner();

            ////////////////////////////////////////////////////////////
            // set_source_area_name
            ////////////////////////////////////////////////////////////
            void set_source_area_name(StrUni const &source_area_name_);

            ////////////////////////////////////////////////////////////
            // get_source_area_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_source_area_name() const
            { return source_area_name; }

            ////////////////////////////////////////////////////////////
            // set_new_area_name
            ////////////////////////////////////////////////////////////
            void set_new_area_name(StrUni const &new_area_name_);

            ////////////////////////////////////////////////////////////
            // get_new_area_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_new_area_name() const
            { return new_area_name; }

            ////////////////////////////////////////////////////////////
            // set_is_permanent
            ////////////////////////////////////////////////////////////
            void set_is_permanent(bool is_permanent_);

            ////////////////////////////////////////////////////////////
            // get_is_permanent
            ////////////////////////////////////////////////////////////
            bool get_is_permanent() const
            { return is_permanent; }

            ////////////////////////////////////////////////////////////
            // get_copy_option
            ////////////////////////////////////////////////////////////
            copy_option_type get_copy_option() const
            { return copy_option; }

            ////////////////////////////////////////////////////////////
            // set_copy_option
            ////////////////////////////////////////////////////////////
            void set_copy_option(
               copy_option_type copy_option_,
               uint4 file_mark_no_ = 0,
               uint4 record_no_ = 0,
               int8 nsec_ = 0); 
            
            ////////////////////////////////////////////////////////////
            // add_column
            //
            // Adds the specified column name and optional subscript to the
            // list of names that will be sent to the server if it supports
            // interface version 1.3.6.5 and newer.  If no names are added, or
            // if the server does not support the extended clone table area
            // transaction, the entire table will be cloned.
            //
            // The string specified should conform to the following syntax:
            //
            //  selector := column-name ["(" subscript { "," subscript } ")"].
            //  column-name := string.
            //  subscript := integer.
            //
            // If the table column being specified is an array and the
            // application does not specify the subscript list for that
            // selector, the server will select the entire column.  If the
            // application specifies the subscript list for a column, the
            // number of subscripts specified should match the number of array
            // dimensions defined for that column in the server's table
            // definitions. 
            ////////////////////////////////////////////////////////////
            void add_column(StrUni const &selector);

            ////////////////////////////////////////////////////////////
            // clear_columns
            ////////////////////////////////////////////////////////////
            void clear_columns();
            
            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            typedef TableAreaClonerClient client_type;
            void start(
               client_type *client_,
               router_handle &router);

            ////////////////////////////////////////////////////////////
            // start (other component)
            ////////////////////////////////////////////////////////////
            void start(
               client_type *client_,
               ClientBase *other_component);

            ////////////////////////////////////////////////////////////
            // finish
            ////////////////////////////////////////////////////////////
            virtual void finish();

         protected:
            ////////////////////////////////////////////////////////////
            // on_devicebase_ready
            ////////////////////////////////////////////////////////////
            virtual void on_devicebase_ready();

            ////////////////////////////////////////////////////////////
            // on_devicebase_failure
            ////////////////////////////////////////////////////////////
            virtual void on_devicebase_failure(devicebase_failure_type failure);

            ////////////////////////////////////////////////////////////
            // on_devicebase_session_failure
            ////////////////////////////////////////////////////////////
            virtual void on_devicebase_session_failure();

            ////////////////////////////////////////////////////////////
            // onNetMessage
            ////////////////////////////////////////////////////////////
            virtual void onNetMessage(
               Csi::Messaging::Router *rtr,
               Csi::Messaging::Message *msg);

            ////////////////////////////////////////////////////////////
            // receive
            ////////////////////////////////////////////////////////////
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

            //@group methods overloaded from class CollectAreasEnumeratorClient
            ////////////////////////////////////////////////////////////
            // on_area_deleted
            ////////////////////////////////////////////////////////////
            virtual void on_area_deleted(
               CollectAreasEnumerator *lister,
               StrUni const &area_name);

            ////////////////////////////////////////////////////////////
            // on_failure
            ////////////////////////////////////////////////////////////
            virtual void on_failure(
               CollectAreasEnumerator *lister,
               failure_type failure);
            //@endgroup

         private:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            client_type *client;

            ////////////////////////////////////////////////////////////
            // state
            ////////////////////////////////////////////////////////////
            enum state_type
            {
               state_standby,
               state_delegate,
               state_active,
            } state;

            ////////////////////////////////////////////////////////////
            // lister
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<CollectAreasEnumerator> lister;
         };
      };
   }; 
};


#endif
