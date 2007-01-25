/* **************************************************************************
   giblib.cpp  giblib reader and state for bridge C/C++ programs

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "giblib.h"

// *****************************************************************************

cGIBLib::cGIBLib()
{
  pszComment = static_cast<char *>(realloc(0,GIBLIB_LENLINE*sizeof(char)));
  pszName    = static_cast<char *>(realloc(0,sizeof(char)));
  pszName[0] = '\0';
  nPlayed = 0;
  nLine = 0;

  pRNG = cRNG::createRNG(eRNG_MT,0);

} // cGIBLib::cGIBLib
// *****************************************************************************

cGIBLib::~cGIBLib()
{
  pszComment = static_cast<char *>(realloc(pszComment,0));
  pszName    = static_cast<char *>(realloc(pszName,0));

  if(pRNG)
    delete pRNG;

} // cGIBLib::~cGIBLib
// *****************************************************************************

bool cGIBLib::generateDeal(int ntrick)
{
  int lsuit[52], lcard[52];
  int player, suit, card, icard, ncard, ndeck;

  if(pRNG == 0)
  { sprintf(szErrMsg,"*** error: create RNG\n");
    return false;
  }

  if((ntrick < 1) || (ntrick > 13))
  { sprintf(szErrMsg,"*** error: generateDeal ntrick=%d must be >=1 and <=13\n",
            ntrick);
    return false;
  }

  memset(mPlayerSuit,0,16*sizeof(ushort));

  player = 0;

  // set up a sorted deck
  icard = 0;
  for(suit=0; suit<4; suit++)
  { for(card=0; card<13; card++)
    { lsuit[icard] = suit;
      lcard[icard] = card;
      icard++;
    }
  }

  if(ntrick < 1)
    ntrick = 1;
  if(ntrick > 13)
    ntrick = 13;
  ncard = 4 * ntrick;
  ndeck = 52;

  while(ncard)
  { if(ncard == 1)
      icard = 0;
    else
      icard = static_cast<int>
        (pRNG->randomUint(static_cast<unsigned>(ndeck)));

    // add new card given by lsuit[icard] and lcard[icard]
    setBit(mPlayerSuit[player][lsuit[icard]],lcard[icard]);

    // shorten deck (remove this card)
    if(icard < ndeck-1)
    { memmove(lsuit+icard,lsuit+icard+1,static_cast<size_t>(ndeck-1-icard)*sizeof(int));
      memmove(lcard+icard,lcard+icard+1,static_cast<size_t>(ndeck-1-icard)*sizeof(int));
    }
    ncard--;
    ndeck--;

    player = ((player+1)%4);
  }

  return true;

} // cGIBLib::generateDeal
// *****************************************************************************

enum eCard cGIBLib::getCard(char chcard)
{
  if(tolower(chcard) == 'a')
    return eCARD_A;
  else if(tolower(chcard) == 'k')
    return eCARD_K;
  else if(tolower(chcard) == 'q')
    return eCARD_Q;
  else if(tolower(chcard) == 'j')
    return eCARD_J;
  else if(tolower(chcard) == 't')
    return eCARD_10;
  else if(isdigit(chcard))
  { if((static_cast<int>(chcard) >= static_cast<int>('2')) && 
      (static_cast<int>(chcard) <= static_cast<int>('9')))
      return static_cast<enum eCard>(14 - static_cast<int>(chcard) + static_cast<int>('0'));
  }

  return eCARD_NONE;

} // cGIBLib::getCard
// *****************************************************************************

bool cGIBLib::getOptions()
{
  char *pchend;
   int suit, card;

  pchend = strstr(pszComment,"name=");
  if(pchend)
    setName(pchend+5);
  else
    pszName[0] = '\0';

  pchend = strstr(pszComment,"trumps=");
  if(pchend)
  {      if(tolower(pchend[7]) == 's') Trumps = 0;
    else if(tolower(pchend[7]) == 'h') Trumps = 1;
    else if(tolower(pchend[7]) == 'd') Trumps = 2;
    else if(tolower(pchend[7]) == 'c') Trumps = 3;
    else if(tolower(pchend[7]) == 'n') Trumps = 4;
    else
    { sprintf(szErrMsg,"*** error: invalid trumps='%c' at line %d\n",
              pchend[7],nLine);
      return false;
    }
  }
  pchend = strstr(pszComment,"leader=");
  if(pchend)
  {      if(tolower(pchend[7]) == 'w') Leader = 0;
    else if(tolower(pchend[7]) == 'n') Leader = 1;
    else if(tolower(pchend[7]) == 'e') Leader = 2;
    else if(tolower(pchend[7]) == 's') Leader = 3;
    else
    { sprintf(szErrMsg,"*** error: invalid leader='%c' at line %d\n",
              pchend[7],nLine);
      return false;
    }
  }
  pchend = strstr(pszComment,"played=");
  if(pchend)
  { pchend += 7;
    nPlayed = 0;
    for(;;)
    { while((pchend[0]=='.') || (pchend[0]=='-'))
        pchend++;
           if(tolower(pchend[0]) == 's') suit = 0;
      else if(tolower(pchend[0]) == 'h') suit = 1;
      else if(tolower(pchend[0]) == 'd') suit = 2;
      else if(tolower(pchend[0]) == 'c') suit = 3;
      else if((pchend[0]==' ')||(pchend[0]=='}')||(pchend[0]=='\n'))
        break;
      else
      { sprintf(szErrMsg,"*** error: invalid suit='%c' at line %d\n",
                pchend[0],nLine);
        return false;
      }
      pchend++;
      card = getCard(pchend[0]);
      if(card == eCARD_NONE)
      { sprintf(szErrMsg,"*** error: invalid card='%c' at line %d\n",
                pchend[0],nLine);
        return false;
      }
      if(nPlayed >= 51)
      { sprintf(szErrMsg,"*** error: too many cards at line %d\n",nLine);
        return false;
      }
      SuitPlayed[nPlayed] = suit;
      CardPlayed[nPlayed] = card;
      nPlayed++;
      pchend++;
    }
  }

  return true;

} // cGIBLib::getOptions
// *****************************************************************************

bool cGIBLib::getTricks(int i, int *pleader, int *ptrumps, int *pntrick)
{
  // defaults
  *pleader = ePLAYER_WEST;
  *ptrumps = eCONTRACT_NOTRUMP;
  *pntrick = 0;

  if((i < 0) || (i >= 20))
    return false;

  // leader
  *pleader = (3 - (i % 4));

  // trumps
  *ptrumps = (i / 4);
  if(*ptrumps == 0)
    *ptrumps = eCONTRACT_NOTRUMP;
  else
    (*ptrumps)--;

  // number of tricks;
  *pntrick = -1;
  char *pch_local = szTricks + i;
  if(isdigit(*pch_local))
    *pntrick = (*pch_local - '0');
  else if(isxdigit(*pch_local))
    *pntrick = (toupper(*pch_local) - 'A' + 10);
  if((*pntrick < 0) || (*pntrick > 13))
  { *pntrick = 0;
    return false;
  }
  else
    return true;

} // cGIBLib::getTricks()
// *****************************************************************************

int cGIBLib::numCard()
{ return (nPlayerCard(0)+nPlayerCard(1)+ nPlayerCard(2)+nPlayerCard(3));}

int cGIBLib::nPlayerCard(int pl)
{ return  bitCount(mPlayerSuit[pl][0])+bitCount(mPlayerSuit[pl][1])
         +bitCount(mPlayerSuit[pl][2])+bitCount(mPlayerSuit[pl][3]);
}
// *****************************************************************************

bool cGIBLib::playCard(int suit, int card)
{
  int player, pl, c;

  if((suit<0) || (suit>3) || (card<0) || (card>12))
  { sprintf(szErrMsg,"*** error: invalid playcard suit=%d card=%d\n",
            suit,card);
    return false;
  }

  // find player
  player = -1;
  for(pl=0; pl<4; pl++)
  { if(isBit(mPlayerSuit[pl][suit],card))
    { player = pl;
      break;
    }
  }
  if(player == -1)
  { sprintf(szErrMsg,"*** error: card %c%c not found in hands\n",
            chSUIT[suit],chCARD[card]);
    return false;
  }

  // set/test player
  if((suit == SuitPlayed[0]) && (card == CardPlayed[0]))
    Player = Leader = player;
  else
  { // check the player, it must be same as current player
    if(player != Player)
    { sprintf(szErrMsg,"*** error: wrong player(%c) for card %c%c\n",
              chPLAYER[player],chSUIT[suit],chCARD[card]);
      return false;
    }
  }

  // player must follow suit
  if(   (nTrickCard != 0)
     && bitCount(mPlayerSuit[player][SuitLed])
     && (suit != SuitLed))
  { sprintf(szErrMsg,"*** error: card %c%c is not following suit led %c\n",
            chSUIT[suit],chCARD[card],chSUIT[SuitLed]);
    return false;
  }

  // update the player's cards
  clearBit(mPlayerSuit[player][suit],card);

  if((nTrickCard % 4) == 0)
  { // first card of trick, remember suitled and card played
    SuitLed = suit;
    TrickStart = 4 * nTrick;
  }

  // set this player
  PlayerPlayed[4*nTrick+nTrickCard] = player;
  SuitPlayed[4*nTrick+nTrickCard] = suit;
  CardPlayed[4*nTrick+nTrickCard] = card;

  nTrickCard++;
  if(nTrickCard == 4)
  { // end of trick, update leader and tricks
    c = TrickStart;
    int winningplayer = PlayerPlayed[c];
    int winningsuit   = SuitPlayed[c];
    int winningcard   = CardPlayed[c];
    for(c=TrickStart+1; c<TrickStart+4; c++)
    { suit = SuitPlayed[c];
      card = CardPlayed[c];
      if(   ((suit==Trumps) && ((winningsuit!=Trumps) || (card<winningcard)))
         || ((suit==SuitLed) && (winningsuit==SuitLed) && (card<winningcard)))
      { // new winner
        winningplayer = PlayerPlayed[c];
        winningsuit   = suit;
        winningcard   = card;
      }
    }
    if(winningplayer & 1)
      nTrickSN++;
    else
      nTrickWE++;
    WinningPlayer[nTrick] = winningplayer;
    nTrickCard = 0;
    nTrick++;
    Leader = Player = winningplayer;
  }
  else
    Player = ((player+1) & 3);

  nPlayed++;

  return true;

} // cGIBLib::playCard
// *****************************************************************************

void cGIBLib::print()
{
  int trumps;
  char sz[8];

  printf("===========================================================================\n"
         "  deal: %s\n",szDeal);
  sz[4] = '\0';
  printf("tricks: ");
  for(trumps=0; trumps<5; trumps++)
  { strncpy(sz,szTricks+4*trumps,4);
    printf("%s ",sz);
  }
  printf("\n");
  printf("leader: %c\n",chPLAYER[Leader]);
  if(pszComment)
    printf("%s",pszComment);

} // cGIBLib::print
// *****************************************************************************

void cGIBLib::printHands()
{
  int pl, suit, maxhand[4], nplayersuit;
  int maxleft, maxmiddle;
  int startleft, startmiddle, startright;
  char line[80];

  memset(maxhand,0,4*sizeof(int));
  for(pl=0; pl<4; pl++)
  { for(suit=0; suit<4; suit++)
    { nplayersuit = bitCount(mPlayerSuit[pl][suit]);
      if(maxhand[pl] < nplayersuit)
        maxhand[pl] = nplayersuit;
    }
  }

  maxleft = maxhand[ePLAYER_WEST] + 6;
  maxmiddle = maxhand[ePLAYER_NORTH];
  if(maxmiddle < maxhand[ePLAYER_SOUTH])
    maxmiddle = maxhand[ePLAYER_SOUTH];
  maxmiddle += 6;

  startleft = 2;
  startmiddle = startleft + maxleft;
  startright = startmiddle + maxmiddle;

  // north hand
  for(suit=0; suit<4; suit++)
  { memset(line,' ',80);
    printSuit(mPlayerSuit[ePLAYER_NORTH][suit],line+startmiddle);
    printf("%s\n",line);
  }

  // west and east hands
  for(suit=0; suit<4; suit++)
  { memset(line,' ',80);
    printSuit(mPlayerSuit[ePLAYER_WEST][suit],line+startleft);
    line[strlen(line)] = ' ';
    printSuit(mPlayerSuit[ePLAYER_EAST][suit],line+startright);
    printf("%s\n",line);
  }

  // south hand
  for(suit=0; suit<4; suit++)
  { memset(line,' ',80);
    printSuit(mPlayerSuit[ePLAYER_SOUTH][suit],line+startmiddle);
    printf("%s\n",line);
  }

} // cGIBLib::printHands
// *****************************************************************************

void cGIBLib::printInfo()
{
  printf("contract:%s  play:%c",
         szCONTRACT[Trumps],chPLAYER[Player]);
  if(nTrickCard)
  { int c = 0;
    printf("  played: ");
    for(c=0; c<nTrickCard; c++)
    { printf("%c%c ",chSUIT[SuitPlayed[nPlayed-nTrickCard+c]],
             chCARD[CardPlayed[nPlayed-nTrickCard+c]]);
    }
  }
  printf("\n");
  if(nTrick)
    printf("tricks: played=%d  sn=%d  we=%d\n",
           nTrick,nTrickSN,nTrickWE);

} // cGIBLib::printInfo
// *****************************************************************************

char *cGIBLib::printSuit(ushort m, char sz[])
{
  int c;

  if(m)
  { sz[0] = '\0';
    while(m)
    { c = leastSignificant1Bit(m);
      clearBit(m,c);
      sprintf(sz+strlen(sz),"%c ",chCARD[c]);
    }
  }
  else
    strcpy(sz,"-");

  return sz;

} // cGIBLib::printSuit
// *****************************************************************************

bool cGIBLib::readComment()
{
  // must be called with position at '{'
  // note that we also save the '{' and '}' delimiters

  char *pchstart, chkeep;
   int len, pos;

  pchstart = pch;
  pos = 0;
  pszComment[0] = '\0';

  while(*pch != '}')
  { pch++;
    if(*pch == '\0')
    { // line end, save the line sofar, read next line
      len = static_cast<int>(strlen(pchstart));
      pszComment = static_cast<char*>(realloc(pszComment,static_cast<size_t>(pos+len+2)*sizeof(char)));
      strcpy(pszComment+pos,pchstart);
      pos += len;
      strcpy(pszComment+pos,"\n");
      pos++;
      if(readLine() == false)
        return false;
      pchstart = szLine;
    }
  }

  // add last bit
  pch++;
  chkeep = *pch;
  *pch = '\0';
  len = static_cast<int>(strlen(pchstart));
  pszComment = static_cast<char*>(realloc(pszComment,static_cast<size_t>(pos+len+2)*sizeof(char)));
  strcpy(pszComment+pos,szLine);
  pos += len;
  strcpy(pszComment+pos,"\n");
  *pch = chkeep;

  return true;

} // cGIBLib::readComment
// *****************************************************************************

bool cGIBLib::readDeal(FILE *fileptr)
{
  fp = fileptr;

  reset();

  for(;;)
  { if(readLine() == false)
      return false;
    while(isspace(*pch))
      pch++;
    if(*pch == '{')
    { if(pszComment[0] == '\0')
      { if(readComment() == false)
          return false;
        if(getOptions() == false)
          return false;
      }
      else
      { sprintf(szErrMsg,"*** error: more than one comment at line %d\n",nLine);
        return false;
      }
    }
    else if(*pch == '\0')
    { // skip empty line
      continue;
    }
    else
    { // neither comment nor empty line, this is a deal
      break;
    }
  }

  pch = strchr(szLine,':');
  if(pch)
    *pch = '\0';
  strcpy(szDeal,szLine);
  if(pch)
  { pch++;
    if(strlen(pch) != 20)
    { sprintf(szErrMsg,"*** error: length tricks not 20 at line %d\n",
              nLine);
      return false;
    }
    strcpy(szTricks,pch);
  }

  return true;

} // cGIBLib::readDeal
// *****************************************************************************

bool cGIBLib::readFile(int ideal0, char *pszname, FILE *fileptr)
{
  fp = fileptr;
  nLine = 0;

  reset();

  for(;;)
  { if(readLine() == false)
      return false;
    while(isspace(*pch))
      pch++;
    if(*pch == '{')
    { if(pszComment[0] == '\0')
      { if(readComment() == false)
          return false;
        if(getOptions() == false)
          return false;
      }
      else
      { sprintf(szErrMsg,"*** error: more than one comment at line %d\n",nLine);
        return false;
      }
    }
    else if(*pch == '\0')
    { // skip empty line
      continue;
    }
    else
    { // neither comment nor empty line, this is a deal
      if(pszname)
      { if(strcmp(pszname,pszName) == 0)
          break;
      }
      else
      { ideal0--;
        if(ideal0 < 0)
          break;
      }
      reset();
    }
  }

  pch = strchr(szLine,':');
  if(pch)
    *pch = '\0';
  strcpy(szDeal,szLine);
  if(pch)
  { pch++;
    if(strlen(pch) != 20)
    { sprintf(szErrMsg,"*** error: length tricks not 20 at line %d\n",nLine);
      return false;
    }
    strcpy(szTricks,pch);
  }

  if(setDeal() == false)
    return false;

  return true;

} // cGIBLib::readFile(ideal0,fp)
// *****************************************************************************

bool cGIBLib::readLine()
{
  char *pchcr;

  bFileEnd = false;
  nLine++;

  if(fgets(szLine,GIBLIB_LENLINE,fp) == 0)
  { if(feof(fp))
    { bFileEnd = true;
      sprintf(szErrMsg,"*** error: end-of-file reading line %d of file\n",nLine);
      return false;
    }
    else
    { sprintf(szErrMsg,"*** error: %s: reading line %d of file\n",
              strerror(errno),nLine);
      return false;
    }
  }

  // remove any '\r' and '\n',
  pchcr = strchr(szLine,'\r');
  if(pchcr)
    *pchcr = '\0';
  pchcr = strchr(szLine,'\n');
  if(pchcr)
    *pchcr = '\0';

  pch = szLine;

  return true;

} // cGIBLib::readLine
// *****************************************************************************

void cGIBLib::reset()
{
  pszName[0] = '\0';
  pszComment[0] = '\0';
  Trumps = 4;                 // default notrump
  Leader = 0;                 // default west
  memset(szTricks,'-',20);
  szTricks[20] = '\0';
  nPlayed = 0;

} // cGIBLib::reset
// *****************************************************************************

bool cGIBLib::setDeal()
{
   int pl, s, nplayer, c;
  char *pc;
  ushort msuit[4];

  pc = szDeal;
  memset(mPlayerSuit,0,4*4*sizeof(ushort));
  memset(msuit,0,4*sizeof(ushort));
  pl = ePLAYER_WEST;
  s = eSUIT_SPADE;
  nplayer = 1;
  for(;;)
  { if(nplayer > 4)
    { sprintf(szErrMsg,"*** error: deal too many players\n");
      return false;
    }
    else if(pc[0] == '\0')
      break;
    else if(pc[0] == ' ')
    { // next player
      pl = ((pl+1) & 3);
      s = eSUIT_SPADE;
      nplayer++;
    }
    else if(*pc == '.')
    { // next suit
      s++;
      if(s > eSUIT_CLUB)
      { sprintf(szErrMsg,"*** error: deal too many suits\n");
        return false;
      }
    }
    else
    { if(bitCount(mPlayerSuit[pl][s]) >= 13)
      { sprintf(szErrMsg,"*** error: more than 13 cards in suit at line %d\n",
                nLine);
        return false;
      }
      c = getCard(pc[0]);
      if(c == eCARD_NONE)
      { sprintf(szErrMsg,"*** error: invalid deal card %c line %d\n",
                pc[0],nLine);
        return false;
      }
      if(isBit(mPlayerSuit[pl][s],c))
      { sprintf(szErrMsg,"*** error: duplicate card %c player %c\n",
                chCARD[c],chPLAYER[pl]);
        return false;
      }
      if(isBit(msuit[s],c))
      { sprintf(szErrMsg,"*** error: duplicate card %c in more than one hand\n",
                chCARD[c]);
        return false;
      }
      setBit(msuit[s],c);
      setBit(mPlayerSuit[pl][s],c);
    }
    pc++;
  }

  // test equal number of cards in each hand
  if(   (nPlayerCard(0) != nPlayerCard(1))
     || (nPlayerCard(0) != nPlayerCard(2))
     || (nPlayerCard(0) != nPlayerCard(3)))
  { sprintf(szErrMsg,"*** error: different number of cards in hands line %d\n",
            nLine);
    return false;
  }

  // play specified cards
  if(setPlayed() == false)
    return false;

  return true;

} // cGIBLib::setDeal
// ******************************************************************************

bool cGIBLib::setGeneratedDeal()
{
     int pos, player, suit, c;
  ushort m;

  reset();

  pos = 0;
  for(player=0; player<4; player++)
  { for(suit=0; suit<4; suit++)
    { if(suit > 0)
      { szDeal[pos] = '.';
        pos++;
      }
      m = mPlayerSuit[player][suit];
      while(m)
      { c = leastSignificant1Bit(m);
        clearBit(m,c);
        szDeal[pos] = chCARD[c];
        pos++;
      }
    }
    szDeal[pos] = ' ';
    pos++;
  }
  szDeal[pos-1] = '\0';

  if(setDeal() == false)
    return false;
  if(setPlayed() == false)
    return false;

  return true;

} // cGIBLib::setGeneratedDeal
// *****************************************************************************

bool cGIBLib::setPlayed()
{
  int i, nplayed;

  Player = Leader;
  nTrick = nTrickCard = nTrickSN = nTrickWE = 0;

  nplayed = nPlayed;
  nPlayed = 0;
  for(i=0; i<nplayed; i++)
  { if(!playCard(SuitPlayed[i],CardPlayed[i]))
      return false;
  }

  return true;

} // cGIBLib::setPlayed
// *****************************************************************************

void cGIBLib::setName(char *pszname)
{
  char *pch_local=pszname;
  int len = 0;

  while((*pch_local != ' ') && (*pch_local != '\n') && (*pch_local != '\0') && (*pch_local != '}'))
  { len++;
    pch_local++;
  }

  pszName = static_cast<char*>(realloc(pszName,static_cast<size_t>(len+1)*sizeof(char)));
  strncpy(pszName,pszname, static_cast<size_t>(len));
  pszName[len] = '\0';

} // cGIBLib::setName
// *****************************************************************************

bool cGIBLib::skipComment()
{
  // must be called with position at '{'

  while(*pch != '}')
  { pch++;
    if(*pch=='\0')
    { // line end, read next line
      if(readLine() == false)
        return false;
    }
  }
  if(readLine() == false)
  { if(bFileEnd == false)
      return false;
  }

  return true;

} // cGIBLib::skipComment
// *****************************************************************************

bool cGIBLib::unplayCard()
{
  // unplay the last card

  int player, suit, card;

  if(nPlayed == 0)
  { sprintf(szErrMsg,"*** error: no cards to unplay in giblibPlayCard()\n");
    return false;
  }

  nPlayed--;
  player = PlayerPlayed[nPlayed];
  suit   = SuitPlayed[nPlayed];
  card   = CardPlayed[nPlayed];

  // update the player's cards
  setBit(mPlayerSuit[player][suit],card);

  nTrickCard = ((nTrickCard -1) & 3);
  if(nTrickCard == 3)
  { nTrick--;
    if(WinningPlayer[nTrick] & 1)
      nTrickSN--;
    else
      nTrickWE--;
    TrickStart = 4 * nTrick;
    SuitLed = SuitPlayed[TrickStart];
    Leader  = PlayerPlayed[TrickStart];
  }
  Player = PlayerPlayed[nPlayed];

  return true;

} // cGIBLib::unplayCard
// *****************************************************************************
