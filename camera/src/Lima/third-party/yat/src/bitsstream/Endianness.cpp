//----------------------------------------------------------------------------
// YAT LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (C) 2006-2012  N.Leclercq & The Tango Community
//
// Part of the code comes from the ACE Framework (asm bytes swaping code)
// see http://www.cs.wustl.edu/~schmidt/ACE.html for more about ACE
//
// The thread native implementation has been initially inspired by omniThread
// - the threading support library that comes with omniORB. 
// see http://omniorb.sourceforge.net/ for more about omniORB.
//
// Contributors form the TANGO community:
// See AUTHORS file 
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
 * \author See AUTHORS file
 */

//=============================================================================
//- Endianness.cpp
//=============================================================================
// abstraction.......Optimized bytes swapping 
// class.............Endianness
// original authors..Aniruddha Gokhale, Carlos O'Ryan, ... (ACE lib)
// hacker............Nicolas Leclercq - SOLEIL
//=============================================================================

//=============================================================================
// DEPENDENCIES
//=============================================================================
#include <yat/bitsstream/Endianness.h>

namespace yat 
{

//=============================================================================
// HOST ENDIANNESS
//=============================================================================
#if YAT_LITTLE_ENDIAN_PLATFORM == 1
  const Endianness::ByteOrder 
    Endianness::host_endianness = Endianness::BO_LITTLE_ENDIAN; 
  const Endianness::ByteOrder 
    Endianness::not_host_endianness = Endianness::BO_BIG_ENDIAN; 
#else
  const Endianness::ByteOrder 
    Endianness::host_endianness = Endianness::BO_BIG_ENDIAN; 
  const Endianness::ByteOrder 
    Endianness::not_host_endianness = Endianness::BO_LITTLE_ENDIAN; 
#endif

//=============================================================================
// ALIGN_BINARY
//=============================================================================
inline uintptr_t ALIGN_BINARY (uintptr_t ptr, uintptr_t alignment)
{
  uintptr_t const tmp = alignment - 1;
  return (ptr + tmp) & (~tmp);
}

//=============================================================================
// PTR_ALIGN_BINARY
//=============================================================================
inline char * PTR_ALIGN_BINARY (char const * ptr, uintptr_t alignment)
{
  return reinterpret_cast<char *>(ALIGN_BINARY(reinterpret_cast<uintptr_t>(ptr), alignment));
}

//=============================================================================
// PTR_ALIGN_BINARY
//=============================================================================
inline char * PTR_ALIGN_BINARY (unsigned char const * ptr, uintptr_t alignment)
{
  return PTR_ALIGN_BINARY(reinterpret_cast<char const *>(ptr), alignment);
}

#if defined (YAT_HAS_INTRINSIC_BYTESWAP)
  //- take advantage of MSVC++ byte swapping compiler intrinsics (found in <stdlib.h>)
# pragma intrinsic (_byteswap_ushort, _byteswap_ulong, _byteswap_uint64)
#elif defined (YAT_HAS_BSWAP_16) || defined (YAT_HAS_BSWAP_32) || defined (YAT_HAS_BSWAP_64)
  //- take advantage of local swapping implementation
# include <byteswap.h>
#endif
  
  //- the number of bytes in a long
#include <limits.h>
#if ! defined (YAT_SIZEOF_LONG)
# if ((ULONG_MAX) == 65535UL)
#   define YAT_SIZEOF_LONG 2
# elif ((ULONG_MAX) == 4294967295UL)
#   define YAT_SIZEOF_LONG 4
# elif ((ULONG_MAX) == 18446744073709551615UL)
#   define YAT_SIZEOF_LONG 8
# else
#   error: unsupported long size, must be updated for this platform!
# endif
#endif
  
//=============================================================================
// Endianness::swap_2
//=============================================================================
void Endianness::swap_2 (const char *orig, char* target)
{
#if defined (YAT_HAS_INTRINSIC_BYTESWAP)
  //- take advantage of MSVC++ compiler intrinsic byte swapping function
  *reinterpret_cast<unsigned short *> (target) =
  _byteswap_ushort (*reinterpret_cast<unsigned short const *> (orig));
#elif defined (YAT_HAS_BSWAP16)
  *reinterpret_cast<uint16_t *> (target) =
  bswap16 (*reinterpret_cast<uint16_t const *> (orig));
#elif defined(YAT_HAS_INTEL_ASSEMBLY)
  unsigned short a =
  *reinterpret_cast<const unsigned short*> (orig);
  asm( "rolw $8, %0" : "=r" (a) : "0" (a) );
  *reinterpret_cast<unsigned short*> (target) = a;
#elif ! defined(YAT_NO_INLINE_ASM) && defined (YAT_HAS_PENTIUM) && defined(_MSC_VER)
  __asm mov ebx, orig;
  __asm mov ecx, target;
  __asm mov ax, [ebx];
  __asm rol ax, 8;
  __asm mov [ecx], ax;
#else
  register yat::uint16 usrc = * reinterpret_cast<const yat::uint16*> (orig);
  register yat::uint16* udst = reinterpret_cast<yat::uint16*> (target);
  *udst = (usrc << 8) | (usrc >> 8);
#endif
}
  
//=============================================================================
// Endianness::swap_4
//=============================================================================
void Endianness::swap_4 (const char* orig, char* target)
{
#if defined (YAT_HAS_INTRINSIC_BYTESWAP)
  //- take advantage of MSVC++ compiler intrinsic byte swapping function
  *reinterpret_cast<unsigned long *> (target) =
  _byteswap_ulong (*reinterpret_cast<unsigned long const *> (orig));
#elif defined (YAT_HAS_BSWAP32)
  *reinterpret_cast<uint32_t *> (target) =
  bswap32 (*reinterpret_cast<uint32_t const *> (orig));
#elif defined(YAT_HAS_INTEL_ASSEMBLY)
  //- we have YAT_HAS_PENTIUM, so we know the sizeof's.
  register unsigned int j =
  *reinterpret_cast<const unsigned int*> (orig);
  asm ("bswap %1" : "=r" (j) : "0" (j));
  *reinterpret_cast<unsigned int*> (target) = j;
#elif ! defined(YAT_NO_INLINE_ASM) && defined(YAT_HAS_PENTIUM) && defined(_MSC_VER)
  __asm mov ebx, orig;
  __asm mov ecx, target;
  __asm mov eax, [ebx];
  __asm bswap eax;
  __asm mov [ecx], eax;
#else
  register yat::uint32 x = * reinterpret_cast<const yat::uint32*> (orig);
  x = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
  * reinterpret_cast<yat::uint32*> (target) = x;
#endif
}

//=============================================================================
// Endianness::swap_8
//=============================================================================
void Endianness::swap_8 (const char* orig, char* target)
{
#if defined (YAT_HAS_INTRINSIC_BYTESWAP)
  //- take advantage of MSVC++ compiler intrinsic byte swapping function
  *reinterpret_cast<unsigned __int64 *> (target) =
  _byteswap_uint64 (*reinterpret_cast<unsigned __int64 const *> (orig));
#elif defined (YAT_HAS_BSWAP64)
  *reinterpret_cast<uint64_t *> (target) =
  bswap64 (*reinterpret_cast<uint64_t const *> (orig));
#elif (defined (__amd64__) || defined (__x86_64__)) && defined(__GNUG__)
  register unsigned long x =
  * reinterpret_cast<const unsigned long*> (orig);
  asm ("bswapq %1" : "=r" (x) : "0" (x));
  *reinterpret_cast<unsigned long*> (target) = x;
#elif defined(YAT_HAS_PENTIUM) && defined(__GNUG__)
  register unsigned int i =
  *reinterpret_cast<const unsigned int*> (orig);
  register unsigned int j =
  *reinterpret_cast<const unsigned int*> (orig + 4);
  asm ("bswap %1" : "=r" (i) : "0" (i));
  asm ("bswap %1" : "=r" (j) : "0" (j));
  *reinterpret_cast<unsigned int*> (target + 4) = i;
  *reinterpret_cast<unsigned int*> (target) = j;
#elif ! defined(YAT_NO_INLINE_ASM) && defined(YAT_HAS_PENTIUM) && defined(_MSC_VER)
  __asm mov ecx, orig;
  __asm mov edx, target;
  __asm mov eax, [ecx];
  __asm mov ebx, 4[ecx];
  __asm bswap eax;
  __asm bswap ebx;
  __asm mov 4[edx], eax;
  __asm mov [edx], ebx;
#elif YAT_SIZEOF_LONG == 8
  //- 64 bit architecture.
  register unsigned long x =
  * reinterpret_cast<const unsigned long*> (orig);
  register unsigned long x84 = (x & 0x000000ff000000ffUL) << 24;
  register unsigned long x73 = (x & 0x0000ff000000ff00UL) << 8;
  register unsigned long x62 = (x & 0x00ff000000ff0000UL) >> 8;
  register unsigned long x51 = (x & 0xff000000ff000000UL) >> 24;
  x = (x84 | x73 | x62 | x51);
  x = (x << 32) | (x >> 32);
  *reinterpret_cast<unsigned long*> (target) = x;
#else
  register yat::uint32 x =
  * reinterpret_cast<const yat::uint32*> (orig);
  register yat::uint32 y =
  * reinterpret_cast<const yat::uint32*> (orig + 4);
  x = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
  y = (y << 24) | ((y & 0xff00) << 8) | ((y & 0xff0000) >> 8) | (y >> 24);
  * reinterpret_cast<yat::uint32*> (target) = y;
  * reinterpret_cast<yat::uint32*> (target + 4) = x;
#endif
}

//=============================================================================
// Endianness::swap_16
//=============================================================================
void Endianness::swap_16 (const char* orig, char* target)
{
  Endianness::swap_8(orig + 8, target);
  Endianness::swap_8(orig, target + 8);
}
  
//=============================================================================
// Endianness::swap_2_array
//=============================================================================
void Endianness::swap_2_array (const char* orig, char* target, size_t n)
{
  if (n == 0)
    return;

  // We pretend that AMD64/GNU G++ systems have a Pentium CPU to
  // take advantage of the inline assembly implementation.

  // Later, we try to read in 32 or 64 bit chunks,
  // so make sure we don't do that for unaligned addresses.
#if YAT_SIZEOF_LONG == 8 && !((defined(__amd64__) || defined (__x86_64__)) && defined(__GNUG__))
  char const * const o8 = PTR_ALIGN_BINARY (orig, 8);
  while (orig < o8 && n > 0)
  {
    Endianness::swap_2(orig, target);
    orig += 2;
    target += 2;
    --n;
  }
#else
  char const * const o4 = PTR_ALIGN_BINARY(orig, 4);
  // this is an _if_, not a _while_. The mistmatch can only be by 2.
  if (orig != o4)
  {
    Endianness::swap_2(orig, target);
    orig += 2;
    target += 2;
    --n;
  }
#endif
  if (n == 0)
    return;

  //
  // Loop unrolling. Here be dragons.
  //

  // (n & (~3)) is the greatest multiple of 4 not bigger than n.
  // In the while loop ahead, orig will move over the array by 8 byte
  // increments (4 elements of 2 bytes).
  // end marks our barrier for not falling outside.
  char const * const end = orig + 2 * (n & (~3));

  // See if we're aligned for writting in 64 or 32 bit chunks...
#if YAT_SIZEOF_LONG == 8 && !((defined(__amd64__) || defined (__x86_64__)) && defined(__GNUG__))
  if (target == PTR_ALIGN_BINARY (target, 8))
#else
  if (target == PTR_ALIGN_BINARY(target, 4))
#endif
  {
    while (orig < end)
    {
#if defined (YAT_HAS_INTEL_ASSEMBLY)
      unsigned int a =
        * reinterpret_cast<const unsigned int*> (orig);
      unsigned int b =
        * reinterpret_cast<const unsigned int*> (orig + 4);
      asm ( "bswap %1"      : "=r" (a) : "0" (a) );
      asm ( "bswap %1"      : "=r" (b) : "0" (b) );
      asm ( "rol $16, %1"   : "=r" (a) : "0" (a) );
      asm ( "rol $16, %1"   : "=r" (b) : "0" (b) );
      * reinterpret_cast<unsigned int*> (target) = a;
      * reinterpret_cast<unsigned int*> (target + 4) = b;
#elif ! defined(YAT_NO_INLINE_ASM) && defined(YAT_HAS_PENTIUM) && defined(_MSC_VER) 
      __asm mov ecx, orig;
      __asm mov edx, target;
      __asm mov eax, [ecx];
      __asm mov ebx, 4[ecx];
      __asm bswap eax;
      __asm bswap ebx;
      __asm rol eax, 16;
      __asm rol ebx, 16;
      __asm mov [edx], eax;
      __asm mov 4[edx], ebx;
#elif YAT_SIZEOF_LONG == 8
      // 64 bit architecture.
      register unsigned long a =
        * reinterpret_cast<const unsigned long*> (orig);

      register unsigned long a1 = (a & 0x00ff00ff00ff00ffUL) << 8;
      register unsigned long a2 = (a & 0xff00ff00ff00ff00UL) >> 8;

      a = (a1 | a2);

      * reinterpret_cast<unsigned long*> (target) = a;
#else
      register yat::uint32 a =
        * reinterpret_cast<const yat::uint32*> (orig);
      register yat::uint32 b =
        * reinterpret_cast<const yat::uint32*> (orig + 4);

      register yat::uint32 a1 = (a & 0x00ff00ffU) << 8;
      register yat::uint32 b1 = (b & 0x00ff00ffU) << 8;
      register yat::uint32 a2 = (a & 0xff00ff00U) >> 8;
      register yat::uint32 b2 = (b & 0xff00ff00U) >> 8;

      a = (a1 | a2);
      b = (b1 | b2);

      * reinterpret_cast<yat::uint32*> (target) = a;
      * reinterpret_cast<yat::uint32*> (target + 4) = b;
#endif
      orig += 8;
      target += 8;
    }
  }
else
  {
    // We're out of luck. We have to write in 2 byte chunks.
    while (orig < end)
    {
#if defined (YAT_HAS_INTEL_ASSEMBLY)
      unsigned int a =
        * reinterpret_cast<const unsigned int*> (orig);
      unsigned int b =
        * reinterpret_cast<const unsigned int*> (orig + 4);
      asm ( "bswap %1" : "=r" (a) : "0" (a) );
      asm ( "bswap %1" : "=r" (b) : "0" (b) );
      // We're little endian.
      * reinterpret_cast<unsigned short*> (target + 2)
          = (unsigned short) (a & 0xffff);
      * reinterpret_cast<unsigned short*> (target + 6)
          = (unsigned short) (b & 0xffff);
      asm ( "shrl $16, %1" : "=r" (a) : "0" (a) );
      asm ( "shrl $16, %1" : "=r" (b) : "0" (b) );
      * reinterpret_cast<unsigned short*> (target + 0)
          = (unsigned short) (a & 0xffff);
      * reinterpret_cast<unsigned short*> (target + 4)
          = (unsigned short) (b & 0xffff);
#elif ! defined(YAT_NO_INLINE_ASM) && defined (YAT_HAS_PENTIUM) && defined (_MSC_VER)
      __asm mov ecx, orig;
      __asm mov edx, target;
      __asm mov eax, [ecx];
      __asm mov ebx, 4[ecx];
      __asm bswap eax;
      __asm bswap ebx;
      // We're little endian.
      __asm mov 2[edx], ax;
      __asm mov 6[edx], bx;
      __asm shr eax, 16;
      __asm shr ebx, 16;
      __asm mov 0[edx], ax;
      __asm mov 4[edx], bx;
#elif YAT_SIZEOF_LONG == 8
      // 64 bit architecture.
      register unsigned long a =
        * reinterpret_cast<const unsigned long*> (orig);

      register unsigned long a1 = (a & 0x00ff00ff00ff00ffUL) << 8;
      register unsigned long a2 = (a & 0xff00ff00ff00ff00UL) >> 8;

      a = (a1 | a2);

      yat::uint16 b1 = static_cast<yat::uint16> (a >> 48);
      yat::uint16 b2 = static_cast<yat::uint16> ((a >> 32) & 0xffff);
      yat::uint16 b3 = static_cast<yat::uint16> ((a >> 16) & 0xffff);
      yat::uint16 b4 = static_cast<yat::uint16> (a & 0xffff);

#if defined(YAT_LITTLE_ENDIAN)
      * reinterpret_cast<yat::uint16*> (target) = b4;
      * reinterpret_cast<yat::uint16*> (target + 2) = b3;
      * reinterpret_cast<yat::uint16*> (target + 4) = b2;
      * reinterpret_cast<yat::uint16*> (target + 6) = b1;
#else
      * reinterpret_cast<yat::uint16*> (target) = b1;
      * reinterpret_cast<yat::uint16*> (target + 2) = b2;
      * reinterpret_cast<yat::uint16*> (target + 4) = b3;
      * reinterpret_cast<yat::uint16*> (target + 6) = b4;
#endif
#else
      register yat::uint32 a =
        * reinterpret_cast<const yat::uint32*> (orig);
      register yat::uint32 b =
        * reinterpret_cast<const yat::uint32*> (orig + 4);

      register yat::uint32 a1 = (a & 0x00ff00ff) << 8;
      register yat::uint32 b1 = (b & 0x00ff00ff) << 8;
      register yat::uint32 a2 = (a & 0xff00ff00) >> 8;
      register yat::uint32 b2 = (b & 0xff00ff00) >> 8;

      a = (a1 | a2);
      b = (b1 | b2);

      yat::uint32 c1 = static_cast<yat::uint16> (a >> 16);
      yat::uint32 c2 = static_cast<yat::uint16> (a & 0xffff);
      yat::uint32 c3 = static_cast<yat::uint16> (b >> 16);
      yat::uint32 c4 = static_cast<yat::uint16> (b & 0xffff);

#if defined(YAT_LITTLE_ENDIAN)
      * reinterpret_cast<yat::uint16*> (target) = c2;
      * reinterpret_cast<yat::uint16*> (target + 2) = c1;
      * reinterpret_cast<yat::uint16*> (target + 4) = c4;
      * reinterpret_cast<yat::uint16*> (target + 6) = c3;
#else
      * reinterpret_cast<yat::uint16*> (target) = c1;
      * reinterpret_cast<yat::uint16*> (target + 2) = c2;
      * reinterpret_cast<yat::uint16*> (target + 4) = c3;
      * reinterpret_cast<yat::uint16*> (target + 6) = c4;
#endif
#endif
      orig += 8;
      target += 8;
      }
  }

  // (n & 3) == (n % 4).
  switch (n&3) 
  {
    case 3:
      Endianness::swap_2(orig, target);
      orig += 2;
      target += 2;
    case 2:
      Endianness::swap_2(orig, target);
      orig += 2;
      target += 2;
    case 1:
      Endianness::swap_2(orig, target);
  }
}

//=============================================================================
// Endianness::swap_4_array
//=============================================================================
void Endianness::swap_4_array (const char* orig, char* target, size_t n)
{
  if (n == 0)
    return;

#if YAT_SIZEOF_LONG == 8
  // Later, we read from *orig in 64 bit chunks,
  // so make sure we don't generate unaligned readings.
  char const * const o8 = PTR_ALIGN_BINARY (orig, 8);
  // The mismatch can only be by 4.
  if (orig != o8)
    {
      Endianness::swap_4(orig, target);
      orig += 4;
      target += 4;
      --n;
    }
#endif  /* YAT_SIZEOF_LONG == 8 */

  if (n == 0)
    return;

  //
  // Loop unrolling. Here be dragons.
  //

  // (n & (~3)) is the greatest multiple of 4 not bigger than n.
  // In the while loop, orig will move over the array by 16 byte
  // increments (4 elements of 4 bytes).
  // ends marks our barrier for not falling outside.
  char const * const end = orig + 4 * (n & (~3));

#if YAT_SIZEOF_LONG == 8
  // 64 bits architecture.
  // See if we can write in 8 byte chunks.
  if (target == PTR_ALIGN_BINARY (target, 8))
  {
    while (orig < end)
    {
      register unsigned long a =
        * reinterpret_cast<const long*> (orig);
      register unsigned long b =
        * reinterpret_cast<const long*> (orig + 8);

#if defined(YAT_HAS_INTEL_ASSEMBLY)
      asm ("bswapq %1" : "=r" (a) : "0" (a));
      asm ("bswapq %1" : "=r" (b) : "0" (b));
      asm ("rol $32, %1" : "=r" (a) : "0" (a));
      asm ("rol $32, %1" : "=r" (b) : "0" (b));
#else
      register unsigned long a84 = (a & 0x000000ff000000ffL) << 24;
      register unsigned long b84 = (b & 0x000000ff000000ffL) << 24;
      register unsigned long a73 = (a & 0x0000ff000000ff00L) << 8;
      register unsigned long b73 = (b & 0x0000ff000000ff00L) << 8;
      register unsigned long a62 = (a & 0x00ff000000ff0000L) >> 8;
      register unsigned long b62 = (b & 0x00ff000000ff0000L) >> 8;
      register unsigned long a51 = (a & 0xff000000ff000000L) >> 24;
      register unsigned long b51 = (b & 0xff000000ff000000L) >> 24;

      a = (a84 | a73 | a62 | a51);
      b = (b84 | b73 | b62 | b51);
#endif

      * reinterpret_cast<long*> (target) = a;
      * reinterpret_cast<long*> (target + 8) = b;

      orig += 16;
      target += 16;
    }
  }
  else
  {
    // We are out of luck, we have to write in 4 byte chunks.
    while (orig < end)
    {
      register unsigned long a =
        * reinterpret_cast<const long*> (orig);
      register unsigned long b =
        * reinterpret_cast<const long*> (orig + 8);

#if defined(YAT_HAS_INTEL_ASSEMBLY)
      asm ("bswapq %1" : "=r" (a) : "0" (a));
      asm ("bswapq %1" : "=r" (b) : "0" (b));
      asm ("rol $32, %1" : "=r" (a) : "0" (a));
      asm ("rol $32, %1" : "=r" (b) : "0" (b));
#else
      register unsigned long a84 = (a & 0x000000ff000000ffL) << 24;
      register unsigned long b84 = (b & 0x000000ff000000ffL) << 24;
      register unsigned long a73 = (a & 0x0000ff000000ff00L) << 8;
      register unsigned long b73 = (b & 0x0000ff000000ff00L) << 8;
      register unsigned long a62 = (a & 0x00ff000000ff0000L) >> 8;
      register unsigned long b62 = (b & 0x00ff000000ff0000L) >> 8;
      register unsigned long a51 = (a & 0xff000000ff000000L) >> 24;
      register unsigned long b51 = (b & 0xff000000ff000000L) >> 24;

      a = (a84 | a73 | a62 | a51);
      b = (b84 | b73 | b62 | b51);
#endif

      yat::uint32 c1 = static_cast<yat::uint32> (a >> 32);
      yat::uint32 c2 = static_cast<yat::uint32> (a & 0xffffffff);
      yat::uint32 c3 = static_cast<yat::uint32> (b >> 32);
      yat::uint32 c4 = static_cast<yat::uint32> (b & 0xffffffff);

#if defined (YAT_LITTLE_ENDIAN)
      * reinterpret_cast<yat::uint32*> (target + 0) = c2;
      * reinterpret_cast<yat::uint32*> (target + 4) = c1;
      * reinterpret_cast<yat::uint32*> (target + 8) = c4;
      * reinterpret_cast<yat::uint32*> (target + 12) = c3;
#else
      * reinterpret_cast<yat::uint32*> (target + 0) = c1;
      * reinterpret_cast<yat::uint32*> (target + 4) = c2;
      * reinterpret_cast<yat::uint32*> (target + 8) = c3;
      * reinterpret_cast<yat::uint32*> (target + 12) = c4;
#endif
      orig += 16;
      target += 16;
    }
  }
#else  /* YAT_SIZEOF_LONG != 8 */

  while (orig < end)
  {
#if defined (YAT_HAS_PENTIUM) && defined (__GNUG__)
    register unsigned int a =
      *reinterpret_cast<const unsigned int*> (orig);
    register unsigned int b =
      *reinterpret_cast<const unsigned int*> (orig + 4);
    register unsigned int c =
      *reinterpret_cast<const unsigned int*> (orig + 8);
    register unsigned int d =
      *reinterpret_cast<const unsigned int*> (orig + 12);

    asm ("bswap %1" : "=r" (a) : "0" (a));
    asm ("bswap %1" : "=r" (b) : "0" (b));
    asm ("bswap %1" : "=r" (c) : "0" (c));
    asm ("bswap %1" : "=r" (d) : "0" (d));

    *reinterpret_cast<unsigned int*> (target) = a;
    *reinterpret_cast<unsigned int*> (target + 4) = b;
    *reinterpret_cast<unsigned int*> (target + 8) = c;
    *reinterpret_cast<unsigned int*> (target + 12) = d;
#elif ! defined(YAT_NO_INLINE_ASM) && defined (YAT_HAS_PENTIUM) && defined (_MSC_VER)
    __asm mov eax, orig
    __asm mov esi, target
    __asm mov edx, [eax]
    __asm mov ecx, 4[eax]
    __asm mov ebx, 8[eax]
    __asm mov eax, 12[eax]
    __asm bswap edx
    __asm bswap ecx
    __asm bswap ebx
    __asm bswap eax
    __asm mov [esi], edx
    __asm mov 4[esi], ecx
    __asm mov 8[esi], ebx
    __asm mov 12[esi], eax
#else
    register yat::uint32 a =
      * reinterpret_cast<const yat::uint32*> (orig);
    register yat::uint32 b =
      * reinterpret_cast<const yat::uint32*> (orig + 4);
    register yat::uint32 c =
      * reinterpret_cast<const yat::uint32*> (orig + 8);
    register yat::uint32 d =
      * reinterpret_cast<const yat::uint32*> (orig + 12);

    // Expect the optimizer reordering this A LOT.
    // We leave it this way for clarity.
    a = (a << 24) | ((a & 0xff00) << 8) | ((a & 0xff0000) >> 8) | (a >> 24);
    b = (b << 24) | ((b & 0xff00) << 8) | ((b & 0xff0000) >> 8) | (b >> 24);
    c = (c << 24) | ((c & 0xff00) << 8) | ((c & 0xff0000) >> 8) | (c >> 24);
    d = (d << 24) | ((d & 0xff00) << 8) | ((d & 0xff0000) >> 8) | (d >> 24);

    * reinterpret_cast<yat::uint32*> (target) = a;
    * reinterpret_cast<yat::uint32*> (target + 4) = b;
    * reinterpret_cast<yat::uint32*> (target + 8) = c;
    * reinterpret_cast<yat::uint32*> (target + 12) = d;
#endif
      orig += 16;
      target += 16;
    }

#endif /* YAT_SIZEOF_LONG == 8 */

  // (n & 3) == (n % 4).
  switch (n & 3) 
  {
    case 3:
      Endianness::swap_4(orig, target);
      orig += 4;
      target += 4;
    case 2:
      Endianness::swap_4(orig, target);
      orig += 4;
      target += 4;
    case 1:
      Endianness::swap_4(orig, target);
  }
}

//=============================================================================
// Endianness::swap_8_array
//=============================================================================
void Endianness::swap_8_array (const char* orig, char* target, size_t n)
{
  if (n == 0)
    return;

  char const * const end = orig + 8 * n;

  while (orig < end)
  {
    swap_8 (orig, target);
    orig += 8;
    target += 8;
  }
}

//=============================================================================
// Endianness::swap_16_array
//=============================================================================
void Endianness::swap_16_array (const char* orig, char* target, size_t n)
{
  if (n == 0)
    return;

  char const * const end = orig + 16 * n;

  while (orig < end)
  {
    swap_16 (orig, target);
    orig += 16;
    target += 16;
  }
}

} //- namespace 

