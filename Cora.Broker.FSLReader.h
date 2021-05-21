/* Cora.Broker.FSLReader.h

   Copyright (C) 2007, 2008 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Thursday Aug 23 2007
   Last Change: Thursday 21 August 2008
   Last Commit: $Date: 2017-10-18 09:11:43 -0600 (Wed, 18 Oct 2017) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Broker_FSLReader_h
#define Cora_Broker_FSLReader_h

#include "Csi.SharedPtr.h"
#include "CsiTypeDefs.h"
#include "Csi.Xml.Element.h"
#include "Cora.Broker.DataFileReader.h"
#include <map>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class FSLReader
      //////////////////////////////////////////////////////////// 
      class FSLReader
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         FSLReader();

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         ~FSLReader();

         //@group tables iterator access methods
         typedef Csi::SharedPtr<Cora::Broker::RecordDesc> record_description_handle;
         typedef std::map<uint4, record_description_handle> record_descs_type;
         typedef record_descs_type::iterator iterator;
         typedef record_descs_type::const_iterator const_iterator;
         typedef record_descs_type::reverse_iterator reverse_iterator;
         typedef record_descs_type::const_reverse_iterator const_reverse_iterator;
         iterator begin() { return record_descs.begin(); }
         const_iterator begin() const { return record_descs.begin(); }
         reverse_iterator rbegin() { return record_descs.rbegin(); }
         const_reverse_iterator rbegin() const { return record_descs.rbegin(); }
         iterator end() { return record_descs.end(); }
         const_iterator end() const { return record_descs.end(); }
         reverse_iterator rend() { return record_descs.rend(); }
         const_reverse_iterator rend() const { return record_descs.rend(); }
         iterator find(uint4 key) { return record_descs.find(key); }
         const_iterator find(uint4 key) const { return record_descs.find(key); }
         record_descs_type::size_type size() const { return record_descs.size(); }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // extract_labels
         ////////////////////////////////////////////////////////////
         void extract_labels(StrAsc const &fsl_file_name_);

         ////////////////////////////////////////////////////////////
         // get_labels_sig
         ////////////////////////////////////////////////////////////
         uint2 get_labels_sig();
         
      protected:
         ////////////////////////////////////////////////////////////
         // extract_from_dld
         //
         // Extracts all final storage FSLReader from the specified DLD stream
         //////////////////////////////////////////////////////////// 
         void extract_from_dld();

         ////////////////////////////////////////////////////////////
         // extract_from_fsl
         //
         // Extracts all final stroage FSLReader from the specified FSL stream
         //////////////////////////////////////////////////////////// 
         void extract_from_fsl();

         ///////////////////////////////////////////////////////////
         // extract_record_descs
         //
         // Extracts the record descriptions from the FSL stream
         ///////////////////////////////////////////////////////////
         void extract_record_descs(std::istream &fsl_in);
         
      private:
         ////////////////////////////////////////////////////////////
         // record_descs
         //
         // The list of record descriptions known for the array id key
         //////////////////////////////////////////////////////////// 
         record_descs_type record_descs;

         ////////////////////////////////////////////////////////////
         // fsl_file_name
         ////////////////////////////////////////////////////////////
         StrAsc fsl_file_name;
      };
   };
};

#endif
