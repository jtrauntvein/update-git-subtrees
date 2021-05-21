/* Csi.StringLoader.h

   Copyright (C) 2004, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 20 September 2004
   Last Change: Tuesday 21 April 2020
   Last Commit: $Date: 2020-04-22 16:53:40 -0600 (Wed, 22 Apr 2020) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_StringLoader_h
#define Csi_StringLoader_h

#include "StrAsc.h"
#include "CsiTypeDefs.h"
#include <list>
#include <map>
#include <stdexcept>
#include <locale>


namespace Csi
{
   namespace StringLoader
   {
      /**
       * Defines a translated string type that is used within the local string loader.
       */
      class value_type
      {
      public:
         /**
          * Specifies the english version of the string.
          */
         StrAsc english_value;

         /**
          * Specifies the localised version of the string.
          */
         StrAsc translated_value;

         /**
          * Specifies the string that describes the context of this string.
          */
         StrAsc context;

         /**
          * Set to true if the fake castellano string should not be generated for this string.
          */
         bool dont_mangle;

         /**
          * Constructor
          */
         value_type(
            StrAsc const &english_value_ = "",
            StrAsc const &translated_value_ = "",
            StrAsc const &context_ = "",
            bool dont_mangle_ = false):
            english_value(english_value_),
            translated_value(translated_value_),
            context(context_),
            dont_mangle(dont_mangle_)
         { }

         /**
          * Copy constructor
          */
         value_type(value_type const &other):
            english_value(other.english_value),
            translated_value(other.translated_value),
            context(other.context),
            dont_mangle(other.dont_mangle)
         { }

         /**
          * Copy operator
          */
         value_type &operator =(value_type const &other)
         {
            english_value = other.english_value;
            translated_value = other.translated_value;
            context = other.context;
            dont_mangle = other.dont_mangle;
            return *this;
         }
      };
   };


   /**
    * Defines an object that can be defined in a locap scope that manages a group of strings within
    * that scope.  Objects of this class will register themselves wit the singleton string loader
    * which will updated the translated string values when a new language is loaded.
    */
   class LocalStringLoader
   {
   private:
      /**
       * Specifies the name of the english version string file to which the strings managed by this
       * loader will be written.
       */
      StrAsc lang_file_base_name;

      /**
       * Specifies the version for the string language file.
       */
      StrAsc lang_file_version;

      /**
       * Specifies the name by which the group will be identified.
       */
      StrAsc group_name;

      /**
       * Specifies the list of strings in this group.
       */
   public:
      typedef StringLoader::value_type value_type;
      typedef std::map<uint4, value_type> values_type;
   private:
      values_type values;

      /**
       * Specifies the last language ID that was loaded.
       */
      uint4 last_loaded_lang_id;
      
   public:
      /**
       * Constructor.
       *
       * @param lang_file_base_name_ Specifies the name of the base (us-english) file for this
       * group.
       *
       * @param lang_file_version_ Specifies the version for the language file.
       *
       * @param group_name_ Specifies the name of the group.
       *
       * @param initialisors Specifies the set of strings in this group.  This is generally declared
       * as a static array.  The key of the last element in this array must be set to a value of
       * zero.
       */
      struct init_type
      {
         uint4 key;
         char const *english_value;
         char const *context;
         bool dont_mangle;
      };
      LocalStringLoader(
         StrAsc const &lang_file_base_name_,
         StrAsc const &lang_file_version_,
         StrAsc const &group_name_,
         init_type const initialisors[]);

      /**
       * Destructor
       */
      virtual ~LocalStringLoader();

      /**
       * @return Looks up the us-english value for the specified string id.
       *
       * @param key Specifies the identifier for the string.
       *
       * @throws std::invalid_argument Throws if the specified key does not exist.
       */
      StrAsc const &get_english_value(uint4 key) const
      {
         values_type::const_iterator vi = values.find(key);
         if(vi == values.end())
            throw std::invalid_argument("Invalid key value");
         return vi->second.english_value;
      }

      /**
       * @return Returns the translated version for the specified string identifier.
       *
       * @param key Specifies the string identifier.
       *
       * @throws std::invalid_argument Throws if the specified key does not exist.
       */
      StrAsc const &get_translated_value(uint4 key) const
      {
         values_type::const_iterator vi = values.find(key);
         if(vi == values.end())
            throw std::invalid_argument("Invalid key value");
         if(vi->second.translated_value.length())
            return vi->second.translated_value;
         else
            return vi->second.english_value;
      }

      /**
       * Sets the translated value for the specified key.
       *
       * @param key Specifies the identifier for the string.
       *
       * @param translated_value Specifies the translated value for the string.
       */
      void set_translated_value(
         uint4 key,
         StrAsc const &translated_value)
      {
         values_type::iterator vi = values.find(key);
         if(vi != values.end())
            vi->second.translated_value = translated_value;
      }

      /**
       * Clears all translated strings in this group.
       */
      void clear_translations()
      {
         last_loaded_lang_id = 0;
         for(values_type::iterator vi = values.begin();
             vi != values.end();
             ++vi)
         {
            vi->second.translated_value.cut(0);
         }
      }

      /**
       * @return Overloads the subscript operator to return the translated string for the
       * subscript.
       */
      StrAsc const &operator [](uint4 key) const
      { return get_translated_value(key); }

      // @group: container access methods

      /**
       * @return Returns the iterator to the first string.
       */
      typedef values_type::const_iterator const_iterator;
      typedef values_type::iterator iterator;
      const_iterator begin() const { return values.begin(); }
      iterator begin() { return values.begin(); }

      /**
       * @return Returns the iterator beyond the last string.
       */
      const_iterator end() const { return values.end(); }
      iterator end() { return values.end(); }

      /**
       * @return Returns true if there are no strings in this group.
       */
      bool empty() const { return values.empty(); }

      /**
       * @return Returns the number of strings in this group.
       */
      values_type::size_type size() const { return values.size(); }
      
      // @endgroup:

      /**
       * @return Returns the name of the us-english file for the stringfile.
       */
      StrAsc const &get_lang_file_base_name() const
      { return lang_file_base_name; }

      /**
       * @return Returns the version for the stringfile.
       */
      StrAsc const &get_lang_file_version() const
      { return lang_file_version; }

      /**
       * @return Returns the group name.
       */
      StrAsc const &get_group_name() const
      { return group_name; }

      /**
       * @return Returns the last language ID that was loaded.
       */
      uint4 get_last_loaded_lang_id() const
      { return last_loaded_lang_id; }

      /**
       * @param last_loaded_lang_id_ Specifies the identifier for the language file that was last
       * loaded for this group.
       */
      void set_last_loaded_lang_id(uint4 last_loaded_lang_id_)
      { last_loaded_lang_id = last_loaded_lang_id_; }
   };


   namespace StringLoader
   {
      /**
       * @return Returns the language identifier based upon the current locale as well as the
       * preferred language stored in the CSI registry keys.
       */
      uint4 get_os_preferred_language_id();

      /**
       * @return Returns the name of the directory where string files are expected to be found.
       */
      StrAsc get_string_files_dir();

      /**
       * Loads translation for all of the registered loaders based upon the preferred language ID
       * and directory name.
       */
      void load_language_files(
         StrAsc const &dir_name = get_string_files_dir(),
         uint4 language_id = get_os_preferred_language_id());

      /**
       * @return Returns the name of the stringfile that holds the translation strings for the
       * specified app name, version, and language ID.
       *
       * @param app_name Specifies the name of the application.
       *
       * @param app_version Specifies the version of the application.
       *
       * @param language_id Specifies the language ID.
       *
       * @param dir_name Specifies the directory to search.
       */
      StrAsc find_language_file(
         StrAsc const &app_name,
         StrAsc const &app_version,
         uint4 language_id = get_os_preferred_language_id(),
         StrAsc const &dir_name = get_string_files_dir());

      /**
       * @return Returns the language ID that was last used to load language files.
       */
      uint4 get_last_language_id();

      /**
       * @return Returns the locale name conforming to RFC-1766 for the specified language ID.
       *
       * @param language_id Specifies the language ID.
       */
      StrAsc languageid_to_rfc1766(uint4 language_id = get_last_language_id());

      /**
       * @return Returns the language ID for the specified locale name.
       *
       * @param language_name Specifies the language name.  This value must conform to RFC-1766.
       *
       * @throws std::exception Throws if the conversion fails.
       */
      uint4 rfc1766_to_languageid(StrAsc const &language_name);

      /**
       * @return Returns a name that can be used to construct a std::locale object based upon the
       * specified language ID.
       *
       * @param language_id Specifies the language ID.
       */
      StrAsc languageid_to_locale_name(uint4 language_id = get_last_language_id());

      /**
       * @return Returns true if the candidate is a better match to the preferred language than the
       * last ID specified.
       *
       * @param candidate Specifies the candidate language ID.
       *
       * @param preferred Specifies the preferred language ID.
       *
       * @param last Specifies the last language evaluated.
       */
      bool is_better_languageid(uint4 candidate, uint4 preferred, uint4 last);
      
      /**
       * @return Returns a locale object based upon the specified language ID.
       *
       * @param language_id Specifies the language ID.  If a value of zero is specified, a "classic
       * C" locale will be returned.
       */
      std::locale make_locale(uint4 language_id = 0xFFFFFFFF);

      /**
       * @return Returns true if the specified candidate language ID is a better fit for the
       * preference than the last language ID that was tested.
       *
       * @param candidate Specifies the candidate language ID.
       *
       * @param preferred Specifies the best match for the locale.
       *
       * @param last Specifies the previous best match.
       */
      bool is_better_languageid(uint4 candidate, uint4 preferred, uint4 last);

      /**
       * @return Returns the name for the specified langauge ID based upon the current regional
       * settings on the host computer.
       *
       * @param language_id Specifies the language ID to describe.
       */
      StrAsc languageid_to_english_name(uint4 language_id = get_last_language_id());

      /**
       * Scans the language files for the specified string file app name and version for the list of
       * languages that are supported.
       *
       * @param languages Returns the list of available translations.
       *
       * @param app_name Specifies the application name.
       *
       * @param app_version Specifies the application version.
       *
       * @param dir_name Specifies the directory where the translation files ar eexpected.
       *
       * @param requires_loader Set to true if the local loader associated with a string file
       * product must be registered in order for the string file to be considered.
       */
      void list_available_languages(
         std::list<uint4> &languages,
         StrAsc const &app_name,
         StrAsc const &app_version,
         StrAsc const &dir_name = get_string_files_dir(),
         bool requires_loader = true);

      /**
       * Writes language files for all of the local loaders that are registered.
       *
       * @param dir_name Specifies the directory where the files will be written.
       *
       * @param write_sp_ar Set to true if the fake spanish files are supposed to be created.
       */
      void write_english_language_files(StrAsc const &dir_name, bool write_sp_ar = true);

      /**
       * Opens the specified input file name, parses it as XML, and expands it as a string project
       * in the directory specified by output_dir.
       *
       * @param input_file_name Specifies the name of the input file.
       *
       * @param input_file Specifies the stream from which to read the input.
       *
       * @param output_dir_ Specifies the name of the directory where the string files will be
       * written.  If this is empty, the value returned by get_string_files_dir() will be used.
       */
      void expand_string_project(std::istream &input, StrAsc const &output_dir);
      void expand_string_project(StrAsc const &input_file_name, StrAsc const &output_dir);

      /**
       * Reads the specified input directory and writes a string file to the specified output file
       * name.
       *
       * @param input_dir Specifies the input directory.
       *
       * @param output_file_name Specifies the output file name.
       *
       * @param product_name Specifies the product name to be written in the file header.
       *
       * @param product_ver Specifies the product version to be written in the file header.
       */
      void pack_string_project(
         StrAsc const &input_dir,
         StrAsc const &output_file_name,
         StrAsc const &product_name,
         StrAsc const &product_ver);

      /**
       * Updates the english strings in the specified output project file name with the english
       * strings in the specified english file name.
       *
       * @param output_file_name Specifies the output file name.
       *
       * @param input_file_name Specifies the input file name.  This must refer to a file that
       * specifies the english language.
       */
      void update_english_string_project(
         StrAsc const &output_file_name, StrAsc const &input_file_name); 
   };
};


#endif
