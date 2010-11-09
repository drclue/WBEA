// Copyright (c) 2009 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef _RESOURCE_UTIL
#define _RESOURCE_UTIL

#include "include/cef.h"

#ifdef _WIN32

// Load a resource of type BINARY
bool LoadBinaryResource(int binaryId, DWORD &dwSize, LPBYTE &pBytes);

#endif // _WIN32

// Returns the suggested mime type for the specified file extension.
std::wstring GetSuggestedMimeType(const std::wstring& fileExt);

#endif // _RESOURCE_UTIL
