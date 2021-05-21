/* LoggerNetBuild.h

   Copyright (C) 2008, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 22 September 2008
   Last Change: Friday 05 February 2021
   Last Commit: $Date: 2021-02-05 07:54:34 -0700 (Fri, 05 Feb 2021) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef LoggerNetBuild_h
#define LoggerNetBuild_h

#define LoggerNet_name "LoggerNet"
#define LoggerNet_version "4.7"
#define LoggerNet_BuildSpAr 1
#if LoggerNet_BuildSpAr == 1
#define LoggerNet_Beta " Beta"
#define LoggerNet_BetaW L" Beta"
#else
#define LoggerNet_Beta ""
#define LoggerNet_BetaW L""
#endif
#define CORASTRINGS_VERSION "1.40"
#define WXTOOLS_VERSION "2.40"


#endif
