// Copyright (c) 2010 The WBEA Authors. Portions copyright (c) 2010 The Chromium
// Embedded Framework Authors. All rights reserved. Use of this source code is
// governed by a BSD-style license that can be found in the LICENSE file.

#ifndef _WBEA_H
#define _WBEA_H

#include "include/cef.h"
#include "include/cef_wrapper.h"


// Client implementation of the browser handler class
class WbeaHandler : public CefThreadSafeBase<CefHandler>
{
public:
  WbeaHandler();
  ~WbeaHandler();

  virtual RetVal HandleBeforeCreated(CefRefPtr<CefBrowser> parentBrowser,
                                     CefWindowInfo& windowInfo, bool popup,
                                     const CefPopupFeatures& popupFeatures,
                                     CefRefPtr<CefHandler>& handler,
                                     CefString& url,
                                     CefBrowserSettings& settings)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleAfterCreated(CefRefPtr<CefBrowser> browser);
  virtual RetVal HandleAddressChange(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     const CefString& url)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleTitleChange(CefRefPtr<CefBrowser> browser,
                                   const CefString& title);
  virtual RetVal HandleBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    NavType navType, bool isRedirect)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleLoadStart(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 bool isMainContent);
  virtual RetVal HandleLoadEnd(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               bool isMainContent,
                               int httpStatusCode);
  virtual RetVal HandleLoadError(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 ErrorCode errorCode,
                                 const CefString& failedUrl,
                                 CefString& errorText);
  virtual RetVal HandleBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefRequest> request,
                                          CefString& redirectUrl,
                                          CefRefPtr<CefStreamReader>& resourceStream,
                                          CefString& mimeType,
                                          int loadFlags);
  virtual RetVal HandleProtocolExecution(CefRefPtr<CefBrowser> browser,
                                         const CefString& url,
                                         bool* allow_os_execution)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleDownloadResponse(CefRefPtr<CefBrowser> browser,
                                        const CefString& mimeType,
                                        const CefString& fileName,
                                        int64 contentLength,
                                        CefRefPtr<CefDownloadHandler>& handler)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleAuthenticationRequest(CefRefPtr<CefBrowser> browser,
                                             bool isProxy,
                                             const CefString& host,
                                             const CefString& realm,
                                             const CefString& scheme,
                                             CefString& username,
                                             CefString& password)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleBeforeMenu(CefRefPtr<CefBrowser> browser,
                                  const MenuInfo& menuInfo)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleGetMenuLabel(CefRefPtr<CefBrowser> browser,
                                    MenuId menuId, CefString& label)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleMenuAction(CefRefPtr<CefBrowser> browser,
                                  MenuId menuId)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandlePrintOptions(CefRefPtr<CefBrowser> browser,
                                    CefPrintOptions& printOptions)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandlePrintHeaderFooter(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefPrintInfo& printInfo,
                                         const CefString& url,
                                         const CefString& title,
                                         int currentPage, int maxPages,
                                         CefString& topLeft,
                                         CefString& topCenter,
                                         CefString& topRight,
                                         CefString& bottomLeft,
                                         CefString& bottomCenter,
                                         CefString& bottomRight);
  virtual RetVal HandleJSAlert(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               const CefString& message)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleJSConfirm(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 const CefString& message, bool& retval)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleJSPrompt(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                const CefString& message,
                                const CefString& defaultValue,
                                bool& retval,
                                CefString& result)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleJSBinding(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Value> object);
  virtual RetVal HandleBeforeWindowClose(CefRefPtr<CefBrowser> browser);
  virtual RetVal HandleTakeFocus(CefRefPtr<CefBrowser> browser, bool reverse)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleSetFocus(CefRefPtr<CefBrowser> browser,
                                bool isWidget)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleTooltip(CefRefPtr<CefBrowser> browser,
                               CefString& text)
  {
    return RV_CONTINUE;
  }
  virtual RetVal HandleStatus(CefRefPtr<CefBrowser> browser,
                              const CefString& value, 
                              StatusType type)
  {
    return RV_CONTINUE;
  }
  RetVal HandleKeyEvent(CefRefPtr<CefBrowser> browser,
                        KeyEventType type,
                        int code,
                        int modifiers,
                        bool isSystemKey);
  RetVal HandleConsoleMessage(CefRefPtr<CefBrowser> browser,
                              const CefString& message,
                              const CefString& source, int line);
  virtual RetVal HandleFindResult(CefRefPtr<CefBrowser> browser,
                                  int identifier, int count,
                                  const CefRect& selectionRect,
                                  int activeMatchOrdinal, bool finalUpdate)
  {
    return RV_CONTINUE;
  }

  // Retrieve the current navigation state flags
  void GetNavState(bool &isLoading, bool &canGoBack, bool &canGoForward);
  void SetMainHwnd(CefWindowHandle hwnd);
  CefWindowHandle GetMainHwnd() { return m_MainHwnd; }
  CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
  CefWindowHandle GetBrowserHwnd() { return m_BrowserHwnd; }

  std::string GetLogFile();

  // Send a notification to the application. Notifications should not block the
  // caller.
  enum NotificationType
  {
    NOTIFY_CONSOLE_MESSAGE,
  };
  void SendNotification(NotificationType type);

  // Load the application archive.
  void LoadArchive();
  void LoadArchiveComplete(CefRefPtr<CefZipArchive> archive,
                           CefRefPtr<CefXmlObject> settings);

  // Load the application menu.
  void LoadMenu();

#ifdef _WIN32
  // Handle a menu action.
  void HandleMenuAction(UINT id);
#endif

  // Retrieves the contents of the file at the specified relative path.
  bool GetFileContents(const std::string& relativePath,
      CefRefPtr<CefStreamReader>& resourceStream, CefString* mimeType);

  // Returns the <app> section of the settings file, if any.
  std::string GetAppSettings();

protected:
#ifdef _WIN32
  void CreateMenu(HMENU menu, CefRefPtr<CefXmlObject> obj);
  HMENU AddMenu(HMENU menu, UINT index, const std::wstring& label,
      bool enabled);
  BOOL AddMenuItem(HMENU menu, UINT index, const std::wstring& label,
      const std::wstring& action, bool enabled);
  BOOL AddMenuSeparator(HMENU menu, UINT index);
#endif

  // The child browser window
  CefRefPtr<CefBrowser> m_Browser;

  // The main frame window handle
  CefWindowHandle m_MainHwnd;

  // The child browser window handle
  CefWindowHandle m_BrowserHwnd;

  // The edit window handle
  CefWindowHandle m_EditHwnd;

  // True if the page is currently loading
  bool m_bLoading;
  // True if the user can navigate backwards
  bool m_bCanGoBack;
  // True if the user can navigate forwards
  bool m_bCanGoForward;

  std::string m_LogFile;

  // Application archive.
  CefRefPtr<CefZipArchive> m_AppArchive;
  // Application settings.
  CefRefPtr<CefXmlObject> m_AppSettings;

#ifdef _WIN32
  // Menu action map.
  typedef std::map<UINT, std::wstring> MenuActionMap;
  MenuActionMap m_MenuActionMap;
  UINT m_MenuLastId;
#endif
};


// Returns the main browser window instance.
CefRefPtr<CefBrowser> AppGetBrowser();

// Returns the main application window handle.
CefWindowHandle AppGetMainHwnd();

// Returns the application working directory.
std::string AppGetWorkingDirectory();

// Returns the application executable directory.
std::string AppGetExecutableDirectory();

// Returns the application executable name.
std::string AppGetExecutableName();

#endif // _WBEA_H
