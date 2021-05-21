/* Csi.Graphics.Graph.h

   Copyright (C) 2015, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 22 January 2015
   Last Change: Tuesday 12 April 2016
   Last Commit: $Date: 2018-12-17 18:21:01 -0600 (Mon, 17 Dec 2018) $
   Last Changed by: $Author: jbritt $

*/

#ifndef Csi_Graphics_Graph_h
#define Csi_Graphics_Graph_h

#include "Csi.Graphics.Driver.h"
#include "Csi.Graphics.NestedRect.h"
#include "Csi.Graphics.GraphAxis.h"
#include "Csi.Graphics.GraphTrace.h"
#include "Csi.Graphics.Colour.h"
#include "Csi.Graphics.Gradient.h"


namespace Csi
{
   namespace Graphics
   {
      /**
       * Defines an object that holds the state of the graph.
       */
      class GraphState
      {
      public:
         /**
          * Specifies the state for the bottom axis.
          */
         typedef LightSharedPtr<GraphAxisState> axis_state_handle;
         axis_state_handle bottom_state;

         /**
          * Specifies the state for the left axis.
          */
         axis_state_handle left_state;

         /**
          * Specifies ther state for the right axis.
          */
         axis_state_handle right_state;
      };

      
      /**
       * Defines an object that represents a data graph to the user and allow
       * him to interact with it.
       */
      class Graph
      {
      public:
         /**
          * Construct this component with the specified driver.
          */
         typedef LightSharedPtr<Driver> driver_handle;
         Graph(driver_handle driver_);

         /**
          * Destructor
          */
         virtual ~Graph();

         /**
          * @return Returns the title for this graph.
          */
         StrUni const &get_title() const
         { return title; }

         /**
          * Sets the title for this component.
          */
         Graph &set_title(StrUni const &title_)
         {
            title = title_;
            positions_invalid = true;
            return *this;
         }

         /**
          * @return Returns the font used for displaying the title.
          */
         typedef Driver::font_handle font_handle;
         font_handle &get_title_font()
         { return title_font; }

         /**
          * @param font Specifies the new title font.
          *
          * @param desc Specifies the description for the font.
          */
         Graph &set_title_font(font_handle font)
         {
            title_font = font;
            positions_invalid = true;
            return *this;
         }
         Graph &set_title_font(FontInfo const &desc)
         { return set_title_font(driver->make_font(desc)); }

         /**
          * @return Returns the colour for the font.
          */
         Colour get_title_colour() const
         { return title_colour; }

         /**
          * Sets the colour for the title.
          */
         Graph &set_title_colour(Colour const &colour)
         {
            title_colour = colour;
            return *this;
         }

         /**
          * @return Returns the font used for drawing the legend.
          */
         font_handle &get_legend_font()
         { return legend_font; }

         /**
          * @param font Sets the font used for drawing the legend.
          *
          * @param desc Specifies the description for the font.
          */
         Graph &set_legend_font(font_handle font)
         {
            legend_font = font;
            positions_invalid = true;
            return *this;
         }
         Graph &set_legend_font(FontInfo const &desc)
         {
            return set_legend_font(driver->make_font(desc));
         }

         /**
          * @return Returns the position occupied by the legend.
          */
         enum legend_pos_type
         {
            legend_left = 0,
            legend_right = 1,
            legend_top = 2,
            legend_bottom = 3,
            legend_not_shown
         };
         legend_pos_type get_legend_pos() const
         { return legend_pos; }

         /**
          * @param pos Sets the position for the legend.
          */
         Graph &set_legend_pos(legend_pos_type pos)
         {
            legend_pos = pos;
            positions_invalid = true;
            return *this;
         }

         /**
          * @return Returns the brush used to fille the legend background.
          */
         brush_handle get_legend_brush() const
         { return legend_brush; }

         /**
          * @param brush Specifies the brush used to draw the legend background.
          */
         Graph &set_legend_background(brush_handle brush)
         {
            legend_brush = brush;
            return *this;
         }

         /**
          * @param desc Specifies the description of the brush.
          */
         Graph &set_legend_background(BrushInfo const &desc)
         { return set_legend_background(driver->make_brush(desc)); }

         /**
          * @param Specifies the solid colour that should be used to draw the legend background.
          */
         Graph &set_legend_background(Colour const &colour)
         {
            legend_brush = driver->make_brush(BrushInfo().set_colour(colour));
            return *this;
         }

         /**
          * @return Returns the colour of text in the legend.
          */
         Colour const &get_legend_text_colour() const
         { return legend_text_colour; }

         /**
          * @param colour Specifies the colour of text for the legend.
          */
         Graph &set_legend_text_colour(Colour const &colour)
         {
            legend_text_colour = colour;
            return *this;
         }

         /**
          * @return Returns the pen used to draw the legend border.
          */
         typedef Driver::pen_handle pen_handle;
         pen_handle get_legend_pen() const
         { return legend_pen; }

         /**
          * @param pen Specifies the pen used to draw the legend border.
          */
         Graph &set_legend_pen(pen_handle pen)
         {
            legend_pen = pen;
            positions_invalid = true;
            return *this;
         }

         /**
          * @return Returns the display width for a time domain graph.
          */
         int8 get_display_width() const
         { return display_width; }

         /**
          * @param width Specifies the width of the display, in milliseconds, for a time domain
          * graph.
          */
         Graph &set_display_width(int8 width)
         {
            display_width = width;
            positions_invalid = true;
            return *this;
         }

         /**
          * @return Returns the newest time for this graph as milliseconds since 1 January 1990.
          */
         int8 get_newest_time() const
         { return newest_time; }

         /**
          * @param value Specifies the newest time for this graph as milliseconds since 1 Jan
          * 1990.
          */
         Graph &set_newest_time(int8 value)
         {
            newest_time = value;
            positions_invalid = true;
            return *this;
         }
         Graph &set_newest_time(LgrDate const &value)
         {
            newest_time = value.get_nanoSec() / LgrDate::nsecPerMSec;
            positions_invalid = true;
            return *this;
         }

         /**
          * @return Returns the brush used to draw the background for this graph.
          */
         brush_handle get_background_brush() const
         { return background_brush; }

         /**
          * @param brush Sets the brush used to draw the background for this graph,
          */
         Graph &set_background_brush(brush_handle brush)
         {
            background_brush = brush;
            return *this;
         }
         Graph &set_background_brush(BrushInfo const &desc)
         { return set_background_brush(driver->make_brush(desc)); }

         /**
          * @param gradient Sets the background brush to use the gradient specified.
          */
         Graph &set_background(Gradient const &gradient)
         {
            background_brush = driver->make_brush(gradient);
            return *this;
         }

         /**
          * @return Returns the pen used to draw the border of this component.
          */
         pen_handle get_background_pen() const
         { return background_pen; }

         /**
          * @param pen Specifies the pen used to draw the border around this component.
          */
         Graph &set_background_pen(pen_handle pen)
         {
            background_pen = pen;
            return *this;
         }
         Graph &set_background_pen(PenInfo const &desc)
         { return set_background_pen(driver->make_pen(desc)); }

         /**
          * @return Returns the brush used for the plot area.
          */
         brush_handle get_plot_area_brush() const
         { return plot_area_brush; }

         /**
          * @param brush Sets the brush used for the plot area.
          */
         Graph &set_plot_area_brush(brush_handle brush)
         {
            plot_area_brush = brush;
            return *this;
         }
         Graph &set_plot_area_brush(BrushInfo const &desc)
         { return set_plot_area_brush(driver->make_brush(desc)); }

         /**
          * @param colour Specifies the solid colour for the plot area brush.
          */
         Graph &set_plot_area_background(Colour const &colour)
         {
            plot_area_brush = driver->make_brush(colour);
            return *this;
         }

         /**
          * @return Returns the pen used to draw a border around the plot area.
          */
         pen_handle get_plot_area_pen() const
         { return plot_area_pen; }

         /**
          * @param pen Specifies the pen used to draw a border around the plot area.  If set to
          * null, no border will be drawn.
          *
          * @param desc Specifies the description for the pen.
          */
         Graph &set_plot_area_pen(pen_handle pen)
         {
            plot_area_pen = pen;
            return *this;
         }
         Graph &set_plot_area_pen(PenInfo const &desc)
         { return set_plot_area_pen(driver->make_pen(desc)); }

         /**
          * @return Returns the graph rotation property.
          */
         enum rotation_type
         {
            rotation_right = -90,
            rotation_none = 0,
            rotation_left = 90
         };
         rotation_type get_rotation() const
         { return rotation; }

         /**
          * @param value Specifies how the axes will appear on this graph.  A value of rotation_none
          * will set up the axes so that the domain is on the bottom and the range axes are on the
          * left and right sides.  A value of rotation_right will set up the axis positions so that
          * the bottom axis is on the right and the range axes are on the botom and top.  A value
          * of rotation_left will set up the axes so that the domain axis is on the left and the
          * range axes are on the top and bottom.
          */
         Graph &set_rotation(rotation_type value);

         /**
          * @return Returns the layout rectangle for this component.
          */
         typedef LightSharedPtr<NestedRect> rect_handle;
         rect_handle &get_layout()
         { return layout; }

         /**
          * @param rect Sets the bounds of the layout rectangle for this graph.
          */
         void set_rect(Rect const &rect)
         {
            positions_invalid = true;
            window_rect = rect;
            layout.bind(new NestedRect(rect));
         }

         /**
          * Writes the configuration of the graph and its components to the specified XML structure.
          *
          * @param elem Specifies the destination XML structure.
          *
          * @param write_traces Set to true if the traces should be written to the XML element.
          */
         virtual void write(Xml::Element &elem, bool write_traces = true);

         /**
          * @return Returns a newly alocated trace object.  An derived class
          * can overload this method to create application specific trace
          * types.
          *
          * @param driver Specifies the driver used by this graph.
          */
         typedef LightSharedPtr<GraphTrace> trace_handle;
         trace_handle make_trace(driver_handle &driver)
         { return new GraphTrace(driver); }

         /**
          * Reads the configuration of this graph and its components from the specified XML
          * structure.
          *
          * @param elem Specifies the source XML structure.
          *
          * @param read_traces Set to true if the traces should be read from the element.
          */
         virtual void read(Xml::Element &elem, bool read_traces = true);
         
         /**
          * @return Returns the rectangle for the plot area for this graph.
          */
         rect_handle &get_plot_rect()
         { return plot_rect; }

         // @group: methods and declarations for acting as a container of traces.

         /**
          * @return Returns the begin iterator for the collection of traces.
          */
         typedef std::deque<trace_handle> traces_type;
         typedef traces_type::iterator iterator;
         typedef traces_type::const_iterator const_iterator;
         iterator begin()
         { return traces.begin(); }
         const_iterator begin() const
         { return traces.begin(); }

         /**
          * @return Returns the end iterator for the collection of traces.
          */
         iterator end()
         { return traces.end(); }
         const_iterator end() const
         { return traces.end(); }

         /**
          * @return Returns the reference to the first trace.
          */
         typedef trace_handle value_type;
         trace_handle &front()
         { return traces.front(); }
         trace_handle const &front() const
         { return traces.front(); }
         
         /**
          * @return Returns the reference to the last trace.
          */
         trace_handle &back()
         { return traces.back(); }
         trace_handle const &back() const
         { return traces.back(); }

         /**
          * @return Returns the number of traces associated with this graph.
          */
         typedef traces_type::size_type size_type;
         size_type size() const
         { return traces.size(); }

         /**
          * @return Returns true if there are no traces.
          */
         bool empty() const
         { return traces.empty(); }

         /**
          * Adds the specified trace to this graph.
          *
          * @param trace Specifies the trace to be added.
          */
         void push_back(trace_handle trace);

         /**
         * Change the order of the traces
         *
         * @param from_index Specifies the source trace location
         * @param to_index  Specifies the destination trace location
         */
         void move_trace(size_t from_index, size_t to_index);


         /**
          * Changes the registration for the specified trace from one vertical axis to the other.
          *
          * @param trace Specifies the trace that has been changed.
          */
         void on_trace_axis_changed(GraphTrace *trace);

         /**
          * Removes the trace at the specified position.
          *
          * @param it Specifies the trace to remove.
          */
         void erase(iterator ti)
         { erase(ti, ti + 1); }

         /**
          * Removes the sequence of traces specified by the iterators.
          *
          * @param begin Specifies the start of the sequence to remove.
          *
          * @param end Specifies the end of the sequence to remove.
          */
         void erase(iterator begin, iterator end);

         /**
          * Erases the specified trace from the graph.
          *
          * @param trace Specifies the trace reference to remove.
          */
         void erase(trace_handle &trace)
         {
            iterator ti(std::find(begin(), end(), trace));
            if(ti != end())
               erase(ti);
         }
         void erase(GraphTrace *trace)
         {
            iterator ti(std::find_if(begin(), end(), HasLightSharedPtr<GraphTrace>(trace)));
            if(ti != end())
               erase(ti);
         }
         
         /**
          * Removes all traces.
          */
         void clear();

         // @endgroup:

         /**
          * @return Returns the driver associated with this graph.
          */
         driver_handle &get_driver()
         { return driver; }

         /**
          * @return Returns the number of traces that are owned by this graph
          * and that are configured as bars.
          */
         double get_bar_count() const
         { return bar_count; }

         /**
         * @return Returns the width of a bar in a bar graph, in pixels
         */
         double get_bar_width() const
         { return bar_width; }

         void calculate_bar_width();

         /**
         * @return Returns the ratio of the space that the bar will take
         * up in the total amount of space available for each bar.
         */
         double get_bar_width_ratio() const
         { return bar_width_ratio; }

         /**
         * @param value Specifies the ratio of the horizontal space taken
         * by the bar in the total amount of space available for each bar.
         */
         void set_bar_width_ratio(double value)
         {
            bar_width_ratio = value;
            positions_invalid = true;
         }

         /**
         * @return Returns the how to draw multiple bars.
         */
         enum bar_overlap_style_type
         {
            bars_overlap = 0,
            bars_side_by_side = 1
         };
         bar_overlap_style_type get_bar_overlap_style() const
         { return bar_overlap_style; }

         /**
         * @param style sets how to draw multiple bars
         */
         Graph &set_bar_overlap_style(bar_overlap_style_type style)
         {
            bar_overlap_style = style;
            positions_invalid = true;
            return *this;
         }



         /**
          * Sets all graph and axis properties to their default values.
          */
         virtual void set_default_properties(bool set_axis_props = true);

         /**
          * @return Returns the reference to the bottom axis.
          */
         typedef LightSharedPtr<GraphAxis> axis_handle;
         axis_handle &get_bottom_axis()
         { return bottom_axis; }

         /**
          * @return Returns the left axis reference.
          */
         axis_handle &get_left_axis()
         { return left_axis; }

         /**
          * @return Returns the reference to the right axis.
          */
         axis_handle &get_right_axis()
         { return right_axis; }

         /**
          * @return Returns true if the graph positions are invalid.
          */
         bool get_positions_invalid() const
         { return positions_invalid; }

         /**
          * Sets the positions_invalid flag.
          */
         Graph &set_positions_invalid()
         {
            positions_invalid = true;
            return *this;
         }

      protected:
         /**
          * Calculates the positions of the axes as well as the labels.
          */
         virtual void calculate_positions();

         /**
         * Force a redraw of the graph
         */
         virtual void force_redraw() {}

         /**
          * Executes the draw code for this component.
          */
         virtual void draw()
         {
            if(positions_invalid)
            {
               calculate_positions();
               positions_invalid = false;
            }
            draw_background();
            draw_title();
            draw_legend();
            draw_plot_area();
            bottom_axis->draw_highlights();
            left_axis->draw_highlights();
            right_axis->draw_highlights();
         }

         /**
          * Responsible for drawing the background area for this component.
          */
         virtual void draw_background();

         /**
          * Responsible for drawing the title for this graph.
          */
         virtual void draw_title();

         /**
          * Responsible for drawing the background for the legend.
          */
         virtual void draw_legend_background();
         
         /**
          * Responsible for drawing the legend for this graph.
          */
         virtual void draw_legend();

         /**
          * Responsible for drawing the plot area background.
          */
         virtual void draw_plot_area_background();
         
         /**
          * Responsible for drawing the plot area for this graph.
          */
         virtual void draw_plot_area();

         /**
          * Draws any decorations (current mark, zoom rectangle, and undo zoom button).
          */
         virtual void draw_decorations();

         /**
          * Called to handle a left mouse button down event.
          *
          * @param pos Specifies the mouse position when the button was pressed.
          *
          * @return Returns true if the mouse event was handled or false if it
          * should be passed on to the system.
          */
         virtual bool on_left_mouse_down(Point const &pos);

         /**
          * Called to handle a left mouse button up event.
          *
          * @param pos Specifies the position of the mouse when the button was
          * released.
          *
          * @param Returns true if the mouse event was handled or false of it
          * should be passed to the system.
          */
         virtual bool on_left_mouse_up(Point const &pos);

         /**
          * Called to handle the right mouse button down event.
          *
          * @param pos Specifies the position of the mouse when the right
          * button was pressed.
          *
          * @return Returns true if the event was handled or false if it should
          * be passed to the system.
          */
         virtual bool on_right_mouse_down(Point const &pos);

         /**
          * Called to handle the right mouse button release event.
          *
          * @param pos Specifies the position of the mouse when the right
          * button was released.
          *
          * @return Returns true if the event was handled or false if it should
          * be passed to the system.
          */
         virtual bool on_right_mouse_up(Point const &pos);

         /**
          * Called to handle the mouse move event.
          *
          * @param pos Specifies the current position of the mouse.
          *
          * @param return Returns true of the event was handled or false if it
          * should be passed to the system.
          */
         virtual bool on_mouse_move(Point const &pos);

         /**
          * Called to handle the mouse wheel event.
          *
          * @param pos Specifies the position of the mouse.
          *
          * @param rotation Specifies the amount of movement of the wheel and
          * the direction of the wheel movement.
          *
          * @param wheel_delta Specifies the threshold for action to be taken.
          *
          * @return Returns true if the event was handled or false of it should
          * be passed to the system.
          */
         virtual bool on_mouse_wheel(Point const &pos, double rotation, double wheel_delta);

         /**
          * Called to start capture of the mouse.
          */
         virtual void start_mouse_capture() = 0;

         /**
          * Called to release capture of the mouse.
          */
         virtual void release_mouse_capture() = 0;

         /**
          * Called to force the graph view to repaint.
          */
         virtual void do_refresh() = 0;

         /**
          * Backs up the current settings for the axes.
          */
         virtual void backup_settings();

         /**
          * Restores the settings for the axes from the current backup.
          */
         virtual void restore_settings();

      protected:
         /**
          * Set to true if the graph positions are invalid and need to be calculated before the next
          * draw.
          */
         bool positions_invalid;

         /**
          * Specifies whether the graph axes will be rotated.
          */
         rotation_type rotation;
         
         /**
          * Specifies the title for this graph.
          */
         StrUni title;

         /**
          * Specifies the font that will be used to formatting the title.
          */
         font_handle title_font;

         /**
          * Specifies the colour for the title.
          */
         Colour title_colour;

         /**
          * Specifies the font that will be used for the legend.
          */
         Driver::font_handle legend_font;

         /**
          * Specifies the position of the legend.
          */
         legend_pos_type legend_pos;

         /**
          * Specifies the brush used to draw the legend background.
          */
         brush_handle legend_brush;

         /**
          * Specifies the colour of text in the legend.
          */
         Colour legend_text_colour;

         /**
          * Specifies the pen used for drawing the legend rectangle border.
          */
         pen_handle legend_pen;

         /**
          * Specifies the layout for this component.
          */
         rect_handle layout;
         Rect window_rect;

         /**
          * Specifies the layout for the title.
          */
         rect_handle title_rect;

         /**
          * Specifies the layout for the plot area.
          */
         rect_handle plot_rect;

         /**
          * Specifies the layout for the legend.
          */
         rect_handle legend_rect;
         
         /**
          * Specifies the bottom axis
          */
         axis_handle bottom_axis;

         /**
          * Specifies the left axis.
          */
         axis_handle left_axis;

         /**
          * Specifies the right axis.
          */
         axis_handle right_axis;

         /**
         * Specifies the width of bar for bar graphs
         */
         double bar_width;

         /**
         * Specifies how many bars are visible in the current graph
         */
         double bar_count;


         /**
         * Specifies how to draw multiple bars
         */
         bar_overlap_style_type bar_overlap_style;

         /**
          * Specifies the driver that will be used for painting this graph.
          */
         LightSharedPtr<Driver> driver;

         /**
          * Specifies the set of traces associated with this graph
          */
         traces_type traces;

         /**
          * Specifies the display width for a time domain graph.
          */
         int8 display_width;

         /**
          * Specifies the newest time stamp for the domain as the number of milliseconds since 1990.
          */
         int8 newest_time;

         /**
         * Specifies the width of the bar as a ratio of the space
         * availabel for each bar.
         */
         double bar_width_ratio;

         /**
         * Specifies the maximum bar width.
         */
         double max_bar_width;

         /**
         * @return Returns the maximum width for the bar.
         */
         double get_max_bar_width() const
         { return max_bar_width; }

         /**
         * Sets the maximum width for the bar.
         *
         * @param value Specifies the maximum width for bars associated with this trace. 
         */
         void set_max_bar_width(double value)
         {
            max_bar_width = value;
         }

         /**
          * Specifies the brush used to draw the graph background
          */
         brush_handle background_brush;

         /**
          * Specifies the pen that will be used to draw the border of the background rectangle.
          */
         pen_handle background_pen;

         /**
          * Specifies the brush used to draw the plot area.
          */
         brush_handle plot_area_brush;

         /**
          * Specifies the pen that will be used to draw the border of the plot area.
          */
         pen_handle plot_area_pen;

         /**
          * Specifies the current mouse state.
          */
         enum mouse_state_type
         {
            mouse_idle,
            mouse_left_down,
            mouse_zoom_rect,
            mouse_right_down,
            mouse_restore_down
         } mouse_state;

         /**
          * Specifies the mouse position when leaving a mouse idle state.
          */
         Point mouse_start_pos;

         /**
          * Specifies the current mouse posiiton.
          */
         Point mouse_current_pos;

         /**
          * Specifies the state of the axes before any zoom or pan operations were applied.
          */
         LightSharedPtr<GraphState> state_backup;

         /**
          * Specifies the rectangle associated with the restore button.
          */
         Rect restore_button_rect;

         /**
          * Specifies the font used for the restore button.
          */
         font_handle restore_font;

         /**
          * Specifies the pen used to draw the border around the restore button.
          */
         pen_handle restore_pen;

         /**
          * Specifies the brush used for the restore button.
          */
         brush_handle restore_brush;
         
         friend class GraphAxis;
         friend class GraphTrace;
      };
   };
};


#endif
