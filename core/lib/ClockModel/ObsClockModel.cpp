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
 * @file ObsClockModel.cpp
 * Yet another abstract class used to define an interface to a model that
 * accepts GPS observation datat and determines a clock model from it. It
 * mainly adds the ability to specify the characteristcs of the observations
 * that are to be accpeted into the model. It also defines a function that
 * accepts Observed Range Deviations and computes the mean of these (that
 * meet the selection criteria) as an estimate of the receiver clock.
 */

#include <math.h>

#include "ObsClockModel.hpp"

namespace gpstk
{
   using namespace std;


      /**
       * @throw ObjectNotFound
       */
   ObsClockModel::SvStatus ObsClockModel::getSvStatus(const SatID& svid) const
   {
      SvStatusMap::const_iterator i = status.find(svid);
      if(i == status.end())
      {
         gpstk::ObjectNotFound e("No status for SV " +
                                 StringUtils::asString(svid) +
                                 " available.");
         GPSTK_THROW(e);
      }
      else
         return i->second;
   }


   ObsClockModel& ObsClockModel::setSvModeMap(const SvModeMap& right)
      throw()
   {
      for(int prn = 1; prn <= gpstk::MAX_PRN; prn++)
         modes[SatID(prn, SatelliteSystem::GPS)] = IGNORE;

      for(SvModeMap::const_iterator i = right.begin(); i != right.end(); i++)
         modes[i->first] = i->second;

      return *this;
   }


      /**
       * @throw ObjectNotFound
       */
   ObsClockModel::SvMode ObsClockModel::getSvMode(const SatID& svid) const
   {
      SvModeMap::const_iterator i = modes.find(svid);
      if(i == modes.end())
      {
         gpstk::ObjectNotFound e("No status for SV " +
                                 StringUtils::asString(svid) +
                                 " available.");
         GPSTK_THROW(e);
      }
      else
         return i->second;
   }


      /**
       * @throw InvalidValue
       */
   gpstk::Stats<double> ObsClockModel::simpleOrdClock(const ORDEpoch& oe)
   {
      gpstk::Stats<double> stat;
      
      status.clear();

      ORDEpoch::ORDMap::const_iterator itr;
      for(itr = oe.ords.begin(); itr != oe.ords.end(); itr++)
      {
         const SatID& svid = itr->first;
         const ObsRngDev& ord=itr->second;
         switch (modes[svid])
         {
            case IGNORE: 
               status[svid] = MANUAL;
               break;
            case ALWAYS:
               status[svid] = USED;
               break;
            case HEALTHY:
               // SV Health bits are defined in ICD-GPS-200C-IRN4 20.3.3.3.1.4
               // It is a 6-bit value where the MSB (0x20) indicates a summary of
               // of NAV data health where 0 = OK, 1 = some or all BAD
               if (ord.getHealth().is_valid() && (ord.getHealth() & 0x20)) 
                  status[svid] = SVHEALTH;
               else
                  status[svid] = USED;
               break;
         }
      
         if (ord.getElevation() < elvmask)
            status[svid] = ELEVATION;

         if (ord.wonky && !useWonkyData)
            status[svid] = WONKY;

         if (status[svid] == USED)
            stat.Add(ord.getORD());
      }
   
      if (stat.N() > 2)
      {
         for (itr = oe.ords.begin(); itr != oe.ords.end(); itr++)
         {
            const SatID& svid = itr->first;

            // don't override other types of stripping
            if (status[svid] == USED)
            {
               // get absolute distance of residual from mean
               double res = itr->second.getORD();
               double dist = res - stat.Average();
               if(fabs(dist) > (sigmam * stat.StdDev()))
                  status[svid] = SIGMA;
            }
         }
   
         // now, recompute the statistics on unstripped residuals to get
         // the clock bias value
         stat.Reset();
         for (itr = oe.ords.begin(); itr != oe.ords.end(); itr++)
            if (status[itr->second.getSvID()] == USED)
               stat.Add(itr->second.getORD());
      }
         
      return stat;
   }

   void ObsClockModel::dump(ostream& s, short detail) const throw()
   {
      s << "min elev:" << elvmask
        << ", max sigma:" << sigmam
        << ", prn/status: ";
      
      ObsClockModel::SvStatusMap::const_iterator i;
      for ( i=status.begin(); i!= status.end(); i++)
         s << i->first << "/" << i->second << " ";
   }
}
