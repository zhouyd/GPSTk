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

/*
*   mergePCodeWords - Helper function used in X2Sequence and GenXSequence
         Given two bit-packed integers, merge the lower portion of the
         first and the higher portion of the second into a new bit-packed
         integer.  The first_bit argument defines the most significant
         bit of the merged word.  The bits are numbered from 0 - MAX_BIT-1.
*/

#include "gpstkplatform.h"

#ifndef MERGEPCODEWORDS_H
#define MERGEPCODEWORDS_H

   inline uint32_t merge( uint32_t w1,
                        uint32_t w2,
                        int first_bit )
   {
      uint32_t outword = w1;
      if (first_bit==0) return(outword);
      
      outword <<= first_bit;
      outword |= w2 >> (gpstk::MAX_BIT - first_bit);
      
      return(outword);
   }

#endif   // end of MERGEPCODEWORDS_H
