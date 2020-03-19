#pragma once

#if defined(_WIN32)

#define WINDOWS_LEAN_AND_MEAN
#include <process.h>
#include <windows.h>
#undef min
#undef max
typedef __int64 TimeTics;

//! Return current time
inline TimeTics tics()
{
  LARGE_INTEGER r;
  QueryPerformanceCounter(&r);
  return r.QuadPart;
}

//! Return tics in X ms
inline TimeTics ms2tics(long long ms)
{
  LARGE_INTEGER f, r;
  QueryPerformanceFrequency(&f);
  r.QuadPart = ms * f.QuadPart / 1000ll;
  return r.QuadPart;
}

#else // Linux
#include <ctime>
#include <sys/time.h>
typedef long long TimeTics;

//! Return current time
inline TimeTics tics()
{
  timeval t;
  gettimeofday(&t, NULL);
  return (TimeTics)t.tv_sec*1000000LL + (TimeTics)t.tv_usec;
}

//! Return tics in X ms
inline TimeTics ms2tics(long int ms)
{
  return (TimeTics)(ms)*1000LL;
}

#endif


