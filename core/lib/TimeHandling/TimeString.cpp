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

/// @file TimeString.cpp  print and scan using all TimeTag derived classes.

#include "TimeString.hpp"

#include "ANSITime.hpp"
#include "CivilTime.hpp"
#include "GPSWeekSecond.hpp"
#include "BDSWeekSecond.hpp"
#include "GALWeekSecond.hpp"
#include "QZSWeekSecond.hpp"
#include "IRNWeekSecond.hpp"
#include "GPSWeekZcount.hpp"
#include "JulianDate.hpp"
#include "MJD.hpp"
#include "UnixTime.hpp"
#include "PosixTime.hpp"
#include "YDSTime.hpp"

#include "TimeConverters.hpp"
#include "TimeConstants.hpp"

using namespace std;

namespace gpstk
{
   string printTime( const CommonTime& t,
                          const string& fmt )
   {
      try
      {
         string rv( fmt );
         try {rv = ANSITime(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = CivilTime(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = GPSWeekSecond(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = GPSWeekZcount(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = JulianDate(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = MJD(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = UnixTime(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = PosixTime(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = YDSTime(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = GALWeekSecond(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = BDSWeekSecond(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = QZSWeekSecond(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         try {rv = IRNWeekSecond(t).printf( rv );} catch (gpstk::InvalidRequest e){};
         return rv;
      }
      catch( gpstk::StringUtils::StringException& se )
      {
         GPSTK_RETHROW( se );
      }
   }
   
      /// Fill the TimeTag object \a btime with time information found in
      /// string \a str formatted according to string \a fmt.
   void scanTime( TimeTag& btime,
                  const string& str,
                  const string& fmt )
   {
      try
      {
            // Get the mapping of character (from fmt) to value (from str).
         TimeTag::IdToValue info;
         TimeTag::getInfo( str, fmt, info );
         
         if( btime.setFromInfo( info ) )
         {
            return;
         }
         
            // Convert to CommonTime, and try to set using all formats.
         CommonTime ct( btime.convertToCommonTime() );
         scanTime( ct, str, fmt );

            // Convert the CommonTime into the requested format.
         btime.convertFromCommonTime( ct );
      }
      catch( gpstk::InvalidRequest& ir )
      {
         GPSTK_RETHROW( ir );
      }
      catch( gpstk::StringUtils::StringException& se )
      {
         GPSTK_RETHROW( se );
      }
   }
   
   void scanTime( CommonTime& t,
                  const string& str,
                  const string& fmt )
   {
      try
      {
         using namespace gpstk::StringUtils;

            // Get the mapping of character (from fmt) to value (from str).
         TimeTag::IdToValue info;
         TimeTag::getInfo( str, fmt, info );
         
            // These indicate which information has been found.
         bool hmjd( false ), hsow( false ), hweek( false ), hfullweek( false ),
            hdow( false ), hyear( false ), hmonth( false ), hday( false ),
            hzcount( false ), hdoy( false ), hzcount29( false ), 
            hzcount32( false ), hhour( false ), hmin( false ), hsec( false ),
            hsod( false ), hunixsec( false ), hunixusec( false ), 
            hepoch( false ), hansi( false ), hjulian( false ),
            hbdsw( false ), hqzsw( false ), hgalw( false ), hirnw( false ),
            hbdsfw( false ), hqzsfw( false ), hgalfw( false ), hirnfw( false ),
            hbdse( false ), hqzse( false ), hgale( false), hirne( false ),
            hposixsec( false ), hposixnsec( false );

            // These are to hold data that no one parses.
         int idow(0);
         TimeSystem ts;

         for( TimeTag::IdToValue::iterator itr = info.begin();
              itr != info.end(); itr++ )
         {
            switch( itr->first )
            {
               case 'P':
                  ts = gpstk::StringUtils::asTimeSystem(itr->second);
                  t.setTimeSystem(ts);
                  break;

               case 'Q':
                  hmjd = true;
                  break;

               case 'Z':
               case 'z':
                  hzcount = true;
                  break;

               case 's':
                  hsod = true;
                  break;

               case 'g':
                  hsow = true;
                  break;

               case 'w':
                  idow = asInt( itr->second );
                  hdow = true;
                  break;

               case 'G':
                  hweek = true;
                  break;

               case 'F':
                  hfullweek = true;
                  break;

               case 'j':
                  hdoy = true;
                  break;

               case 'b':
               case 'B':
                  hmonth = true;
                  break;

               case 'Y':
               case 'y':
                  hyear = true;
                  break;

               case 'a':
               case 'A':
                  {
                     hdow = true;
                     string thisDay = firstWord( itr->second );
                     lowerCase(thisDay);
                     if (isLike(thisDay, "sun.*")) idow = 0;
                     else if (isLike(thisDay, "mon.*")) idow = 1;
                     else if (isLike(thisDay, "tue.*")) idow = 2;
                     else if (isLike(thisDay, "wed.*")) idow = 3;
                     else if (isLike(thisDay, "thu.*")) idow = 4;
                     else if (isLike(thisDay, "fri.*")) idow = 5;
                     else if (isLike(thisDay, "sat.*")) idow = 6;
                     else
                     {
                        hdow = false;
                     }
                  }
                  break;
                  
               case 'm':
                  hmonth = true;
                  break;

               case 'd':
                  hday = true;
                  break;

               case 'H':
                  hhour = true;
                  break;

               case 'M':
                  hmin = true;
                  break;

               case 'S':
                  hsec = true;
                  break;

               case 'f':
                  hsec = true;
                  // a small hack to make fractional seconds work
                  info['S'] = info['f'];
                  break;

               case 'U':
                  hunixsec = true;
                  break;

               case 'u':
                  hunixusec = true;
                  break;

               case 'W':
                  hposixsec = true;
                  break;

               case 'N':
                  hposixnsec = true;
                  break;
                  
               case 'c':
                  hzcount29 = true;
                  break;

               case 'C':
                  hzcount32 = true;
                  break;

               case 'J':
                  hjulian = true;
                  break;
                  
               case 'K':
                  hansi = true;
                  break;
                  
               case 'E':
                  hepoch = true;
                  break;

               case 'R': hepoch = hbdse = true; break;
               case 'T': hepoch = hgale = true; break;
               case 'V': hepoch = hqzse = true; break;
               case 'X': hepoch = hirne = true; break;

               case 'D': hfullweek = hbdsfw = true; break;
               case 'e': hweek = hbdsw = true; break;
               case 'L': hfullweek = hgalfw = true; break;
               case 'l': hweek = hgalw = true; break;
               case 'h': hfullweek = hqzsfw = true; break;
               case 'i': hweek = hqzsw = true; break;
               case 'O': hfullweek = hirnfw = true; break;
               case 'o': hweek = hirnw = true; break;


               default:
                  {
                     // do nothing
                  }
                  break;

            };
         }     // end loop over Id/Value pairs

         if( hyear )
         {
            if( hmonth && hday )
            {
               CivilTime tt;
               tt.setFromInfo( info );
               if( hsod )
               {
                  convertSODtoTime( asDouble( info['s'] ), 
                                    tt.hour, tt.minute, tt.second );
               }
               t = tt.convertToCommonTime();
               return;
            }
            else  // use YDSTime as default
            {
               YDSTime tt;
               tt.setFromInfo( info );
               if( hhour && hmin && hsec )
               {
                  tt.sod = convertTimeToSOD( asInt( info['H'] ), 
                                             asInt( info['M'] ), 
                                             asDouble( info['S'] ) );
               }
               t = tt.convertToCommonTime();
               return;
            }

         } // end of if( hyear )

         if( hzcount32 ||
             (hfullweek && (hzcount || hzcount29)) ||
             (hepoch && (hzcount29 || 
                         (hweek && hzcount))) )
         {
            GPSWeekZcount tt;
            tt.setFromInfo( info );
            t = tt.convertToCommonTime();
            return;
         }

         if ( (hepoch && hweek) || hfullweek )
         {
            WeekSecond* ptt;
            if(hbdse || hbdsfw || hbdsw) ptt = new BDSWeekSecond();
            else if(hqzse || hqzsfw || hqzsw) ptt = new QZSWeekSecond();
            else if(hgale || hgalfw || hgalw) ptt = new GALWeekSecond();
            else if(hirne || hirnfw || hirnw) ptt = new IRNWeekSecond();
            else ptt = new GPSWeekSecond();
            ptt->setFromInfo(info);
            if( hdow && !hsow )
            {
               ptt->sow = asInt( info['w'] ) * SEC_PER_DAY;
               if( hsod )
               {
                  ptt->sow += asDouble( info['s'] );
               }
               else if( hhour && hmin && hsec )
               {
                  ptt->sow += convertTimeToSOD( asInt( info['H'] ), 
                                              asInt( info['M'] ), 
                                              asDouble( info['S'] ) );
               }
            }

            t = ptt->convertToCommonTime();
            delete ptt;
            return;
         }

         if( hmjd )
         {
            MJD tt;
            tt.setFromInfo( info );
            t = tt.convertToCommonTime();
            return;
         }

         if( hjulian )
         {
            JulianDate tt;
            tt.setFromInfo( info );
            t = tt.convertToCommonTime();
            return;
         }

         if( hansi )
         {
            ANSITime tt;
            tt.setFromInfo( info );
            t = tt.convertToCommonTime();
            return;
         } 
         
         if( hunixsec || hunixusec )
         {
            UnixTime tt;
            tt.setFromInfo( info );
            t = tt.convertToCommonTime();
            return;
         }

         if( hposixsec || hposixnsec )
         {
            PosixTime tt;
            tt.setFromInfo( info );
            t = tt.convertToCommonTime();
            return;
         }

         InvalidRequest ir("Incomplete time specification for readTime");
         GPSTK_THROW( ir );
      }
      catch( gpstk::StringUtils::StringException& se )
      {
         GPSTK_RETHROW( se );
      }
   }   

   void mixedScanTime( CommonTime& t,
                       const string& str,
                       const string& fmt )
   {
      try
      {
         using namespace gpstk::StringUtils;

            // Get the mapping of character (from fmt) to value (from str).
         TimeTag::IdToValue info;
         TimeTag::getInfo( str, fmt, info );
         
            // These indicate which information has been found.
         bool hsow( false ), hweek( false ), hfullweek( false ),
            hdow( false ), hyear( false ), hmonth( false ), hday( false ),
            hzcount( false ), hdoy( false ), hzcount29( false ), 
            hhour( false ), hmin( false ), hsec( false ),
            hsod( false ), hepoch( false ),
            //hunixsec( false ), hunixusec( false ),
            hbdsw( false ), hqzsw( false ), hgalw( false ), hirnw( false ),
            hbdsfw( false ), hqzsfw( false ), hgalfw( false ), hirnfw( false ),
            hbdse( false ), hqzse( false ), hgale( false ), hirne( false );

            // MJD, Julian Date, ANSI time, Unix time, Posix time, and
            // 32-bit Zcounts are treated as stand-alone types and are
            // not mixed with others if detected.
         
            // These variables will hold the values for use later.
         double isow, isod, isec;
         int iweek, ifullweek, idow, iyear, imonth, iday, izcount, idoy,
            izcount29, ihour, imin, iepoch;
         TimeSystem ts;

         for( TimeTag::IdToValue::iterator itr = info.begin();
              itr != info.end(); itr++ )
         {
            switch( itr->first )
            {
               case 'P':
                  ts = gpstk::StringUtils::asTimeSystem(itr->second);
                  t.setTimeSystem(ts);
                  break;

               case 'Q':
                  t = MJD( asLongDouble(itr->second) );
                  break;

               case 'J':
                  t = JulianDate( asLongDouble(itr->second) );
                  break;
                  
               case 'C':
                  t = GPSWeekZcount().setZcount32( asInt(itr->second) );
                  break;

               case 'K':
                  t = ANSITime( asInt(itr->second) );
                  break;
                  
               case 'U':
               case 'u':
               {
                  UnixTime tt;
                  tt.setFromInfo( info );
                  t = tt.convertToCommonTime();
                  break;
               }
               break;

               case 'N':
               case 'W':
               {
                  PosixTime tt;
                  tt.setFromInfo( info );
                  t = tt.convertToCommonTime();
                  break;
               }
               break;

               case 'z':
               case 'Z':
                  hzcount = true;
                  izcount = asInt(itr->second);
                  break;

               case 's':
                  hsod = true;
                  isod = asDouble(itr->second);
                  break;

               case 'g':
                  hsow = true;
                  isow = asDouble(itr->second);
                  break;

               case 'w':
                  idow = asInt(itr->second);
                  hdow = true;
                  break;

               case 'G':
                  hweek = true;
                  iweek = asInt(itr->second);
                  break;

               case 'F':
                  hfullweek = true;
                  ifullweek = asInt(itr->second);
                  break;

               case 'j':
                  hdoy = true;
                  idoy = asInt(itr->second);
                  break;

               case 'b':
                  hmonth = true;
                  imonth = CivilTime::monthAbbrev(itr->second);
                  if (imonth < 1)
                  {
                        ///@todo use a more appropriate exception class
                     Exception
                        exc("Invalid month abbreviation: " + itr->second);
                     GPSTK_THROW(exc);
                  }
                  break;

               case 'B':
                  hmonth = true;
                  imonth = CivilTime::monthLong(itr->second);
                  if (imonth < 1)
                  {
                        ///@todo use a more appropriate exception class
                     Exception
                        exc("Invalid month name: " + itr->second);
                     GPSTK_THROW(exc);
                  }
                  break;

               case 'Y':
                  hyear = true;
                  iyear = asInt(itr->second);
                  break;

               case 'y':
                  hyear = true;
                     // match the POSIX strptime() function:
                     /* Year within century. When a century is not
                      * otherwise specified, values in the range 69-99
                      * refer to years in the twentieth century (1969
                      * to 1999 inclusive); values in the range 00-68
                      * refer to years in the twenty-first century
                      * (2000 to 2068 inclusive). */
                  if( itr->second.length() > 2)
                  {
                        ///@todo use a more appropriate exception class
                     Exception
                        exc("Invalid format for %y: expected 2 digits");
                     GPSTK_THROW(exc);
                  }
                  iyear = asInt( itr->second );
                  if (iyear >= 69)
                     iyear += 1900;
                  else
                     iyear += 2000;
                  break;

               case 'a':
               case 'A':
               {
                  hdow = true;
                  string thisDay = firstWord( itr->second );
                  lowerCase(thisDay);
                  if (isLike(thisDay, "sun.*")) idow = 0;
                  else if (isLike(thisDay, "mon.*")) idow = 1;
                  else if (isLike(thisDay, "tue.*")) idow = 2;
                  else if (isLike(thisDay, "wed.*")) idow = 3;
                  else if (isLike(thisDay, "thu.*")) idow = 4;
                  else if (isLike(thisDay, "fri.*")) idow = 5;
                  else if (isLike(thisDay, "sat.*")) idow = 6;
               }
               break;
                  
               case 'm':
                  hmonth = true;
                  imonth = asInt(itr->second);
                  break;

               case 'd':
                  hday = true;
                  iday = asInt(itr->second);
                  break;

               case 'H':
                  hhour = true;
                  ihour = asInt(itr->second);
                  break;

               case 'M':
                  hmin = true;
                  imin = asInt(itr->second);
                  break;

               case 'S':
                  hsec = true;
                  isec = asDouble(itr->second);
                  break;

               case 'f':
                  hsec = true;
                  isec = asDouble(itr->second);
                  break;

               case 'c':
                  hzcount29 = true;
                  izcount29 = asInt(itr->second);
                  break;

               case 'E':
                  hepoch = true;
                  iepoch = asInt(itr->second);
                  break;

               case 'R':
                  hepoch = hbdse = true;
                  iepoch = asInt(itr->second);
                  break;

               case 'T':
                  hepoch = hgale = true;
                  iepoch = asInt(itr->second);
                  break;

               case 'V':
                  hepoch = hqzse = true;
                  iepoch = asInt(itr->second);
                  break;

               case 'X':
                  hepoch = hirne = true;
                  iepoch = asInt(itr->second);
                  break;

               case 'D': hfullweek = hbdsfw = true; break;
               case 'e': hweek = hbdsw = true; break;
               case 'L': hfullweek = hgalfw = true; break;
               case 'l': hweek = hgalw = true; break;
               case 'I': hfullweek = hqzsfw = true; break;
               case 'i': hweek = hqzsw = true; break;
               case 'O': hfullweek = hirnfw = true; break;
               case 'o': hweek = hirnw = true; break;

               default:
                     // do nothing
                  break;

            };
         }

         bool hbds(hbdse || hbdsfw || hbdsw);
         bool hgal(hgale || hgalfw || hgalw);
         bool hqzs(hqzse || hqzsfw || hqzsw);
         bool hirn(hirne || hirnfw || hirnw);

            // We'll copy this time to 't' after all of the processing.
         CommonTime ct(t);
         ct.setTimeSystem(t.getTimeSystem());
         
            // Go through all of the types in order of least precise to most
            // precise.
         if( hepoch ) 
         {
            WeekSecond *ptt;
            if(hbds) ptt = new BDSWeekSecond(ct);
            else if(hqzs) ptt = new QZSWeekSecond(ct);
            else if(hgal) ptt = new GALWeekSecond(ct);
            else if(hirn) ptt = new IRNWeekSecond(ct);
            else ptt = new GPSWeekSecond(ct);
            ptt->setEpoch( iepoch );
            ct = ptt->convertToCommonTime();
            delete ptt;
         }
         
         if( hyear )
         {
            YDSTime tt(ct);
            tt.year = iyear;
            ct = tt.convertToCommonTime();
         }
 
         if( hmonth )
         {
            CivilTime tt(ct);
            tt.month = imonth;
            ct = tt.convertToCommonTime();
         }

         if( hfullweek )
         {
            WeekSecond *ptt;
            if(hbds) ptt = new BDSWeekSecond();
            else if(hqzs) ptt = new QZSWeekSecond();
            else if(hgal) ptt = new GALWeekSecond();
            else if(hirn) ptt = new IRNWeekSecond();
            else ptt = new GPSWeekSecond();

            //When if( hfullweek ) is the first if entered in the list
            //of if's the conversion of CommonTime to WeekSecond
            //causes an InvalidParameter error. The following if-else
            //is a workaround.
	    if (ct == CommonTime::BEGINNING_OF_TIME)
            {
              ptt->week = ifullweek;
            }
	    else 
            {
              ptt->convertFromCommonTime(ct);
              ptt->week = ifullweek;
            }

            ct = ptt->convertToCommonTime();
            delete ptt;
         }
         
         if( hweek )
         {
            WeekSecond *ptt;
            if(hbds) ptt = new BDSWeekSecond(ct);
            else if(hqzs) ptt = new QZSWeekSecond(ct);
            else if(hgal) ptt = new GALWeekSecond(ct);
            else if(hirn) ptt = new IRNWeekSecond(ct);
            else ptt = new GPSWeekSecond(ct);
            ptt->setModWeek(iweek);
            ct = ptt->convertToCommonTime();
            delete ptt;
         }
         
         if( hdow )
         {
            WeekSecond *ptt;
            if(hbds) ptt = new BDSWeekSecond(ct);
            else if(hqzs) ptt = new QZSWeekSecond(ct);
            else if(hgal) ptt = new GALWeekSecond(ct);
            else if(hirn) ptt = new IRNWeekSecond(ct);
            else ptt = new GPSWeekSecond(ct);
            ptt->sow = static_cast<double>(idow) * SEC_PER_DAY;
            ct = ptt->convertToCommonTime();
            delete ptt;
         }
         
         if( hday )
         {
            CivilTime tt(ct);
            tt.day = iday;
            ct = tt.convertToCommonTime();
         }
         
         if( hdoy )
         {
            YDSTime tt(ct);
            tt.doy = idoy;
            ct = tt.convertToCommonTime();
         }
         
         if( hzcount29 )
         {
            GPSWeekZcount tt(ct);
            tt.setZcount29( izcount29 );
            ct = tt.convertToCommonTime(); 
         }

         if( hzcount )
         {
            GPSWeekZcount tt(ct);
            tt.zcount = izcount;
            ct = tt.convertToCommonTime();
         }

         if( hhour )
         {
            CivilTime tt(ct);
            tt.hour = ihour;
            ct = tt.convertToCommonTime();
         }

         if( hmin )
         {
            CivilTime tt(ct);
            tt.minute = imin;
            ct = tt.convertToCommonTime();
         }
         
         if( hsow )
         {
            WeekSecond *ptt;
            if(hbds) ptt = new BDSWeekSecond(ct);
            else if(hqzs) ptt = new QZSWeekSecond(ct);
            else if(hgal) ptt = new GALWeekSecond(ct);
            else if(hirn) ptt = new IRNWeekSecond(ct);
            else ptt =  new GPSWeekSecond(ct);
            ptt->sow = isow;
            ct = ptt->convertToCommonTime();
            delete ptt;
         }
         
         if( hsod )
         {
            YDSTime tt(ct);
            tt.sod = isod;
            ct = tt.convertToCommonTime();
         }

         if( hsec )
         {
            CivilTime tt(ct);
            tt.second = isec;
            ct = tt.convertToCommonTime();
         }
         t = ct;
      }
      catch( gpstk::StringUtils::StringException& se )
      {
         GPSTK_RETHROW( se );
      }
   }   

} // namespace gpstk
