// precise.c -- measure time using precision timer
//
// based on sample code from Microsoft: see
//  http://msdn.microsoft.com/library/techart/optcode.htm#optcode_topic4
//
// in the original example, you passed a function into a timing
// routine and got back a number.
//
// in my modifications, you call precise_start() to start timing, and then
// precise_stop() to stop timing. The result is a double measuring seconds
// (as in the original code), so that you do not have to deal with strange 
// units and clock frequencies.
//
// the original code tries to run pending Windows messages before calling
// the function, and sets the priority to the highest possible, but this 
// code does not. I recommend that you take the minimum, maximum, or mean 
// over many calls, depending upon what you are looking for.
//
// finally, the original code used LARGE_INTEGER rather than __int64, which
// is not supported by earlier compilers, but which is much easier to use.
// I converted the code to use __int64, which is much easier to understand.

#include "windows.h"
#include "time.h"
#include "precise.h"

static enum { ttuUnknown, ttuHiRes, ttuClock } TimerToUse = ttuUnknown;
static LARGE_INTEGER PerfFreq;  // ticks per second
static __int64 OverheadTicks;   // overhead in calling timer
static LARGE_INTEGER tStart;


// precision_read -- this function acts just like precision_stop(), but
// it returns the raw timer value. It is used to estimate the overhead
// of calling precision_stop().
//
static __int64 precise_read()
{
    LARGE_INTEGER tStop;
    if (TimerToUse == ttuHiRes) {
        QueryPerformanceCounter(&tStop);
    } else {
        tStop.QuadPart = 0; // this branch should never be taken
    }
    return tStop.QuadPart - tStart.QuadPart;
}


// DetermineTimer -- see if we can use the precision timer
// 
// If the precision timer can be used, the timer frequency and
// the overhead of calling timer routines is estimated for use
// in real measurements by calling precision_start() & _stop() functions.
//
static void DetermineTimer()
{
    // Assume the worst
    TimerToUse = ttuClock;
    if (QueryPerformanceFrequency(&PerfFreq)) {
        // We can use hires timer, determine overhead
        TimerToUse = ttuHiRes;
        OverheadTicks = 200;
        for (int i = 0; i < 20; i++) {
            __int64 Ticks;
            precise_start();
            Ticks = precise_read();
            if (Ticks >= 0 && Ticks < OverheadTicks)
                OverheadTicks = Ticks;
        }
    }
    return;
}


void precise_start()
{
    if (TimerToUse == ttuUnknown) DetermineTimer();
    if (TimerToUse == ttuHiRes) {
        QueryPerformanceCounter(&tStart);
    } else {
        tStart.QuadPart = clock();
    }
}


double precise_stop()
{
    LARGE_INTEGER tStop;
    double time;
    if (TimerToUse == ttuHiRes) {
        QueryPerformanceCounter(&tStop);
        time = ((double) (tStop.QuadPart - tStart.QuadPart - 
                          OverheadTicks)) / PerfFreq.QuadPart;
    } else {
        tStop.QuadPart = clock();
        time = ((double) (tStop.QuadPart - tStart.QuadPart)) /
               CLOCKS_PER_SEC;
    }
    return time;
}

