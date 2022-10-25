#ifndef __NXVERSION_H__
#define __NXVERSION_H__



#include "nxversionnum.h"
#if !defined(VERSION_MAJOR) || !defined(VERSION_MINOR) || !defined(BUILD_NUMBER)
#error "ERROR: version numbers are not defined by nxversionnum.h!"
#endif



#define VER_FILEVERSION                                 \
  VERSION_MAJOR,VERSION_MINOR,BUILD_NUMBER
#define VER_PRODUCTVERSION                              \
  VERSION_MAJOR,VERSION_MINOR,BUILD_NUMBER



#define VERSION_STR_BASE(maj,min,build)                 \
  #maj "." #min "." #build
#define VERSION_STR(maj,min,build)                      \
  VERSION_STR_BASE(maj,min,build)

#define VER_FILEVERSION_STR                             \
  VERSION_STR(VERSION_MAJOR,VERSION_MINOR,BUILD_NUMBER)
#define VER_PRODUCTVERSION_STR                          \
  VERSION_STR(VERSION_MAJOR,VERSION_MINOR,BUILD_NUMBER)



#define VER_LEGALCOPYRIGHT_STR                          \
  "Copyright (C) 2014-2018 NextLabs, Inc.  All rights reserved."



#endif  // __NXVERSION_H__
