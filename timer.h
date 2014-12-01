/* **************************************************************************
   timer.h timer for C/C++ programs
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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with DDD; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

   ************************************************************************** */

#ifndef TIMER_H
#define TIMER_H

// -----------------------------------------------------------------------------
// Timer - elapsed seconds
// -----------------------------------------------------------------------------

class cTimer
{
  public:
    // constructor
    // sets elapsed variables to 0
    cTimer();

    // destructor
    ~cTimer();

    // current date/time 'dd-mon-yyyy hh:mm:ss'
    //
    static void getFormattedTime(char sztime[32]);

    // start()
    //
    // sets start point of variables,
    // sets elapsed variables to 0
    //
    void start();

    // check(...)
    //
    // if started
    // gets elapsed time since start() or previous check()
    // if not started
    // calls start()
    //
    // may be called any number of times after start()
    //
    void check();

    // stats available only after check()/stop()
    //
    double dblElapsed()
    {
      return dElapsed;
    }
    double dblElapsed(double dmin)
    {
      return (dElapsed < dmin) ? dmin : dElapsed;
    }

    // delta elapsed time
    //
    // elapsed time from previous check(...),
    // or from start() when no previous check(...) has been called
    // NOTE: this should only be used after check(...)/stop()
    //
    double dblDeltaElapsed()
    {
      return deltaElapsed;
    }

    // stop()
    //
    // if started
    // does check(),
    // sets not started
    // if not started
    // does nothing
    //
    void stop();

    // INTERNAL read-only:

    bool bStarted;
    double dElapsed, dElapsed0,
           prevdElapsed, deltaElapsed;

    void getTimerInfo(double * pdelapsed);

}; // cTimer
// -----------------------------------------------------------------------------

#endif
