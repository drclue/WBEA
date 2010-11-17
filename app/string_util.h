// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef _CEFCLIENT_STRING_UTIL_H
#define _CEFCLIENT_STRING_UTIL_H

#include "include/cef.h"
#include <string>


// Convert a std::string to a std::wstring
std::wstring StringToWString(const std::string& s);

// Convert a std::wstring to a std::string
std::string WStringToString(const std::wstring& s);

#endif // _CEFCLIENT_STRING_UTIL_H
