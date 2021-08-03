/* Cora.Broker.DataFileReader.cpp

   Copyright (C) 2003, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 12 August 2003
   Last Change: Thursday 07 September 2017
   Last Commit: $Date: 2017-09-07 16:55:39 -0600 (Thu, 07 Sep 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.DataFileReader.h"
#include "Cora.Broker.ValueName.h"
#include "Cora.Broker.TobFileReader.h"
#include "Cora.Broker.Toa5FileReader.h"
#include "Cora.Broker.Toa6FileReader.h"
#include "Cora.Broker.Toa1FileReader.h"
#include "Cora.Broker.XmlFileReader.h"
#include "Cora.Broker.MixedCsvFileReader.h"
#include "Csi.OsException.h"
#include "Csi.Utils.h"
#include <iterator>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class DataFileReader definitions
      ////////////////////////////////////////////////////////////
      DataFileReader::DataFileReader(
         Csi::SharedPtr<ValueFactory> &value_factory_):
         value_factory(value_factory_)
      {
         if(value_factory == 0)
            value_factory.bind(new ValueFactory);
      } // constructor


      DataFileReader::~DataFileReader()
      { close(); }


      DataFileReader::record_handle DataFileReader::make_record(uint4 array_id)
      {
         record_descriptions_type::iterator di = record_descriptions.find(array_id);
         
         if(di == record_descriptions.end())
            throw DataFileException(DataFileException::failure_not_initialised);
         return record_handle(new Record(di->second, *value_factory));
      } // make_record


      void DataFileReader::process_field_name(char const *field_name)
      {
         Csi::SharedPtr<ValueDesc> value_description(new ValueDesc);
         ValueName value_name(field_name);

         if(record_descriptions.size() == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         record_description_handle &record_description = record_descriptions.begin()->second;
         value_description->name = value_name.get_column_name();
         std::copy(
            value_name.begin(),
            value_name.end(),
            std::back_inserter(value_description->array_address));
         value_description->data_type = CsiUnknown;
         value_description->modifying_cmd = 0;
         record_description->values.push_back(value_description);
      } // process field_name


      ////////////////////////////////////////////////////////////
      // function make_data_file_reader
      ////////////////////////////////////////////////////////////
      Csi::SharedPtr<DataFileReader> make_data_file_reader(
         StrAsc const &file_name,
         Csi::SharedPtr<ValueFactory> factory,
         StrAsc const &fsl_name)
      {
         Csi::SharedPtr<DataFileReader> rtn;
         Csi::data_file_types file_type = Csi::get_data_file_type(file_name.c_str());
         switch(file_type)
         {
         case Csi::data_file_type_toaci1:
            rtn.bind(new Toa1FileReader(factory));
            break;
                     
         case Csi::data_file_type_toa5:
            rtn.bind(new Toa5FileReader(factory));
            break;

         case Csi::data_file_type_toa6:
            rtn.bind(new Toa6FileReader(factory));
            break;
            
         case Csi::data_file_type_tob1:
         case Csi::data_file_type_tob2:
         case Csi::data_file_type_tob3:
            rtn.bind(new TobFileReader(factory));
            break;

         case Csi::data_file_type_error:
            throw DataFileException(DataFileException::failure_cannot_open);
            break;
            
         case Csi::data_file_type_csixml:
            rtn.bind(new XmlFileReader(factory));
            break;

         case Csi::data_file_type_mixed_csv:
            rtn.bind(new MixedCsvFileReader(factory));
            break;

         case Csi::data_file_type_unknown:
         default:
            throw DataFileException(DataFileException::failure_unsupported_file_type);
            break;
         }
         if(rtn != 0)
         {
            rtn->file_type = file_type;
            rtn->open(file_name, fsl_name);
         }
         return rtn;
      } // make_data_file_reader
   };
};

