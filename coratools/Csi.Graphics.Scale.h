/* Csi.Graphics.Scale.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 14 January 2015
   Last Change: Tuesday 02 June 2015
   Last Commit: $Date: 2020-07-14 15:42:11 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_Scale_h
#define Csi_Graphics_Scale_h

#include "Csi.Graphics.Font.h"
#include "Csi.Graphics.NestedRect.h"
#include "Csi.Graphics.Driver.h"
#include "Csi.Expression.Token.h"
#include "Csi.LightSharedPtr.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines an object that represents a label on a scale.
       */
      class ScaleLabel
      {
      public:
         /**
          * Specifies the text label.
          */
         StrUni label;

         /**
          * Specifies the value for this label.
          */
         typedef LightPolySharedPtr<Expression::Token, Expression::Operand> value_handle;
         value_handle value;

         /**
          * Specifies the position on the scale.
          */
         double position;

         /**
          * Specifies the rectangle for the label text.
          */
         typedef NestedRect::child_handle rect_handle;
         rect_handle rect;

         /**
          * Constructor
          */
         ScaleLabel(
            StrUni const &label_,
            value_handle &value_,
            double position_,
            rect_handle rect_):
            label(label_),
            value(value_),
            position(position_),
            rect(rect_)
         { }
      };
      
      
      /**
       * Defines an object that can be used to represent a scale that has
       * direction, limits, and labels.
       */
      class Scale
      {
      public:
         /**
          * Represents the size of the area on which the scale can be drawn.
          */
         double size;

         /**
          * Represents the pixels to real units ratio calculated for this
          * scale.
          */
         double scale;

         /**
          * Set to true if this scale represents time domain labels.
          */
         bool time_domain;

         /**
          * Specifies the format string that will be used to format time domain
          * labels.
          */
         StrAsc time_format;

         /**
          * Set to true if this scale is logarithmic.
          */
         bool logarithmic;

         /**
          * Set to true if the labels on this scale are inverted.
          */
         bool inverted;

         /**
          * Specifies the base for logarithms when the scale is configured as
          * logarithmic.
          */
         double log_base;

         /**
          * Set to true if floating point labels must be formatted using fixed
          * decimal notation.
          */
         bool fixed_decimals;

         /**
          * Specifies the number of digits to format for fixed decimal
          *notation.
          */
         int decimal_places;

         /**
          * Specifies the font that will be used for formatting labels.
          */
         LightSharedPtr<Font> font;

         /**
          * Set to true when this scale is set up to be vertical.
          */
         bool vertical;

         /**
          * Specifies the angle, in degrees, at which labels should be rotated.
          */
         double rotation;

         /**
          * Set to true if this scale must have only one label.
          */
         bool one_tick_only;

         /**
          * Specifies the maximum value for this scale.
          */
         typedef LightPolySharedPtr<Expression::Token, Expression::Operand> value_handle;
         value_handle max_value;

         /**
          * Specifies the minimum value for this scale.
          */
         value_handle min_value;

         /**
          * Specifies the interval between labels for this scale.
          */
         value_handle label_interval;
         bool auto_interval;

         /**
          * Specifies the last set of labels generated for this scale.
          */
         typedef LightSharedPtr<ScaleLabel> label_handle;
         typedef std::deque<label_handle> labels_type;
         labels_type labels;

         /**
          * Specifies the stream that will be used for formatting labels.
          */
         Csi::OStrAscStream scratch;

         /**
          * Holds the reference to the driver used for this scale.
          */
         typedef LightSharedPtr<Driver> driver_handle;
         driver_handle driver;

      public:
         /**
          * @param driver_ Specifies the driver used to measure text.
          */
         Scale(driver_handle driver_);

         /**
          * Destructor
          */
         virtual ~Scale();

         /**
          * @return Returns the formatted label associated with the specified
          * value.
          *
          * @param value Specifies the value to format.
          *
          * @param for_mark Set to true if the label is being formatted for display on a mark.
          */
         StrAsc const &format_label(
            value_handle value, bool for_mark = false);

         /**
          * Sets the properties for this scale to their default values.
          */
         virtual void set_default_properties();

         /**
          * Writes the properties for this scale to the specified XML object.
          *
          * @param elem Specifies the destination XML object.
          */
         virtual void write(Xml::Element &elem);

         /**
          * Reads the properties for this scale from the specified XML object.
          *
          * @param elem Specifies the source XML object.
          */
         virtual void read(Xml::Element &elem);

         /**
          * Calculates the scale based upon the specified max and min value.
          *
          * @param max_value_ Specifies the new maximum value for this scale.
          *
          * @param min_value_ Specifies the new minimum value for this scale.
          */
         double calculate_scale(value_handle &max_value_, value_handle &min_value_);

         /**
          * Generates the set of labels for this scale based upon the specified
          * maximum and minimum values.
          *
          * @param max_value_  Specifies the new maximum value.
          *
          * @param min_value_ Specifies the new minimum value.
          *
          * @return Returns the list of labels that were generated.
          */
         labels_type &generate_labels(value_handle max_value_, value_handle min_value_);

         /**
          * Generates the set of positions for minor ticks.
          *
          * @param minor_ticks Specifies the container of minor ticks to fill.
          *
          * @param min_width Specifies the minimum interval between minor
          * ticks.
          *
          * @param count Specifies the number of minor ticks to generate for
          * each major interval.  If omitted or specified as less than zero, as
          * many ticks as possible will be fit into the available space.
          */
         typedef std::deque<double> minor_ticks_type;
         void generate_minor_ticks(
            minor_ticks_type &minor_ticks,
            double min_width,
            int4 count = -1);

         void generate_minor_ticks_uint8(
            minor_ticks_type &minor_ticks,
            double min_width,
            double tick_interval,
            double scale_max,
            double scale_min,
            int4 count);

         void generate_minor_ticks_double(
            minor_ticks_type &minor_ticks,
            double min_width,
            double tick_interval,
            double scale_min, 
            double scale_max,
            int4 count);

         /**
          * @return Calculates the position on this scale for the specified
          * value.
          *
          * @param value Specifies the value to consider.
          */
         double value_to_pos(value_handle &value);

         /**
         * @return Returns the number of pixels required to draw the value.
         *
         * @param value Specifies the value to consider.
         */
         double offset_to_size(double);

         /**
          * @return Calculates the value associated with the specified scale
          * position.
          *
          * @param pos Specifies the position to consider.
          */
         value_handle pos_to_value(double pos);

         /**
          * @return Calculates the value for the specified pixel offset near
          * the specified value on the scale.
          *
          * @param value Specifies the value near which the offset should be
          * considered.
          *
          * @param offset Specifies the screen offset.
          */
         double offset_weight(value_handle &value, double offset);

         // @group: methods and declarations that allow this class to act as a container of labels.

         /**
          * @return Returns an iterator to the first label.
          */
         typedef labels_type::iterator iterator;
         typedef labels_type::const_iterator const_iterator;
         iterator begin()
         { return labels.begin(); }
         const_iterator begin() const
         { return labels.begin(); }

         /**
          * @return Returns the end labels iterator.
          */
         iterator end()
         { return labels.end(); }
         const_iterator end() const
         { return labels.end(); }

         /**
          * @return Returns true if there are no labels.
          */
         bool empty() const
         { return labels.empty(); }

         /**
          * @return Returns a reference to the first label.
          */
         typedef label_handle value_type;
         value_type &front()
         { return labels.front(); }
         value_type const &front() const
         { return labels.front(); }

         /**
          * @return Returns a reference to the last label.
          */
         value_type &back()
         { return labels.back(); }
         value_type const &back() const
         { return labels.back(); }
         

         // @endgroup:

      private:
         /**
          * @return Calculates the value for the next tick based upon the
          * first, number of ticks previosuly evaluated, and the interval
          * between ticks.
          *
          * @param tick_prev Specifies the value for the previous tick.
          *
          * @param tick_interval Specifies the interval between ticks.
          */
         double next_tick(
            double tick_prev,
            double tick_interval);

         /**
          * Generates the optimum step size assuming that we are scaling numbers.
          *
          * @param range Specifies the range over which we will calculate.
          *
          * @param space Specifies the amount of space available.
          *
          * @param label_req Specifies the maximum label requirement.
          *
          * @return Returns the optimum step size.
          */
         value_handle scalar_optimum_step_size(
            double range, double space, double label_req);

         /**
          * Generates the optimum step size assuming that the scale represents time.
          *
          * @param range Specifies the range to consider.
          *
          * @param space Specifies the amount of space available.
          *
          * @param label_req Specifies the maximum size label.
          *
          * @return Returns the optimum step size.
          */
         value_handle time_optimum_step_size(
            double range, double space, double label_req);
      };
   };
};


#endif
