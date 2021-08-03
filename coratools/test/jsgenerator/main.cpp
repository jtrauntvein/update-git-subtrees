/* main.cpp

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 29 July 2010
   Last Change: Friday 01 April 2011
   Last Commit: $Date: 2011-04-01 13:23:40 -0600 (Fri, 01 Apr 2011) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Expression.JsGenerator.h"
#include "Csi.Expression.JsResourceManager.h"
#include "Cora.DataSources.LgrNetSource.h"
#include "Cora.DataSources.DataFileSource.h"
#include "Cora.DataSources.CsiDbSource.h"
#include "Cora.DataSources.HttpSource.h"
#include "Csi.Xml.Element.h"
#include "Csi.Win32Dispatch.h"
#include "Csi.Win32.WinsockInitialisor.h"
#include "Csi.FileSystemObject.h"
#include <iostream>
#include <fstream>


namespace
{
   char const index_contents[] =
      "<html>\n"
      "<head>\n"
      "  <title>Expressions test page</title>\n"
      "  <script src=\"/jquery.js\" type=\"text/javascript\"></script>\n"
      "  <script src=\"./RTMCProject.js\" type=\"text/javascript\"></script>\n"
      "</head\n"
      "<body>\n"
      "  <h1>Expressions Test Page</h1>\n"
      "  <h2>Expression 0</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 1</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 2</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 3</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 4</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 5</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 6</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 7</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 8</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 9</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 10</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 11</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 12</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 13</h2>\n"
      "  <table border='1'></table>\n"
      "  <h2>Expression 14</h2>\n"
      "  <table border='1'></table>\n"
      "</html>\n";
};


int main(int argc, char const *argv[])
{
   int rtn = 0;
   try
   {
      // we need to perform some coratools initialisation
      Csi::MessageWindow::initialise(::GetModuleHandle(0));
      Csi::Event::set_dispatcher(new Csi::Win32Dispatch);
      Csi::Win32::WinsockInitialisor sockets_init;
      
      // check the arguments
      if(argc < 3)
         throw std::invalid_argument("configuration file needed");

      // open the input file
      using namespace Csi::Xml;
      using namespace Cora::DataSources;
      std::ifstream input_file(argv[1]);
      if(!input_file)
         throw Csi::OsException("Failed to open the input file:");
      Element config(L"config");
      config.input(input_file);

      // we need to locate the sources section of the input so we can instantiate all of the data sources
      Element::value_type sources(config.find_elem(L"sources"));
      Csi::SharedPtr<Manager> manager(new Manager);
      for(Element::iterator si = sources->begin(); si != sources->end(); ++si)
      {
         Element::value_type &source = *si;
         if(source->get_name() == L"source")
         {
            Element::value_type &settings(source->find_elem(L"settings"));
            StrAsc source_name(source->get_attr_str(L"name"));
            StrAsc source_type(source->get_attr_str(L"type"));
            if(source_type == "loggernet")
            {
               Manager::value_type source(new LgrNetSource(source_name));
               source->set_properties(*settings);
               manager->add_source(source);
            }
            else if(source_type == "file" || source_type == "data-file")
            {
               Manager::value_type source(new DataFileSource(source_name));
               source->set_properties(*settings);
               manager->add_source(source);
            }
            else if(source_type == "database" || source_type == "csidb")
            {
               Manager::value_type source(new DataFileSource(source_name));
               source->set_properties(*settings);
               manager->add_source(source);
            }
            else if(source_type == "http")
            {
               Manager::value_type source(new HttpSource(source_name));
               source->set_properties(*settings);
               manager->add_source(source);
            }
            else
               throw std::invalid_argument("invalid source type");
         }
      }

      // we can now look for the expressions element
      Element::value_type expressions(config.find_elem(L"expressions"));
      Csi::Expression::TokenFactory factory;
      Csi::SharedPtr<Csi::ResourceManager> resources(new Csi::Expression::JsResourceManager);
      Csi::Expression::JsGenerator generator(manager, resources);
      typedef Csi::Expression::ExpressionHandler expression_type;
      typedef Csi::SharedPtr<expression_type> expression_handle;
      typedef std::list<expression_handle> expressions_type;
      expressions_type compiled_expressions;
      StrAsc contents(index_contents);
      
      for(Element::iterator ei = expressions->begin(); ei != expressions->end(); ++ei)
      {
         Element::value_type &expression_xml(*ei);
         Csi::Expression::TokenFactory::requests_type requests;
         expression_handle expression(
            Csi::Expression::TokenFactory::make_expression(
               0, expression_xml->get_cdata_str(), requests, "", &factory));
         Csi::OStrAscStream expr_name;
         Csi::OStrAscStream table_heading;
         
         generator.add_expression(expression);
         compiled_expressions.push_back(expression);
         expr_name << "<h2>Expression " << expression->get_js_index() << "</h2>";
         table_heading << "<pre>\n" << expression_xml->get_cdata_str() << "\n</pre>\n";
         contents.replace(expr_name.str().c_str(), table_heading.str().c_str());
      }

      // we will generate the output files in the directory specified in the second parameter.  We
      // will first generate .source.xml
      StrAsc dest_dir(argv[2]);
      Csi::OStrAscStream temp;
      std::ofstream sources_file;
      
      Csi::createNestedDir(argv[2]);
      if(dest_dir.last() != Csi::FileSystemObject::dir_separator())
         dest_dir.append(Csi::FileSystemObject::dir_separator());
      temp << dest_dir << ".sources.xml";
      sources_file.open(temp.str().c_str(), std::ios::binary);
      if(!sources_file)
         throw Csi::OsException("Failed to open the sources file");
      sources->output(sources_file, true);
      sources_file.close();

      // now generate the RTMCProject.js file
      std::ofstream project_file;
      temp.str("");
      temp << dest_dir << "RTMCProject.js";
      project_file.open(temp.str().c_str(), std::ios::binary);
      if(!project_file)
         throw Csi::OsException("Failed to open the project file");
      generator.generate_dependencies(project_file);
      generator.generate_other_dependencies(project_file, "TestComponent.js");
      project_file << "function start()\n{\n";
      generator.generate(project_file);
      while(!compiled_expressions.empty())
      {
         expression_handle expression(compiled_expressions.front());
         compiled_expressions.pop_front();
         project_file << "  var test" << expression->get_js_index()
                      << " = new TestComponent(expressions["
                      << expression->get_js_index() << "], "
                      << expression->get_js_index() << ");\n";
      }
      project_file << "\n  dataManager.start();\n";
      project_file << "}\n\n$(document).ready(start);\n\n";
      project_file.close();

      // Finally, we will generate the test page
      std::ofstream page_file;
      temp.str("");
      temp << dest_dir << "index.html";
      page_file.open(temp.str().c_str(), std::ios::binary);
      page_file.write(contents.c_str(), contents.length());
   }
   catch(std::exception &e)
   {
      std::cout << "Test error: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main

