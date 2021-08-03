/* Csi.Alert2.h

   Copyright (C) 2016, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 05 July 2016
   Last Change: Wednesday 05 May 2021
   Last Commit: $Date: 2021-05-05 12:13:03 -0600 (Wed, 05 May 2021) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alert2_h
#define Csi_Alert2_h
#include "Csi.LgrDate.h"
#include "Csi.LightSharedPtr.h"
#include "Csi.CsvRec.h"
#include "Csi.InstanceValidator.h"
#include "Csi.ByteQueue.h"
#include <deque>


namespace Csi
{
   namespace Alert2
   {
      /**
       * Lists the types of sensor reports for concentration or self-reporting data.
       */
      enum report_type_code
      {
         report_concentration = 0,
         report_general_sensor = 1,
         report_rain_gauge = 2,
         report_multi_sensor_english = 3,
         report_multi_sensor_metric = 4,
         report_measurement_suite = 5,
         report_time_series = 7
      };


      /**
       * Lists the types of sensor report values.
       */
      enum sensor_report_value_type
      {
         sensor_value_uint = 1,
         sensor_value_int = 2,
         sensor_value_float = 3,
         sensor_value_transmission_offset = 13,
         sensor_value_day_offset = 14,
         sensor_value_timestamp = 15
      };


      /**
       * Defines the standard sensor identifiers
       */
      enum standard_sensor_ids
      {
         standard_sensor_rain = 0,
         standard_sensor_air_temp = 1,
         standard_sensor_rh = 2,
         standard_sensor_bp = 3,
         standard_sensor_ws = 4,
         standard_sensor_wd = 5,
         standard_sensor_wp = 6,
         standard_sensor_stage = 7,
         standard_sensor_battery = 8
      };


      /**
       * Defines the type codes for IND messages.
       */
      enum ind_message_type
      {
         ind_message_airlink,
         ind_message_mant,
         ind_message_concentration,
         ind_message_status
      };


      /**
       * @return Returns the default name for a sensor report associated with the specified sensor
       * ID.
       *
       * @param sensor_id Specifies the sensor ID.
       */
      StrUni default_report_name(uint2 sensor_id);


      /**
       * @return Returns the default conversion expression associated with the specified sensor
       * ID.
       *
       * @param sensor_id Specifies the sensor ID.
       */
      StrAsc default_report_convert_expression(uint2 sensor_id);
      

      // @group: class forward declarations
      class MantPduBase;
      class SensorReportBase;
      class IndMessageBase;
      class IndStream;
      class IndMessageMant;
      // @endgroup:


      /**
       * Defines an object that represents a value in a sensor report.
       */
      class SensorReportValue
      {
      private:
         /**
          * Specifies the sensor report from which this value came.
          */
         SensorReportBase *report;

         /**
          * Specifies the report value type.
          */
         sensor_report_value_type value_type;

         /**
          * Specifies the storage for this value.
          */
         union
         {
            uint4 uint_value;
            int4 int_value;
            double float_value;
         } storage;

         /**
          * Specifies the sensor ID for this value.
          */
         uint2 sensor_id;

         /**
          * Specifies the time offset in nanoseconds for this sensor value relative to the sensor
          * report.
          */
         int8 value_time_offset;

         friend class FormatLen;
         
      public:
         /**
          * Default constructor to be used in conjunction with the FormatLen initialisor object.
          */
         SensorReportValue():
            report(0),
            sensor_id(0),
            value_time_offset(0)
         { }
         
         /**
          * Construct from a memory image.
          *
          * @param report_ Specifies the report from which this value was read.
          *
          * @param sensor_id_ Specifies the sensor identifier for this value.
          *
          * @param buff Specifies the start of the memory buffer for this value.
          *
          * @param buff_len Specifies the number of bytes available.
          */
         explicit SensorReportValue(
            SensorReportBase *report_,
            uint2 sensor_id_,
            void const *buff,
            size_t buff_len);

         /**
          * Construct with explicit value type.
          *
          * @param report_ Specifies the report from which this value was read.
          *
          * @param sensor_id_ Specifies the sensor identifier.
          *
          * @param value_type_ Specifies the value type.
          *
          * @param value Specifies the value to initialise.
          */
         explicit SensorReportValue(
            SensorReportBase *report_,
            uint2 sensor_id_,
            sensor_report_value_type value_type_,
            uint4 value):
            report(report_),
            sensor_id(sensor_id_),
            value_type(value_type_),
            value_time_offset(0)
         { storage.uint_value = value; }

         explicit SensorReportValue(
            SensorReportBase *report_,
            uint2 sensor_id_,
            sensor_report_value_type value_type_,
            int4 value):
            report(report_),
            sensor_id(sensor_id_),
            value_type(value_type_),
            value_time_offset(0)
         { storage.int_value = value; }

         /**
          * Copy constructor
          */
         explicit SensorReportValue(SensorReportValue const &other):
            report(other.report),
            sensor_id(other.sensor_id),
            storage(other.storage),
            value_type(other.value_type),
            value_time_offset(other.value_time_offset)
         { }

         /**
          * Copy operator
          */
         SensorReportValue &operator =(SensorReportValue const &other)
         {
            report = other.report;
            sensor_id = other.sensor_id;
            storage = other.storage;
            value_type = other.value_type;
            value_time_offset = other.value_time_offset;
            return *this;
         }

         /**
          * Destructor
          */
         ~SensorReportValue()
         { report = 0; }

         /**
          * @return Returns the sensor ID.
          */
         uint2 get_sensor_id() const
         { return sensor_id; }

         /**
          * @return Returns the value type.
          */
         sensor_report_value_type get_value_type() const
         { return value_type; }

         /**
          * @return Returns the value as a floating point number.
          */
         double get_value_float()
         {
            double rtn;
            switch(value_type)
            {
            case sensor_value_uint:
            case sensor_value_transmission_offset:
            case sensor_value_day_offset:
            case sensor_value_timestamp:
               rtn = storage.uint_value;
               break;
               
            case sensor_value_int:
               rtn = storage.int_value;
               break;
               
            case sensor_value_float:
               rtn = storage.float_value;
               break;
            }
            return rtn;
         }

         /**
          * @return Returns the value as an integer.
          */
         int4 get_value_int()
         {
            int rtn;
            switch(value_type)
            {
            case sensor_value_uint:
            case sensor_value_transmission_offset:
            case sensor_value_day_offset:
            case sensor_value_timestamp:
               rtn = static_cast<int4>(storage.uint_value);
               break;

            case sensor_value_int:
               rtn = storage.int_value;
               break;

            case sensor_value_float:
               rtn = static_cast<int4>(storage.float_value);
               break;
            } 
            return rtn;
         } // get_value_int

         /**
          * @return Returns the message received time.
          */
         LgrDate const &get_received_time() const;

         /**
          * @return Returns the sensor report station ID.
          */
         uint2 get_station_id() const;

         /**
          * @return Returns the time stamp associated with this sensor report.  This value will be
          * derived from the message received time and the sensor report time stamp.
          */
         LgrDate get_time_stamp() const;

         /**
          * @return Returns the sensor report PDU time offset in units of nanoseconds.
          */
         int8 get_time_offset() const;

         /**
          * @param value Specifies the time offset in nanoseconds of this value relative to the
          * sensor report time stamp.
          */
         void set_value_time_offset(int8 value)
         { value_time_offset = value; }
         
         /**
          * @return Returns true if the PDU that contains this value specifies its own time stamp.
          */
         bool get_has_time_stamp() const;

         /**
          * @return Returns the cyclic PDU id for the PDU that contains this value.
          */
         byte get_apdu_id() const;

         /**
          * @return Returns true of the PDU that contains this value has the test flag set.
          */
         bool get_from_test() const;

         /**
          * @return Returns the type code for the sensor report that contains this value.
          */
         report_type_code get_report_type() const;
      };
      

      /**
       * Defines an object that decodes the format/length structure from an ALERT2 sensor report and
       * can initialise a sensor report value.
       */
      class FormatLen
      {
      public:
         /**
          * Specifies the value type code.
          */
         sensor_report_value_type const value_type;

         /**
          * Specifies the expected length of the value.
          */
         size_t const value_len;

         /**
          * @param fl Specifies the encoded format/len object.
          */
         FormatLen(byte fl):
            value_len(fl & 0b00001111),
            value_type(static_cast<sensor_report_value_type>((fl & 0b11110000) >> 4))
         { }

         /**
          * Initialises the specified sensor report value based upon the type and length.
          *
          * @param value_report Specifies the value report that will be initialised.
          * @param report Specifies the sensor report that owns this value.
          * @param sensor_id Specifies the sensor report identifier.
          * @param buff_ Specifies the pointer at the beginning of the value storage.
          * @param buff_len Specifies the number of bytes available.
          */
         void init_value_report(
            SensorReportValue &value,
            SensorReportBase *report,
            uint2 sensor_id,
            void const *buff_,
            size_t buff_len);
      };
      

      /**
       * Defines the base class for an object that will represent a sensor report.  This object will
       * act as a container for report values.
       */
      class SensorReportBase
      {
      public:
         /**
          * @param pdu_ Specifies the PDU that contains this report.
          *
          * @param report_type_ Specifies the report type.
          */
         SensorReportBase(MantPduBase *pdu_, report_type_code report_type_):
            pdu(pdu_),
            report_type(report_type_)
         { }

         /**
          * Destructor
          */
         virtual ~SensorReportBase()
         {
            pdu = 0;
            values.clear();
         }

         /**
          * @return Returns the report type code
          */
         report_type_code get_report_type() const
         { return report_type; }

         
         // @group: declarations to act as a container of report values.

         /**
          * @return Returns a reference to the first report value.
          */
         typedef SensorReportValue value_type;
         typedef std::deque<value_type> values_type;
         typedef values_type::iterator iterator;
         typedef values_type::const_iterator const_iterator;
         iterator begin()
         { return values.begin(); }
         const_iterator begin() const
         { return values.begin(); }

         /**
          * @return Returns the iterator at the end of the values sequence.
          */
         iterator end()
         { return values.end(); }
         const_iterator end() const
         { return values.end(); }

         /**
          * @return Returns the number of values.
          */
         typedef values_type::size_type size_type;
         size_type size() const
         { return values.size(); }

         /**
          * @return Returns true if there are no values.
          */
         bool empty() const
         { return values.empty(); }

         /**
          * @return Returns the value at the specified offset.
          *
          * @param index Specifies the index of the value to return.
          */
         value_type &at(size_t index)
         { return values.at(index); }
         value_type const &at(size_t index) const
         { return values.at(index); }

         // @endgroup:

      protected:
         /**
          * Specifies the reference to the PDU from which this report came.
          */
         MantPduBase *pdu;

         /**
          * Specifies the type code for this sensor report.
          */
         report_type_code report_type;

         /**
          * Specifies the list of values that were read for this report.
          */
         values_type values;

         friend class SensorReportValue;
      };


      /**
       * Defines an object that represents a general sensor report.
       */
      class SensorReportGeneral: public SensorReportBase
      {
      public:
         /**
          * @param pdu_ Specifies the PDU from which this report was read.
          *
          * @param buff Specifies the start of the buffer that contains this sensor report.
          *
          * @param buff_len Specifies the number of bytes available in the buffer.
          */
         SensorReportGeneral(MantPduBase *pdu_, void const *buff, size_t buff_len);

         /**
          * Destructor
          */
         virtual ~SensorReportGeneral()
         { }
      };


      /**
       * Defines an object that represents a rain gauge sensor report.
       */
      class SensorReportRainGauge: public SensorReportBase
      {
      public:
         /**
          * @param pdu_ Specifies the PDU from which this report was read.
          *
          * @param buff Specifies the start of the buffer that contains this sensor report.
          *
          * @param buff_len Specifies the number of bytes available in the buffer.
          */
         SensorReportRainGauge(MantPduBase *pdu_, void const *buff, size_t buff_len);

         /**
          * Destructor
          */
         virtual ~SensorReportRainGauge()
         { }
      };


      /**
       * Defines an object the represents a multi-sensor report with english units.
       */
      class SensorReportMultiEnglish: public SensorReportBase
      {
      public:
         /**
          * @param pdu_ Specifies the PDU from which this report was read.
          *
          * @param buff Specifies the start of the buffer that contains this report.
          *
          * @param buff_len Specifies the number of bytes in the report.
          */
         SensorReportMultiEnglish(MantPduBase *pdu_, void const *buff, size_t buff_len);
      };


      /**
       * Defines an object that represents a multi-sensor report with metric units.
       */
      class SensorReportMultiMetric: public SensorReportBase
      {
      public:
         /**
          * @param pdu_ Specifies the PDU from which this report was read.
          *
          * @param buff Specifies the start of the buffer that contains this report.
          *
          * @param buff_len Specifies the number of bytes in the report.
          */
         SensorReportMultiMetric(MantPduBase *pdu_, void const *buff, size_t buff_len);
      };


      /**
       * Defines an object that parses and represents a time-series sensor report.  This type of
       * report is essentially an array of values with a reported start time and a reported interval
       * between values.
       */
      class SensorReportTimeSeries: public SensorReportBase
      {
      public:
         /**
          * @param pdu_ Specifies the PDU from which this report was read.
          * @param buff Specifies the start of the buffer that contains this report.
          * @param buff_len Specifies the length of the buffer in bytes.
          */
         SensorReportTimeSeries(MantPduBase *pdu, void const *buff, size_t buff_len);
      };


      /**
       * Defines a sensor report that reports concentration values.
       */
      class ConcentrationReport: public SensorReportBase
      {
      public:
         /**
          * @param pdu_ Specifies the PDU that owns this report.
          *
          * @param buff Specifies the data containing the values.
          *
          * @param buff_len Specifies the number of bytes that can be read.
          */
         ConcentrationReport(MantPduBase *pdu_, void const *buff, size_t buff_len);
      };
      

      /**
       * Defines the base class that represents the payload of a MANT message.
       */
      class MantPduBase
      {
      public:
         /**
          * @param message_ Specifies the message that contains this PDU.
          */
         MantPduBase(IndMessageMant *message_):
            message(message_),
            version(0),
            has_time_stamp(false),
            from_test(false),
            apdu_id(7),
            time_stamp(0)
         { }

         /**
          * Destructor
          */
         virtual ~MantPduBase()
         { }

         /**
          * @return Returns the report version number.
          */
         byte get_version() const
         { return version; }

         /**
          * @return Returns true if the report has its own time stamp.
          */
         bool get_has_time_stamp() const
         { return has_time_stamp; }

         /**
          * @return Returns true of this report was from test conditions.
          */
         bool get_from_test() const
         { return from_test; }

         /**
          * @return Returns the cyclic pdu identifier for this report.
          */
         byte get_apdu_id() const
         { return apdu_id; }

         /**
          * @return Returns the time offset of this report as seconds since midnight or noon
          * whichever passed most recently.
          */
         uint2 get_time_stamp() const
         { return time_stamp; }

         /**
          * @return Returns the time stamp for this PDU.  This will be based upon the message
          * received time and will incorporate the time offset for this PDU if specified.
          */
         LgrDate get_pdu_time() const;

         // @group: definitions to act as a container for sensor reports.

         /**
          * @return Returns the first iterator for the group of sensor reports.
          */
         typedef Csi::LightSharedPtr<SensorReportBase> value_type;
         typedef std::deque<value_type> reports_type;
         typedef reports_type::iterator iterator;
         typedef reports_type::const_iterator const_iterator;
         iterator begin()
         { return reports.begin(); }

         const_iterator begin() const
         { return reports.begin(); }

         /**
          * @return Returns the last iterator for the set of sensor reports.
          */
         iterator end()
         { return reports.end(); }
         const_iterator end() const
         { return reports.end(); }

         /**
          * @return Returns the number of sensor reports for this PDU.
          */
         typedef reports_type::size_type size_type;
         size_type size() const
         { return reports.size(); }

         /**
          * @return Returns true if there are no reports.
          */
         bool empty() const
         { return reports.empty(); }
         
         // @endgroup:

      protected:
         /**
          * Specifies the MANT message that contains this PDU.
          */
         IndMessageMant *message;

         /**
          * Specifies the PDU version.
          */
         byte version;

         /**
          * Set to true if this PDU contains a time stamp.
          */
         bool has_time_stamp;

         /**
          * Set to true of this PDU was flagged as generated from a test.
          */
         bool from_test;

         /**
          * Specifies the cyclic PDU identifier.
          */
         byte apdu_id;

         /**
          * Specifies the time offset for this PDU as seconds since noon or midnight.
          */
         uint2 time_stamp;

         /**
          * Specifies the sensor reports contained within this payload.
          */
         reports_type reports;

         friend class SensorReportValue;
      };

      
      /**
       * Defines an object that represents a self reporting value set.  It will act as a container
       * for sensor reports which will, in turn, act as containers for sensor values.
       */
      class SelfReportPdu: public MantPduBase
      {
      public:
         /**
          * Constructor
          *
          * @param message_ Specifies the message that contains this PDU.
          *
          * @param report_buff Specifies the start of the buffer that describes this report.
          *
          * @param report_buff_len Specifies the number of bytes available for this report.
          */
         SelfReportPdu(
            IndMessageMant *message_,
            void const *report_buff,
            size_t report_buff_len);
         
         /**
          * Destructor
          */
         virtual ~SelfReportPdu()
         { }

         friend class SensorReportValue;
      };


      /**
       * Defines an object that represents a concentration PDU.
       */
      class ConcentrationPdu: public MantPduBase
      {
      public:
         /**
          * @param message_ Specifies the message that contains this PDU.
          *
          * @param buff Specifies the data for this PDU.
          *
          * @param buff_len Specifies the length of the PDU buffer.
          */
         ConcentrationPdu(
            IndMessageMant *message_, void const *buff, size_t buff_len);

         /**
          * Destructor
          */
         virtual ~ConcentrationPdu()
         { }
      };


      /**
       * Defines a base class for messages that can be parsed from an ALERT2 IND stream.
       */
      class IndMessageBase
      {
      public:
         /**
          * @param message_type_ Specifies the type code for this message.
          */
         IndMessageBase(ind_message_type message_type_):
            message_type(message_type_)
         { }

         /**
          * Destructor
          */
         virtual ~IndMessageBase()
         { }

         /**
          * @return Returns the type code for this message.
          */
         ind_message_type get_message_type() const
         { return message_type; }
         
      protected:
         /**
          * Specifies the type code for this message.
          */
         ind_message_type const message_type;
      };


      /**
       * Defines an object that represents an AirLink IND message.
       */
      class IndMessageAirlink: public IndMessageBase
      {
      public:
         /**
          * @param content Specifies the content of this message specified as comma-separated
          * strings.
          */
         IndMessageAirlink(CsvRec const &contents):
            IndMessageBase(ind_message_airlink)
         { }
      };


      /**
       * Defines an object that represents a MANT message.
       */
      class IndMessageMant: public IndMessageBase
      {
      public:
         /**
          * @param content Specifies the content of this message formatted as comma-separated
          * strings.
          */
         IndMessageMant(CsvRec const &content);

         /**
          * @return Returns the time stamp when this message was received.
          */
         LgrDate const &get_received_time() const
         { return received_time; }

         /**
          * @return Returns the protocol ID.
          */
         enum protocol_type
         {
            protocol_broadcast = 0,
            protocol_end_to_end = 1
         };
         protocol_type get_protocol() const
         { return protocol; }

         /**
          * @return Returns true if the add path service was requested.
          */
         bool get_add_path_service() const
         { return add_path_service; }

         /**
          * @return Returns true if the destination address was included.
          */
         bool get_has_destination_address() const
         { return has_destination_address; }

         /**
          * @return Returns the service port.
          */
         enum service_port_type
         {
            port_self_reporting = 0,
            port_concentration = 1
         };
         service_port_type get_service_port() const
         { return service_port; }

         /**
          * @return Returns true if the message contains a MANT pdu identifier.
          */
         bool get_has_mant_pdu_id() const
         { return has_mant_pdu_id; }

         /**
          * @return Returns the reported hop limit.
          */
         uint4 get_hop_limit() const
         { return hop_limit; }

         /**
          * @return Returns the source address.
          */
         uint2 get_source_address() const
         { return source_address; }

         /**
          * @return Returns the MANT PDU identifier.  This value will only have meaning if
          * get_has_man_pdu_id() returns true.
          */
         uint4 get_mant_pdu_id() const
         { return mant_pdu_id; }

         /**
          * @return Returns the list of added source addresses.
          */
         typedef std::deque<uint2> source_addresses_type;
         source_addresses_type const &get_source_addresses() const
         { return source_addresses; }

         /**
          * @return Returns the PDU for this message.  
          */
         typedef LightSharedPtr<MantPduBase> pdu_handle;
         pdu_handle &get_pdu()
         { return pdu; }

         /**
          * @return Returns the list of sensor identifiers in this message.
          */
         typedef std::deque<uint2> sensor_identifiers_type;
         sensor_identifiers_type get_sensor_identifiers() const;

         /**
          * @return Returns the total number of sensor report values carried by this message.
          */
         size_t get_values_count() const;

      private:
         /**
          * Specifies the time when this message was received.
          */
         LgrDate received_time;

         /**
          * Specifies the protocol type.
          */
         protocol_type protocol;

         /**
          * Set to true if the add path service was requested.
          */
         bool add_path_service;

         /**
          * Set to true if the destination address was specified.
          */
         bool has_destination_address;

         /**
          * Specifies the destination address.
          */
         uint2 destination_address;

         /**
          * Specifies the service port.
          */
         service_port_type service_port;

         /**
          * Set to true if the MANT PDU id was specified.
          */
         bool has_mant_pdu_id;

         /**
          * Specifies the hop limit.
          */
         uint4 hop_limit;

         /**
          * Specifies the source address.
          */
         uint2 source_address;

         /**
          * Specifies the MANTR PDU id.
          */
         uint4 mant_pdu_id;

         /**
          * Specifies the source repeater addresses.
          */
         source_addresses_type source_addresses;

         /**
          * Specifies the self reporting data PDU that might have been in this message.
          */
         pdu_handle pdu;
      };


      /**
       * Specifies a message that deals with concentration PDUs.
       */
      class IndMessageConcentration: public IndMessageBase
      {
      public:
         /**
          * @param content Specifies the content for the message.
          */
         IndMessageConcentration(CsvRec const &content):
            IndMessageBase(ind_message_concentration)
         { }
      };


      /**
       * Specifies a message that deals with status PDUs.
       */
      class IndMessageStatus: public IndMessageBase
      {
      public:
         /**
          * @param content Specifies the content for the message.
          */
         IndMessageStatus(CsvRec const &content):
            IndMessageBase(ind_message_status),
            parameters(content.begin() + 1, content.end())
         { }

         // @group: declarations that allow this class to act as a parameters container.

         /**
          * @return Returns the iterator to the first parameter.
          */
         typedef StrAsc value_type;
         typedef std::deque<StrAsc> parameters_type;
         typedef parameters_type::iterator iterator;
         typedef parameters_type::const_iterator const_iterator;
         iterator begin()
         { return parameters.begin(); }
         const_iterator begin() const
         { return parameters.begin(); }

         /**
          * @return Returns the parameter position beyond the end.
          */
         iterator end()
         { return parameters.end(); }
         const_iterator end() const
         { return parameters.end(); }

         /**
          * @return Returns the number of parameters.
          */
         typedef parameters_type::size_type size_type;
         size_type size() const
         { return parameters.size(); }

         /**
          * @return Returns true if there are no parameters.
          */
         bool empty() const
         { return parameters.empty(); }
         
         // @endgroup
         
      private:
         /**
          * Specifies the message parameters.
          */
         parameters_type parameters;
      };


      /**
       * Defines the applications interface to receive messages from the IND stream.
       */
      class IndStreamClient: public InstanceValidator
      {
      public:
         /**
          * Called when the end of a message has been recognised.
          *
          * @param sender Specifies the message sender.
          *
          * @param content Specifies the content of the message including the start of the new
          * line.
          */
         virtual void on_message_content(IndStream *sender, StrBin const &content)
         { }
         
         /**
          * Called when an IND message has been parsed from the stream.
          *
          * @param sender Specifies the stream that called this method.
          * 
          * @param message Specifies the message that has been parsed.
          *
          * @return Returns the number of report values that were processed.
          */
         typedef LightSharedPtr<IndMessageBase> message_handle;
         virtual uint4 on_message(IndStream *sender, message_handle &message) = 0;

         /**
          * Called when an error has been detected while processing an IND message.
          *
          * @param sender Specifies the stream that called this method.
          *
          * @param message Specifies the error message.
          *
          * @param content Specifies the content that triggered the error.
          */
         virtual void on_error(
            IndStream *sender,
            StrAsc const &error,
            StrBin const &content)
         { }
      };


      /**
       * Defines an object that will parse the stream of content from an ALERT2 IND decoder.  In
       * order to use this component, the application must provide an object that is derived from
       * class IndStreamClient.  It passes received data to the on_data() method and will receive
       * notifications of parsed messages through the client object's on_message() method. 
       */
      class IndStream
      {
      public:
         /**
          * @param client_ Specifies the application object that will receive notification of new
          * messages.
          */
         IndStream(IndStreamClient *client_);

         /**
          * Destructor
          */
         virtual ~IndStream()
         { }

         /**
          * Handles incoming data from the IND.
          */
         virtual void on_data(void const *buff, size_t buff_len);

         /**
          * Clears out any remaining data in the receive buffer.
          */
         virtual void clear();
         
      private:
         /**
          * Specifies the application object that will receive notifications of new messages.
          */
         IndStreamClient *client;

         /**
          * Specifies the buffer for incoming data.
          */
         ByteQueue rx_buff;

         /**
          * Specifies the buffer that stores the content of the current message.
          */
         StrBin message_buff;

         /**
          * Used to process received data.
          */
         CsvRec parser;
      };
   };
};


#endif
