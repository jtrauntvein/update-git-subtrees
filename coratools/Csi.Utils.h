/* Csi.Utils.h

   Copyright (C) 2001, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 13 July 2001
   Last Change: Wednesday 02 October 2019
   Last Commit: $Date: 2019-10-02 17:21:24 -0600 (Wed, 02 Oct 2019) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Utils_h
#define Csi_Utils_h

#include <stdio.h>
#include "CsiTypeDefs.h"
#include "Csi.LgrDate.h"
#include "Csi.OsException.h"
#include "Csi.ByteOrder.h"
#include "Csi.FloatUtils.h"
#include "StrAsc.h"
#include "StrBin.h"


namespace Csi
{
   /**
    * Calculates the check sum for the string of length strLen.  The sum is
    * calculated by adding the characters in sequence using a thirteen bit word
    * size.
    */
   uint2 calcCheckSumFor(void const *buff, uint4 len);

   /**
    * Calculates the CRC value for the data based upon the previous value of
    * seed. A seed of zero should be used to start a new calculation.
    */
   uint4 crc32(void const *buff, uint4 len, uint4 seed = 0);

   /**
    * Calculates the CRC value for the data similarly to the crc32 function. This function differs from
    * crc32 in that the crc value is calculated in reverse order (starting with the last value in the
    * buffer and iterating toward the first).
    */
   uint4 crc32r(void const *buff, uint4 len, uint4 seed = 0);

   /**
    * Calculates the CRC value for the data based upon the previous value of
    * the seed.  A value of zero should be used to start a new calculation. 
    */
   uint2 crc16(void const *buff, uint4 buff_len, uint2 seed = 0);

   /**
    * @return Returns the checksum of the specified block of data using the specified mask.
    *
    * @param buff Specifies the beginning of the block of data.
    *
    * @param buff_len Specifies the number of bytes to process.
    *
    * @param mask Specifies the mask for the checksum.
    */
   uint4 checksum(void const *buff, uint4 buff_len, uint4 mask);
   

   /**
    * Calculates the 32 bit CRC value for the open file.
    */
   uint4 crcForFile(FILE *in);

   /**
    * Performs the same operation as fread only it checks the return value and
    * throws an OsException if the return value is unsuccessful
    */
   void efread(void *buffer, size_t size, size_t cnt, FILE *in);

   /**
    * Performs the same operation as fwrite only it checks the return value and
    * throws an OsException on failure
    */
   void efwrite(void const *buffer, size_t size, size_t cnt, FILE *out);

   /**
    * Traverses a path name creating the all of the directories in that path that
    * do not exist.  Throws an OsException if any of these operations fail.
    */
   void createNestedDir(char const *path);

   /**
    * Attempts to delete all of the files in the named directory and then
    * remove the directory if purge_dir is true. Returns true if all files and
    * the directory were deleted or false if there were remnants
    */
   bool purgeDirectory(char const *dir_name, bool purge_dir = true);

   /**
    * Calculate the CSI signature of a block of bytes. The default value for the
    * seed parameter is the defined initial seed for the CSI signature algorithm.
    */
   uint2 calcSigFor(void const *buff, size_t len, uint2 seed = 0xAAAA);

   /**
    * Calculates the signature of the contents of the open file handle from
    * current position up to len bytes from the current position (if len is
    * specified as less than zero, the signature will be calculated up to the
    * end of the file).
    */
   uint2 calc_file_sig(FILE *in, int8 len = -1, uint2 seed = 0xAAAA);

   /**
    * Calculates the nullifier for the specified signature and returns it in a uint2. In order for
    * the nullifier to work within a packet, it must be written most significant byte first (note
    * that the Intel binary representation for a uint2 is least significant byte first).
    */
   uint2 calcSigNullifier(uint2 sig);

   /**
    * adjusts a time stamp by plus or minus one day based upon the relative hour
    * value of two clocks.
    */
   void adjustForMidnight(LgrDate &stamp, int hour1, int hour2);

   /**
    * Validates the signature of a block of data assuming that the signature for
    * the block is written at the last two bytes of the block
    */
   bool validateSig(void const *buff, uint4 len);

   /**
    * Uses the system clock to simulate a free-running counter. Counts time in
    * units of milli-seconds.  The return value represents the difference between
    * the parameter and the current value of the counter.
    */
   uint4 counter(uint4 cnt);

   /**
    * Uses the system clock to return the time of day.
    */
   uint4 timeOfDay();

   /**
    * Returns the amount of free space available on the disc indicated by the path
    */
   int8 getDiscFreeSpace(char const *path);

   /**
    * Converts a long file name to a file name that conforms to the 8.3 file name rules. The input file
    * name is expected to be the file name only with no path or device information associated with
    * it. The source string is assumed to be null terminated.
    */
   char const *longFileNameToShort(StrAsc &dest, char const *source);

   /**
    * Attempts to make a backup of the file specified by base to a new unique file name.
    *
    * @param base_name Specifies the name and path of the file to back up.
    *
    * @param backup_extension Specifies the extension that should be applied to the backup file.
    *
    * @param make_unique Set to true if the backup name should be manged until no existing file will
    * be overwritten.
    *
    * @param use_rename Set to true if the file is to be backed up using a rename operation rather than a
    * copy.
    *
    * @return Returns true if the operation succeeded.
    */
   bool backup_file(
      char const *base_name,
      char const *backup_extension,
      bool make_unique = false,
      bool use_rename = false);

   /**
    * Copies the file named by source into the file and path named by
    * destination. Returns true if the file could be copied.
    *
    * @param destination Specifies the path and file name for the new file.
    *
    * @param source Specifies the path and file name for the source file.
    *
    * @return Returns true if the operation suceeded.
    */
   bool copy_file(char const *destination, char const *source);

   /**
    * Moves and/or renames the file specified by old_name to the location and
    * name speciifed by new_name.
    *
    * @param old_name Specifies the name of the file to be moved or copied.
    *
    * @param new_name Specifies the new path of file name for the file.
    *
    * @return Returns true if the operation succeeded.
    */
   bool move_file(char const *old_name, char const *new_name);

   /**
    * Deletes the file specified by file_name.
    */
   int remove_file(char const *file_name);

   /**
    * Splits the provided path into parent_path and file name components.
    *
    * @param parent_path Pointer to the buffer that will receive the directory for the file name.
    * If this is null, the parent path will not be returned.
    *
    * @param name Pointer to the buffer to which the file name will eb written.  If the path ends
    * with a directory separator, this will be an empty string.  If this parameter is null, no file
    * name will be returned.
    *
    * @param path Specifies the path and file name to be split.  The follwing syntax is supported:
    *
    *   file-path    := [ root-spec ] [ dir-name separator ] file-name.
    *   root-spec    := windows-root | unc-root | unix-root.
    *   dir-name     := string.
    *   separator    := "/" | "\\".
    *   windows-root := drive-letter ":\\".
    *   unc-root     := "\\\\" machine-name "\\" service [ "\\" ].
    *   unix-root    := "/" | "~/".
    *
    * The application can provide null pointers for parent_path or name.  The
    * function will write to the parameters that are provided.  
   */
   void split_path(
      StrAsc *parent_path,
      StrAsc *name,
      StrAsc const &path);
   
   /**
    * Extracts the path portion of a file name. Places the path in the dest parameter.  If there is
    * no path component to the provided string, the return value (and the dest parameter) will be an
    * empty string.
    *
    * @param dest Specifies the buffer to which the path will be written.
    *
    * @param file_name Specifies 
    */
   char const *get_path_from_file_name(StrAsc &dest, char const *file_name);

   /**
    * Extracts the name portion of the a file path and places it in the dest
    * parameter.
    *
    * @param dest Specifies the buffer to which the file name will be written.
    *
    * @param file_path Specifies the path for the file.
    *
    * @return Returns the start of the dest buffer.
    */
   char const *get_name_from_file_path(StrAsc &dest, char const *file_path);

   /**
    * Checks the specified file name path to see if it exists. If the path does not exist, this
    * function will attempt to create the path using createNestedDir(). An OsException object will
    * be thrown if the path cannot be created.
    *
    * @param file_name Specifies the name and path of the file to check.
    */
   void check_file_name_path(char const *file_name);

   /**
    * @return Returns true if the specified file exists.
    */
   bool file_exists(char const *file_name);

   /**
    * Deletes the specified file identified by the name.
    *
    * @param file_name Specifies the path and name of the file to be deleted.
    *
    * @return Returns true if the specified file is deleted
    */
   bool delete_file(char const *file_name);

   /**
    * Generates a file name for the specified directory that is unique at the
    * time of the call.  The name will be returned in the file_name argument.
    * If a unique name cannot be generated, an object derived from
    * std::exception will be thrown.
    *
    * @param directory Specifies the directory for the temporary file.
    *
    * @param file_name Specifies the buffer to which the file name will be written. 
    *
    * @return The return value will point to the beginning of the file_name parameter.
    */
   char const *make_temp_file_name(
      char const *directory,
      StrAsc &file_name);
   
   /**
    * Creates a temporary file in the specified directory and returns a handle
    * to the open file.  The name and path of the file will be returned in the
    * file_name argument.  If the file can not be created, an object derived
    * from std::exception will be thrown.
    *
    * @return Returns the handle to the termporary file.
    *
    * @param file_name Specifies the buffer to which the temporary file name will be written.
    *
    * @param open_mode Specifies the mode used to open the file.
    */
   FILE *make_temp_file(
      char const *directory,
      StrAsc &file_name,
      char const *open_mode = "w+b");

   /**
    * @return Evaluates whether the specified file name is one of the "reserved names" under
    * windows.  Under posix, this function will be a no-op (will always return false).
    */
   bool is_reserved_file_name(char const *file_name);

   /**
    * Reverses the byte order of the specified buffer
    *
    * @param buffer Specifies the start of the buffer to be reversed.
    *
    * @param buffer_len Specifies the length of the buffer to be reversed.
    */
   void reverse_byte_order(void *buffer, size_t buffer_len);

   /**
    * @return Returns the directory from which the current process was loaded.
    *
    * @param dir Specifies the buffer to which the directory name will be written.
    */
   char const *get_app_dir(StrAsc &dir);

   /**
    * @return Returns the working directory for the specified application name and version using
    * Roy's overly compiicated registry treasure hunt.
    *
    * @param dir Specifies the buffer to which the working directory will be written.
    *
    * @param app_name Specifies the name of the application or product.
    *
    * @param version Specifies the version of the application or product.
    */
   char const *get_work_dir(
      StrAsc &dir, 
      StrAsc const &app_name,
      StrAsc const &version = "1.0");

   /**
    * @return Returns the application's directory in the user's home directory.
    *
    * @param app_name Specifies the name of the application.
    */
   StrAsc get_app_home_dir(StrAsc const &app_name);

   /**
    * @return Returns the name and path of the current program executable file in one string.
    *
    * @param dest Specifies the buffer to which the program path will be written.
    */
   char const *get_program_path(StrAsc &dest);


   /**
    * @return Returns the application's current working directory.
    */
   StrAsc get_system_working_dir();

   /**
    * @return Obtains from the operating system the amount of available virtual memory.
    */
   int8 get_available_virtual_memory();

   /**
    */
   uint4 file_length(FILE *f);
   
   /**
    * @return Returns the length of the file referenced by the specified open handle. 
    */
   int8 long_file_length(FILE *f);

   /**
    * Portable function that performs file seeking using file pointers that can be larger than 32
    * bits.
    *
    * @param file Specifies the file handle in which to seek.
    *
    * @param offset Specifies the offset to seek.
    *
    * @param where Specifies the code at which the seek should take place.  This should be one of
    * SEEK_SET, SEEK_CUR, or SEEK_END.
    *
    * @return The return code should be evaluated the same as that returned by fseek()
    */
   int file_seek(FILE *file, int8 offset, int where);

   /**
    * @return Return Returns the current position in the file as a 64 bit integer.
    *
    * @param file Specifies the file to evaluate.
    */
   int8 file_tell(FILE *file);

   /**
    * 
    * Locates the specified sub-string pattern within the provided buffer.
    *
    * @param buff Specifies the buffer in which to search.
    *
    * @param buff_len Specifies the number of bytes in the buffer.
    *
    * @param pattern Specifies the pattern to search.
    *
    * @param pattern_len Specifies the length of the pattern in bytes.
    *
    * @return If the sub-string was located, the offset of its first element will be returned.
    * Otherwise, the return value will be greater than or equal to the length of the buffer.
    */
   size_t locate_sub_string(
      void const *buff,
      size_t buff_len,
      void const *pattern,
      size_t pattern_len);

   /**
    * @return Returns the application command line.  Under windows, this value depends on
    * set_command_line having been called first.
    */
   char const *get_command_line();

   /**
    * Sets the command line vector.  Under windows, this is no-op.
    */
   void set_command_line(
      int argc,
      char const *argv[]);

   /**
    * @return Returns true if the supplied string matches the supplied wildcard
    * expression.
    *
    * @param str Specifies the string to test.
    *
    * @param expr The expression will support '*' and '?' to match zero or more characters or
    * exactly one character.  The comparisons will be case insensitive.
   */
   bool matches_wildcard_expr(
      StrAsc const &str,
      StrAsc const &expr);

   /**
    * @return Returns the data file type code for the specified file or file name.
    *
    * @param file_name Specifies the name of the file to test.
    *
    * @param file Specifies a file that has already been opened that needs to be tested.
    *
    * @param append_offset Specifies a pointer to a file offset that, if not null, will receive the
    * position in the file where data can be appended.
    */
   enum data_file_types
   {
      data_file_type_error = -1,
      data_file_type_unknown = 0,
      data_file_type_toaci1 = 1,
      data_file_type_toa5 = 2,
      data_file_type_tob1 = 3,
      data_file_type_tob2 = 4,
      data_file_type_tob3 = 5,
      data_file_type_csixml = 6,
      data_file_type_mixed_csv = 7,
      data_file_type_mixed_printable = 8,
      data_file_type_mixed_binary = 9,
      data_file_type_toa6 = 10
   };
   data_file_types get_data_file_type(
      char const *file_name,
      int8 *append_offset = 0);
   data_file_types get_data_file_type(
      FILE *file,
      int8 *append_offset);


   /**
    * @return Returns a string that represents the name for the specified data file type code.
    *
    * @param file_type_code Specifies the file type.
    */
   char const *data_file_type_name(int file_type_code);

   /**
    * @return Returns true of the specified string represents a valid time stamp as would be written
    * to a TOA5 or TOACI1 file format.
    *
    * @param s Specifies the string to test.
    */
   bool is_toa_time(char const *s);


   /**
    *
    * Extracts the header portion of a csixml file into the destination buffer.
    * Will throw an exception if the expected syntax markers are not found. 
    */
   void extract_csi_xml_header(
      StrAsc &dest,
      FILE *in);


   /**
    * Evaluates whether data can be appended to the specified data file given
    * a target type and header.
    *
    * @param file_name Specifies the name of the file to test.
    *
    * @param target_type Specifies the expected data format of the the data file.
    *
    * @param target_header Specifies the expected header for the data file.
    *
    * @param target_header_len Specifies the length of the expected header in bytes.
    *
    * @param append_offset Specifies a pointer to a file offset that, if not null, will receive the
    * position where data can be appended to the file.
    *
    * @parasm reason Specifies a pointer to a string that, if not null, will receive the explanation
    * of why the file cannot be appended.
    *
    * @param max_size Optionally specifies the maximum allowed size of the data file.
    *
    * @return The return value will be greater than zero if appending is considered to be possible
    * and will be less than or equal to zero otherwise.
   */
   enum can_append_codes
   {
      can_append_success = 1,
      can_append_with_warnings = 2,
      can_append_with_severe_warnings = 3,
      can_append_unknown_error = 0,
      can_append_open_failure = -1,
      can_append_incompatible_types = -2,
      can_append_incompatible_headers = -3,
      can_append_not_supported = -4,
      can_append_too_large = -5
   };
   int data_file_can_append(
      char const *file_name,
      int target_type,
      char const *target_header,
      uint4 target_header_len,
      int8 *append_offset = 0,
      StrAsc *reason = 0,
      int8 max_size = -1);
   

   /**
    * Searches in reverse for the specified pattern in the file.  If the
    * pattern located, the file will be positioned at the beginning of that
    * pattern and the offset returned.  If the pattern is not found, the file
    * offset will be set at the end of the file.
    *
    * @param f Specifies the file handle to search.  The search will take place from that file
    * handles current position.
    *
    * @param pattern Specifies the pattern to search for.
    *
    * @param pattern_len Specifies the length of the pattern in bytes.
    *
    * @return Returns the position in the file where the pattern was found or zero if the pattern
    * was not found.
    */
   int8 search_file_backward(
      FILE *f,
      void const *pattern,
      uint4 pattern_len);


   /**
    * Sets the time stamp of last modification as well as creation for the
    * specified file name.   
    *
    * @param file_name Specifies the name of the file to adjust.
    *
    * @param stamp Specifies the time stamp to assign to the file's last modified date.
    *
    * @return Returns true if the file's statmp was successfully modified.
    */
   bool set_file_time(char const *file_name, Csi::LgrDate const &stamp);


   /**
    * @return Returns true if the specified application ane and version has been installed as a
    * demo.
    *
    * @param app_name Specifies the name of the application to test.
    *
    * @param version Specifies the version of the application to test.
    */
   bool is_demo_app(
      StrAsc const &app_name,
      StrAsc const &version);

   
   /**
    * @return Returns true if the application can be run as a demo.
    *
    * @param demo_msg Specifies the message, if any that explains why the demo can't run.
    *
    * @param app_name Specifies the name of the application to test.
    *
    * @param version Specifies the version of the application to test.
    *
    * @return is_product Set to true if the application name specifies the model  number for a
    * product. 
    */
   bool can_demo_run(
      StrAsc &demo_msg,
      StrAsc const &app_name,
      StrAsc const &version,
      bool is_product);

   /**
    * Applies the signature encryption algorithm to the supplied data buffer.
    *
    * @param output Specifies the buffer to which the encrypted data will be written.
    *
    * @param data Specifies the data fragment to be encrypted.
    *
    * @param data_len Specifies the length of the data fragment to be encrypted.
    *
    * @param seed Specifies the initialisation vector for the encryption algorithm.  If the
    * application is encrypting a large block with multiple passes, this value should be equal to
    * the return value for the previous block.
    *
    * @return Returns the initialisation vector state at the end of encrypting.
    */
   uint2 encrypt_sig(
      StrBin &output, void const *data, uint4 data_len, uint2 seed = 0xAAAA);

   /**
    * Appies the signature encryption algorithm in reverse.
    *
    * @return Returns the last key used.  This must be specified as seed if encrypting a sequence in
    * multiple parts.
    *
    * @param output Specifies the buffer to which the decrypted content will be written.
    *
    * @param data Specifies the start of the encrypted data.
    *
    * @param data_len Specifies the number of bytes in the data buffer.
    *
    * @param seed Specifies the initialisation vector for the encryption algorithm.
    */
   inline uint2 decrypt_sig(
      StrBin &output, void const *data, uint4 data_len, uint2 seed = 0xAAAA)
   { return encrypt_sig(output, data, data_len, seed); }

   /**
    * @return Returns true if the specified string can serve as a valid WEP password.
    *
    * @param s Specifies the string to test.
    */
   bool valid_wep_password(StrAsc const &s);

   /**
    * This function parses the host address string from a URI into the address
    * and port portions.
    *
    * @param address_portion Specifies the string to which the address portion will be written.
    *
    * @param port_portion Specifies the variable reference that will receive the TCP port
    * assignment.
    *
    * @param address Specifies the address uri to convert. The syntax supported is as follows:
    * address = [ "[" ] address-portion [ "]" ] [ ":" port.
    */
   void parse_uri_address(
      StrAsc &address_portion,
      uint2 &port_portion,
      StrAsc const &address);

   /**
    * Converts the specified unicode entity to a UTF-8 sequence.
    *
    * @param dest Specifies the stirng to which the utf-8 encoding will be written.
    *
    * @param entity Specifies the unicode entity code to convert.
    */
   void unicode_to_utf8(StrAsc &dest, uint4 entity);

   /**
    * @return Returns the file handle associated with the specified name and mode.
    *
    * @param file_name Specifies the file name encoded as either UTF-8 or multi-byte.
    *
    * @param mode Specifies the file open mode.
    */
   FILE *open_file(char const *file_name, char const *mode);

   /**
    * @return Returns a file handle for the specified file name and mode.
    *
    * @param file_name Specifies the file name encoded as a unicode string.
    *
    * @param mode Specifies the file open mode.
    */
   FILE *open_file(StrUni const &file_name, StrUni const &mode);

   /**
    * @return Returns true if this process is a 32 bit process running on a 64 bit platform.
    */
   bool is_wow64();

   /**
    * @return Returns a GUID string representation.
    */
   StrAsc make_guid();

   /**
    * @return Attempts to resolve the value for mime type (used by HTTP Content-Type) based upon the
    * file name extension.
    *
    * @param file_name Specifies the name of the file to evaluate.
    */
   typedef std::pair<StrAsc, StrAsc> content_encoding_type;
   content_encoding_type resolve_content_type(StrAsc const &file_name);

   /**
    * @return Returns the file extension associated with the specified content type string or an
    * empty string if the content type is unknown.
    *
    * @param content_type Specifies the content type to resolve.
    */
   StrAsc content_type_ext(StrAsc const &content_type);
};


#endif
