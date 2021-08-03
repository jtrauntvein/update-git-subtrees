/* Csi.NmeaParser.h

   Copyright (C) 2021, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 11 January 2021
   Last Change: Thursday 21 January 2021
   Last Commit: $Date: 2021-01-21 17:08:13 -0600 (Thu, 21 Jan 2021) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_NmeaParser_h
#define Csi_NmeaParser_h
#include "Csi.Json.h"
#include "Csi.ByteQueue.h"
#include "Csi.InstanceValidator.h"
#include "Csi.CsvRec.h"


namespace Csi
{
   class NmeaParser;

   
   /**
    * Defines the interface for an application to receive NMEA sentences from the parser component.
    */
   class NmeaParserClient: public InstanceValidator
   {
   public:
      /**
       * Can be overloaded to allow the application to access a raw NMEA sentence that has been
       * verified by the parser.
       *
       * @param sender Specifies the component that is calling this method.
       *
       * @param sentence Specifies the sentence that has been verified.
       */
      virtual void on_sentence(NmeaParser *sender, StrAsc const &sentence)
      { }

      /**
       * Called when a error has happened while processing a sentence.
       *
       * @param sender Specifies the component calling this method.
       *
       * @param reason Specifies a string that describes the reason for the failure.
       */
      virtual void on_failure(NmeaParser *sender, StrAsc const &reason)
      { }
   };


   /**
    * Defines an object about a reported satellite in view.
    */
   class NmeaSatellite
   {
   private:
      /**
       * Specifies the satellite ID.
       */
      double id;

      /**
       * Specifies the satellite elevation (degrees from horizon)
       */
      double elevation;

      /**
       * Specifies the satellite azimuth (degrees from true north)
       */
      double azimuth;

      /**
       * Specifies the signal to noise ratio in dB
       */
      double snr;

   public:
      /**
       * Default Constructor
       */
      NmeaSatellite():
         id(std::numeric_limits<double>::quiet_NaN()),
         elevation(std::numeric_limits<double>::quiet_NaN()),
         azimuth(std::numeric_limits<double>::quiet_NaN()),
         snr(std::numeric_limits<double>::quiet_NaN())
      { }

      /**
       * Copy Constructor
       */
      NmeaSatellite(NmeaSatellite const &other):
         id(other.id),
         elevation(other.elevation),
         azimuth(other.azimuth),
         snr(other.snr)
      { }
            
      /**
       * Constructor
       *
       * @param sentence Specifies the split sentence from which this record will be
       * constructed.
       *
       * @param offset Specifies the offset within the split record to start.
       */
      NmeaSatellite(CsvRec const &sentence, size_t offset);

      /**
       * @return Returns the satellite numeric ID.
       */
      double get_id() const
      { return id; }

      /**
       * @return Returns the satellite elevation angle in degrees from the horizon.
       */
      double get_elevation() const
      { return elevation; }

      /**
       * @return Returns the satellite azimuth angle in degrees from true north.
       */
      double get_azimuth() const
      { return azimuth; }

      /**
       * @return Returns the signal to noise ratio for the satellite in dB
       */
      double get_snr() const
      { return snr; }
   };
   

   /**
    * Defines a component that can be used to parse a stream of NMEA sentences by an application.
    * In order to use this component, the application must provide an object that implements the
    * NmeaParserClient interface.  It should then create an instance of this component.  It can then
    * call the parser component's on_data() method as many times as is needed.  The parser will pick
    * out the sentences that have a valid checksum and send events to the application by calling the
    * client object's on_sentence() method.
    */
   class NmeaParser
   {
   protected:
      /**
       * Specifies the application client object.
       */
      NmeaParserClient *client;

      /**
       * Specifies the buffer for data that is yet to be parsed.
       */
      ByteQueue buffer;

      /**
       * Specifies the host time stamp when the sentence began.
       */
      Csi::LgrDate sentence_time;

      /**
       * Specifies the buffer used to handle the last sentence.  This will be emptied each time that
       * a new sentence is begun.
       */
      StrAsc sentence;

      /**
       * Specifies the position of the checksum in the sentence.
       */
      size_t checksum_pos;

      /**
       * Specifies the state of this parser.
       */
      enum state_type
      {
         state_between_sentences,
         state_in_sentence,
         state_in_checksum
      } state;

      /**
       * Specifies the sentence split between commas.
       */
      Csi::CsvRec split_sentence;

      /**
       * Specifies the talker ID returned from the first value in any sentence.
       */
      StrAsc talker_id;

      /**
       * Specifies the sentence type specified in the first value of any sentence.
       */
      StrAsc sentence_type;

      /**
       * Set to true if the position fix was updated in the last sentence.
       */
      bool position_updated;

      /**
       * Set to true if the satellites were updated in the last sentence.
       */
      bool satellites_updated;
      
      /**
       * Specifies the time stamp carried in the last sentence.
       */
      LgrDate stamp;
      
      /**
       * Specifies the last reported latitude.
       */
      double latitude;

      /**
       * Specifies the last reported longitude
       */
      double longitude;

      /**
       * Specifies the quality code for the GPS fix.
       */
   public:
      enum fix_quality_type
      {
         fix_none = 0,
         fix_gps = 1,
         fix_gps_diff = 2,
         fix_pps = 3,
         fix_real_time_kinematic = 4,
         fix_float_rtk = 5,
         fix_estimated = 6,
         fix_manual = 7,
         fix_simulation = 8
      };
   protected:
      fix_quality_type fix_quality;

      /**
       * Specifies the horizontal dilution of precision (in metres)
       */
      double horizontal_dilution;

      /**
       * Specifies the antenna altitude in metres.
       */
      double antenna_altitude;

      /**
       * Specifies descriptions of satellites that have been detected by the receiver.
       */
   public:
      typedef std::vector<NmeaSatellite> satellites_type;
   protected:
      satellites_type satellites;
      
   public:
      /**
       * Constructor
       *
       * @param client_ Specifies the application object that will receive event notifications from
       * the client.
       */
      NmeaParser(NmeaParserClient *client_);

      /**
       * Destructor
       */
      virtual ~NmeaParser();

      /**
       * Adds a new block of received data.
       *
       * @param data Specifies the pointer to the start of the block.
       *
       * @param data_len Specifies the number of bytes to add.
       */
      void on_data(void const *data, uint4 data_len);

      /**
       * @return Returns the time when the last sentence began.
       */
      Csi::LgrDate const &get_sentence_time() const
      { return sentence_time; }

      /**
       * @return Returns the last sentence parsed.
       */
      StrAsc const &get_sentence() const
      { return sentence; }

      /**
       * @return Returns true if the position fix was updated for the last sentence.
       */
      bool get_position_updated() const
      { return position_updated; }

      /**
       * @return Returns true if the satellites were updated for the last sentence.
       */
      bool get_satellites_updated() const
      { return satellites_updated; }

      /**
       * @return Returns the time stamp parsed from the last sentence.
       */
      LgrDate const &get_stamp() const
      { return stamp; }
      /**
       * @return Returns the latitude in degrees.
       */
      double get_latitude() const
      { return latitude; }

      /**
       * @return Returns the longitude in degrees.
       */
      double get_longitude() const
      { return longitude; }

      /**
       * @return Returns the position fix quality indicator.
       */
      fix_quality_type get_fix_quality() const
      { return fix_quality; }

      /**
       * @return Returns the antenna altitude in metres.
       */
      double get_altitude() const
      { return antenna_altitude; }

      /**
       * @return Returns the horizontal dilution  of precision in metres.
       */
      double get_horizontal_dilution() const
      { return horizontal_dilution; }

      /**
       * @return Returns an iterator that references the first satellite.
       */
      typedef satellites_type::iterator iterator;
      typedef satellites_type::const_iterator const_iterator;
      iterator begin()
      { return satellites.begin(); }
      const_iterator begin() const
      { return satellites.begin(); }

      /**
       * @return Returns an iterator beyond the end of the list of satellites.
       */
      iterator end()
      { return satellites.end(); }
      const_iterator end() const
      { return satellites.end(); }

      /**
       * @return Returns true if the list of satellites is empty.
       */
      bool empty() const
      { return satellites.empty(); }

      /**
       * @return Returns the number of satellites in view.
       */
      satellites_type::size_type size() const
      { return satellites.size(); }

      /**
       * @return Returns a reference to the satellite at the specified position.
       *
       * @param pos Specifies the position to look up.
       */
      NmeaSatellite const &at(size_t pos) const
      { return satellites.at(pos); }

   protected:
      /**
       * Processes the sentence that last came out of the parser.
       */
      virtual void process_sentence();

      /**
       * Processes a GGA (GPS Positioning Fix) sentence.
       */
      virtual void process_gga();

      /**
       * Processes a GLL (geographical position) sentence.
       */
      virtual void process_gll();

      /**
       * Processes a GNS (fix data) sentence.
       */
      virtual void process_gns();

      /**
       * Processed a GSV (satellites in view) sentence.
       */
      virtual void process_gsv();
      
   private:
      /**
       * Parses the next sentence from the input buffer.
       *
       * @return Returns true if there is the data in the input buffer that can possible parse the
       * next sentence.
       */
      bool parse_next_sentence();
   };
};


#endif
