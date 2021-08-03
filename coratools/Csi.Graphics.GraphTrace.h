/* Csi.Graphics.GraphTrace.h

   Copyright (C) 2015, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 22 January 2015
   Last Change: Monday 08 October 2018
   Last Commit: $Date: 2020-07-14 15:42:11 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_GraphTrace_h
#define Csi_Graphics_GraphTrace_h

#include "Csi.Graphics.Pen.h"
#include "Csi.Graphics.Font.h"
#include "Csi.Graphics.Driver.h"
#include "Csi.Graphics.NestedRect.h"
#include "Csi.LightSharedPtr.h"
#include "Csi.Expression.Token.h"
#include "Csi.Graphics.ShapeBase.h"


namespace Csi
{
   namespace Graphics
   {
      class Graph;
      class GraphAxis;
      class GraphTrace;
      class ColoursController;
      

      /**
       * Defines a point object on a trace.
       */
      class GraphPoint
      {
      public:
         /**
          * Reference to the value for the domain.
          */
         typedef LightPolySharedPtr<Expression::Token, Expression::Operand> value_handle;
         value_handle x;

         /**
          * Reference to the value for the range.
          */
         value_handle y;

         /**
         * Reference to a direction, for wind barb graphs
         */
         value_handle wind_dir;


         /**
          * Reference to the trace that owns this point.
          */
         GraphTrace *owner;

         /**
          * Construct a point.
          *
          * @param x_ Specifies the domain value for this point.
          *
          * @param y_ Specifies the range value for this point.
          *
          * @param owner_ Specifies the trace that owns this point.
          */
         GraphPoint(value_handle x_, value_handle y_, GraphTrace *owner_):
            x(x_),
            y(y_),
            wind_dir(0),
            owner(owner_)
         { }

         GraphPoint(value_handle x_, value_handle y_, value_handle z_, GraphTrace *owner_):
            x(x_),
            y(y_),
            wind_dir(z_),
            owner(owner_)
         { }

         /**
          * Default constructor
          */
         GraphPoint():
            owner(0)
         { }

         /**
          * Copy constructor
          */
         GraphPoint(GraphPoint const &other):
            x(other.x),
            y(other.y),
            wind_dir(other.wind_dir),
            owner(other.owner)
         { }

         /**
          * Copy operator
          */
         GraphPoint &operator =(GraphPoint const &other)
         {
            x = other.x;
            y = other.y;
            wind_dir = other.wind_dir;
            owner = other.owner;
            return *this;
         }
      };



      /**
       * Specifies the preferred direction of a mark relative to its point.
       */
      enum GraphTraceMarkDirType
      {
         mark_dir_north = 90,
         mark_dir_north_west = 135,
         mark_dir_west = 180,
         mark_dir_south_west = 225,
         mark_dir_south = 270,
         mark_dir_south_east = 315,
         mark_dir_east = 0,
         mark_dir_north_east = 45
      };
      
      
      /**
       * Defines an object that tracks a mark on the graph trace.
       */
      class GraphTraceMark
      {
      private:
         /**
          * Specifies the point for this mark.
          */
         GraphPoint point;

         /**
          * Specifies the label for this mark.
          */
         StrAsc label;

         /**
          * Set to true if this mark was created by user interaction.
          */
         bool user_specified;

         /**
          * Specifies the preferred direction of this mark relative to its point.
          */
         GraphTraceMarkDirType preferred_direction;
         
      public:
         /**
          * Constructor.
          *
          * @param point_ Specifies the point for this mark.
          *
          * @param label_ Specifies the label for this mark.
          *
          * @param user_specified_ Set to true if this mark was created through user (mouse) action.
          *
          * @param preferred_direction_ Specifies the preferred direction of this mark relative
          * to its associated point.
          */
         GraphTraceMark(
            GraphPoint const &point_,
            StrAsc const &label_,
            bool user_specified_,
            GraphTraceMarkDirType preferred_direction_ = mark_dir_north):
            point(point_),
            label(label_),
            user_specified(user_specified_),
            preferred_direction(preferred_direction_)
         { }

         /**
          * @return Returns a reference to the point for this mark.
          */
         GraphPoint const &get_point() const
         { return point; }

         /**
          * @return Returns the label for this mark.
          */
         StrAsc const &get_label() const
         { return label; }

         /**
          * @return Returns true if this mark was created by user interaction.
          */
         bool get_user_specified() const
         { return user_specified; }

         /**
          * @return Returns the preferred direction for this mark relative to its point.
          */
         GraphTraceMarkDirType get_preferred_direction() const
         { return preferred_direction; }
      };
      
      
      /**
       * Defines an object that represents a data series on a graph.
       */
      class GraphTrace
      {
      public:
         /**
          * Constructor
          */
         typedef LightSharedPtr<Driver> driver_handle;
         GraphTrace(driver_handle driver_);

         /**
          * @return Returns the range axis type for this trace.
          */
         enum vertical_axis_type
         {
            range_left,
            range_right
         };
         vertical_axis_type get_vertical_axis() const
         { return vertical_axis; }

         /**
          * Sets the axis type for the range for this trace.
          *
          * @param axis Specifies the axis associated with this range.
          */
         GraphTrace &set_vertical_axis(vertical_axis_type axis);

         /**
          * @return Returns a code that identifies how this trace will be drawn.
          */
         enum trace_type_code
         {
            trace_line = 0,
            trace_bar = 1,
            trace_point = 2,
            trace_area = 3,
            trace_wind_dir = 4,
            trace_reference_line = 5,
            trace_vertical_reference_line = 6,
            trace_reference_dot = 7,
            trace_types_count
         };
         trace_type_code get_trace_type() const
         { return trace_type; }

         /**
          * @param value Specifies how this trace will be represented.
          */
         GraphTrace &set_trace_type(trace_type_code value)
         {
            trace_type = value;
            return *this;
         }

         /**
         * @param value_:  Specifies position of where to place a horizontal line.
         */
         GraphTrace &set_reference_line(double value_)
         {
            reference_line = value_;
            return *this;
         }

         GraphTrace &set_reference_dot(double value_)
         {
            reference_dot = value_;
            return *this;
         }

         /**
          * @return Returns the title for this trace.
          */
         StrUni const &get_title() const
         { return title; }

         /**
          * @param value Specifies the title for this trace.
          */
         GraphTrace &set_title(StrUni const &value)
         {
            title = value;
            thresholds.high_legend_text = title;
            thresholds.low_legend_text = title;
            thresholds.high_legend_text.append(L"_high");
            thresholds.low_legend_text.append(L"_low");
            return *this;
         }
         
         /**
          * @return Returns the pen that will be used for drawing line traces.
          */
         typedef LightSharedPtr<Pen> pen_handle;
         pen_handle get_pen() const
         { return pen; }

         /**
          * @param pen_ Specifies the pen that will be used to draw lines for this trace.
          *
          * @param desc Specifies a description for the pen.
          */
         GraphTrace &set_pen(pen_handle pen_)
         {
            pen = pen_;
            return *this;
         }
         GraphTrace &set_pen(PenInfo const &desc)
         { return set_pen(driver->make_pen(desc)); }

         /**
          * @return Returns true if this trace is set up to draw stair steps between points.
          */
         bool get_use_stairs() const
         { return use_stairs; }

         /**
          * @param value Set to true if thus trace is to be drawn as stair
          * steps between points.
          */
         GraphTrace &set_use_stairs(bool value)
         {
            use_stairs = value;
            return *this;
         }

         /**
          * @return Returns the shape that will be used to draw the symbol.
          */
         typedef LightSharedPtr<ShapeBase> symbol_handle;
         symbol_handle get_point_type() const
         { return point_type; }

         /**
          * @param value Specifies the shape that will be drawn for this trace.
          */
         GraphTrace &set_point_type(symbol_handle value)
         {
            point_type = value;
            return *this;
         }

         /**
          * @param code Specifies the code for the symbol type that will be drawn for this trace.
          */
         GraphTrace &set_point_type(ShapeBase::symbol_type code)
         {
            point_type = ShapeBase::make_shape(code);
            return *this;
         }

         /**
          * @param controller Specifies the object that can be used to specify the colours of
          * individual points.
          */
         typedef LightSharedPtr<ColoursController> colours_controller_handle;
         GraphTrace &set_colours_controller(colours_controller_handle controller)
         { colours_controller = controller; 
           return *this;
         }

         /**
          * @return Returns the colours controller object.
          */
         colours_controller_handle get_colours_controller()
         { return colours_controller; }

         /**
          * @return Returns the colour used for drawing points.
          */
         Colour const &get_point_colour() const
         { return point_colour; }

         /**
         * @return Returns the colour used for drawing points, with an age gradient
         */
         Colour const &get_point_colour_gradient() const
         { return point_colour_gradient; }

         /**
          * @param colour Specifies the colour that should be used for drawing
          * points.
          */
         GraphTrace &set_point_colour(Colour const &colour_)
         {
            point_colour = colour_;
            PenInfo penInfo = point_pen->get_desc();
            penInfo.set_colour(colour_);
            point_pen = driver->make_pen(penInfo);
            point_brush = driver->make_brush(BrushInfo().set_colour(colour_));
            return *this;
         }

         GraphTrace &set_point_colour_gradient(Colour const &colour_)
         {
            point_colour_gradient = colour_;
            return *this;
         }


         /**
          * @return Returns the size of points for this trace.
          */
         double get_point_size() const
         { return point_size; }

         /**
          * @param size Specifies the size for points on this trace.
          */
         GraphTrace &set_point_size(double size)
         {
            point_size = size;

            double width = (size-1) * (4.f / 20.f) + 1;
            PenInfo penInfo = point_pen->get_desc();
            penInfo.set_width(width);
            point_pen = driver->make_pen(penInfo);

            if (thresholds.low_point_pen != nullptr)
            {
               penInfo = thresholds.low_point_pen->get_desc();
               penInfo.set_width(width);
               thresholds.low_point_pen = driver->make_pen(penInfo);
            }

            if (thresholds.high_point_pen != nullptr)
            {
               penInfo = thresholds.high_point_pen->get_desc();
               penInfo.set_width(width);
               thresholds.high_point_pen = driver->make_pen(penInfo);
            }

            return *this;
         }

         /**
          * @return Returns the time offset applied to the x axis values
          * associated with this trace.
          */
         int8 get_time_offset() const
         { return time_offset; }

         /**
          * @param value Specifies the time offset that should be applied to
          * all points in this trace.
          */
         GraphTrace &set_time_offset(int8 value)
         {
            time_offset = value;
            return *this;
         }

         /**
         * @return Returns the whether marks are enabled for this trace
         */
         bool const get_marks_enabled() const
         { return mark_props.enabled; }

         /**
         * @param font Specifies whether to show all marks, all the time
         */
         GraphTrace &set_marks_enabled(bool enable_)
         {
            mark_props.enabled = enable_;
            if (!mark_props.enabled)
               clear_marks();
            return *this;
         }


         /**
          * @return Returns the font used to draw marks.
          */
         typedef LightSharedPtr<Font> font_handle;
         font_handle const &get_marks_font() const
         { return mark_props.font; }

         /**
          * @param font Specifies the font used to draw marks.
          */
         GraphTrace &set_marks_font(Csi::Graphics::FontInfo desc)
         {
            mark_props.font = driver->make_font(desc);
            return *this;
         }

         /**
         * @return Returns the whether to show all marks, all the time
         */
         bool const get_marks_always_show() const
         { return mark_props.always_show; }

         /**
         * @param font Specifies whether to show all marks, all the time
         */
         GraphTrace &set_marks_always_show(bool show_)
         {
            clear_marks();
            mark_props.always_show = show_;
            return *this;
         }

         /**
         * @return Returns the decimation to use for marks, if mark_props.always_show is true
         */
         int const get_marks_decimation() const
         { return mark_props.decimation; }

         /**
         * @param font Specifies the decimation for marks, if mark_props.always_show is true
         */
         GraphTrace &set_marks_decimation(int decimation_)
         {
            mark_props.decimation = decimation_;
            return *this;
         }

         /**
         * @return Returns the whether to show the x value in the marks
         */
         bool const get_marks_show_x_value() const
         { return mark_props.show_x; }

         /**
         * @param font Specifies whether to show the y value in the marks
         */
         GraphTrace &set_marks_show_x_value(bool enable_)
         {
            mark_props.show_x = enable_;
            return *this;
         }

         /**
         * @return Returns the whether to show the x value in the marks
         */
         bool const get_marks_show_y_value() const
         { return mark_props.show_y; }

         /**
         * @param font Specifies whether to show the y value in the marks
         */
         GraphTrace &set_marks_show_y_value(bool enable_)
         {
            mark_props.show_y = enable_;
            return *this;
         }

         /**
         * @return Returns the any units to show with the marks
         */
         StrUni get_marks_y_units() const
         { return mark_props.y_units; }

         /**
         * @param font Specifies the units to show with the marks
         */
         GraphTrace &set_marks_y_units(StrUni units_)
         {
            mark_props.y_units = units_;
            return *this;
         }

         /**
         * @return Returns the any units to show with the marks
         */
         StrUni get_marks_x_units() const
         { return mark_props.x_units; }

         /**
         * @param font Specifies the units to show with the marks
         */
         GraphTrace &set_marks_x_units(StrUni units_)
         {
            mark_props.x_units = units_;
            return *this;
         }

         /**
         * @return Returns the whether to show the x value in the marks
         */
         StrUni const get_marks_time_format() const
         { return mark_props.time_format; }

         /**
         * @param font Specifies whether to show the y value in the marks
         */
         GraphTrace &set_marks_time_format(StrUni format_)
         {
            mark_props.time_format = format_;
            return *this;
         }

         /**
         * @return Returns the whether how to draw the mark, if it has a transparent background or not
         */
         bool const get_marks_transparent_back() const
         { return mark_props.transparent_back; }

         /**
         * @param font Specifies whether to show the bg color when drawing the mark
         */
         GraphTrace &set_marks_transparent_back(bool b_)
         {
            mark_props.transparent_back = b_;
            return *this;
         }

         /**
         * @return Returns the colour to use for drawing the mark
         */
         Colour const get_marks_colour() const
         { return mark_props.colour; }

         /**
         * @param c_ Specifies the colour for drawing the mark
         */
         GraphTrace &set_marks_colour(Colour c_)
         {
            mark_props.colour = c_;
            return *this;
         }

         /**
         * @return Returns if the edges of the marks are rounded or not (not available for all drivers), if the mark is not transparent
         */
         bool const get_marks_rounded_back() const
         { return mark_props.rounded_back; }

         /**
         * @param b_ Sets whether to round the corners of the mark or not (not availabel for all drivers), if the mark is not a tranparent
         */
         GraphTrace &set_marks_rounded_back(bool b_)
         {
            mark_props.rounded_back = b_;
            return *this;
         }


         /**
          * @param colour Specifies the colour of the brush to use when the
          *        value is higher than high_threshold
          */
         GraphTrace &set_high_threshold_pen(PenInfo &info)
         {
            thresholds.high_pen = driver->make_pen(info);
            return *this;
         }

         GraphTrace &set_low_threshold_pen(PenInfo &info)
         {
            thresholds.low_pen = driver->make_pen(info);
            return *this;
         }

         GraphTrace &set_high_threshold_point_colour(Colour colour_)
         {
            thresholds.high_point_colour = colour_;

            PenInfo penInfo = thresholds.high_point_pen->get_desc();
            penInfo.set_colour(colour_);
            thresholds.high_point_pen   = driver->make_pen(penInfo);
            thresholds.high_point_brush = driver->make_brush(BrushInfo().set_colour(colour_));
            return *this;
         }

         GraphTrace &set_low_threshold_point_colour(Colour colour_)
         {
            thresholds.low_point_colour = colour_;
            PenInfo penInfo = thresholds.low_point_pen->get_desc();
            penInfo.set_colour(colour_);
            thresholds.low_point_pen   = driver->make_pen(penInfo);
            thresholds.low_point_brush = driver->make_brush(BrushInfo().set_colour(colour_));
            return *this;
         }


         GraphTrace &set_high_threshold_age_point_colour(Colour colour_)
         {
            thresholds.high_age_point_colour = colour_;
            return *this;
         }

         GraphTrace &set_low_threshold_age_point_colour(Colour colour_)
         {
            thresholds.low_age_point_colour = colour_;
            return *this;
         }


         /*GraphTrace &set_high_point_size(double size)
         {
            thresholds.high_point_size = size;

            double width = (size-1) * (4.f / 20.f) + 1;
            PenInfo penInfo = thresholds.high_point_pen->get_desc();
            penInfo.set_width(width);
            thresholds.high_point_pen = driver->make_pen(penInfo);

            return *this;
         }

         GraphTrace &set_low_point_size(double size)
         {
            thresholds.low_point_size = size;

            double width = (size-1) * (4.f / 20.f) + 1;
            PenInfo penInfo = thresholds.low_point_pen->get_desc();
            penInfo.set_width(width);
            thresholds.low_point_pen = driver->make_pen(penInfo);

            return *this;
         }*/

         /**
          * @return Specifies the brush used to draw an upward pointing bar.
          */
         brush_handle get_bar_up_brush() const
         { return bar_up_brush; }

         /**
          * @return Returns the brush used to draw a downward pointing bar.
          */
         brush_handle get_bar_down_brush() const
         { return bar_down_brush; }

         /**
          * @param up Specifies the brush that should be used for an upward pointing bar.
          *
          * @param down Specifies the brush that should be used for downward pointing bars.
          */
         GraphTrace &set_bar_brushes(brush_handle up, brush_handle down)
         {
            bar_up_brush = up;
            bar_down_brush = down;
            return *this;
         }

         /**
          * @param colour Specifies the colour of the brush to use with the bar.
          */
         GraphTrace &set_bar_colour(Colour const &colour)
         {
            bar_up_brush = driver->make_brush(colour);
            bar_down_brush = bar_up_brush;
            return *this;
         }

         GraphTrace &set_bar_up_colour(Colour const &colour)
         {
            bar_up_brush = driver->make_brush(colour);
            return *this;
         }


         GraphTrace &set_bar_down_colour(Colour const &colour)
         {
            bar_down_brush = driver->make_brush(colour);
            return *this;
         }

         GraphTrace &set_bar_down_pen(Colour const &colour)
         {
            bar_down_pen = driver->make_pen(PenInfo().set_colour(colour));
            return *this;
         }

         /**
          * @return Returns the position of this trace ralative to other bar
          * traces on the same graph.
          */
         int get_bar_position() const
         { return bar_position; }

         /**
          * @param value Specifies a position relative to other bar series on
          * the same graph.
          */
         GraphTrace &set_bar_position(int value)
         {
            bar_position  = value;
            return *this;
         }

         /**
          * Sets the domain axis for this trace.
          *
          * @param axis Specifies the domain axis reference.
          */
         void set_domain_axis(GraphAxis *axis)
         { domain_axis = axis; }

         /**
          * @return Returns the domain axis for this trace.
          */
         GraphAxis *get_domain_axis()
         { return domain_axis; }

         /**
          * Sets the domain axis reference for this trace.
          *
          * @param axis Specifies the range axis pointer for this trace.
          */
         void set_range_axis(GraphAxis *axis)
         { range_axis = axis; }

         /**
          * @return Returns the range axis for this trace.
          */
         GraphAxis *get_range_axis()
         { return range_axis; }


         /**
          * @return Returns the pen used to draw the border around a bar.
          */
         pen_handle get_bar_pen() const
         { return bar_pen; }

         pen_handle get_bar_down_pen() const
         { return bar_down_pen; }

         /**
          * Sets the pen used to draw a border around the bar.
          *
          * @param pen Specifies the pen that should be used.
          *
          * @param desc Specifies a description for the pen.
          */
         GraphTrace &set_bar_pen(pen_handle pen)
         {
            bar_pen = pen;
            return *this;
         }
         GraphTrace &set_bar_pen(PenInfo const &desc)
         { 
            return set_bar_pen(driver->make_pen(desc)); 
         }

         /**
         * Sets the pen used to draw a border around the bar, when the values are negative
         *
         * @param pen Specifies the pen that should be used.
         *
         * @param desc Specifies a description for the pen.
         */
         GraphTrace &set_bar_down_pen(pen_handle pen)
         {
            bar_down_pen = pen;
            return *this;
         }

         GraphTrace &set_bar_down_pen(PenInfo const &desc)
         { return set_bar_down_pen(driver->make_pen(desc)); }


         /**
         * @Determine if we should show one or two items in the legend
         *  (one for positive and one for negative) for bar and area series.
         */
         bool get_bar_separate_legend()
         { 
            return bar_show_two_legend_items;
         }

         GraphTrace &set_bar_separate_legend(bool bSeparate_)
         { 
            bar_show_two_legend_items = bSeparate_;
            return *this;
         }
         
         /**
          * @return Returns true if this trace is visible.
          */
         bool get_visible() const
         { return visible; }

         /**
          * @param value Set to true if this trace should be be visible in the graph.
          */
         GraphTrace &set_visible(bool value);

         /**
         * used for Drawing Lines
         */
         bool get_enable_high_threshold()       {return thresholds.enable_high;}
         void set_enable_high_threshold(bool b) { thresholds.enable_high = b; }

         bool get_enable_low_threshold()        {return thresholds.enable_low;}
         void set_enable_low_threshold(bool b)  { thresholds.enable_low = b; }

         double get_high_threshold()            {return thresholds.high_value;}
         void set_high_threshold(double d)      { thresholds.high_value = d;}

         double get_low_threshold()             {return thresholds.low_value;}
         void set_low_threshold(double d)       { thresholds.low_value = d;}

         pen_handle get_high_threshold_pen()    {return thresholds.high_pen;}
         pen_handle get_low_threshold_pen()     {return thresholds.low_pen;}

         Colour get_high_threshold_point_colour()  {return thresholds.high_point_colour;}
         Colour get_low_threshold_point_colour()  {return thresholds.low_point_colour;}

         Colour get_high_threshold_age_point_colour() {return thresholds.high_age_point_colour;}
         Colour get_low_threshold_age_point_colour()  {return thresholds.low_age_point_colour;}

         pen_handle get_point_pen()             {return point_pen;}
         brush_handle get_point_brush()         {return point_brush;}

         pen_handle get_high_point_pen()        {return thresholds.high_point_pen;}
         brush_handle get_high_point_brush()    {return thresholds.high_point_brush;}

         pen_handle get_low_point_pen()         {return thresholds.low_point_pen;}
         brush_handle get_low_point_brush()     {return thresholds.low_point_brush;}

         //double get_high_point_size()         {return thresholds.high_point_size;}
         //double get_low_point_size()          {return thresholds.low_point_size;}

         bool get_show_high_in_legend() { return thresholds.show_high_in_legend; }
         void set_show_high_in_legend(bool show_) {thresholds.show_high_in_legend = show_;}

         bool get_show_low_in_legend() {return thresholds.show_low_in_legend;}
         void set_show_low_in_legend(bool show_) {thresholds.show_low_in_legend = show_;}

         StrUni get_legend_high_text() {return thresholds.high_legend_text;}
         void set_legend_high_text(StrUni text_) {thresholds.high_legend_text= text_;}

         StrUni get_legend_low_text() {return thresholds.low_legend_text;}
         void set_legend_low_text(StrUni text_) {thresholds.low_legend_text= text_;}
         
         /**
          * Sets the trace properties to their default values.
          */
         virtual void set_default_properties();

         /**
          * Writes the parameters for this trace to the specified XML structure.
          *
          * @param elem Specifies the destination XML object.
          */
         virtual void write(Xml::Element &elem);

         /**
          * Reads the properties for this trace from the specified XML structure.
          *
          * @param elem Specifies the source XML object.
          */
         virtual void read(Xml::Element &elem);

         /**
          * Evaluates the range of values stored for this trace on the domain or range axis.
          *
          * @param for_domain Set to true if the domain range is to be evaluated.
          *
          * @return Returns a pair of doubles.  The first of this pair is the minimum value for the
          * axis and the second of the pair is the maximum value of the axis.
          */
         typedef std::pair<double, double> bounds_type;
         bounds_type get_bounds(bool for_domain = true) const;

         // @group; declarations and methods for acting as a container of points.

         /**
          * @return Returns the first iterator in the list of points.
          */
         typedef GraphPoint value_type;
         typedef std::deque<GraphPoint> points_type;
         typedef points_type::iterator iterator;
         typedef points_type::const_iterator const_iterator;
         iterator begin()
         { return points.begin(); }
         const_iterator begin() const
         { return points.begin(); }

         /**
          * @return Returns the end iterator for the container of points.
          */
         iterator end()
         { return points.end(); }
         const_iterator end() const
         { return points.end(); }

         /**
          * @return Returns the first value in the container of points.
          */
         value_type &front()
         { return points.front(); }
         value_type const &front() const
         { return points.front(); }

         /**
          * @return Returns the last value in the container of points.
          */
         value_type &back()
         { return points.back(); }
         value_type const &back() const
         { return points.back(); }

         /**
          * @return Returns true if there are no points for this trace.
          */
         bool empty() const
         { return points.empty(); }

         /**
          * Removes all points for this trace.
          */
         void clear()
         { 
            points.clear(); 
            clear_marks(false);
         }

         /**
          * @return Returns the number of points stored in this trace.
          */
         points_type::size_type size() const
         { return points.size(); }

         /**
          * Removes the point at the specified iterator.
          *
          * @param start Specifies the start of the iterator range.
          *
          * @param end Specifies the position one past the iterator range.
          */
         void erase(iterator start)
         { erase(start, start + 1); }
         void erase(iterator start, iterator end);
            
         // @endgroup:

         /**
          * Adds a point to the set managed for this trace.
          *
          * @param x Specifies the domain value for the point.
          *
          * @param y Specifies the range value for the point.
          *
          * @return Returns a reference to the point that was added.
          */
         typedef LightPolySharedPtr<Expression::Token, Expression::Operand> value_handle;
         value_type add_point(value_handle x, value_handle y);
         value_type add_point(value_handle x, value_handle y, value_handle z);

         /**
          * Removes points that have value time stamps older than the specified cut-off date.
          *
          * @param cutoff Specifies the date for which all older points will be removed.
          */
         void remove_older_points(Csi::LgrDate const &cutoff);

         /**
         * Removes the first so many points so that the size of points does not exceed 
         * max_num_points, used for XY traces
         *
         * @param cutoff Specifies the maximum number of points we should have in our points list
         */
         void remove_older_points(size_t max_num_points);

         /**
          * Sets the current mark to the specified point.
          *
          * @param mark Specifies the point for the current mark.
          */
         GraphTrace &set_current_mark(GraphPoint const &mark)
         {
            clear_marks(true);
            if(mark.x != 0 && mark.y != 0)
               marks.push_back(new mark_type(mark, format_mark(mark), true));
            return *this;
         }

         /**
          * Adds the specified mark to the set managed for this trace.
          *
          * @param point Specifies the point for this mark.
          *
          * @param label Specifies the label for this mark.
          *
          * @param user_specified Set to true if this marks was created through user interaction.
          *
          * @param preferred_dir Specifies the preferred direction of the mark relative to
          * the point.
          */
         void add_mark(
            GraphPoint const &point,
            StrAsc const &label,
            bool user_specified = false,
            GraphTraceMarkDirType preferred_dir = mark_dir_north)
         {
            marks.push_back(new mark_type(point, label, user_specified, preferred_dir));
         }

         /**
          * Clears marks for this trace.
          *
          * @param current_only Set to true (the default) if only user specified marks should be cleared.
          */
         void clear_marks(bool current_only = true);

         /**
          * @return Returns the first user specified mark or null if there are such marks.
          */
         typedef GraphTraceMark mark_type;
         typedef SharedPtr<mark_type> mark_handle;
         mark_handle get_current_mark() const;
         
         /**
          * @return Returns a rectangle the represents the size of this legend.
          *
          * @param legend_font Specifies the font used for drawing legend labels.
          */
         typedef LightSharedPtr<NestedRect> rect_handle;
         void make_legend_rect(rect_handle &, font_handle &legend_font);
         rect_handle make_legend_rect(StrUni &title, font_handle &legend_font, rect_handle &text_rect, rect_handle &symbol_rect);
         void get_show_thresholds_in_legend(bool &show_high_in_legend, bool &show_low_in_legend);
         /**
          * Draws this trace's symbol and title at the rectangles calculated in make_legend_rect().
          *
          * @param legend_font Specifies the legend font.
          *
          * @param background Specifies the background colour.
          *
          * @param foreground Specifies the foreground colour.
          */
         virtual void draw_legend(
            StrUni &title,
            rect_handle &text_rect,
            rect_handle &symbol_rect,
            pen_handle &pen,
            Colour &point_colour,
            pen_handle &bar_pen,
            brush_handle &bar_brush,
            font_handle &legend_font,
            Colour const &foreground,
            Colour const &background);

         virtual void draw_legend(
            font_handle &legend_font,
            Colour const &foreground,
            Colour const &background);

         /**
          * Implements the code that draws this trace.
          */
         virtual void draw();

         /**
          * Evaluates whether there a point on this trace that lies near the
          * specified screen position and within the specified radius.
          *
          * @return Returns a pair that holds the closest point (if any) in the
          * first position and the distance of that point in the second.  If
          * there is no point within the specified radius, the returned point
          * will have invalid x and y coordinates (null pointers).
          *
          * @param pos Specifies the screen coordinate to evaluate.
          *
          * @param radius Specifies the for which to search.
          */
         typedef std::pair<GraphPoint, double> hit_test_value;
         virtual hit_test_value hit_test(Point const &pos, double radius) const;

      protected:
         /**
          * @return Returns a formatted mark for the specified point.
          *
          * @param point Specifies the point to be formatted.
          */
         virtual StrAsc format_mark(GraphPoint const &point);

      private:
         /**
         * Implements the code that fills in the marks
         */
         void get_marks();

         /**
         * Implements the code to draw lines for this trace.
         */
         void draw_lines();

         void draw_lines_solid();
         void draw_lines_with_thresholds();

         typedef struct {
            Point coord;
            pen_handle pen;
            double value;
         } multiColored_t;
         typedef std::list<multiColored_t> myPoints_t;

         void draw_threshold_colored_line(multiColored_t &prev_point, multiColored_t &point, Driver::points_type &drawn);

         /**
          * Implements the code that draws points for this trace.
          */
         void draw_points();

         /**
          * Implements the code to draw bars for this trace.
          */
         void draw_bars();


         /**
          * Draws a bar at the specified rectangle.
          *
          * @param rect Specifies the rectangle in which the bar must be contained.
          *
          * @param pointing_up Set to true if the shape for the bar should be pointing toward the top.
          */
         void draw_bar(Rect const &rect, bool pointing_up);

         /**
         * Implements the code to draw areas for this trace.
         */
         void draw_areas();
         void draw_area(Point const &point1, Point const &point2, bool pointing_up);

         /**
         * Implements the code to draw wind vectors for this trace.
         */
         void draw_wind_dirs();

         /**
         * Implements the code to drawing a reference line
         */
         void draw_reference_line();
         void draw_vertical_reference_line();
         void draw_reference_dot();

         /**
          * Draws the marks for this trace.
          */
         void draw_marks();

         /**
          * Implements the code that draws the specified mark.
          *
          * @param mark Specifies the mark to draw.
          */
         void draw_mark(mark_type const &mark);
         
      private:
         /**
          * Specifies the graph that owns this trace.
          */
         Graph *graph;

         /**
          * Specifies the axis for the range of this trace.
          */
         vertical_axis_type vertical_axis;

         /**
          * Specifies the title for this trace.
          */
         StrUni title;
         
         /**
          * Specifies how this trace will be represented.
          */
         trace_type_code trace_type;

         /**
          * Specifies the pen that will be used to draw lines for this trace.
          */
         pen_handle pen;

         /**
         * Specifies the value where we will draw a horizontal (or vertical for rotated graphs) line.
         */
         double reference_line;
         double reference_dot;

         /**
         * Specifies whether or not to use the threshold values
         */
         struct 
         {
            bool enable_high;
            bool enable_low;

            double high_value;
            double low_value;

            bool show_high_in_legend;
            bool show_low_in_legend;

            StrUni high_legend_text;
            StrUni low_legend_text;

            pen_handle high_pen;
            pen_handle low_pen;

            pen_handle high_point_pen;
            brush_handle high_point_brush;

            pen_handle low_point_pen;
            brush_handle low_point_brush;

            Colour high_point_colour;
            Colour low_point_colour;

            //double high_point_size;
            //double low_point_size;

            Colour high_age_point_colour;
            Colour low_age_point_colour;

            bool high_data_exists;
            bool low_data_exists;

         } thresholds;

         /**
          * Set to true of traces are to be drawn in stair steps.
          */
         bool use_stairs;

         /**
          * Specifies a code that will identify the type of point that will be
          * drawn for this trace.
          */
         symbol_handle point_type;

         /**
          * Specifies the colour used for drawing points for this trace.
          */
         Colour point_colour;

         /**
         * Specifies the pen and brush that will be used for drawing points
         */
         pen_handle point_pen;
         brush_handle point_brush;


         /**
          * Used to control the colour used for individual points.
          */
         colours_controller_handle colours_controller;

         /**
         * Specifies the end colour used for calculating the gradient.
         */
         Csi::Graphics::Colour point_colour_gradient;

         /**
          * Specifies the number of pixels that points should occupy.
          */
         double point_size;

         /**
          * Specifies an offset in milliseconds that will be added to time
          * value for this trace.
          */
         int8 time_offset;

         /**
         * Specifies the mark attributes
         */
         struct {
            bool enabled;                // marks are enabled
            bool always_show;            // show every mark (with decimation). Otherwise, only show the clicked mark
            int  decimation;              // the decimation to use for marks if marks.always_show is true

            bool show_x, show_y;         // Specifies if we are showing the X and Y values or not
            StrUni time_format;          // format to use for marks, if it includes a time portion
            StrUni y_units, x_units;     // Specifies any units string to include with the marks
            font_handle font;            // the font used to draw marks

            bool transparent_back;
            Colour colour;
            bool rounded_back;

         } mark_props;


         /**
          * Specifies the brush that will be use for upward pointing bars.
          */
         brush_handle bar_up_brush;

         /**
          * Specifies the brush that will be used for downward pointing bars.
          */
         brush_handle bar_down_brush;
         
         /**
          * Specifies the offset within a slot in which the bar will be drawn
          */
         int bar_position;

         /**
         * Specifies whether we should show two legend items or not for bar graphs
         */
         bool bar_show_two_legend_items;

         /**
          * Specifies the pen used to draw the borders around a bar.
          */
         pen_handle bar_pen;
         pen_handle bar_down_pen;

         /**
          * Specifies the graphics driver object responsible for measuring and
          * drawing.
          */
         driver_handle driver;

         /**
          * Specifies the axis used for the domain of this trace.
          */
         GraphAxis *domain_axis;

         /**
          * Specifies the axis used for the range of this trace.
          */
         GraphAxis *range_axis;

         /**
          * Specifies the list of points for this trace.
          */
         points_type points;

         /**
          * Stores inrformation about the legend position for this trace
          */
         struct
         {
            rect_handle text_rect;
            rect_handle symbol_rect;

            rect_handle text_high_rect;
            rect_handle symbol_high_rect;

            rect_handle text_low_rect;
            rect_handle symbol_low_rect;

         } legend_pos;

         /**
         /**
          * Specifies the collection of marks for this trace.
          */
         typedef std::deque<mark_handle> marks_type;
         marks_type marks;

         /**
          * Set to true if this trace should be drawn.
          */
         bool visible;

         friend class Graph;
      };


      /**
       * Defines an interface that the application can implement to control the colours of
       * individual points.
       */
      class ColoursController
      {
      public:
         /**
          * Destructor
          */
         virtual ~ColoursController()
         { }

         /**
          * @return Returns the colour that should be used to draw the point at the specified
          * position.
          *
          * @param sender Specifies the trace that is calling this method.
          *
          * @param pos Specifies the position of the point being drawn.
          */
         virtual Colour get_point_colour(GraphTrace *sender, uint4 pos, Colour start_, Colour end_)
         { return sender->get_point_colour(); }

      };
   };
};


#endif
