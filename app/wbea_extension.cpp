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
  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception)
  {
#ifdef _WIN32
    UINT command_id = 0;
    if(name == "find") {
      command_id = ID_FIND;
    } else if(name == "print") {
      command_id = ID_PRINT;
    } else if(name == "about") {
      command_id = IDM_ABOUT;
    } else if(name == "exit") {
      command_id = IDM_EXIT;
    } else if(name == "windowrect") {
      RECT rect;
      GetWindowRect(AppGetMainHwnd(), &rect);
      retval = CefV8Value::CreateArray();
      retval->SetValue("x", CefV8Value::CreateInt(rect.left));
      retval->SetValue("y", CefV8Value::CreateInt(rect.top));
      retval->SetValue("width", CefV8Value::CreateInt(rect.right - rect.left));
      retval->SetValue("height",
          CefV8Value::CreateInt(rect.bottom - rect.top));
      return true;
    } else if(name == "setwindowrect") {
      if(arguments.size() != 4)
        return false;
      RECT rect;
      rect.left = arguments[0]->GetIntValue();
      rect.top = arguments[1]->GetIntValue();
      rect.right = rect.left + arguments[2]->GetIntValue();
      rect.bottom = rect.top + arguments[3]->GetIntValue();
      if(rect.left >= rect.right || rect.top >= rect.bottom)
        return false;
      SetWindowPos(AppGetMainHwnd(), NULL, rect.left, rect.top,
          rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
      return true;
    } else if(name == "settings") {
      CefRefPtr<WbeaHandler> handler(
          (WbeaHandler*)AppGetBrowser()->GetHandler().get());
      retval = CefV8Value::CreateString(handler->GetAppSettings());
      return true;
    } else if(name == "showdevtools") {
      AppGetBrowser()->ShowDevTools();
      return true;
    } else if(name == "closedevtools") {
      AppGetBrowser()->CloseDevTools();
      return true;
    }
    
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
  CefString code = "var app;"
    "if (!app)"
    "  app = {};"
    "(function() {"
    "  app.find = function() {"
    "    native function find();"
    "    find();"
    "  };"
    "  app.print = function() {"
    "    native function print();"
    "    print();"
    "  };"
    "  app.about = function() {"
    "    native function about();"
    "    about();"
    "  };"
    "  app.exit = function() {"
    "    native function exit();"
    "    exit();"
    "  };"
    "  app.windowrect = function() {"
    "    native function windowrect();"
    "    return windowrect();"
    "  };"
    "  app.setwindowrect = function(x, y, w, h) {"
    "    native function setwindowrect();"
    "    setwindowrect(x, y, w, h);"
    "  };"
    "  app.settings = function() {"
    "    native function settings();"
    "    return settings();"
    "  };"
    "  app.showdevtools = function() {"
    "    native function showdevtools();"
    "    return showdevtools();"
    "  };"
    "  app.closedevtools = function() {"
    "    native function closedevtools();"
    "    return closedevtools();"
    "  };"
    "})();";
  CefRegisterExtension("v8/app", code, new WbeaV8ExtensionHandler());
}
