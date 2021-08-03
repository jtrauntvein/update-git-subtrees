/* Cora.DataSources.CsiDbHelpers.DbProperties.cpp

   Copyright (C) 2009, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 07 August 2009
   Last Change: Thursday 19 January 2017
   Last Commit: $Date: 2018-12-04 15:08:45 -0600 (Tue, 04 Dec 2018) $
   Last Changed by: $Author: alex $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.CsiDbHelpers.DbProperties.h"
#include "Cora.DataSources.CsiDbSource.h"


namespace Cora
{
   namespace DataSources
   {
      namespace CsiDbHelpers
      {
         ////////////////////////////////////////////////////////////
         // class DbProperties definitions
         ////////////////////////////////////////////////////////////
         DbProperties::DbProperties():
            db_type(db_type_sql_server_compact),
            poll_interval(300000),
            db_use_windows_authentication(true),
            db_save_user_id(true),
            should_poll_meta(false)
         { }


         DbProperties::DbProperties(DbProperties const &other):
            db_type(other.db_type),
            db_data_source(other.db_data_source),
            db_user_id(other.db_user_id),
            db_password(other.db_password),
            db_initial_catalog(other.db_initial_catalog),
            poll_interval(other.poll_interval),
            db_use_windows_authentication(other.db_use_windows_authentication),
            db_save_user_id(other.db_save_user_id),
            should_poll_meta(other.should_poll_meta)
         { }
         

         DbProperties::~DbProperties()
         { }


         StrAsc DbProperties::make_connect_str() const
         {
            Csi::OStrAscStream rtn;
            rtn << "DBType=" << db_type;
            switch(db_type)
            {
               case db_type_mysql:
                  if(db_user_id.length())
                     rtn << ";User ID=\"" << db_user_id << "\"";
                  if(db_password.length())
                     rtn << ";Password=\"" << db_password << "\"";
                  if(db_initial_catalog.length())
                     rtn << ";Initial Catalog=\"" << db_initial_catalog << "\"";
                  rtn << ";Data Source=\"" << db_data_source << "\"";
                  break;
                  
               case db_type_sql_server:
                  if(!db_use_windows_authentication)
                  {
                     if(db_user_id.length())
                        rtn << ";User ID=\"" << db_user_id << "\"";
                     else
                        rtn << ";User ID=\" \""; //Issue #26153

                     if(db_password.length())
                        rtn << ";Password=\"" << db_password << "\"";
                  }
                  if(db_initial_catalog.length())
                     rtn << ";Initial Catalog=\"" << db_initial_catalog << "\"";
                  rtn << ";Data Source=\"" << db_data_source << "\"";
                  break;
                  
               case db_type_sql_server_compact:
                  rtn << ";Data Source=\"" << db_data_source << "\"";
                  break;

               case db_type_postgresql:
                  if (db_user_id.length())
                     rtn << ";User ID=\"" << db_user_id << "\"";
                  if (db_password.length())
                     rtn << ";Password=\"" << db_password << "\"";
                  if (db_initial_catalog.length())
                     rtn << ";Initial Catalog=\"" << db_initial_catalog << "\"";
                  rtn << ";Data Source=\"" << db_data_source << "\"";
                  break;

               case db_type_oracle:
                  if (db_user_id.length())
                     rtn << ";User ID=\"" << db_user_id << "\"";
                  if (db_password.length())
                     rtn << ";Password=\"" << db_password << "\"";
                  if (db_initial_catalog.length())
                     rtn << ";Initial Catalog=\"" << db_initial_catalog << "\"";
                  rtn << ";Data Source=\"" << db_data_source << "\"";
                  break;
            }
            return rtn.str();
         } // make_connect_str


         void DbProperties::get_properties(Csi::Xml::Element &prop_xml)
         {
            prop_xml.set_attr_uint4(db_type, CsiDbSource::db_type_name);
            prop_xml.set_attr_str(db_data_source, CsiDbSource::db_data_source_name);
            prop_xml.set_attr_str(db_user_id, CsiDbSource::db_user_id_name);
            prop_xml.set_attr_str(db_password, CsiDbSource::db_password_name);
            prop_xml.set_attr_str(db_initial_catalog, CsiDbSource::db_initial_catalog_name);
            prop_xml.set_attr_uint4(poll_interval, CsiDbSource::poll_interval_name);
            prop_xml.set_attr_bool(db_use_windows_authentication, CsiDbSource::db_use_windows_authentication_name);
            prop_xml.set_attr_bool(db_save_user_id, CsiDbSource::db_save_user_id_name);
            prop_xml.set_attr_bool(should_poll_meta, CsiDbSource::should_poll_meta_name);
         } // get_properties


         void DbProperties::set_properties(Csi::Xml::Element &prop_xml)
         {
            uint4 type = prop_xml.get_attr_uint4(CsiDbSource::db_type_name);
            if(type < db_type_mysql || type > db_type_oracle)
               throw std::invalid_argument("invalid DB type");
            db_type = static_cast<db_type_code>(type);
            db_data_source = prop_xml.get_attr_str(CsiDbSource::db_data_source_name);
            db_user_id = prop_xml.get_attr_str(CsiDbSource::db_user_id_name);
            db_password = prop_xml.get_attr_str(CsiDbSource::db_password_name);
            db_initial_catalog = prop_xml.get_attr_str(CsiDbSource::db_initial_catalog_name);
            poll_interval = 300000;
            if(prop_xml.has_attribute(CsiDbSource::poll_interval_name))
            {
               poll_interval = prop_xml.get_attr_uint4(CsiDbSource::poll_interval_name);
               if(poll_interval < 1000)
                  poll_interval = 1000;
            }
            db_use_windows_authentication = false;
            if(prop_xml.has_attribute(CsiDbSource::db_use_windows_authentication_name))
               db_use_windows_authentication = prop_xml.get_attr_bool(CsiDbSource::db_use_windows_authentication_name);
            db_save_user_id = true;
            if(prop_xml.has_attribute(CsiDbSource::db_save_user_id_name))
               db_save_user_id = prop_xml.get_attr_bool(CsiDbSource::db_save_user_id_name);
            if(prop_xml.has_attribute(CsiDbSource::should_poll_meta_name))
               should_poll_meta = prop_xml.get_attr_bool(CsiDbSource::should_poll_meta_name);
            else
               should_poll_meta = false;
         } // set_properties


         Csi::LgrDate DbProperties::round_timestamp(Csi::LgrDate const &time) const
         {
            // we need to determine the smallest unit of time for the db type
            int8 nsec(time.get_nanoSec() % Csi::LgrDate::nsecPerSec);
            int8 seconds(time.get_nanoSec() - nsec);
            int8 smallest(1);
            switch(db_type)
            {
            case db_type_mysql:
               smallest = Csi::LgrDate::nsecPerUSec;
               break;
               
            case db_type_sql_server:
               smallest = 100;
               break;
               
            case db_type_sql_server_compact:
               smallest = 3 * Csi::LgrDate::nsecPerMSec;
               break;

            case db_type_postgresql:
               smallest = Csi::LgrDate::nsecPerUSec;
               break;

            case db_type_oracle:
               smallest = Csi::LgrDate::nsecPerUSec;
               break;
            }

            // we now need to determine whether the date needs to be rounded
            int8 remainder(nsec % smallest);
            if(remainder != 0)
            {
               nsec -= remainder;
               nsec += smallest;
            }
            return Csi::LgrDate(seconds + nsec);
         } // round_timestamp
      };
   };
};

