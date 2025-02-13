//==============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 3.0 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//  
//  This software was developed by Applied Research Laboratories at the
//  University of Texas at Austin.
//  Copyright 2004-2020, The Board of Regents of The University of Texas System
//
//==============================================================================

//==============================================================================
//
//  This software was developed by Applied Research Laboratories at the
//  University of Texas at Austin, under contract to an agency or agencies
//  within the U.S. Department of Defense. The U.S. Government retains all
//  rights to use, duplicate, distribute, disclose, or release this software.
//
//  Pursuant to DoD Directive 523024 
//
//  DISTRIBUTION STATEMENT A: This software has been approved for public 
//                            release, distribution is unlimited.
//
//==============================================================================

/**
 * @file FileUtils.hpp
 * File and directory utilities
 */

#ifndef GPSTK_FILEUTILS_HPP
#define GPSTK_FILEUTILS_HPP

// #ifdef __sun
// #include <libgen.h>
// #else
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
// #endif

#include <fstream>
#include <string>
#include "StringUtils.hpp"

#ifdef WIN32
#include <direct.h>
#include <io.h>
#endif

using namespace std;
namespace gpstk
{
      /// @ingroup FileDirProc
      //@{

      /**
       * These functions and macros help process files and directories.
       */
   namespace FileUtils
   {
         /**
          * Creates a hierarchy of directories rather than just one dir.
          * This intentionally doesn't check the mkdir return codes because
          * there is no difference between return codes for directories
          * that already exist and error creating new ones.
          * @param path the full path of the directory you want created
          * @param mode the permission of the new directory (like 0755)
          * @return always 0
          */
     
#ifdef WIN32
      inline int makeDir(const std::string& path, unsigned mode)
      {
        std::string temppath = path;
        
        //Clean up windows file path 
        std::replace(temppath.begin(), temppath.end(), '\\', '/');
        std::string::size_type i = 0;

         while ((i = temppath.find('/',i+1)) != std::string::npos)
         {
            std::string thispath(temppath.substr(0,i));
            if (thispath[thispath.length() - 1] == '/')
               thispath.erase(thispath.length() - 1);
            _mkdir(thispath.c_str());

         }
         _mkdir(temppath.c_str());
         return 0;
      }
#else
      inline int makeDir(const std::string& path, unsigned mode)
      {
            //  #ifdef __sun
            //      mkdirp(path.c_str(), mode);
            //  #else
         std::string::size_type i = 0;

         while ((i = path.find('/',i+1)) != std::string::npos)
         {
            std::string thispath(path.substr(0,i));
            if (thispath[thispath.length() - 1] == '/')
               thispath.erase(thispath.length() - 1);

            mkdir(thispath.c_str(), mode);

         }
         mkdir(path.c_str(), mode);
            // #endif
         return 0;
      }
     
#endif
         /**
          * makeDir that takes a char* for an argument.
          * @param path the full path of the directory you want created
          * @param mode the permission of the new directory (like 0755)
          * @return always 0
          */
      inline int makeDir(const char* path, unsigned mode)
      {
         return makeDir(std::string(path), mode);
      }

         /**
          * Returns true if the file exists. Only readability is
          * verified unless the user inputs the openmode of interest.
          * @param[in] fname Name of the file to check
          * @param[in] mode Mode of access to check (default is
          *   readable, std::ios::in)
          * @warning This method fails to work correctly under Debian
          *   7, which seems to have a bug in the OS/system calls that
          *   erroneously allow opening read-only files with write
          *   access.  There's nothing we can do about this.
          * @return true if the file can be accessed
          */
      inline bool fileAccessCheck(const char* fname, 
                                  std::ios::openmode mode=std::ios::in)
      {
         std::fstream test(fname, mode);
         return !test.fail();
      }

      inline bool fileAccessCheck(const std::string& fname, 
                                  std::ios::openmode mode=std::ios::in)
      {
         return fileAccessCheck(fname.c_str(), mode);
      }
      

   } // namespace FileUtils

      //@}

} // namespace


#endif
