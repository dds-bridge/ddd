// =============================================================================
/* rng.cpp  RNG - random number generators

            PMC 14-jun-2005
            PMC 06-jul-2005
            PMC 13-jul-2006

   Copyright 2005-2006 P.M.Cronje

   RNG is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   RNG is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with RNG; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
// =============================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "portab_DDD.h"
#include "rng.h"

const char *pszRNGGen[eRNG_COUNT]
   = {
       "qd1",
       "mt",
       "mthr",
       "well"
     };

const char *pszRNGGenList = "qd1/mt/mthr/well";
const char *pszRNGGenDefault = "mt";

double dRAN_SCALE = 1.0 / (1.0+double(static_cast<unsigned int>(0xFFFFFFFF)));

unsigned int qd1(unsigned int *puseed);
void reset1Bit(unsigned int u1bit, unsigned int *pu);
unsigned int count1Bits(unsigned int useed);
unsigned int qd1Uint(unsigned int *puseed, unsigned int urange);
unsigned int equiDistribute1Bits(unsigned int u, unsigned int *puseed);
void set0Bit(unsigned int u0bit, unsigned int *pu);
double gammln(float xx);
void gcf(double *gammcf, double aa, double x, double *gln);
void gser(double *gamser, double aa, double x, double *gln);

// *****************************************************************************
// cRNG
// *****************************************************************************

unsigned int cRNG::randomUint(unsigned int urange)
{
  return static_cast<unsigned int>(static_cast<double>(urange) * 
    static_cast<double>(random()) * dRAN_SCALE);

} // cRNG::randomUint
// *****************************************************************************

cRNG *cRNG::createRNG(eRNG erng, unsigned int useed)
{
  cRNG *prng;
  int rng;

  rng = static_cast<int>(erng);
  if(rng == eRNG_QD1)
    prng = new cRNG_QD1(useed);
  else if(rng == eRNG_MTHR)
    prng = new cRNG_Mother(useed);
  else if (rng == eRNG_WELL)
    prng = new cRNG_WELL(useed);
  else // if(rng == eRNG_MT)                     // default
    prng = new cRNG_MT19937(useed);

  return prng;

} // cRNG::createRNG(erng)
// *****************************************************************************

cRNG *cRNG::createRNG(char *pszrng, unsigned int useed)
{
  cRNG *prng;

  if(0 == strcasecmp(pszrng,"qd1"))
    prng = new cRNG_QD1(useed);
  else if(0 == strcasecmp(pszrng,"mthr"))
    prng = new cRNG_Mother(useed);
  else if (0 == strcasecmp(pszrng,"well"))
    prng = new cRNG_WELL(useed);
  else // if(0 == strcasecmp(pszrng,"mt"))       // default
    prng = new cRNG_MT19937(useed);

  return prng;


} // cRNG::createRNG(pszgen)
// *****************************************************************************
// ranQD1
// *****************************************************************************

void cRNG_QD1::set(unsigned int useed)
{
  uSeed = useed;
  RNG = eRNG_QD1;
  strcpy(szGen,pszRNGGen[RNG]);
}

unsigned int cRNG_QD1::random()
{
  uSeed = (static_cast<unsigned int>(1664525L) * uSeed + 
    static_cast<unsigned int>(1013904223L));
  return uSeed;

} // cRNG_QD1::random
/* ************************************************************************** */
// equiDistribute1Bits
/* ************************************************************************** */

unsigned int qd1(unsigned int *puseed)
{
  *puseed = (static_cast<unsigned int>(1664525L) * (*puseed) + 
    static_cast<unsigned int>(1013904223L));
  return *puseed;

} // qd1

unsigned int qd1Uint(unsigned int *puseed, unsigned int urange)
{
  *puseed = (static_cast<unsigned int>(1664525L) * (*puseed) + 
    static_cast<unsigned int>(1013904223L));
  return static_cast<unsigned int>(static_cast<double>(urange) * 
    static_cast<double>(*puseed) * dRAN_SCALE);

} // qd1Uint

unsigned int count1Bits(unsigned int useed)
{
  unsigned int mask, n1, i;

  // count the number of 1-bits
  mask = 0x00000001;
  n1 = 0;
  for(i=0; i<32; i++)
  { if(mask & useed)
      n1++;
    mask = (mask << 1);
  }

  return n1;

} // count1Bits

void reset1Bit(unsigned int u1bit, unsigned int *pu)
{
  unsigned int mask, n, i;

  // find and zero the bit
  mask = 0x00000001;
  n = 0;
  for(i=0; i<32; i++)
  { if(mask & (*pu))
    { if(n == u1bit)
      { *pu = ((*pu) & (~mask));
        return;
      }
      n++;
    }
    mask = (mask << 1);
  }

} // reset1Bit

void set0Bit(unsigned int u0bit, unsigned int *pu)
{
  unsigned int mask, n, i;

  // find and set the bit
  mask = 0x00000001;
  n = 0;
  for(i=0; i<32; i++)
  { if(0 == (mask & (*pu)))
    { if(n == u0bit)
      { *pu = ((*pu) | mask);
        return;
      }
      n++;
    }
    mask = (mask << 1);
  }

} // set0Bit


unsigned int equiDistribute1Bits(unsigned int u, unsigned int *puseed)
{
  // if number of 1-bits in useed not 16,
  //   if excess of 1-bits, randomly reset them to 0-bits
  //   if excess of 0-bits, randomly set them to 1-bits

  unsigned int n1, i,ueq, ubit, n0;

  // count the number of 1-bits
  n1 = count1Bits(u);

  ueq = u;

  if(n1 > 16)
  { for(i=n1; i>16; i--)
    { ubit = qd1Uint(puseed,i);
      reset1Bit(ubit,&ueq);
    }
  }
  else if(n1 < 16)
  { n0 = 32 - n1;
    for(i=n0; i>16; i--)
    { ubit = qd1Uint(puseed,i);
      set0Bit(ubit,&ueq);
    }
  }

  return ueq;

} // equiDistribute1Bits
// *****************************************************************************
// WELL1024u
// *****************************************************************************

#define MAT0POS(t,v) (v^(v>>t))
#define MAT0NEG(t,v) (v^(v<<(-(t))))
#define Identity(v) (v)

#define W 32
#define R 32
#define M1 3
#define M2 24
#define M3 10

#define V0     STATE[ state_i                  ]
#define VM1    STATE[(state_i+M1) & 0x0000001fU]
#define VM2    STATE[(state_i+M2) & 0x0000001fU]
#define VM3    STATE[(state_i+M3) & 0x0000001fU]
#define VRm1   STATE[(state_i+31) & 0x0000001fU]
#define newV0  STATE[(state_i+31) & 0x0000001fU]
#define newV1  STATE[ state_i                  ]

unsigned int cRNG_WELL::random()
{
  z0      = VRm1;
  z1      = Identity(V0)       ^ MAT0POS(  8, VM1);
  z2      = MAT0NEG (-19, VM2) ^ MAT0NEG(-14, VM3);
  newV1   = z1                 ^ z2;
  newV0   =   MAT0NEG(-11,z0)
            ^ MAT0NEG( -7,z1)
            ^ MAT0NEG(-13,z2);

  state_i = (state_i + 31) & 0x0000001fU;

  return STATE[state_i];

} // cRNG_WELL::random

#undef W
#undef R
#undef M1
#undef M2
#undef M3

#undef V0
#undef VM1
#undef VM2
#undef VM3
#undef VRm1
#undef newV0
#undef newV1

void cRNG_WELL::set(unsigned int useed)
{
  int j;
  unsigned int u, uqd1seed;

  RNG = eRNG_WELL;
  strcpy(szGen,pszRNGGen[RNG]);

  /* initialize
     using specified seed, set state mixing 0 and 1 bits
  */
  uSeed = useed;
  state_i = 0;
  u = uqd1seed = useed;
  for (j = 0; j < 32; j++)
  { STATE[j] = equiDistribute1Bits(u,&useed);
    u = qd1(&useed);
  }

  /* run the generator for a while to escape from the seeded state */
  for(j=0; j<50000; j++)
    random();

} // cRNG_WELL::set
// *****************************************************************************
// Mother
// *****************************************************************************

void cRNG_Mother::set(unsigned int useed)
{
  RNG = eRNG_MTHR;
  strcpy(szGen,pszRNGGen[RNG]);
  uSeed = useed;

  smthr[0] = 5115;
  smthr[1] = 1776;
  smthr[2] = 1492;
  smthr[3] = 2111111111;

  xm1    = static_cast<uint64>(smthr[0]);
  xm2    = static_cast<uint64>(smthr[1]);
  xm3    = static_cast<uint64>(smthr[2]);
  xm4    = static_cast<uint64>(smthr[3]);

  unsigned int sum = static_cast<unsigned>(xm1 + xm2 + xm3 + xm4);
  cRNG_WELL rng(uSeed);
  mcarry = static_cast<uint64>(rng.randomUint(sum));

} // cRNG_Mother::set

unsigned int cRNG_Mother::random()
{
  static uint64 am1 = static_cast<uint64>(2111111111);
  static uint64 am2 = static_cast<uint64>(1492);
  static uint64 am3 = static_cast<uint64>(1776);
  static uint64 am4 = static_cast<uint64>(5115);

  uint64 x = am1 * xm1
           + am2 * xm2
           + am3 * xm3
           + am4 * xm4
           + mcarry;

  xm1    = xm2;
  xm2    = xm3;
  xm3    = xm4;
  xm4    = (x & 0x00000000ffffffffULL);
  mcarry = (x >> 32);

  return static_cast<unsigned int>(xm4);

} // cRNG_Mother::random
// *****************************************************************************
// mt19937ar
// *****************************************************************************

// PMC20050619 - updated MT to mt19937ar

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

// static unsigned long mt[N]; /* the array for the state vector  */
// static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */

void cRNG_MT19937::initBySeed(unsigned int useed)
{
  U[0]= useed & 0xffffffffUL;
  for (mti=1; mti<N; mti++)
  {
    U[mti] = (1812433253UL * (U[mti-1] ^ (U[mti-1] >> 30)) + mti);
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array p->U[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
    U[mti] &= 0xffffffffUL;
    /* for >32 bit machines */
  }

} // cRNG_MT19937::initBySeed


/* initialize by an array with array-length */
/* p->Key is the array for initializing keys */
/* p->LenKey is its length */
/* slight change for C++, 2004/2/26 */

void cRNG_MT19937::initByArray(unsigned int lenkey, unsigned int key[])
{
  int i, j, k;

  initBySeed(19650218UL);

  i=1; j=0;
  k = (N>lenkey ? N : static_cast<int>(lenkey));
  for (; k; k--)
  {
    U[i] = (U[i] ^ ((U[i-1] ^ (U[i-1] >> 30)) * 1664525UL))
      + key[j] + static_cast<unsigned>(j); /* non linear */
    U[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
    i++; j++;
    if (i>=N) { U[0] = U[N-1]; i=1; }
    if (j>=static_cast<int>(lenkey)) j=0;
  }
  for (k=N-1; k; k--)
  {
    U[i] = (U[i] ^ ((U[i-1] ^ (U[i-1] >> 30)) * 1566083941UL))
      - static_cast<unsigned>(i); /* non linear */
    U[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
    i++;
    if (i>=N) { U[0] = U[N-1]; i=1; }
  }

  U[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */

} // initByArray

void cRNG_MT19937::set(unsigned int useed)
{
  unsigned int key[624];
  int i;

  uSeed = useed;
  RNG = eRNG_MT;
  strcpy(szGen,pszRNGGen[RNG]);
  cRNG_WELL rng(useed);

  mag01[0] = 0x0UL;
  mag01[1] = MATRIX_A;

  for(i=0; i<624; i++)
    key[i] = rng.random();
  initByArray(624,key);

} // cRNG_MT19937::set

/* generates a random number on [0,0xffffffff]-interval */

unsigned int cRNG_MT19937::random()
{
  unsigned long y;

  if (mti >= N)
  { /* generate N words at one time */

    int kk;

    if (mti == N+1)   /* if not initalized, */
      initBySeed(5489UL);      /* a default initial seed is used */

    for (kk=0;kk<N-M;kk++)
    { y = (U[kk]&UPPER_MASK)|(U[kk+1]&LOWER_MASK);
      U[kk] = U[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    for (;kk<N-1;kk++)
    { y = (U[kk]&UPPER_MASK)|(U[kk+1]&LOWER_MASK);
      U[kk] = U[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    y = (U[N-1]&UPPER_MASK)|(U[0]&LOWER_MASK);
    U[N-1] = U[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

    mti = 0;
  }

  y = U[mti++];

  /* Tempering */
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);

  return y;

} // cRNG_MT19937::random

#undef N
#undef M
#undef MATRIX_A
#undef UPPER_MASK
#undef LOWER_MASK
// *****************************************************************************
// Entropy test
// *****************************************************************************

double gammq(double a, double x);

#define PI 3.14159265358979323846
#define log2of10 3.32192809488736234787

/* Treat input as a bitstream */
static bool binary = false;

/* Bins to count occurrences of values */
static long ccount[256];

/* Total bytes counted */
static long totalc = 0;

/* Probabilities per bin for entropy */
static double prob[256];

/*  LOG2  --  Calculate log to the base 2  */
//static double log2(double x)
//{
//    return log2of10 * log10(x);
//}

/* Bytes used as Monte Carlo
   co-ordinates.  This should be no more
   bits than the mantissa of your
   "double" floating point type.
*/
#define MONTEN  6

static int mp;
static bool sccfirst;
static unsigned int monte[MONTEN];
static long inmont, mcount;
static double a, cexp, incirc, montex, montey, montepi,
        scc, sccun, sccu0, scclast, scct1, scct2, scct3,
        ent, chisq, datasum;

void initEnt(bool binmode)
{
  int i;

  /* Set binary/byte mode */
  binary = binmode;

  /* Initialise for calculations */

  ent = 0.0;             /* Clear entropy accumulator */
  chisq = 0.0;           /* Clear Chi-Square */
  datasum = 0.0;         /* Clear sum of bytes for arithmetic mean */

  mp = 0;                     /* Reset Monte Carlo accumulator pointer */
  mcount = 0;                 /* Clear Monte Carlo tries */
  inmont = 0;                 /* Clear Monte Carlo inside count */
  incirc = 65535.0 * 65535.0; /* In-circle distance for Monte Carlo */

  sccfirst = true;             /* Mark first time for serial correlation */
  scct1 = scct2 = scct3 = 0.0; /* Clear serial correlation terms */

  incirc = pow(pow(256.0, static_cast<double> (MONTEN / 2)) - 1, 2.0);

  for (i = 0; i < 256; i++)
    ccount[i] = 0;
  totalc = 0;

} // initEnt

void addEnt(unsigned char *buf, int buflen)
{
  unsigned char *bp = buf;
  int oc, c, bean;

  while (bean = 0, (buflen-- > 0))
  {
    oc = *bp++;

    do
    {
      if (binary)
        c = !!(oc & 0x80);
      else
        c = oc;
      ccount[c]++;      /* Update counter for this bin */
      totalc++;

      /* Update inside/outside circle counts for Monte Carlo computation of PI */

      if (bean == 0)
      {
        monte[mp++] = static_cast<unsigned>(oc);       /* Save character for Monte Carlo */
        if (mp >= MONTEN)
        {  /* Calculate every MONTEN character */
          int mj;

          mp = 0;
          mcount++;
          montex = montey = 0;
          for (mj = 0; mj < MONTEN / 2; mj++)
          { montex = (montex * 256.0) + monte[mj];
            montey = (montey * 256.0) + monte[(MONTEN / 2) + mj];
          }
          if ((montex * montex + montey *  montey) <= incirc)
            inmont++;
        }
      }

      /* Update calculation of serial correlation coefficient */

      sccun = c;
      if (sccfirst)
      { sccfirst = false;
        scclast = 0;
        sccu0 = sccun;
      }
      else
        scct1 = scct1 + scclast * sccun;
      scct2 = scct2 + sccun;
      scct3 = scct3 + (sccun * sccun);
      scclast = sccun;
      oc <<= 1;
    } while (binary && (++bean < 8));
  }

} // addEnt

void endEnt(double *r_ent, double *r_chisq, double *r_mean,
            double *r_montepicalc, double *r_scc)
{
  int i;

  /* Complete calculation of serial correlation coefficient */

  scct1 = scct1 + scclast * sccu0;
  scct2 = scct2 * scct2;
  scc = totalc * scct3 - scct2;
  if (scc == 0.0)
    scc = -100000;
  else
    scc = (totalc * scct1 - scct2) / scc;

  /* Scan bins and calculate probability for each bin and
     Chi-Square distribution */

  cexp = totalc / (binary ? 2.0 : 256.0);  /* Expected count per bin */
  for (i = 0; i < (binary ? 2 : 256); i++)
  { prob[i] = static_cast<double>(ccount[i]) / totalc;
    a = ccount[i] - cexp;
    chisq = chisq + (a * a) / cexp;
    datasum += (static_cast<double>(i)) * ccount[i];
  }

  /* Calculate entropy */

  for (i = 0; i < (binary ? 2 : 256); i++)
  { if (prob[i] > 0.0)
      ent += prob[i] * log2(1 / prob[i]);
  }

  /* Calculate Monte Carlo value for PI from percentage of hits
     within the circle
  */

  montepi = 4.0 * ((static_cast<double>(inmont)) / mcount);

  /* Return results through arguments */

  *r_ent = ent;
  *r_chisq = chisq;
  *r_mean = datasum / totalc;
  *r_montepicalc = montepi;
  *r_scc = scc;

} // endEnt

void prtEnt(double r_chisq, double r_mean,
            double r_montepicalc, double r_scc)
{
  // probability that observed chi^2 will exceed the value chi^2
  // by chance EVEN for a correct model:
  //     Q(chi^2,nu) = gammaq(nu/2,chi^2/2);
  //
  double probq = 100.0 * gammq(127.5,0.5*r_chisq);

  printf("  %ss,Entropy,Chi-square,Mean,Monte-Carlo-Pi,Serial-Correlation\n",
         binary ? "bit" : "byte");
  printf("  %ld,%f,%.1f(%.2f%%),%f,%f(%.2f%%),%f\n",
           //totalc,ent,r_chisq,100.0*chip,r_mean,
           totalc,ent,r_chisq,probq,r_mean,
           r_montepicalc,100.0*(r_montepicalc-PI),r_scc);

} // prtEnt
// *****************************************************************************


double gammln(float xx)
{
  double x, y, tmp, ser;
  static double cof[6]
    = {  76.18009172947146,
        -86.50532032941677,
         24.01409824083091,
         -1.231739572450155,
          0.1208650973866179e-2,
         -0.5395239384953e-5
      };
  int j;

  y=x=xx;
  tmp=x+5.5;
  tmp -= (x+0.5)*log(tmp);
  ser=1.000000000190015;
  for (j=0;j<=5;j++)
    ser += cof[j]/++y;
  return -tmp+log(2.5066282746310005*ser/x);

} // gammln
// *****************************************************************************
#define ITMAX 100
#define EPS 3.0e-7
#define FPMIN 1.0e-30


void gcf(double *gammcf, double aa, double x, double *gln)
{
  int i;
  double an, b, c, d, del, h;

  *gln=gammln(static_cast<float>(aa));
  b=x+1.0-aa;
  c=1.0/FPMIN;
  d=1.0/b;
  h=d;
  for (i=1;i<=ITMAX;i++)
  { an = -i*(i-aa);
    b += 2.0;
    d=an*d+b;
    if (fabs(d) < FPMIN)
      d=FPMIN;
    c=b+an/c;
    if (fabs(c) < FPMIN)
      c=FPMIN;
    d=1.0/d;
    del=d*c;
    h *= del;
    if (fabs(del-1.0) < EPS)
      break;
  }
  if (i > ITMAX)
  { printf("*** error %s: a too large, ITMAX too small\n","gcf");
    exit(1);
  }
  *gammcf=exp(-x+aa*log(x)-(*gln))*h;

} // gcf

#undef ITMAX
#undef EPS
#undef FPMIN
// *****************************************************************************
#define ITMAX 100
#define EPS 3.0e-7


void gser(double *gamser, double aa, double x, double *gln)
{
  int n;
  double sum,del,ap;

  *gln=gammln(static_cast<float>(aa));
  if (x <= 0.0)
  {
    if (x < 0.0)
    { printf("*** error %s: x less than 0\n","gser");
      exit(1);
    }
    *gamser=0.0;
    return;
  }
  else
  {
    ap=aa;
    del=sum=1.0/aa;
    for (n=1;n<=ITMAX;n++)
    { ++ap;
      del *= x/ap;
      sum += del;
      if (fabs(del) < fabs(sum)*EPS)
      { *gamser=sum*exp(-x+aa*log(x)-(*gln));
        return;
      }
    }
    printf("*** error %s: a too large, ITMAX too small\n","gser");
    exit(1);
  }

} // gser

#undef ITMAX
#undef EPS
// *****************************************************************************

double gammq(double aa, double x)
{
  double gamser, gammcf, gln;

  if (x < 0.0 || a <= 0.0)
  { printf("*** error %s: invalid arguments\n","gammq");
    exit(1);
  }
  if (x < (aa+1.0))
  { gser(&gamser,aa,x,&gln);
    return 1.0-gamser;
  }
  else
  {
    gcf(&gammcf,aa,x,&gln);
    return gammcf;
  }

} // gammq
// *****************************************************************************

