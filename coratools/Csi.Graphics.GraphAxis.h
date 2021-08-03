/* Csi.Graphics.GraphAxis.h

   Copyright (C) 2015, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 22 January 2015
   Last Change: Friday 20 January 2017
   Last Commit: $Date: 2020-07-14 15:42:11 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_GraphAxis_h
#define Csi_Graphics_GraphAxis_h

#include "Csi.Graphics.GraphTrace.h"
#include "Csi.Graphics.Driver.h"
#include "Csi.Graphics.Scale.h"


namespace Csi
{
   namespace Graphics
   {
      class Graph;
      class GraphAxis;

      /**
       * Defines a highlighted region associated with the graph axis.
       */
      class GraphAxisHighlight
      {
      public:
         /**
          * Specifies the value where the highlighted region begins.
          */
         typedef Scale::value_handle value_handle;
         value_handle start;

         /**
          * Specifies the value where the highlighted region ends.
          */
         value_handle stop;

         /**
          * Specifies the brush used to fill the highlight region.
          */
         brush_handle brush;

         /**
          * Pointer to an application object that identifies this highlight.
          */
         void const *owner;

         /**
          * Default Constructor
          */
         GraphAxisHighlight():
            owner(0)
         { }

         /**
          * Copy constructor.
          */
         GraphAxisHighlight(GraphAxisHighlight const &other):
            start(other.start),
            stop(other.stop),
            brush(other.brush),
            owner(other.owner)
         { }

         /**
          * Construct with all members.
          */
         GraphAxisHighlight(
            value_handle start_,
            value_handle stop_,
            brush_handle brush_,
            void const *owner_):
            start(start_),
            stop(stop_),
            brush(brush_),
            owner(owner_)
         { }

         /**
          * Copy operator.
          */
         GraphAxisHighlight &operator =(GraphAxisHighlight const &other)
         {
            start = other.start;
            stop = other.stop;
            brush = other.brush;
            owner = other.owner;
            return *this;
         }
      };


      /**
       * Defines an object that stores the state of an axis.  The values
       * recorded by this object reflect those that can change when the user
       * zooms or pans the graph.
       */
      class GraphAxisState
      {
      public:
         /**
          * Specifies the time format field for the axis.
          */
         StrAsc time_format;

         /**
          * Set to true if the maximum value is calculated automatically.
          */
         bool auto_max;

         /**
          * Set to true if the minimum value is calculated automatically.
          */
         bool auto_min;

         /**
          * Specifies the minimum value for the axis.
          */
         typedef Scale::value_handle value_handle;
         value_handle min_value;

         /**
          * Specifies the maximum value for the axis.
          */
         value_handle max_value;

         /**
          * Set to true if the axis will automatically generate labels.
          */
         bool auto_label;

         /**
          * Set to true if axis labels are generated automatically forthe time
          * domain.
          */
         bool auto_time;

         /**
          * Specifies the max offset.
          */
         double max_offset;

         /**
          * Specifies the min offset.
          */
         double min_offset;

         /**
          * Specifies whether fixed decimals should be used to format axis values.
          */
         bool fixed_decimals;
      };
      
      
      /**
       * Defines a component that represents an axis on the graph component.
       */
      class GraphAxis
      {
      public:
         /**
          * Constructs this axis using the specified graph and type.
          *
          * @param graph_ Specifies the graph that owns this axis.
          *
          * @param axis_type_ Specifies the position of this axis.
          *
          * @param domain_axis_ Set to true if this axis is marked as the domain axis.
          */
         enum axis_type_code
         {
            axis_bottom,
            axis_top,
            axis_left,
            axis_right
         };
         GraphAxis(Graph *graph_, axis_type_code axis_type_, bool domain_axis_);

         /**
          * Destructor
          */
         virtual ~GraphAxis();

         // @group: properties

         /**
          * @return Returns the time format specification.
          */
         StrAsc const &get_time_format() const
         { return time_format; }

         /**
          * Sets the format used for formatting time stamps on the scale.
          *
          * @param format Specifies the format string as accepted by
          * Csi::LgrDate::format().  If this is empty and this is a time based
          * axis, the axis will choose the most appropriate format based upon
          * the range of data on the axis.
          */
         GraphAxis &set_time_format(StrAsc const &format)
         {
            time_format = format;
            return *this;
         }

         /**
          * @return Returns the caption shown for this axis.
          */
         StrUni const &get_caption() const
         { return caption; }

         /**
          * @param caption_ Specifies the caption shown for this axis.
          */
         GraphAxis &set_caption(StrUni const &caption_);

         /**
          * @return Returns the font used for the caption.
          */
         typedef LightSharedPtr<Font> font_handle;
         font_handle &get_caption_font()
         { return caption_font; }

         /**
          * Sets the font used for the caption.
          *
          * @param font Specifies the font caption.
          */
         GraphAxis &set_caption_font(font_handle font)
         {
            caption_font = font;
            return *this;
         }
         GraphAxis &set_caption_font(FontInfo const &desc)
         { return set_caption_font(driver->make_font(desc)); }

         /**
          * @return Returns the colour used for the caption.
          */
         Colour const &get_caption_colour() const
         { return caption_colour; }

         /**
          * Sets the colour used for the caption.
          *
          * @param colour Specifies the caption colour.
          */
         GraphAxis &set_caption_colour(Colour const &colour)
         {
            caption_colour = colour;
            return *this;
         }

         /**
          * @return Returns the caption angle.
          */
         double get_caption_angle() const
         { return caption_angle; }

         /**
          * Sets the angle for the caption.
          *
          * @param angle Specifies the caption angle in degrees.  A value of
          * zero will mean that the caption will be drawn parallel to the x
          * axis.
          */
         GraphAxis &set_caption_angle(double angle)
         {
            caption_angle = angle;
            return *this;
         }

         /**
          * @return Returns true if the caption should be drawn.
          */
         bool get_caption_visible() const
         { return caption_visible && caption_font != 0; }

         /**
          * @param value Set to true if the caption should be drawn.
          */
         GraphAxis &set_caption_visible(bool value)
         {
            caption_visible = value;
            if(caption_font == 0)
               caption_font = driver->make_font(FontInfo());
            return *this;
         }

         /**
          * @return Returns true if the minimum value is automatically
          * calculated.
          */
         bool get_auto_min() const
         { return auto_min; }

         /**
          * @return Returns true if the maximum value is automatically
          * calculated.
          */
         bool get_auto_max() const
         { return auto_max; }

         /**
          * Sets whether the maximum and minimum bounds for this axis
          * should be calculated.
          *
          * @param auto_min_ Set to true if the axis should automatically set
          * the minimum value on the scale based upon the associated traces.
          *
          * @param auto_max Set to true if the axis should automatically set
          * the maximum value on the scale based upon the data in associated
          * traces. 
          */
         GraphAxis &set_auto_bounds(bool auto_min_, bool auto_max_);

         /**
          * @return Returns the last minimum value that was evaluated.
          */
         typedef Scale::value_handle value_handle;
         value_handle const &get_min_value() const
         { return min_value; }

         /**
          * @return Returns the minimum value calculated for the scale.
          */
         value_handle const &get_scale_min() const
         { return scale->min_value; }

         /**
          * @return Returns the last maximum value that was evaluated.
          */
         value_handle const &get_max_value() const
         { return max_value; }

         /**
          * @return Returns the minimum value calculated for the scale.
          */
         value_handle const &get_scale_max() const
         { return scale->max_value; }

         /**
          * Sets the minimum value for this axis and clears the auto_min flag.
          *
          * @param value Specifies the new value for the axis minumum.  If this
          * value is null, the auto_min flag will be set.
          */
         GraphAxis &set_min_value(value_handle value)
         {
            min_value = value;
            auto_min = (value == 0);
            return *this;
         }

         /**
          * Sets the minimum value for this axis and sets the auto_min flag.  if the value is not
          * finite, the auto_min flag will be set to true.
          *
          * @param value Specifies the new minimum value for this axis.
          */
         GraphAxis &set_min_value(double value);

         /**
          * Sets the minimum value as a time stamp.
          *
          * @param value Specifies the value as a time stamp.
          */
         GraphAxis &set_min_value(LgrDate const &value)
         {
            min_value.bind(new Expression::Operand);
            min_value->set_val(value.get_nanoSec() / LgrDate::nsecPerMSec, value);
            auto_min = false;
            return *this;
         }

         /**
          * Sets the maximum value for this axis and clears the auto_max flag.
          *
          * @param value Specifies the new maximum value.  If null, the auto_max flag will be set.
          */
         GraphAxis &set_max_value(value_handle value)
         {
            max_value = value;
            auto_max = (value == 0);
            return *this;
         }

         /**
          * Sets the maximum value for this axis and sets the auto_max flag according to whether the
          * specified value is finite.
          *
          * @param value Specifies the new max value.
          */
         GraphAxis &set_max_value(double value);

         /**
          * Sets the maximum value as a time stamp.
          *
          * @param value Specifies maximum value.
          */
         GraphAxis &set_max_value(LgrDate const &value)
         {
            max_value.bind(new Expression::Operand);
            max_value->set_val(value.get_nanoSec() / LgrDate::nsecPerMSec, value);
            auto_max = false;
            return *this;
         }

         /**
          * @return Returns the offset in pixels that will be applied to the
          * maximum scale value.
          */
         double get_min_offset() const
         { return min_offset; }

         /**
          * Sets the minimum scale offset.
          *
          * @param offset Specifies the offset in pixels that will be applied
          * to the minimum scale value.
          */
         GraphAxis &set_min_offset(double offset);

         /**
          * @return Returns the offset in pixels that will be applied to the
          * maximum scale value.
          */
         double get_max_offset() const
         { return max_offset; }

         /**
          * Sets the maximum scale offset.
          *
          * @param offset Specifies the offset in pixels that will be applied
          * to the maximum scale value.
          */
         GraphAxis &set_max_offset(double offset);

         /**
          * @return Returns the pen used for drawing the major grid.
          */
         typedef LightSharedPtr<Pen> pen_handle;
         pen_handle &get_major_grid_pen()
         { return major_grid_pen; }

         /**
          * Sets the pen used for drawing the major grid.
          *
          * @param pen Specifies the pen used to draw the major grid lines.
          *
          * @param info Specifies the information needed to construct the pen.
          */
         GraphAxis &set_major_grid_pen(pen_handle pen)
         {
            major_grid_pen = pen;
            return *this;
         }
         GraphAxis &set_major_grid_pen(PenInfo const &info)
         { return set_major_grid_pen(driver->make_pen(info)); }

         /**
          * @return Returns the pen used for drawing the minor grid.
          */
         pen_handle &get_minor_grid_pen()
         { return minor_grid_pen; }

         /**
          * Sets the pen used for drawing the minor grid.
          *
          * @param pen Specifies the pen used to draw the minor grid.
          *
          * @param desc Specifies a description for the pen.
          */
         GraphAxis &set_minor_grid_pen(pen_handle pen)
         {
            minor_grid_pen = pen;
            return *this;
         }
         GraphAxis &set_minor_grid_pen(PenInfo const &desc)
         { return set_minor_grid_pen(driver->make_pen(desc)); }

         /**
          * @return Returns the pen handle used for drawing major ticks.
          */
         pen_handle &get_major_ticks_pen()
         { return major_ticks_pen; }

         /**
          * Sets the pen used for drawing major ticks.
          *
          * @param pen Specifies the major ticks pen.
          *
          * @param desc Specifies the description for the major ticks pen.
          */
         GraphAxis &set_major_ticks_pen(pen_handle pen)
         {
            major_ticks_pen = pen;
            return *this;
         }
         GraphAxis &set_major_ticks_pen(PenInfo const &desc)
         { return set_major_ticks_pen(driver->make_pen(desc)); }

         /**
          * @return Returns the pen used for drawing minor ticks.
          */
         pen_handle &get_minor_ticks_pen()
         { return minor_ticks_pen; }

         /**
          * Sets the pen used for drawing minor ticks.
          *
          * @param pen Specifies the pen used for drawing minor ticks.
          *
          * @param desc Specifies a description for the pen.
          */
         GraphAxis &set_minor_ticks_pen(pen_handle pen)
         {
            minor_ticks_pen = pen;
            return *this;
         }
         GraphAxis &set_minor_ticks_pen(PenInfo const &desc)
         { return set_minor_ticks_pen(driver->make_pen(desc)); }

         /**
         * Sets and Receives the length of the major/minor tick marks
         */

         GraphAxis& set_major_tick_length(int4 length_)
         { major_tick_length = length_; return *this; }

         GraphAxis& set_minor_tick_length(int4 length_)
         { minor_tick_length = length_; return *this; }

         int4 get_major_tick_length()
         { return major_tick_length;}

         int4 get_minor_tick_length()
         { return minor_tick_length;}

         /**
          * @return Returns the number of minor ticks that should be formatted
          * for each label.  This parameter will be ignored if auto_label is
          * set to true.
          */
         int4 get_minor_tick_count() const
         { return minor_tick_count; }

         /**
          * @param value Specifies the number of minor ticks that should be
          * drawn for each label interval.  If less than or equal to zero, no
          * minor ticks will be drawn.  This member will be ignored if
          * auto_scale is set to true.
          */
         GraphAxis &set_minor_tick_count(int4 value);

         /**
          * @return Returns the pen used to draw the line for this axis.
          */
         pen_handle &get_axis_pen()
         { return axis_pen; }

         /**
          * Sets the pen used to draw the line for this axis.
          */
         GraphAxis &set_axis_pen(pen_handle pen)
         {
            axis_pen = pen;
            return *this;
         }
         GraphAxis &set_axis_pen(PenInfo const &desc)
         { return set_axis_pen(driver->make_pen(desc)); }

         /**
          * @return Returns true if labels are visible for this axis.
          */
         bool get_labels_visible() const
         { return labels_visible; }

         /**
          * Sets whether labels are visible for this axis.
          *
          * @param visible Set to true if labels should be drawn for this axis.
          */
         GraphAxis &set_labels_visible(bool visible)
         {
            labels_visible = visible;
            return *this;
         }

         /**
          * @return Returns the font used for labels on this axis.
          */
         font_handle const &get_labels_font() const
         { return scale->font; }

         /**
          * Sets the font used for labels on this axis.
          *
          * @param font Specifies the font that should be used for labels on this axis.
          *
          * @param desc Specifies the font description.
          */
         GraphAxis &set_labels_font(font_handle font)
         {
            scale->font = font;
            return *this;
         }
         GraphAxis &set_labels_font(FontInfo const &desc)
         { return set_labels_font(driver->make_font(desc)); }

         /**
          * @return Returns the labels colour.
          */
         Colour const &get_labels_colour() const
         { return labels_colour; }

         /**
          * @param colour Specifies the labels colour.
          */
         GraphAxis &set_labels_colour(Colour const &colour)
         {
            labels_colour = colour;
            return *this;
         }

         /**
          * @return Returns the minimum size (width or height) of the labels rectangle.
          */
         double get_labels_size() const
         { return labels_size; }

         /**
          * @return Returns the rotation angle for labels.
          */
         double get_labels_rotation() const
         { return scale->rotation; }

         /**
          * @param value Specifies the angle (in degrees) at which the labels for this axis will be
          * rotated.  A value of zero (the default) means that the labels will be horizontal.
          */
         GraphAxis &set_labels_rotation(double value)
         {
            scale->rotation = value;
            return *this;
         }
         
         /**
          * Sets the minimum size for the labels rectangle.
          *
          * @param size Specifies the minimum size for the labels rectangle.
          */
         GraphAxis &set_labels_size(double size);

         /**
          * @return Returns true if this axis uses a logarithmic scale.
          */
         bool get_logarithmic() const
         { return scale->logarithmic; }

         /**
          * Sets whether this axis and associated scale is logarithmic.
          *
          * @param value Set to true if this axis is to be logarithmic.
          */
         GraphAxis &set_logarithmic(bool value)
         {
            scale->logarithmic = value;
            return *this;
         }

         /**
          * @return Returns the logarithm base.
          */
         double get_log_base() const
         { return scale->log_base; }

         /**
          * @param value Specifies the new value for the logarithm bvase.
          */
         GraphAxis &set_log_base(double value);
         
         /**
          * @return Returns true if this axis is inverted meaning that the
          * maximum value is shown before the minimum value.
          */
         bool get_inverted() const
         { return scale->inverted; }

         /**
          * Sets whether this axis is inverted.  An axis is inverted when the
          * maximum value is listed on the top (vertical axes) or on the left
          * (horizontal axes).
          *
          * @param inverted_ Set to true if this axis is to be inverted.
          */
         GraphAxis &set_inverted(bool inverted_);
         
         /**
          * @return Returns true if floating point axis labels are to be
          * formatted with fixed decimal places.
          */
         bool get_fixed_decimals() const
         { return scale->fixed_decimals; }

         /**
          * Sets whether floating point labels are to be formatted with fixed
          * decimal places.
          *
          * @param value Set to true if floating point axis labels are to be
          * formatted with fixed decimal places.
          */
         GraphAxis &set_fixed_decimals(bool value);
         
         /**
          * @return Returns the number of decimals places that should be used
          * when formatting floating point labels and fixed_decimals is set to
          * true.
          */
         int get_decimal_places() const
         { return scale->decimal_places; }

         /**
          * Sets the number of decimal places that should be used for
          * formatting floating point values when fixed_decimals is set to
          * true.
          *
          * @param value Specifies the number of decimal places.
          */
         GraphAxis &set_decimal_places(int value);

         /**
          * @return Returns true if the scale is vertical.
          */
         bool get_vertical() const
         { return scale->vertical; }
         
         /**
          * @return Returns the scale object used for this axis.
          */
         typedef LightSharedPtr<Scale> scale_handle;
         scale_handle &get_scale()
         { return scale; }

         /**
          * @return Returns true if the interval between labels should be
          * calculated automatically.
          */
         bool get_auto_label() const
         { return auto_label; }

         /**
          * @param value Set to true if the interval between labels should be
          * calculated automatically.  If set to false, the application must
          * set the interval by calling set_label_interval().
          */
         GraphAxis &set_auto_label(bool value);

         /**
         * @param value Set to true if the interval between labels should be
         * based on a set number of tick marks.  If set to false, the application must
         * set the interval by calling set_label_interval() OR by setting the auto_label
         * to true.
         */
         bool get_fixed_num_labels() {return use_fixed_num_steps;}
         GraphAxis &set_fixed_num_labels(bool value);

         /**
         * @param value Set to the number of tick marks that should appear in the axis
         * if fixed_num_labels is true
         */
         int get_num_fixed_steps() {return num_fixed_steps;}
         GraphAxis &set_num_fixed_steps(int value);
         
         /**
          * @return Returns the interval between labels.
          */
         value_handle const &get_label_interval() const
         { return scale->label_interval; }

         /**
          * @value Specifies the interval between labels.
          */
         GraphAxis &set_label_interval(value_handle value)
         {
            scale->label_interval = value;
            return *this;
         }
         GraphAxis &set_label_interval(double value)
         { return set_label_interval(new Expression::Operand(value, LgrDate::local())); }

         /**
          * @return Returns true if this axis is configured for time domain.
          */
         bool get_time_domain() const
         { return scale->time_domain; }

         /**
          * @param value Set to true if this axis represents the time domain.
          */
         GraphAxis &set_time_domain(bool value)
         {
            scale->time_domain = value;
            return *this;
         }

         /**
          * @return Returns true if time based max and min values should be
          * calculated automatically.
          */
         bool get_auto_time() const
         { return auto_time; }

         /**
          * @param value Set to true if time based max and min values must be
          * calculated automatically.
          */
         GraphAxis &set_auto_time(bool value);
         
         /**
          * @return Returns the application specified label interval.
          */
         double get_increment() const
         { return increment; }

         /**
          * @return Returns the increment value adjusted for the increment units.
          */
         double get_increment_with_units() const;

         /**
          * @param value Specifies the application assigned label interval.
          */
         GraphAxis &set_increment(double value);

         /**
          * @return Returns the units index for the time domain axis' label interval.
          */
         enum increment_units_type
         {
            increment_seconds = 0,
            increment_minutes = 1,
            increment_hours = 2,
            increment_days = 3,
            increment_weeks = 4
         };
         increment_units_type get_increment_units() const
         { return increment_units; }

         /**
          * @param value Specifies the units for the increment value on a time domain axis.
          */
         GraphAxis &set_increment_units(increment_units_type value);
         
         /**
          * @param value Specifies the label interval for the time domain axis.
          *
          * @param value_units Specifies the units for the value.
          */
         GraphAxis &set_increment_with_units(double value, int value_units);

         /**
          * @return Returns the type code for this axis.
          */
         axis_type_code get_axis_type() const
         { return axis_type; }

         /**
          * @param value Specifies the new value for the axis type.
          */
         GraphAxis &set_axis_type(axis_type_code value);

         /**
          * @return Returns the title for the axis type.
          */
         StrUni get_axis_type_title() const;

         /**
          * @return Returns true if this axis is marked as the domain axis.
          */
         bool get_domain_axis() const
         { return domain_axis; }

         /**
          * Sets all properties on the graph and scale to their default values.
          */
         virtual void set_default_properties();

         /**
          * Writes the properties for this axis to the specified XML structure.
          *
          * @param elem Specifies the destination XML structure.
          */
         virtual void write(Xml::Element &elem);
         
         /**
          * Reads the properties for this axis from the specified XML structure.
          *
          * @param elem Specifies the XML structure that should describe these properties.
          */
         virtual void read(Xml::Element &elem);
         
         // @endgroup:

         // @group: declarations and methods to act as a container of traces.

         /**
          * @return Returns the first iterator in the series of traces
          * associated with this axis.
          */
         typedef LightSharedPtr<GraphTrace> trace_handle;
         typedef std::deque<trace_handle> traces_type;
         typedef traces_type::iterator iterator;
         typedef traces_type::const_iterator const_iterator;
         iterator begin()
         { return traces.begin(); }
         const_iterator begin() const
         { return traces.begin(); }
         
         /**
          * @return Returns the last iterator in the series of traces
          * associated with this axis.
          */
         iterator end()
         { return traces.end(); }
         const_iterator end() const
         { return traces.end(); }

         /**
          * @return Returns true if there are no traces associated with this
          * axis.
          */
         bool empty() const
         { return traces.empty(); }

         /**
          * @return Returns the number of traces associated with this axis.
          */
         typedef traces_type::size_type size_type;
         size_type size() const
         { return traces.size(); }

         /**
          * Associates the specified trace with this axis.
          */
         void push_back(trace_handle trace)
         { traces.push_back(trace); }

         /**
          * Removes the trace specified by the iterator.
          *
          * @param ti Specifies the trace iterator to remove.
          */
         void erase(iterator ti)
         { traces.erase(ti); }

         /**
          * Removes the set of traces specified by the given range.
          *
          * @param begin Specifies the beginning of the range.
          *
          * @param end Specifies the end of the range.
          */
         void erase(iterator begin, iterator end)
         { traces.erase(begin, end); }

         /**
          * Removes the specified trace from the set managed by this axis.
          */
         void erase(trace_handle &trace)
         {
            iterator ti(std::find(begin(), end(), trace));
            if(ti != end())
               traces.erase(ti);
         }

         /**
          * Removes all traces from this axis.
          */
         void clear()
         { traces.clear(); }

         /**
          * @return Returns the first trace.
          */
         typedef trace_handle value_type;
         value_type &front()
         { return traces.front(); }
         value_type const &front() const
         { return traces.front(); }

         /**
          * @return Returns the last trace.
          */
         value_type &back()
         { return traces.back(); }
         value_type const &back() const
         { return traces.back(); }
         
         // @endgroup:

         /**
          * Generates the scale labels and layout for this axis.
          *
          * @param space_ Specifies the amount of space allocated for this axis.
          *
          * @return Returns the layout rectangle for this axis.
          */
         typedef LightSharedPtr<NestedRect> layout_handle;
         layout_handle &generate_axis_rect(double space_);

         /**
          * @return Returns the position associated with the specified value.
          *
          * @param value Specifies the value to evaluate.
          */
         double value_to_pos(value_handle value);
         double value_to_pos(double value)
         { return value_to_pos(new Expression::Operand(value, 0)); }

         /**
          * @return Returns a value associated with the specified position.
          *
          * @param pos Specifies the position to evaluate.
          */
         value_handle pos_to_value(double pos);

         /**
          * Implements the code that draws items associated with this axis.
          *
          * @param draw_grid Set to true (default) if the major and minor grid lines should
          * be drawn.
          */
         virtual void draw(bool draw_grid = true);

         /**
          * Adds a highlight region to this axis.
          *
          * @param start Specifies the starting value for the highlight region.
          *
          * @param stop Specifies the stopping value for the highlight region.
          *
          * @param brush Specifies the brush used to paint the region.
          *
          * @param colour Specifies a solid colour for the highlight brush.
          */
         virtual GraphAxis &add_highlight(
            value_handle start,
            value_handle stop,
            brush_handle brush,
            void const *owner = 0)
         {
            highlights.push_back(GraphAxisHighlight(start, stop, brush, owner));
            return *this;
         }
         virtual GraphAxis &add_highlight(
            value_handle start,
            value_handle stop,
            Colour const &colour,
            void const *owner = 0)
         {
            highlights.push_back(
               GraphAxisHighlight(
                  start, stop, driver->make_brush(BrushInfo().set_colour(colour)), owner));
            return *this;
         }
         
         /**
          * Removes any highlights associated with this axis.
          */
         virtual GraphAxis &clear_highlights()
         {
            highlights.clear();
            return *this;
         }

         std::deque<GraphAxisHighlight> get_highlights()
         {
            return highlights;
         }
         
         /**
          * Removes any highlights associated with the specified owner.
          *
          * @param owner Specifies the owner pointer used when the highlight was added.
          */
         virtual GraphAxis &remove_highlight(void const *owner);

         /**
          * Draws the highlights for this axis.  This is defined as a public method because the
          * graph will need to draw highlights after everything else has been drawn.
          */
         virtual void draw_highlights();

         /**
          * @return Returns an object that stores the state of this axis.
          */
         typedef LightSharedPtr<GraphAxisState> state_handle;
         state_handle get_state() const
         {
            state_handle rtn(new GraphAxisState);
            rtn->time_format = time_format;
            rtn->auto_max = auto_max;
            rtn->auto_min = auto_min;
            rtn->min_value = min_value;
            rtn->max_value = max_value;
            rtn->auto_label = auto_label;
            rtn->auto_time = auto_time;
            rtn->max_offset = max_offset;
            rtn->min_offset = min_offset;
            rtn->fixed_decimals = scale->fixed_decimals;
            return rtn;
         }

         /**
          * Restores the state of this axis from the specified state object.
          */
         void set_state(GraphAxisState const &state)
         {
            time_format = state.time_format;
            auto_min = state.auto_min;
            auto_max = state.auto_max;
            min_value = state.min_value;
            max_value = state.max_value;
            auto_label = state.auto_label;
            auto_time = state.auto_time;
            max_offset = state.max_offset;
            min_offset = state.min_offset;
            scale->fixed_decimals = state.fixed_decimals;
         }

         /**
          * @return Returns the number of traces associated with this axis.
          */
         size_t get_traces_count() const
         { return traces.size(); }
         
      protected:
         /**
          * Implements the code that will draw the caption for this axis.
          */
         virtual void draw_caption();

         /**
          * Implements the code that will draw the axis line for this axis.
          */
         virtual void draw_axis();

         /**
          * Implements the code that will draw the labels for this axis.
          */
         virtual void draw_labels();

         /**
          * Implements the code that draws major ticks for this axis.
          */
         virtual void draw_major_ticks();

         /**
          * Implements the code that draws minor tics for this axis.
          */
         virtual void draw_minor_ticks();

         /**
          * Implements the code that draws the major grid lines for this axis.
          */
         virtual void draw_major_grid();

         /**
          * Implements the code that draws the minor grid lines for this axis.
          */
         virtual void draw_minor_grid();
         
      private:
         /**
          * Specifies the graph that owns this axis.
          */
         Graph *graph;

         /**
          * Specifies the driver for this axis.
          */
         LightSharedPtr<Driver> driver;

         /**
          * Specifies the position of this axis.
          */
         axis_type_code axis_type;

         /**
          * Sets to true if this axis is marked as the domain axis.
          */
         bool const domain_axis;

         /**
          * Specifies the format specification for a time based scale.
          */
         StrAsc time_format;

         /**
          * Specifies the caption for this axis.
          */
         StrUni caption;

         /**
          * Specifies the font used for the caption.
          */
         font_handle caption_font;

         /**
          * Specifies the colour used for the caption.
          */
         Colour caption_colour;

         /**
          * Specifies the caption angle in degrees.
          */
         double caption_angle;

         /**
          * Set to true if the caption should be visible.
          */
         bool caption_visible;

         /**
          * Specifies whether the minimum value will be automatically
          * calculated for this axis.
          */
         bool auto_min;

         /**
          * Specifies whether the maximum value will be automatically
          * calculated for this axis.
          */
         bool auto_max;

         /**
          * Specifies an offset in pixels that will be applied to the minimum
          * scale value.
          */
         double min_offset;

         /**
          * Specifies an offset in pixels that will be applied to the maximum
          * scale value.
          */
         double max_offset;

         /**
          * Specifies the application provided label interval.
          */
         double increment;

         /**
          * Specifies the units for the label interval for a time domain axis.
          */
         increment_units_type increment_units;

         /**
          * Specifies the pen used for drawing the major grid.
          */
         pen_handle major_grid_pen;

         /**
          * Specifies the minor grid pen.
          */
         pen_handle minor_grid_pen;

         /**
          * Specifies the pen used for drawing major ticks.
          */
         pen_handle major_ticks_pen;

         /**
          * Specifies the pen used for drawing minor ticks.
          */
         pen_handle minor_ticks_pen;

         /**
          * Specifies the number of minor ticks that should be shown for each label interval.
          */
         int4 minor_tick_count;

         /**
         * Specifies how long the axis tick marks are
         */
         int4 major_tick_length;
         int4 minor_tick_length;

         /**
          * Specifies the pen used for drawing the line associated with this
          * axis.
          */
         pen_handle axis_pen;

         /**
          * Specifies whether labels are visible for this axis.
          */
         bool labels_visible;

         /**
          * Specifies the colour used for labels.
          */
         Colour labels_colour;

         /**
          * Specifies whether labels will be calculated automatically for this
          * axis.
          */
         bool auto_label;


         /**
         * This is used when we don't want to automatically determine the number
         * of lables and don't specify the increment, but calculate the increment,
         * based on the number of tick marks (or steps)
         */
         bool use_fixed_num_steps;
         int  num_fixed_steps;

         /**
          * Specifies the scale used for this axis.
          */
         scale_handle scale;

         /**
          * Specifies the minimum width or height of the labels rectangle.
          */
         double labels_size;

         /**
          * Specifies whether time based max and min values should be
          * calculated automatically.
          */
         bool auto_time;

         /**
          * Specifies the set of traces associated with this axis.
          */
         traces_type traces;

         /**
          * Specifies the layout rectangle for this axis.
          */
         layout_handle axis_rect;

         /**
          * Specifies the layout rectangle for the axis title.
          */
         layout_handle caption_rect;

         /**
          * Specifies the rectangle for the scale.
          */
         layout_handle scale_rect;

         /**
          * Specifies the layout used for labels.
          */
         layout_handle labels_rect;

         /**
          * Specifies the application specified minimum value for this axis.
          */
         value_handle min_value;

         /**
          * Stores the application specified maximum value for this axis.
          */
         value_handle max_value;

         /**
          * Specifies the values for minor ticks on this axis.
          */
         typedef Scale::minor_ticks_type minor_ticks_type;
         minor_ticks_type minor_ticks;

         /**
          * Specifies the highlights associated with this axis.
          */
         typedef std::deque<GraphAxisHighlight> highlights_type;
         highlights_type highlights;
      };
   };
};


#endif
