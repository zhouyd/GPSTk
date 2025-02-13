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
 * @file Rinex3ObsStream.hpp
 * File stream for RINEX 3 observation file data.
 */

#ifndef RINEX3OBSSTREAM_HPP
#define RINEX3OBSSTREAM_HPP

#include <vector>
#include <list>
#include <map>
#include <string>

#include "FFTextStream.hpp"
#include "Rinex3ObsHeader.hpp"

namespace gpstk
{
      /// @ingroup FileHandling
      //@{

      /**
       * This class reads RINEX 3 Obs files.
       *
       * @sa Rinex3ObsData and Rinex3ObsHeader.
       */
   class Rinex3ObsStream : public FFTextStream
   {
   public:
         /// Default constructor
      Rinex3ObsStream();

         /** Common constructor.
          *
          * @param[in] fn the RINEX file to open
          * @param[in] mode how to open \a fn.
          */
      Rinex3ObsStream( const char* fn,
                       std::ios::openmode mode = std::ios::in );

         /** Common constructor.
          *
          * @param[in] fn the RINEX file to open
          * @param[in] mode how to open \a fn.
          */
      Rinex3ObsStream( const std::string fn,
                       std::ios::openmode mode = std::ios::in );

         /// Destructor
      virtual ~Rinex3ObsStream();

         /** Overrides open to reset the header
          *
          * @param fn the RINEX file to open
          * @param mode how to open \a fn.
          */
      virtual void open( const char* fn,
                         std::ios::openmode mode );

         /** Overrides open to reset the header
          *
          * @param fn the RINEX 3 file to open
          * @param mode how to open \a fn.
          */
      virtual void open( const std::string& fn,
                         std::ios::openmode mode );

         /// Whether or not the Rinex3ObsHeader has been read
      bool headerRead;

         /// The header for this file.
      Rinex3ObsHeader header;

         /// Time system for epochs in this file
      TimeSystem timesystem;

         /// Check if the input stream is the kind of Rinex3ObsStream
      static bool isRinex3ObsStream(std::istream& i);

   private:
         /// Initialize internal data structures.
      void init();
   }; // class 'Rinex3ObsStream'

      //@}

} // namespace gpstk

#endif // RINEX3OBSSTREAM_HPP
