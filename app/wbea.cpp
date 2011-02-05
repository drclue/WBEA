// Copyright (c) 2010 The WBEA Authors. Portions copyright (c) 2010 The Chromium
// Embedded Framework Authors. All rights reserved. Use of this source code is
// governed by a BSD-style license that can be found in the LICENSE file.

#include "include/cef.h"
#include "include/cef_wrapper.h"
#include "wbea.h"
#include "aes_default_key.h"
#include "resource_util.h"
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
    CefRefPtr<CefFrame> frame, bool isMainContent)
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
    CefRefPtr<CefFrame> frame, bool isMainContent, int httpStatusCode)
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
    const CefString& failedUrl, CefString& errorText)
{
  if(errorCode == ERR_CACHE_MISS)
  {
    // Usually caused by navigating to a page with POST data via back or
    // forward buttons.
    errorText = "<html><head><title>Expired Form Data</title></head>"
                "<body><h1>Expired Form Data</h1>"
                "<h2>Your form request has expired. "
                "Click reload to re-submit the form data.</h2></body>"
                "</html>";
  }
  else
  {
    // All other messages.
    std::stringstream ss;
    ss <<       "<html><head><title>Load Failed</title></head>"
                "<body><h1>Load Failed</h1>"
                "<h2>Load of URL " << failedUrl.ToString().c_str() <<
                " failed with error code " << static_cast<int>(errorCode) <<
                ".</h2></body>"
                "</html>";
    errorText = ss.str();
  }
  return RV_HANDLED;
}

CefHandler::RetVal WbeaHandler::HandlePrintHeaderFooter(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefPrintInfo& printInfo, const CefString& url, const CefString& title,
    int currentPage, int maxPages, CefString& topLeft,
    CefString& topCenter, CefString& topRight, CefString& bottomLeft,
    CefString& bottomCenter, CefString& bottomRight)
{
  // Place the page title at top left
  topLeft = title;
  // Place the page URL at top right
  topRight = url;
  
  // Place "Page X of Y" at bottom center
  std::stringstream strstream;
  strstream << "Page " << currentPage << " of " << maxPages;
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
    CefRefPtr<CefBrowser> browser, const CefString& message,
    const CefString& source, int line)
{
  Lock();
  bool first_message = m_LogFile.empty();
  if(first_message) {
    std::stringstream ss;
    ss << AppGetWorkingDirectory() << "\\console.log";
    m_LogFile = ss.str();
  }
  CefString logFile = m_LogFile;
  Unlock();
  
  FILE* file = NULL;
  
#ifdef _WIN32
  fopen_s(&file, logFile.ToString().c_str(), "a");
#else
  file = fopen(logFile.ToString().c_str(), "a");
#endif
  
  if(file) {
    std::stringstream ss;
    ss << "Message: " << message.ToString().c_str() <<
        "\r\nSource: " << source.ToString().c_str() <<
        "\r\nLine: " << line << "\r\n-----------------------\r\n";
    fputs(ss.str().c_str(), file);
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

std::string WbeaHandler::GetLogFile()
{
  Lock();
  std::string str = m_LogFile;
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
        std::stringstream ss;
        ss << AppGetExecutableDirectory() << AppGetExecutableName() <<
            ".wbeal";
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
            CefString error;
            settings = new CefXmlObject("");
            settings->Load(xmlReader, XML_ENCODING_NONE, "", &error);
            delete [] out_bytes;
          }
        }
      }

      // Load the archive file, if any.
      {
        CefRefPtr<CefStreamReader> reader;
        std::stringstream ss;
        ss << AppGetExecutableDirectory() << AppGetExecutableName() << ".zip";
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
                  settings->FindChild("settings"));
              if (settingsObj.get())
                  settingsObj = settingsObj->FindChild("encryption_key");
              if (settingsObj.get()) {
                CefString encryptionKey = settingsObj->GetValue();
                if (encryptionKey.length() == 16) {
                  encryptionKeyStr = encryptionKey;
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
          "http://__app/index.html");
    }
  private:
    CefRefPtr<WbeaHandler> handler_;
  };
  
  CefRefPtr<CefTask> task(new Task(this));
  CefPostTask(TID_UI, task);

  // Load the application menu.
  LoadMenu();
}

bool WbeaHandler::GetFileContents(const std::string& relativePath,
                                  CefRefPtr<CefStreamReader>& resourceStream,
                                  CefString* mimeType)
{
  if(m_AppArchive.get()) {
    // Attempt to load from the application archive.
    CefRefPtr<CefZipArchive::File> file = m_AppArchive->GetFile(relativePath);
    if(file.get())
      resourceStream = file->GetStreamReader();
  }

  if(!resourceStream.get()) {
    // Attempt to load from the filesystem.
    std::stringstream ss;
    ss << AppGetExecutableDirectory() << AppGetExecutableName() << "\\" <<
        relativePath.c_str();
    resourceStream = CefStreamReader::CreateForFile(ss.str());
  }

  if(resourceStream.get()) {
    if(mimeType) {
      // Determine the correct mime type.
      std::string relativePathStr = relativePath;
      LPCSTR ext = strchr(relativePathStr.c_str(), '.');
      if (ext)
        *mimeType = GetSuggestedMimeType(ext+1);
      if(mimeType->empty()) {
        ASSERT(FALSE); // Should not be reached.
        *mimeType = "text/plain";
      }
    }

    return true;
  }

  return false;
}

std::string WbeaHandler::GetAppSettings()
{
  std::string ret;
  
  if (!m_AppSettings.get())
    return ret;

  CefRefPtr<CefXmlObject> obj(m_AppSettings->FindChild("settings"));
  if (obj)
    obj = obj->FindChild("app");
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
