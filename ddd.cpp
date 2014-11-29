/* **************************************************************************
   ddd.cpp  bridge double dummy driver
            PM Cronje June 2006

   Copyright 2006 P.M.Cronje

   This file is part of the Double Dummer Driver (DDD).

   DDD is a driver for the double dummy solver DDS,
   released separately under the GPL by Bo Haglund.

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
// Changes in cleanSB 07-10-11 by Bo Haglund due to changes in dds 1.1.7 */ 

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>


// -----------------------------------------------------------------------------

// HOW TO COMPILE AND LINK THIS WITH THE LATEST VERSION OF DDS
//
// The following is suggested:
//
//   - obtain the latest dds11x.cpp and dds11x.h file for version 1.1x
//     of Bo Haglund's DDS and copy them into this directory
//
// Linux: compile and link as follows:
//   g++ -O2 -Wall -o ./ddd ddd.cpp dds11x.cpp defs.cpp timer.cpp giblib.cpp rng.cpp
//
// Windows: compile and link as follows:
//   g++ -O2 -Wall -o ddd.exe ddd.cpp dds11x.cpp defs.cpp timer.cpp giblib.cpp rng.cpp
//
// for debugging change the switch '-O2' to '-g'
//
// Note: on MingW you must have _WIN32 defined to compile code in timer.cpp
//
// -----------------------------------------------------------------------------

#include "dds/include/dll.h"
#include "portab_DDD.h"
#include "giblib.h"
#include "timer.h"

#define szVERSION "1.05"
#define szVERSION_DATE "25-Jan-2007"

/* v1.03 10-Jul-2006  add set tricks to -giblib option
                      add -tricks option
   v1.04 12-Jul-2006  changed giblib to C++ class cGIBLib
                      (not struct sGIBLIb anymore)
         13-jul-2006  add generateDeal(. ..) in class cGibLib
         14-Jul-2006  add optional flag 'all' for option
                         -giblib=d1-d2[-all]
                      changes to szTricks etc. in class cGibLib
         15-Jul-2006  fix 0 elapsed times for -gen
         25-Jan-2007  changes to use dds11x
*/

// -----------------------------------------------------------------------------
// prototypes defined in this file
// -----------------------------------------------------------------------------

void cleanSB();
void getSBScore(const struct futureTricks &fut, int *pmaxscore,
                bool bscore[14], ushort m[14][4]);
bool generate(int gen, unsigned int genseed, int gencards, int gentricks);
bool giblib(char *pszfile, int target, int sol, int mode, char *pszgiblib);
FILE *openFile(char *pszfile);
void playDD(cGIBLib *pgib, int target, int sol);
void printSBScore(int target, int solutions, struct futureTricks *pfut,
                  double elapsed);
void printSBScore(bool bscore[14], ushort m[14][4]);
bool setDDS(cGIBLib *pgib, struct deal *pdl);
bool testSBCode(int sbcode);
bool timeAll(char *pszfile, int trumps, int leader);
bool timeg(char *pszfile, int target, int sol, int mode,
           char *pszxcn, int leader, bool bverbose);
bool tricks(cGIBLib *pgib, int ideal, int target, int sol, int mode);

// -----------------------------------------------------------------------------
// macros to convert to/from DDS players/cards
// -----------------------------------------------------------------------------

#define PLAYER2DDS(pl) ((pl-1)&3)
#define CARD2DDS(card) (14-card)

#define DDS2PLAYER(ddspl) ((ddspl+1)&3)
#define DDS2CARD(ddscard) (14-ddscard)

#define szGENFILE "gen.txt"

// *****************************************************************************
// DDD definitions
// *****************************************************************************

int main(int argc, char *argv[])
{
  cGIBLib gib;
  char *pszfile=0, *pszname=0, *pszxcn=0, *pszgiblib=0;
  int iarg, target=-1, solutions=3, mode=1, deal=1, trumps=-1, leader=-1;
  int sbcode;
  bool bok=false, btimeall=false, bplaydd=false, bverbose=false, btricks=false;
   int gen=0, gencards=52, gentricks=1;
  unsigned int genseed=0;
  FILE *fp;
  cTimer timer;

  struct deal dl;
  struct futureTricks fut;

  printf("\nDouble Dummy Driver (DDD) version %s (%s)\n",
         szVERSION,szVERSION_DATE);

  if(argc < 2)
  { printf(
    "\n"
    "DDD usage:\n"
    "      ddd_executable file [opts]\n"
    "where:\n"
    "                file : path for 'giblib' input file \n"
    "optional arguments [opts] are one or more of: \n"
    "                  -v : verbose where applicable \n"
    "           -target=d : (default -1), see dll reference \n"
    "              -sol=d : solution 1/2/3 (default 3), see dll reference \n"
    "             -mode=d : 0/1 (default 1), see dll reference \n"
    "           -trumps=t : s/h/d/c/n, this overrides the file (default=n) \n"
    "           -leader=l : w/n/e/s, this overrides the file (default=w) \n"
    "                       but used only when no cards have been played \n"
    "             -deal=d : 1/2/... deal number in giblib file \n"
    "                       only one of -deal or -name should be specified \n"
    "           -name=str : deal with 'name=str' in giblib file \n"
    "                       only one of -deal or -name should be specified \n"
    "             -playdd : play deal choosing between DDS alternatives \n"
    "            -timeall : time all deals in file for sol=1/2/3, print stats \n"
    "          -timeg=xcn : x - hex digit, total tricks by n-s \n"
    "                       c - contract s/h/d/c/n \n"
    "                       n - number of deals \n"
    "                       time the first n deals in the giblib file, \n"
    "                       having total tricks x at contract c, \n"
    "                       for target=-1 sol=1 mode=1 \n"
    "                       and for the specified/default leader,\n"
    "                       each deal is validated \n"
    " -giblib=d1-d2[-all] : validate all deals from d1 to d2 in giblib file \n"
    "                       for target=-1 sol=1 mode=1 \n"
    "                       1. if '-all' is given, this is done for all of  \n"
    "                          the 20 trick values even if some of them are '-'   \n"
    "                       2. if '-all' is not given, this is done only for \n"
    "                          those trick values which are not '-' \n"
    "             -tricks : like -giblib, but for single deal specified \n"
    "                       by -name=str -deal=d or option \n"
    " generate deals: \n"
    "              -gen=n : (required) n=number of deals to generate \n"
    "                       output is written to a file (see below) \n"
    "          -genseed=s : (default 0) seed for random generator \n"
    "         -gencards=c : (default=52) number of cards generated per deal,\n"
    "                       must be multiple of 4 \n"
    "        -gentricks=t : 0,1,...,20 (default 1), number of tricks values \n"
    "                       to set randomly \n"
    " generate output is written to a file:        \n"
    "    gen-'genseed'-'ndeal'-'gencards'-'gentricks'.txt \n"
    "\n"
    );
    return -1;
  }
  for(iarg=1; iarg<argc; iarg++)
  {
    if(argv[iarg][0] == '-')
    {
      if(strncasecmp(argv[iarg],"-target=",8) == 0)
        target = atol(argv[iarg]+8);
      else if(strcasecmp(argv[iarg],"-timeall") == 0)
        btimeall = true;
      else if(strncasecmp(argv[iarg],"-timeg=",7) == 0)
        pszxcn = argv[iarg] + 7;
      else if(strncasecmp(argv[iarg],"-giblib=",8) == 0)
        pszgiblib = argv[iarg] + 8;
      else if(strncasecmp(argv[iarg],"-tricks",7) == 0)
        btricks = true;
      else if(strcasecmp(argv[iarg],"-playdd") == 0)
        bplaydd = true;
      else if(strncasecmp(argv[iarg],"-gen=",5) == 0)
        gen = atol(argv[iarg]+5);
      else if(strncasecmp(argv[iarg],"-genseed=",9) == 0)
        genseed = static_cast<unsigned int>(atol(argv[iarg]+9));
      else if(strncasecmp(argv[iarg],"-gencards=",10) == 0)
        gencards = atol(argv[iarg]+10);
      else if(strncasecmp(argv[iarg],"-gentricks=",11) == 0)
        gentricks = atol(argv[iarg]+11);
      else if(strncasecmp(argv[iarg],"-sol=",5) == 0)
        solutions = atol(argv[iarg]+5);
      else if(strncasecmp(argv[iarg],"-mode=",6) == 0)
      { mode = atol(argv[iarg]+6);
        if(mode < 0)
          mode = 0;
        else if(mode > 0)
          mode = 1;
      }
      else if(strncasecmp(argv[iarg],"-trumps=",8) == 0)
      { if(tolower(argv[iarg][8]) == 's') trumps = 0;
        else if(tolower(argv[iarg][8]) == 'h') trumps = 1;
        else if(tolower(argv[iarg][8]) == 'd') trumps = 2;
        else if(tolower(argv[iarg][8]) == 'c') trumps = 3;
        else if(tolower(argv[iarg][8]) == 'n') trumps = 4;
        else
        { printf("*** error: invalid trumps option '%s'\n",argv[iarg]);
          return -1;
        }
      }
      else if(strncasecmp(argv[iarg],"-leader=",8) == 0)
      { if(tolower(argv[iarg][8]) == 'w') leader = 0;
        else if(tolower(argv[iarg][8]) == 'n') leader = 1;
        else if(tolower(argv[iarg][8]) == 'e') leader = 2;
        else if(tolower(argv[iarg][8]) == 's') leader = 3;
        else
        { printf("*** error: invalid leader option '%s'\n",argv[iarg]);
          return -1;
        }
      }
      else if(strncasecmp(argv[iarg],"-deal=",6) == 0)
      { deal = atol(argv[iarg]+6);
        if(deal < 1)
          deal = 1;
      }
      else if(strncasecmp(argv[iarg],"-name=",6) == 0)
        pszname = argv[iarg] + 6;
      else if(strcasecmp(argv[iarg],"-v") == 0)
        bverbose = true;
      else
      { printf("*** error: invalid option '%s'\n",argv[iarg]);
        return -1;
      }
    }
    else
      pszfile = argv[iarg];
  }

  SetMaxThreads(0);

  if(gen > 0)
  { generate(gen,genseed,gencards,gentricks);
    goto cleanup;
  }
  if(btimeall)
  { timeAll(pszfile,trumps,leader);
    goto cleanup;
  }
  else if(pszxcn)
  { timeg(pszfile,-1,1,1,pszxcn,(leader==-1?0:leader),bverbose);
    goto cleanup;
  }
  else if(pszgiblib)
  { giblib(pszfile,-1,1,1,pszgiblib);
    goto cleanup;
  }

  // open file
  fp = openFile(pszfile);
  if(fp == 0)
    return -1;

  // read giblib deal
  bok = gib.readFile(deal-1,pszname,fp);
  fclose(fp);
  if(!bok)
  { printf("%s",gib.szErrMsg);
    return -1;
  }

  if(btricks)
  { tricks(&gib,deal,-1,1,1);
    goto cleanup;
  }

  // if overridden from command line, set trumps/leader
  if(trumps != -1)
    gib.Trumps = trumps;
  if((gib.nPlayed == 0) && (leader != -1))
  { // override leader/player only when no cards have been played
    gib.Leader = gib.Player = leader;
  }

  if(bplaydd)
  { playDD(&gib,target,solutions);
    goto cleanup;
  }

  // set up dds10
  if(setDDS(&gib,&dl) == false)
    return -1;

  printf("\n");
  gib.print();
  printf("\n");
  gib.printHands();
  gib.printInfo();
  printf("\n");
  fflush(stdout);

  timer.start();
  sbcode = SolveBoard(dl,target,solutions,mode,&fut,0);
  timer.check();
  if(testSBCode(sbcode) == false)
    exit(-1);
  printSBScore(target,solutions,&fut,timer.dblElapsed());

  cleanup:
  cleanSB();

  return 0;

} // main
// *****************************************************************************

void cleanSB()
{
}
// *****************************************************************************

bool generate(int gen, unsigned int genseed, int gencards, int gentricks)
{
  // generate deals
  // write giblib extended format to specified file

  cGIBLib gib;
      int ntrick, sbcode, ideal, trickpos, trumps, leader;
      int settrick[20], setpos[20], nsettrick, itrick;
      int maxscore, nerror, score, ntotal;
     bool bscore[14];
   ushort m[14][4];

  FILE *fp;
  cTimer timer;
  struct deal dl;
  struct futureTricks fut;

  double elapsed, totalelapsed;
  unsigned long long nodes, totalnodes;
  char sz1[32], szfile[256];

  int target=-1, sol=1, mode=1;

  // check arguments
  sprintf(szfile,"gen-%u-%d-%d-%d.txt",genseed,gen,gencards,gentricks);
  // if existing file, warn the user

  fp = fopen(szfile, "r");
  if (fp)
  { 
    fclose(fp);
    printf("\n*** WARNING: generate deals\n"
             "             the file '%s' is an existing file\n"
             "             and it may be overwritten\n"
             "do you want to continue? (y/n)):",szfile);
    fflush(stdout);
    char buf[255], *pch;
    pch = fgets(buf,255,stdin);
    if(pch == 0)
      return false;
    if(tolower(*pch) != 'y')
      return false;
  }

  if(gen < 0)
  { printf("*** error: gen=%d invalid number of deals to generate\n",gen);
    return false;
  }
  if((gencards < 4) || (gencards > 52))
  { printf("*** error: gencards=%d must be >=4 and <=52\n",gencards);
    return false;
  }
  if((gencards % 4) != 0)
  { printf("*** error: gencards=%d not a multiple ofg 4\n",gencards);
    return false;
  }
  if((gentricks < 0) || (gentricks > 20))
  { printf("*** error: gentricks=%d must be >=0 and <=20\n",gentricks);
    return false;
  }

  fp = fopen(szfile,"w");
  if(fp == 0)
  { printf("*** error: : cannot open gen file %s for writing\n", szfile);
    return false;
  }

  printf("\ngenerate: seed=%u deals=%d cards=%d tricks=%d file=%s\n",
         genseed,gen,gencards,gentricks,szfile);
  fflush(stdout);

  gib.setRNGSeed(genseed);

  timer.start();

  nerror = ntotal = 0;
  elapsed = totalelapsed = 0;
  nodes = totalnodes = 0;

  for(ideal=0; ideal<gen; ideal++)
  { // loop over generated deal

    // generate the deal
    if(gib.generateDeal(gencards/4) == false)
      return false;
    if(gib.setGeneratedDeal() == false)
      break;

    // set tricks
    if(gentricks > 0)
    {
      // initialize trick positions
      for(trickpos=0; trickpos<20; trickpos++)
        settrick[trickpos] = trickpos;
      nsettrick = 20;

      // get sorted trick positions
      for(itrick=0; itrick<gentricks; itrick++)
      { // get a random trick position
        trickpos = static_cast<int>(gib.pRNG->randomUint
	  (static_cast<unsigned>(nsettrick)));
        setpos[itrick] = settrick[trickpos];
        if(trickpos < nsettrick-1)
          memmove(settrick+trickpos,settrick+trickpos+1,
                 static_cast<size_t>(nsettrick-1-trickpos)*sizeof(int));
        nsettrick--;
      }

      // compute trick values
      for(itrick=0; itrick<gentricks; itrick++)
      { timer.check();
        trickpos = setpos[itrick];
        gib.getTricks(trickpos,&leader,&trumps,&ntrick);
        // set up dds10
        gib.Trumps = trumps;
        gib.Leader = leader;
        if(setDDS(&gib,&dl) == false)
          return false;
        sbcode = SolveBoard(dl,target,sol,mode,&fut,0);
        if(testSBCode(sbcode) == false)
          return false;
        getSBScore(fut,&maxscore,bscore,m);
        if(maxscore < 0)
        { printf("*** error: deal=%d leader=%c : no score\n",
               ideal+1,chPLAYER[gib.Leader]);
          return false;
        }
        score = maxscore;
        if(score >= 0)
          score = ((gib.Leader & 1) ? score: gib.numCard()/4-score);

        timer.check();
        nodes = static_cast<unsigned long long>(fut.nodes);
        totalnodes += nodes;
        elapsed = timer.dblDeltaElapsed();

        printf("  deal=%d leader=%c %d%c nodes=%s elapsed=%0.2f\n",
               ideal+1,chPLAYER[gib.Leader],score,(trumps==4)?'N':chSUIT[trumps],
               format64(nodes,sz1),elapsed);

        if(score < 10)
          gib.szTricks[trickpos] = static_cast<char>(score + '0');
        else
          gib.szTricks[trickpos] = static_cast<char>(score -10 + 'A');
      }
    }

    // write the deal to file
    fprintf(fp,"%s:%s\n",gib.szDeal,gib.szTricks);
    // flush file, so that cancel does not lose data
    fflush(fp);

    // print the deal
    printf("%d %s:%s\n",ideal+1,gib.szDeal,gib.szTricks);
  }

  timer.check();
  totalelapsed = timer.dElapsed;
  printf("deals=%d nodes=%s elapsed=%0.2f\n"
         "output written to file %s\n",
         gen,format64(totalnodes,sz1),
         totalelapsed,szfile);

  fclose(fp);
  return true;

} // generate
// *****************************************************************************

void getSBScore(const struct futureTricks &fut, int *pmaxscore,
                bool bscore[14], ushort m[14][4])
{
  // get DDS 'score' after SolveBoard(..) has been run

  int iscore, suit, alt;

  for(iscore=0; iscore<14; iscore++)
    bscore[iscore] = false;
  memset(m,0,14*4*sizeof(ushort));
  *pmaxscore = -1;

  for(alt=0; alt<fut.cards; alt++)
  { iscore = fut.score[alt];
    if(iscore == -1)
      continue;
    if(fut.rank[alt])
    { bscore[iscore] = true;
      if(*pmaxscore < iscore)
        *pmaxscore = iscore;
      suit = fut.suit[alt];
      setBit(m[iscore][suit],14-fut.rank[alt]);
      ushort malt = static_cast<ushort>(fut.equals[alt]);
      while(malt)
      { int c = leastSignificant1Bit(malt);
        clearBit(malt,c);
        setBit(m[iscore][suit],14-c);
      }
    }
  }

} // getSBScore
// *****************************************************************************

bool giblib(char *pszfile, int target, int sol, int mode, char *pszgiblib)
{
  // run deals in giblib file for all possible tricks,
  // collect and print the stats

  cGIBLib gib;
      int sbcode, ideal1, ideal2, ideal, trickpos, trumps,
          tricks=0, leader;
      int maxscore, nerror, score, ntotal;
     char *pch;
     bool ball, bscore[14];
   ushort m[14][4];

  bool bok=false;
  FILE *fp;
  cTimer timer;
  struct deal dl;
  struct futureTricks fut;

  double elapsed, dealelapsed, totalelapsed;
  unsigned long long nodes, dealnodes, totalnodes;
  char sz1[32], sz2[32];

  // get/test arguments
  pch = strchr(pszgiblib,'-');
  if(pch == 0)
  { printf("*** error: giblib=%s not 'deal1-deal2[-all]'\n",pszgiblib);
    return false;
  }
  ideal1 = atol(pszgiblib);
  if((ideal1 < 1) || (strlen(pszgiblib) == 0))
  { printf("*** error: giblib=%s invalid deal1\n",pszgiblib);
    return false;
  }
  pch++;
  ideal2 = atol(pch);
  if((ideal2 < 1) || (strlen(pch) == 0))
  { printf("*** error: giblib=%s invalid deal2\n",pszgiblib);
    return false;
  }
  if(ideal1 > ideal2)
  { printf("*** error: giblib=%s ideal1 > ideal2\n",pszgiblib);
    return false;
  }
  ball = false;
  pch = strchr(pch,'-');
  if(pch)
  { if(strcmp(pch,"-all") != 0)
    { printf("*** error: giblib=%s expected '-all' after ideal2\n",pszgiblib);
      return false;
    }
    ball = true;
  }

  fp = openFile(pszfile);
  if(fp == 0)
    return false;

  printf("\n"
         "giblib=%s target=%d sol=%d mode=%d\n",pszgiblib,target,sol,mode);
  fflush(stdout);

  timer.start();

  nerror = ntotal = 0;
  elapsed = totalelapsed = 0;
  nodes = totalnodes = 0;

  // skip to startdeal
  for(ideal=1; ideal<ideal1; ideal++)
  { bok = gib.readDeal(fp);
    if(!bok)
      return false;
  }

  for(ideal=ideal1; ideal<=ideal2; ideal++)
  { // loop over deals in file

    // read giblib deal
    bok = gib.readDeal(fp);
    if(!bok)
      break;
    if(gib.setDeal() == false)
      break;

    if((gib.numCard() % 4) != 0)
    { printf("  warning: deal=%d cards=%d not multiple of 4, skipping deal ...\n",
           ideal,gib.numCard());
      continue;
    }

    // Note that we are using gib.szTricks internally,
    //   - if not read, we are setting it,
    //   - in addition we are correcting any errors.

    dealnodes = 0;
    dealelapsed = 0.0;

    for(trickpos=0; trickpos<20; trickpos++)
    {
      if(gib.getTricks(trickpos,&leader,&trumps,&tricks) == false)
      { if(ball == false)
          continue;
      }

      // set up dds10
      gib.Trumps = trumps;
      gib.Leader = leader;
      if(setDDS(&gib,&dl) == false)
        return false;

      sbcode = SolveBoard(dl,target,sol,mode,&fut,0);
      timer.check();
      if(testSBCode(sbcode) == false)
        return false;

      getSBScore(fut,&maxscore,bscore,m);
      if(maxscore < 0)
      { printf("*** error: deal=%d leader=%c no score\n",
               ideal,chPLAYER[gib.Leader]);
        return false;
      }
      score = maxscore;
      if(score >= 0)
        score = ((gib.Leader & 1) ? score: gib.numCard()/4-score);

      nodes = static_cast<unsigned long long>(fut.nodes);
      dealnodes += nodes;
      totalnodes += nodes;
      elapsed = timer.dblDeltaElapsed();
      dealelapsed += elapsed;
      totalelapsed += elapsed;

      if(gib.szTricks[trickpos] != '-')
      { printf("    deal=%d leader=%c %d%c score=%d nodes=%s elapsed=%0.2f\n",
               ideal,chPLAYER[gib.Leader],tricks,(trumps==4)?'N':chSUIT[trumps],
               score,format64(nodes,sz1),elapsed);
        bok = true;

        // test that tricks and score agree
        if((score < 0) || score != tricks)
        { nerror++;
          bok = false;
          printf("        error: deal=%d leader=%c %d%c score=%d\n",
                 ideal,chPLAYER[gib.Leader],tricks,(trumps==4)?'N':chSUIT[trumps],
                 score);
        }
      }
      else
      { printf("    deal=%d leader=%c %d%c nodes=%s elapsed=%0.2f\n",
               ideal,chPLAYER[gib.Leader],score,(trumps==4)?'N':chSUIT[trumps],
               format64(nodes,sz1),elapsed);
        bok = false;
      }
      if(bok == false)
      { if(score < 10)
          gib.szTricks[trickpos] = static_cast<char>
	    (score + static_cast<int>('0'));
        else
          gib.szTricks[trickpos] = static_cast<char>
	    (score - 10 + static_cast<int>('A'));
      }
      ntotal++;
    }

    printf("  deal=%d nodes=%s elapsed=%0.2f totalelapsed=%0.2f\n"
           "       ",
           ideal,format64(dealnodes,sz1),dealelapsed,totalelapsed);
    if(strlen(gib.pszName))
      printf("name=%s ",gib.pszName);
    printf("tricks=%s (max=%d)\n",
           gib.szTricks,gib.numCard()/4);
  }

  printf("deals=%s nodes=%s avg=%s elapsed=%0.2f avg=%.02f\n",
         pszgiblib,format64(totalnodes,sz1),
         ntotal ? format64(totalnodes/static_cast<unsigned>(ntotal),sz2) :
	   format64(totalnodes,sz2),
         totalelapsed,
	 ntotal ? totalelapsed / static_cast<double>(ntotal) :
	   static_cast<double>(totalelapsed));
  if(nerror)
    printf("*** ERROR: nerror=%d, tricks and score different\n",nerror);
  printf("\n");

  fclose(fp);

  return true;

} // giblib
// *****************************************************************************

FILE *openFile(char *pszfile)
{
  if(pszfile == 0)
  { printf("*** error: no 'giblib' file specified\n");
    return 0;
  }

  /*if(access(pszfile,F_OK) != 0)
  { printf("*** error: non-existing file %s\n",pszfile);
    return 0;
  }
  */ 

  FILE *fp = fopen(pszfile,"r");
  if(fp == 0)
  { printf("*** : cannot open file %s for reading\n", pszfile);
    return 0;
  }

  return fp;

} // openFile
// *****************************************************************************

void playDD(cGIBLib *pgib, int target, int sol)
{
  cTimer timer;
  double elapsed=0;
  struct deal dl;
  struct futureTricks fut;
  int sbcode, suit, card, ntotalcard;
  char *pch, *pchend, buf[255];
  bool bhelp=false, bcompute=true;
  char szplayer[4][6] = {"west","north","east","south"};

  printf("\n");
  pgib->print();
  printf("\n");

  timer.start();
  for(;;)
  {
    pgib->printHands();
    pgib->printInfo();
    printf("\n");
    fflush(stdout);

    ntotalcard = pgib->numCard();
    if(ntotalcard)
    { // DDS only when there are cards left to play
      if(bcompute)
      { // set up dds10
        if(setDDS(pgib,&dl) == false)
          return;
  
        timer.check();
        sbcode = SolveBoard(dl,target,sol,1,&fut,0);
        timer.check();
        if(testSBCode(sbcode) == false)
          exit(-1);
        elapsed += timer.dblDeltaElapsed();
      }
      printSBScore(target,sol,&fut,elapsed);
    }

    if(bhelp)
    { printf("help for options:\n"
             "  'q'  - quit \n"
             "  'h'  - print this help \n"
             "  'u'  - unplay previous card (if available) \n"
             "  'sc' - card/suit to play, e.g. dq/ht/s3\n"
             "         must be valid for current player \n"
            );
      bhelp = false;
    }

    bcompute = false;
    if(ntotalcard > 0)
      printf("enter options for %s ",szplayer[pgib->Player]);
    else
      printf("enter options ");
    if(ntotalcard <= 0)
      printf("(q/h/u): ");
    else if(pgib->nPlayed > 0)
      printf("(q/h/u/sc): ");
    else
      printf("(q/h/sc): ");
    fflush(stdout);
    pch = fgets(buf,255,stdin);
    if(pch == 0)
      continue;
    pchend = strchr(buf,'\n');
    if(pchend)
      *pchend = '\0';
    pchend = strchr(buf,'\r');
    if(pchend)
      *pch = '\0';
    while(*pch == ' ')
      pch++;
    if(*pch == '\0')
      continue;
    if(*pch == 'q')
      break;
    else if((pch[0] == 'h') && ((pch[1] == ' ') || (pch[1] == '\0')))
    { bhelp = true;
      continue;
    }
    else if((pgib->nPlayed > 0) && (*pch == 'u'))
    { if(pgib->unplayCard())
        bcompute = true;
    }
    else if(ntotalcard)
    {      if(tolower(pch[0]) == 's') suit = 0;
      else if(tolower(pch[0]) == 'h') suit = 1;
      else if(tolower(pch[0]) == 'd') suit = 2;
      else if(tolower(pch[0]) == 'c') suit = 3;
      else
        continue;
      pch++;
      card = static_cast<int>(cGIBLib::getCard(pch[0]));
      if(card == eCARD_NONE)
        continue;

      // play the card
      pgib->SuitPlayed[pgib->nPlayed] = suit;
      pgib->CardPlayed[pgib->nPlayed] = card;
      if(pgib->playCard(suit,card))
        bcompute = true;
    }
  }

} // playDD
// *****************************************************************************

void printSBScore(int target, int solutions, struct futureTricks *pfut,
                  double elapsed)
{
  // print DDS 'score' after SolveBoard(..) has been run

    char sz1[32];
    bool bscore[14];
  ushort m[14][4];
     int maxscore;

  printf("-- sb completed: nodes=%s tgt=%d sol=%d alt=%d elapsed=%0.2f\n",
         format(static_cast<unsigned>(pfut->nodes),sz1),target,solutions,pfut->cards,elapsed);
  getSBScore(*pfut,&maxscore,bscore,m);
  printSBScore(bscore,m);

} // printSBScore
// *****************************************************************************

void printSBScore(bool bscore[14], ushort m[14][4])
{
  int iscore, suit, nscore=0;

  for(iscore=0; iscore<14; iscore++)
  { if(bscore[iscore])
    { nscore++;
      printf("     %2d: ",iscore);
      for(suit=0; suit<4; suit++)
      { if(m[iscore][suit])
        { printf("%c ",chSUIT[suit]);
          while(m[iscore][suit])
          { int c = leastSignificant1Bit(m[iscore][suit]);
            clearBit(m[iscore][suit],c);
            printf("%c",chCARD[c]);
          }
          printf("  ");
        }
      }
      printf("\n");
    }
  }

  if(nscore == 0)
    printf("     none\n");

} // printSBScore
// *****************************************************************************

bool setDDS(cGIBLib *pgib, struct deal *pdl)
{
  // setup deal structure for DDS

  int pl, suit, ntrick, ntrickcard;
  int lastsuit, lastcard, card;

  pdl->trump = pgib->Trumps;               // s=0,h=1,d=2,c=3,nt=4

  // remaining cards from partial trick
  ntrick = (pgib->nPlayed / 4);
  ntrickcard = (pgib->nPlayed - 4 * ntrick);
  memset(pdl->currentTrickSuit,0,3*sizeof(int));
  memset(pdl->currentTrickRank,0,3*sizeof(int));
  if(ntrickcard > 0)
  { for(pl=0; pl<ntrickcard; pl++)
    { lastsuit = pgib->SuitPlayed[4*ntrick+pl];
      lastcard = pgib->CardPlayed[4*ntrick+pl];
      pdl->currentTrickSuit[pl] = lastsuit;
      pdl->currentTrickRank[pl] = CARD2DDS(lastcard);
    }
  }

  // DDS leader
  pdl->first = PLAYER2DDS(pgib->Leader);   // n=0,e=1,s=2,w=3

  // DDS remaining cards
  for(pl=0; pl<4; pl++)
  { int ddspl = PLAYER2DDS(pl);
    for(suit=0; suit<4; suit++)
    { ushort m=0, mp=pgib->mPlayerSuit[pl][suit];
      while(mp)
      { card = leastSignificant1Bit(mp);
        clearBit(mp,card);
        setBit(m,CARD2DDS(card));
      }
      pdl->remainCards[ddspl][suit] = static_cast<unsigned int>(m);
    }
  }

  return true;

} // setDDDS
// *****************************************************************************

bool testSBCode(int sbcode)
{
  // test DDS return code after SolveBoard(..) has been run

  if (sbcode != RETURN_NO_FAULT)
  {
    char line[80];
    ErrorMessage(sbcode, line);
    printf("*** error: %s\n", line);
    return false;
  }

  return true;

} // testSBCode
// *****************************************************************************

bool timeAll(char *pszfile, int trumps, int leader)
{
  // run all deals in giblib file for sol=1/2/3,
  // collect and print the stats

  cGIBLib gib;
  int sbcode, ideal;
  bool bok=false;
  FILE *fp;
  cTimer timer;
  struct deal dl;
  struct futureTricks fut;

  double totalelapsed=0.0, elapsed[3]={0.0,0.0,0.0};
  unsigned long long totalnodes=0, nodes[3]={0,0,0};

  #define LEN_RESULTNAME 15
  struct sResult
  {    int nNode[3];
    double Elapsed[3];
      char szName[LEN_RESULTNAME+1];
  };

  #define MAX_RESULT 10
  int sol, target=-1, mode=1, ndealalloc=0, ndeal, nresult=0;
  struct sResult *presult=0;

  for(sol=1; sol<=3; sol++)
  { ndeal = 0;
    for(;;)
    { // loop over deals in file

      // open file
      fp = openFile(pszfile);
      if(fp == 0)
        return false;

      // read giblib deal
      bok = gib.readFile(ndeal,0,fp);
      fclose(fp);
      if(!bok)
        break;

      if(trumps != -1)
        gib.Trumps = trumps;
      if(leader != -1)
        gib.Leader = leader;

      printf("\n");
      gib.print();
      printf("\n");
      gib.printHands();
      gib.printInfo();
      printf("\n");
      fflush(stdout);

      // set up dds10
      if(setDDS(&gib,&dl) == false)
        return false;

      timer.start();
      sbcode = SolveBoard(dl,target,sol,mode,&fut,0);
      timer.check();
      if(testSBCode(sbcode) == false)
        return false;
      printSBScore(target,sol,&fut,timer.dblElapsed());

      if(ndeal >= ndealalloc)
      { presult = static_cast<sResult*>
        (realloc(presult, (static_cast<size_t>(ndealalloc)+25)*sizeof(sResult)));
        if(presult == 0)
        { printf("*** error: cannot allocate result array\n");
          return false;
        }
        ndealalloc += 25;
      }

      presult[ndeal].nNode[sol-1]   = fut.nodes;
      presult[ndeal].Elapsed[sol-1] = timer.dblElapsed();
      strncpy(presult[ndeal].szName,gib.pszName,LEN_RESULTNAME);
      presult[ndeal].szName[LEN_RESULTNAME] = '\0';
      ndeal++;
      if(nresult < ndeal)
        nresult = ndeal;

      totalnodes += static_cast<unsigned long long>(fut.nodes);
      nodes[sol-1] += static_cast<unsigned long long>(fut.nodes);
      totalelapsed += timer.dblElapsed();
      elapsed[sol-1] += timer.dblElapsed();
    }
  }

  // print the results
  printf("\n"
         "test                     sol=1                 sol=2              sol=3\n"
         "---------         ----------------     ----------------      ---------------\n"
        );
  for(ideal=0; ideal<nresult; ideal++)
  { char szline[128], sz1[32], sz2[32];
    int len, pos;
    memset(szline,' ',127);
    len = static_cast<int>(strlen(presult[ideal].szName));
    if(len)
      strncpy(szline, presult[ideal].szName, static_cast<size_t>(len));
    pos = LEN_RESULTNAME + 1;
    for(sol=0; sol<3; sol++)
    { format( static_cast<unsigned>(presult[ideal].nNode[sol]),sz1);
      sprintf(sz2,"%0.2f",presult[ideal].Elapsed[sol]);
      len = static_cast<int>(strlen(sz1));
      strncpy(szline+pos+12-len,sz1, static_cast<size_t>(len));
      pos += 13;
      len = static_cast<int>(strlen(sz2));
      strncpy(szline+pos+5-len,sz2, static_cast<size_t>(len));
      pos += 8;
    }
    szline[pos] = '\0';
    printf("%s\n",szline);
  }
  printf("\n");
  { char sz1[32], sz2[32], sz3[32], sz4[32];
    printf("  nodes: total=%s  sol-1=%s  sol-2=%s  sol-3=%s\n",
           format64(totalnodes,sz1),format64(nodes[0],sz2),
           format64(nodes[1],sz3),format64(nodes[2],sz4));
  }
  printf("elapsed: total=%0.2f  sol-1=%0.2f  sol-2=%0.2f  sol-3=%0.2f\n",
         totalelapsed,elapsed[0],elapsed[1],elapsed[2]);

  return true;

} // timeAll
// *****************************************************************************

bool timeg(char *pszfile, int target, int sol, int mode,
           char *pszxcn, int leader, bool bverbose)
{
  // run and validate specified number of deals deals in
  // giblib file for given contract/leader/target/sol/mode

  cGIBLib gib;
      int sbcode, ideal, currentdeal, trickpos;
      int maxscore, nerror, score;
     bool bscore[14];
   ushort m[14][4];

  bool bok=false;
  FILE *fp;
  cTimer timer;
  struct deal dl;
  struct futureTricks fut;

  double elapsed=0.0, minelapsed=1.0e10, maxelapsed=0.0;
  unsigned long long nodes=0;
  int minnode=2000000000, maxnode=0;
  char sz1[32], sz2[32], sz3[32];
  char optx;
  int tricks, contract, ndeal;

  // get/test arguments
  if(strlen(pszxcn) < 3)
  { printf("*** error: timeg options=%s is not >=3\n",pszxcn);
    return false;
  }
  optx = pszxcn[0];
  if(!isxdigit(optx))
  { printf("*** error: timeg optionX=%c is not hex digit 3\n",optx);
    return false;
  }
  if((tolower(optx)=='e') || (tolower(optx)=='f'))
  { printf("*** error: timeg optionX=%c more than 13 tricks\n",optx);
    return false;
  }
  optx = static_cast<char>(tolower(optx));
  tricks = (static_cast<int>(optx) - static_cast<int>('0'));
  if((tricks < 0) || (tricks > 9))
    tricks = static_cast<int>(optx - static_cast<int>('a') + 10);

       if(pszxcn[1] == 's') contract = 0;
  else if(pszxcn[1] == 'h') contract = 1;
  else if(pszxcn[1] == 'd') contract = 2;
  else if(pszxcn[1] == 'c') contract = 3;
  else if(pszxcn[1] == 'n') contract = 4;
  else
  { printf("*** error: timeg contract=%c is not s/h/d/c/n\n",pszxcn[1]);
    return false;
  }
  ndeal = atol(pszxcn+2);
  if(ndeal <= 0)
  { printf("*** error: timeg ndeal=%d is not >= 0\n",ndeal);
    return false;
  }

  if(contract == 4)
    trickpos = 0;               // nt
  else
    trickpos = 4 + 4 * contract;    // s/h/d/c
  // giblib leader goes s/e/n/w
  //    our leader goes w/n/e/s
  trickpos += (3 - leader);

  fp = openFile(pszfile);
  if(fp == 0)
    return false;

  printf("\n"
         "timeg tricks=%d contract=%c deals=%d target=%d sol=%d mode=%d leader=%c\n",
         tricks,"shdcn"[contract],ndeal,target,sol,mode,chPLAYER[leader]);
  fflush(stdout);

  timer.start();

  ideal = currentdeal = nerror = 0;
  for(;;)
  { // loop over deals in file

    // read giblib deal
    bok = gib.readDeal(fp);
    if(!bok)
      break;
    if(gib.setDeal() == false)
      break;

    currentdeal++;

    if(optx != tolower(gib.szTricks[trickpos]))
      continue;

    ideal++;
    // set up dds10
    gib.Trumps = contract;
    if(leader != -1)
      gib.Leader = leader;
    if(setDDS(&gib,&dl) == false)
      return false;

    sbcode = SolveBoard(dl,target,sol,mode,&fut,0);
    timer.check();
    if(testSBCode(sbcode) == false)
      return false;

    getSBScore(fut,&maxscore,bscore,m);
    if(maxscore < 0)
    { nerror++;
      printf("\r  --- error: deal=%d ndeal=%d no score\n",
             currentdeal,ideal);
    }
    else
    { score = maxscore;
      if(score >= 0)
        //score = ((leader & 1) ? gib.numCard()/4-score : score);
        score = ((leader & 1) ? score: gib.numCard()/4-score);
      if((score < 0) || score != tricks)
      { nerror++;
        printf("\r  --- error: deal=%d ndeal=%d tricks=%d score=%d\n",
               currentdeal,ideal,tricks,score);
      }
    }

    if(!bverbose)
      printf("\r");
    printf("  deal=%d ndeal=%d nodes=%s elapsed=%0.2f",
           currentdeal,ideal,format64(static_cast<uint64>(fut.nodes),sz1),
	     timer.deltaElapsed);
    if(!bverbose)
      printf("                    ");
    else
      printf("\n");
    fflush(stdout);

    nodes += static_cast<unsigned long long>(fut.nodes);
    elapsed += timer.dblDeltaElapsed();

    if(minnode > fut.nodes)
      minnode = fut.nodes;
    if(maxnode < fut.nodes)
      maxnode = fut.nodes;

    if(minelapsed > timer.deltaElapsed)
      minelapsed = timer.deltaElapsed;
    if(maxelapsed < timer.deltaElapsed)
      maxelapsed = timer.deltaElapsed;

    if(ideal >= ndeal)
      break;
  }
  if(ideal <= 0)
    printf("no deals found\n");
  else
  { printf("\rdeals=%d nodes=%s elapsed=%0.2f                              \n",
           ndeal,format64(nodes,sz1),elapsed);
    printf("min_node=%s max_node=%s avg=%s\n",
           format(static_cast<unsigned>(minnode),sz1),
	   format(static_cast<unsigned>(maxnode),sz2),
           format(static_cast<unsigned long long>
	     ( ideal > 0 ? static_cast<unsigned>(nodes)/ static_cast<unsigned>(ideal) : static_cast<unsigned>(nodes)), 
	     sz3));
    printf("min_elapsed=%0.2f max_elapsed=%0.2f avg=%0.2f\n",
           minelapsed,maxelapsed,(elapsed>0.0)?elapsed/
	     static_cast<double>(ideal) :elapsed);
  }
  if(nerror)
    printf("*** ERROR: nerror=%d, incorrect maximum tricks\n",nerror);
  else
    printf("errors=0\n");
  printf("\n");

  fclose(fp);

  return true;

} // timeg
// *****************************************************************************

bool tricks(cGIBLib *pgib, int ideal, int target, int sol, int mode)
{
  // run deal in giblib file for all possible tricks,
  // collect and print the stats

     int sbcode, trickpos, trumps;
     int maxscore, score;
    bool bscore[14];
  ushort m[14][4];

  cTimer timer;
  struct deal dl;
  struct futureTricks fut;

  double elapsed, dealelapsed, totalelapsed;
  unsigned long long nodes, dealnodes, totalnodes;
  char sz1[32];

  if(pgib->pszName && strlen(pgib->pszName))
    printf("\nname=%s",pgib->pszName);
  else
    printf("\ndeal=%d",ideal);
  printf(" target=%d sol=%d mode=%d\n",target,sol,mode);
  fflush(stdout);

  timer.start();

  elapsed = totalelapsed = 0;
  nodes = totalnodes = 0;

  dealnodes = 0;
  dealelapsed = 0.0;

  memset(pgib->szTricks,'-',20);
  pgib->szTricks[20] = '\0';

  for(trickpos=0; trickpos<20; trickpos++)
  {
    if(trickpos < 4)
      trumps = 4;
    else
      trumps = (trickpos - 4) / 4;

    // set up dds10
    pgib->Trumps = trumps;
      pgib->Leader = 3 - (trickpos % 4);
    if(setDDS(pgib,&dl) == false)
      return false;

    sbcode = SolveBoard(dl,target,sol,mode,&fut,0);
    timer.check();
    if(testSBCode(sbcode) == false)
      return false;

    getSBScore(fut,&maxscore,bscore,m);
    if(maxscore < 0)
    { printf("*** error: leader=%c no score\n",
           chPLAYER[pgib->Leader]);
      return false;
    }
    score = maxscore;
    if(score >= 0)
      score = ((pgib->Leader & 1) ? score: pgib->numCard()/4-score);

    nodes = static_cast<unsigned long long>(fut.nodes);
    dealnodes += nodes;
    totalnodes += nodes;
    elapsed = timer.dblDeltaElapsed();
    dealelapsed += elapsed;
    totalelapsed += elapsed;

    printf("  leader=%c %d%c nodes=%s elapsed=%0.2f\n",
           chPLAYER[pgib->Leader],score,(trumps==4)?'N':chSUIT[trumps],
           format64(nodes,sz1),elapsed);

    if(score < 10)
      pgib->szTricks[trickpos] = static_cast<char>
        (score + static_cast<int>('0'));
    else
      pgib->szTricks[trickpos] = static_cast<char>
        (score - 10 + static_cast<int>('A'));
  }
  printf("nodes=%s elapsed=%0.2f totalelapsed=%0.2f\n",
         format64(dealnodes,sz1),dealelapsed,totalelapsed);

  printf("\n");
  pgib->printHands();
  printf("\n");
  if(pgib->pszName && strlen(pgib->pszName))
    printf("name=%s",pgib->pszName);
  else
    printf("deal=%d",ideal);
  printf("  tricks(leader=senw):");
  for(trickpos=0; trickpos<20; trickpos++)
  { if((trickpos%4) == 0)
    { if(trickpos == 0)
        printf(" n=");
      else if(trickpos == 4)
        printf(" s=");
      else if(trickpos == 8)
        printf(" h=");
      else if(trickpos == 12)
        printf(" d=");
      else if(trickpos == 16)
        printf(" c=");
    }
    printf("%c",pgib->szTricks[trickpos]);
  }
  printf("  (max=%d)\n\n",pgib->numCard()/4);

  return true;

} // tricks
// *****************************************************************************
/*
================================================================================
output for:   test.gib -timeall
================================================================================

test                     sol=1                 sol=2              sol=3
---------         ----------------     ----------------      ---------------
gib2                      13  0.02             17  0.02             17  0.02
gib3                      55  0.02             74  0.03             74  0.02
gib4                     144  0.02            239  0.02            290  0.02
gib5                     283  0.02            828  0.03          1,321  0.02
gib10                 24,616  0.05         56,172  0.09         73,522  0.10
gin                   17,372  0.05        330,860  0.37        836,869  0.96
dbldum1                1,806  0.03          3,052  0.03          3,084  0.03
dbldum2              272,365  0.25        767,339  0.71        767,339  0.69
devil1                94,935  0.12        179,691  0.21        336,969  0.34
callahan             502,044  0.60      1,303,934  1.99      1,679,319  2.33
g410-scc             121,233  0.15        140,942  0.20        464,298  0.45
giblibno1          4,026,572  5.01      6,887,233  9.50      8,470,777 11.83
giblibno2            469,241  0.48        586,377  0.61        586,377  0.60
bo17                 578,078  0.69      1,342,200  1.64      1,813,889  2.06
bo21               2,614,869  3.65      3,229,875  4.45      6,668,155  9.07
giblib1               70,093  0.10        158,829  0.20        158,829  0.18
giblib2            1,362,230  1.34      1,698,895  1.68      7,052,520  6.82
giblib3              178,884  0.18        477,891  0.38        477,899  0.39
giblib4              148,267  0.16        345,545  0.35      1,523,658  1.43
giblib5              997,945  1.00      3,119,281  3.24      3,119,281  3.24
giblib6            1,634,670  1.60      3,446,862  3.70      3,805,367  3.85
giblib7              116,215  0.16        575,832  0.67        606,341  0.64
giblib8              188,369  0.20      1,115,375  1.08      1,115,375  1.06
giblib9            1,287,045  1.33      2,542,975  2.88      4,461,078  5.17
giblib10             141,894  0.41        440,351  0.49        872,148  0.91

  nodes: total=88,494,703  sol-1=14,849,238  sol-2=28,750,669  sol-3=44,894,796
elapsed: total=104.43  sol-1=17.65  sol-2=34.56  sol-3=52.23

================================================================================
output for:   ../giblib -sol=1 -v -timeg=9n10
================================================================================

timeg tricks=9 contract=n deals=10 target=-1 sol=1 mode=1 leader=W
  deal=4 ndeal=1 nodes=148,267 elapsed=0.17
  deal=5 ndeal=2 nodes=997,945 elapsed=1.02
  deal=8 ndeal=3 nodes=188,369 elapsed=0.20
  deal=56 ndeal=4 nodes=2,496,093 elapsed=2.88
  deal=68 ndeal=5 nodes=1,942,886 elapsed=2.07
  deal=81 ndeal=6 nodes=595,946 elapsed=0.57
  deal=94 ndeal=7 nodes=910,639 elapsed=1.07
  deal=99 ndeal=8 nodes=2,916,346 elapsed=3.54
  deal=103 ndeal=9 nodes=247,399 elapsed=0.26
  deal=116 ndeal=10 nodes=353,807 elapsed=0.38
deals=10 nodes=10,797,697 elapsed=12.16
min_node=148,267 max_node=2,916,346 avg=1,079,769
min_elapsed=0.17 max_elapsed=3.54 avg=1.22
errors=0

================================================================================
output for:   ../giblib -giblib=1-1
================================================================================

giblib=1-1 target=-1 sol=1 mode=1
    deal=1 leader=S 8N score=8 nodes=76,507 elapsed=0.10
    deal=1 leader=E 8N score=8 nodes=73,856 elapsed=0.10
    deal=1 leader=N 8N score=8 nodes=66,062 elapsed=0.09
    deal=1 leader=W 8N score=8 nodes=70,093 elapsed=0.10
    deal=1 leader=S 7S score=7 nodes=633,804 elapsed=0.77
    deal=1 leader=E 7S score=7 nodes=227,684 elapsed=0.26
    deal=1 leader=N 7S score=7 nodes=617,829 elapsed=0.78
    deal=1 leader=W 7S score=7 nodes=239,070 elapsed=0.28
    deal=1 leader=S 10H score=10 nodes=44,135 elapsed=0.07
    deal=1 leader=E 9H score=9 nodes=64,593 elapsed=0.10
    deal=1 leader=N 10H score=10 nodes=25,802 elapsed=0.05
    deal=1 leader=W 9H score=9 nodes=66,538 elapsed=0.10
    deal=1 leader=S 7D score=7 nodes=710,876 elapsed=0.95
    deal=1 leader=E 7D score=7 nodes=129,917 elapsed=0.17
    deal=1 leader=N 7D score=7 nodes=740,822 elapsed=1.00
    deal=1 leader=W 7D score=7 nodes=156,316 elapsed=0.20
    deal=1 leader=S 8C score=8 nodes=287,964 elapsed=0.31
    deal=1 leader=E 8C score=8 nodes=234,815 elapsed=0.26
    deal=1 leader=N 8C score=8 nodes=367,526 elapsed=0.39
    deal=1 leader=W 8C score=8 nodes=128,382 elapsed=0.16
  deal=1 nodes=4,962,591 elapsed=6.25 totalelapsed=6.25
       tricks=88887777A9A977778888 (max=13)
deals=1-1 nodes=4,962,591 avg=248,129 elapsed=6.25 avg=0.31

================================================================================
output for:   test.gib -tricks -name=gin
================================================================================

name=gin target=-1 sol=1 mode=1
  leader=S 9N nodes=252,521 elapsed=0.32
  leader=E 9N nodes=538,589 elapsed=0.59
  leader=N 9N nodes=276,076 elapsed=0.35
  leader=W 8N nodes=17,372 elapsed=0.05
  leader=S 9S nodes=1,268,000 elapsed=2.08
  leader=E 8S nodes=1,726,949 elapsed=2.50
  leader=N 9S nodes=695,799 elapsed=0.96
  leader=W 8S nodes=1,656,521 elapsed=2.83
  leader=S 9H nodes=476,674 elapsed=0.59
  leader=E 9H nodes=531,859 elapsed=0.61
  leader=N 9H nodes=437,013 elapsed=0.53
  leader=W 9H nodes=298,225 elapsed=0.33
  leader=S 6D nodes=474,565 elapsed=0.57
  leader=E 6D nodes=238,345 elapsed=0.29
  leader=N 6D nodes=370,831 elapsed=0.44
  leader=W 6D nodes=60,061 elapsed=0.09
  leader=S 7C nodes=1,118,923 elapsed=1.46
  leader=E 7C nodes=1,074,097 elapsed=1.27
  leader=N 7C nodes=1,267,293 elapsed=1.70
  leader=W 7C nodes=587,523 elapsed=0.62
nodes=13,367,236 elapsed=18.18 totalelapsed=18.18

               K Q 9
               A Q J
               9 6 4 3 2
               8 6
  T 6                     8 7 3 2
  T 9 2                   7 5 3
  T                       A K Q J 8 5
  A J T 9 5 3 2           -
               A J 5 4
               K 8 6 4
               7
               K Q 7 4

name=gin  tricks(leader=senw): n=9998 s=9898 h=9999 d=6666 c=7777  (max=13)

dds/jack database format:
-------------------------
deal: A873.QJ8.T832.42 T52.965.J76.JT86 KJ96.K7.AQ.A9753 Q4.AT432.K954.KQ

sol=3
contract:hearts  play:W
      6: D T
      7: S A873  H QJ8  D 832  C 42
contract:hearts  play:N
      5: D J
      6: S T52  H 965  D 76  C JT86
contract:hearts  play:E
      7: S KJ96  H K7  D AQ  C A9753
contract:hearts  play:S
      5: D K954
      6: S Q4  H AT432  C KQ

db row for sol=3:
A873.QJ8.T832.42 T52.965.J76.JT86 KJ96.K7.AQ.A9753 Q4.AT432.K954.KQ:H:
7777777677777666666566666677777777777776666666555566

================================================================================
dds10x output for:   test.gib -timeall
================================================================================

test                     sol=1                 sol=2              sol=3
---------         ----------------     ----------------      ---------------
gib2                      13  0.02             17  0.02             17  0.02
gib3                      55  0.02             74  0.02             74  0.02
gib4                     132  0.02            220  0.03            267  0.02
gib5                     232  0.02            677  0.02          1,091  0.03
gib10                 23,990  0.05         54,710  0.09         71,783  0.10
gin                   12,536  0.05        301,204  0.36        712,655  0.82
dbldum1                1,320  0.03          2,045  0.02          2,070  0.03
dbldum2              271,126  0.26        761,667  0.75        761,667  0.72
devil1                80,278  0.11        145,941  0.17        279,078  0.30
callahan             373,482  0.47        987,935  1.54      1,273,233  1.87
g410-scc              82,136  0.12         98,589  0.13        324,586  0.41
giblibno1          2,905,488  3.92      5,015,652  7.65      6,153,242  9.48
giblibno2            339,994  0.38        440,591  0.49        440,591  0.49
bo17                 448,820  0.56        985,737  1.23      1,378,182  1.62
bo21               2,103,531  3.08      2,622,826  3.82      5,511,592  7.83
giblib1               39,201  0.08         97,312  0.13         97,312  0.14
giblib2              838,043  0.95      1,081,582  1.19      5,357,787  5.55
giblib3              118,210  0.12        310,481  0.28        310,487  0.28
giblib4              113,139  0.14        210,143  0.23      1,155,003  1.14
giblib5              720,826  0.77      2,301,738  2.66      2,301,738  2.60
giblib6              813,750  0.90      1,629,511  1.92      1,837,380  2.04
giblib7               89,093  0.12        443,105  0.51        469,467  0.54
giblib8              137,034  0.20        751,701  0.81        751,701  0.78
giblib9              936,282  1.05      1,734,880  2.02      3,170,680  3.77
giblib10             117,587  0.15        371,259  0.44        710,326  0.79
  nodes: total=63,987,904  sol-1=10,566,298  sol-2=20,349,597  sol-3=33,072,009
elapsed: total=81.50  sol-1=13.60  sol-2=26.52  sol-3=41.38

================================================================================
dds10x output for:   ../giblib -sol=1 -v -timeg=9n10
================================================================================

timeg tricks=9 contract=n deals=10 target=-1 sol=1 mode=1 leader=W
  deal=4 ndeal=1 nodes=113,139 elapsed=0.14
  deal=5 ndeal=2 nodes=720,826 elapsed=0.79
  deal=8 ndeal=3 nodes=137,034 elapsed=0.17
  deal=56 ndeal=4 nodes=1,837,912 elapsed=2.17
  deal=68 ndeal=5 nodes=1,224,530 elapsed=1.37
  deal=81 ndeal=6 nodes=410,308 elapsed=0.42
  deal=94 ndeal=7 nodes=593,127 elapsed=0.73
  deal=99 ndeal=8 nodes=1,946,230 elapsed=2.37
  deal=103 ndeal=9 nodes=155,728 elapsed=0.18
  deal=116 ndeal=10 nodes=174,796 elapsed=0.22
deals=10 nodes=7,313,630 elapsed=8.57
min_node=113,139 max_node=1,946,230 avg=731,363
min_elapsed=0.14 max_elapsed=2.37 avg=0.86
errors=0

================================================================================
dds10x output for:   ../giblib -giblib=1-1
================================================================================

giblib=1-1 target=-1 sol=1 mode=1
    deal=1 leader=S 8N score=8 nodes=50,178 elapsed=0.08
    deal=1 leader=E 8N score=8 nodes=39,811 elapsed=0.08
    deal=1 leader=N 8N score=8 nodes=43,703 elapsed=0.08
    deal=1 leader=W 8N score=8 nodes=39,201 elapsed=0.07
    deal=1 leader=S 7S score=7 nodes=420,588 elapsed=0.56
    deal=1 leader=E 7S score=7 nodes=155,765 elapsed=0.23
    deal=1 leader=N 7S score=7 nodes=426,794 elapsed=0.60
    deal=1 leader=W 7S score=7 nodes=164,092 elapsed=0.23
    deal=1 leader=S 10H score=10 nodes=33,596 elapsed=0.07
    deal=1 leader=E 9H score=9 nodes=45,895 elapsed=0.09
    deal=1 leader=N 10H score=10 nodes=19,817 elapsed=0.05
    deal=1 leader=W 9H score=9 nodes=47,289 elapsed=0.09
    deal=1 leader=S 7D score=7 nodes=454,538 elapsed=0.63
    deal=1 leader=E 7D score=7 nodes=80,961 elapsed=0.13
    deal=1 leader=N 7D score=7 nodes=492,525 elapsed=0.69
    deal=1 leader=W 7D score=7 nodes=90,791 elapsed=0.14
    deal=1 leader=S 8C score=8 nodes=169,389 elapsed=0.21
    deal=1 leader=E 8C score=8 nodes=144,696 elapsed=0.19
    deal=1 leader=N 8C score=8 nodes=239,940 elapsed=0.29
    deal=1 leader=W 8C score=8 nodes=86,347 elapsed=0.13
  deal=1 nodes=3,245,916 elapsed=4.63 totalelapsed=4.63
       tricks=88887777A9A977778888 (max=13)
deals=1-1 nodes=3,245,916 avg=162,295 elapsed=4.63 avg=0.23

================================================================================
dds105 output for:   test.gib -timeall
================================================================================

test                     sol=1                 sol=2              sol=3
---------         ----------------     ----------------      ---------------
gib2                      13  0.02             17  0.02             17  0.02
gib3                      51  0.03             70  0.02             70  0.02
gib4                     108  0.02            196  0.02            241  0.02
gib5                     201  0.02            615  0.02          1,029  0.03
gib10                 23,631  0.05         53,970  0.08         70,393  0.09
gin                   11,804  0.05        264,863  0.32        639,086  0.77
dbldum1                1,190  0.02          1,756  0.02          1,781  0.03
dbldum2              244,373  0.24        691,508  0.68        691,508  0.68
devil1                77,028  0.11        140,404  0.17        269,045  0.29
callahan             342,844  0.44        918,162  1.45      1,182,747  1.72
g410-scc              77,328  0.12         92,379  0.14        316,332  0.36
giblibno1          2,211,230  3.19      3,884,687  6.13      4,797,243  7.54
giblibno2            307,453  0.36        401,005  0.47        401,005  0.47
bo17                 344,725  0.44        826,074  1.08      1,184,613  1.42
bo21               1,793,723  2.60      2,249,871  3.23      4,791,239  6.78
giblib1               37,717  0.07         90,410  0.13         90,410  0.13
giblib2              780,451  0.88      1,009,229  1.15      5,027,108  5.38
giblib3              111,864  0.12        294,222  0.28        294,228  0.28
giblib4              102,296  0.13        191,583  0.22      1,086,325  1.13
giblib5              671,160  0.73      2,180,365  2.49      2,180,365  2.50
giblib6              763,607  0.85      1,531,703  1.79      1,722,183  1.98
giblib7               71,888  0.10        375,551  0.47        399,876  0.49
giblib8              120,940  0.15        689,874  0.73        689,874  0.73
giblib9              802,763  0.93      1,507,576  1.80      2,802,602  3.46
giblib10              88,779  0.13        272,940  0.34        586,850  0.69
  nodes: total=55,882,367  sol-1=8,987,167  sol-2=17,669,030  sol-3=29,226,170
elapsed: total=72.11  sol-1=11.83  sol-2=23.27  sol-3=37.01

================================================================================
dds105 output for:   ../giblib -sol=1 -v -timeg=9n10
================================================================================

timeg tricks=9 contract=n deals=10 target=-1 sol=1 mode=1 leader=W
  deal=4 ndeal=1 nodes=102,296 elapsed=0.14
  deal=5 ndeal=2 nodes=671,160 elapsed=0.74
  deal=8 ndeal=3 nodes=120,940 elapsed=0.17
  deal=56 ndeal=4 nodes=1,700,168 elapsed=2.07
  deal=68 ndeal=5 nodes=1,070,739 elapsed=1.26
  deal=81 ndeal=6 nodes=372,940 elapsed=0.40
  deal=94 ndeal=7 nodes=490,804 elapsed=0.61
  deal=99 ndeal=8 nodes=1,819,501 elapsed=2.30
  deal=103 ndeal=9 nodes=103,882 elapsed=0.14
  deal=116 ndeal=10 nodes=150,449 elapsed=0.19
deals=10 nodes=6,602,879 elapsed=8.01
min_node=102,296 max_node=1,819,501 avg=660,287
min_elapsed=0.14 max_elapsed=2.30 avg=0.80
errors=0

================================================================================
dds105 output for:   ../giblib -giblib=1-1
================================================================================

giblib=1-1 target=-1 sol=1 mode=1
    deal=1 leader=S 8N score=8 nodes=44,008 elapsed=0.08
    deal=1 leader=E 8N score=8 nodes=38,642 elapsed=0.07
    deal=1 leader=N 8N score=8 nodes=36,150 elapsed=0.07
    deal=1 leader=W 8N score=8 nodes=37,717 elapsed=0.07
    deal=1 leader=S 7S score=7 nodes=342,376 elapsed=0.46
    deal=1 leader=E 7S score=7 nodes=134,659 elapsed=0.18
    deal=1 leader=N 7S score=7 nodes=358,216 elapsed=0.49
    deal=1 leader=W 7S score=7 nodes=143,287 elapsed=0.20
    deal=1 leader=S 10H score=10 nodes=26,798 elapsed=0.06
    deal=1 leader=E 9H score=9 nodes=36,768 elapsed=0.08
    deal=1 leader=N 10H score=10 nodes=16,269 elapsed=0.05
    deal=1 leader=W 9H score=9 nodes=37,968 elapsed=0.08
    deal=1 leader=S 7D score=7 nodes=417,861 elapsed=0.60
    deal=1 leader=E 7D score=7 nodes=77,732 elapsed=0.14
    deal=1 leader=N 7D score=7 nodes=448,376 elapsed=0.66
    deal=1 leader=W 7D score=7 nodes=86,576 elapsed=0.15
    deal=1 leader=S 8C score=8 nodes=147,551 elapsed=0.21
    deal=1 leader=E 8C score=8 nodes=132,378 elapsed=0.18
    deal=1 leader=N 8C score=8 nodes=212,582 elapsed=0.27
    deal=1 leader=W 8C score=8 nodes=78,989 elapsed=0.13
  deal=1 nodes=2,854,903 elapsed=4.23 totalelapsed=4.23
       tricks=88887777A9A977778888 (max=13)
deals=1-1 nodes=2,854,903 avg=142,745 elapsed=4.23 avg=0.21

================================================================================
dds105 output for:   test.gib -tricks -name=gin
================================================================================

name=gin target=-1 sol=1 mode=1
  leader=S 9N nodes=217,042 elapsed=0.30
  leader=E 9N nodes=490,389 elapsed=0.54
  leader=N 9N nodes=242,708 elapsed=0.32
  leader=W 8N nodes=12,536 elapsed=0.05
  leader=S 9S nodes=1,067,876 elapsed=1.72
  leader=E 8S nodes=1,423,650 elapsed=2.10
  leader=N 9S nodes=579,046 elapsed=0.82
  leader=W 8S nodes=1,413,760 elapsed=2.35
  leader=S 9H nodes=421,052 elapsed=0.54
  leader=E 9H nodes=462,996 elapsed=0.55
  leader=N 9H nodes=388,427 elapsed=0.50
  leader=W 9H nodes=263,960 elapsed=0.30
  leader=S 6D nodes=400,416 elapsed=0.51
  leader=E 6D nodes=192,319 elapsed=0.24
  leader=N 6D nodes=312,717 elapsed=0.40
  leader=W 6D nodes=47,573 elapsed=0.09
  leader=S 7C nodes=987,365 elapsed=1.38
  leader=E 7C nodes=946,439 elapsed=1.20
  leader=N 7C nodes=1,102,393 elapsed=1.59
  leader=W 7C nodes=487,664 elapsed=0.56
nodes=11,460,328 elapsed=16.06 totalelapsed=16.06
               K Q 9
               A Q J
               9 6 4 3 2
               8 6
  T 6                     8 7 3 2
  T 9 2                   7 5 3
  T                       A K Q J 8 5
  A J T 9 5 3 2           -
               A J 5 4
               K 8 6 4
               7
               K Q 7 4
name=gin  tricks(leader=senw): n=9998 s=9898 h=9999 d=6666 c=7777  (max=13)

================================================================================
dds105 output for:   test.gib -tricks -name=gin
================================================================================

name=gin target=-1 sol=1 mode=1
  leader=S 9N nodes=196,320 elapsed=0.27
  leader=E 9N nodes=422,811 elapsed=0.49
  leader=N 9N nodes=216,293 elapsed=0.29
  leader=W 8N nodes=11,804 elapsed=0.05
  leader=S 9S nodes=966,576 elapsed=1.68
  leader=E 8S nodes=1,247,854 elapsed=2.01
  leader=N 9S nodes=514,885 elapsed=0.76
  leader=W 8S nodes=1,225,697 elapsed=2.09
  leader=S 9H nodes=388,951 elapsed=0.52
  leader=E 9H nodes=428,708 elapsed=0.55
  leader=N 9H nodes=360,926 elapsed=0.48
  leader=W 9H nodes=238,759 elapsed=0.28
  leader=S 6D nodes=307,283 elapsed=0.41
  leader=E 6D nodes=164,629 elapsed=0.22
  leader=N 6D nodes=240,740 elapsed=0.32
  leader=W 6D nodes=43,630 elapsed=0.08
  leader=S 7C nodes=847,071 elapsed=1.21
  leader=E 7C nodes=831,338 elapsed=1.09
  leader=N 7C nodes=966,299 elapsed=1.45
  leader=W 7C nodes=430,252 elapsed=0.51
nodes=10,050,826 elapsed=14.77 totalelapsed=14.77
               K Q 9
               A Q J
               9 6 4 3 2
               8 6
  T 6                     8 7 3 2
  T 9 2                   7 5 3
  T                       A K Q J 8 5
  A J T 9 5 3 2           -
               A J 5 4
               K 8 6 4
               7
               K Q 7 4
name=gin  tricks(leader=senw): n=9998 s=9898 h=9999 d=6666 c=7777  (max=13)
================================================================================
dds106 test.gib -timeal
 ================================================================================
test                     sol=1                 sol=2              sol=3
---------         ----------------     ----------------      ---------------
gib2                       6  0.02             10  0.02             10  0.02
gib3                      29  0.02             45  0.02             45  0.02
gib4                      65  0.02            153  0.02            179  0.02
gib5                     150  0.02            518  0.02            857  0.02
gib10                 17,663  0.04         41,178  0.06         52,041  0.07
gin                    8,763  0.04        224,298  0.27        581,064  0.69
dbldum1                  988  0.02          1,514  0.02          1,539  0.02
dbldum2              136,602  0.15        385,323  0.41        385,323  0.42
devil1                74,661  0.10        135,218  0.15        234,862  0.24
callahan             261,312  0.33        747,439  1.13        938,208  1.33
g410-scc              45,331  0.08         57,262  0.09        237,488  0.27
giblibno1          1,744,338  2.56      3,109,979  5.08      3,888,830  6.30
giblibno2            203,773  0.25        270,424  0.31        270,424  0.32
bo17                 290,006  0.35        691,275  0.85        930,229  1.07
bo21               1,502,044  2.16      1,882,400  2.66      3,926,551  5.61
giblib1               34,024  0.06         75,758  0.10         75,758  0.10
giblib2              685,093  0.73        889,056  0.93      3,631,179  3.96
giblib3               39,792  0.06        113,138  0.13        113,144  0.13
giblib4               58,370  0.08        108,555  0.12        724,213  0.74
giblib5              565,021  0.60      1,738,081  2.07      1,738,081  2.08
giblib6              631,095  0.66      1,332,592  1.48      1,502,434  1.64
giblib7               49,494  0.07        261,284  0.32        282,134  0.34
giblib8              106,018  0.12        625,995  0.66        625,995  0.66
giblib9              534,453  0.58      1,145,748  1.36      2,246,335  2.71
giblib10              28,477  0.06         91,990  0.12        371,359  0.40
  nodes: total=43,705,083  sol-1=7,017,568  sol-2=13,929,233  sol-3=22,758,282
elapsed: total=56.73  sol-1=9.17  sol-2=18.39  sol-3=29.17
================================================================================
dds106 ../exe/ddd ../giblib -sol=1 -v -timeg=9n10
================================================================================
timeg tricks=9 contract=n deals=10 target=-1 sol=1 mode=1 leader=W
  deal=4 ndeal=1 nodes=58,370 elapsed=0.09
  deal=5 ndeal=2 nodes=565,021 elapsed=0.61
  deal=8 ndeal=3 nodes=106,018 elapsed=0.13
  deal=56 ndeal=4 nodes=1,436,228 elapsed=1.74
  deal=68 ndeal=5 nodes=853,171 elapsed=0.99
  deal=81 ndeal=6 nodes=341,423 elapsed=0.35
  deal=94 ndeal=7 nodes=450,116 elapsed=0.54
  deal=99 ndeal=8 nodes=1,483,787 elapsed=1.92
  deal=103 ndeal=9 nodes=92,027 elapsed=0.11
  deal=116 ndeal=10 nodes=129,085 elapsed=0.16
deals=10 nodes=5,515,246 elapsed=6.65
min_node=58,370 max_node=1,483,787 avg=551,524
min_elapsed=0.09 max_elapsed=1.92 avg=0.66
errors=0
================================================================================
dds106 ../exe/ddd ../giblib -giblib=1-1
================================================================================
giblib=1-1 target=-1 sol=1 mode=1
    deal=1 leader=S 8N score=8 nodes=32,808 elapsed=0.06
    deal=1 leader=E 8N score=8 nodes=32,398 elapsed=0.06
    deal=1 leader=N 8N score=8 nodes=24,961 elapsed=0.05
    deal=1 leader=W 8N score=8 nodes=34,024 elapsed=0.06
    deal=1 leader=S 7S score=7 nodes=286,567 elapsed=0.35
    deal=1 leader=E 7S score=7 nodes=103,269 elapsed=0.13
    deal=1 leader=N 7S score=7 nodes=298,877 elapsed=0.38
    deal=1 leader=W 7S score=7 nodes=123,280 elapsed=0.16
    deal=1 leader=S 10H score=10 nodes=23,933 elapsed=0.05
    deal=1 leader=E 9H score=9 nodes=34,552 elapsed=0.06
    deal=1 leader=N 10H score=10 nodes=14,916 elapsed=0.04
    deal=1 leader=W 9H score=9 nodes=32,641 elapsed=0.06
    deal=1 leader=S 7D score=7 nodes=348,309 elapsed=0.47
    deal=1 leader=E 7D score=7 nodes=70,070 elapsed=0.10
    deal=1 leader=N 7D score=7 nodes=372,832 elapsed=0.50
    deal=1 leader=W 7D score=7 nodes=76,573 elapsed=0.11
    deal=1 leader=S 8C score=8 nodes=124,102 elapsed=0.15
    deal=1 leader=E 8C score=8 nodes=108,527 elapsed=0.13
    deal=1 leader=N 8C score=8 nodes=169,251 elapsed=0.20
    deal=1 leader=W 8C score=8 nodes=67,632 elapsed=0.10
  deal=1 nodes=2,379,522 elapsed=3.22 totalelapsed=3.22
       tricks=88887777A9A977778888 (max=13)
deals=1-1 nodes=2,379,522 avg=118,976 elapsed=3.22 avg=0.16
================================================================================
dds106 ../exe/ddd test.gib -tricks -name=gin
================================================================================
name=gin target=-1 sol=1 mode=1
  leader=S 9N nodes=189,465 elapsed=0.25
  leader=E 9N nodes=367,149 elapsed=0.43
  leader=N 9N nodes=208,008 elapsed=0.27
  leader=W 8N nodes=8,763 elapsed=0.04
  leader=S 9S nodes=804,494 elapsed=1.43
  leader=E 8S nodes=1,078,255 elapsed=1.69
  leader=N 9S nodes=436,355 elapsed=0.64
  leader=W 8S nodes=1,058,980 elapsed=1.86
  leader=S 9H nodes=320,228 elapsed=0.42
  leader=E 9H nodes=336,922 elapsed=0.41
  leader=N 9H nodes=293,834 elapsed=0.38
  leader=W 9H nodes=199,614 elapsed=0.24
  leader=S 6D nodes=259,761 elapsed=0.33
  leader=E 6D nodes=143,529 elapsed=0.19
  leader=N 6D nodes=213,182 elapsed=0.27
  leader=W 6D nodes=40,411 elapsed=0.07
  leader=S 7C nodes=591,728 elapsed=0.75
  leader=E 7C nodes=565,863 elapsed=0.69
  leader=N 7C nodes=596,756 elapsed=0.77
  leader=W 7C nodes=256,286 elapsed=0.29
nodes=7,969,583 elapsed=11.38 totalelapsed=11.38
               K Q 9
               A Q J
               9 6 4 3 2
               8 6
  T 6                     8 7 3 2
  T 9 2                   7 5 3
  T                       A K Q J 8 5
  A J T 9 5 3 2           -
               A J 5 4
               K 8 6 4
               7
               K Q 7 4
name=gin  tricks(leader=senw): n=9998 s=9898 h=9999 d=6666 c=7777  (max=13)
================================================================================
dds110 ./ddd test.gib -timeall
================================================================================
test                     sol=1                 sol=2              sol=3
---------         ----------------     ----------------      ---------------
gib2                       6  0.02             10  0.02             10  0.02
gib3                      29  0.02             45  0.02             45  0.02
gib4                      65  0.02            153  0.02            179  0.02
gib5                     150  0.02            518  0.02            857  0.02
gib10                 17,639  0.04         41,129  0.06         51,992  0.07
gin                    8,763  0.04        224,637  0.25        582,238  0.64
dbldum1                  988  0.02          1,514  0.02          1,539  0.02
dbldum2              137,710  0.14        387,069  0.35        387,069  0.35
devil1                74,416  0.09        135,018  0.15        234,410  0.24
callahan             258,062  0.28        747,584  0.80        938,511  0.98
g410-scc              45,860  0.08         57,788  0.09        240,189  0.27
giblibno1          1,747,984  1.83      3,116,388  3.34      3,897,902  4.19
giblibno2            203,086  0.22        269,483  0.29        269,483  0.29
bo17                 291,051  0.32        694,139  0.73        931,866  0.95
bo21               1,460,883  1.54      1,838,366  1.93      3,875,408  4.04
giblib1               34,009  0.06         75,902  0.10         75,902  0.10
giblib2              686,922  0.66        891,102  0.85      3,662,838  3.45
giblib3               39,792  0.06        113,138  0.12        113,144  0.12
giblib4               58,380  0.08        109,081  0.13        724,372  0.70
giblib5              558,633  0.56      1,741,928  1.73      1,741,928  1.73
giblib6              630,322  0.63      1,336,872  1.35      1,505,222  1.50
giblib7               49,621  0.07        261,723  0.28        282,573  0.30
giblib8              105,897  0.12        630,239  0.64        630,239  0.64
giblib9              526,845  0.53      1,146,045  1.16      2,250,098  2.30
giblib10              28,477  0.06         92,836  0.13        372,485  0.41
  nodes: total=43,648,796  sol-1=6,965,590  sol-2=13,912,707  sol-3=22,770,499
elapsed: total=45.42  sol-1=7.49  sol-2=14.56  sol-3=23.37
================================================================================
dds110 ./ddd ../giblib -sol=1 -v -timeg=9n10
================================================================================
timeg tricks=9 contract=n deals=10 target=-1 sol=1 mode=1 leader=W
  deal=4 ndeal=1 nodes=58,380 elapsed=0.09
  deal=5 ndeal=2 nodes=558,633 elapsed=0.56
  deal=8 ndeal=3 nodes=105,897 elapsed=0.13
  deal=56 ndeal=4 nodes=1,431,474 elapsed=1.46
  deal=68 ndeal=5 nodes=851,777 elapsed=0.88
  deal=81 ndeal=6 nodes=341,552 elapsed=0.33
  deal=94 ndeal=7 nodes=449,189 elapsed=0.46
  deal=99 ndeal=8 nodes=1,495,869 elapsed=1.53
  deal=103 ndeal=9 nodes=92,254 elapsed=0.11
  deal=116 ndeal=10 nodes=129,354 elapsed=0.16
deals=10 nodes=5,514,379 elapsed=5.71
min_node=58,380 max_node=1,495,869 avg=551,437
min_elapsed=0.09 max_elapsed=1.53 avg=0.57
errors=0
================================================================================
dds110 ./ddd ../giblib -giblib=1-1
giblib=1-1 target=-1 sol=1 mode=1
    deal=1 leader=S 8N score=8 nodes=32,808 elapsed=0.06
    deal=1 leader=E 8N score=8 nodes=32,441 elapsed=0.06
    deal=1 leader=N 8N score=8 nodes=25,021 elapsed=0.05
    deal=1 leader=W 8N score=8 nodes=34,009 elapsed=0.06
    deal=1 leader=S 7S score=7 nodes=279,639 elapsed=0.31
    deal=1 leader=E 7S score=7 nodes=102,254 elapsed=0.13
    deal=1 leader=N 7S score=7 nodes=301,914 elapsed=0.33
    deal=1 leader=W 7S score=7 nodes=121,529 elapsed=0.14
    deal=1 leader=S 10H score=10 nodes=24,004 elapsed=0.05
    deal=1 leader=E 9H score=9 nodes=34,337 elapsed=0.06
    deal=1 leader=N 10H score=10 nodes=14,921 elapsed=0.04
    deal=1 leader=W 9H score=9 nodes=32,523 elapsed=0.06
    deal=1 leader=S 7D score=7 nodes=347,634 elapsed=0.42
    deal=1 leader=E 7D score=7 nodes=69,322 elapsed=0.11
    deal=1 leader=N 7D score=7 nodes=371,443 elapsed=0.44
    deal=1 leader=W 7D score=7 nodes=76,652 elapsed=0.11
    deal=1 leader=S 8C score=8 nodes=129,621 elapsed=0.15
    deal=1 leader=E 8C score=8 nodes=110,722 elapsed=0.13
    deal=1 leader=N 8C score=8 nodes=172,313 elapsed=0.20
    deal=1 leader=W 8C score=8 nodes=64,853 elapsed=0.09
  deal=1 nodes=2,377,960 elapsed=2.99 totalelapsed=2.99
       tricks=88887777A9A977778888 (max=13)
deals=1-1 nodes=2,377,960 avg=118,898 elapsed=2.99 avg=0.15
================================================================================
dds110 ./ddd test.gib -tricks -name=gin
name=gin target=-1 sol=1 mode=1
  leader=S 9N nodes=192,174 elapsed=0.24
  leader=E 9N nodes=367,542 elapsed=0.40
  leader=N 9N nodes=207,824 elapsed=0.25
  leader=W 8N nodes=8,763 elapsed=0.04
  leader=S 9S nodes=785,551 elapsed=0.89
  leader=E 8S nodes=1,083,174 elapsed=1.24
  leader=N 9S nodes=437,999 elapsed=0.50
  leader=W 8S nodes=1,067,505 elapsed=1.18
  leader=S 9H nodes=321,295 elapsed=0.38
  leader=E 9H nodes=332,370 elapsed=0.39
  leader=N 9H nodes=295,031 elapsed=0.34
  leader=W 9H nodes=200,005 elapsed=0.22
  leader=S 6D nodes=260,627 elapsed=0.31
  leader=E 6D nodes=143,798 elapsed=0.19
  leader=N 6D nodes=214,479 elapsed=0.26
  leader=W 6D nodes=40,383 elapsed=0.07
  leader=S 7C nodes=596,131 elapsed=0.65
  leader=E 7C nodes=580,584 elapsed=0.64
  leader=N 7C nodes=596,086 elapsed=0.67
  leader=W 7C nodes=258,317 elapsed=0.28
nodes=7,989,638 elapsed=9.12 totalelapsed=9.12
               K Q 9
               A Q J
               9 6 4 3 2
               8 6
  T 6                     8 7 3 2
  T 9 2                   7 5 3
  T                       A K Q J 8 5
  A J T 9 5 3 2           -
               A J 5 4
               K 8 6 4
               7
               K Q 7 4
name=gin  tricks(leader=senw): n=9998 s=9898 h=9999 d=6666 c=7777  (max=13)
================================================================================
ddd112 ./ddd test.gib -timeall
================================================================================
test                     sol=1                 sol=2              sol=3
---------         ----------------     ----------------      ---------------
gib2                       6  0.02             10  0.02             10  0.02
gib3                      29  0.02             45  0.03             45  0.02
gib4                      65  0.02            153  0.02            177  0.02
gib5                     155  0.02            486  0.02            657  0.02
gib10                  9,258  0.03         19,016  0.04         22,652  0.04
gin                    6,324  0.03        219,444  0.26        475,279  0.56
dbldum1                  540  0.02          1,013  0.03          1,038  0.02
dbldum2              136,598  0.15        259,032  0.25        259,032  0.25
devil1                81,548  0.10        116,045  0.14        153,721  0.18
callahan             174,145  0.21        430,962  0.49        471,674  0.54
g410-scc              43,903  0.08         44,681  0.08        112,062  0.15
giblibno1          1,163,737  1.32      1,697,850  1.98      1,788,191  2.13
giblibno2            160,340  0.20        223,954  0.27        223,954  0.27
bo17                 157,301  0.19        372,446  0.43        438,133  0.49
bo21               1,795,469  2.03      1,959,809  2.25      2,407,449  2.85
giblib1               31,058  0.06         44,026  0.08         44,026  0.07
giblib2              255,255  0.28        418,134  0.53      1,931,126  2.06
giblib3               18,470  0.04         38,885  0.06         38,891  0.07
giblib4               48,509  0.07        143,359  0.17        401,817  0.45
giblib5              417,649  0.43      1,711,054  1.99      1,711,054  1.86
giblib6              560,081  0.60        937,852  1.02        996,590  1.10
giblib7               41,668  0.07        114,334  0.14        119,228  0.14
giblib8               74,780  0.10        264,695  0.29        264,695  0.29
giblib9              427,057  0.48        899,105  1.00      1,163,416  1.32
giblib10              30,338  0.06         96,455  0.14        270,423  0.33
  nodes: total=28,942,468  sol-1=5,634,283  sol-2=10,012,845  sol-3=13,295,340
elapsed: total=33.59  sol-1=6.62  sol-2=11.71  sol-3=15.26
================================================================================
ddd112 ./ddd ../giblib -sol=1 -v -timeg=9n10
================================================================================
Double Dummy Driver (DDD) version 1.05 (25-Jan-2007)
timeg tricks=9 contract=n deals=10 target=-1 sol=1 mode=1 leader=W
  deal=4 ndeal=1 nodes=48,509 elapsed=0.07
  deal=5 ndeal=2 nodes=417,649 elapsed=0.43
  deal=8 ndeal=3 nodes=74,780 elapsed=0.10
  deal=56 ndeal=4 nodes=1,241,660 elapsed=1.34
  deal=68 ndeal=5 nodes=319,869 elapsed=0.36
  deal=81 ndeal=6 nodes=158,661 elapsed=0.18
  deal=94 ndeal=7 nodes=666,682 elapsed=0.73
  deal=99 ndeal=8 nodes=1,607,014 elapsed=1.79
  deal=103 ndeal=9 nodes=51,985 elapsed=0.08
  deal=116 ndeal=10 nodes=128,401 elapsed=0.16
deals=10 nodes=4,715,210 elapsed=5.24
min_node=48,509 max_node=1,607,014 avg=471,521
min_elapsed=0.07 max_elapsed=1.79 avg=0.52
errors=0
================================================================================
ddd112 ./ddd ../giblib -giblib=1-1
================================================================================
Double Dummy Driver (DDD) version 1.05 (25-Jan-2007)
giblib=1-1 target=-1 sol=1 mode=1
    deal=1 leader=S 8N score=8 nodes=18,322 elapsed=0.04
    deal=1 leader=E 8N score=8 nodes=32,837 elapsed=0.06
    deal=1 leader=N 8N score=8 nodes=10,500 elapsed=0.04
    deal=1 leader=W 8N score=8 nodes=31,058 elapsed=0.06
    deal=1 leader=S 7S score=7 nodes=79,659 elapsed=0.11
    deal=1 leader=E 7S score=7 nodes=47,581 elapsed=0.07
    deal=1 leader=N 7S score=7 nodes=52,907 elapsed=0.08
    deal=1 leader=W 7S score=7 nodes=45,893 elapsed=0.07
    deal=1 leader=S 10H score=10 nodes=9,517 elapsed=0.03
    deal=1 leader=E 9H score=9 nodes=26,390 elapsed=0.05
    deal=1 leader=N 10H score=10 nodes=6,318 elapsed=0.03
    deal=1 leader=W 9H score=9 nodes=37,377 elapsed=0.07
    deal=1 leader=S 7D score=7 nodes=143,865 elapsed=0.19
    deal=1 leader=E 7D score=7 nodes=31,690 elapsed=0.06
    deal=1 leader=N 7D score=7 nodes=139,623 elapsed=0.19
    deal=1 leader=W 7D score=7 nodes=42,067 elapsed=0.07
    deal=1 leader=S 8C score=8 nodes=22,516 elapsed=0.05
    deal=1 leader=E 8C score=8 nodes=51,243 elapsed=0.08
    deal=1 leader=N 8C score=8 nodes=13,037 elapsed=0.04
    deal=1 leader=W 8C score=8 nodes=53,801 elapsed=0.08
  deal=1 nodes=896,201 elapsed=1.48 totalelapsed=1.48
       tricks=88887777A9A977778888 (max=13)
deals=1-1 nodes=896,201 avg=44,810 elapsed=1.48 avg=0.07
================================================================================
./ddd test.gib -tricks -name=gin
================================================================================
Double Dummy Driver (DDD) version 1.05 (25-Jan-2007)
name=gin target=-1 sol=1 mode=1
  leader=S 9N nodes=204,883 elapsed=0.25
  leader=E 9N nodes=341,850 elapsed=0.39
  leader=N 9N nodes=202,525 elapsed=0.26
  leader=W 8N nodes=6,324 elapsed=0.03
  leader=S 9S nodes=404,513 elapsed=0.51
  leader=E 8S nodes=880,321 elapsed=1.09
  leader=N 9S nodes=481,945 elapsed=0.62
  leader=W 8S nodes=679,639 elapsed=0.83
  leader=S 9H nodes=317,100 elapsed=0.40
  leader=E 9H nodes=230,488 elapsed=0.32
  leader=N 9H nodes=341,234 elapsed=0.45
  leader=W 9H nodes=53,051 elapsed=0.08
  leader=S 6D nodes=420,142 elapsed=0.52
  leader=E 6D nodes=111,851 elapsed=0.15
  leader=N 6D nodes=448,369 elapsed=0.56
  leader=W 6D nodes=37,943 elapsed=0.07
  leader=S 7C nodes=460,906 elapsed=0.55
  leader=E 7C nodes=276,054 elapsed=0.34
  leader=N 7C nodes=515,007 elapsed=0.63
  leader=W 7C nodes=297,298 elapsed=0.33
nodes=6,711,443 elapsed=8.38 totalelapsed=8.38
               K Q 9
               A Q J
               9 6 4 3 2
               8 6
  T 6                     8 7 3 2
  T 9 2                   7 5 3
  T                       A K Q J 8 5
  A J T 9 5 3 2           -
               A J 5 4
               K 8 6 4
               7
               K Q 7 4
name=gin  tricks(leader=senw): n=9998 s=9898 h=9999 d=6666 c=7777  (max=13)
*/
