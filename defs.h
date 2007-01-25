/* **************************************************************************
   defs.h    bridge definitions for C/C++ programs
             PM Cronje June 2006

   Copyright 2006 P.M.Cronje

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

#ifndef DEFS_H
#define DEFS_H

extern const char *szPLAYER[4];
extern const char  chPLAYER[4];
extern const char *szSUIT[4];
extern const char  chSUIT[4];
extern const char *szCONTRACT[5];
extern const char chCARD[13];

typedef unsigned short int ushort;
typedef unsigned long long int uint64;
typedef unsigned char uchar;

extern const ushort usMASK[16];
extern const ushort usNotMASK[16];
extern const unsigned int ulMASK[32];
extern const unsigned int ulNotMASK[32];

// -----------------------------------------------------------------------------

 int fromHex(char ch);
char toHex(int i);

 int bitCount(ushort m);
 int leastSignificant1Bit(ushort m);
 int mostSignificant1Bit(ushort m);
void clearBit(ushort &m, int ibit);
bool isBit(ushort m, int ibit);
void setBit(ushort &m, int ibit);

char *format64(uint64 number, char sz[]);
char *format(unsigned int number, char sz[]);
char *mPerSec(unsigned int count, double elapsed, char sz[]);

// -----------------------------------------------------------------------------

enum eContract
{

  eCONTRACT_SPADE   =  0,
  eCONTRACT_HEART   =  1,
  eCONTRACT_DIAMOND =  2,
  eCONTRACT_CLUB    =  3,

  eCONTRACT_NOTRUMP =  4,

  eCONTRACT_PASS    =  5,

  eCONTRACT_NONE    =  -1

}; // enum eContract
// -----------------------------------------------------------------------------

enum eContractRisk
{
  eCONTRACTRISK_UNDOUBLED = 0,
  eCONTRACTRISK_DOUBLED   = 1,
  eCONTRACTRISK_REDOUBLED = 2

}; // enum eContractRisk
// -----------------------------------------------------------------------------

enum ePlayer
{
  ePLAYER_WEST  = 0,
  ePLAYER_NORTH = 1,
  ePLAYER_EAST  = 2,
  ePLAYER_SOUTH = 3,

  ePLAYER_NONE  = -1

}; // enum ePlayer
// -----------------------------------------------------------------------------

enum eSuit
{
  eSUIT_SPADE   = 0,
  eSUIT_HEART   = 1,
  eSUIT_DIAMOND = 2,
  eSUIT_CLUB    = 3,

  eSUIT_NONE    = -1

}; // enum eSuit
// -----------------------------------------------------------------------------

enum eCard
{
  eCARD_A        =  0,
  eCARD_K        =  1,
  eCARD_Q        =  2,
  eCARD_J        =  3,
  eCARD_10       =  4,
  eCARD_9        =  5,
  eCARD_8        =  6,
  eCARD_7        =  7,
  eCARD_6        =  8,
  eCARD_5        =  9,
  eCARD_4        = 10,
  eCARD_3        = 11,
  eCARD_2        = 12,

  eCARD_NONE     = -1

}; // enum eCard
// -----------------------------------------------------------------------------
#endif
