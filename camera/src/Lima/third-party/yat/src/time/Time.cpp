//----------------------------------------------------------------------------
// YAT LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (C) 2006-2012  The Tango Community
//
// Part of the code comes from the ACE Framework (asm bytes swaping code)
// see http://www.cs.wustl.edu/~schmidt/ACE.html for more about ACE
//
// The thread native implementation has been initially inspired by omniThread
// - the threading support library that comes with omniORB. 
// see http://omniorb.sourceforge.net/ for more about omniORB.
//
// Contributors form the TANGO community:
// see AUTHORS file 
//
// The YAT library is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
//
// The YAT library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// See COPYING file for license details 
//
// Contact:
//      Nicolas Leclercq
//      Synchrotron SOLEIL
//------------------------------------------------------------------------------
/*!
 * \author S.Poirier - Synchrotron SOLEIL
 */

//=============================================================================
// DEPENDENCIES
//=============================================================================
#include <yat/time/Time.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if (defined WIN32 || defined _WIN32)
  #include <sys\timeb.h>
  #define _X86_
  #include <windef.h>
  #include <winbase.h>
  #include <winnls.h>
#elif (defined _linux_ || defined __linux__)
  #include <sys/time.h>
#endif

// standard library objets
#include <iostream>
#include <time.h>

namespace yat
{

/// Absolute reference : 0 january 4713 b JC at 12h ( -> -4712 for the calculations )
const int REF_YEAR = -4712;

/// Julian date for UNIX reference
const int JULIAN_REF_UNIX = 2440588;

// Error msgs
const char DATE_SET_ERR[]        = "BAD_ARGS";

// Reasons msgs
const char DATE_BAD_ARGS[]        = "Cannot Setting Date object";

//============================================================================
// Internal functions
//============================================================================

// Number of days between 1th march 0 and 1th march iY
long DeltaDaysYear(int iY)
{
  return 365l * iY +
   ((long)iY/4l) -     // leap year
   ((long)iY/100l) +   // 100th are not leap
   ((long)iY/400l);    // 400th are leap years
}

// Number of days from 1th march to 1th <iM>
long DeltaDaysMonth(int iM)
{
  return (306l*iM + 4l)/10l;
}

//----------------------------------------------------------------------------
// Calculate julian date for a day at 12.
// piDeltaDays : pointer to delta with julian calendar without the reform (in 1582)
//----------------------------------------------------------------------------
long calc_julian_date(int iY, int iM, int iD, int *piDeltaDays=NULL)
{
  int flGregLeap = 0;
  int flJulianLeap = 0;
  int iYMod = iY % 4;
  int iYFromRef = iY - REF_YEAR;
  int iDeltaDays = 0;
  // Number of days in Julian calendar
  long lNbDays = iYFromRef * 365L + ((iYFromRef + 3) / 4);

  // Leap year candidate
  if( iYMod == 0 )
  {
    flJulianLeap = 1;
    flGregLeap = 1;
  }

  // Year in Gregorian calendar
  // Applying reform : 
  if( iY > 1582 || (iY == 1582 && iM > 12 ) || (iY == 1582 && iM == 12 && iD > 19) )
    iDeltaDays = 10;

  // Calculating delta
  int iDM = 0;
  if( iY >= 1700 )
    iDM = -((iY - 1) / 100) + ((iY - 1) / 400) + 12;

  if( (iY % 400) && !(iY % 100) )
    flGregLeap = 0;

  iDeltaDays -= iDM;

  // Add number of days
  if( iM <= 8 )
    lNbDays += (iM-1) * 30 + iM/2;
  else
    lNbDays += (iM-1) * 30 + (iM-1) / 2 + 1;

  if( iM >= 3 && flGregLeap )
    lNbDays--;
  else if( iM >= 3 && !flGregLeap )
  {
    lNbDays += flJulianLeap - 2;
    iDeltaDays += flJulianLeap;
  }

  if( piDeltaDays )
    // Returns delta between Gregorian et Julian calandar
    *piDeltaDays = iDeltaDays;

  // Julian day in right calendar
  return lNbDays + iD - 1 - iDeltaDays;
}

//----------------------------------------------------------------------------
// Calculate Julian day date
//----------------------------------------------------------------------------
void JJToDate(long lJJ, int16 *piYear, uint8 *puiMonth, uint8 *puiDay)
{
  int iYear, iMonth=0, iDay;
  // Retreive year
  int iJulianYear = (int)(lJJ / 365.25);
  iYear =  iJulianYear + REF_YEAR;
  int iDeltaDays;
  // Retreive le julian day for first day of this year
  long lJJBegYear = calc_julian_date(iYear, 1, 1, &iDeltaDays);

  // Position in respect to the reform
  int iDJ = int(lJJ - lJJBegYear);
  int flLeap = 0;

  if( lJJ < 2299226.5 )
  {
    iDJ -= iDeltaDays;

    if( (iYear % 4) == 0 )
      flLeap = 1;
  }
  else if( iYear == 1582 ) // && dJJ >= 2299226.5
    iDJ += 10;

  if( flLeap == 0 )
  {
    if( (iYear % 4) == 0 )
      flLeap = 1;
  }

  if( lJJ >= 2299226.5 && (iYear % 100) == 0 && (iYear % 400) != 0 )
    flLeap = 0;

  int iDM = 0;
  if( flLeap && iDJ == 59 )
  {
    // 29th februar
    iDM = 30;
    iMonth = 2;
  }

  if( flLeap && iDJ > 59 )
    // Substract leap day
    iDJ -= 1;

  // Retreive month
  iDJ -= 1;
  if( iDJ >= 333 )
  {
    iMonth = 12;
    iDM = 333;
  }
  else if( iDJ >= 303 )
  {
    iMonth = 11;
    iDM = 303;
  }
  else if( iDJ >= 272 )
  {
    iMonth = 10;
    iDM = 272;
  }
  else if( iDJ >= 242 )
  {
    iMonth = 9;
    iDM = 242;
  }
  else if( iDJ >= 211 )
  {
    iMonth = 8;
    iDM = 211;
  }
  else if( iDJ >= 180 )
  {
    iMonth = 7;
    iDM = 180;
  }
  else if( iDJ >= 150 )
  {
    iMonth = 6;
    iDM = 150;
  }
  else if( iDJ >= 119 )
  {
    iMonth = 5;
    iDM = 119;
  }
  else if( iDJ >= 89 )
  {
    iMonth = 4;
    iDM = 89;
  }
  else if( iDJ >= 58 && iDM == 0 )
  {
    iMonth = 3;
    iDM = 58;
  }
  else if( iDJ >= 30 && iDM == 0 )
  {
    iMonth = 2;
    iDM = 30;
  }
  else if( iDM == 0 )
  {
    iMonth = 1;
    iDM = -1;
  }

  // Retreive day
  iDay = iDJ - iDM + 1;

  if( iDJ > 333 && iDay > 31 )
  {
    iYear += 1;
    iDay -= 31;
    iMonth = 1;
  }

  *piYear = iYear;
  *puiMonth = iMonth;
  *puiDay = iDay;
}

//----------------------------------------------------------------------------
// TimeZoneBias
//----------------------------------------------------------------------------
double TimeZoneBias()
{
  #if (defined WIN32 || defined _WIN32)
    TIME_ZONE_INFORMATION TimeZoneInformation;
    GetTimeZoneInformation(&TimeZoneInformation);
    double dSec = - (double)TimeZoneInformation.Bias * SEC_PER_MIN;
    return dSec;
  #elif (defined _linux_ || defined __linux__)
    struct timeval tv;
    struct timezone tzp;
    gettimeofday(&tv, &tzp);
 
    // Calculer l'heure universelle
    time_t tGM = tv.tv_sec;
    struct tm tmGM;    
    tGM = mktime(gmtime_r(&tGM, &tmGM));

    // Calculer l'heure locale
    time_t tLocal = tv.tv_sec;
    struct tm tmLocal;        
    tLocal = mktime(localtime_r(&tLocal, &tmLocal));

    double dDiff = difftime(tLocal, tGM);
    return dDiff;
  #endif
}

//=============================================================================
// DateFields structure
//
//=============================================================================
//----------------------------------------------------------------------------
// DateFields::Clear
//----------------------------------------------------------------------------
void DateFields::clear()
{
  memset(this, 0, sizeof(*this));
}

//----------------------------------------------------------------------------
// DateFields::is_empty
//----------------------------------------------------------------------------
int DateFields::is_empty() const
{
  return year == 0 && 
         month == 0 && 
         day == 0 && 
         hour == 0 && 
         min == 0 &&
         sec == 0.;
}

//=============================================================================
// Time class
//
//=============================================================================

//----------------------------------------------------------------------------
// Time::nb_days_in_month
//----------------------------------------------------------------------------
uint8 Time::nb_days_in_month(uint8 iMonth, int16 iYear)
{
  switch( iMonth )
  {
    case 1: // january
    case 3: // march
    case 5: // may
    case 7: // july
    case 8: // august
    case 10:// october
    case 12:// december
      return 31;
   
    case 2: // februar : depend of year
      if( (iYear%4 == 0 && iYear < 1582) ||
          (iYear%4 == 0 && ( (iYear%100) || !(iYear%400) )) )
        return 29; // leap years
      else 
        return 28; // ordinary years

    default : // other months
      return 30;
  }
}

//----------------------------------------------------------------------------
// Time::nb_days_in_year
//----------------------------------------------------------------------------
uint16 Time::nb_days_in_year(int16 iYear)
{
  if( !(iYear%4) && ( (iYear%100) || !(iYear%400) ) )
    // Leap year
    return 366;

  return 365;
}

//----------------------------------------------------------------------------
// Time::month_name
//----------------------------------------------------------------------------
pcsz Time::month_name(uint8 iMonth)
{
  if( iMonth > 1 && iMonth < 13 )
    return s_pszMonth[iMonth-1];

  return "";
}

//----------------------------------------------------------------------------
// Time::set_current
//----------------------------------------------------------------------------
void Time::set_current(bool bUt)
{  
  #ifdef WIN32
    SYSTEMTIME sysTm;
    if( bUt )
      GetSystemTime(&sysTm);
    else
      GetLocalTime(&sysTm);
    set(sysTm.wYear, (uint8)sysTm.wMonth, (uint8)sysTm.wDay, (uint8)sysTm.wHour, (uint8)sysTm.wMinute, 
        (double)sysTm.wSecond + ((double)sysTm.wMilliseconds)/1000.0);

  #else
    long lTm, lMs;
    struct timeval tv;
    struct timezone tzp;
    gettimeofday(&tv, &tzp);
    lTm = tv.tv_sec;
    lMs = tv.tv_usec/1000;

    // Convert from 'time_t' format to 'struct tm' format
    struct tm tmCurrent;
    if (bUt)
      gmtime_r(&lTm, &tmCurrent);
    else
      localtime_r(&lTm, &tmCurrent);

    set(tmCurrent.tm_year+1900, tmCurrent.tm_mon+1, tmCurrent.tm_mday, tmCurrent.tm_hour, 
        tmCurrent.tm_min, (double)tmCurrent.tm_sec + ((double)lMs /1000.));
  #endif
}

//----------------------------------------------------------------------------
// Time::local_to_UT
//----------------------------------------------------------------------------
void Time::local_to_UT()
{
  *this -= TimeZoneBias();
}

//----------------------------------------------------------------------------
// Time::UT_to_local
//----------------------------------------------------------------------------
void Time::UT_to_local()
{
  *this += TimeZoneBias();
}

//----------------------------------------------------------------------------
// Time::get
//----------------------------------------------------------------------------
void Time::get(DateFields *pDF) const
{              
  // Retreive Julian date
  long lJJ = julian_date();

  // Day of week
  pDF->day_of_week = (int)(lJJ % 7) + 2;
  if( pDF->day_of_week > 7 )
    pDF->day_of_week -= 7;

  // Set monday as first day of week
  pDF->day_of_week -= 1;
  if( pDF->day_of_week == 0 )
    pDF->day_of_week = 7;

  // Retreive date
  JJToDate(lJJ, &pDF->year, &pDF->month, &pDF->day);

  // Hour, minutes, seconds
  uint32 ulSec = uint32((m_llTime / int64(MICROSEC_PER_SEC)) % int64(SEC_PER_DAY));
  pDF->hour = (uint8)(ulSec / SEC_PER_HOUR);
  ulSec -= pDF->hour * SEC_PER_HOUR;
  pDF->min = (uint8)(ulSec / SEC_PER_MIN);
  int iSec = (uint8)(ulSec - pDF->min * SEC_PER_MIN);
  // Retreive micro-seconds
  double dMicro = double(m_llTime % MICROSEC_PER_SEC);
  pDF->sec = iSec + dMicro / (double)MICROSEC_PER_SEC;

  // Calculate day of year
  pDF->day_of_year = (int16)(julian_date() - calc_julian_date(pDF->year, 1, 1)) + 1;

  // Week of year
  int16 iDay = pDF->day_of_year - pDF->day_of_week;
  pDF->week_of_year = iDay / 7; 
  if( iDay >= 0 )
    pDF->week_of_year++;
}

//----------------------------------------------------------------------------
// Time::set
//----------------------------------------------------------------------------
void Time::set(const DateFields &df) throw(yat::Exception)
{
  if( df.is_empty() )
    // Empty date
    m_llTime = 0;
  else
  {
    uint8 uiMonth = df.month;
    int16 iYear  = df.year;

    // Check fields
    if( uiMonth > 12 ||  
        df.day < 1 || df.day > nb_days_in_month(uiMonth, iYear) || 
        df.hour > 24 ||
        df.min > 59 || 
        df.sec < 0 || df.sec >= 60. )
    {
      throw Exception(DATE_SET_ERR, DATE_BAD_ARGS, "Time::Set");
    }

    // Julian day at 12.
    double dJJ = calc_julian_date(df.year, df.month , df.day);
    m_llTime = int64(dJJ * SEC_PER_DAY) * int64(MICROSEC_PER_SEC) + 
               int64(df.hour * SEC_PER_HOUR) * int64(MICROSEC_PER_SEC) +
               int64(df.min * SEC_PER_MIN) * int64(MICROSEC_PER_SEC) +
               int64(df.sec * MICROSEC_PER_SEC);
  }
}
 
//----------------------------------------------------------------------------
// Time::set
//----------------------------------------------------------------------------
void Time::set(int16 iYear, uint8 uiMonth, uint8 uiDay, 
               uint8 uiHour, uint8 uiMin, double dSec)
{
  DateFields df;
  df.year  = iYear;
  df.month = uiMonth;
  df.day   = uiDay;
  df.hour  = uiHour;
  df.min   = uiMin;
  df.sec   = dSec;
  set(df);
}

//----------------------------------------------------------------------------
// Time::set_day_of_year
//----------------------------------------------------------------------------
void Time::set_day_of_year(uint16 iDayOfYear, int16 iYear)
{
  DateFields df;
  get(&df);
  set(iYear, 1, 1, df.hour, df.min, df.sec);
  add_sec((iDayOfYear - 1) * SEC_PER_DAY);
}

// Macro for implementing access to date fields
#define IMPLEMENT_DATEFIELD(Name)     \
void Time::set_##Name(uint8 ui)       \
{                                     \
  DateFields df;                     \
  get(&df);                           \
  df.Name = ui;                     \
  set(df);                            \
}                                     \
                                      \
uint8 Time::Name() const               \
{                                     \
  DateFields df;                     \
  get(&df);                           \
  return df.Name;                  \
}                                     

IMPLEMENT_DATEFIELD(hour)
IMPLEMENT_DATEFIELD(day)
IMPLEMENT_DATEFIELD(month)

//----------------------------------------------------------------------------
// Time::set_minute
//----------------------------------------------------------------------------
void Time::set_minute(uint8 iMinute)
{
  DateFields df;
  get(&df);
  df.min = iMinute;
  set(df);
}

//----------------------------------------------------------------------------
// Time::minute
//----------------------------------------------------------------------------
uint8 Time::minute() const
{
  DateFields df;
  get(&df);
  return df.min;
}
 
//----------------------------------------------------------------------------
// Time::set_year
//----------------------------------------------------------------------------
void Time::set_year(int16 iYear)
{
  DateFields df;
  get(&df);
  df.year = iYear;
  set(df);
}

//----------------------------------------------------------------------------
// Time::year
//----------------------------------------------------------------------------
int16 Time::year() const
{
  DateFields df;
  get(&df);
  return df.year;
}
 
//----------------------------------------------------------------------------
// Time::set_sec
//----------------------------------------------------------------------------
void Time::set_second(double dSec)
{
  DateFields df;
  get(&df);
  df.sec = dSec;
  set(df);
}

//----------------------------------------------------------------------------
// Time::sec 
//----------------------------------------------------------------------------
double Time::second() const
{
  DateFields df;
  get(&df);
  return df.sec;
}

//----------------------------------------------------------------------------
// Time::micro
//----------------------------------------------------------------------------
long Time::micro() const
{
  return long(m_llTime % MICROSEC_PER_SEC);
}

//----------------------------------------------------------------------------
// Time::ms
//----------------------------------------------------------------------------
long Time::ms() const
{
  return long(m_llTime % MICROSEC_PER_SEC) / 1000;
}

//----------------------------------------------------------------------------
// Time::julian_date
//----------------------------------------------------------------------------
long Time::julian_date() const
{
  return long(m_llTime / MICROSEC_PER_DAY);
}

//----------------------------------------------------------------------------
// Time::set_julian_date
//----------------------------------------------------------------------------
void Time::set_julian_date(long lJulianDate)
{
  m_llTime = lJulianDate * MICROSEC_PER_DAY;
}

//----------------------------------------------------------------------------
// Time::day_of_week 
//----------------------------------------------------------------------------
uint8 Time::day_of_week() const
{
  DateFields df;
  get(&df);
  return df.day_of_week;
}

//----------------------------------------------------------------------------
// Time::day_of_year
//----------------------------------------------------------------------------
uint16 Time::day_of_year() const
{
  long lJStartDate = calc_julian_date(year(), 1, 1);
  return (int)(julian_date() - lJStartDate) + 1;
}

//----------------------------------------------------------------------------
// Time::week_of_year
//----------------------------------------------------------------------------
uint8 Time::week_of_year() const
{
  DateFields df;
  get(&df);
  return df.week_of_year;
}

//----------------------------------------------------------------------------
// Time::long_unix()
//----------------------------------------------------------------------------
long Time::long_unix() const
{
  return (long)(((m_llTime / MICROSEC_PER_SEC) - int64(JULIAN_REF_UNIX) * int64(SEC_PER_DAY)));
}

//----------------------------------------------------------------------------
// Time::set_long_unix
//----------------------------------------------------------------------------
void Time::set_long_unix(long lRefSec)
{
  m_llTime = (int64(JULIAN_REF_UNIX) * int64(SEC_PER_DAY) + int64(lRefSec)) * int64(MICROSEC_PER_SEC);
}

//----------------------------------------------------------------------------
// Time::double_unix()
//----------------------------------------------------------------------------
double Time::double_unix() const
{
  return double(((m_llTime / MICROSEC_PER_SEC) - int64(JULIAN_REF_UNIX) * int64(SEC_PER_DAY)))
         + ms() / 1000.0;
}

//----------------------------------------------------------------------------
// Time::set_double_unix
//----------------------------------------------------------------------------
void Time::set_double_unix(double dRefSec)
{
  m_llTime = (int64(JULIAN_REF_UNIX) * int64(SEC_PER_DAY) * int64(1000) + 
             int64(dRefSec * 1000)) * int64(1000);
}

//----------------------------------------------------------------------------
// Time::to_local_ISO8601
//----------------------------------------------------------------------------
String Time::to_local_ISO8601() const
{
  String strDate;
  String strFmt;

  double dTZ = TimeZoneBias();
  if( dTZ < 0 )
    strFmt = "%04d-%02d-%02dT%02d:%02d:%02d-%02d%02d";
  else
    strFmt = "%04d-%02d-%02dT%02d:%02d:%02d+%02d%02d";

  // Prevent any rounding error
  if( dTZ < 0 )
    dTZ = -dTZ;

  int iSec = int(dTZ + 0.5);
   
  int iHourBias = iSec / 3600;
  int iMinBias = (iSec - (3600 * iHourBias)) / 60;

  // Split date into fields
  DateFields df;
  get(&df);
  
  strDate = String::str_format(PSZ(strFmt), df.year, df.month, df.day,
                              df.hour, df.min, uint32(df.sec + 0.5), iHourBias, iMinBias);
  return strDate;
}

//----------------------------------------------------------------------------
// Time::to_ISO8601
//----------------------------------------------------------------------------
String Time::to_ISO8601() const
{
  String strDate;
  String strFmt = "%04d-%02d-%02dT%02d:%02d:%02dZ";

  // Split date into fields
  DateFields df;
  get(&df);
  
  strDate = String::str_format(PSZ(strFmt), df.year, df.month, df.day,
                              df.hour, df.min, uint32(df.sec + 0.5));
  return strDate;
}

//----------------------------------------------------------------------------
// Time::to_ISO8601_ms_TU
//----------------------------------------------------------------------------
String Time::to_ISO8601_ms_TU() const
{
  String strDate;
  String strFmt = "%04d-%02d-%02dT%02d:%02d:%06.3lfZ";

  // Split date into fields
  DateFields df;
  get(&df);
  
  strDate = String::str_format(PSZ(strFmt), df.year, df.month, df.day,
                              df.hour, df.min, df.sec);
  return strDate;
}

//----------------------------------------------------------------------------
// Time::to_ISO8601_ms
//----------------------------------------------------------------------------
String Time::to_ISO8601_ms() const
{
  String strDate;
  String strFmt;

  strFmt = "%04d-%02d-%02dT%02d:%02d:%06.3lf";

  // Split date into fields
  DateFields df;
  get(&df);
  
  strDate = String::str_format(PSZ(strFmt), df.year, df.month, df.day,
                              df.hour, df.min, df.sec);
  return strDate;
}

//----------------------------------------------------------------------------
// Time::to_local_ISO8601_ms
//----------------------------------------------------------------------------
String Time::to_local_ISO8601_ms() const
{
  String strDate;
  String strFmt;

  double dTZ = TimeZoneBias();
  if( dTZ < 0 )
    strFmt = "%04d-%02d-%02dT%02d:%02d:%06.3lf-%02d%02d";
  else
    strFmt = "%04d-%02d-%02dT%02d:%02d:%06.3lf+%02d%02d";

  // Prevent any rounding error
  if( dTZ < 0 )
    dTZ = -dTZ;

  int iSec = int(dTZ + 0.5);
   
  int iHourBias = iSec / 3600;
  int iMinBias = (iSec - (3600 * iHourBias)) / 60;

  // Split date into fields
  DateFields df;
  get(&df);
  
  strDate = String::str_format(PSZ(strFmt), df.year, df.month, df.day,
                              df.hour, df.min, df.sec, iHourBias, iMinBias);
  return strDate;
}

//----------------------------------------------------------------------------
// Time::to_inter
//----------------------------------------------------------------------------
String Time::to_inter(bool bMillis) const
{
  String strDate;
  String strFmt;

  if( bMillis )
    strFmt = "%04d-%02d-%02d %02d:%02d:%06.3lf";
  else
    strFmt = "%04d-%02d-%02d %02d:%02d:%02d";

  // Split date into fields
  DateFields df;
  get(&df);
  
  if( bMillis )
    strDate = String::str_format(PSZ(strFmt), df.year, df.month, df.day,
                              df.hour, df.min, df.sec);
  else
    strDate = String::str_format(PSZ(strFmt), df.year, df.month, df.day,
                              df.hour, df.min, int(df.sec+0.5));

  return strDate;
}

//----------------------------------------------------------------------------
// Time::UnixTime
//----------------------------------------------------------------------------
int32 Time::unix_time()
{
  return CurrentTime().long_unix();
}

//===========================================================================
//
// CurrentTime
//
//===========================================================================
//----------------------------------------------------------------------------
// CurrentTime::CurrentTime
//----------------------------------------------------------------------------
CurrentTime::CurrentTime(bool bUT)
{
  set_current(bUT);
}

}