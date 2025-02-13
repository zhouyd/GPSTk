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
 * @file RinexClockData.cpp
 * Encapsulate RINEX clock file data, including I/O
 */

#include <string>
#include "RinexClockData.hpp"
#include "RinexClockHeader.hpp"
#include "RinexClockStream.hpp"
#include "StringUtils.hpp"
#include "FFStream.hpp"
#include "FFStreamError.hpp"

using namespace std;

namespace gpstk
{
   using namespace StringUtils;

   void RinexClockData::dump(ostream& s) const
   {
      s << "Type: " << type.type << " " 
        << "Name: " << name << " "
        << "Epoch Time: " << writeTime(epochTime) << " "
        << "#of Data Values: " << dvCount;
      
      for (int i = 0; i < dvCount; i++)
      {
         if (i%4 == 0)
         {
            s << endl 
              << "   ";
         }
         
         s << clockData[i]
           << "  ";
      }
      
   }  // dump

   
   void RinexClockData::reallyPutRecord(FFStream& s) const
   {      
      if ( type != AR &&
           type != AS &&
           type != CR &&
           type != DR &&
           type != MS )
      {
            // invalid type - throw
         FFStreamError e("Invalid type: " + type.type);
         GPSTK_THROW(e);
      }

      if ( dvCount < 1 || dvCount > 6 )
      {
            // invalid dvCount - throw
         FFStreamError e("Invalid number of data values: " + asString(dvCount));
         GPSTK_THROW(e);
      }

      s << setw(2) << type.type << ' '
        << setw(4) << name << ' '
        << writeTime(epochTime) 
        << right << setw(3) << dvCount << left << "   "
        << clockData[0]
        << ' ';
      
      if (dvCount >= 2)
      {
         s << clockData[1]
           << endlpp;
      }
      else // dvCount == 1
      {
         s << endlpp;
      }
      
      if (dvCount > 2)
      {
         for (int i = 2; i < dvCount; i++)
         {
            s << clockData[i];

            if ( i < 5 )
            {
               s << ' ';
            }
         }
         s << endlpp;
      }
      
   }  // reallyPutRecord


   void RinexClockData::reallyGetRecord(FFStream& ffs)
   {
      RinexClockStream& strm = dynamic_cast<RinexClockStream&>(ffs);

         // If the header hasn't been read, read it...
      if(!strm.headerRead)
      {
         strm >> strm.header;
      }
    
         // Clear out this object
      RinexClockHeader& hdr = strm.header;
      
      RinexClockData rcd;
      *this=rcd;
      
      string line;
      
      strm.formattedGetLine(line, true);
      
      if (line.size() < 59 || line.size() > 80 )
      {
            // invalid record size - throw
         FFStreamError e("Invalid record length: " + asString(line.size()));
         GPSTK_THROW(e);
      }

      if ( line[2]  != ' ' || line[7]  != ' ' ||
           line[37] != ' ' || line[38] != ' ' )
      {
            // invalid record - throw
         FFStreamError e("Invalid clock record.");
         GPSTK_THROW(e);
      }

      string ts = upperCase(line.substr(0,2));
      if      (ts == "AR") type = AR;
      else if (ts == "AS") type = AS;
      else if (ts == "CR") type = CR;
      else if (ts == "DR") type = DR;
      else if (ts == "MS") type = MS;
      else
      {
            // invalid type - throw
         FFStreamError e("Invalid clock type: " + type.type);
         GPSTK_THROW(e);
      }

      name = line.substr(3,4);

      epochTime = parseTime(line.substr(8,26));

      dvCount = asInt(line.substr(34,3));
      if ( dvCount < 1 || dvCount > 6 )
      {
            // invalid dvCount - throw
         FFStreamError e("Invalid number of data values: " + asString(dvCount));
         GPSTK_THROW(e);
      }

      clockData[0] = line.substr(40,19);
      
      if (dvCount >= 2)
      {
         clockData[1] = line.substr(60,19);
      }

      if (dvCount > 2)
      { 
            // get continuation line
         strm.formattedGetLine(line, true);

         if (line.size() < 19 || line.size() > 80)
         {
               // invalid continuation line size - throw
            FFStreamError e("Invalid continuation line length: " + 
                            asString(line.size()));
            GPSTK_THROW(e);
         }
         
         for (int i = 2; i < dvCount; i++)
         {
            clockData[i] = line.substr( (i-2)*20, 19 );
         }
      }

   }  // reallyGetRecord


}  // namespace


