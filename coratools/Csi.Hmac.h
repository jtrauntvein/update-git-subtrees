/* $HeadURL: svn://engsoft/cora/coratools/Csi.Hmac.h $

Copyright (C) 2014, Campbell Scientific, Inc.
Started On: Monday, May 19, 2014

Started By: Tyler Mecham
$LastChangedBy: tmecham $
$LastChangedDate: 2014-05-19 14:03:25 -0600 (Mon, 19 May 2014) $
$LastChangedRevision: 21747 $
*/


#pragma once
#ifndef Csi_Hmac_h
#define Csi_Hmac_h

#include "Csi.LgrDate.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // csi_hmac_md5
   ////////////////////////////////////////////////////////////
   void csi_hmac_md5(
      StrAsc const &msg,
      StrAsc const &key,
      unsigned char *digest);

   ////////////////////////////////////////////////////////////
   // csi_hmac_sha1
   ////////////////////////////////////////////////////////////
   void csi_hmac_sha1(
      StrAsc const &msg,
      StrAsc const &key,
      unsigned char *digest);
};


#endif
