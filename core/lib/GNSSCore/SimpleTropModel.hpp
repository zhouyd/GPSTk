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


#ifndef SIMPLE_TROP_MODEL_HPP
#define SIMPLE_TROP_MODEL_HPP

#include "TropModel.hpp"

namespace gpstk
{
      //---------------------------------------------------------------------------------
      /// A simple Black model of the troposphere. temp is in Kelvin.
   class SimpleTropModel : public TropModel
   {
   public:
         /// Empty constructor
      SimpleTropModel(void);

         /** Creates a trop model, with weather observation input
          * @param wx the weather to use for this correction.
          * @throw InvalidParameter
          */
      SimpleTropModel(const WxObservation& wx);

         /** Create a tropospheric model from explicit weather data
          * @param T temperature in degrees Celsius
          * @param P atmospheric pressure in millibars
          * @param H relative humidity in percent
          * @throw InvalidParameter
          */
      SimpleTropModel(const double& T,
                      const double& P,
                      const double& H);

         /// Return the name of the model
      virtual std::string name(void)
      { return std::string("Simple"); }

         /*** Compute and return the zenith delay for dry component of
          * the troposphere
          * @throw InvalidTropModel
          */
      virtual double dry_zenith_delay(void) const;

         /** Compute and return the zenith delay for wet component of
          * the troposphere
          * @throw InvalidTropModel
          */
      virtual double wet_zenith_delay(void) const;

         /** Compute and return the mapping function for dry component
          * of the troposphere
          * @param elevation Elevation of satellite as seen at
          *   receiver, in degrees
          * @throw InvalidTropModel
          */
      virtual double dry_mapping_function(double elevation) const;

         /** Compute and return the mapping function for wet component
          * of the troposphere
          * @param elevation Elevation of satellite as seen at
          *   receiver, in degrees
          * @throw InvalidTropModel
          */
      virtual double wet_mapping_function(double elevation) const;

         /** Re-define the tropospheric model with explicit weather data.
          * Typically called just before correction().
          * @param T temperature in degrees Celsius
          * @param P atmospheric pressure in millibars
          * @param H relative humidity in percent
          * @throw InvalidParameter
          */
      virtual void setWeather(const double& T,
                              const double& P,
                              const double& H);

         /** Re-define the tropospheric model with explicit weather data.
          * Typically called just before correction().
          * @param wx the weather to use for this correction
          * @throw InvalidParameter
          */
      virtual void setWeather(const WxObservation& wx);

   private:
      double Cdrydelay;
      double Cwetdelay;
      double Cdrymap;
      double Cwetmap;
   };
}
#endif
