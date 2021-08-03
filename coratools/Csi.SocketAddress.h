/* Csi.SocketAddress.h

   Copyright (C) 2012, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 18 June 2012
   Last Change: Saturday 09 March 2019
   Last Commit: $Date: 2019-03-11 11:39:36 -0600 (Mon, 11 Mar 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SocketAddress_h
#define Csi_SocketAddress_h

#include "StrAsc.h"
#include "CsiTypeDefs.h"
#include <list>
#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netdb.h>
#endif


namespace Csi
{
   /**
    * Defines an object that describes an IP address and a port. It has the capbility of parsing
    * IPv4 and IPv6 addresses as well as resolving domain names to one or more IPv6 or IPv4
    * addresses.
    */
   class SocketAddress
   {
   public:
      /**
       * Default constructor
       */
      SocketAddress():
         address_len(0)
      {
         memset(&storage, 0, sizeof(storage));
      }

      /**
       * String constructor
       *
       * @param s Specifies an IPv4 or IPv6 address.
       *
       * @param port Specifies a port number.
       */
      SocketAddress(char const *s, uint2 port = 0);

      /**
       * Copy constructor
       *
       * @param other Specifies the address to copy.
       */
      SocketAddress(SocketAddress const &other):
         storage(other.storage),
         address_len(other.address_len)
      { }

      /**
       * Construct from a socket address structure.
       *
       * @param addr Specifies the address structure.
       *
       * @param addr_len Specifies the length of the address.
       */
      SocketAddress(struct sockaddr const *addr, uint4 addr_len):
         address_len(addr_len)
      {
         memset(&storage, 0, sizeof(storage));
         memcpy(&storage, addr, addr_len);
      }

      /**
       * @return Returns the family for the address represented by this object.
       */
      enum family_type
      {
         family_ipv4 = AF_INET,
         family_ipv6 = AF_INET6
      };
      family_type get_family() const
      { return static_cast<family_type>(storage.ss_family); }

      /**
       * @param family Specifies the family for this address.
       */
      void set_family(family_type family)
      { storage.ss_family = family; }

      /**
       * @return Returns the formatted address.
       */
      StrAsc get_address() const;

      /**
       * @return Returns the address structure.
       */
      sockaddr const *get_storage() const
      { return reinterpret_cast<sockaddr const *>(&storage); }
      sockaddr *get_storage()
      { return reinterpret_cast<sockaddr *>(&storage); }

      /**
       * @return Returns the length of the address structure.
       */
      uint4 get_address_len() const
      { return address_len; }

      /**
       * @return Returns the port from the address structure.
       */
      uint2 get_port() const;

      /**
       * @param port Specifies the port for the address structure.
       */
      void set_port(uint2 port);

      /**
       * Performs a lookup or conversiion for the specified address and port.  Returns the set of
       * resolved addresses in the addresses parameter.
       *
       * @param addresses Specifies a reference to the container of returned addresses.
       *
       * @param address Specifies the IP address or domain name.
       *
       * @param port Specifies the port.
       *
       * @param passive Set to true if the passive flag should be used when resolving.
       *
       * @param numeric Set to true if the address is expected to be numeric.
       */
      typedef std::list<SocketAddress> addresses_type;
      static void resolve(
         addresses_type &addresses,
         char const *address = 0,
         uint2 port = 0,
         bool passive = false,
         bool numeric = false);

      /**
       * Resets this address to a null structure.
       */
      void clear();

      /**
       * @return Returns a code that identifies the rssults of validating the address string.
       *
       * @param address Specifies the address string to parse.
       */
      enum validate_result_type
      {
         validate_empty,
         validate_ipv4_complete,
         validate_ipv4_incomplete,
         validate_ipv6_complete,
         validate_ipv6_incomplete,
         validate_error
      };
      static validate_result_type validate(char const *address);

      /**
       * Formats the validation result to the specified stream.
       *
       * @param result Specifies the validation result code.
       *
       * @param out Specifies the stream to which the description will be written.
       */
      static void format_validate_result(std::ostream &out, validate_result_type result);
      
      bool operator ==(SocketAddress const &other) const;
      bool operator !=(SocketAddress const &other) const
      { return !(*this == other); }
      bool operator <(SocketAddress const &other) const;
      bool operator <=(SocketAddress const &other) const
      { return (*this == other) || (*this < other); }
      bool operator >(SocketAddress const &other) const
      { return (*this != other) && !(*this < other); }
      bool operator >=(SocketAddress const &other) const
      { return !(*this < other); }
      
   private:
      /**
       * Specifies the raw storage for this address.
       */
      struct sockaddr_storage storage;

      /**
       * specifies the length of the address structure.
       */
      uint4 address_len;
   };


   std::ostream &operator <<(std::ostream &out, SocketAddress const &address);
};


#endif
