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
 * @file Chi2Distribution.hpp
 * This class implements the Chi-square distribution.
 */

#ifndef CHI2DISTRIBUTION_HPP
#define CHI2DISTRIBUTION_HPP

#include "BaseDistribution.hpp"


namespace gpstk
{

      /// @ingroup math 
      //@{

      /** This class implements the Chi-square distribution.
       *
       * A typical way to use this class follows:
       *
       * @code
       *
       *      // Declare a 'Chi2Distribution' object with 2 degrees of freedom
       *   Chi2Distribution chiObj;
       *
       *   double x(5.7);
       *
       *   cout << chiObj.pdf(x) << " | "
       *        << chiObj.cdf(x) << " | "
       *        << chiObj.Q(x) << endl;
       *
       *      // Now, the same but with four degrees of freedom
       *   cout << chiObj.pdf(x, 4) << " | "
       *        << chiObj.cdf(x, 4) << " | "
       *        << chiObj.Q(x, 4) << endl;
       *
       * @endcode
       *
       * @sa SpecialFunctions.hpp for useful functions, and
       *     GaussianDistribution.hpp for a normal distribution.
       *
       */
   class Chi2Distribution : public BaseDistribution
   {
   public:


         /// Default constructor. Sets the number of degrees of freedom to 2.
      Chi2Distribution() : ndf(2) {};


         /** Explicit constructor.
          *
          * @param n       Degrees of freedom
          *
          * \warning "n" must be > 0, otherwise n = |n|.
          */
      Chi2Distribution( int n )
      { setNDF(n); };


         /** Computes the probability density function
          *
          * @param x    Value
          */
      virtual double pdf(double x);


         /** Computes the probability density function
          *
          * @param x       Value
          * @param n       Degrees of freedom
          *
          * \warning "n" must be > 0, otherwise n = |n|.
          */
      virtual double pdf(double x, int n)
      { setNDF(n); return pdf(x); };


         /** Computes the cumulative distribution function
          *
          * @param x       Value
          */
      virtual double cdf(double x);


         /** Computes the cumulative distribution function
          *
          * @param x       Value
          * @param n       Degrees of freedom
          *
          * \warning "n" must be > 0, otherwise n = |n|.
          */
      virtual double cdf(double x, int n)
      { setNDF(n); return cdf(x); };


         /** Computes the upper tail of the Chi-square probability
          *  function Q(x, ndf).
          *
          * @param x       Value
          */
      virtual double Q(double x)
      { return ( 1.0 - cdf(x) ); };


         /** Computes the upper tail of the Chi-square probability
          *  function Q(x, n).
          *
          * @param x       Value
          * @param n       Degrees of freedom
          *
          * \warning "n" must be > 0, otherwise n = |n|.
          */
      virtual double Q(double x, int n)
      { return ( 1.0 - cdf(x,n) ); };


         /// Get number of degrees of freedom
      virtual double getNDF(void) const
      { return ndf; };


         /** Set the number of degrees of freedom.
          *
          * @param n       Degrees of freedom
          *
          * \warning "n" must be > 0, otherwise n = |n|.
          * @throw InvalidParameter if n == 0
          */
      virtual Chi2Distribution& setNDF(int n);


   private:


         /// Number of degrees of freedom
      int ndf;


   };  // End of class "Chi2Distribution"

      //@}

}  // End of namespace gpstk
#endif   // CHI2DISTRIBUTION_HPP
