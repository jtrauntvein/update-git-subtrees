/* Csi.StringLoader.cpp

   Copyright (C) 2004, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 20 September 2004
   Last Change: Wednesday 22 April 2020
   Last Commit: $Date: 2020-04-22 16:53:40 -0600 (Wed, 22 Apr 2020) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#endif
#include "Csi.RegistryManager.h"
#include "Csi.StringLoader.h"
#include "assert.h"
#include "Csi.OrderedList.h"
#include "Csi.FileSystemObject.h"
#include "Csi.Utils.h"
#include "Csi.ReadFileMapping.h"
#include "Csi.SharedPtr.h"
#include "Csi.StrAscStream.h"
#include "Csi.Protector.h"
#include "Csi.Xml.Element.h"
#include "Csi.Xml.EventParser.h"
#include "Csi.BuffStream.h"
#include "Csi.fstream.h"
#include <algorithm>
#include <sstream>
#include <functional>


namespace Csi
{
   namespace
   {
      class LocalStringLoaderRefLess
      {
      public:
         bool operator() (LocalStringLoader const *l1, LocalStringLoader const *l2) const
         {
            bool rtn = false;
            int base_name_comp = l1->get_lang_file_base_name().compare(
               l2->get_lang_file_base_name(), false);
            if(base_name_comp == 0)
               rtn = l1->get_group_name() < l2->get_group_name();
            else
               rtn = base_name_comp < 0;
            return rtn;
         }
      };


      typedef OrderedList<LocalStringLoader *, LocalStringLoaderRefLess> local_loaders_type;
      local_loaders_type *local_loaders = 0;


      class loader_uses_file
      {
      private:
         StrAsc file_name;
         
      public:
         loader_uses_file(StrAsc const &file_name_):
            file_name(file_name_)
         { }

         typedef LocalStringLoader const *argument_type;
         bool operator() (LocalStringLoader const*loader) const
         { return loader->get_lang_file_base_name() == file_name; }
      };


      class loader_uses_group
      {
      private:
         StrAsc group_name;

      public:
         loader_uses_group(StrAsc const &group_name_):
            group_name(group_name_)
         { }

         typedef LocalStringLoader const *argument_type;
         bool operator() (LocalStringLoader const *loader) const
         { return loader->get_group_name() == group_name; }
      };


      uint4 const usenglish_language_id = 1033;
      uint4 const spar_language_id = 11274;
      StrUni const string_file_name(L"stringfile");
      StrUni const language_id_name(L"languageid");
      StrUni const product_name(L"product");
      StrUni const version_name(L"version");
      StrUni const group_name(L"group");
      StrUni const group_name_name(L"name");
      StrUni const string_name(L"string");
      StrUni const english_name(L"english");
      StrUni const translated_name(L"translated");
      StrUni const context_name(L"context");


      /**
       * Extracts string file information from the root XML element.
       *
       * @return Returns true if the file could be parsed.
       *
       * @param info Specifies the file information that will be returned.
       *
       * @param input Specifies the input stream.
       */
      struct lang_file_info_type
      {
         StrAsc product_name;
         StrAsc version;
         uint4 language_id;
      };
      bool parse_file_language_info(lang_file_info_type &info, std::istream &in)
      {
         bool rtn(true);
         try
         {
            typedef Csi::Xml::EventParser parser_type;
            parser_type parser;
            bool found_product(false);
            bool found_version(false);
            bool found_languageid(false);
            parser_type::parse_outcome_type rcd(parser.parse(in));
            while(!found_product || !found_version || !found_languageid)
            {
               if(rcd == parser_type::parse_start_of_element)
               {
                  if(parser.get_elem_name() != string_file_name)
                     throw std::invalid_argument("invalid root element");
               }
               else if(rcd == parser_type::parse_attribute_read)
               {
                  if(parser.get_attr_name() == product_name)
                  {
                     info.product_name = parser.get_value().to_utf8();
                     found_product = true;
                  }
                  else if(parser.get_attr_name() == version_name)
                  {
                     info.version = parser.get_value().to_utf8();
                     found_version = true;
                  }
                  else if(parser.get_attr_name() == language_id_name)
                  {
                     info.language_id = parser.get_value_uint4();
                     found_languageid = true;
                  }
               }
               if(!found_languageid || !found_version || !found_product)
                  rcd = parser.parse(in);
            }
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // parse_file_language_info


      /**
       * Tracks the last language identifer that was specified in load_language_files().
       */
      uint4 last_language_id = 0;


      /**
       * Defines the mask that separates the base language from the sublanguage.  Masking the
       * language identifier will result in the code for the primary language.  Masking with the
       * bitwise inverse of this value will result in the sub-language identifier.
       */
      uint4 const language_mask = 0x3ff;
   };
   
   
   ////////////////////////////////////////////////////////////
   // class StringLoader definitions
   ////////////////////////////////////////////////////////////
   LocalStringLoader::LocalStringLoader(
      StrAsc const &lang_file_base_name_,
      StrAsc const &lang_file_version_,
      StrAsc const &group_name_,
      init_type const initialisors[]):
      lang_file_base_name(lang_file_base_name_),
      group_name(group_name_),
      last_loaded_lang_id(0),
      lang_file_version(lang_file_version_)
   {
      // create the strings for this loader
      for(size_t i = 0; initialisors[i].key != 0; ++i)
      {
         bool inserted = values.insert(
            std::make_pair(
               initialisors[i].key,
               value_type(
                  initialisors[i].english_value,
                  "",
                  initialisors[i].context,
                  initialisors[i].dont_mangle))).second;
         assert(inserted);
      }
      if(local_loaders == 0)
         local_loaders = new local_loaders_type;
      local_loaders->push(this);
   } // constructor


   LocalStringLoader::~LocalStringLoader()
   {
      if(local_loaders != 0)
      {
         local_loaders->remove(this);
         if(local_loaders->empty())
         {
            delete local_loaders;
            local_loaders = 0;
         }
      }
   } // destructor


   namespace StringLoader
   {
      uint4 get_os_preferred_language_id()
      {
#ifdef _WIN32
         uint4 rtn = ::GetUserDefaultLangID();
         uint4 reg_value;
         if(RegistryManager::read_shared_uint4(
               reg_value,
               "Language",
               HKEY_CURRENT_USER))
         {
            if(reg_value != 0)
               rtn = reg_value;
         }
#else
         uint4 rtn = usenglish_language_id;
         StrAsc lang_spec(getenv("LANG"));
         if(lang_spec.length())
         {
            lang_spec.cut(lang_spec.find("."));
            rtn = rfc1766_to_languageid(lang_spec);
         }
#endif
         return rtn;
      } // get_os_preferred_language_id


      StrAsc get_string_files_dir()
      {
         StrAsc rtn;
         RegistryManager::read_shared_value(rtn, "Languages");
         return rtn;
      } // get_string_files_dir


      void load_language_files(
         StrAsc const &base_dir,
         uint4 language_id)
      {
         // we will begin by clearing out any translations that might have already been loaded
         for(local_loaders_type::iterator li = local_loaders->begin();
             li != local_loaders->end();
             ++li)
            (*li)->clear_translations();

         // we can now consider each group of loaders that share the same base file name.  This will
         // limit the number of files that must actually be scanned
         local_loaders_type::iterator file_begin = local_loaders->begin();
         while(file_begin != local_loaders->end())
         {
            // we first need to locate the loader beyond the end of the group
            local_loaders_type::value_type &first_loader = *file_begin;
            local_loaders_type::iterator file_end = std::find_if(
               file_begin,
               local_loaders->end(),
               std::not1(loader_uses_file(first_loader->get_lang_file_base_name())));

            // we can now format the name of the directory where we expect to find this set of
            // loader's files
            std::list<StrAsc> loader_dir;
            loader_dir.push_back(base_dir);
            loader_dir.push_back(first_loader->get_lang_file_base_name());
            loader_dir.push_back(first_loader->get_lang_file_version());
            
            // we want to now scan this subdirectory
            FileSystemObject lang_dir(join_path(loader_dir.begin(), loader_dir.end()).c_str());
            FileSystemObject::children_type children;
            lang_dir.get_children(children, "*.str");
            while(!children.empty())
            {
               FileSystemObject str_file_info(children.front());
               children.pop_front();
               if(!str_file_info.get_is_valid() || str_file_info.is_directory())
                  continue;
               try
               {
                  // we need to examine the root attributes to determine whether the file should be
                  // parsed.  
                  lang_file_info_type file_info;
                  Csi::ifstream str_file(str_file_info.get_complete_name(), std::ios_base::binary);
                  if(!parse_file_language_info(file_info, str_file))
                     continue;
                  str_file.seekg(0);
                  
                  // We must ignore the file if its language is completely unrelated to the specified
                  // language 
                  uint4 file_lang = file_info.language_id;
                  bool use_file = true;
                  
                  if(file_lang != language_id &&
                     (file_lang & language_mask) != (language_id & language_mask))
                     use_file = false;
                  else
                  {
                     use_file = is_better_languageid(
                        language_id, file_lang, first_loader->get_last_loaded_lang_id());
                     if(file_info.version != first_loader->get_lang_file_version())
                        use_file = false;
                  }
                  if(!use_file)
                     continue;

                  // we have now verified that the language file is worth loading in its entirety
                  Xml::Element root(string_file_name);
                  root.input(str_file);
                  
                  // we now need to look at each group described in the XML structure.  Each one
                  // must correspond with a particular loader or it cannot be used
                  for(Xml::Element::iterator gi = root.begin();
                      use_file && gi != root.end();
                      ++gi)
                  {
                     Xml::Element::value_type &group = *gi;
                     if(group->get_name() == group_name)
                     {
                        StrAsc group_name(group->get_attr_str(group_name_name));
                        local_loaders_type::iterator group_begin = std::find_if(
                           file_begin, file_end, loader_uses_group(group_name));
                        local_loaders_type::iterator group_end = std::find_if(
                           group_begin, file_end, std::not1(loader_uses_group(group_name)));
                        for(Xml::Element::iterator si = group->begin(); si != group->end(); ++si)
                        {
                           Xml::Element::value_type &xml_string = *si;
                           Xml::Element::iterator ti = xml_string->find(translated_name);
                           if(ti != xml_string->end())
                           {
                              StrAsc translated_value((*ti)->get_cdata_str());
                              uint4 string_id = xml_string->get_attr_uint4(group_name_name);
                              for(local_loaders_type::iterator li = group_begin; li != group_end; ++li)
                              {
                                 LocalStringLoader *loader = *li;
                                 loader->set_translated_value(string_id, translated_value);
                                 loader->set_last_loaded_lang_id(file_lang);
                                 last_language_id = file_lang;
                              }
                           }
                        }
                     }
                  } 
               }
               catch(std::exception &)
               { }
            }
            file_begin = file_end;
         }
      } // load_language_files


      StrAsc find_language_file(
         StrAsc const &app_name,
         StrAsc const &app_version,
         uint4 language_id,
         StrAsc const &dir_name)
      {
         // we need to start by formatting the path to for the specified product.
         std::list<StrAsc> path;
         StrAsc app_path;
         path.push_back(dir_name);
         path.push_back(app_name);
         path.push_back(app_version);
         app_path = join_path(path.begin(), path.end());

         // we can now search for all of the files in that directory.
         FileSystemObject lang_dir(app_path.c_str());
         FileSystemObject::children_type children;
         uint4 last_language_id(0);
         StrAsc rtn;
         
         lang_dir.get_children(children, "*.str");
         while(!children.empty())
         {
            FileSystemObject str_file_info(children.front());
            children.pop_front();
            if(!str_file_info.get_is_valid() || str_file_info.is_directory())
               continue;
            try
            {
               // we need to read the header of the file.
               lang_file_info_type file_info;
               Csi::ifstream str_file(str_file_info.get_complete_name(), std::ios_base::binary);
               if(!parse_file_language_info(file_info, str_file))
                  continue;

               // we need to determine whether the file language is appropriate.
               uint4 file_lang(file_info.language_id);
               bool use_file(true);
               if(file_lang != language_id &&
                  (file_lang & language_mask) != (language_id & language_mask))
                  use_file = false;
               else
               {
                  use_file = is_better_languageid(language_id, file_lang, last_language_id);
                  if(file_info.version != app_version)
                     use_file = false;
               }
               if(use_file)
                  rtn = str_file_info.get_complete_name();
            }
            catch(std::exception &)
            { }
         }
         return rtn;
      } // find_language_file

      
      uint4 get_last_language_id()
      { return last_language_id; }


      namespace
      {
         /**
          * Lists information related to language and sub-language (generally country).  This
          * information also includes the windows locale name as well as the english name of the
          * language.  This information is derived from web sites describing ISO 639 and microsoft
          * language codes and includes the following:
          *
          *  http://www.loc.gov/standards/iso639-2/langcodes.html     - Library of Congress
          *  http://ftp.ics.uci.edu/pub/ietf/http/related/iso639.txt  - ISO 639:1988
          *  http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/nls_19ir.asp -
          *  sublang ids
          *  http://userpage.chemie.fu-berlin.de/diverse/doc/ISO_3166.html - ISO 3166 Country Codes
          */
         struct language_type
         {
            uint4 pri_id;
            uint4 sub_id;
            char const *rfc_name;
            char const *win32_locale_name;
            char const *english_lang_name;
         } languages[] =
         {
            { 0x36, 0x00, "af", "", "Afrikaans" },
            { 0x01, 0x00, "ar", "", "Arabic" },
            { 0x01, 0x01, "ar-SA", "", "Arabic (Saudi Arabia)" },
            { 0x01, 0x02, "ar-IQ", "", "Arabic (Iraq)" },
            { 0x01, 0x03, "ar-EG", "", "Arabic (Egypt)" },
            { 0x01, 0x04, "ar-LY", "", "Arabic (Libya)" },
            { 0x01, 0x05, "ar-DZ", "", "Arabic (Algeria)" },
            { 0x01, 0x06, "ar-MA", "", "Arabic (Morocco)" },
            { 0x01, 0x07, "ar-TN", "", "Arabic (Tunisia)" },
            { 0x01, 0x08, "ar-OM", "", "Arabic (Oman)" },
            { 0x01, 0x09, "ar-YE", "", "Arabic (Yemen)" },
            { 0x01, 0x0a, "ar-SY", "", "Arabic (Syria)" },
            { 0x01, 0x0b, "ar-JO", "", "Arabic (Jordan)" },
            { 0x01, 0x0c, "ar-LB", "", "Arabic (Lebanon)" },
            { 0x01, 0x0d, "ar-KW", "", "Arabic (Kuwait)" },
            { 0x01, 0x0e, "ar-AE", "", "Arabic (United Arab Emirates)" },
            { 0x01, 0x0f, "ar-BH", "", "Arabic (Bahrain)" },
            { 0x01, 0x10, "ar-QA", "", "Arabic (Qatar)" },
            { 0x4d, 0x00, "as", "", "Assames" },
            { 0x2c, 0x01, "az-latin", "", "Azerbaijani" },
            { 0x2c, 0x02, "az-cyrillic", "", "Azerbaijani" },
            { 0x23, 0x00, "be", "", "Byelorussian" },
            { 0x02, 0x00, "bg", "", "Bulgarian" },
            { 0x45, 0x00, "bn", "", "Bengali" },
            { 0x03, 0x00, "ca", "", "Catalan" },
            { 0x05, 0x00, "cs", "czech", "Czech" },
            { 0x06, 0x00, "da", "danish", "Danish" },
            { 0x07, 0x00, "de", "german", "German" },
            { 0x07, 0x01, "de", "german", "German" },
            { 0x07, 0x02, "de-CH", "german-swiss", "German (Swiss)" },
            { 0x07, 0x05, "de-LI", "german", "German (Lichenstein)" },
            { 0x07, 0x04, "de-LU", "german", "German (Luxembourg)" },
            { 0x65, 0x00, "div", "" },
            { 0x08, 0x00, "el", "", "Greek" },
            { 0x09, 0x00, "en", "english", "English" },
            { 0x09, 0x03, "en-AU", "english-aus", "English (Australia)" },
            { 0x09, 0x01, "en-US", "english-us", "English (US)" },
            { 0x09, 0x0a, "en-BZ", "english", "English (Belize)" },
            { 0x09, 0x04, "en-CA", "english-can", "English (Canada)" },
            { 0x09, 0x02, "en-GB", "english-uk", "English (United Kingdom)" },
            { 0x09, 0x06, "en-IE", "english-uk", "English (Ireland)" },
            { 0x09, 0x08, "en-JM", "english-uk", "English (Jamaica)" },
            { 0x09, 0x05, "en-NZ", "english-nz", "English (New Zealand)" },
            { 0x09, 0x0d, "en-PH", "english", "English (Philippines)" },
            { 0x09, 0x0b, "en-TT", "english", "English (Trinidad and Tabago)" },
            { 0x09, 0x07, "en-ZA", "english", "English (South Africa)" },
            { 0x09, 0x0c, "en-ZW", "english", "English (Zimbabwe)" },
            { 0x25, 0x00, "et", "", "Estonian" },
            { 0x2d, 0x00, "eu", "", "Basque" },
            { 0x0a, 0x00, "es", "spanish", "Spanish" },
            { 0x0a, 0x03, "es", "spanish", "Spanish" },
            { 0x0a, 0x0b, "es-AR", "spanish", "Spanish (Argentina)" },
            { 0x0a, 0x10, "es-BO", "spanish", "Spanish (Bolivia)" },
            { 0x0a, 0x0d, "es-CL", "spanish", "Spanish (Chile)" },
            { 0x0a, 0x09, "es-CO", "spanish", "Spanish (Colombia)" },
            { 0x0a, 0x05, "es-CR", "spanish", "Spanish (Costa Rica)" },
            { 0x0a, 0x07, "es-DO", "spanish", "Spanish (Dominican Republic)" },
            { 0x0a, 0x0c, "es-EC", "spanish", "Spanish (Ecuador)" },
            { 0x0a, 0x04, "es-GT", "spanish", "Spanish (Guatemala)" },
            { 0x0a, 0x12, "es-HN", "spanish", "Spanish (Honduras)" },
            { 0x0a, 0x02, "es-MX", "spanish-mexican", "Spanish (Mexico)" },
            { 0x0a, 0x13, "es-NI", "spanish", "Spanish (Nicaragua)" },
            { 0x0a, 0x06, "es-PA", "spanish", "Spanish (Panama)" },
            { 0x0a, 0x0a, "es-PE", "spanish", "Spanish (Peru)" },
            { 0x0a, 0x14, "es-PR", "spanish", "Spanish (Puerto Rico)" },
            { 0x0a, 0x0f, "es-PY", "spanish", "Spanish (Paraguay)" },
            { 0x0a, 0x0e, "es-UY", "spanish", "Spanish (Uruguay)" },
            { 0x0a, 0x11, "es-SV", "spanish", "Spanish (El Salvador)" },
            { 0x0a, 0x08, "es-VE", "spanish", "Spanish (Venezuela)" },
            { 0x0c, 0x00, "fr", "french", "French" },
            { 0x0c, 0x01, "fr", "french", "French" },
            { 0x0c, 0x02, "fr-BE", "french-belgian", "French(Belgian)" },
            { 0x0c, 0x03, "fr-CA", "french-canadian", "French(Canadian)" },
            { 0x0c, 0x04, "fr-CH", "french", "French(Swiss)" },
            { 0x0c, 0x05, "fr-LU", "french", "French(Luxembourg)" },
            { 0x0c, 0x06, "fr-MC", "french", "French(Monaco)" },
            { 0x0b, 0x00, "fi", "", "Finnish" },
            { 0x38, 0x00, "fo", "", "Faroese" },
            { 0x47, 0x00, "gu", "", "Gujarati" },
            { 0x0d, 0x00, "he", "", "Hebrew" },
            { 0x39, 0x00, "hi", "", "Hindi" },
            { 0x1a, 0x00, "hr", "", "Croation" },
            { 0x0e, 0x00, "hu", "hungarian", "Hungarian" },
            { 0x2b, 0x00, "hy", "", "Armenian" },
            { 0x21, 0x00, "id", "", "Indonesian" },
            { 0x0f, 0x00, "is", "", "Icelandic" },
            { 0x10, 0x00, "it", "italian", "Italian" },
            { 0x10, 0x01, "it", "italian", "Italian" },
            { 0x10, 0x02, "it-CH", "italian-swiss" "Italian (Swiss)" },
            { 0x11, 0x00, "ja", "japanese", "Japanese" },
            { 0x37, 0x00, "ka", "", "Georgian" },
            { 0x3f, 0x00, "kk", "", "Kazahk" },
            { 0x4b, 0x00, "kn", "", "Kannada" },
            { 0x12, 0x00, "ko", "", "Korean" },
            { 0x12, 0x01, "ko-KR", "", "Korean" },
            { 0x60, 0x00, "ks", "", "Kashmiri" },
            { 0x60, 0x02, "ks-IN", "", "Kashmiri (India)" },
            { 0x40, 0x00, "ky", "", "Kirghiz" },
            { 0x27, 0x00, "lt", "", "Lithuanian" },
            { 0x27, 0x01, "lt", "", "Lithuanian" },
            { 0x26, 0x00, "lv", "", "Latvian" },
            { 0x2f, 0x00, "mk", "", "Macedonian" },
            { 0x50, 0x00, "mn", "", "Mongolian" },
            { 0x4e, 0x00, "mr", "", "Marathi" },
            { 0x3e, 0x00, "ms", "", "Malay" },
            { 0x3e, 0x01, "ms-MY", "", "Malay (Malaysia)" },
            { 0x3e, 0x01, "ms-BN", "", "Malay (Brunei Darussalam" },
            { 0x61, 0x00, "ne", "", "Nepali" },
            { 0x61, 0x02, "ne-IN", "", "Nepali (India)" },
            { 0x13, 0x00, "nl", "dutch", "Dutch" },
            { 0x13, 0x01, "nl", "dutch", "Dutch" },
            { 0x13, 0x02, "nl-BE", "dutch-belgian", "Dutch (Belgium)" },
            { 0x14, 0x00, "no", "norwegian", "Norwegian" },
            { 0x48, 0x00, "or", "", "Oriya" },
            { 0x15, 0x00, "pl", "", "Polish" },
            { 0x16, 0x00, "pt", "portuguese", "Portuguese" },
            { 0x16, 0x02, "pt", "portuguese", "Portuguese (Portugal)" },
            { 0x16, 0x01, "pt-BR", "portuguese_brazil", "Portuguese (Brasil)" },
            { 0x18, 0x00, "ro", "", "Romanian" },
            { 0x19, 0x00, "ru", "russian", "Russian" },
            { 0x4f, 0x00, "sa", "", "Sanskrit" },
            { 0x59, 0x00, "sd", "" , "Sindhi"},
            { 0x1b, 0x00, "sk", "slovak", "Slovak" },
            { 0x24, 0x00, "sl", "", "Slovenian" },
            { 0x1c, 0x00, "sq", "", "Albanian" },
            { 0x1a, 0x00, "sr", "", "Serbian" },
            { 0x1d, 0x00, "sv", "swedish", "Swedish" },
            { 0x1d, 0x01, "sv", "swedish", "Swedish" },
            { 0x1d, 0x02, "sv-FI", "swedish-finland", "Swedish (Finland)" },
            { 0x41, 0x00, "sw", "", "Swahili" },
            { 0x49, 0x00, "ta", "", "Tamil" },
            { 0x4a, 0x00, "te", "", "Telugu" },
            { 0x1e, 0x00, "th", "", "Thai" },
            { 0x1f, 0x00, "tr", "turkish", "Turkish" },
            { 0x44, 0x00, "tt", "", "Tatar" },
            { 0x22, 0x00, "uk", "", "Ukrainian" },
            { 0x20, 0x00, "ur", "", "Urdu" },
            { 0x20, 0x01, "ur-PK", "", "Urdu (Pakistan)" },
            { 0x20, 0x02, "ur-IN", "", "Urdu (India)" },
            { 0x43, 0x00, "uz", "", "Uzbek" },
            { 0x2a, 0x00, "vi", "", "Vietmanese" }, 
            { 0x04, 0x01, "zh-TW", "chinese-traditional", "Chinese (Taiwan)" },
            { 0x04, 0x02, "zh-CN", "chinese-simplified", "Chinese (China)" },
            { 0x04, 0x03, "zh-HK", "chinese", "Chinese (Hong Kong)" },
            { 0x04, 0x04, "zh-SG", "chinese", "Chinese (Singapore)" },
            { 0x04, 0x05, "zh-MC", "chinese", "Chinese (Macau)" },
            { 0x00, 0x00, 0, 0 }
         };


         /*
          * Holds a cache of locales that have been previously constructed. 
          */
         typedef std::map<uint4, std::locale> locale_cache_type;
         Protector<locale_cache_type> locale_cache;
      };

      
      StrAsc languageid_to_rfc1766(uint4 language_id)
      {
         StrAsc rtn;
         uint4 lp = language_id & language_mask;
         uint4 ls = (language_id & ~language_mask) >> 10;

         // search for the language that best matches the identifier
         for(int i = 0; languages[i].pri_id != 0; ++i)
         {
            if(lp == languages[i].pri_id)
            {
               rtn = languages[i].rfc_name;
               for(int j = i; languages[j].pri_id == lp; ++j)
               {
                  if(languages[j].sub_id == ls)
                  {
                     rtn = languages[j].rfc_name;
                     break;
                  }
               }
               break;
            }
         }
         return rtn;
      } // languageid_to_rfc1766


      uint4 rfc1766_to_languageid(StrAsc const &language_name)
      {
         // we will allow the dash or underscore to divide the primary from the secondary language id
         uint4 rtn = 0;
         StrAsc name(language_name);
         name.replace("_","-");

         for(int i = 0; rtn == 0 && languages[i].pri_id != 0; ++i)
         {
            if(name == languages[i].rfc_name)
               rtn = (languages[i].sub_id << 10) + languages[i].pri_id;
         }
         return rtn;
      } // rfc1766_to_language_id


      StrAsc languageid_to_locale_name(uint4 language_id)
      {
         StrAsc rtn;
         uint4 lp = language_id & language_mask;
         uint4 ls = (language_id & ~language_mask) >> 10;
         
         for(int i = 0; languages[i].pri_id != 0; ++i)
         {
            if(languages[i].pri_id == lp)
            {
               rtn = languages[i].win32_locale_name;
               for(int j = i; languages[j].pri_id == lp; ++j)
               {
                  if(languages[j].sub_id == ls)
                  {
#ifdef _WIN32
                     rtn = languages[j].win32_locale_name;
#else
                     rtn = languages[j].rfc_name;
                     rtn.replace("-","_");
#endif
                     break;
                  } 
               }
               break;
            }
         }
         return rtn;
      } // languageid_to_locale_name
      

      std::locale make_locale(uint4 language_id)
      {
         Protector<locale_cache_type>::key_type cache(locale_cache);
         locale_cache_type::iterator li = cache->find(language_id);
         if(li == cache->end())
         {
            if(language_id == 0)
            {
               std::pair<locale_cache_type::iterator, bool> rtn = cache->insert(
                  locale_cache_type::value_type(0, std::locale::classic()));
               li = rtn.first;
            }
            else if(language_id == 0xFFFFFFFF)
            {
               std::pair<locale_cache_type::iterator, bool> rtn(
                  cache->insert(
                     locale_cache_type::value_type(0xFFFFFFFF, std::locale(""))));
               li = rtn.first;
            }
            else
            {
               std::pair<locale_cache_type::iterator, bool> rtn = cache->insert(
                  locale_cache_type::value_type(
                     language_id,
                     std::locale(
                        languageid_to_locale_name(language_id).c_str())));
               li = rtn.first;
            }
         }
         return li->second;
      } // make_locale

      
      bool is_better_languageid(
         uint4 candidate,
         uint4 preferred,
         uint4 last)
      {
         bool rtn = false;
         uint4 cp = candidate & language_mask;
         uint4 cs = candidate & ~language_mask;
         uint4 pp = preferred & language_mask;
         uint4 ps = preferred & ~language_mask;
         uint4 lp = last & language_mask;
         uint4 ls = last & ~language_mask;

         if(lp == 0 && ls == 0 && pp == cp)
            rtn = true;
         if(pp == cp && ps == cs)
            rtn = true;
         if(pp != lp && pp == cp)
            rtn = true;
         if(lp == pp && ls == ps)
            rtn = false;
         return rtn;
      } // is_better_languageid


      StrAsc languageid_to_english_name(uint4 language_id)
      {
         OStrAscStream rtn;
#ifdef _WIN32
         // since windows provides the functions to generate the names, we will use them.
         wchar_t lc_data[256];
         int rcd = ::GetLocaleInfoW(
            MAKELCID(
               language_id,
               SORT_DEFAULT),
            LOCALE_SENGLANGUAGE,
            lc_data,
            sizeof(lc_data));
         if(rcd != 0)
         {
            rtn << lc_data;
            rcd = ::GetLocaleInfoW(
               MAKELCID(
                  language_id,
                  SORT_DEFAULT),
               LOCALE_SENGCOUNTRY,
               lc_data,
               sizeof(lc_data));
            if(rcd != 0 && lc_data[0] != 0)
               rtn << " " << lc_data;
         }
#else
         // otherwise, we will use the names recorded in the table
         uint4 primary = language_id & language_mask;
         uint4 secondary = (language_id & ~language_mask) >> 10;
         int best_match = -1;

         for(int i = 0; languages[i].pri_id != 0; ++i)
         {
            if(languages[i].pri_id == primary)
            {
               if(languages[i].sub_id == secondary)
               {
                  best_match = i;
                  break;
               }
               else if(languages[i].sub_id == 0)
                  best_match = i;
            }
         }
         if(best_match != -1)
            rtn << languages[best_match].english_lang_name;
#endif
         return rtn.str();
      } // languageid_to_english_name


      void list_available_languages(
         std::list<uint4> &languages,
         StrAsc const &app_name,
         StrAsc const &app_version,
         StrAsc const &dir_name,
         bool requires_loader) 
      {
         // we always assume support for us english
         languages.clear();
         languages.push_back(usenglish_language_id);

         // we will look in the directory specific to this application for the possible language
         // files.
         std::list<StrAsc> my_dir;
         my_dir.push_back(dir_name);
         my_dir.push_back(app_name);
         my_dir.push_back(app_version);
         
         // the algorithm for listing the languages will be similar to that used for loading
         // language files.  we will scan the directory for all string files and then, each time we
         // encounter a string file that matches one of our loaders and has a unique language
         // identifier (one we haven't encountered before), we will include it in the languages
         // list.
         FileSystemObject lang_dir(join_path(my_dir.begin(), my_dir.end()).c_str());
         FileSystemObject::children_type children;

         lang_dir.get_children(children, "*.str");
         while(!children.empty())
         {
            FileSystemObject str_file_info = children.front();
            children.pop_front();
            if(!str_file_info.get_is_valid() || str_file_info.is_directory())
               continue;

            try
            {
               lang_file_info_type file_info;
               Csi::ifstream input(str_file_info.get_complete_name(), std::ios_base::binary);
               if(!parse_file_language_info(file_info, input))
                  continue;

               // we'll ignore the file if we already know its language
               if(std::find(languages.begin(),languages.end(),file_info.language_id) != languages.end())
                  continue;

               if(requires_loader)
               {
                  // we need to find a loader to claim this file
                  local_loaders_type::iterator file_begin = std::find_if(
                     local_loaders->begin(),
                     local_loaders->end(),
                     loader_uses_file(file_info.product_name));
                  if(file_begin == local_loaders->end() ||
                     file_info.version != (*file_begin)->get_lang_file_version())
                     continue;
               }

               // we have passed all of the tests for uniqueness and use if we made it this far.  We
               // will put the language id in the list.
               languages.push_back(file_info.language_id); 
            }
            catch(std::exception &)
            { } 
         } 
      } // list_available_languages

      
      void write_english_language_files(StrAsc const &dir_name, bool write_sp_ar)
      {
         if(local_loaders != 0)
         {
            // we will iterate the list of local loaders.  We need to output each set of loaders that
            // share the same base file name into the same file.  Fortunately, the list of loaders is
            // kept sorted so that all loaders that share the same file name are adjacent.  We will
            // take advantage of this by using find_if() to locate the end range
            local_loaders_type::const_iterator file_begin = local_loaders->begin();
            local_loaders_type::const_iterator file_end;
            local_loaders_type::const_iterator loaders_end = local_loaders->end();
            
            while(file_begin != loaders_end)
            {
               // the end of the range will be the first loader that does not use the same file as
               // the start of the range.
               LocalStringLoader const *first_loader = *file_begin;
               file_end = std::find_if(
                  file_begin,
                  loaders_end,
                  std::not1(loader_uses_file(first_loader->get_lang_file_base_name())));

               // all of the loaders in the selected range should be output to the same file.
               Xml::Element root(string_file_name);
               Xml::Element sp_root(string_file_name);
               
               root.set_attr_str(first_loader->get_lang_file_base_name(), product_name);
               sp_root.set_attr_str(first_loader->get_lang_file_base_name(), product_name);
               root.set_attr_str(first_loader->get_lang_file_version(), version_name);
               sp_root.set_attr_str(first_loader->get_lang_file_version(), version_name);
               root.set_attr_uint4(usenglish_language_id, language_id_name);
               sp_root.set_attr_uint4(spar_language_id, language_id_name);

               // as we partitioned all loaders that share the same file, we also need to partition
               // all loaders that share the same group within the sub-range.
               local_loaders_type::const_iterator group_begin = file_begin;
               local_loaders_type::const_iterator group_end;
            
               while(group_begin != file_end)
               {
                  LocalStringLoader *first_group_loader = *group_begin;
                  SharedPtr<Xml::Element> xml_group = root.add_element(group_name);
                  Xml::Element::value_type spxml_group(sp_root.add_element(group_name)); 
               
                  group_end = std::find_if(
                     group_begin,
                     file_end,
                     std::not1(loader_uses_group(first_group_loader->get_group_name())));
                  xml_group->set_attr_str(first_group_loader->get_group_name(), group_name_name);
                  spxml_group->set_attr_str(first_group_loader->get_group_name(), group_name_name);
                  while(group_begin != group_end)
                  {
                     LocalStringLoader *loader = *group_begin;
                     
                     for(LocalStringLoader::const_iterator li = loader->begin();
                         li != loader->end();
                         ++li)
                     {
                        Xml::Element::value_type en_xml_string(xml_group->add_element(string_name));
                        Xml::Element::value_type sp_xml_string(spxml_group->add_element(string_name));
                        Xml::Element::value_type en_english(en_xml_string->add_element(english_name));
                        Xml::Element::value_type sp_english(sp_xml_string->add_element(english_name));
                        Xml::Element::value_type en_translated(en_xml_string->add_element(translated_name));
                        Xml::Element::value_type sp_translated(sp_xml_string->add_element(translated_name));
                        Xml::Element::value_type en_context(en_xml_string->add_element(context_name));
                        Xml::Element::value_type sp_context(sp_xml_string->add_element(context_name));
                        OStrAscStream translated_value;
                        
                        en_english->set_cdata_str(li->second.english_value);
                        sp_english->set_cdata_str(li->second.english_value); 
                        en_context->set_cdata_str(li->second.context);
                        sp_context->set_cdata_str(li->second.context); 
                        en_xml_string->set_attr_uint4(li->first, group_name_name);
                        sp_xml_string->set_attr_uint4(li->first, group_name_name);
                        translated_value << li->second.english_value;
                        if(!li->second.dont_mangle)
                           translated_value << "_" << li->first;
                        sp_translated->set_cdata_str(translated_value.str());
                     }   
                     ++group_begin;
                  } 
               }

               // before the file is written, we must ensure that the directory for that file exists
               OStrAscStream file_dir;
               file_dir << dir_name;
               if(dir_name.last() != FileSystemObject::dir_separator())
                  file_dir << FileSystemObject::dir_separator();
               file_dir << first_loader->get_lang_file_base_name() << FileSystemObject::dir_separator()
                        << first_loader->get_lang_file_version();
               createNestedDir(file_dir.str().c_str());
               
               // we have now created all of the groups within the file.  We must now output the
               // document to the file.
               OStrAscStream file_name;
               file_name << file_dir.str() << FileSystemObject::dir_separator()
                         << first_loader->get_lang_file_base_name()
                         << "_" << first_loader->get_lang_file_version()
                         << "_english.str";
               Csi::ofstream output_file(file_name.str().c_str());
               output_file << "<?xml version='1.0'?>\n";
               root.output(output_file,true);
               output_file << "\n\n";
               if(write_sp_ar)
               {
                  file_name.str("");
                  file_name << file_dir.str() << FileSystemObject::dir_separator()
                            << first_loader->get_lang_file_base_name()
                            << "_" << first_loader->get_lang_file_version()
                            << "_sp-ar.str";
                  Csi::ofstream sp_output_file(file_name.str().c_str());
                  sp_output_file << "<?xml version='1.0'?>\n";
                  sp_root.output(sp_output_file, true);
                  sp_output_file << "\n\n";
               }
               file_begin = file_end;
            }
         }
      } // write_english_language_files


      void expand_string_project(StrAsc const &input_file_name, StrAsc const &output_dir)
      {
         Csi::ifstream input(input_file_name, std::ios_base::binary);
         if(!input)
            throw OsException("failed to open input");
         expand_string_project(input, output_dir);
      } // expand_string_project

      
      void expand_string_project(std::istream &input, StrAsc const &output_dir_)
      {
         // choose an output directory.
         StrAsc output_dir(output_dir_);
         if(output_dir.length() == 0)
            output_dir = get_string_files_dir();

         // we need to open the input file name.
         Xml::Element string_project(L"stringproject");
         string_project.input(input);
         if(string_project.get_name() != L"stringproject")
            throw std::invalid_argument("input is not a string project");

         // We can now iterate through the the stringfile children of the string project.
         StrAsc product_name, product_version;
         uint4 language_id;
         Csi::OStrAscStream string_file_name;
         StrAsc string_path;
         char input_buffer[1024];
         
         for(auto fi = string_project.begin(); fi != string_project.end(); ++fi)
         {
            auto &child(*fi);
            size_t child_size((size_t)(child->get_end_offset() - child->get_begin_offset() + 1));
            size_t bytes_read(0);
            if(child->get_name() == L"stringfile")
            {
               // we need to format the name of the file to be written.
               product_name = child->get_attr_str(L"product");
               product_version = child->get_attr_str(L"version");
               language_id = child->get_attr_uint4(L"languageid");
               string_file_name.str("");
               string_file_name << product_name << "_" << product_version << "_"
                                << languageid_to_english_name(language_id) << ".str";
               
               // we need to put together the components of the string file name to form the path.
               std::deque<StrAsc> path_comps = {
                  output_dir, product_name, product_version, string_file_name.str()
               };
               string_path = join_path(path_comps.begin(), path_comps.end());
               check_file_name_path(string_path.c_str());

               // we now need to open the output file.
               Csi::ofstream output(string_path, std::ios_base::binary);
               if(!output)
                  throw OsException("failed to open output file");
               input.seekg(child->get_begin_offset());
               while(bytes_read < child_size)
               {
                  size_t block_size(csimin(sizeof(input_buffer), child_size - bytes_read));
                  input.read(input_buffer, block_size);
                  output.write(input_buffer, block_size);
                  bytes_read += block_size;
               }
               output.close();
            }
         }
      } // expand_string_project


      void pack_string_project(
         StrAsc const &input_dir, StrAsc const &output_file_name, StrAsc const &product_name, StrAsc const &product_ver)
      {
         // we need to work through all of the children in the directory and pick out all of the
         // english language string files.
         std::deque<FileSystemObject> input_queue{ FileSystemObject(input_dir.c_str()) };
         std::deque<StrAsc> output_queue;

         while(!input_queue.empty())
         {
            FileSystemObject candidate(input_queue.front());
            input_queue.pop_front();
            if(candidate.get_is_valid() && candidate.get_name() != "." && candidate.get_name() != "..")
            {
               if(candidate.is_directory())
               {
                  FileSystemObject::children_type children;
                  candidate.get_children(children);
                  input_queue.insert(input_queue.end(), children.begin(), children.end());
               }
               else
               {
                  StrAsc input_name(candidate.get_complete_name());
                  Csi::ifstream input(input_name, std::ios_base::binary);
                  if(input)
                  {
                     Xml::EventParser parser;
                     bool should_use(true);
                     while(should_use)
                     {
                        try
                        {
                           Xml::EventParser::parse_outcome_type outcome(parser.parse(input));
                           if(outcome == Xml::EventParser::parse_start_of_element)
                           {
                              if(parser.get_elem_name() != L"stringfile")
                                 should_use = false;
                           }
                           if(outcome == Xml::EventParser::parse_attribute_read)
                           {
                              if(parser.get_attr_name() == L"languageid" && parser.get_value() == L"1033")
                                 break;
                              else
                                 should_use = false;
                           }
                           else if(outcome == Xml::EventParser::parse_end_of_element)
                              should_use = false;
                        }
                        catch(std::exception &e)
                        {
                           std::cout << "error parsing " << input_name << ": " << e.what() << std::endl;;
                           should_use = false;
                        }
                     }
                     
                     if(should_use)
                        output_queue.push_back(candidate.get_complete_name());
                  }
               }
            }
         }
         if(output_queue.empty())
            throw std::invalid_argument("no string files in input directory");

         // we have now accumulated the names of all of the string files to include.  We can now
         // generate the output file and its header.
         Csi::ofstream output(output_file_name, std::ios_base::binary);
         char buff[1024];
         char const *xml_header("<?xml version='1.0'?>");
         size_t const xml_header_len(strlen(xml_header));
         
         if(!output)
            throw std::invalid_argument("failed to open output file");
         output << xml_header << "\r\n"
                << "<stringproject product=\"" << product_name << "\" version=\"" << product_ver << "\">\r\n";
         while(!output_queue.empty())
         {
            StrAsc input_name(output_queue.front());
            FILE *input(open_file(output_queue.front(), "rb"));
            
            output_queue.pop_front();
            if(input)
            {
               size_t bytes_read;
               uint4 fragments_count(0);
               while((bytes_read = fread(buff, 1, sizeof(buff), input)) != 0)
               {
                  char const *start(buff);
                  if(fragments_count == 0)
                  {
                     if(memcmp(buff, xml_header, xml_header_len) == 0)
                     {
                        bytes_read -= xml_header_len;
                        start += xml_header_len;
                     }
                  }
                  output.write(start, bytes_read);
                  ++fragments_count;
               }
               fclose(input);
               output << "\r\n";
            }
         }
         output << "</stringproject>\r\n";
      } // pack_string_project


      namespace
      {
         /**
          * Reads text from the specified input stream between the two offsets into the provided
          * buffer.
          *
          * @param input Specifies the input stream.
          *
          * @param dest Specifies the buffer to which the text will be written.
          *
          * @param begin Specifies the start offset in the stream.
          *
          * @param end Specifies the end offset in the stream,
          */
         void read_text(StrAsc &dest, std::istream &input, int8 begin, int8 end)
         {
            char buff[2048];
            int8 const size(end - begin + 1);
            int8 bytes_read(0);
            
            dest.cut(0);
            input.seekg(begin);
            while(bytes_read < size)
            {
               size_t block_size(csimin(sizeof(buff), size_t(size - bytes_read)));
               input.read(buff, block_size);
               dest.append(buff, block_size);
               bytes_read += block_size;
            }
         }
      };
      

      void update_english_string_project(
         StrAsc const &output_file_name, StrAsc const &input_file_name)
      {
         // we need to open and try to parse the input file.
         Csi::ifstream input(input_file_name, std::ios_base::binary);
         Xml::Element input_xml(L"");
         if(!input)
            throw std::invalid_argument("failed to open input");
         input_xml.input(input);

         // we now need to build a map of all the input strings.
         typedef std::map<StrAsc, StrAsc> group_type;
         typedef SharedPtr<group_type> group_handle;
         typedef std::map<StrAsc, group_handle> file_type;
         typedef SharedPtr<file_type> file_handle;
         typedef std::map<StrAsc, file_handle> project_type;
         project_type project;
         StrAsc english;
         for(auto fi = input_xml.begin(); fi != input_xml.end(); ++fi)
         {
            auto file_xml(*fi);
            if(file_xml->get_name() == L"stringfile" && file_xml->get_attr_uint4(L"languageid") == 1033)
            {
               StrAsc product_name(file_xml->get_attr_str(L"product"));
               file_handle file(new file_type);
               project[product_name] = file;
               for(auto gi = file_xml->begin(); gi != file_xml->end(); ++gi)
               {
                  auto &group_xml(*gi);
                  if(group_xml->get_name() == L"group")
                  {
                     StrAsc group_name(group_xml->get_attr_str(L"name"));
                     group_handle group(new group_type);
                     (*file)[group_name] = group;
                     for(auto si = group_xml->begin(); si != group_xml->end(); ++si)
                     {
                        auto string_xml(*si);
                        if(string_xml->get_name() == L"string")
                        {
                           // we want to leave any HTML and other markup within the string unchanged
                           // so we will read the untranslated content directly from the file.
                           StrAsc string_name(string_xml->get_attr_str(L"name"));
                           auto english_xml(string_xml->find_elem(L"english"));
                           read_text(english, input, english_xml->get_begin_offset(), english_xml->get_end_offset());
                           (*group)[string_name] = english;
                        }
                     }
                  }
               }
            }
         }
         input.close();

         // we now need to open and parse the output file.
         FileSystemObject output_info(output_file_name.c_str());
         Xml::Element target(L"");
         input.open(output_file_name, std::ios_base::binary);
         if(!input)
            throw std::invalid_argument("unable to open the output file");
         target.input(input);

         // with the output parsed, we can now iterate through it and build up a collection of
         // changes that need to be made to the file in order to synch the english strings.
         struct replacement_type
         {
            int8 begin_pos;
            int8 end_pos;
            StrAsc replacement;
            StrAsc preceding;
         };
         typedef SharedPtr<replacement_type> replacement_handle;
         std::deque<replacement_handle> replacements;
         for(auto pi = target.begin(); pi != target.end(); ++pi)
         {
            auto file_xml(*pi);
            if(file_xml->get_name() == L"stringfile")
            {
               auto file_it(project.find(file_xml->get_attr_str(L"product")));
               if(file_it != project.end())
               {
                  file_handle &file(file_it->second);
                  for(auto gi = file_xml->begin(); gi != file_xml->end(); ++gi)
                  {
                     auto group_xml(*gi);
                     auto group_it(file->find(group_xml->get_attr_str(L"name")));
                     if(group_it != file->end())
                     {
                        auto &group(group_it->second);
                        for(auto si = group_xml->begin(); si != group_xml->end(); ++si)
                        {
                           auto string_xml(*si);
                           auto string_it(group->find(string_xml->get_attr_str(L"name")));
                           if(string_it != group->end())
                           {
                              // generate the replacement structure.
                              auto english_xml(string_xml->find_elem(L"english"));
                              replacement_handle replacement(new replacement_type);
                              replacement->end_pos = english_xml->get_end_offset();
                              replacement->begin_pos = english_xml->get_begin_offset();
                              replacement->replacement = string_it->second;

                              // we need to capture any preceding text.
                              int8 preceding_start(0);
                              if(!replacements.empty())
                                 preceding_start = replacements.back()->end_pos + 1;
                              read_text(replacement->preceding, input, preceding_start, replacement->begin_pos - 1);
                              replacements.push_back(replacement);
                           }
                        }
                     }
                  }
               }
            }
         }

         // we need to add a final replacement for everything between the last and the end of the
         // file.
         if(!replacements.empty())
         {
            replacement_handle replacement(new replacement_type);
            replacement->begin_pos = replacements.back()->end_pos + 1;
            replacement->end_pos = output_info.get_size() - 1;
            read_text(replacement->preceding, input, replacement->begin_pos, replacement->end_pos);
            replacements.push_back(replacement);
         }
         input.close();

         // we are finally ready to overwrite the output file using the replacements list generated
         // above.
         Csi::ofstream output(output_file_name, std::ios_base::binary);
         if(!output)
            throw std::invalid_argument("failed to open output for overwriting");
         for(auto ri = replacements.begin(); ri != replacements.end(); ++ri)
         {
            auto &replacement(*ri);
            output.write(replacement->preceding.c_str(), replacement->preceding.length());
            output.write(replacement->replacement.c_str(), replacement->replacement.length());
         }
      } // update_english_string_project
   }; 
};   
