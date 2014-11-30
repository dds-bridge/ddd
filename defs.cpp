/* **************************************************************************
   defs.cpp bridge definitions for C/C++ programs
             PM Cronje June 2006

   Copyright 2006 P.M.Cronje

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

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "defs.h"
#include "portab_DDD.h"

const char * szPLAYER[4] = {"west", "north", "east", "south"};
const char chPLAYER[4] = {'W', 'N', 'E', 'S'};
const char * szSUIT[4] = {"spades", "hearts", "diamonds", "clubs"};
const char chSUIT[4] = {'S', 'H', 'D', 'C'};
const char * szCONTRACT[5] = {"spades", "hearts", "diamonds", "clubs", "notrump"};
const char chCARD[13] = {'A', 'K', 'Q', 'J', 'T', '9', '8', '7', '6', '5', '4', '3', '2'};

const ushort usMASK[16]
  = { 0x0001, 0x0002, 0x0004, 0x0008,
      0x0010, 0x0020, 0x0040, 0x0080,
      0x0100, 0x0200, 0x0400, 0x0800,
      0x1000, 0x2000, 0x4000, 0x8000
    };
const ushort usNotMASK[16]
  = { 0xfffe, 0xfffd, 0xfffb, 0xfff7,
      0xffef, 0xffdf, 0xffbf, 0xff7f,
      0xfeff, 0xfdff, 0xfbff, 0xf7ff,
      0xefff, 0xdfff, 0xbfff, 0x7fff
    };

const unsigned int ulMASK[32]
  = { 0x00000001, 0x00000002, 0x00000004, 0x00000008,
      0x00000010, 0x00000020, 0x00000040, 0x00000080,
      0x00000100, 0x00000200, 0x00000400, 0x00000800,
      0x00001000, 0x00002000, 0x00004000, 0x00008000,
      0x00010000, 0x00020000, 0x00040000, 0x00080000,
      0x00100000, 0x00200000, 0x00400000, 0x00800000,
      0x01000000, 0x02000000, 0x04000000, 0x08000000,
      0x10000000, 0x20000000, 0x40000000, 0x80000000
    };
const unsigned int ulNotMASK[32]
  = { 0xfffffffe, 0xfffffffd, 0xfffffffb, 0xfffffff7,
      0xffffffef, 0xffffffdf, 0xffffffbf, 0xffffff7f,
      0xfffffeff, 0xfffffdff, 0xfffffbff, 0xfffff7ff,
      0xffffefff, 0xffffdfff, 0xffffbfff, 0xffff7fff,
      0xfffeffff, 0xfffdffff, 0xfffbffff, 0xfff7ffff,
      0xffefffff, 0xffdfffff, 0xffbfffff, 0xff7fffff,
      0xfeffffff, 0xfdffffff, 0xfbffffff, 0xf7ffffff,
      0xefffffff, 0xdfffffff, 0xbfffffff, 0x7fffffff
    };

// *****************************************************************************

int fromHex(char ch)
{
  if (isdigit(ch))
    return (static_cast<int>(tolower(ch)) - static_cast<int>('0'));
  else
    return (static_cast<int>(tolower(ch)) - static_cast<int>('a' + 10));

} // fromHex
// *****************************************************************************

char toHex(int i)
{
  if (i < 10)
    return static_cast<char>(i + '0');
  else
    return static_cast<char>
           (toupper(static_cast<char>(i - 10 + static_cast<int>('a'))));

} // toHex
// *****************************************************************************

// initialization of static arrays
// - BitsInByte[256]
// count of 1-bits in byte
// - MSBitInByte[256]
// most significant bit in byte

static bool initBitsInByte(int bits[256], int msb[256])
{
  int i, n, m;

  bits[0] = msb[0] = 0;

  // set bits in byte
  for (i = 1; i < 256; i++)
  {
    n = 0;
    m = i;
    while (m)
    {
      n++;
      m = (m & (m - 1)); // strip right-most bit
    };
    bits[i] = n;
  }

  // most significant bit in byte
  for (i = 1; i < 256; i++)
  {
    n = 0;
    m = i;
    while (m)
    {
      n++;
      m >>= 1;
    };
    msb[i] = n - 1;
  }

  return true;

} // initBitsInChar
// *****************************************************************************

static int BitsInByte[256];
static int MSBitInByte[256];
static bool bInitBitsInByte = initBitsInByte(BitsInByte, MSBitInByte);

// *****************************************************************************

int bitCount(ushort m)
{
  return (BitsInByte[m & 0x00ff] + BitsInByte[(m >> 8) & 0x00ff]);

} // bitCount
// *****************************************************************************

int leastSignificant1Bit(ushort m)
{
  m = static_cast<ushort>(static_cast<ushort>(m) ^ (static_cast<ushort>(m) - 1));

  return bitCount(m) - 1;

} // leastSignificant1Bit
// *****************************************************************************

int mostSignificant1Bit(ushort m)
{
  /*
    if(m & 0xff00)
      return (8 + MSBitInByte[(m >> 8) & 0x00ff]);
    else
      return MSBitInByte[m];
  */
  register unsigned int x = static_cast<unsigned>(m | (m >> 1));
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);

  //x = (x & (~(x >> 1)));
  return bitCount(static_cast<ushort>(x)) - 1;

} // mostSignificant1Bit
// *****************************************************************************
void clearBit(ushort & m, int ibit)
{
  m &= usNotMASK[ibit];
}
bool isBit(ushort m, int ibit)
{
  return ((m & usMASK[ibit]) ? true : false);
}
void setBit(ushort & m, int ibit)
{
  m |= usMASK[ibit];
}
/* ************************************************************************** */

char * format(char str[], char sz[]);

char * format64(unsigned long long number, char sz[])
{
  char str[32];

  snprintf(str, 31, "%llu", number);
  format(str, sz);

  return sz;

} // format64
/* ************************************************************************** */

char * format(unsigned int number, char sz[])
{
  char str[32];

  snprintf(str, 31, "%u", number);
  format(str, sz);

  return sz;

} // format
/* ************************************************************************** */

char * format(char str[], char sz[])
{
  int len, n, istart, ndigit, i, pos;

  len = static_cast<int>(strlen(str));
  n = len / 3;
  if (3 * n != len)
    n++;
  istart = pos = 0;
  for (i = 0; i < n; i++)
  {
    if (i == 0)
      ndigit = len - 3 * (n - 1);
    else
      ndigit = 3;
    strncpy(sz + pos, str + istart, static_cast<size_t>(ndigit));
    istart += ndigit;
    pos += ndigit;
    if (i < n - 1)
    {
      sz[pos] = ',';
      pos++;
    }
  }
  sz[pos] = '\0';

  return sz;

} // format
// *****************************************************************************

char * mPerSec(unsigned int count, double elapsed, char sz[])
{
  if (elapsed >= 0.01)
    sprintf(sz, "%0.3fm/s", 1.0e-6 * static_cast<double>(count) / elapsed);
  else
    strcpy(sz, "-.---m/s");

  return sz;

} // mPerSec
/* ************************************************************************** */

