/* Cora.DataSources.CsiDbHelpers.DbProperties.h

   Copyright (C) 2009, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 07 August 2009
   Last Change: Thursday 19 January 2017
   Last Commit: $Date: 2018-12-04 15:06:32 -0600 (Tue, 04 Dec 2018) $
   Last Changed by: $Author: alex $

*/

#pragma once
#ifndef Cora_DataSources_CsiDbHelpers_DbProperties_h
#define Cora_DataSources_CsiDbHelpers_DbProperties_h

#include "Csi.Xml.Element.h"


namespace Cora
{
   namespace DataSources
   {
      namespace CsiDbHelpers
      {
         /**
          * Defines an object that represents the properties used for maintaing and connecting to a
          * database source.
          */
         class DbProperties
         {
         public:
            /**
             * Constructor
             */
            DbProperties();

            /**
             * Copy constructor
             */
            DbProperties(DbProperties const &other);

            /**
             * Destructor
             */
            ~DbProperties();

            /**
             * @return Returns the database source type.
             */
            enum db_type_code
            {
               db_type_mysql = 1,
               db_type_sql_server = 2,
               db_type_sql_server_compact = 3,
               db_type_postgresql = 4,
               db_type_oracle = 5
            };
            db_type_code get_db_type() const
            { return db_type; }

            /**
             * @return Returns true if the connection to a SQL server should use windows account
             * authentication.
             */
            bool get_db_use_windows_authentication() const
            { return db_use_windows_authentication; }

            /**
             * @return Returns true if the user identifier should be saved.
             */
            bool get_db_save_user_id() const
            { return db_save_user_id; }
            
            /**
             * @return Returns the database user identifier.
             */
            StrAsc const &get_db_user_id() const
            { return db_user_id; }

            /**
             * @return Returns the database user account password.
             */
            StrAsc const &get_db_password() const
            { return db_password; }

            /**
             * @return Returns the database source string.
             */
            StrAsc const &get_db_data_source() const
            { return db_data_source; }

            /**
             * @return Returns the initial catalog that should be used.
             */
            StrAsc const &get_db_initial_catalog() const
            { return db_initial_catalog; }

            /**
             * @return Returns the interval at which cursors whould be polled for new data.
             */
            uint4 get_poll_interval() const
            { return poll_interval; }

            /**
             * @return Generates the connection string.
             */
            StrAsc make_connect_str() const;

            /**
             * Writes these parameters to the specified XML structure.
             */
            void get_properties(Csi::Xml::Element &prop_xml);

            /**
             * Reads these parameters from the specified XML structure.
             */
            void set_properties(Csi::Xml::Element &prop_xml);

            /**
             * @return Rounds the time stamp up to the nearest unit for the given database type.
             *
             * @param time Specifies the time to round.
             */
            Csi::LgrDate round_timestamp(Csi::LgrDate const &time) const;

            /**
             * @return Returns true if the meta-data for a table should be polled for data requests.
             */
            bool get_should_poll_meta() const
            { return should_poll_meta; }
            
         private:
            /**
             * Specifies the type of database connection.
             */
            db_type_code db_type;

            /**
             * set to true if windows authentication should be used.
             */
            bool db_use_windows_authentication;

            /**
             * Set ti true if user identification should be saved.
             */
            bool db_save_user_id;
            
            /**
             * Specifies the user identifier.
             */
            StrAsc db_user_id;

            /**
             * Specifies the user password.
             */
            StrAsc db_password;

            /**
             * Specifies the string that identifies the data source.
             */
            StrAsc db_data_source;

            /**
             * Specifies the initial catalog.
             */
            StrAsc db_initial_catalog;

            /**
             * Specifies the interval at which cursors will be refreshed.
             */
            uint4 poll_interval;

            /**
             * Set to true if the meta table should be polled for data requests.
             */
            bool should_poll_meta;
         };
      };
   };
};


#endif
