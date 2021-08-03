/* Cora.Broker.DataFileReader.h

   Copyright (C) 2003, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 12 August 2003
   Last Change: Monday 11 July 2016
   Last Commit: $Date: 2016-07-11 09:29:07 -0600 (Mon, 11 Jul 2016) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_DataFileReader_h
#define Cora_Broker_DataFileReader_h

#include "Cora.Broker.Record.h"
#include "Cora.Broker.RecordDesc.h"
#include "Csi.Utils.h"
#include <map>
#include <deque>


namespace Cora
{
   namespace Broker
   {
      /**
       * Specifies an exception that will be thrown on an unsuccessful attempt
      to open or read data from the data file.
      */
      class DataFileException: public std::exception
      {
      public:
         /**
          * Exception constructor.
          *
          * @param failure_reason_  Specifies the reason for the reported
          * failure.
          */
         enum failure_reason_type
         {
            failure_unknown,
            failure_cannot_open,
            failure_unsupported_file_type,
            failure_corrupt_file,
            failure_not_initialised,
            failure_fsl,
            failure_max_unknown_error
         };
         DataFileException(
            failure_reason_type failure_reason_):
            failure_reason(failure_reason_)
         { }

         /**
          * @return Returns the failure that was used to construct this
          * exception.
          */
         failure_reason_type get_failure_reason() const
         { return failure_reason; }

         /**
          * @return Returns a nul terminated string that describes the failure
          */
         virtual char const *what() const throw ()
         {
            static char const *reasons[] = {
               "unknown failure",
               "cannot open file",
               "unsupported file type", 
               "corrupt file",
               "not initialised",
               "failure to open or read the FSL file"
            };
            int reason = failure_reason;
            if(reason >= failure_max_unknown_error)
               reason = 0;
            return reasons[reason];
         }
         
      private:
         /**
          * Specifies the failure that occurred.
          */
         failure_reason_type failure_reason; 
      };


      /**
       * Attempts to open the specified file name and to read enough content to
       * determine its for,ay.  If the format is supported, an appropriate data
       * file reader object will be allocated and initialised with the file
       * contents.
       *
       * @return Returns a new data file reader object or null if the format is
       * unsupported or the data file could not be read or opened.
       *
       * @param file_name Specifies the path to the input data file.
       *
       * @param factory  Specifies a factory object for creating record values.
       *
       * @para fsl_name Specifies the name and path for a final storage labels
       * file.  This parameter is ignored for table based file formats.
       */
      class DataFileReader;
      Csi::SharedPtr<DataFileReader> make_data_file_reader(
         StrAsc const &file_name,
         Csi::SharedPtr<ValueFactory> factory = 0,
         StrAsc const &fsl_name = 0);


      /**
       * Defines an index entry for a record in the data file.  This index will
       * include the record time stamp, record number, and the data offset
       * where the record begins.
       */
      class DataFileIndexEntry
      {
      public:
         /**
          * Specifies the time stamp for the record at this index.
          */
         Csi::LgrDate time_stamp;

         /**
          * Specifies the record number of the record at this index.
          */
         uint4 record_no;

         /**
          * Specifies the data offset where the record begins.
          */
         int8 data_offset;

         /**
          * Specifies the array ID for this record.
          */
         uint4 array_id;
         
      public:
         /**
          * Default constructor.
          */
         DataFileIndexEntry():
            data_offset(-1),
            record_no(0xffffffff),
            array_id(0)
         { }

         /**
          * Copy constructor
          *
          * @param other  Specifies the index entry to copy.
          */
         DataFileIndexEntry(DataFileIndexEntry const &other):
            data_offset(other.data_offset),
            record_no(other.record_no),
            time_stamp(other.time_stamp),
            array_id(other.array_id)
         { }

         /**
          * Copy operator.
          */
         DataFileIndexEntry &operator =(DataFileIndexEntry const &other)
         {
            data_offset = other.data_offset;
            record_no = other.record_no;
            time_stamp = other.time_stamp;
            array_id = other.array_id;
            return *this;
         }

         /**
          * Construct from members.
          *
          * @param data_offset_ specifies the data offset where the record was found.
          *
          * @param time_stamp_ Specifies the time stamp for the record.
          *
          * @param record_no_ Specifies the record number.
          */
         DataFileIndexEntry(
            int8 data_offset_,
            Csi::LgrDate const &time_stamp_,
            uint4 record_no_,
            uint4 array_id_ = 0):
            data_offset(data_offset_),
            time_stamp(time_stamp_),
            record_no(record_no_),
            array_id(array_id_)
         { }

         /**
          * Compares this index entry against another.
          *
          * @param other Specifies the other index entry.
          *
          * @return Will return a positive value if this entry is greater then the other, a negative
          * value if this entry is less than the other, and zero if this entry is equal to the
          * other.
          */
         int compare(DataFileIndexEntry const &other) const
         {
            int rtn(0);
            if(array_id == other.array_id)
            {
               if(time_stamp == other.time_stamp)
               {
                  if(record_no == other.record_no)
                  {
                     if(data_offset != other.data_offset)
                        rtn = (data_offset < other.data_offset ? -1 : 1);
                  }
                  else
                     rtn = (record_no < other.record_no ? -1 : 1);
               }
               else
                  rtn = (time_stamp < other.time_stamp ? -1 : 1);
            }
            else
               rtn = (array_id < other.array_id ? -1 : 1);
            return rtn;
         }

         /**
          * overload the less than operator.
          */
         bool operator <(DataFileIndexEntry const &other) const
         { return compare(other) < 0; }

         /**
          * Overload the <= operator.
          */
         bool operator <=(DataFileIndexEntry const &other) const
         { return compare(other) <= 0; }

         /**
          * Overload the == operator
          */
         bool operator ==(DataFileIndexEntry const &other) const
         { return compare(other) == 0; }

         /**
          * Overload the >= operator
          */
         bool operator >=(DataFileIndexEntry const &other) const
         { return compare(other) >= 0; }

         /**
          * Overload the > operator
          */
         bool operator >(DataFileIndexEntry const &other) const
         { return compare(other) > 0; }
      };


      /**
       * Defines a base class for a set of components that can be used to read
       * data nad meta-data files produced by the dataloggers and by LoggerNet.
       */
      class DataFileReader
      {
      public:
         typedef Csi::SharedPtr<RecordDesc> record_description_handle;
         typedef std::map<uint4, record_description_handle> record_descriptions_type;
         typedef record_descriptions_type::value_type value_type;

      protected:
         /**
          * Specifies the set of descriptions for the types of records
          * contained in this file (a mixed array format can contain data for
          * multiple tables.
          */
         record_descriptions_type record_descriptions;

         /**
          * Specifies the object used to allocate record value objects.
          */
         Csi::SharedPtr<ValueFactory> value_factory;

         /**
          * Specifies a code for thedata file format.
          */
         Csi::data_file_types file_type;
         
      public:
         /**
          * Constructor
          *
          * @param value_factory_ Specifies the value factory object that will
          * be used by this reader.
          */
         typedef Csi::SharedPtr<ValueFactory> value_factory_handle;
         DataFileReader(value_factory_handle &value_factory_);

         /**
          * Destructor
          */
         virtual ~DataFileReader();

         /**
          * Must be overloaded to open the specified data file and optional
          * labels file and to parse the header to format the set of record
          * descriptions.  When this method returns without any exception, the
          * reader must be in a state to read the oldest record in the file.
          *
          * @param file_name Specifies the name and path of the data file.
          *
          * @param labels_file_name  Optionally specifies the name and path for
          * the final storage labels file.  This value must be specified for
          * mixed array format data files but will be ignored for table based
          * formats.
          *
          * @throws DataFileException Will throw an exception if the file (or
          * associated labels file) cannot be opened or parsed.
          */
         virtual void open(
            StrAsc const &file_name,
            StrAsc const &labels_file_name = "") = 0;

         /**
          * Can be overloaded to resynchronise any cached information about the
          * file.  Some reader types, notably the TOB2/3 file readers, will
          * cache the positions of the oldest and newest file records.  If the
          * file is afterwards changed, this cached information can become
          * out of date.  This method must be overloaded for those instances.
          */
         virtual void resynch()
         { }

         /**
          * Releases any resources associated with the data file.
          */
         virtual void close()
         { record_descriptions.clear(); }

         /**
          * Must be overloaded in order to hibernate the data file.  This
          * means that the derived class must release any handle to associated
          * file(s) but remain in a state where record descriptions are still
          * valid.  A reader placed in this state should be able to be released
          * from this state through a call to wake_up().
          */
         virtual void hibernate() = 0;

         /**
          * Must be overloaded by derived classes to reanimate the data file
          * reader after it has been placed in a hibernated state.  In essence,
          * the data file handle(s) should be re-opened but the headers should
          * not be fully parsed.  An implementation may choose to validate the
          * signature of data file headers and, possibly to validate the last
          * data that was read.
          *
          * @return Returns true if the wake up process succeeded.
          *
          * @param all_data_overwritten Specifies a boolean reference that will
          * be set to true of the reader detects that the data file has been
          * overwritten. 
          */
         virtual bool wake_up(bool &all_data_overwritten) = 0;

         /**
          * @return Returns the record description associated with the
          * specified array ID.
          *
          * @throws DataFileException will throw a data file exception with the
          * exc_not_initialised failure if the reader is not in an open state.
          */
         record_description_handle &get_record_description(uint4 array_id = 0)
         {
            if(record_descriptions.size() == 0)
               throw DataFileException(DataFileException::failure_not_initialised);
            iterator find_it = find(array_id);
            if(find_it == end())
               throw DataFileException(DataFileException::failure_not_initialised);
            else
               return find_it->second;
         }

         /**
          * @return Generates a record object based upon the content of the
          * record description associated with the speified array ID.
          *
          * @param array_id  Specifies the array identifier for the
          * description.
          *
          * @throws DataFileException Will throw a data file exception with the
          * exc_not_initialised failure code if the data file is not open.
          */
         typedef Csi::SharedPtr<Record> record_handle;
         virtual record_handle make_record(uint4 array_id = 0);

         /**
          * Reads the next record from the data file current offset.  The
          * application is responsible for providing a suitable record object
          * created using the record description associated with the specified
          * array identifier (this can be done through a call to
          * make_record()).
          *
          * @param destination Specifies the record object that will be filled
          * in.
          *
          * @param file_mark_after_record Optionally specifies a pointer to a
          * boolean value that, if not null, will be set to true of a file mark
          * was found after reading the record.
          *
          * @param remove_mark_after_record Optionally specifies a pointer to a
          * boolean value that, if not null, will be set to true if a remove
          * mark was found after reading the record.
          *
          * @param array_id  Optionally specifies the array identifier to read.
          *
          * @return Returns a code that indicates the outcome of the read
          * operation.
          */
         enum read_outcome_type
         {
            read_outcome_success = 1,
            read_outcome_end_of_file = 2,
            read_outcome_not_initialised = 3,
            read_outcome_invalid_record = 4,
         };
         virtual read_outcome_type read_next_record(
            record_handle &destination,
            bool *file_mark_after_record = 0,
            bool *remove_mark_after_record = 0,
            uint4 array_id = 0) = 0;

         /**
          * @return Can be overloaded to return true of the last record time
          * stamp was specified using the 24:00 notation.
          */
         virtual bool last_time_was_2400() const
         { return false; }

         /**
          * @return Can be overloaded to return false if the file does not
          * contain a time stamp field.
          */
         virtual bool has_time_stamp() const
         { return true; }

         /**
          * @return Can be overloaded to return false if the file does not
          * contain a record number field.
          */
         virtual bool has_record_no() const
         { return true; }

         /**
          * @return Returns a code that represents the data file's format.
          */
         typedef Csi::data_file_types file_type_code;
         file_type_code get_type() const
         { return file_type; }

         /**
          * @return Returns the number of bytes occupied by the data file
          * header.
          */
         virtual int8 get_header_len() = 0;

         /**
          * @return Returns the signature of the data file header.  TYpically,
          * this will be the header portion of the file but can alos be the
          * signature of the file storage labels for mixed array files.
          */
         virtual uint2 get_header_sig() = 0;

         /**
          * @return Returns the number of bytes in the data file that can store
          * actual data (usually the file length less the header length).
          */
         virtual int8 get_data_len() = 0;

         /**
          * @return Returns the current offset of the current file reading
          * position in terms of distance from the oldest data in the file
          * (note that the oldest position for TOB2 files can be in the middle
          * of the file.
          */
         virtual int8 get_data_offset() = 0;

         /**
          * Positions the file handle at the specified offset and optionally
          * searches backward for the beginning of the closest record.
          *
          * @param offset Specifies the data offset seek position.  Note that
          * this is not the same as the file offset.
          *
          * @param search_for_prev_record Set to true if the reader must search
          * backward from the specified position to find the beginning of the
          * next record to read.
          */
         virtual void seek_data(int8 offset, bool search_for_prev_record = true) = 0;

         /**
          * Seeks to the position of the newest record associated with the
          * specified array identifier.
          *
          * @param array_id Specifies the array for which the reader will
          * search.
          */
         virtual void seek_to_newest(uint4 array_id = 0)
         {
            int8 data_len = get_data_len();
            if(data_len > 0)
               seek_data(data_len - 1);
            else
               seek_data(0);
         }

         /**
          * Must be overloaded to generate an index of the records in the data file starting at the
          * current position and running to the last record.
          *
          * @param index Specifies the list of record indices.
          *
          * @param should_abort Controls whether the indexing loop should be aborted.
          *
          * @param Used for some formats where the record number must be generated.  Specifies the
          * next record number that should be used.  Ignored if set to null.
          */
         typedef std::deque<DataFileIndexEntry> index_type;
         virtual void generate_index(index_type &index, bool &should_abort, uint4 *next_record_no) = 0;
         

         // @group: methods that make this class act as a container of record
         // descriptions.

         
         /**
          * @return Returns the iterator to the first record description.
          */
         typedef record_descriptions_type::iterator iterator;
         typedef record_descriptions_type::const_iterator const_iterator;
         iterator begin()
         { return record_descriptions.begin(); }
         const_iterator begin() const
         { return record_descriptions.begin(); }

         /**
          * @return Returns the reverse iterator to the last record
          * description.
          */
         typedef record_descriptions_type::reverse_iterator reverse_iterator;
         typedef record_descriptions_type::const_reverse_iterator const_reverse_iterator;
         reverse_iterator rbegin()
         { return record_descriptions.rbegin(); }
         const_reverse_iterator rbegin() const
         { return record_descriptions.rbegin(); }

         /**
          * @return Returns the forward end iterator for the record
          * descriptions.
          */
         iterator end()
         { return record_descriptions.end(); }
         const_iterator end() const
         { return record_descriptions.end(); }

         /**
          * @return Returns the reverse end iterator for the record
          * descriptions.
          */
         reverse_iterator rend()
         { return record_descriptions.rend(); }
         const_reverse_iterator rend() const
         { return record_descriptions.rend(); }

         /**
          * @return Returns the iterator the specified array ID or end() if no
          * such array ID exists.
          *
          * @param key  Specifies the array ID to find.
          */
         iterator find(uint4 key)
         { return record_descriptions.find(key); }
         const_iterator find(uint4 key) const
         { return record_descriptions.find(key); }
         
         /**
          * @return Returns the number of record descriptions.
          */
         typedef record_descriptions_type::size_type size_type;
         size_type size() const
         { return record_descriptions.size(); }

         // @endgroup

      protected:
         /**
          * Implements the logic to parse the specified field name and extract
          * the array index informastion from it.  This is provided as a
          * service because all data file formats use the same syntax.
          *
          * @param field_name  Specifies the field name as read from the file.
          */
         virtual void process_field_name(char const *field_name);

         /**
          * Throws a data file exception with the specified reason after first
          * closing the data file.
          */
         void throw_init_failure(DataFileException::failure_reason_type reason)
         {
            close();
            throw DataFileException(reason);
         }

         friend Csi::SharedPtr<DataFileReader> make_data_file_reader(
            StrAsc const &file_name,
            Csi::SharedPtr<ValueFactory> factory,
            StrAsc const &fsl_name);
      };
   };
};


#endif
