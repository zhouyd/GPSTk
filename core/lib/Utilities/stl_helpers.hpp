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
 * @file stl_helpers.hpp
 * Useful functions that take advantage of STL containers
 */

#ifndef GPSTK_STL_HELPERS_HPP
#define GPSTK_STL_HELPERS_HPP

#include <cmath>
#include <algorithm>
#include <list>
#include <vector>

namespace gpstk
{
      /** @defgroup datastructsgroup STL helpers */
      //@{

      /// A simple way to get the max value of a list of numbers
   template<class For> For max(const std::list<For>& lst) 
   {
      return *max_element(lst.begin(), lst.end());
   }
   
      /// A simple way to get the minimum value of a list of numbers
   template<class For> For min(const std::list<For>& lst) 
   {
      return *min_element(lst.begin(), lst.end());
   }

      /// ListStats (i.e. Statistics on a list of numbers)
   template<class bt>
   struct ListStats
   {
         /// number of items used in the statistics taking
      unsigned n;
         /// statistics values
      bt mean, sigma, min, max;
         /// constructor
      ListStats():n(0),mean(0),sigma(0),min(0),max(0){};
   };

      /// Compute the statistics for a list of numbers.
      /// This algorithm is written to be stable in computing the standard
      /// deviation for sequences of number with a large mean value.
      /// (i.e. it doesn't accumulate sum of the value squared.)
   template<class bt> ListStats<bt> stats(const std::list<bt>& lst)
   {
      ListStats<bt> s;
      bt sum=0, sumsq=0;

      s.n = lst.size();
      if (s.n<1)
         return s;
      
      typename std::list<bt>::const_iterator li;
      li=lst.begin();
      s.min = s.max = *li;
      for(; li!=lst.end(); li++)
      {
         s.min = std::min(s.min, *li);
         s.max = std::max(s.max, *li);
         sum += *li;
      }
      s.mean = sum/s.n;

      if (s.n<2)
         return s;

      for(li=lst.begin(); li!=lst.end(); li++)
      {
         bt z=*li-s.mean;
         sumsq += z*z;
      }
   
      s.sigma = std::sqrt( (double)(sumsq/(s.n-1)) );

      return s;
   }

      /// find the index of the first element of a vector with a given value
      /// @param vec vector<T> in which to look for value
      /// @param value T value to search for in vector
      /// @return -1 if value is not found, else index in vector of value
   template <class T> int vectorindex(const std::vector<T>& vec, const T& value) 
   {
      typename std::vector<T>::const_iterator it;
      it = std::find(vec.begin(), vec.end(), value);
      if(it == vec.end()) return -1;
      return int(it - vec.begin());
   }

      /// find the intersection of two vectors - elements common to both vectors
      /// NB STL algorithms require sorting
      /// @param v1 first vector to consider
      /// @param v2 second vector to consider
      /// @return vector<T> of elements common to v1 and v2
   template <class T> std::vector<T> vec_intersect(
                           const std::vector<T>& v1, const std::vector<T>& v2)
   {
      std::vector<T> vinter;
      typename std::vector<T>::const_iterator it;
      for(it=v1.begin(); it!=v1.end(); ++it) {
         if(std::find(v2.begin(), v2.end(), *it) != v2.end())
            vinter.push_back(*it);
      }
      return vinter;
   }

      /// find the union minus the intersection of two vectors,
      /// that is, elements that appear in either one of the two vectors but not both.
      /// NB STL algorithms require sorting
      /// @param v1 first vector to consider
      /// @param v2 second vector to consider
      /// @return vector<T> of elements not common to v1 and v2
   template <class T> std::vector<T> vec_notintersect(
                           const std::vector<T>& v1, const std::vector<T>& v2)
   {
      std::vector<T> vinter;
      typename std::vector<T>::const_iterator it;
      for(it=v1.begin(); it!=v1.end(); ++it) {
         if(std::find(v2.begin(), v2.end(), *it) == v2.end())
            vinter.push_back(*it);
      }
      for(it=v2.begin(); it!=v2.end(); ++it) {
         if(std::find(v1.begin(), v1.end(), *it) == v1.end())
            vinter.push_back(*it);
      }
      return vinter;
   }

      //@}

} // namespace
   
#endif
