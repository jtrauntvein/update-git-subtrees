/* MyFrame.cpp

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 12 February 2015
   Last Change: Wednesday 03 June 2015
   Last Commit: $Date: 2015-06-04 14:27:54 -0600 (Thu, 04 Jun 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "MyFrame.h"
#include "Cora.DataSources.Manager.h"
#include "Cora.DataSources.DataFileSource.h"
#include "Csi.Expression.TokenFactory.h"
#include "Csi.Expression.ExpressionHandler.h"
#include "Csi.BuffStream.h"
#include "wxStringStream.h"
#include "wxtools.utils.h"
using namespace Csi::Graphics;


class TestNoTrace: public TestBase
{
public:
   TestNoTrace(StrUni const &name, StrUni const &caption_ = L""):
      caption(caption_),
      TestBase(name)
   { }

   virtual void start_test(graph_type *graph)
   {
      graph->set_title(caption);
   }

   virtual void stop_test(graph_type *graph)
   { }

   virtual void on_config_changed()
   { }

private:
   ////////////////////////////////////////////////////////////
   // caption
   ////////////////////////////////////////////////////////////
   StrUni const caption;
};


class TestTraceNoData: public TestBase
{
public:
   TestTraceNoData(
      StrUni const &name,
      StrUni const &trace_name_,
      StrUni const &caption_ = L"",
      graph_type::legend_pos_type legend_pos_ = Graph::legend_right,
      ShapeBase::symbol_type symbol_ = ShapeBase::symbol_rectangle,
      double symbol_size_ = 1):
      caption(caption_),
      trace_name(trace_name_),
      TestBase(name),
      legend_pos(legend_pos_),
      symbol(symbol_),
      symbol_size(symbol_size_)
   { }

   virtual void start_test(graph_type *graph)
   {
      graph_type::value_type trace(
         new GraphTrace(graph->get_driver()));
      trace->set_title(trace_name).set_point_type(symbol).set_point_size(symbol_size);
      graph->push_back(trace);
      graph->set_title(caption);
      graph->set_legend_pos(legend_pos);
   }

   virtual void stop_test(graph_type *graph)
   {
   }

   virtual void on_config_changed()
   { }
   
private:
   StrUni const caption;
   StrUni const trace_name;
   graph_type::legend_pos_type const legend_pos;
   ShapeBase::symbol_type const symbol;
   double const symbol_size;
};


class TestData: public TestBase, public Cora::DataSources::SinkBase
{
protected:
   StrUni const caption;
   StrUni expression_src;
   typedef Csi::Expression::TokenFactory::expression_handle expression_handle;
   typedef Csi::Expression::TokenFactory::requests_type requests_type;
   typedef Cora::DataSources::Manager manager_type;
   typedef Csi::SharedPtr<manager_type> manager_handle;
   manager_handle manager;
   expression_handle expression;
   requests_type requests;
   typedef graph_type::trace_handle trace_handle;
   trace_handle trace;
   StrUni const trace_caption;
   graph_type *graph;
   
public:
   TestData(
      StrUni const &name,
      StrUni const &caption_,
      StrUni const &expression_src_,
      StrUni const &trace_caption_):
      TestBase(name),
      caption(caption_),
      expression_src(expression_src_),
      trace_caption(trace_caption_)
   { }

   virtual void start_test(graph_type *graph_)
   {
      // we need to configure the data source manager
      using Csi::Xml::Element;
      Element source_props(L"settings");
      graph = graph_;
      source_props.set_attr_wstr(L"CR1000_one_hour.dat", L"file-name");
      source_props.set_attr_wstr(L"1990-01-01", L"poll-base");
      source_props.set_attr_wstr(L"5000", L"poll-int");
      manager_type::value_type source(new Cora::DataSources::DataFileSource(L"file"));
      manager.bind(new manager_type);
      source->set_properties(source_props);
      manager->add_source(source, true);

      // we now need to compile the expression
      expression = Csi::Expression::TokenFactory::make_expression(
         this, expression_src, requests);
      for(auto ri = requests.begin(); ri != requests.end(); ++ri)
         manager->add_request(*ri);
      manager->activate_requests();

      // initialise the trace
      trace.bind(new GraphTrace(graph->get_driver()));
      trace->set_title(trace_caption).set_point_type(ShapeBase::symbol_rectangle);
      graph->set_title(caption);
      graph->get_bottom_axis()->set_labels_rotation(-90);
      graph->push_back(trace);
   }

   virtual void stop_test(graph_type *graph)
   {
      manager.clear();
      expression.clear();
      requests.clear();
      graph = 0;
   }

   virtual void on_config_changed()
   {
      manager->stop_sources();
      if(!graph->empty())
         trace = graph->front();
      manager->start_sources();
   }

   virtual void on_sink_failure(
      manager_type *manager_,
      request_handle &request,
      sink_failure_type failure)
   {
   }

   virtual void on_sink_records(
      manager_type *manager_,
      requests_type &requests,
      records_type const &records)
   {
      using namespace Csi::Expression;
      for(auto rqi = requests.begin(); rqi != requests.end(); ++rqi)
      {
         auto &request(*rqi);
         for(auto ri = records.begin(); ri != records.end(); ++ri)
         {
            try
            {
               auto &record(*ri);
               expression->assign_request_variables(*record, *request);
               auto result(expression->eval());
               GraphTrace::value_handle x(new Operand);
               x->set_val(result->get_timestamp().get_nanoSec() / Csi::LgrDate::nsecPerMSec, 0);
               trace->add_point(x, result->clone());
            }
            catch(std::exception &)
            { }
         }
      }
      graph->Refresh();
   }
};


class TestBar: public TestData
{
public:
   TestBar(
      StrUni const &name,
      StrUni const &caption,
      StrUni const &expression_src,
      StrUni const &trace_caption):
      TestData(name, caption, expression_src, trace_caption)
   { }

   virtual void start_test(graph_type *graph)
   {
      TestData::start_test(graph);
      graph->get_bottom_axis()->set_labels_rotation(0);
      trace->set_trace_type(GraphTrace::trace_bar).set_bar_colour(red_colour());
   }
};


class TestHighlights: public TestData
{
public:
   TestHighlights(
      StrUni const &name,
      StrUni const &caption,
      StrUni const &expression_src,
      StrUni const &trace_caption):
      TestData(name, caption, expression_src, trace_caption)
   { }

   virtual void start_test(graph_type *graph)
   {
      using namespace Csi::Expression;
      TestData::start_test(graph);
      graph->get_left_axis()->set_labels_rotation(0).add_highlight(
         new Operand(75, 0),
         new Operand(100, 0),
         light_gray_colour());
      graph->set_plot_area_background(milk_white_colour());
   }
};


enum control_ids
{
   id_controls_begin = wxID_HIGHEST + 5000,
   id_graph,
   id_choice_tests,
   id_text_graph_config,
   id_pb_apply_config,
   id_controls_end
};


MyFrame::MyFrame():
   wxFrame(0, wxID_ANY, L"Graph Test Harness")
{
   // initialise the tests
   tests = {
      new TestNoTrace("Default Settings with No Traces or Caption"),
      new TestNoTrace("Default Settings With Caption", "Hello World!"),
      new TestTraceNoData("Default Settings with One Trace - right legend", "Trace 0", "Hello World!"),
      new TestTraceNoData("Default Settings with One Trace - bottom legend", "Trace 0", "Hello World!", Graph::legend_bottom),
      new TestTraceNoData("Default Settings with One Trace - left legend", "Trace 0", "Hello World!", Graph::legend_left),
      new TestTraceNoData("Default Settings with One Trace - top legend", "Trace 0", "Hello World!", Graph::legend_top),
      new TestTraceNoData(
         "Default Settings with One Trace - circle symbol", "Trace 0", "Hello World!", Graph::legend_right, ShapeBase::symbol_circle),
      new TestTraceNoData(
         "Default Settings with One Trace - large circle symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_circle,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - triangle symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_up_triangle,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - down triangle symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_down_triangle,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - cross symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_cross,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - diagonal cross symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_diagonal_cross,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - star symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_star,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - diamond symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_diamond,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - small dot symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_small_dot,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - no symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_nothing,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - left triangle symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_left_triangle,
         10),
      new TestTraceNoData(
         "Default Settings with One Trace - right triangle symbol",
         "Trace 0",
         "Hello World!",
         Graph::legend_right,
         ShapeBase::symbol_right_triangle,
         10),
      new TestData(
         "Trace with Data Domain Labels Rotated -90 Degrees",
         "Graph!",
         "StartRelativeToNewest(nsecPerWeek, OrderCollected); file:one_hour.temp_degf_avg",
         "temperature"),
      new TestHighlights(
         "Trace with Left Axis Highlights",
         "Graph!",
         "StartRelativeToNewest(nsecPerWeek, OrderCollected); file:one_hour.temp_degf_avg",
         "temperature"),
      new TestBar(
         "Trace with Rectangular Bar",
         "Bar Graph!",
         "StartRelativeToNewest(nsecPerDay, OrderCollected); file:one_hour.temp_degf_avg",
         "temperature"),
   };
   
   // initialise the controls
   using namespace Csi::Graphics;
   wxSizer *sizer(new wxBoxSizer(wxVERTICAL));
   wxPanel *panel(new wxPanel(this));
   wxSizer *frame_sizer(new wxBoxSizer(wxVERTICAL));
   
   graph = new Csi::Graphics::wxGraph(panel, id_graph);
   sizer->Add(graph, 1, wxEXPAND, 0);
   choice_test = new MyChoice(panel, id_choice_tests);
   for(auto test: tests)
   {
      choice_test->Append(make_wxString(test->name));
      if(current_test == 0)
      {
         current_test = test;
         test->start_test(graph);
         choice_test->SetSelection(0);
      }
   }
   sizer->Add(choice_test, 0, wxALL, 10);
   text_graph_config = new wxTextCtrl(
      panel, id_text_graph_config, L"", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
   sizer->Add(text_graph_config, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
   pb_apply_config = new wxButton(panel, id_pb_apply_config, L"Apply Config");
   sizer->Add(pb_apply_config, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM, 10);
   panel->SetSizerAndFit(sizer);
   frame_sizer->Add(panel, 1, wxEXPAND, 0);
   frame_sizer->SetItemMinSize(panel, 600, 400);
   SetSizerAndFit(frame_sizer);
} // constructor



MyFrame::~MyFrame()
{ }


void MyFrame::on_test_selected(wxCommandEvent &event)
{
   if(current_test != 0)
      current_test->stop_test(graph);
   current_test = tests[choice_test->GetSelection()];
   graph->clear();
   graph->set_default_properties();
   current_test->start_test(graph);
   graph->Refresh();

   // we need to get the configuration for the graph and set the control.
   Csi::Xml::Element config(L"graph");
   OwxStringStream formatted_config;
   graph->write(config);
   config.output(formatted_config, true, 0, false, true);
   text_graph_config->SetValue(formatted_config.str());
}


void MyFrame::on_apply_config_clicked(wxCommandEvent &event)
{
   try
   {
      StrAsc value(make_StrAsc(text_graph_config->GetValue()));
      Csi::IBuffStream input(value.c_str(), value.length());
      Csi::Xml::Element config("graph");
      config.input(input);
      graph->read(config);
      current_test->on_config_changed();
      graph->Refresh();
   }
   catch(std::exception &)
   { }
} // on_apply_config_clicked


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_CHOICE(id_choice_tests, MyFrame::on_test_selected)
EVT_BUTTON(id_pb_apply_config, MyFrame::on_apply_config_clicked)
END_EVENT_TABLE()
