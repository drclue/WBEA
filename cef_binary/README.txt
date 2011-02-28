Chromium Embedded Framework (CEF) Binary Distribution
-------------------------------------------------------------------------------

CEF Revision:        195
Chromium Revision:   74933
Date:                February 28, 2011

This distribution contains all files necessary to build an application using
CEF.  Please read the included LICENSE.txt file for licensing terms and
restrictions.


CONTENTS
--------

build       Contains libcef.dll and other DLLs required to run CEF-based
            applications.  Also acts as the build target for the included
            cefclient sample application.

cefclient   Contains the cefclient sample application configured to build
            using the files in this distribution.

include     Contains all required CEF and NPAPI-related header files.  Read
            the include/npapi/NPAPI-README.txt file for more information about
            the NPAPI-related header files.

lib         Contains the libcef.lib and libcef_dll_wrapper.lib library files
            that all CEF-based C++ applications must link against.


USAGE
-----

Visual Studio 2008: Open the cefclient/cefclient2008.sln solution and build.
Visual Studio 2005: Building with VS2005 is no longer actively supported by the
   CEF developers. As a result, CEF binary distributions no longer include C++
   libraries for VS2005.
   * The included VS2008 builds of libcef.dll and libcef.lib can be used with
     VS2005 projects.
   * Developers using the C++ API with VS2005 must build their own version of
     libcef_dll_wrapper.lib from source code. Since Chromium projects are not
     guaranteed to compile with VS2005 the most reliable way to do this is as
     follows:
     1. Build all projects using VS2008.
        A. Set the GYP_MSVS_VERSION environment variable to "2008".
        B. Run cef_create_projects.bat in the CEF root directory.
        C. Open cef.sln in VS2008.
        D. Build all projects for both Debug and Release configurations.
     2. Rebuild just the libcef_dll_wrapper project using VS2005.
        A. Set the GYP_MSVS_VERSION environment variable to "2005".
        B. Run cef_create_projects.bat in the CEF root directory.
        C. Open cef.sln in VS2005.
        D. Right click on the libcef_dll_wrapper project and select "Project
           Only" > "Rebuild only libcef_dll_wrapper" for both Debug and Release
           configurations.
        E. Copy libcef_dll_wrapper.lib from the Debug/lib and Release/lib
           directories.

Please visit the CEF Website for additional usage information.

http://code.google.com/p/chromiumembedded
