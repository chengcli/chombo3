#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#ifndef _CLOCKTICKS_H_
#define _CLOCKTICKS_H_

#include <chrono>

// (dfm 4/29/08)note that NamespaceHeader.H gets included 3x, once for
// each "if"...

#if defined(__INTEL_COMPILER) && defined(__ia64__)
#include <ia64intrin.h>
#include <ia64regs.h>
#include "BaseNamespaceHeader.H"

inline unsigned long long int ch_ticks()
{
  volatile unsigned long long int rtn = __getReg(_IA64_REG_AR_ITC);
  return rtn;
}
#define CH_TICKS

#elif defined(__x86_64__) || defined(__i386__) || defined(__i686__)
#include "BaseNamespaceHeader.H"

union clockunion
{
  unsigned int i[2];
  unsigned long long int l;
};

inline unsigned long long int ch_ticks()
{
  volatile clockunion ret;
  __asm__ __volatile__ ("rdtsc" : "=a"(ret.i[0]), "=d"(ret.i[1]));
  return ret.l;

  // for some reason this form executes on 64, bit acts funny.
  //  unsigned long long int rtn;
  //  __asm__ __volatile__("rdtsc" : "=A"(rtn));
  //  return rtn;
}
#define CH_TICKS

#elif defined(_POWER) || defined(_POWERPC) || defined(__powerpc__)
#include "BaseNamespaceHeader.H"

inline unsigned long long int ch_ticks()
{
  volatile unsigned long long int tbr;
  __asm__ __volatile__("mftb %[tbr]" : [tbr] "=r" (tbr):);
  return tbr;
}
#define CH_TICKS
#else
inline unsigned long long int ch_ticks()
{
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
#define CH_TICKS
#include "BaseNamespaceHeader.H"
#endif

#include "BaseNamespaceFooter.H"
#endif
