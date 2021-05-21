/* MyFrame.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 12 February 2015
   Last Change: Wednesday 03 June 2015
   Last Commit: $Date: 2015-06-04 14:27:54 -0600 (Thu, 04 Jun 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef MyFrame_h
#define MyFrame_h
#include "wxlib_config.h"
#include "Csi.Graphics.wxGraph.h"
#include "MyChoice.h"


class TestBase
{
public:
   /**
    * Constructor.
    *
    * @param name_ Specifies the name of this test.
    */
   TestBase(StrUni const &name_):
      name(name_)
   { }

   /**
    * virtual destructor
    */
   virtual ~TestBase()
   { }

   /**
    * Responsible for starting the test with the specified graph.
    *
    * @param graph Specifies the graph to configure.
    */
   typedef Csi::Graphics::wxGraph graph_type;
   virtual void start_test(graph_type *graph) = 0;

   /**
    * Called to end any test conditions.
    */
   virtual void stop_test(graph_type *graph) = 0;

   /**
    * Called to handle the event where the graph configuration has changed.
    */
   virtual void on_config_changed() = 0;
   
   /**
    * Specifies the name of the test.
    */
   StrUni const name;
};


class MyFrame: public wxFrame
{
public:
   /**
    * constructor.
    */
   MyFrame();

   /**
    * destructor
    */
   virtual ~MyFrame();

private:
   /**
    * Handles the case where the test selection has changed.
    */
   void on_test_selected(wxCommandEvent &event);

   /**
    * Handles the event where the apply config button was clicked.
    */
   void on_apply_config_clicked(wxCommandEvent &event);
   
private:
   /**
    * Specifies the graph view.
    */
   Csi::Graphics::wxGraph *graph;

   /**
    * Specifies the choice control that will select the test.
    */
   MyChoice *choice_test;

   /**
    * Specifies the current test.
    */
   Csi::LightSharedPtr<TestBase> current_test;

   /**
    * Specifies the available tests
    */
   typedef Csi::LightSharedPtr<TestBase> test_handle;
   typedef std::deque<test_handle> tests_type;
   tests_type tests;

   /**
    * Specifies the text control that shows the graph configuration.
    */
   wxTextCtrl *text_graph_config;
   wxButton *pb_apply_config;
   
private:
   DECLARE_EVENT_TABLE()
};


#endif

