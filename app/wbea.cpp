// Copyright (c) 2010 The WBEA Authors. Portions copyright (c) 2010 The Chromium
// Embedded Framework Authors. All rights reserved. Use of this source code is
// governed by a BSD-style license that can be found in the LICENSE file.

#include "include/cef.h"
#include "include/cef_wrapper.h"
#include "wbea.h"
#include "aes_default_key.h"
#include "resource_util.h"
#include "string_util.h"
#include "util.h"
#include <sstream>
#include <string>


// WbeaHandler implementation

WbeaHandler::~WbeaHandler()
{
}

CefHandler::RetVal WbeaHandler::HandleAfterCreated(
    CefRefPtr<CefBrowser> browser)
{
  Lock();
  if(!browser->IsPopup())
  {
    // We need to keep the main child window, but not popup windows
    m_Browser = browser;
    m_BrowserHwnd = browser->GetWindowHandle();
  }
  Unlock();
  return RV_CONTINUE;
}

CefHandler::RetVal WbeaHandler::HandleLoadStart(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame)
{
  if(!browser->IsPopup() && !frame.get())
  {
    Lock();
    // We've just started loading a page
    m_bLoading = true;
    m_bCanGoBack = false;
    m_bCanGoForward = false;
    Unlock();
  }
  return RV_CONTINUE;
}

CefHandler::RetVal WbeaHandler::HandleLoadEnd(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame)
{
  if(!browser->IsPopup() && !frame.get())
  {
    Lock();
    // We've just finished loading a page
    m_bLoading = false;
    m_bCanGoBack = browser->CanGoBack();
    m_bCanGoForward = browser->CanGoForward();
    Unlock();
  }
  return RV_CONTINUE;
}

CefHandler::RetVal WbeaHandler::HandleLoadError(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame, ErrorCode errorCode,
    const std::wstring& failedUrl, std::wstring& errorText)
{
  if(errorCode == ERR_CACHE_MISS)
  {
    // Usually caused by navigating to a page with POST data via back or
    // forward buttons.
    errorText = L"<html><head><title>Expired Form Data</title></head>"
                L"<body><h1>Expired Form Data</h1>"
                L"<h2>Your form request has expired. "
                L"Click reload to re-submit the form data.</h2></body>"
                L"</html>";
  }
  else
  {
    // All other messages.
    std::wstringstream ss;
    ss <<       L"<html><head><title>Load Failed</title></head>"
                L"<body><h1>Load Failed</h1>"
                L"<h2>Load of URL " << failedUrl <<
                L" failed with error code " << static_cast<int>(errorCode) <<
                L".</h2></body>"
                L"</html>";
    errorText = ss.str();
  }
  return RV_HANDLED;
}

CefHandler::RetVal WbeaHandler::HandlePrintHeaderFooter(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefPrintInfo& printInfo, const std::wstring& url, const std::wstring& title,
    int currentPage, int maxPages, std::wstring& topLeft,
    std::wstring& topCenter, std::wstring& topRight, std::wstring& bottomLeft,
    std::wstring& bottomCenter, std::wstring& bottomRight)
{
  // Place the page title at top left
  topLeft = title;
  // Place the page URL at top right
  topRight = url;
  
  // Place "Page X of Y" at bottom center
  std::wstringstream strstream;
  strstream << L"Page " << currentPage << L" of " << maxPages;
  bottomCenter = strstream.str();

  return RV_CONTINUE;
}

CefHandler::RetVal WbeaHandler::HandleJSBinding(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Value> object)
{
  return RV_HANDLED;
}

CefHandler::RetVal WbeaHandler::HandleBeforeWindowClose(
    CefRefPtr<CefBrowser> browser)
{
  if(m_BrowserHwnd == browser->GetWindowHandle())
  {
    // Free the browser pointer so that the browser can be destroyed
    m_Browser = NULL;
    m_AppArchive = NULL;
  }
  return RV_CONTINUE;
}

CefHandler::RetVal WbeaHandler::HandleConsoleMessage(
    CefRefPtr<CefBrowser> browser, const std::wstring& message,
    const std::wstring& source, int line)
{
  Lock();
  bool first_message = m_LogFile.empty();
  if(first_message) {
    std::wstringstream ss;
    ss << AppGetWorkingDirectory() << L"\\console.log";
    m_LogFile = ss.str();
  }
  std::wstring logFile = m_LogFile;
  Unlock();
  
  FILE* file = NULL;
  _wfopen_s(&file, logFile.c_str(), L"a, ccs=UTF-8");
  
  if(file) {
    std::wstringstream ss;
    ss << L"Message: " << message << L"\r\nSource: " << source <<
        L"\r\nLine: " << line << L"\r\n-----------------------\r\n";
    fputws(ss.str().c_str(), file);
    fclose(file);

    if(first_message)
      SendNotification(NOTIFY_CONSOLE_MESSAGE);
  }

  return RV_HANDLED;
}

void WbeaHandler::GetNavState(bool &isLoading, bool &canGoBack,
                              bool &canGoForward)
{
  Lock();
  isLoading = m_bLoading;
  canGoBack = m_bCanGoBack;
  canGoForward = m_bCanGoForward;
  Unlock();
}

void WbeaHandler::SetMainHwnd(CefWindowHandle hwnd)
{
  Lock();
  m_MainHwnd = hwnd;
  Unlock();
}

std::wstring WbeaHandler::GetLogFile()
{
  Lock();
  std::wstring str = m_LogFile;
  Unlock();
  return str;
}

void WbeaHandler::LoadArchive()
{
  class Task : public CefThreadSafeBase<CefTask>
  {
  public:
    Task(CefRefPtr<WbeaHandler> handler) : handler_(handler) {}
    virtual void Execute(CefThreadId threadId)
    {
      CefRefPtr<CefZipArchive> archive;
      CefRefPtr<CefXmlObject> settings;

      const char* encryption_key = AES_DEFAULT_KEY;
      std::string encryptionKeyStr;
      
      // Load the encrypted settings file, if any.
      {
        CefRefPtr<CefStreamReader> reader;
        std::wstringstream ss;
        ss << AppGetExecutableDirectory() << AppGetExecutableName() <<
            L".wbeal";
        reader = CefStreamReader::CreateForFile(ss.str());
        if (reader.get()) {
          // Decrypt the settings file.
          unsigned char* out_bytes = NULL;
          size_t out_size = 0;
          if(Decrypt(encryption_key, reader, &out_bytes, &out_size)) {
            // Read the XML data.
            CefRefPtr<CefStreamReader> xmlReader(
                CefStreamReader::CreateForHandler(
                    new CefByteReadHandler(out_bytes, out_size, NULL)));
            std::wstring error;
            settings = new CefXmlObject(L"");
            settings->Load(xmlReader, XML_ENCODING_NONE, L"", &error);
            delete [] out_bytes;
          }
        }
      }

      // Load the archive file, if any.
      {
        CefRefPtr<CefStreamReader> reader;
        std::wstringstream ss;
        ss << AppGetExecutableDirectory() << AppGetExecutableName() << L".zip";
        reader = CefStreamReader::CreateForFile(ss.str());
        if (reader.get()) {
          bool decrypt = false;
          
          // Test if the zip archive is encrypted.
          char header[2] = {0};
          reader->Read(&header, 1, 2);
          decrypt = (header[0] != 'P' || header[1] != 'K');
          reader->Seek(0, SEEK_SET);

          CefRefPtr<CefStreamReader> archiveReader;

          unsigned char* out_bytes = NULL;
          if (decrypt) {
            if (settings.get()) {
              // Retrieve the archive encryption key from the settings.
              CefRefPtr<CefXmlObject> settingsObj(
                  settings->FindChild(L"settings"));
              if (settingsObj.get())
                  settingsObj = settingsObj->FindChild(L"encryption_key");
              if (settingsObj.get()) {
                std::wstring encryptionKey = settingsObj->GetValue();
                if (encryptionKey.length() == 16) {
                  encryptionKeyStr = WStringToString(encryptionKey);
                  encryption_key = encryptionKeyStr.c_str();
                }
              }
            }

            // Decrypt the zip archive.
            size_t out_size = 0;
            if(Decrypt(encryption_key, reader, &out_bytes, &out_size)) {
              archiveReader = CefStreamReader::CreateForHandler(
                new CefByteReadHandler(out_bytes, out_size, NULL));
            }
          }

          if (!archiveReader.get()) {
            // Assume the archive isn't encrypted.
            reader->Seek(0, SEEK_SET);
            archiveReader = reader;
          }

          archive = new CefZipArchive();
          archive->Load(archiveReader, true);

          if (out_bytes)
            delete [] out_bytes;
        }
      }

      handler_->LoadArchiveComplete(archive, settings);
    }
  private:
    CefRefPtr<WbeaHandler> handler_;
  };

  CefRefPtr<CefTask> task(new Task(this));
  CefPostTask(TID_FILE, task);
}

void WbeaHandler::LoadArchiveComplete(CefRefPtr<CefZipArchive> archive,
                                      CefRefPtr<CefXmlObject> settings)
{
  m_AppArchive = archive;
  m_AppSettings = settings;

  class Task : public CefThreadSafeBase<CefTask>
  {
  public:
    Task(CefRefPtr<WbeaHandler> handler) : handler_(handler) {}
    virtual void Execute(CefThreadId threadId)
    {
      // Redirect to the index page.
      handler_->GetBrowser()->GetMainFrame()->LoadURL(
          L"http://__app/index.html");
    }
  private:
    CefRefPtr<WbeaHandler> handler_;
  };
  
  CefRefPtr<CefTask> task(new Task(this));
  CefPostTask(TID_UI, task);

  // Load the application menu.
  LoadMenu();
}

bool WbeaHandler::GetFileContents(const std::wstring& relativePath,
                                  CefRefPtr<CefStreamReader>& resourceStream,
                                  std::wstring* mimeType)
{
  if(m_AppArchive.get()) {
    // Attempt to load from the application archive.
    CefRefPtr<CefZipArchive::File> file = m_AppArchive->GetFile(relativePath);
    if(file.get())
      resourceStream = file->GetStreamReader();
  }

  if(!resourceStream.get()) {
    // Attempt to load from the filesystem.
    std::wstringstream ss;
    ss << AppGetExecutableDirectory() << AppGetExecutableName() << L"\\" <<
        relativePath;
    resourceStream = CefStreamReader::CreateForFile(ss.str());
  }

  if(resourceStream.get()) {
    if(mimeType) {
      // Determine the correct mime type.
      LPWSTR ext = wcsrchr(const_cast<LPWSTR>(relativePath.c_str()), '.');
      if (ext)
        *mimeType = GetSuggestedMimeType(ext+1);
      if(mimeType->empty()) {
        ASSERT(FALSE); // Should not be reached.
        *mimeType = L"text/plain";
      }
    }

    return true;
  }

  return false;
}

std::wstring WbeaHandler::GetAppSettings()
{
  std::wstring ret;
  
  if (!m_AppSettings.get())
    return ret;

  CefRefPtr<CefXmlObject> obj(m_AppSettings->FindChild(L"settings"));
  if (obj)
    obj = obj->FindChild(L"app");
  if (!obj)
    return ret;

  return obj->GetValue();
}


// Global functions

CefRefPtr<WbeaHandler> g_handler;

CefRefPtr<CefBrowser> AppGetBrowser()
{
  if(!g_handler.get())
    return NULL;
  return g_handler->GetBrowser();
}

CefWindowHandle AppGetMainHwnd()
{
  if(!g_handler.get())
    return NULL;
  return g_handler->GetMainHwnd();
}
