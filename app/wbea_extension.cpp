// Copyright (c) 2010 The WBEA Authors. All rights reserved. Use of this source
// code is governed by a BSD-style license that can be found in the LICENSE
// file.

#include "wbea_extension.h"
#include "wbea.h"
#ifdef _WIN32
#include "resource.h"
#endif

// Implementation of the V8 handler class for the "app" extension.
class WbeaV8ExtensionHandler : public CefThreadSafeBase<CefV8Handler>
{
public:
  WbeaV8ExtensionHandler() {}
  virtual ~WbeaV8ExtensionHandler() {}

  // Execute with the specified argument list and return value.  Return true if
  // the method was handled.
  virtual bool Execute(const std::wstring& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       std::wstring& exception)
  {
#ifdef _WIN32
    UINT command_id = 0;
    if(name == L"find")
      command_id = ID_FIND;
    else if(name == L"print")
      command_id = ID_PRINT;
    else if(name == L"about")
      command_id = IDM_ABOUT;
    else if(name == L"exit")
      command_id = IDM_EXIT;
    
    if(command_id != 0) {
      PostMessage(AppGetMainHwnd(), WM_COMMAND, command_id, 0);
      return true;
    }
#endif // _WIN32
    return false;
  }
};


void InitWbeaExtension()
{
  // Register a V8 extension with the below JavaScript code that calls native
  // methods implemented in WbeaV8ExtensionHandler.
  std::wstring code = L"var app;"
    L"if (!app)"
    L"  app = {};"
    L"(function() {"
    L"  app.find = function() {"
    L"    native function find();"
    L"    find();"
    L"  };"
    L"  app.print = function() {"
    L"    native function print();"
    L"    print();"
    L"  };"
    L"  app.about = function() {"
    L"    native function about();"
    L"    about();"
    L"  };"
    L"  app.exit = function() {"
    L"    native function exit();"
    L"    exit();"
    L"  };"
    L"})();";
  CefRegisterExtension(L"v8/app", code, new WbeaV8ExtensionHandler());
}
