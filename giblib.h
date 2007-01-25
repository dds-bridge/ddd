/* **************************************************************************
   giblib.h    giblib reader and state for bridge C/C++ programs

               PM Cronje June 2006
               PM Cronje 13-Jul-2006  add deal genearator

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

#ifndef GIBLIB_H
#define GIBLIB_H

#include "defs.h"
#include "rng.h"

#define GIBLIB_LENLINE 128

/* =============================================================================
   Description
   =============================================================================

   To read deals from a text (i.e. not a binary) giblib file.

   It can also be used to store the state of the deal, and to play/unplay
   cards starting with a given deal.

   The maximum line length is defined above by GIBLIB_LENLINE.

   Player and card values
   ----------------------

   See the file defs.h for definitions used.

   Standard giblib file
   --------------------
   The standard text giblib file consists of records like this:

JT852.93.KQ7.J82 AQ97.JT654.T6.A5 43.AK8.A542.7643 K6.Q72.J983.KQT9:88887777A9A977778888

   The first 67 characters is the deal, starting with the west hand,
   followed by north, east and south hands. The deal must consist of 52 cards.

   This is followed by a colon and 20 characters for possible tricks
   (number of tricks which can be made by north-south).
   Each group of four characters is for leader s/e/n/w in that order,
   with five groups for the contracts n/s/h/d/c in that order.

   Extensions
   ----------

   This module allows a number of extensions:

   1. The deal can be any number of tricks between 1 and 13 tricks,
      any multiple of four cards 4,8,12, ... ,48,52.

   2. The possible tricks are optional. If omitted notrumps is assumed.

   3. The possible tricks may be partial, e.g. 20 possible tricks are given
      but some of them are unknown, given by the character '-'.

   4. Any number of blank lines are allowed between deals.

   5. Comment lines may precede a deal. A comment starts with an open brace '{'
      and is terminated by a closing brace '}', spanning any number of lines.
      Any text can be include inside the comments, excluding the opening and
      closing brace characters.

   Options
   -------

   Options may be imbedded in the comment.

   Name Option
   -----------

   This has the form name=... followed by at least a blank or closing brace.
   This is used as the name for the deal, e.g.

     { name=gliblib1}

   Trumps Option
   -------------

   This has the form trumps= followed by one of the characters s/h/d/c/n, e.g.

     { name=gliblib1 trumps=d}

   The default contract is notrump.

   Leader Option
   -------------

   This has the form leader= followed by one of the characters w/n/e/s.
   This is the player leading the first card. In a 52-card deal it is
   understood that his RHO is the declarer, e.g.

     { name=gliblib1 trumps=n leader=e}

   The default leader is west.

   Played Option
   -------------

   Specifies that one or more cards are to be played starting with the deal
   given. The leader is determined by the first card. All cards must be present
   in the deal, be in the hand of the right player, must follow suit, and so on.
   Each trick must be started by the winner of the previous trick.

   A card is given as two characters, the suit and the rank. Use a dash '-'
   to separate tricks, and a period '.' to separate cards in a trick.

   The following is for the first three tricks and a further three cards played
   from the 52-card deal given:

     {name=cpt leader=w played=c8.c9.ct.ck-h7.h2.hk.hj-d3.d8.dk.d5-h9.h3.ha}
     j95.t8632.j65.85 t862.ak54.73.q92 k743.j.a98.ajt76 aq.q97.kqt42.k43

*/
// -----------------------------------------------------------------------------

class cGIBLib;

// -----------------------------------------------------------------------------

class cGIBLib
{
  public:

                   // construct/destruct
                   cGIBLib();
                   ~cGIBLib();

                   // print
             void print();
             void printHands();
             void printInfo();

                  // read deal (ideal0=0,1,2, ...) from GIBLib file
             bool readFile(int ideal0, char *pszname, FILE *fileptr);
                  // read next deal in the file
             bool readDeal(FILE *fileptr);

                  // play a card (it be present in current player's hand)
                  // all variables affected are updated
             bool playCard(int suit, int card);

                  // unplay previous card (there must be one or more played)
             bool unplayCard();

                  // number of cards in hand of player 0,1,2,3 (i.e. w,n,e,s)
              int nPlayerCard(int pl);

                  // number of cards in all hands
              int numCard();

                  // decode szTricks[i], 0<=i<20
                  // if successful returns true with result in
                  //   *pleader/*ptrumps/*pntrick
                  // if unsuccessful returns false with default result
                  //   *pleader = ePLAYER_WEST
                  //   *ptrumps = eCONTRACT_NOTRUMP
                  //   *pntrick = 0
             bool getTricks(int i, int *pleader, int *ptrumps, int *pntrick);

                   // ----------------------------------------------------------
                   // deal generation
                   // ----------------------------------------------------------

                   // generate a random deal
                   //   this sets mPlayerSuit[4][4] only,
                   //   nothing else,
                   //   to set the giblib object,
                   //   call setGeneratedDeal(...) after this
                   // the idea is that you may want to do filtering on the deals
                   // checking whether a deal is acceptable first, without
                   // having the overhead of also setting the deal into the
                   // giblib object
              bool generateDeal(int ntrick);

                   // set generated deal,
                   // call this only after generateDeal(...),
                   // this will set szDeal and clear szTricks with '-',
                   // and initialize some other variables
              bool setGeneratedDeal();

                   // ----------------------------------------------------------
                   // READ-ONLY public data (after successful read)
                   // ----------------------------------------------------------

                   // date read: deal, giblib tricks, comment, name
              char szDeal[68];
              char szTricks[24];
              char *pszComment, *pszName;

                   // --------------------
                   // play/unplay
                   // --------------------

                   // cards played
               int nPlayed;
                   // player, suit, card for i=0,...,nPlayed-1
               int PlayerPlayed[52], SuitPlayed[52], CardPlayed[52];
                   // trick number 0,1,...
                   // trick card 0,1,2,3
                   // tricks won sofar by south/north and west/east
               int nTrick, nTrickCard, nTrickSN, nTrickWE;
                   // TrickStart is index into the three lists ...Played[52]
                   // for the first card of this trick
               int TrickStart;
                   // suit led at this trick
                   // player to this card of this trick
                   // winning player this trick (at completion of trick)
               int SuitLed, Player, WinningPlayer[13];

                   // failed calls write their error message here,
                   // print this to see the reason
              char szErrMsg[512];

                   // ----------------------------------------------------------
                   // READ-ONLY processed data (after successful read)
                   // ----------------------------------------------------------

               int Trumps;       // s=0,h=1,d=2,c=3,n=4

                   // leader of current trick
               int Leader;       // w=0,n=1,e=2,s=3

                   // bit masks of cards per player/suit:
            ushort mPlayerSuit[4][4];        // [player][suit]

                   // ----------------------------------------------------------
                   // INTERNAL interface
                   // ----------------------------------------------------------

              char *pch, szLine[GIBLIB_LENLINE];
               int nLine;
              bool bFileEnd;
              FILE *fp;

 static enum eCard getCard(char chcard);
              bool getOptions();
       static char *printSuit(ushort m, char sz[]);
              bool readComment();
              bool readLine();
              void reset();
              bool setDeal();
              bool setPlayed();
              void setName(char *pszname);
              bool skipComment();

                   // deal generator
                   // uses RNG (random number generator)
              cRNG *pRNG;

                   // set the RNG seed
              void setRNGSeed(unsigned int useed)
                   { if(pRNG) pRNG->set(useed);}

}; // cGIBLib
// -----------------------------------------------------------------------------
#endif

