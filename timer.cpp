/* **************************************************************************
   timer.cpp  timer for C/C++ programs
              20-Jun-2006 PMC gettimeofday() not available on MingW,
                              changed this to pure elapsed time only,
                              to accomodate MingW and timing problems
                              on Windoze

   Copyright 2003-2006 P.M.Cronje

   This file is part of the Double Dummer Driver (DDD).

   DDD is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   DDD is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with DDD; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "timer.h"

// *****************************************************************************
// timer workaround for Windoze,
// must define _WIN32 for compilation
// *****************************************************************************

#if defined(_WIN32)

typedef struct sFileTime
{
  unsigned int dwLowDateTime;
  unsigned int dwHighDateTime;
};

extern "C"
{
  void _stdcall GetSystemTimeAsFileTime(struct sFileTime *pft);
}
void gettimeofday(struct timeval* p, void* pv);

void gettimeofday(struct timeval* p, void* pv)
{
  union
  {
     long long ns100;           // time since 1 Jan 1601 in 100ns units
        struct sFileTime ft;

  } now;

  GetSystemTimeAsFileTime(&(now.ft));

  p->tv_usec = (int)((now.ns100 / 10LL) % 1000000LL);
  p->tv_sec  = (int)((now.ns100 - (116444736000000000LL))/10000000LL);

}

#endif

// *****************************************************************************
// cTimer
// *****************************************************************************

cTimer::cTimer()
{
  bStarted = false;

  dElapsed = prevdElapsed = deltaElapsed = 0.0;

} // cTimer::cTimer
// *****************************************************************************

cTimer::~cTimer()
{
  stop();

} // cTimer::~cTimer
// *****************************************************************************

void cTimer::check()
{
  // find elapsed statistics

  if(bStarted)
  {
    getTimerInfo(&dElapsed);

    deltaElapsed = dElapsed - prevdElapsed;
    prevdElapsed = dElapsed;

    dElapsed -= dElapsed0;
  }
  else
    start();

} // cTimer::check
// *****************************************************************************

void cTimer::getFormattedTime(char sztime[32])
{
  // DD-MON-YYYY HH:MM:SS

  time_t timeval;
  struct tm *ptm;

  // find the current date and time
  time(&timeval);
  ptm = localtime(&timeval);
  strftime(sztime,21,"%d-%b-%Y %H:%M:%S",ptm);

} // cTimer::getFormattedTime
// *****************************************************************************

void cTimer::getTimerInfo(double *pdelapsed)
{
  struct timeval tv;

  // elapsed
  gettimeofday(&tv,0);
  *pdelapsed = (double)tv.tv_sec + 0.000001 * (double)tv.tv_usec;
  if(*pdelapsed < 0.000001)
    *pdelapsed = 0.000001;

} // cTimer::getTimerInfo
// *****************************************************************************

void cTimer::start()
{
  getTimerInfo(&dElapsed0);

  dElapsed = dElapsed0;

  prevdElapsed  = dElapsed;
  deltaElapsed = 0.0;

  bStarted = true;

} // cTimer::start
// *****************************************************************************

void cTimer::stop()
{
  if(bStarted)
  {
    check();
    bStarted = false;
  }

} // cTimer::stop
// *****************************************************************************

