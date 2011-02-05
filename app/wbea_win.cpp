// Copyright (c) 2010 The WBEA Authors. Portions copyright (c) 2010 The Chromium
// Embedded Framework Authors. All rights reserved. Use of this source code is
// governed by a BSD-style license that can be found in the LICENSE file.

#include "include/cef.h"
#include "include/cef_wrapper.h"
#include "wbea.h"
#include "wbea_extension.h"
#include "resource.h"
#include "resource_util.h"
#include "util.h"
#include <sstream>
#include <commdlg.h>
#include <direct.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
WCHAR szTitle[MAX_LOADSTRING];					// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
CHAR szWorkingDir[MAX_PATH];   // The current working directory
CHAR szExecutableDir[MAX_PATH];   // The current executable directory
CHAR szExecutableName[MAX_PATH];   // The current executable name
UINT uFindMsg;  // Message identifier for find events.
HWND hFindDlg = NULL; // Handle for the find dialog.

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

// The global WbeaHandler reference.
extern CefRefPtr<WbeaHandler> g_handler;

#ifdef _WIN32
// Add Common Controls to the application manifest because it's required to
// support the default tooltip implementation.
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// Program entry point function.
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Retrieve the current working directory.
  if (_getcwd(szWorkingDir, MAX_PATH) == NULL)
    szWorkingDir[0] = 0;

  // Retrieve the executable directory and name.
  CHAR modulepath[MAX_PATH], filepath[MAX_PATH];
  if (GetModuleFileNameA(NULL, modulepath, MAX_PATH) > 0 &&
      GetLongPathNameA(modulepath, filepath, MAX_PATH) > 0) {
    LPCSTR name = strchr(filepath, '\\');
    if (name) {
      name++;
      strncpy_s(szExecutableDir, MAX_PATH, filepath, name - filepath);
      LPCSTR ext = strchr(filepath, '.');
      if (ext)
        strncpy_s(szExecutableName, MAX_PATH, name, ext - name);
    }
  }

  CHAR cachepath[MAX_PATH];
  sprintf_s(cachepath, MAX_PATH, "%s%s_cache", szExecutableDir,
      szExecutableName);

  // Initialize the CEF with messages processed using the current application's
  // message loop.
  CefSettings settings;
  CefBrowserSettings browser_defaults;

  CefString(&settings.cache_path).FromASCII(cachepath);
#ifdef NDEBUG
  settings.log_severity = LOGSEVERITY_DISABLE;
#endif

  CefInitialize(settings, browser_defaults);
  
  MSG msg;
  HACCEL hAccelTable;

  // Initialize global strings
  LoadString(hInstance, IDS_WBEA_TITLE, szTitle, MAX_LOADSTRING);
  LoadString(hInstance, IDC_WBEA, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization
  if (!InitInstance (hInstance, nCmdShow))
    return FALSE;

  hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WBEA));

  // Register the find event message.
  uFindMsg = RegisterWindowMessage(FINDMSGSTRING);

  // Initialize the WBEA extension.
  InitWbeaExtension();

  // Main message loop
  while (GetMessage(&msg, NULL, 0, 0))
  {
    // Allow the CEF to do its message loop processing.
    CefDoMessageLoopWork();

    // Allow processing of find dialog messages.
    if(hFindDlg && IsDialogMessage(hFindDlg, &msg))
      continue;

    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  // Shut down the CEF
  CefShutdown();

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this
//    function so that the application will get 'well formed' small icons
//    associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WBEA));
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WBEA_SMALL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WBEA);
	wcex.lpszClassName	= szWindowClass;

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  HWND hWnd;

  hInst = hInstance; // Store instance handle in our global variable

  hWnd = CreateWindow(szWindowClass, szTitle,
      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, 0, CW_USEDEFAULT,
      0, NULL, NULL, hInstance, NULL);

  if (!hWnd)
    return FALSE;

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  // Static members used for the find dialog.
  static FINDREPLACE fr;
  static WCHAR szFindWhat[80] = {0};
  static WCHAR szLastFindWhat[80] = {0};
  static bool findNext = false;
  static bool lastMatchCase = false;
  
  int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

  if (message == uFindMsg)
  { 
    // Find event.
    LPFINDREPLACE lpfr = (LPFINDREPLACE)lParam;

    if (lpfr->Flags & FR_DIALOGTERM)
    { 
      // The find dialog box has been dismissed so invalidate the handle and
      // reset the search results.
      hFindDlg = NULL; 
      if(g_handler.get())
      {
        g_handler->GetBrowser()->StopFinding(true);
        szLastFindWhat[0] = 0;
        findNext = false;
      }
      return 0; 
    } 

    if ((lpfr->Flags & FR_FINDNEXT) && g_handler.get()) 
    {
      // Search for the requested string.
      bool matchCase = (lpfr->Flags & FR_MATCHCASE?true:false);
      if(matchCase != lastMatchCase ||
        (matchCase && wcsncmp(szFindWhat, szLastFindWhat,
          sizeof(szLastFindWhat)/sizeof(WCHAR)) != 0) ||
        (!matchCase && _wcsnicmp(szFindWhat, szLastFindWhat,
          sizeof(szLastFindWhat)/sizeof(WCHAR)) != 0))
      {
        // The search string has changed, so reset the search results.
        if(szLastFindWhat[0] != 0) {
          g_handler->GetBrowser()->StopFinding(true);
          findNext = false;
        }
        lastMatchCase = matchCase;
        wcscpy_s(szLastFindWhat, sizeof(szLastFindWhat)/sizeof(WCHAR),
            szFindWhat);
      }

      g_handler->GetBrowser()->Find(0, lpfr->lpstrFindWhat,
          (lpfr->Flags & FR_DOWN)?true:false, matchCase, findNext);
      if(!findNext)
        findNext = true;
    }

    return 0; 
  }
  else
  {
    // Callback for the main window
	  switch (message)
	  {
    case WM_CREATE:
      {
        // Create the single static handler class instance
        g_handler = new WbeaHandler();
        g_handler->SetMainHwnd(hWnd);

        RECT rect;
        CefWindowInfo info;

        GetClientRect(hWnd, &rect);
        
        // Initialize window info to the defaults for a child window
        info.SetAsChild(hWnd, rect);

        // Creat the new child child browser window
        CefBrowser::CreateBrowser(info, false,
            static_cast<CefRefPtr<CefHandler> >(g_handler),
            "http://__int/loading");

        // Load the application archive.
        g_handler->LoadArchive();
      }
      return 0;

    case WM_COMMAND:
      {
        CefRefPtr<CefBrowser> browser;
        if(g_handler.get())
          browser = g_handler->GetBrowser();

		    wmId    = LOWORD(wParam);
		    wmEvent = HIWORD(wParam);
		    // Parse the menu selections:
        if(wmId >= ID_MENU_FIRST && wmId <= ID_MENU_LAST) {
          if(g_handler.get())
            g_handler->HandleMenuAction(wmId);
        } else switch (wmId) {
		    case IDM_ABOUT:
          DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			    return 0;
		    case IDM_EXIT:
			    DestroyWindow(hWnd);
			    return 0;
        case ID_WARN_CONSOLEMESSAGE:
          if(g_handler.get()) {
            std::stringstream ss;
            ss << "Console messages will be written to "
               << g_handler->GetLogFile();
            MessageBoxA(hWnd, ss.str().c_str(), "Console Messages",
                MB_OK | MB_ICONINFORMATION);
          }
          return 0;
        case ID_FIND:
          if(!hFindDlg)
          {
            // Create the find dialog.
            ZeroMemory(&fr, sizeof(fr));
            fr.lStructSize = sizeof(fr);
            fr.hwndOwner = hWnd;
            fr.lpstrFindWhat = szFindWhat;
            fr.wFindWhatLen = sizeof(szFindWhat);
            fr.Flags = FR_HIDEWHOLEWORD | FR_DOWN;

            hFindDlg = FindText(&fr);
          }
          else
          {
            // Give focus to the existing find dialog.
            ::SetFocus(hFindDlg);
          }
          return 0;
        case ID_PRINT:
          if(browser.get())
            browser->GetMainFrame()->Print();
          return 0;
        }
      }
		  break;

	  case WM_PAINT:
		  hdc = BeginPaint(hWnd, &ps);
		  EndPaint(hWnd, &ps);
		  return 0;

    case WM_SETFOCUS:
      if(g_handler.get() && g_handler->GetBrowserHwnd())
      {
        // Pass focus to the browser window
        PostMessage(g_handler->GetBrowserHwnd(), WM_SETFOCUS, wParam, NULL);
      }
      return 0;

    case WM_SIZE:
      if(g_handler.get() && g_handler->GetBrowserHwnd())
      {
        // Resize the browser window to match the new frame window size
        RECT rect;
        GetClientRect(hWnd, &rect);
        
        SetWindowPos(g_handler->GetBrowserHwnd(), NULL, rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
      }
      break;

    case WM_ERASEBKGND:
      if(g_handler.get() && g_handler->GetBrowserHwnd())
      {
        // Dont erase the background if the browser window has been loaded
        // (this avoids flashing)
		    return 0;
      }
      break;

    case WM_DESTROY:
      // The frame window has exited
      PostQuitMessage(0);
		  return 0;
    }
  	
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// WbeaHandler implementation

WbeaHandler::WbeaHandler()
  : m_MainHwnd(NULL), m_BrowserHwnd(NULL), m_bLoading(false),
    m_bCanGoBack(false), m_bCanGoForward(false), m_MenuLastId(ID_MENU_FIRST)
{
}

CefHandler::RetVal WbeaHandler::HandleTitleChange(
    CefRefPtr<CefBrowser> browser, const CefString& title)
{
  // Set the frame window title bar
  CefWindowHandle hwnd = browser->GetWindowHandle();
  if(!browser->IsPopup())
  {
    // The frame window will be the parent of the browser window
    hwnd = GetParent(hwnd);
  }
  SetWindowText(hwnd, title.c_str());
  return RV_CONTINUE;
}

CefHandler::RetVal WbeaHandler::HandleBeforeResourceLoad(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefRequest> request,
    CefString& redirectUrl, CefRefPtr<CefStreamReader>& resourceStream,
    CefString& mimeType, int loadFlags)
{
  DWORD dwSize;
  LPBYTE pBytes;
  
  std::string url = request->GetURL();
  if(url == "http://__int/loading") {
    // Show the internal loading.html contents.
    if(LoadBinaryResource(IDS_LOADING, dwSize, pBytes)) {
      resourceStream = CefStreamReader::CreateForHandler(
          new CefByteReadHandler(pBytes, dwSize, NULL));
      mimeType = "text/html";
    }
    return RV_CONTINUE;
  }

  std::string prefix = "http://__app/";
  if(url.find(prefix) == 0) {
    std::string path = url.substr(prefix.length());
		int pos = path.find('#');
		if(pos > 0)
			path = path.substr(0, pos);
		pos = path.find('?');
		if(pos > 0)
			path = path.substr(0, pos);
    GetFileContents(path, resourceStream, &mimeType);
  }

  return RV_CONTINUE;
}

CefHandler::RetVal WbeaHandler::HandleKeyEvent(CefRefPtr<CefBrowser> browser,
                                               KeyEventType type,
                                               int code,
                                               int modifiers,
                                               bool isSystemKey)
{
  // Reload on CTRL-R.
  if (m_Browser.get() && type == KEYEVENT_RAWKEYDOWN && code == 'R' &&
      GetKeyState(VK_CONTROL) < 0) {
    m_Browser->Reload();
    return RV_HANDLED;
  }

  return RV_CONTINUE;
}

void WbeaHandler::SendNotification(NotificationType type)
{
  UINT id;
  switch(type)
  {
  case NOTIFY_CONSOLE_MESSAGE:
    id = ID_WARN_CONSOLEMESSAGE;
    break;
  default:
    return;
  }
  PostMessage(m_MainHwnd, WM_COMMAND, id, 0);
}

void WbeaHandler::LoadMenu()
{
  CefRefPtr<CefStreamReader> reader;
  if(!GetFileContents("menu.xml", reader, NULL))
    return;

  CefRefPtr<CefXmlObject> xmlObj(new CefXmlObject(""));
  CefString errorStr;
  if(!xmlObj->Load(reader, XML_ENCODING_NONE, "", &errorStr)) {
    std::stringstream ss;
    ss << "Failed to parse menu.xml\n" << errorStr.ToString().c_str();
    MessageBoxA(m_MainHwnd, ss.str().c_str(), "Error", MB_ICONERROR | MB_OK);
    return;
  }

  CefRefPtr<CefXmlObject> topMenuObj(xmlObj->FindChild("topmenu"));
  if(!topMenuObj.get())
    return;

  HMENU topMenu = ::CreateMenu();
  CreateMenu(topMenu, topMenuObj);

  // Set as the application menu.
  SetMenu(m_MainHwnd, topMenu);
}

void WbeaHandler::HandleMenuAction(UINT id)
{
  MenuActionMap::const_iterator it = m_MenuActionMap.find(id);
  if(it == m_MenuActionMap.end())
    return;

  m_Browser->GetMainFrame()->ExecuteJavaScript(it->second, "", 0);
}

void WbeaHandler::CreateMenu(HMENU menu, CefRefPtr<CefXmlObject> obj)
{
  CefXmlObject::ObjectVector children;
  if(obj->GetChildren(children) == 0)
    return;

  CefRefPtr<CefXmlObject> child;
  std::wstring label, action;
  UINT index = 0;

  CefXmlObject::ObjectVector::const_iterator it = children.begin();
  for(; it != children.end(); ++it) {
    child = *it;
    
    if(child->HasAttribute("label")) {
      label = child->GetAttributeValue("label");
      for(UINT i = 0; i < label.length(); ++i) {
        if(label[i] == '_')
          label[i] = '&';
      }
    } else {
      label = L"";
    }

    if(child->GetName() == "menuitem") {
      if(child->HasAttribute("separator")) {
        // Add a separator item.
        AddMenuSeparator(menu, index++);
      } else {
        // Add a standard item.
        action = child->GetAttributeValue("action");
        AddMenuItem(menu, index++, label, action, true);
      }
    }
    else if(child->GetName() == "menu") {
      // Add a sub-menu.
      HMENU subMenu = AddMenu(menu, index++, label, true);
      CreateMenu(subMenu, child);
    }
  }
}

HMENU WbeaHandler::AddMenu(HMENU menu, UINT index, const std::wstring& label,
                           bool enabled)
{
  HMENU newMenu = CreatePopupMenu();
  if(!newMenu)
    return NULL;
  
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_SUBMENU;
  mii.fType = MFT_STRING;
  if(!enabled) {
    mii.fMask |= MIIM_STATE;
    mii.fState = MFS_GRAYED;
  }
  mii.hSubMenu = newMenu;
  mii.dwTypeData = const_cast<wchar_t*>(label.c_str());

  if(!InsertMenuItem(menu, index, TRUE, &mii)) {
    DestroyMenu(newMenu);
    return NULL;
  }

  return newMenu;
}

BOOL WbeaHandler::AddMenuItem(HMENU menu, UINT index, const std::wstring& label,
                              const std::wstring& action, bool enabled)
{
  m_MenuLastId++;
  m_MenuActionMap.insert(
      std::make_pair<UINT,std::wstring>(m_MenuLastId,action));

  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
  mii.fType = MFT_STRING;
  if(!enabled) {
    mii.fMask |= MIIM_STATE;
    mii.fState = MFS_GRAYED;
  }
  mii.wID = m_MenuLastId;
  mii.dwTypeData = const_cast<wchar_t*>(label.c_str());

  return InsertMenuItem(menu, index, TRUE, &mii);
}

BOOL WbeaHandler::AddMenuSeparator(HMENU menu, UINT index)
{
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_FTYPE;
  mii.fType = MFT_SEPARATOR;

  return InsertMenuItem(menu, index, TRUE, &mii);
}


// Global functions

std::string AppGetWorkingDirectory()
{
	return szWorkingDir;
}

std::string AppGetExecutableDirectory()
{
  return szExecutableDir;
}

std::string AppGetExecutableName()
{
  return szExecutableName;
}
