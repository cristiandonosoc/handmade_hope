/*  =====================================================================
    $File: utilsstring.cpp
    $Creation Date: 2015-06-17
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */
#ifndef _UTILS_STRINGS_CPP

namespace Utils
{
namespace String
{

  /**
  * Concatenates two strings by creating a new char[]
  * in the free-store. Remember to free afterwards!
  */
  inline char*
  CatStrings(char* sourceA, char* sourceB)
  {
    int sizeA = sizeof(sourceA);
    int sizeB = sizeof(sourceB);
    char* resString = (char*)malloc(sizeA + sizeB);
    char* scan = resString;
    for(char* scanA = sourceA;
        *scanA;
        *(scan++) = *(scanA++));

    for(char* scanB = sourceB;
        *scanB;
        *(scan++) = *(scanB++));

    // We terminate the string
    *scan++ = 0;

    return resString;
  }

  inline char*
  CopyString(char* source)
  {
    char* res = (char*)malloc(sizeof(source));
    for(;
        *res;
        *res++=*source++);
    return res;
  }

  inline char*
  ScanForLastCharacter(char* source, char target)
  {
    char* res = source;
    for(;
        *source; // We check for the NULL-terminated string
        ++source)
    {
      if(*source == '\\')
      {
        res = source + 1;
      }
    }
    return res;
  }

}
}

#define _UTILS_STRINGS_CPP
#endif
