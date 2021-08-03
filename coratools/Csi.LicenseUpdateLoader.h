/* Csi.LicenseUpdateLoader.h

   Copyright (C) 2018, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 26 May 2018
   Last Change: Wednesday 11 July 2018
   Last Commit: $Date: 2020-12-04 11:27:25 -0600 (Fri, 04 Dec 2020) $
   Last Changed by: $Author: amortenson $

*/

#ifndef Csi_LicenseUpdateLoader_h
#define Csi_LicenseUpdateLoader_h

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAIN
#include <windows.h>
#endif
#include "CsiTypeDefs.h"


namespace Csi
{
   /**
    * Defines a component that is responsible for loading the license client updater DLL and making
    * its functions available to the operating system.
    */
   class LicenseUpdateLoader
   {
   public:
      /**
       * Loads the CSI License updater DLL.  The DLL will be expected in the application binary
       * directory for process for the debug build and will be expected to be in the common
       * directory otherwise.
       */
      LicenseUpdateLoader();

      /**
       * Destructor
       */
      virtual ~LicenseUpdateLoader();

      /**
       * Pointer to the DLL cslicense_initialise() function.  This will initialise a license client
       * within the DLL using the product parameters.
       *
       * @param app_name Specifies the name of the application. This value is only used if the
       * is_licensed parameter is set to non-zero.
       *
       * @param model Specifies the model of the product.
       *
       * @param version Specifies the version of the product.
       *
       * @param serial_no Specifies the serial number of the product.  This is only used if the
       * is_licensed parameter is set to zero.
       *
       * @param is_licensed Set to non-zero if the product is licensed and its product key can be
       * found in the registry associated with the app_name.
       *
       * @return Returns a handle to the product license client.  Returns a null pointer if the
       * connection could not be established.
       */
      typedef void const * (__stdcall initialise_function_type)(
         char const *app_name,
         char const *model,
         char const *version,
         char const *serial_no,
         int is_licensed);
      initialise_function_type *initialise;

      /**
       * Pointer to the DLL cslicense_close() function. This will close a license client within the
       * DLL that was created earlier with initialise().
       *
       * @param server_handle Specifies the handle that was returned by initialise().
       */
      typedef void (__stdcall close_function_type)(
         void const *server_handle);
      close_function_type *close;

      /**
       * Pointer to the DLL cslicense_update() function.  This will initiate the process of
       * downloading a product update patch to the specified file name.
       *
       * @param server_handle Specifies the handle for the license client created previously by
       * calling initialise().
       *
       * @param version_json Specifies a JSON formatted string that describes the version to
       * retrieve.
       *
       * @param file_name Specifies the path and name of the file to store the retrieved update
       * patch image.
       *
       * @return Returns a handle to the update object.  This can be used to get the update status
       * and to close the update.  Will return a null pointer if the update could not be started.
       */
      typedef void const * (__stdcall update_function_type)(
         void const *server_handle,
         char const *version_json,
         char const *file_name);
      update_function_type *update;

      /**
       * Pointer to the DLL cslicense_check_update function.  This will checks the status of an
       * update in progress.
       *
       * @param update_handle Specifies the update handle that was returned from a previous call to
       * update().
       *
       * @param outcome Specifies a pointer to which a code that represents the current status of
       * the update will be written.   This value should be one of those values defined in the
       * update_status_type enumeration.
       *
       * @param file_size Specifies a pointer to which the size in bytes of the file being retrieved
       * will be written.
       *
       * @param bytes_received Specifies a pointer to which the number of bytes received has been
       * written. 
       */
      typedef void (__stdcall check_update_function_type)(
         void const *update_handle,
         int *outcome,
         int8 *file_size,
         int8 *bytes_received);
      check_update_function_type *check_update;

      /**
       * Pointer to the DLL cslicense_close_update() function.  This will close an update that was
       * started earlier by a call to update().
       *
       * @param update_handle Specifies the update handle that was returned earlier by a call to
       * update().
       */
      typedef void (__stdcall close_update_function_type)(
         void const *update_handle);
      close_update_function_type *close_update;

      /**
       * Pointer to the DLL cslicense_get_changelog() function.  This will start the process of
       * retrieving the change log for a specified version.
       *
       * @return Returns a handle to the changelog operation.  This handle will be used in future
       * calls to check_get_changelog() and close_get_changelog().
       *
       * @param server_handle Specifies the handle that was returned from a previous call to
       * cslicense_initialise().
       *
       * @param version Specifies the version number for the application to return.
       */
      typedef void const * (__stdcall get_changelog_function_type)(
         void const *server_handle,
         char const *version);
      get_changelog_function_type *get_changelog;

      /**
       * Pointer to the DLL cslicense_check_get_changelog() function.  This function checks an
       * operation in progress and returns operation status through the parameters.
       *
       * @param changelog_handle Reference to an operation started through a call to
       * get_changelog().
       *
       * @param outcome Pointer to a value that will return the current status of the operation.
       * This value will be -1 if the operation is in progress, -2 if the operation does not exist,
       * or greater than or less than zero to describe the outcome of the operation.
       *
       * @param content Specifies a string pointer that will be assigned by the DLL.  This will be
       * an empty string unless the value of outcome is one.  If the value of outcome is one, thbis
       * string will contain the formatted JSON structure for the change log.
       *
       * @param content_len Specifies a pointer to which the length of the content will be written.
       */
      typedef void (__stdcall check_get_changelog_function_type)(
         void const *changelog_handle,
         int *outcome,
         char const **content,
         size_t *content_len);
      check_get_changelog_function_type *check_get_changelog;

      /**
       * Pointer to the DLL cslicense_close_get_changelog() function.  This function will release
       * the resources associated with the given handle.
       *
       * @param changelog_handle Specifies the handle that was created by a call to
       * get_changelog().
       */
      typedef void (__stdcall close_get_changelog_function_type)(
         void const *changelog_handle);
      close_get_changelog_function_type *close_get_changelog;

      /**
       * Pointer to the DLL cslicense_get_versions() function.  This function will start an
       * operartion to get an array of JSON version descriptions for the product.
       *
       * @param server_handle Specifies the license client connection that was created by a call to
       * initialise().
       */
      typedef void const * (__stdcall get_versions_function_type)(
         void const *server_handle);
      get_versions_function_type *get_versions;

      /**
       * Pointer to the DLL cslicense_get_highest_version() function. This function will start an
       * operation to get the JSON description for the product's highest version.
       *
       * @param server_handle Specifies the opaque handle that was returned by cslicense_initialise().
       * 
       * @param include_beta Specifies if products without a release date should be considered.
       */
      typedef void const* (__stdcall get_highest_version_type)(
         void const* server_handle,
         int include_beta);
      get_highest_version_type* get_highest_version;

      /**
       * Pointer to the DLL cslicensae_check_get_versions() function.  This function will allow the
       * application to check the status of an operation that is underway.
       *
       * @param get_versions_handle Specifies the handle that was opened with a call to
       * get_versions().
       *
       * @param outcome Pointer to an outcome or status code for the operation.  If this value is
       * returned as -1, the operation is still underway.  If -2, there get_versions_handle argument
       * is invalid.  If greater than or equal to zero, the value will report the operation
       * outcome.
       *
       * @param content Pointer to a string that is set by the DLL when the operation has
       * successfully concluded.  When the value of outcome is one, this string will be formatted
       * JSON that presents a collection of version reports.
       *
       * @param content_len Pointer to an integer to which the length of the content buffer will be
       * written.
       */
      typedef void (__stdcall check_get_versions_function_type)(
         void const *get_versions_handle,
         int *outcome,
         char const **content,
         size_t *content_len);
      check_get_versions_function_type *check_get_versions;

      /**
       * Pointer to the DLL function cslicense_close_get_versions().  This function releases the
       * resources associated with the specified handle.
       *
       * @param get_versions_handle Specifies a handle that was opened by a previous call to
       * get_versions().
       */
      typedef void (__stdcall close_get_versions_function_type)(
         void const *get_versions_handle);
      close_get_versions_function_type *close_get_versions;
   
   private:
      /**
       * Specifies the module handle for the DLL.
       */
      HMODULE dll_handle;
   };
};


#endif
