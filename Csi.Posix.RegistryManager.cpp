/* Csi.Posix.RegistryManager.cpp

   Copyright (C) 2005, 2017 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Friday 20 May 2005
   Last Change: Friday 28 July 2017
   Last Commit: $Date: 2017-07-28 16:16:14 -0600 (Fri, 28 Jul 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.RegistryManager.h"
#include "Csi.FileSystemObject.h"
#include "Csi.Xml.Element.h"
#include "trace.h"
#include <sstream>
#include <fstream>


namespace Csi
{
   namespace Posix
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // decode_address
         //
         // Given the root document element and a key string, this function will attempt to locate
         // the element in the document associated with the address.  If the addressed element does
         // not exist, that element (and its parents if need be) will be created and the e_created
         // parameter (if set to non-null) will be set to true.
         ////////////////////////////////////////////////////////////
         SharedPtr<Xml::Element> decode_address(
            SharedPtr<Xml::Element> &root,
            char const *path,
            bool create_if_needed)
         {
            using namespace Xml;
            SharedPtr<Element> parent = root;
            StrAsc key_name;
            SharedPtr<Element> rtn;
            
            for(int i = 0; path[i] != 0; ++i)
            {
               if(path[i] == '\\' || path[i + 1] == 0)
               {
                  if(path[i + 1] == 0)
                     key_name += path[i];
                  rtn.clear();
                  for(Element::iterator ei = parent->begin(); ei != parent->end(); ++ei)
                  {
                     SharedPtr<Element> &child = *ei;
                     if(child->get_name() == L"key" && child->get_attr_str(L"name") == key_name)
                     {
                        rtn = child;
                        break;
                     }
                  }
                  if(rtn != 0)
                     parent = rtn;
                  else if(create_if_needed)
                  {
                     rtn = parent->add_element(L"key");
                     rtn->set_attr_str(key_name,L"name");
                     parent = rtn;
                  }
                  else
                     break;
                  key_name.cut(0);
               }
               else
                  key_name.append(path[i]);
            }
            return rtn;
         } // decode_address


         ////////////////////////////////////////////////////////////
         // class FileManager
         //
         // Defines an object that attempts to reflect the content of an XML file.
         ////////////////////////////////////////////////////////////
         class FileManager
         {
         private:
            ////////////////////////////////////////////////////////////
            // doc
            ////////////////////////////////////////////////////////////
            SharedPtr<Xml::Element> root;

            ////////////////////////////////////////////////////////////
            // last_mod_date
            ////////////////////////////////////////////////////////////
            LgrDate last_mod_date;

            ////////////////////////////////////////////////////////////
            // file_name
            ////////////////////////////////////////////////////////////
            StrAsc file_name;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            FileManager(bool is_local)
            {
               if(is_local)
               {
                  file_name = getenv("HOME");
                  if(file_name.last() != '/')
                     file_name.append('/');
                  file_name.append(".csi_registry");
               }
               else
                  file_name = "/etc/opt/CampbellSci/csi_registry.xml";
            } // constructor

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            ~FileManager()
            { }

            ////////////////////////////////////////////////////////////
            // get_doc
            //
            // Responsible for verifying that the document content is still valid.  Will do this by
            // checking the stored date of modification with the one reported by the OS.  If they
            // don't match, the document will be (re)loaded before it is returned.
            ////////////////////////////////////////////////////////////
            SharedPtr<Xml::Element> &get_doc()
            {
               // verify that our content is up to date
               bool should_load = true;
               FileSystemObject doc_info(file_name.c_str());
               if(root != 0)
               {
                  if(doc_info.get_last_write_date() == last_mod_date)
                     should_load = false;
               }
               if(should_load)
               {
                  try
                  {
                     std::ifstream in(file_name.c_str());
                     root.bind(new Xml::Element(L"registry"));
                     root->input(in);
                     last_mod_date = doc_info.get_last_write_date();
                  }
                  catch(...)
                  { root.bind(new Xml::Element(L"registry")); }
               } 
               return root;
            }

            ////////////////////////////////////////////////////////////
            // save
            ////////////////////////////////////////////////////////////
            void save()
            {
               if(root != 0)
               {
                  std::ofstream out(file_name.c_str());
                  if(out)
                  {
                     root->output(out,true);
                     FileSystemObject doc_info(file_name.c_str());
                     last_mod_date = doc_info.get_last_write_date();
                  }
               }
            }
         };


         ////////////////////////////////////////////////////////////
         // user_hive
         //
         // Maintains the local registry
         ////////////////////////////////////////////////////////////
         SharedPtr<FileManager> user_hive;

         
         ////////////////////////////////////////////////////////////
         // machine_hive
         //
         // Maintains the machine registry
         ////////////////////////////////////////////////////////////
         SharedPtr<FileManager> machine_hive;

         
         ////////////////////////////////////////////////////////////
         // get_hive
         ////////////////////////////////////////////////////////////
         SharedPtr<FileManager> &get_hive(HKEY hive_id)
         {
            if(hive_id == HKEY_CURRENT_USER)
            {
               if(user_hive == 0)
                  user_hive.bind(new FileManager(true));
               return user_hive;
            }
            else
            {
               if(machine_hive == 0)
                  machine_hive.bind(new FileManager(false));
               return machine_hive;
            }
            return user_hive;
         }


         ////////////////////////////////////////////////////////////
         // reg_key_prefix
         ////////////////////////////////////////////////////////////
         char const reg_key_prefix[] = "SOFTWARE\\Campbell Scientific";
         

         ////////////////////////////////////////////////////////////
         // try_open_element
         ////////////////////////////////////////////////////////////
         SharedPtr<Xml::Element> try_open_element(
            HKEY hive_id,
            StrAsc const &app_name,
            StrAsc const &version,
            StrAsc const &server_name)
         {
            // we need ot get the document to search
            SharedPtr<FileManager> &hive = get_hive(hive_id);
            SharedPtr<Xml::Element> &doc = hive->get_doc();
            
            // now we create the list of potential keys to try
            std::list<StrAsc> keys_to_try;
            std::ostringstream temp;

            if(version.length() != 0)
            {
               temp << reg_key_prefix << "\\" << app_name << "\\" << version;
               if(server_name.length() != 0 && server_name != "localhost")
                  temp << "\\" << server_name;
               keys_to_try.push_back(temp.str().c_str());
               temp.str("");
            }
            temp << reg_key_prefix << "\\" << app_name;
            if(server_name.length() > 0 && server_name != "localhost")
               temp << "\\" << server_name;
            keys_to_try.push_back(temp.str().c_str());

            // we will try each of these keys until we find one that opens (we will force the last
            // key to be created).
            SharedPtr<Xml::Element> rtn;
            while(rtn == 0 && !keys_to_try.empty())
            {
               rtn = decode_address(
                  doc,
                  keys_to_try.front().c_str(),
                  keys_to_try.size() == 1);
               keys_to_try.pop_front();
            }
            return rtn;
         }
      };

      
      ////////////////////////////////////////////////////////////
      // class RegistryManager definitions
      ////////////////////////////////////////////////////////////
      RegistryManager::RegistryManager(
         StrAsc const &app_name_,
         StrAsc const &version_):
         app_name(app_name_),
         version(version_)
      { }

      
      RegistryManager::~RegistryManager()
      { }

      
      int RegistryManager::read_int(
         int &val,
         char const *value_name,
         HKEY key)
      {
         StrAsc temp;
         read_string(temp,value_name,key);
         if(temp.length() > 0)
            val = atoi(temp.c_str());
         return val;
      } // read_int

      
      uint4 RegistryManager::read_uint4(
         uint4 &val,
         char const *value_name,
         HKEY key)
      {
         StrAsc temp;
         read_string(temp,value_name,key);
         if(temp.length() > 0)
            val = strtoul(temp.c_str(),0,10);
         return val;
      } // read_uint4

      
      void RegistryManager::write_int(
         int val,
         char const *value_name,
         HKEY key)
      {
         std::ostringstream temp;
         temp.imbue(std::locale::classic());
         temp << val;
         write_string(temp.str().c_str(),value_name,key);
      } // write_int

      
      void RegistryManager::write_uint4(
         uint4 val,
         char const *value_name,
         HKEY key)
      {
         std::ostringstream temp;
         temp.imbue(std::locale::classic());
         temp << val;
         write_string(temp.str().c_str(),value_name,key);
      } // write_uint4

      
      bool RegistryManager::read_bool(
         bool &val,
         char const *value_name,
         HKEY key)
      {
         StrAsc temp;
         read_string(temp,value_name,key);
         if(temp.length() > 0)
         {
            if(temp == "true" || temp == "t" || temp == "1")
               val = true;
            else
               val = false;
         }
         return val;
      } // read_bool

      
      void RegistryManager::write_bool(
         bool &val,
         char const *value_name,
         HKEY key)
      {
         write_string(val ? "1" : "0",value_name,key);
      } // write_bool

      
      char const *RegistryManager::read_string(
         StrAsc &val,
         char const *value_name,
         HKEY key)
      {
         SharedPtr<Xml::Element> e = try_open_element(key,app_name,version,server_name);
         if(e != 0)
         {
            // search for the named value
            val.cut(0);
            for(Xml::Element::iterator ei = e->begin();
                ei != e->end();
                ++ei)
            {
               SharedPtr<Xml::Element> &child = *ei;
               if(child->get_name() == L"value" && child->get_attr_str(L"name") == value_name)
               {
                  child->get_cdata_wstr().toMulti(val);
                  break;
               }
            }
         }
         else
            val.cut(0);
         return val.c_str();
      } // read_string

      
      void RegistryManager::write_string(
         StrAsc const &val,
         char const *value_name,
         HKEY key)
      {
         SharedPtr<Xml::Element> e = try_open_element(key,app_name,version,server_name);
         if(e != 0)
         {
            // search for the named value
            SharedPtr<Xml::Element> choice;
            for(Xml::Element::iterator ei = e->begin();
                choice == 0 && ei != e->end();
                ++ei)
            {
               SharedPtr<Xml::Element> &child = *ei;
               if(child->get_name() == L"value" && child->get_attr_str(L"name") == value_name)
                  choice = child;
            }
            if(choice == 0)
            {
               choice = e->add_element(L"value");
               choice->set_attr_str(value_name,L"name");
            }
            choice->set_cdata_str(val);
            get_hive(key)->save();
         }
      } // write_string

      
      char const *RegistryManager::read_work_dir(StrAsc &buff)
      {
         read_string(buff,"WorkDir",HKEY_LOCAL_MACHINE);
         if(buff.length() == 0)
         {
            buff = "/var/opt/CampbellSci/" + app_name + "/";
            if(version.length())
               buff += version + "/";
         }
         else if(buff.last() != '/')
            buff.append('/'); 
         return buff.c_str();
      } // read_work_dir

      
      char const *RegistryManager::read_shared_value(
         StrAsc &val,
         char const *value_name,
         HKEY key)
      {
         SharedPtr<FileManager> &hive = get_hive(key);
         SharedPtr<Xml::Element> e = decode_address(
            hive->get_doc(),
            reg_key_prefix,
            false);             // don't create the key if not there

         val.cut(0);
         if(e != 0)
         {
            for(Xml::Element::iterator ei = e->begin(); ei != e->end(); ++ei)
            {
               SharedPtr<Xml::Element> &child = *ei;
               if(child->get_name() == L"value" && child->get_attr_str(L"name") == value_name)
               {
                  val = child->get_cdata_str();
                  break;
               }
            }
         }
         return val.c_str();
      } // read_shared_value


      bool RegistryManager::write_shared_value(
         char const *value,
         char const *value_name,
         HKEY key)
      {
         SharedPtr<FileManager> &hive = get_hive(key);
         SharedPtr<Xml::Element> e = decode_address(
            hive->get_doc(),
            reg_key_prefix,
            true);             // create the key if not there
         bool rtn = true;

         if(e != 0)
         {
            SharedPtr<Xml::Element> v;
            for(Xml::Element::iterator ei = e->begin();
                ei != e->end() && v == 0;
                ++ei)
            {
               SharedPtr<Xml::Element> &child = *ei;
               if(child->get_name() == L"value" && child->get_attr_str(L"name") == value_name)
                  v = child;
            }
            if(v == 0)
            {
               v = e->add_element(L"value");
               v->set_attr_str(value_name,L"name");
            }
            v->set_cdata_str(value);
         }
         else
            rtn = false;
         if(rtn)
            hive->save();
         return rtn;
      } // write_shared_value

      
      bool RegistryManager::read_shared_uint4(
         uint4 &value,
         char const *value_name,
         HKEY key)
      {
         bool rtn = false;
         StrAsc temp;
         read_shared_value(temp,value_name,key);
         if(temp.length())
         {
            char *endptr;
            value = strtoul(temp.c_str(),&endptr,10);
            if(endptr[0] == 0)
               rtn = true;
         }
         return rtn;
      } // read_shared_uint4

      
      bool RegistryManager::write_shared_uint4(
         uint4 value,
         char const *value_name,
         HKEY key)
      {
         std::ostringstream temp;
         temp.imbue(std::locale::classic());
         temp << value;
         return write_shared_value(
            temp.str().c_str(),
            value_name,
            key);
      } // write_shared_uint4 


      char const *RegistryManager::read_anywhere_string(
         StrAsc &value,
         char const *value_name,
         char const *path,
         HKEY key,
         bool needs_64bit_value)
      {
         SharedPtr<FileManager> hive(get_hive(key));
         SharedPtr<Xml::Element> e(decode_address(hive->get_doc(), path, false));
         
         value.cut(0);
         if(e != 0)
         {
            for(Xml::Element::iterator ei = e->begin(); ei != e->end(); ++ei)
            {
               SharedPtr<Xml::Element> &child(*ei);
               if(child->get_name() == L"value" && child->get_attr_str(L"name") == value_name)
               {
                  value = child->get_cdata_str();
                  break;
               }
            }
         }
         return value.c_str();
      } // read_anywhere_string
   };
};

